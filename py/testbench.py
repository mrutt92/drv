import subprocess
import datetime

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
        self.generate_tests()
        self.run_tests()
        self.coalesce_results()
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
        
