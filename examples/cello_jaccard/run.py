import testbench as tb
import itertools
import re

class JSTestbench(tb.CoreThreadSpeedupTestbench):
    INPUTS  = ['u10k16']
    def __init__(self, tbname):
        super().__init__(tbname, "cello_jaccard")

    def tag_prefixes(self):
        return ["jaccard"]
    
    def tests(self):
        return itertools.product(self.INPUTS, self.CORES, self.THREADS)

    def test_to_mk(self, test):
        inputs, cores, threads = test
        graph = inputs
        return "TESTS += $(call test-name,{pxns:},{pods:},{cores:},{threads:},{graph:})\n".format(
            threads=threads, cores=cores, pods=1, pxns=1, graph=graph
        )

    def test_to_dir(self, test):
        inputs, cores, threads = test
        graph = inputs
        return "pxns_{pxns:}__pods_{pods:}__cores_{cores:}__threads_{threads:}__graph_{graph:}".format(
            threads=threads, cores=cores, pods=1, pxns=1, graph=graph
        )

    def format_input(self, inputs):
        graph = inputs
        return "graph_{}".format(graph)

JSTestbench("cello_jaccard").run()
