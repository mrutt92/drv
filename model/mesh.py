import itertools
import sst
import enum
import addressmap
from addressmap import Bitfield
from clock import Clock
from cmdline import parse_args

ARGUMENTS = parse_args()

CORES_X = 16
CORES_Y = 8
X = CORES_X
Y = CORES_Y+2
MEMSIZE = 2**31
CACHE_LINE_SIZE = 64

CPU_VERBOSE_LEVEL = 1
NETWORK_DEBUG_LEVEL = 1
UPDATES_PER_CORE = 1000

CORE_CLOCK = Clock(1e9)
MEMORY_CLOCK = Clock(1e9)

NETWORK_BANDWIDTH = f'{CORE_CLOCK * 8 * 3}B/s'
XBAR_BANDWIDTH = f'{CORE_CLOCK * 8 * 3 * 6}B/s'

class AddressType(enum.Enum):
    """
    An address type.
    """
    L1SP = 0
    DRAM = 1
    CTRL = 2

class AddressInfo(object):
    """
    A decoded address.
    """
    def __init__(self):
        self.address_type = AddressType.DRAM
        self._offset = 0
        self.core_x = 0
        self.core_y = 0
        self.pod = 0
        self.local = False

    def __str__(self):
        if self.address_type in (AddressType.L1SP, AddressType.CTRL):
            return f"<{self.address_type.name} pod={self.pod} y={self.core_x} x={self.core_y} {self._offset:08x}>"
        return f"<{self.address_type.name} pod={self.pod} {self._offset:08x}>"

    def is_l1sp(self):
        return self.address_type == AddressType.L1SP

    def set_l1sp(self):
        self.address_type = AddressType.L1SP
        return self

    def is_dram(self):
        return self.address_type == AddressType.DRAM

    def set_dram(self):
        self.address_type = AddressType.DRAM
        return self

    def is_ctrl(self):
        return self.address_type == AddressType.CTRL

    def set_ctrl(self):
        self.address_type = AddressType.CTRL
        return self

    def offset(self):
        return self._offset

    def set_offset(self, offset):
        self._offset = offset
        return self

    def core_x(self):
        return self.core_x

    def set_core_x(self, core_x):
        self.core_x = core_x
        return self

    def core_y(self):
        return self.core_y

    def set_core_y(self, core_y):
        self.core_y = core_y

    def pod(self):
        return self.pod

    def set_pod(self, pod):
        self.pod = pod
        return self

    def is_local(self):
        return self.local

    def set_local(self):
        self.local = True
        return self

    def is_global(self):
        return not self.local

    def set_global(self):
        self.local = False
        return self

class AddressMap(object):
    """
    Decodes and encodes addresses.
    """
    def __init__(self):
        self.is_dram = Bitfield(31)
        self.is_remote_bit = Bitfield(29)
        self.y = Bitfield(28, 24)
        self.x = Bitfield(23, 18)
        self.dram_offset = Bitfield(30, 0)
        self.l1sp_offset = Bitfield(17, 0)

    def is_remote(self, addr):
        return not self.is_dram(addr) and self.is_remote_bit(addr)

    def decode(self, addr, my_x = 0, my_y = 0):
        """
        Returns an AddressInfo() object.
        """
        info = AddressInfo()
        if self.is_dram(addr):
            info.set_dram()\
                .set_global()\
                .set_oiffset(self.dram_offset(addr))
        elif self.is_remote(addr):
            info.set_l1sp()\
                .set_global()\
                .set_offset(self.l1sp_offset(addr))\
                .set_core_x(self.x(addr))\
                .set_core_y(self.y(addr))
        else:
            info.set_l1sp()\
                .set_local()\
                .set_offset(self.l1sp_offset(addr))\
                .set_core_x(my_x)\
                .set_core_y(my_y)
        return info

    def encode(self, info):
        """
        Returns an address.
        """
        address = 0
        # dram
        if info.is_dram():
            address = self.is_dram.set(address, 1)
            address = self.dram_offset.set(address, info.offset())
        # remote l1sp
        elif info.is_l1sp() and info.is_global():
            address = self.is_remote_bit.set(address, 1)
            address = self.x.set(address, info.core_x())
            address = self.y.set(address, info.core_y())
            address = self.l1sp_offset.set(address, info.offset())
        # local l1sp
        elif info.is_l1sp() and info.is_local():
            address = self.l1sp_offset.set(address, info.offset())
        # return the address
        return address

def dram_range(bank_id, banks, memsize, interleave):
    """
    Address range for a dram bank.
    return addr_start, addr_end, interleave, stride
    """
    address_map = AddressMap()
    bank_size = memsize // banks

    if banks == 1:
        bank_id = 0

    stride = interleave * banks
    start = bank_id * interleave
    stop = memsize - (banks - bank_id - 1) * interleave - 1

    start_info = AddressInfo().set_dram().set_global().set_offset(start)
    stop_info = AddressInfo().set_dram().set_global().set_offset(stop)
    return (address_map.encode(start_info),
            address_map.encode(stop_info),
            interleave,
            stride)

