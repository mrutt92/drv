import subprocess
import datetime
import argparse
import pandas
import re

class Testbench(object):
    HOST = "bicycle.cs.washington.edu"
    def __init__(self, tbname):
        self.tbname_ = tbname
    
    @property
    def tbname(self):
        return self.tbname_
    
    def result_local(self):
        return '{}-results.csv'.format(self.tbname)

    def result_remote(self):
        return '{host:}:/cse/web/homes/mrutt/results/cello-drv/{tb_name:}-results-{time:}.csv'.format(
            host=self.HOST,
            tb_name=self.tbname,
            time=datetime.datetime.now().strftime("%Y.%m.%d.%H.%M.%S")
        )

    def result_remote_latest(self):
        return '{host:}:/cse/web/homes/mrutt/results/cello-drv/{tb_name:}-results-latest.csv'.format(
            host=self.HOST,
            tb_name=self.tbname
        )

    def parse_args(self):
        """
        Parse the arguments
        """
        parser = argparse.ArgumentParser(description='Run a testbench')
        parser.add_argument('--do', type=str, default="all",
                            help='Do one step of running the testbench [all, generate, run, coalesce_results, upload]')
        self.args = parser.parse_args()

    @property
    def do(self):
        return self.args.do

    @property
    def do_generate(self):
        return self.do == "all" or self.do == "generate"

    @property
    def do_run(self):
        return self.do == "all" or self.do == "run"

    @property
    def do_coalesce_results(self):
        return self.do == "all" or self.do == "coalesce_results"

    @property
    def do_upload(self):
        return self.do == "all" or self.do == "upload"    
        
    def generate_tests(self):
        """
        Generate the tests
        """
        with open("tests.mk", "w") as test_mk:
            for test in self.tests():
                test_mk.write(self.test_to_mk(test))
    
            
    def run_tests(self):
        """
        Run the tests
        """
        subprocess.run(['make', '-j', '8', 'run'])

    def coalesce_results(self):
        """
        Coalesce the results into a single file
        """
        with open(self.result_local(), "w") as results:
            results.write(self.result_header())
            for test in self.tests():
                tdir = self.test_to_dir(test)
                with open(tdir + '/run.log', 'r') as f:
                    seconds = 0.0
                    stats = {}
                    for line in f:
                        seconds += self.parse_seconds(line)
                        stats = self.parse_stats(line, stats)

                with open(tdir + '/sim_options.log','r') as f:
                    sim_options = f.read().strip()

                with open(tdir + '/tags.csv','r') as f:
                    try:
                        tags = pandas.read_csv(f)
                    except Exception as e:
                        print("Error reading {}/tags.csv: {}".format(tdir, e))
                        exit(1)

                with open(tdir + '/stats.csv', 'r') as f:
                    try:
                        data = pandas.read_csv(f)
                    except Exception as e:
                        print("Error reading {}/stats.csv: {}".format(tdir, e))
                        exit(1)

                data = pandas.merge(data, tags, on='SimTime')
                for op in ('load', 'store', 'atomic'):
                    stats[op + 's'] = 0
                
                for tag in self.tag_prefixes():
                    starts = data[data['TagName'] == tag + '_start']
                    ends = data[data['TagName'] == tag + '_stop']
                    for op in ('load', 'store', 'atomic'):
                        ops_start = starts[starts['StatisticName'].str.contains(op + '_')]
                        ops_end = ends[ends['StatisticName'].str.contains(op + '_')]
                        stats[op + 's'] += int(ops_end['Sum.u64'].sum() - ops_start['Sum.u64'].sum())

                results.write(self.result(test, sim_options, seconds, stats))
                
    def upload_results(self):
        """
        Upload the results to the remote server
        """
        subprocess.run(
            ['scp', self.result_local(), self.result_remote()]
        )
        subprocess.run(
            ['scp', self.result_local(), self.result_remote_latest()]
        )
        
    def run(self):
        """
        Run the testbench
        """
        self.parse_args()
        if self.do_generate:
            print("{}: Generating tests".format(self.tbname))
            self.generate_tests()

        if self.do_run:
            print("{}: Running tests".format(self.tbname))
            self.run_tests()

        if self.do_coalesce_results:
            print("{}: Coalescing results".format(self.tbname))
            self.coalesce_results()

        if self.do_upload:
            print("{}: Uploading results".format(self.tbname))
            self.upload_results()

    def tests(self):
        """
        Return a iterable of test parameters
        """
        raise NotImplementedError

    def test_to_mk(self, test):
        """
        Convert a test to a makefile rule
        """
        raise NotImplementedError

    def tag_prefixes(self):
        """
        Return the prefix for the tags
        """
        raise NotImplementedError
    
    def test_to_dir(self, test):
        """
        Convert a test to a directory
        """
        raise NotImplementedError

    def parse_seconds(self, line):
        """
        Parse the seconds from a line
        """
        raise NotImplementedError

    def parse_stats(self, line, stats):
        """
        Parse the flops from a line
        """
        return stats

    def result_header(self):
        """
        Return the header for the results file
        """
        raise NotImplementedError

    def result(self, test, sim_options, seconds, stats):
        """
        Return the result for a single test
        """
        raise NotImplementedError
        

class CoreThreadSpeedupTestbench(Testbench):
    CORES = range(1,33)
    THREADS = range(1,33)
    INPUTS = [""]
    def __init__(self, tbname, application):
        super().__init__(tbname)
        self.application = application
    
    def result_header(self):
        return "Application,Input,Sim Options,PXN,Pods,Cores,Threads,Seconds,FADDS,FSUBS,FMULS,FDIVS,FMADDS,LOADS,STORES,ATOMICS\n"

    def result(self, test, sim_options, seconds, stats):
        inputs, cores, threads = test
        return "{Application:},{Input:},{SimOptions:},{PXN:},{Pods:},{Cores:},{Threads:},{Seconds:1.12f},{FADDS:},{FSUBS:},{FMULS:},{FDIVS:},{FMADDS:},{LOADS:},{STORES:},{ATOMICS:}\n".format(
            Application=self.application,
            Input=self.format_input(inputs),
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
            LOADS=stats['loads'],
            STORES=stats['stores'],
            ATOMICS=stats['atomics']
        )

    def format_input(self, input):
        raise NotImplementedError

    def parse_stats(self, line, stats):
        for op in ['fadd', 'fsub', 'fmul', 'fdiv', 'fmadd']:
            if op+'s' not in stats:
                stats[op + 's'] = 0
            
            for tag in self.tag_prefixes():
                match = re.search(r'{}: {}: ([0-9]+)'.format(tag,op), line)
                if match:
                    stats[op + 's'] += int(match.group(1))

        return stats

    def parse_seconds(self, line):
        for tag in self.tag_prefixes():
            match = re.search(r'{}: Elapsed time: ([0-9.]+) seconds'.format(tag), line)
            if match:
                return float(match.group(1))
        
        return 0.0
