import sst
from addressmap import *

class MemoryBuilder(object):
    """
    A base class for a memory tile builder
    """
    def __init__(self):
        """
        Initialize the memory tile builder
        """
        super().__init__()
        return
    
class Memory(object):
    """
    A base class for a memory tile
    """
    def __init__(self, name):
        """
        Initialize the memory tile
        """
        self.name = name
        return

    def network_if(self):
        """
        Returns the network interface subcomponent and port name
        (interface, portname) pair
        """
        raise NotImplementedError("network_if() method not implemented")


class L1SP(Memory):
    """
    A base class for a L1 SP
    """
    def __init__(self, name):
        """
        Initialize the L1 SP memory tile
        """
        super().__init__(name)
        self.memctrl = None
        self.backend = None
        self.cmdhandler = None
        self.nic = None
        return

    def network_interface(self):
        return (self.nic, "port")

class L1SPBuilder(MemoryBuilder):
    """
    A base class for a L1 SP
    """
    def __init__(self):
        """
        Initialize the L1 SP memory tile builder
        """
        super().__init__()
        self.network_bw = "1GB/s"
        self.size = 4*1024
        self.clock = "1GHz"
        self.access_time = "1ns"
        return

    def memctrl_name(self, name):
        """
        Return the name of the memory controller
        """
        return name + "_memctrl"

    @property
    def group(self):
        """
        Return the routing group number
        """
        return 1
    
    def build(self, system_builder, name):
        """
        Build the L1 SP memory tile
        """
        l1sp = L1SP(name)
        addrmap = system_builder.addressmap()
        addrrangebuilder = L1SPAddressBuilder(addrmap, self.size)
        addr_start, addr_stop, _0, _1 = addrrangebuilder(
            system_builder.pxn.id,
            system_builder.pxn.pod.id,
            system_builder.pxn.pod.compute.id,
        )
        # make the memory controller
        l1sp.memctrl = sst.Component(self.memctrl_name(name),\
                                     "memHierarchy.MemController")
        l1sp.memctrl.addParams({
            "clock" : self.clock,
            "addr_range_start" : addr_start,
            "addr_range_end" : addr_stop,
        })

        # make the backend
        l1sp.backend = l1sp.memctrl.setSubComponent("backend", "Drv.DrvSimpleMemBackend")
        l1sp.backend.addParams({
            "access_time" : self.access_time,
            "max_requests_per_cycle" : 1,            
            "mem_size" : '{}B'.format(self.size),
        })

        # make the command handler
        l1sp.cmdhandler = \
            l1sp.memctrl.setSubComponent("customCmdHandler", "Drv.DrvCmdMemHandler")
        
        # make the nic
        l1sp.nic = l1sp.memctrl.setSubComponent("cpulink", "memHierarchy.MemNIC")
        l1sp.nic.addParams({
            "group" : self.group,
            "network_bw" : self.network_bw,
        })
        return l1sp

class L2SP(Memory):
    """
    A base class for a L2SP
    """
    def __init__(self, name):
        """
        Initialize the L2 SP memory tile
        """
        super().__init__(name)
        self.memctrl = None
        self.backend = None
        self.cmdhandler = None
        self.nic = None
        return

    def network_interface(self):
        return (self.nic, "port")

class DRAM(Memory):
    """
    A base class for a DRAM
    """
    def __init__(self, name):
        """
        Initialize the DRAM memory tile
        """
        super().__init__(name)
        self.memctrl = None
        self.backend = None
        self.cmdhandler = None
        self.nic = None
        return

    def network_interface(self):
        return (self.nic, "port")
    
class L2SPBuilder(MemoryBuilder):
    """
    A base class for a L2 SP memory tile builder
    """
    def __init__(self):
        """
        Initialize the L2 SP memory tile builder
        """
        self.id = 0
        self.size = 64*1024
        self.interleave_size = 0
        self.interleave_step = 0
        self.network_bw = "1GB/s"
        return

    def memctrl_name(self, name):
        """
        Return the name of the memory controller
        """
        return name + "_memctrl"

    @property
    def group(self):
        return 2
    
    def build(self, system_builder, name):
        l2sp = L2SP(name)
        addrmap = system_builder.addressmap()
        addrrangebuilder = L2SPAddressBuilder(addrmap, \
                                              self.size, \
                                              self.interleave_size, \
                                              self.interleave_step)

        addr_start, addr_stop, addr_interleave_size, addr_interleave_step \
            = addrrangebuilder(system_builder.pxn.id, \
                               system_builder.pxn.pod.id, \
                               system_builder.pxn.pod.l2sp.id)

        l2sp.memctrl = sst.Component(self.memctrl_name(name),\
                                     "memHierarchy.MemController")
        l2sp.memctrl.addParams({
            "clock" : self.clock,
            "addr_range_start" : addr_start,
            "addr_range_end" : addr_stop,
            "interleave_size" : '{}B'.format(addr_interleave_size),
            "interleave_step" : '{}B'.format(addr_interleave_step),
        })

        l2sp.backend = l2sp.memctrl.setSubComponent("backend", "Drv.DrvSimpleMemBackend")
        l2sp.backend.addParams({
            "access_time" : self.access_time,
            "max_requests_per_cycle" : 1,
            "mem_size" : '{}B'.format(self.size),
        })

        l2sp.cmdhandler = \
            l2sp.memctrl.setSubComponent("customCmdHandler", "Drv.DrvCmdMemHandler")

        l2sp.nic = l2sp.memctrl.setSubComponent("cpulink", "memHierarchy.MemNIC")
        l2sp.nic.addParams({
            "group" : self.group,
            "network_bw" : self.network_bw,
        })
        return l2sp

class DRAMBuilder(MemoryBuilder):
    """
    A base class for a DRAM memory tile builder
    """
    def __init__(self):
        """
        Initialize the DRAM memory tile builder
        """
        super().__init__()
        self.id = 0
        self.size = 1024*1024*1024
        self.interleave_size = 0
        self.interleave_step = 0
        self.network_bw = "1GB/s"
        return

    def memctrl_name(self, name):
        """
        Return the name of the memory controller
        """
        return name + "_memctrl"

    @property
    def group(self):
        return 2
    
    def build(self, system_builder, name):
        dram = DRAM(name)
        addrmap = system_builder.addressmap()
        addrrangebuilder = DRAMAddressBuilder(addrmap, \
                                              self.size, \
                                              self.interleave_size, \
                                              self.interleave_step)
        addr_start, addr_stop, addr_interleave_size, addr_interleave_step \
            = addrrangebuilder(system_builder.pxn.id, \
                               system_builder.pxn.dram.id)
        dram.memctrl = sst.Component(self.memctrl_name(name),\
                                     "memHierarchy.MemController")
        dram.memctrl.addParams({
            "clock" : self.clock,
            "addr_range_start" : addr_start,
            "addr_range_end" : addr_stop,
            "interleave_size" : '{}B'.format(addr_interleave_size),
            "interleave_step" : '{}B'.format(addr_interleave_step),
        })

        dram.backend = dram.memctrl.setSubComponent("backend", "Drv.DrvSimpleMemBackend")
        dram.backend.addParams({
            "access_time" : self.access_time,
            "mem_size" : '{}B'.format(self.size),
            "max_requests_per_cycle" : 1,            
        })

        dram.cmdhandler = \
            dram.memctrl.setSubComponent("customCmdHandler", "Drv.DrvCmdMemHandler")

        dram.nic = dram.memctrl.setSubComponent("cpulink", "memHierarchy.MemNIC")
        dram.nic.addParams({
            "group" : self.group,
            "network_bw" : self.network_bw,
        })
        return dram

