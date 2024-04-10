import testbench as tb
import itertools
import pandas as pd
class RuntimeProfilingTestbench(tb.Testbench):
    APPLICATIONS = ['bfs', 'gemm', 'gups', 'jaccard', 'pr', 'spmm', 'tc', 'vadd']
    TIMES = {
        'bfs'    : 'bfs',
        'gemm'   : 'gemm',
        'gups'   : 'gups',
        'jaccard': 'jaccard',
        'pr'     : 'pagerank',
        'spmm'   : 'row-wise product',
        'tc'     : 'triangle counting',
        'vadd'   : 'vadd'
    }
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
         return "Application,Runtime Cycles,Application Cycles"

    # override
    def coalesce_results(self):
        data = pd.DataFrame()        
        for test in self.tests():
            app, cores, threads = test
            tdir = self.test_to_dir(test)
            # find time range of import
            times = pd.read_csv("{}/tags.csv".format(tdir))
            start_time, = times[times['TagName']==(self.TIMES[app]+'_start')]['SimTime'].unique()
            stop_time,  = times[times['TagName']==(self.TIMES[app]+'_stop')] ['SimTime'].unique()
            # filter data
            tdata = pd.read_csv("{}/stats.csv".format(tdir))
            tdata = tdata[tdata['StatisticName']=='tag_cycles']

            start_data = tdata[tdata['SimTime']==start_time]
            stop_data  = tdata[tdata['SimTime']==stop_time]
                
            runtime_cycles_start = start_data['Bin1:1-1.u64'].sum()
            runtime_cycles_stop  = stop_data['Bin1:1-1.u64'].sum()
            runtime_cycles = runtime_cycles_stop - runtime_cycles_start            
            
            application_cycles_start = start_data['Bin0:0-0.u64'].sum()
            application_cycles_stop  = stop_data['Bin0:0-0.u64'].sum()
            application_cycles = application_cycles_stop - application_cycles_start

            data = data.append(
                pd.DataFrame({
                    'Application':[app],
                    'Runtime Cycles':[runtime_cycles],
                    'Application Cycles':[application_cycles]
                })
            )
        # write it out
        data.to_csv(self.result_local())

    def test_to_dir(self, test):
        app, cores, threads = test
        return "app_{app:}__threads_{threads:}__cores_{cores:}__pods_1__pxns_1".format(
            app=app, cores=cores, threads=threads
        )
RuntimeProfilingTestbench("runtime_profiling").run()
