import itertools
from sst.merlin import *
import sst
import enum

X = 2
Y = 2

NORTH = (0, 0)
SOUTH = (0, 1)
EAST =  (1, 0)
WEST =  (1, 1)

class Identifiable(object):
    def id(self, x, y):
        return x * Y + y

CPU_VERBOSE_LEVEL = 1
NETWORK_DEBUG_LEVEL = 0

def portof(direction):
    dim, neg = direction
    return 2*dim + neg

DIRECTIONS = [NORTH, SOUTH, EAST, WEST]

class Memory(Identifiable):
    size = 1024
    def __init__(self):
        pass

    def build(self, x, y):
        memory = sst.Component(f"memory_{x}_{y}", "memHierarchy.MemController")
        start = self.id(x, y) * Memory.size
        end = (self.id(x, y) + 1) * Memory.size - 1
        print(f"Memory {x} {y} {start:x}-{end:x}")
        memory.addParams({
            "debug_level" : 10,
            "verbose" : 0,
            "clock" : "1GHz",
            "addr_range_start" : start,
            "addr_range_end" : end,
            "interleave_size" : f"{Memory.size}B",
            "interleave_step" : f"{X*Y*Memory.size}B",
        })
        backend = memory.setSubComponent("backend", "memHierarchy.simpleMem")
        backend.addParams({
            "access_time" : "1ns",
            "mem_size" : f"{Memory.size}B",
        })
        nic = memory.setSubComponent("cpulink", "memHierarchy.MemNIC")
        nic.addParams({
            "group" : "1",
            "network_bw" : "1024GB/s",
            "sources" : "0",
            "debug_level" : NETWORK_DEBUG_LEVEL,
            "debug" : 1,
        })
        return (nic, "port", "1ns")

class Core(Identifiable):
    def __init__(self):
        pass

    def build(self, x, y):
        core = sst.Component(f"core_{x}_{y}", "miranda.BaseCPU")
        core.addParams({
            "verbose" : CPU_VERBOSE_LEVEL,
            "maxloadmemreqpending" : 1,
            "maxstorememreqpending" : 1,
            "maxcustommemreqpending" : 1,
        })
        generator = core.setSubComponent("generator", "miranda.GUPSGenerator")
        generator.addParams({
            "verbose" : 4,            
            "max_address" : Memory.size * X * Y - 8,
            "count" : 100,
            "clock" : "1GHz",
            "seed_a" : self.id(x, y),
            "seed_b" : 7*self.id(x, y)+1,
        })
        interface = core.setSubComponent("memory", "memHierarchy.standardInterface")
        nic = interface.setSubComponent("memlink", "memHierarchy.MemNIC")
        nic.addParams({
            "group" : "0",
            "network_bw" : "1024GB/s",
            "destinations" : "1",
            "debug_level" : NETWORK_DEBUG_LEVEL,
            "debug" : 1,
        })
        return (nic, "port", "1ns")
    
class Tile(Identifiable):
    def __init__(self):
        self.core = Core()
        self.memory = Memory()

    def num_ports(self, x, y):
        local = 2
        network = 4
        return local + network    

    def ports(self, x, y, router):
        ports = {
            WEST  : (router, f"port{portof(WEST)}", "1ns"),
            EAST  : (router, f"port{portof(EAST)}", "1ns"),
            NORTH : (router, f"port{portof(NORTH)}", "1ns"),
            SOUTH : (router, f"port{portof(SOUTH)}", "1ns"),
        }
        return ports

    def core_port(self):
        return "port4"

    def memory_port(self):
        return "port5"
    
    def build(self, x, y):
        router = sst.Component(f"router_{x:02}_{y:02}", "merlin.hr_router")
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
            "num_ports" : self.num_ports(x,y),
        })
        topo = router.setSubComponent("topology", "merlin.mesh")
        topo.addParams({
            "shape" : f"{X}x{Y}",
            "width" : "1",
            "local_ports" : "2",
        })
        core_if, core_port, core_latency = self.core.build(x, y)
        link = sst.Link(f"link_core_router_{x}_{y}")
        link.connect((core_if, core_port, core_latency), (router, self.core_port(), "1ns"))

        mem_if, mem_port, mem_latency = self.memory.build(x, y)
        link = sst.Link(f"link_router_memory_{x}_{y}")
        link.connect((router, self.memory_port(), "1ns"), (mem_if, mem_port, mem_latency))
        return self.ports(x, y, router)
        
tile = Tile()
nodes = {}
for (x,y) in itertools.product(range(X), range(Y)):
    nodes[(x,y)] = tile.build(x, y)

link_00_10 = sst.Link("link_00_10")
link_00_01 = sst.Link("link_00_01")
link_01_11 = sst.Link("link_01_11")
link_10_11 = sst.Link("link_10_11")

link_00_01.connect(nodes[(0,0)][NORTH], nodes[(0,1)][SOUTH])
link_00_10.connect(nodes[(0,0)][EAST],  nodes[(1,0)][WEST])
link_01_11.connect(nodes[(0,1)][EAST],  nodes[(1,1)][WEST])
link_10_11.connect(nodes[(1,0)][NORTH], nodes[(1,1)][SOUTH])

