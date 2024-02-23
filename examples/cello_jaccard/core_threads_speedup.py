import testbench as tb
import itertools
import re

class JSTestbench(tb.Testbench):
    CORES   = [1, 2]
    THREADS = [1, 2]
    INPUTS  = ['u7k1']
    def __init__(self, tbname):
        super().__init__(tbname)

    def tests(self):
        return itertools.product(self.INPUTS, self.CORES, self.THREADS)

    def test_to_mk(self, test):
        inputs, cores, threads = test
        graph = inputs
        return "TESTS += $(call test-name,{pxns:},{pods:},{cores:},{threads:},{graph:})\n".format(
            threads=threads, cores=cores, pods=1, pxns=1, graph=graph
        )

    def result_header(self):
        return "Application,Input,Sim Options,PXN,Pods,Cores,Threads,Seconds\n"

    def test_to_dir(self, test):
        inputs, cores, threads = test
        graph = inputs
        return "pxns_{}__pods_{}__cores_{}__threads_{}__graph_{}".format(
            threads, cores, 1, 1, graph
        )

    def parse_seconds(self, line):
        match = re.search(r'jaccard: Elapsed time: ([0-9.]+) seconds', line)
        if match:
            return float(match.group(1))
        return 0.0

    def result(self, test, sim_options, seconds):
        inputs, cores, threads = test
        graph = inputs
        return "{Application:},{Input:},{SimOptions:},{PXN:},{Pods:},{Cores:},{Threads:},{Seconds:1.12f}\n".format(
            Application="cello_jaccard",
            Input="graph_{}".format(graph),
            SimOptions=sim_options,
            PXN=1,
            Pods=1,
            Cores=cores,
            Threads=threads,
            Seconds=seconds,
        )

JSTestbench("cello_jaccard").run()
