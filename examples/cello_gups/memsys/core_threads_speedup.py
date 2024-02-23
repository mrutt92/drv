import subprocess
import re
import itertools
import testbench as tb

class GUPSMemsysTestbench(tb.Testbench):
    CORES      = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
    THREADS    = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
    MEMSYS     = ['HBM2-1Gb-x64','HBM2-1Gb-x128','LPDDR4-1Gb-x16-2400']
    TABLE_SIZE = [(2**29)//8]
    UPDATES    = [10**6]

    def __init__(self, tbname):
        super().__init__(tbname)

    def tests(self):
        return itertools.product(self.TABLE_SIZE, self.UPDATES, self.MEMSYS, self.CORES, self.THREADS)

    def test_to_mk(self, test):
        table_size, updates, memsys, cores, threads = test
        return "TESTS += $(call test-name,{},{},{},{},{},{},{},{})\n".format(
            table_size, updates, memsys, 1, threads, cores, 1, 1
        )

    def result_header(self):
        return "Application,Memsys,Input,Sim Options,Core Clock Hz,PXN,Pods,Cores,Threads,Seconds\n"
      
    def test_to_dir(self, test):
        table_size, updates, memsys, cores, threads = test
        return "table-size_{}__updates_{}__memsys_{}__ports_{}__threads_{}__cores_{}__pods_1__pxns_1".format(
            table_size, updates, memsys, 1, threads, cores
        )

    def parse_seconds(self, line):
        match = re.search(r'gups: Elapsed time: ([0-9.]+) seconds', line)
        if match:
            return float(match.group(1))
        return 0.0

    def result(self, test, sim_options, seconds):
        table_size, updates, memsys, cores, threads = test
        core_clock_hz = re.search(r'--core-clock=([0-9]+[KMG]?Hz)', sim_options).group(1)
        return "{Application:},{Memsys:},{Input:},{SimOptions:},{CoreClockHz:},{PXN:},{Pods:},{Cores:},{Threads:},{Seconds:1.12f}\n".format(
            Application="cello_gups",
            Memsys=memsys,
            Input="table-size_{}_updates_{}".format(table_size, updates),
            SimOptions=sim_options,
            PXN=1,
            Pods=1,
            Cores=cores,
            CoreClockHz=core_clock_hz,
            Threads=threads,
            Seconds=seconds,
        )
    
GUPSMemsysTestbench("cello_gups-memsys").run()
