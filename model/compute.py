import sst
from memory import L1SPBuilder
from addressmap import L1SPAddressBuilder
#from tile import Tile, TileBuilder

class CoreDebug(object):
    """
    A class to hold the debug configuration for a core
    """
    def __init__(self):
        self.debug_level = 0
        self.debug_init = False
        self.debug_clock = False
        self.debug_requests = False
        self.debug_responses = False
        self.debug_loopback = False
        self.debug_mmio = False
        self.debug_memory = False
        self.debug_syscalls = False
        self.trace_remote_pxn = False
        self.trace_remote_pxn_load = False
        self.trace_remote_pxn_store = False
        self.trace_remote_pxn_atomic = False
        self.isa_test = False
        self.test_name = ""

    def to_dict(self):
        return {
            "verbose" : self.debug_level,
            "debug_init" : self.debug_init,
            "debug_clock" : self.debug_clock,
            "debug_requests" : self.debug_requests,
            "debug_loopback" : self.debug_loopback,
            "debug_mmio" : self.debug_mmio,
            "debug_memory" : self.debug_memory,
            "debug_responses" : self.debug_responses,
            "debug_syscalls" : self.debug_syscalls,
            "trace_remote_pxn" : self.trace_remote_pxn,
            "trace_remote_pxn_load" : self.trace_remote_pxn_load,
            "trace_remote_pxn_store" : self.trace_remote_pxn_store,
            "trace_remote_pxn_atomic" : self.trace_remote_pxn_atomic,
            "isa_test" : self.isa_test,
            "test_name" : self.test_name
        }

class Core(object):
    """
    A base class for a core
    """
    def __init__(self, name):
        self.component = None
        self.memory = None
        self.memory_interface = None
        self.memory_nic = None
        self.name = name

    def network_interface(self):
        return (self.memory_nic, "port")

class CoreBuilder(object):
    """
    A base class for a core builder
    """
    def __init__(self):
        self.id = 0
        self.debug = CoreDebug()
        self.network_bw = "1GB/s"

    @property
    def destinations(self):
        """
        groups this core can talk to
        """
        return "0,1,2"

    @property
    def group(self):
        """
        network group
        """
        return "0"

    def build(self, system_builder, name):
        """
        Build the core
        """
        core = Core(name)
        core.component = sst.Component(core.name, self.core_model)
        core.component.addParams(self.system_params(system_builder))
        core.component.addParams(self.debug_params())
        core.component.addParams(self.core_params(system_builder))
        self.build_core_network_interface(system_builder, core)
        return core

    def system_params(self, system_builder):
        """
        Return the system parameters
        """
        return {
            "sys_num_pxn" : system_builder.pxns,
            "sys_pxn_pods" : system_builder.pxn.pods,
            "sys_pod_cores" : system_builder.pxn.pod.cores,
            "sys_core_threads" : system_builder.pxn.pod.compute.core.threads,
            "sys_core_clock" : "1GHz",
            "sys_core_l1sp_size" : system_builder.pxn.pod.compute.l1sp.size,
            "sys_pxn_dram_size" : system_builder.pxn.dram_size,
            "sys_pxn_dram_ports" : system_builder.pxn.dram_banks,
            "sys_pxn_dram_interleave_size" : system_builder.pxn.dram_interleave,
            "sys_pod_l2sp_size" : system_builder.pxn.pod.l2sp_size,
            "sys_pod_l2sp_banks" : system_builder.pxn.pod.l2sp_banks,
            "sys_pod_l2sp_interleave_size" : system_builder.pxn.pod.l2sp_interleave,
            "sys_nw_flit_dwords" : 1, # todo; get this from the system flit size
            "sys_nw_obuf_dwords" : 8, # todo: get this from... what is this used for?
            "sys_cp_present" : system_builder.pxn.hostcore_present,
        }

    def debug_params(self):
        """
        Get the debug parameters
        """
        return self.debug.to_dict()

    def build_core_network_interface(self, system_builder, core):
        """
        Build the network interface for the core
        """
        raise NotImplementedError("build_core_network_interface is not implemented")

    @property
    def core_model(self):
        """
        Get the core model
        """
        raise NotImplementedError("core_model() method not implemented")

    def core_params(self, system_builder):
        """
        Get the core parameters
        """
        raise NotImplementedError("core_params() method not implemented")

class XCoreBuilder(CoreBuilder):
    """
    Builds a DrvX core
    """
    def __init__(self):
        super().__init__()

    def build_core_network_interface(self, system_builder, core):
        """
        Build the network interface for the core
        """
        core.memory \
            = core.component.setSubComponent("memory", "Drv.DrvStdMemory")
        core.memory.addParams({
            "verbose" : self.debug.debug_level,
            "verbose_init" : self.debug.debug_init,
            "verbose_requests" : self.debug.debug_requests,
            "verbose_responses" : self.debug.debug_responses,
        })

        core.memory_interface \
            = core.memory.setSubComponent("memory", "memHierarchy.standardInterface")
        core.memory_interface.addParams({
            "verbose" : self.debug.debug_level,
        })

        core.memory_nic \
            = core.memory_interface.setSubComponent("memlink", "memHierarchy.MemNIC")
        core.memory_nic.addParams({
            "group" : self.group,
            "network_bw" : self.network_bw,
            "destinations" : self.destinations,
            "verbose_level" : 0
        })
        return core

    @property
    def core_model(self):
        """
        Get the core model
        """
        return "Drv.DrvCore"

    def core_params(self, system_builder):
        """
        Get the core parameters
        """
        return {
            "threads" : self.threads,
            "executable" : self.executable,
            "argv" : self.argv,
            "id" : self.id,
            "pod" : system_builder.pxn.pod.id,
            "pxn" : system_builder.pxn.id,
        }