def vcache_range(xdim, ydim, x, south_not_north, memsize, interleave):
    """
    Address range for a vcache bank.
    return addr_start, addr_end, interleave, stride
    """
    banks = xdim*2 # north and south
    snn = 1 if south_not_north else 0
    bank_id = x + snn*xdim
    return dram_range(bank_id, banks, memsize, interleave)


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
        return (self.nic, "port", f'{CORE_CLOCK.cycle_ps}ps')
    
class MemoryBuilder(Identifiable):
    size = 4*1024
    bandwidth = 8e9 # 8GB/s
    def __init__(self, xdim, ydim, meshid):
        super().__init__(xdim, ydim, meshid)

    def build(self, x, y):
        memory = Memory()
        memory.controller = sst.Component(f"memory_{x}_{y}_mesh{self.meshid}",
                                          "memHierarchy.MemController")
        start = self.absid(x, y) * MemoryBuilder.size
        end = (self.absid(x, y) + 1) * MemoryBuilder.size - 1
        memory.controller.addParams({
            "debug_level" : 10,
            "verbose" : 0,
            "clock" : f'{CORE_CLOCK}Hz',
            "addr_range_start" : start,
            "addr_range_end" : end,
            "interleave_size" : f"{MemoryBuilder.size}B",
            "interleave_step" : f"{X*Y*MemoryBuilder.size}B",
        })
        memory.backend = memory.controller.setSubComponent("backend",
                                                           "memHierarchy.simpleMem")
        memory.backend.addParams({
            "access_time" : f'{CORE_CLOCK.cycle_ps}ps',
            "mem_size" : f"{MemoryBuilder.size}B",
        })
        memory.nic = memory.controller.setSubComponent("cpulink",
                                                       "memHierarchy.MemNIC")
        memory.nic.addParams({
            "group" : "1",
            "network_bw" : NETWORK_BANDWIDTH,
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
        return (self.nic, "port", f'{CORE_CLOCK.cycle_ps}ps')

class MirandaCoreBuilder(Identifiable):
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
            "max_address" : MirandaCoreBuilder.max_address,
            "min_address" : MirandaCoreBuilder.min_address,
            "count" : UPDATES_PER_CORE if x == 1 and y == 1 else 0,
            "clock" : f'{CORE_CLOCK}Hz',
            "seed_a" : self.id(x, y),
            "seed_b" : 7*self.id(x, y)+1,
        })
        core.interface = core.core.setSubComponent("memory", "memHierarchy.standardInterface")
        core.nic = core.interface.setSubComponent("memlink", "memHierarchy.MemNIC")
        core.nic.addParams({
            "group" : "0",
            "network_bw" : NETWORK_BANDWIDTH,
            "destinations" : "1",
            "debug_level" : NETWORK_DEBUG_LEVEL,
            "debug" : 1,
        })
        return core

