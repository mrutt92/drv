import re
import itertools
import testbench as tb

class BFSTestbench(tb.Testbench):
    CORES = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
    THREADS = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
    INPUTS  = [('u16k16','0')]
    def __init__(self, tbname):
        super().__init__(tbname)

    def tests(self):
        return itertools.product(self.INPUTS, self.CORES, self.THREADS)

    def test_to_mk(self, test):
        inputs, cores, threads = test
        graph, root = inputs
        return "TESTS += $(call test-name,{threads:},{cores:},{pods:},{pxns:},{graph:},{root:})\n".format(
            threads=threads, cores=cores, pods=1, pxns=1, graph=graph, root=root
        )

    def result_header(self):
        return "Application,Input,Sim Options,PXN,Pods,Cores,Threads,Seconds,FADDS,FSUBS,FMULS,FDIVS,FMADDS\n"

    def test_to_dir(self, test):
        inputs, cores, threads = test
        graph, root = inputs
        return "threads_{}__cores_{}__pods_{}__pxns_{}__graph_{}__start_{}".format(
            threads, cores, 1, 1, graph, root
        )

    def parse_seconds(self, line):
        match = re.search(r'bfs: Elapsed time: ([0-9.]+) seconds', line)
        if match:
            return float(match.group(1))
        return 0.0

    def parse_stats(self, line, stats):
        match = re.search(r'bfs: fadd: ([0-9]+)', line)
        if match:
            stats['fadds'] = int(match.group(1))
        match = re.search(r'bfs: fsub: ([0-9]+)', line)
        if match:
            stats['fsubs'] = int(match.group(1))
        match = re.search(r'bfs: fmul: ([0-9]+)', line)
        if match:
            stats['fmuls'] = int(match.group(1))
        match = re.search(r'bfs: fdiv: ([0-9]+)', line)
        if match:
            stats['fdivs'] = int(match.group(1))
        match = re.search(r'bfs: fmadd: ([0-9]+)', line)
        if match:
            stats['fmadds'] = int(match.group(1))
        return stats

    def result(self, test, sim_options, seconds, stats):
        inputs, cores, threads = test
        graph, root = inputs
        return "{Application:},{Input:},{SimOptions:},{PXN:},{Pods:},{Cores:},{Threads:},{Seconds:1.12f},{FADDS:},{FSUBS:},{FMULS:},{FDIVS:},{FMADDS:}\n".format(
            Application="cello_bfs",
            Input="graph_{}__root_{}".format(graph, root),
            SimOptions=sim_options,
            PXN=1,
            Pods=1,
            Cores=cores,
            Threads=threads,
            Seconds=seconds,
            FADDS=stats['fadds'],
            FSUBS=stats['fsubs'],
            FMULS=stats['fmuls'],
            FDIVS=stats['fdivs'],
            FMADDS=stats['fmadds'],
        )
        
BFSTestbench("cello_bfs").run()        
