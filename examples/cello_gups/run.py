import re
import itertools
import testbench as tb

class GUPSTestbench(tb.CoreThreadSpeedupTestbench):
    INPUTS = [ (int(x),int(y)) for (x,y) in [((2**29)/8, 1e6)] ]
    
    def __init__(self, tbname):
        super().__init__(tbname, "cello_gups")

    def tag_prefixes(self):
        return ["gups"]

    def tests(self):
        return itertools.product(self.INPUTS, self.CORES, self.THREADS)

    def test_to_mk(self, test):
        inputs, cores, threads = test
        table_size, updates = inputs
        return "TESTS += $(call test-name,{threads:},{cores:},{pods:},{pxns:},{table_size:},{updates:})\n".format(
            threads=threads, cores=cores, pods=1, pxns=1, table_size=table_size, updates=updates
        )

    def test_to_dir(self, test):
        inputs, cores, threads = test
        table_size, updates = inputs
        return "threads_{}__cores_{}__pods_{}__pxns_{}__table-size_{}__updates_{}".format(
            threads, cores, 1, 1, table_size, updates
        )

    def format_input(self, test):
        table_size, updates = test
        return "table_size_{}_updates_{}".format(table_size, updates)
        
GUPSTestbench("cello_gups").run()