class RCoreBuilder(CoreBuilder):
    """
    Builds a DrvR core
    """
    def __init__(self):
        super().__init__()

    def build_core_network_interface(self, system_builder, core):
        """
        Build the network interface for the core
        """
        core.memory_interface \
            = core.component.setSubComponent("memory", "memHierarchy.standardInterface")
        core.memory_interface.addParams({
            "verbose" : self.debug.debug_level,
        })

        core.memory_nic \
            = core.memory_interface.setSubComponent("memlink", "memHierarchy.MemNIC")
        core.memory_nic.addParams({
            "group" : self.group,
            "network_bw" : self.network_bw,
            "destinations" : self.destinations,
            "verbose_level" : 0
        })
        return core

    @property
    def core_model(self):
        """
        Get the core model
        """
        return "Drv.RISCVCore"

    def core_params(self, system_builder):
        """
        Get the core parameters
        """
        p = {
            "num_harts" : self.threads,
            "program" : self.executable,
            "core" : self.id,
            "release_reset" : 10000,
            "pod" : system_builder.pxn.pod.id,
            "pxn" : system_builder.pxn.id,
        }
        # set the stack pointer
        addrmap = system_builder.addressmap()
        addrangebuilder = L1SPAddressBuilder(addrmap, system_builder.pxn.pod.compute.l1sp.size)
        addr_start, addr_stop, *_ = addrangebuilder(system_builder.pxn.id, system_builder.pxn.pod.id, self.id)
        stack_base = addr_start
        stack_bytes = addr_stop - addr_start + 1
        stack_words = stack_bytes // 8
        thread_stack_words = stack_words // self.threads
        thread_stack_bytes = thread_stack_words * 8
        # build a string of stack pointers
        # to pass a parameter to the core
        sp_v = ["{} {}".format(i, stack_base + ((i+1)*thread_stack_bytes) - 8) for i in range(self.threads)]
        sp_str = "[" + ", ".join(sp_v) + "]"
        p["sp"] = sp_str
        return p

class Compute(object):
    """
    A base class for a compute tile
    """
    def __init__(self, name):
        """
        Initialize the compute tile
        """
        self.router = None
        self.core = None
        self.l1sp = None
        self.bridge = None
        self.name = name
        return

    def network_interface(self):
        """
        Returns the network interface subcomponent and port name
        (interface, portname) pair
        """
        return (self.bridge, "network1")


class ComputeBuilder(object):
    """
    A base class for a compute tile builder
    """
    def __init__(self):
        """
        Initialize the compute tile builder
        """
        self.id = 0
        self.clock = "1GHz"
        self.threads = 1
        self.executable = ""
        self.argv = []
        self.l1sp = L1SPBuilder()
        self.core = CoreBuilder()
        self.xbar_bw = "1GB/s"
        self.link_bw = "1GB/s"
        self.network_bw = "1GB/s"
        self.input_buf_size = "1KB"
        self.output_buf_size = "1KB"
        self.router_latency = "1ns"
        return

    def router_name(self, name):
        """
        Returns the router name
        """
        return "{}_router".format(name)

    def core_name(self, name):
        """
        Returns the core name
        """
        return "{}_core".format(name)

    def l1sp_name(self, name):
        """
        Returns the l1sp name
        """
        return "{}_l1sp".format(name)

    def bridge_name(self, name):
        """
        Returns the bridge name
        """
        return "{}_bridge".format(name)

    def core_port(self):
        return "port0"

    def l1sp_port(self):
        return "port1"

    def network_port(self):
        return "port2"

    def ports(self):
        # core + l1sp + network
        return 3

    @property
    def group(self):
        return "0"

    @property
    def destinations(self):
        return "0,1,2"

    def build(self, system_builder, name):
        """
        Build the compute tile
        """
        compute = Compute(name)

        # build the router
        compute.router = sst.Component(self.router_name(name), "merlin.hr_router")
        compute.router.addParams({
            # semantic parameters
            "id" : 0,
            "num_ports" : self.ports(),
            "topology" : "merlin.singlerouter",
            # configuration parameters
            "xbar_bw" : self.xbar_bw,
            "link_bw" : self.link_bw,
            "flit_size" : "8B",
            "input_buf_size" : self.input_buf_size,
            "output_buf_size" : self.output_buf_size,
            "input_latency" : self.router_latency,
            "output_latency" : self.router_latency,
        })
        compute.router.setSubComponent("topology", "merlin.singlerouter")

        # build the core
        self.core.id = self.id
        compute.core = self.core.build(system_builder, self.core_name(name))
        nwif, port  = compute.core.network_interface()
        link = sst.Link("{}_to_{}".format(self.router_name(name), self.core_name(name)))
        link.connect(
            (nwif, port, "1ns"),
            (compute.router, self.core_port(), "1ns")
        )

        # build the l1sp
        compute.l1sp = self.l1sp.build(system_builder, self.l1sp_name(name))
        nwif, port = compute.l1sp.network_interface()
        link = sst.Link("{}_to_{}".format(self.router_name(name), self.l1sp_name(name)))
        link.connect(
            (compute.router, self.l1sp_port(), "1ns"),
            (nwif, port, "1ns")
        )

        # build the network bridge
        compute.bridge = sst.Component(self.bridge_name(name), "merlin.Bridge")
        compute.bridge.addParams({
            "translator" : "memHierarchy.MemNetBridge",
            "network_bw" : self.network_bw,
        })
        link = sst.Link("{}_to_{}".format(self.router_name(name), self.bridge_name(name)))
        link.connect(
            (compute.router, self.network_port(), "1ns"),
            (compute.bridge, "network0", "1ns"),
        )
        return compute


