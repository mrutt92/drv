import testbench as tb
import itertools
import pandas as pd
class TaskQueueProfilingTestbench(tb.Testbench):
    APPLICATIONS = ['bfs', 'gemm', 'gups', 'jaccard', 'pr', 'spmm', 'tc', 'vadd']
    CORES   = [8]
    THREADS = [4]
    # APPLICATIONS = ['gemm']
    # CORES   = [1, 2]
    # THREADS = [1, 2]
    def __init__(self, tbname):
        super().__init__(tbname)

    def tests(self):
        return itertools.product(self.APPLICATIONS, self.CORES, self.THREADS)

    def test_to_mk(self, test):
        app, cores, threads = test
        return "TESTS += $(call test-name,{app:},{threads:},{cores:},1,1)\n".format(
            app=app, cores=cores, threads=threads
        )

    def result_header(self):
        return "Application,time,pxn,pod,cores,threads,size"

    # override
    def coalesce_results(self):
        data = pd.DataFrame()
        for test in self.tests():
            tdir = self.test_to_dir(test)
            tdata = pd.read_csv("{}/task_queue_profiler.csv".format(tdir))
            tdata['Application'] = test[0]
            data = data.append(tdata)
        # write it out
        data.to_csv(self.result_local())

    def test_to_dir(self, test):
        app, cores, threads = test
        return "app_{app:}__threads_{threads:}__cores_{cores:}__pods_1__pxns_1".format(
            app=app, cores=cores, threads=threads
        )
TaskQueueProfilingTestbench("task_queue_profiling").run()
