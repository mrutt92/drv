import itertools
import sst
import enum
import addressmap

CORES_X = 16
CORES_Y = 8
X = CORES_X
Y = CORES_Y+2
MEMSIZE = 2**31

CPU_VERBOSE_LEVEL = 1
NETWORK_DEBUG_LEVEL = 1
UPDATES_PER_CORE = 100

class Mesh(object):
    def __init__(self):
        self.tiles = {}

class MeshBuilder(object):
    # x direction
    EAST =  (0, 0)
    WEST =  (0, 1)
    # y direction
    NORTH = (1, 0)
    SOUTH = (1, 1)
    # local ports
    LOCAL0 = (2, 0)
    LOCAL1 = (2, 1)

    @classmethod
    def portof(cls, direction):
        dim, neg = direction
        return 2*dim + neg

    @classmethod
    def west_port(cls):
        return cls.portof(cls.WEST)

    @classmethod
    def east_port(cls):
        return cls.portof(cls.EAST)

    @classmethod
    def north_port(cls):
        return cls.portof(cls.NORTH)

    @classmethod
    def south_port(cls):
        return cls.portof(cls.SOUTH)

    def __init__(self, xdim, ydim, meshid):
        self.xdim = xdim
        self.ydim = ydim
        self.meshid = meshid
        self.tile_builder = {
            (x,y) : ComputeTileBuilder for (x,y) in itertools.product(range(xdim), range(ydim))
        }

    def build(self):
        mesh = Mesh()
        # build all tiles
        for (x,y) in itertools.product(range(self.xdim), range(self.ydim)):
            bldr = self.tile_builder[(x,y)](self.xdim, self.ydim, self.meshid)
            mesh.tiles[(x,y)] = bldr.build(x, y)

        # connect all tiles
        for (x,y) in itertools.product(range(self.xdim), range(self.ydim)):
            # connect to north neighbor
            if y < self.ydim-1:
                link = sst.Link(f"link_{x}x{y}_to_{x}x{y+1}_mesh{self.meshid}")
                link.connect(mesh.tiles[(x,y)].north, mesh.tiles[(x,y+1)].south)
            # connect to east neighbor
            if x < self.xdim-1:
                link = sst.Link(f"link_{x}x{y}_to_{x+1}x{y}_mesh{self.meshid}")
                link.connect(mesh.tiles[(x,y)].east, mesh.tiles[(x+1,y)].west)

        return mesh

class Identifiable(object):
    def __init__(self, xdim, ydim, meshid):
        self.xdim = xdim
        self.ydim = ydim
        self.meshid = meshid

    def id(self, x, y):
        # dimension order x than y
        return y*self.xdim + x

    def absid(self, x, y):
        return self.id(x, y) + self.meshid * self.xdim * self.ydim

class Memory(object):
    def __init__(self):
        self.controller = None
        self.backend = None
        self.nic = None    

    @property
    def network_interface(self):
        return (self.nic, "port", "1ns")
    
class MemoryBuilder(Identifiable):
    size = 1024
    def __init__(self, xdim, ydim, meshid):
        super().__init__(xdim, ydim, meshid)

    def build(self, x, y):
        memory = Memory()
        memory.controller = sst.Component(f"memory_{x}_{y}_mesh{self.meshid}", "memHierarchy.MemController")
        start = self.absid(x, y) * MemoryBuilder.size
        end = (self.absid(x, y) + 1) * MemoryBuilder.size - 1
        print(f"Memory {x} {y} {start:x}-{end:x}")
        memory.controller.addParams({
            "debug_level" : 10,
            "verbose" : 0,
            "clock" : "1GHz",
            "addr_range_start" : start,
            "addr_range_end" : end,
            "interleave_size" : f"{MemoryBuilder.size}B",
            "interleave_step" : f"{X*Y*MemoryBuilder.size}B",
        })
        memory.backend = memory.controller.setSubComponent("backend", "memHierarchy.simpleMem")
        memory.backend.addParams({
            "access_time" : "1ns",
            "mem_size" : f"{MemoryBuilder.size}B",
        })
        memory.nic = memory.controller.setSubComponent("cpulink", "memHierarchy.MemNIC")
        memory.nic.addParams({
            "group" : "1",
            "network_bw" : "1024GB/s",
            "sources" : "0",
            "debug_level" : NETWORK_DEBUG_LEVEL,
            "debug" : 1,
        })
        return memory

