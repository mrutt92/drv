import re
import itertools
import testbench as tb

class BFSTestbench(tb.CoreThreadSpeedupTestbench):
    INPUTS  = [('u16k16','0')]
    def __init__(self, tbname):
        super().__init__(tbname, 'cello_bfs')

    def tag_prefixes(self):
        return ["bfs"]
    
    def tests(self):
        return itertools.product(self.INPUTS, self.CORES, self.THREADS)

    def test_to_mk(self, test):
        inputs, cores, threads = test
        graph, root = inputs
        return "TESTS += $(call test-name,{threads:},{cores:},{pods:},{pxns:},{graph:},{root:})\n".format(
            threads=threads, cores=cores, pods=1, pxns=1, graph=graph, root=root
        )

    def test_to_dir(self, test):
        inputs, cores, threads = test
        graph, root = inputs
        return "threads_{}__cores_{}__pods_{}__pxns_{}__graph_{}__start_{}".format(
            threads, cores, 1, 1, graph, root
        )

    def format_input(self, inputs):
        graph, root = inputs
        return "graph_{}__root_{}".format(graph, root)
            
BFSTestbench("cello_bfs").run()        
