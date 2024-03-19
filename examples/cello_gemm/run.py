from testbench import *
import itertools

class GEMMTestbench(CoreThreadSpeedupTestbench):
    INPUTS = [(8, 8, 8)]
    def __init__(self, tbname):
        super().__init__(tbname, tbname)

    def tag_prefixes(self):
        return ["gemm"]
    
    def tests(self):
        return itertools.product(self.INPUTS, self.CORES, self.THREADS)
    
    def test_to_mk(self, test):
        inputs, cores, threads = test
        n, m, k = inputs
        return "TESTS += $(call test-name,{threads:},{cores:},{pods:},{pxns:},{n:},{m:},{k:})\n".format(
            threads=threads, cores=cores, pods=1, pxns=1, n=n, m=m, k=k
        )

    def test_to_dir(self, test):
        inputs, cores, threads = test
        n, m, k = inputs
        return "threads_{}__cores_{}__pods_{}__pxns_{}__n_{}__m_{}__k_{}".format(
            threads, cores, 1, 1, n, m, k
        )

    def format_input(self, inputs):
        n, m, k = inputs
        return "n_{}__m_{}__k_{}".format(n, m, k)
    
GEMMTestbench("cello_gemm").run()