class Core(object):
    def __init__(self):
        self.core = None
        self.generator = None
        self.interface = None
        self.nic = None

    @property
    def network_interface(self):
        return (self.nic, "port", "1ns")

class CoreBuilder(Identifiable):
    max_address = 0
    min_address = 0
    def __init__(self, xdim, ydim, meshid):
        super().__init__(xdim, ydim, meshid)

    def build(self, x, y):
        core = Core()
        core.core = sst.Component(f"core_{x}_{y}_mesh{self.meshid}", "miranda.BaseCPU")
        core.core.addParams({
            "verbose" : CPU_VERBOSE_LEVEL,
            "maxloadmemreqpending" : 1,
            "maxstorememreqpending" : 1,
            "maxcustommemreqpending" : 1,
        })
        core.generator = core.core.setSubComponent("generator", "miranda.GUPSGenerator")
        core.generator.addParams({
            "verbose" : 4,
            # todo: modify this to access DRAM
            "max_address" : CoreBuilder.max_address,
            "min_address" : CoreBuilder.min_address,
            "count" : UPDATES_PER_CORE,
            "clock" : "1GHz",
            "seed_a" : self.id(x, y),
            "seed_b" : 7*self.id(x, y)+1,
        })
        core.interface = core.core.setSubComponent("memory", "memHierarchy.standardInterface")
        core.nic = core.interface.setSubComponent("memlink", "memHierarchy.MemNIC")
        core.nic.addParams({
            "group" : "0",
            "network_bw" : "1024GB/s",
            "destinations" : "1",
            "debug_level" : NETWORK_DEBUG_LEVEL,
            "debug" : 1,
        })
        return core

class MeshTile(object):
    def __init__(self):
        self.router = None
        self.memory = None
        self.core = None

    @property
    def network_interfaces(self):
        return {
            MeshBuilder.WEST  : (self.router, f"port{MeshBuilder.west_port()}", "1ns"),
            MeshBuilder.EAST  : (self.router, f"port{MeshBuilder.east_port()}", "1ns"),
            MeshBuilder.NORTH : (self.router, f"port{MeshBuilder.north_port()}", "1ns"),
            MeshBuilder.SOUTH : (self.router, f"port{MeshBuilder.south_port()}", "1ns"),
            MeshBuilder.LOCAL0 : (self.router, f"port{MeshBuilder.portof(MeshBuilder.LOCAL0)}", "1ns"),
            MeshBuilder.LOCAL1 : (self.router, f"port{MeshBuilder.portof(MeshBuilder.LOCAL1)}", "1ns"),
        }

    @property
    def north(self):
        return self.network_interfaces[MeshBuilder.NORTH]

    @property
    def south(self):
        return self.network_interfaces[MeshBuilder.SOUTH]

    @property
    def east(self):
        return self.network_interfaces[MeshBuilder.EAST]

    @property
    def west(self):
        return self.network_interfaces[MeshBuilder.WEST]

    @property
    def local0(self):
        return self.network_interfaces[MeshBuilder.LOCAL0]

    @property
    def local1(self):
        return self.network_interfaces[MeshBuilder.LOCAL1]
    
