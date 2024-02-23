import re
import itertools
import testbench as tb

class SPMMTestbench(tb.Testbench):
    CORES = [1, 2]
    THREADS = [1, 2]
    INPUTS = [('u7k1','g7k1')]

    def __init__(self, tbname):
        super().__init__(tbname)

    def tests(self):
        return itertools.product(self.INPUTS, self.CORES, self.THREADS)

    def test_to_mk(self, test):
        inputs, cores, threads = test
        m0, m1 = inputs
        return "TESTS += $(call test-name,{threads:},{cores:},{pods:},{pxns:},{m0:},{m1:})\n".format(
            threads=threads, cores=cores, pods=1, pxns=1, m0=m0, m1=m1
        )

    def result_header(self):
        return "Application,Input,Sim Options,PXN,Pods,Cores,Threads,Seconds\n"

    def test_to_dir(self, test):
        inputs, cores, threads = test
        m0, m1 = inputs
        return "threads_{}__cores_{}__pods_{}__pxns_{}__m0_{}__m1_{}".format(
            threads, cores, 1, 1, m0, m1
        )

    def parse_seconds(self, line):
        match = re.search(r'row-wise product: Elapsed time: ([0-9.]+) seconds', line)        
        if match:
            return float(match.group(1))

        match = re.search(r'product to csr: Elapsed time: ([0-9.]+) seconds', line)
        if match:
            return float(match.group(1))

        return 0.0

    def result(self, test, sim_options, seconds):
        inputs, cores, threads = test
        m0, m1 = inputs
        return "{Application:},{Input:},{SimOptions:},{PXN:},{Pods:},{Cores:},{Threads:},{Seconds:1.12f}\n".format(
            Application="cello_spmm",
            Input="m0_{}__m1_{}".format(m0, m1),
            SimOptions=sim_options,
            PXN=1,
            Pods=1,
            Cores=cores,
            Threads=threads,
            Seconds=seconds,
        )
    
SPMMTestbench("cello_spmm").run()