class DrvXCoreBuilder(Identifiable):
    def __init__(self, xdim, ydim, meshid):
        super().__init__(xdim, ydim, meshid)

    def core_id(self, x, y):
        return x + (y-1) * self.xdim
    
    def build(self, x, y):
        core = Core()
        core.core = sst.Component(f"core_{x}_{y}_mesh{self.meshid}", "Drv.DrvCore")
        core.core.addParams({
            "clock" : f'{CORE_CLOCK}Hz',
            "max_idle" : 2,
            "threads" : 1,
            "executable" : ARGUMENTS.program,
            "argv" : ' '.join(ARGUMENTS.argv),
            "id" : self.core_id(x, y),
            "pod" : 0,
            "pxn" : 0,
            "sys_num_pxn" : 1,
            "sys_pxn_pods" : 1,
            "sys_pod_cores" : CORES_X*CORES_Y,
            "sys_core_threads" : 1,
            "sys_core_clock" : f'{CORE_CLOCK}Hz',
            "sys_core_l1sp_size" : MemoryBuilder.size,
            "sys_pod_l2sp_size" : 0,
            "sys_pod_l2sp_banks" : 0,
            "sys_pod_l2sp_interleave_size" : 0,
            "sys_nw_flit_dwords" : 1,
            "sys_nw_obuf_dwords" : 24,
            "sys_cp_present" : False,
        })
        core.memory = core.core.setSubComponent("memory", "Drv.DrvStdMemory")
        core.interface = core.memory.setSubComponent("memory", "memHierarchy.standardInterface")
        core.nic = core.interface.setSubComponent("memlink", "memHierarchy.MemNIC")
        core.nic.addParams({
            "group" : "0",
            "network_bw" : NETWORK_BANDWIDTH,
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
            MeshBuilder.WEST  : (self.router, f"port{MeshBuilder.west_port()}", f'{CORE_CLOCK.cycle_ps}ps'),
            MeshBuilder.EAST  : (self.router, f"port{MeshBuilder.east_port()}", f'{CORE_CLOCK.cycle_ps}ps'),
            MeshBuilder.NORTH : (self.router, f"port{MeshBuilder.north_port()}", f'{CORE_CLOCK.cycle_ps}ps'),
            MeshBuilder.SOUTH : (self.router, f"port{MeshBuilder.south_port()}", f'{CORE_CLOCK.cycle_ps}ps'),
            MeshBuilder.LOCAL0 : (self.router, f"port{MeshBuilder.portof(MeshBuilder.LOCAL0)}", f'{CORE_CLOCK.cycle_ps}ps'),
            MeshBuilder.LOCAL1 : (self.router, f"port{MeshBuilder.portof(MeshBuilder.LOCAL1)}", f'{CORE_CLOCK.cycle_ps}ps'),
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
            "xbar_bw" : XBAR_BANDWIDTH,
            "link_bw" : XBAR_BANDWIDTH,
            "input_latency" : f'{0*CORE_CLOCK.cycle_ps}ps',
            "output_latency" : f'{0*CORE_CLOCK.cycle_ps}ps',
            "input_buf_size" : f"{2*3*8}B",
            "output_buf_size" : f"{2*3*8}B",
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
        self.core = DrvXCoreBuilder(xdim, ydim, meshid)
        self.memory = MemoryBuilder(xdim, ydim, meshid)
        super().__init__(xdim, ydim, meshid)

    def make_mesh_tile(self):
        return ComputeTile()

    def build_local_endpoints(self, x, y, tile):
        #print(f"ComputeTile {x} {y}")
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
        return (self.cpulink, "port", f'{CORE_CLOCK.cycle_ps}ps')

    @property
    def memory_interface(self):
        return (self.memlink, "port", f'{CORE_CLOCK.cycle_ps}ps')

class VictimCacheBuilder(Identifiable):
    # use this to control all victim caches
    sysconfig = None
    banks = 0
    memsize = MEMSIZE
    cache_line_size = CACHE_LINE_SIZE
    def __init__(self, xdim, ydim, meshid):
        super().__init__(xdim, ydim, meshid)

    def addressmap(self):
        return addressmap.AddressMap(self.sysconfig)

    @property
    def bank_size(self):
        return self.memsize // self.banks
    
    def build(self, x, y):
        start, stop, interleave, stride \
            = vcache_range(self.xdim, self.ydim, x, not (y==0), \
                           self.memsize, self.cache_line_size)

        victim_cache = VictimCache()
        victim_cache.cache = sst.Component(f"victim_cache_{x}_{y}_mesh{self.meshid}",\
                                           "memHierarchy.Cache")
        victim_cache.cache.addParams({
            "cache_frequency" : f'{CORE_CLOCK}Hz',
            "cache_size" : "1KB",
            "associativity" : "2",
            "access_latency_cycles" : '1',
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
            "network_bw" : NETWORK_BANDWIDTH,
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
    start, stop, interleave, stride \
        = dram_range(0, 1, MEMSIZE, VictimCacheBuilder.cache_line_size)

    #print(f"Memory range: {start:08x} - {stop:08x}")

    for x in range(X):
        mesh_builder.tile_builder[(x,0)]   = VictimCacheTileBuilder
        mesh_builder.tile_builder[(x,Y-1)] = VictimCacheTileBuilder

    MirandaCoreBuilder.max_address = stop-8
    MirandaCoreBuilder.min_address = start

    # create a memory
    memory = sst.Component("memory", "memHierarchy.MemController")
    memory.addParams({
        "clock" : f'{MEMORY_CLOCK}Hz',
        "addr_range_start" : start,
        "addr_range_end" : stop,
        "interleave_size" : f'{interleave}B',
        "interleave_step" : f'{stride}B',
        "max_requests_per_cycle" : 1,
    })
    backend = memory.setSubComponent("backend", "memHierarchy.simpleMem")
    backend.addParams({
        "mem_size" : f"{VictimCacheBuilder.memsize}B",
        "access_time" : f'{MEMORY_CLOCK.cycle_ps * 2}ps',
    })
    memlink = memory.setSubComponent("cpulink", "memHierarchy.MemLink")

    # create a bus
    bus = sst.Component("bus", "memHierarchy.Bus")
    bus.addParams({
        "bus_frequency" : f'{CORE_CLOCK}Hz',
        "bus_latency" : f'{CORE_CLOCK.cycle_ps}ps',
    })

    # connect memory to bus
    link = sst.Link("link_memory_bus")
    link.connect(
        (memlink, "port", f'{CORE_CLOCK.cycle_ps}ps'),
        (bus, "low_network_0", f'{CORE_CLOCK.cycle_ps}ps')
    )

    # build the mesh
    mesh = mesh_builder.build()

    # connect bus to vcs
    for (i, (x,y)) in enumerate(itertools.product(range(X), (0, Y-1))):
        # connect vc to memory backend
        vc_tile = mesh.tiles[(x,y)]
        link = sst.Link(f"link_vc_memory_{x}_{y}_mesh0")
        link.connect(vc_tile.victim_cache.memory_interface, (bus, f"high_network_{i}", "1ns"))


