import re
import itertools
import testbench as tb

class TCTestbench(tb.CoreThreadSpeedupTestbench):
    INPUTS = ['u12k16']

    def __init__(self, tbname):
        super().__init__(tbname, "cello_tc")

    def tag_prefixes(self):
        return ["triangle counting"]
    
    def tests(self):
        return itertools.product(self.INPUTS, self.CORES, self.THREADS)

    def test_to_mk(self, test):
        inputs, cores, threads = test
        graph = inputs
        return "TESTS += $(call test-name,{threads:},{cores:},{pods:},{pxns:},{graph:})\n".format(
            threads=threads, cores=cores, pods=1, pxns=1, graph=graph
        )

    def test_to_dir(self, test):
        inputs, cores, threads = test
        graph = inputs
        return "threads_{}__cores_{}__pods_{}__pxns_{}__graph_{}".format(
            threads, cores, 1, 1, graph
        )

    def format_input(self, inputs):
        graph = inputs
        return "graph_{}".format(graph)

TCTestbench("cello_tc").run()