class MeshTileBuilder(Identifiable):
    def __init__(self, xdim, ydim, meshid):
        self.local_ports = 2
        super().__init__(xdim, ydim, meshid)

    @property
    def num_ports(self):
        return self.local_ports + 4

    def build_router(self, x, y):
        router = sst.Component(f"router_{x}_{y}_mesh{self.meshid}", "merlin.hr_router")
        router.addParams({
            "id" : self.id(x, y),
            "num_vns" : 2,
            "xbar_bw" : "1024GB/s",
            "link_bw" : "1024GB/s",
            "input_latency" : "1ns",
            "output_latency" : "1ns",
            "input_buf_size" : "1KB",
            "output_buf_size" : "1KB",
            "flit_size" : "8B",
            "num_ports" : self.num_ports,
        })
        topo = router.setSubComponent("topology", "merlin.mesh")
        topo.addParams({
            "shape" : f"{X}x{Y}",
            "width" : "1",
            "local_ports" : "2",
        })
        return router
            
    def build(self, x, y):
        mesh_tile = self.make_mesh_tile()
        mesh_tile.router = self.build_router(x, y)
        self.build_local_endpoints(x, y, mesh_tile)
        return mesh_tile

    def make_mesh_tile(self):
        raise NotImplementedError("MeshTileBuilder.make_mesh_tile")
    
    def build_local_endpoints(self, x, y, mesh_tile):
        raise NotImplementedError("MeshTile.build_local_endpoints")

class ComputeTile(MeshTile):
    def __init__(self):
        super().__init__()
        self.core = None
        self.memory = None

class ComputeTileBuilder(MeshTileBuilder):
    def __init__(self, xdim, ydim, meshid):
        self.core = CoreBuilder(xdim, ydim, meshid)
        self.memory = MemoryBuilder(xdim, ydim, meshid)
        super().__init__(xdim, ydim, meshid)

    def make_mesh_tile(self):
        return ComputeTile()

    def build_local_endpoints(self, x, y, tile):
        print(f"ComputeTile {x} {y}")
        tile.core = self.core.build(x, y)
        link = sst.Link(f"link_core_router_{x}_{y}_mesh{self.meshid}")
        link.connect(tile.core.network_interface, tile.local0)

        tile.memory = self.memory.build(x, y)
        link = sst.Link(f"link_router_memory_{x}_{y}_mesh{self.meshid}")
        link.connect(tile.memory.network_interface, tile.local1)

class VictimCache(object):
    def __init__(self):
        self.cache = None
        self.cpulink = None
        self.memlink = None

    @property
    def network_interface(self):
        return (self.cpulink, "port", "1ns")

    @property
    def memory_interface(self):
        return (self.memlink, "port", "1ns")

class VictimCacheBuilder(Identifiable):
    # use this to control all victim caches
    sysconfig = None
    banks = 0
    bank_id = 0
    memsize = 2**20
    cache_line_size = 64
    def __init__(self, xdim, ydim, meshid):
        super().__init__(xdim, ydim, meshid)

    def addressmap(self):
        return addressmap.AddressMap(self.sysconfig)

    @property
    def bank_size(self):
        return self.memsize // self.banks
    
    @classmethod
    def new_bank_id(cls):
        r = cls.bank_id
        cls.bank_id += 1
        return r

    def address(self, bank_id):
        addrmap = self.addressmap()
        builder = addressmap.DRAMAddressBuilder(addrmap, self.bank_size, self.cache_line_size, self.banks * self.cache_line_size)
        start, stop, interleave, stride = builder(0, bank_id)
        start -= 0xc000_0000_0000_0000
        start += 0x0000_0000_8000_0000
        stop  -= 0xc000_0000_0000_0000
        stop  += 0x0000_0000_8000_0000
        return (start, stop, interleave, stride)
        
    def build(self, x, y):
        bank_id = self.new_bank_id()
        start, stop, interleave, stride = self.address(bank_id)
        victim_cache = VictimCache()
        victim_cache.cache = sst.Component(f"victim_cache_{x}_{y}_mesh{self.meshid}", "memHierarchy.Cache")
        victim_cache.cache.addParams({
            "cache_frequency" : "1GHz",
            "cache_size" : "1KB",
            "associativity" : "2",
            "access_latency_cycles" : "1",
            "replacement_policy" : "lru",
            "mshr_num_entries" : "2",
            "L1" : "true",
            "cache_line_size" : self.cache_line_size,
            "coherence_protocol" : "mesi",
            "cache_type" : "inclusive",
            "addr_range_start" : start,
            "addr_range_end" : stop,
            "interleave_size" : f'{interleave}B',
            "interleave_step" : f'{stride}B',
        })
        victim_cache.cpulink = victim_cache.cache.setSubComponent("cpulink", "memHierarchy.MemNIC")
        victim_cache.cpulink.addParams({
            "group" : 1,
            "network_bw" : "1024GB/s",
        })
        victim_cache.memlink = victim_cache.cache.setSubComponent("memlink", "memHierarchy.MemLink")
        return victim_cache

