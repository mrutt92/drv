import sst
from compute import ComputeBuilder, XCoreBuilder
from memory import L2SPBuilder, DRAMBuilder

    
class Pod(object):
    """
    A base class for a pod
    """
    def __init__(self, name):
        """
        Initialize the pod
        """
        self.name = name
        self.cores = []
        self.l2sp_banks = []
        self.bridge = None
        self.id = 0
        return
    
    def network_interface(self):
        """
        Returns the network interface subcomponent and port name
        (interface, portname) pair
        """
        return (self.bridge, "network1")

class PodBuilder(object):
    """
    A base class for a pod builder
    """
    def __init__(self):
        """
        Initialize the pod builder
        """
        self.compute = ComputeBuilder()
        self.l2sp_size = 1024*1024
        self.l2sp_banks = 1
        self._l2sp_interleave = self.l2sp_size//self.l2sp_banks
        self.l2sp = L2SPBuilder()
        self.xbar_bw = "1GB/s"
        self.link_bw = "1GB/s"
        self.input_buf_size = "1KB"
        self.output_buf_size = "1KB"
        self.router_latency = "0ns"
        self.network_bw = "1GB/s"
        return

    @property
    def l2sp_bank_size(self):
        return self.l2sp_size // self.l2sp_banks

    @property
    def l2sp_interleave(self):
        if self._l2sp_interleave == 0:
            return self.l2sp_bank_size
        return self._l2sp_interleave

    @l2sp_interleave.setter
    def l2sp_interleave(self, value):
        self._l2sp_interleave = value
        return

    @property
    def l2sp_interleave_step(self):
        return self.l2sp_interleave * self.l2sp_banks
    
    def router_name(self, name):
        """
        Returns the router name
        """
        return "{}_router".format(name)

    def core_name(self, name, core_id):
        """
        Returns the core name
        """
        return "{}_core{}".format(name, core_id)

    def l2sp_name(self, name, bank_id):
        """
        Returns the L2SP name
        """
        return "{}_l2sp{}".format(name, bank_id)

    def bridge_name(self, name):
        """
        Returns the bridge name
        """
        return "{}_bridge".format(name)

    def ports(self):
        """
        Returns the number of ports
        """
        # cores + l2sp banks + on-chip network
        return self.cores + self.l2sp_banks + 1

    def build(self, system_builder, name):
        pod = Pod(name)

        # build the router
        pod.network = sst.Component(self.router_name(name), "merlin.hr_router")
        pod.network.addParams({
            "id" : 0,
            "num_ports" : self.ports(),
            "topology" : "merlin.singlerouter",
            # performance models
            "xbar_bw" : self.xbar_bw,
            "link_bw" : self.link_bw,
            "flit_size" : "8B",
            "input_buf_size" : self.input_buf_size,
            "output_buf_size" : self.output_buf_size,
            "input_latency" : self.router_latency,
            "output_latency" : self.router_latency
        })
        pod.network.setSubComponent("topology", "merlin.singlerouter")
        current_port = 0

        # build the cores
        for core_id in range(self.cores):
            self.compute.id = core_id
            # core is a full compute tile in this context
            core = self.compute.build(system_builder, self.core_name(name, core_id))
            nwif, port = core.network_interface()
            link = sst.Link("link_{}_{}".format(core.name, self.router_name(name)))
            link.connect(
                (nwif, port, "1ns"),
                (pod.network, "port{}".format(current_port), "1ns")
            )
            current_port += 1        
            pod.cores.append(core)
                    
        # build the l2sp banks
        self.l2sp.size = self.l2sp_bank_size
        self.l2sp.interleave_size = self.l2sp_interleave
        self.l2sp.interleave_step = self.l2sp_interleave_step        
        for bank_id in range(self.l2sp_banks):
            self.l2sp.id = bank_id
            l2sp = self.l2sp.build(system_builder, self.l2sp_name(name, bank_id))
            nwif, port = l2sp.network_interface()
            link = sst.Link("link_{}_{}".format(l2sp.name, self.router_name(name)))
            link.connect(
                (nwif, port, "1ns"),
                (pod.network, "port{}".format(current_port), "1ns")
            )
            pod.l2sp_banks.append(l2sp)
            current_port += 1
        
        # build the bridge
        pod.bridge = sst.Component(self.bridge_name(name), "merlin.Bridge")
        pod.bridge.addParams({
            "translator" : "memHierarchy.MemNetBridge",
            "network_bw" : self.network_bw
        })
        
        link = sst.Link("link_{}_{}".format(self.router_name(name), self.bridge_name(name)))
        link.connect(
            (pod.network, "port{}".format(current_port), "1ns"),
            (pod.bridge, "network0", "1ns")
        )

        return pod
