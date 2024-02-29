import re
import itertools
import testbench as tb

class SPMMTestbench(tb.Testbench):
    CORES = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
    THREADS = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
    INPUTS = [('u10k4','g10k4')]

    def __init__(self, tbname):
        super().__init__(tbname)

    def tests(self):
        return itertools.product(self.INPUTS, self.CORES, self.THREADS)

    def test_to_mk(self, test):
        inputs, cores, threads = test
        m0, m1 = inputs
        return "TESTS += $(call test-name,{threads:},{cores:},{pods:},{pxns:},{m0:},{m1:})\n".format(
            threads=threads, cores=cores, pods=1, pxns=1, m0=m0, m1=m1
        )

    def result_header(self):
        return "Application,Input,Sim Options,PXN,Pods,Cores,Threads,Seconds\n"

    def test_to_dir(self, test):
        inputs, cores, threads = test
        m0, m1 = inputs
        return "threads_{}__cores_{}__pods_{}__pxns_{}__m0_{}__m1_{}".format(
            threads, cores, 1, 1, m0, m1
        )

    def parse_seconds(self, line):
        match = re.search(r'row-wise product: Elapsed time: ([0-9.]+) seconds', line)        
        if match:
            return float(match.group(1))

        match = re.search(r'product to csr: Elapsed time: ([0-9.]+) seconds', line)
        if match:
            return float(match.group(1))

        return 0.0

    def parse_stats(self, line, stats):
        if 'fadds' not in stats:
            stats['fadds'] = 0
        if 'fsubs' not in stats:
            stats['fsubs'] = 0
        if 'fmuls' not in stats:
            stats['fmuls'] = 0
        if 'fdivs' not in stats:
            stats['fdivs'] = 0
        if 'fmadds' not in stats:
            stats['fmadds'] = 0

        match = re.search(r'(row-wise product|product to csr): fadd: ([0-9]+)', line)
        if match:
            stats['fadds'] += int(match.group(2))

        match = re.search(r'(row-wise product|product to csr): fsub: ([0-9]+)', line)
        if match:
            stats['fsubs'] += int(match.group(2))

        match = re.search(r'(row-wise product|product to csr): fmul: ([0-9]+)', line)
        if match:
            stats['fmuls'] += int(match.group(2))

        match = re.search(r'(row-wise product|product to csr): fdiv: ([0-9]+)', line)
        if match:
            stats['fdivs'] += int(match.group(2))

        match = re.search(r'(row-wise product|product to csr): fmadd: ([0-9]+)', line)
        if match:
            stats['fmadds'] += int(match.group(2))

        return stats
    
    def result(self, test, sim_options, seconds, stats):
        inputs, cores, threads = test
        m0, m1 = inputs
        return "{Application:},{Input:},{SimOptions:},{PXN:},{Pods:},{Cores:},{Threads:},{Seconds:1.12f},{FADDS:},{FSUBS:},{FMULS:},{FDIVS:},{FMADDS:}\n".format(        
            Application="cello_spmm",
            Input="m0_{}__m1_{}".format(m0, m1),
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
    
SPMMTestbench("cello_spmm").run()
