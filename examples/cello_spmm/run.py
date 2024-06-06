import re
import itertools
import testbench as tb

class SPMMTestbench(tb.CoreThreadSpeedupTestbench):
    INPUTS = [('u10k4','g10k4')]

    def __init__(self, tbname):
        super().__init__(tbname, "cello_spmm")

    def tests(self):
        return itertools.product(self.INPUTS, self.CORES, self.THREADS)

    def test_to_mk(self, test):
        inputs, cores, threads = test
        m0, m1 = inputs
        return "TESTS += $(call test-name,{threads:},{cores:},{pods:},{pxns:},{m0:},{m1:})\n".format(
            threads=threads, cores=cores, pods=1, pxns=1, m0=m0, m1=m1
        )

    def test_to_dir(self, test):
        inputs, cores, threads = test
        m0, m1 = inputs
        return "threads_{}__cores_{}__pods_{}__pxns_{}__m0_{}__m1_{}".format(
            threads, cores, 1, 1, m0, m1
        )

    def tag_prefixes(self):
        return ["row-wise product"]
    
    def format_input(self, inputs):
        m0, m1 = inputs
        return "m0_{}__m1_{}".format(m0, m1)
    
SPMMTestbench("cello_spmm").run()
