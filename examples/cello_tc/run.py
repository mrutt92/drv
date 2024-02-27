import re
import itertools
import testbench as tb

class TCTestbench(tb.Testbench):
    CORES = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
    THREADS = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
    INPUTS = ['u12k16']

    def __init__(self, tbname):
        super().__init__(tbname)

    def tests(self):
        return itertools.product(self.INPUTS, self.CORES, self.THREADS)

    def test_to_mk(self, test):
        inputs, cores, threads = test
        graph = inputs
        return "TESTS += $(call test-name,{threads:},{cores:},{pods:},{pxns:},{graph:})\n".format(
            threads=threads, cores=cores, pods=1, pxns=1, graph=graph
        )

    def result_header(self):
        return "Application,Input,Sim Options,PXN,Pods,Cores,Threads,Seconds\n"

    def test_to_dir(self, test):
        inputs, cores, threads = test
        graph = inputs
        return "threads_{}__cores_{}__pods_{}__pxns_{}__graph_{}".format(
            threads, cores, 1, 1, graph
        )

    def parse_seconds(self, line):
        match = re.search(r'triangle counting: Elapsed time: ([0-9.]+) seconds', line)
        if match:
            return float(match.group(1))
        return 0.0

    def result(self, test, sim_options, seconds):
        inputs, cores, threads = test
        graph = inputs
        return "{Application:},{Input:},{SimOptions:},{PXN:},{Pods:},{Cores:},{Threads:},{Seconds:1.12f}\n".format(
            Application="cello_tc",
            Input="graph_{}".format(graph),
            SimOptions=sim_options,
            PXN=1,
            Pods=1,
            Cores=cores,
            Threads=threads,
            Seconds=seconds,
        )
TCTestbench("cello_tc").run()
