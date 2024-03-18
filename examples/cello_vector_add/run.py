import testbench as tb
import itertools
class VectorAddTestbench(tb.CoreThreadSpeedupTestbench):
    INPUTS = [str(2**20)]
    def __init__(self, tbname):
        super().__init__(tbname, "cello_vector_add")

    def tag_prefixes(self):
        return ["vadd"]
    
    def tests(self):
        return itertools.product(self.INPUTS, self.CORES, self.THREADS)

    def test_to_mk(self, test):
        inputs, cores, threads = test
        n = inputs
        return "TESTS += $(call test-name,{threads:},{cores:},{pods:},{pxns:},{n:})\n".format(
            threads=threads, cores=cores, pods=1, pxns=1, n=n
        )

    def test_to_dir(self, test):
        inputs, cores, threads = test
        n = inputs
        return "threads_{}__cores_{}__pods_{}__pxns_{}__n_{}".format(
            threads, cores, 1, 1, n
        )

    def format_input(self, inputs):
        n = inputs
        return "n_{}".format(n)

VectorAddTestbench("cello_vector_add").run()
