import testbench as tb
import itertools
import re

class JSTestbench(tb.Testbench):
    CORES = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
    THREADS = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]    
    INPUTS  = ['u10k16']
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
        return "pxns_{pxns:}__pods_{pods:}__cores_{cores:}__threads_{threads:}__graph_{graph:}".format(
            threads=threads, cores=cores, pods=1, pxns=1, graph=graph
        )

    def parse_seconds(self, line):
        match = re.search(r'jaccard: Elapsed time: ([0-9.]+) seconds', line)
        if match:
            return float(match.group(1))
        return 0.0

    def parse_stats(self, line, stats):
        match = re.search(r'jaccard: fadd: ([0-9]+)', line)
        if match:
            stats['fadd'] = int(match.group(1))
        match = re.search(r'jaccard: fsub: ([0-9]+)', line)
        if match:
            stats['fsub'] = int(match.group(1))
        match = re.search(r'jaccard: fmul: ([0-9]+)', line)
        if match:
            stats['fmul'] = int(match.group(1))
        match = re.search(r'jaccard: fdiv: ([0-9]+)', line)
        if match:
            stats['fdiv'] = int(match.group(1))
        match = re.search(r'jaccard: fmadd: ([0-9]+)', line)
        if match:
            stats['fmad'] = int(match.group(1))
        return stats

    def result(self, test, sim_options, seconds, stats):
        inputs, cores, threads = test
        graph = inputs
        return "{Application:},{Input:},{SimOptions:},{PXN:},{Pods:},{Cores:},{Threads:},{Seconds:1.12f},{FADDS:},{FSUBS:},{FMULS:},{FDIVS:},{FMADDS:}\n".format(        
            Application="cello_jaccard",
            Input="graph_{}".format(graph),
            SimOptions=sim_options,
            PXN=1,
            Pods=1,
            Cores=cores,
            Threads=threads,
            Seconds=seconds,
            FADDS=stats['fadd'],
            FSUBS=stats['fsub'],
            FMULS=stats['fmul'],
            FDIVS=stats['fdiv'],
            FMADDS=stats['fmad']
        )

JSTestbench("cello_jaccard").run()
