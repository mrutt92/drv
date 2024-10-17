from compute import XCoreBuilder, RCoreBuilder, ComputeBuilder
from memory import L1SPBuilder, L2SPBuilder, DRAMBuilder
from pod import PodBuilder
from pxn import PXNBuilder
from system import SystemBuilder

class PANDOHammer(object):
    """
    A PANDOHammer Simulation
    """
    def __init__(self, arguments, core_builder=XCoreBuilder):
        """
        Initialize the PANDOHammer Simulation
        arguments are parsed from the command line
        """
        # l1sp
        l1sp = L1SPBuilder()
        l1sp.clock = "1GHz"
        l1sp.access_time = "1ns"
        l1sp.size = arguments.core_l1sp_size
        
        # core
        core = core_builder()
        core.clock = "1GHz"
        core.threads = arguments.core_threads
        core.executable = arguments.program
        core.argv = ' '.join(arguments.argv)
        core.threads = arguments.core_threads
        
        # compute tile
        compute = ComputeBuilder()
        compute.l1sp = l1sp
        compute.core = core
        
        # l2sp tile
        l2sp = L2SPBuilder()
        l2sp.clock = "1GHz"
        l2sp.access_time = "10ns"
        
        # pod
        pod = PodBuilder()
        pod.compute = compute
        pod.l2sp = l2sp
        pod.cores = arguments.pod_cores
        pod.l2sp_size = arguments.pod_l2sp_size
        pod.l2sp_banks = arguments.pod_l2sp_banks
        pod.l2sp_interleave = arguments.pod_l2sp_interleave
    
        # host core
        hostcore = XCoreBuilder()
        hostcore.clock = "1GHz"
        hostcore.threads = 1
        hostcore.executable = arguments.with_command_processor
        hostcore.argv = ' '.join([arguments.program] + arguments.argv)
        
        # dram
        dram = DRAMBuilder()
        dram.backend = "simple"
        dram.clock = "1GHz"
        dram.access_time = "100ns"
        
        # pxn
        pxn = PXNBuilder()
        pxn.pod = pod
        pxn.pods = arguments.pxn_pods
        pxn.hostcore_present = bool(arguments.with_command_processor)
        pxn.hostcore = hostcore
        pxn.dram = dram
        pxn.dram_size = arguments.pxn_dram_size
        pxn.dram_banks = arguments.pxn_dram_banks
        pxn.dram_interleave = arguments.pxn_dram_interleave
        
        # system
        system = SystemBuilder()
        system.pxn = pxn
        system.pxns = arguments.num_pxn
        self.system = system

        return

    def build(self):
        """
        Build the PANDOHammer Simulation
        """
        return self.system.build()