class VictimCacheTile(MeshTile):
    def __init__(self):
        super().__init__()
        self.victim_cache = None

class VictimCacheTileBuilder(MeshTileBuilder):
    def __init__(self, xdim, ydim, meshid):
        self.victim_cache_builder = VictimCacheBuilder(xdim, ydim, meshid)
        super().__init__(xdim, ydim, meshid)

    def make_mesh_tile(self):
        return VictimCacheTile()

    def build_local_endpoints(self, x, y, tile):
        tile.victim_cache = self.victim_cache_builder.build(x, y)
        link = sst.Link(f"link_router_memory_{x}_{y}_mesh{self.meshid}")
        link.connect(tile.victim_cache.network_interface, tile.local0)
        
class EmptyTile(MeshTile):
    def __init__(self):
        super().__init__()

class EmptyTileBuilder(MeshTileBuilder):
    def __init__(self, xdim, ydim, meshid):
        super().__init__(xdim, ydim, meshid)

    def make_mesh_tile(self):
        return EmptyTile()

    def build_local_endpoints(self, x, y, tile):
        pass

class Sysconfig(object):
    def __init__(self, num_cores):
        self.num_cores = num_cores

    def cores(self):
        return self.num_cores

    def pxns(self):
        return 1

    def pods(self):
        return 1

if __name__ == "__main__":
    mesh_builder = MeshBuilder(X, Y, 0)
    VictimCacheBuilder.sysconfig = Sysconfig(CORES_X*CORES_Y)
    VictimCacheBuilder.memsize = MEMSIZE
    VictimCacheBuilder.banks = 2*X

    # create the memory address range
    addrmap = addressmap.AddressMap(VictimCacheBuilder.sysconfig)
    range_builder = addressmap.DRAMAddressBuilder(addrmap, VictimCacheBuilder.memsize, 0, 0)
    start, stop, interleave, stride = range_builder(0, 0)
    start -= 0xc000_0000_0000_0000
    start += 0x0000_0000_8000_0000
    stop  -= 0xc000_0000_0000_0000
    stop  += 0x0000_0000_8000_0000
    print(f"Memory range: {start:08x} - {stop:08x}")

    for x in range(X):
        mesh_builder.tile_builder[(x,0)]   = VictimCacheTileBuilder
        mesh_builder.tile_builder[(x,Y-1)] = VictimCacheTileBuilder

    CoreBuilder.max_address = stop-8
    CoreBuilder.min_address = start

    # create a memory
    memory = sst.Component("memory", "memHierarchy.MemController")
    memory.addParams({
        "clock" : "1GHz",
        "addr_range_start" : start,
        "addr_range_end" : stop,
        "interleave_size" : f'{interleave}B',
        "interleave_step" : f'{stride}B',
        "max_requests_per_cycle" : 1,
    })
    backend = memory.setSubComponent("backend", "memHierarchy.simpleMem")
    backend.addParams({
        "mem_size" : f"{VictimCacheBuilder.memsize}B",
        "access_time" : "1ns",
    })
    memlink = memory.setSubComponent("cpulink", "memHierarchy.MemLink")

    # create a bus
    bus = sst.Component("bus", "memHierarchy.Bus")
    bus.addParams({
        "bus_frequency" : "1GHz",
        "bus_latency" : "1ns",
    })

    # connect memory to bus
    link = sst.Link("link_memory_bus")
    link.connect((memlink, "port", "1ns"), (bus, "low_network_0", "1ns"))

    # build the mesh
    mesh = mesh_builder.build()

    # connect bus to vcs
    for (i, (x,y)) in enumerate(itertools.product(range(X), (0, Y-1))):
        # connect vc to memory backend
        vc_tile = mesh.tiles[(x,y)]
        link = sst.Link(f"link_vc_memory_{x}_{y}_mesh0")
        link.connect(vc_tile.victim_cache.memory_interface, (bus, f"high_network_{i}", "1ns"))


