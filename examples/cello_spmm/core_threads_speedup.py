# TESTS += $(call test-name,[table-size],[updates],[memsys],[ports],[threads],[cores],[pods],[pxns])
import subprocess
import re
import itertools
import numpy as np
from datetime import datetime, timezone

#CORES      = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
#THREADS    = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
CORES      = [1, 2, 3, 4]
THREADS    = [1, 2, 3, 4]
INPUTS     = [('u7k1','g7k1'),('g7k1','u7k1')]

def test(threads, cores, m0, m1):
    return "TESTS += $(call test-name,{threads:},{cores:},{pods:},{pxns:},{m0:},{m1:})\n".format(
        threads=threads, cores=cores, pods=1, pxns=1, m0=m0, m1=m1
    )

def test_dir(threads, cores, m0, m1):
    return "threads_{}__cores_{}__pods_{}__pxns_{}__m0_{}__m1_{}".format(
        threads, cores, 1, 1, m0, m1
    )

def result_header():
    return "Application,Input,Sim Options,PXN,Pods,Cores,Threads,Seconds\n"

def result(threads, cores, m0, m1, sim_options, seconds):
    return "{Application:},{Input:},{SimOptions:},{PXN:},{Pods:},{Cores:},{Threads:},{Seconds:1.12f}\n".format(
        Application="cello_spmm",
        Input="m0_{}__m1_{}".format(m0, m1),
        SimOptions=sim_options,
        PXN=1,
        Pods=1,
        Cores=cores,
        Threads=threads,
        Seconds=seconds,
        m0=m0,
        m1=m1,
    )

with open("tests.mk", "w") as f:
    for inputs, cores, threads in itertools.product(INPUTS, CORES, THREADS):
        (m0, m1) = inputs
        f.write(test(threads, cores, m0, m1))

subprocess.run(['make', '-j', '8', 'run'])

result_local = "cello_spmm-results.csv"
with open(result_local, "w") as results:
    results.write(result_header())
    for inputs, cores, threads in itertools.product(INPUTS, CORES, THREADS):
        tdir = test_dir(threads, cores, inputs[0], inputs[1])
        with open(tdir + '/run.log', 'r') as f:
            row_wise_seconds = 0.0
            product_to_csr_seconds = 0.0
            for line in f:
                match = re.search(r'row-wise product: Elapsed time: ([0-9.]+) seconds', line)
                if match:
                    row_wise_seconds = float(match.group(1))
                    continue

                match = re.search(r'product to csr: Elapsed time: ([0-9.]+) seconds', line)
                if match:
                    product_to_csr_seconds = float(match.group(1))

            seconds = row_wise_seconds + product_to_csr_seconds

        with open(tdir + '/sim_options.log','r') as f:
            sim_options = f.read().strip()

        results.write(result(threads, cores, inputs[0], inputs[1], sim_options, seconds))

host = "bicycle.cs.washington.edu"
result_remote = "{host:}:/cse/web/homes/mrutt/results/cello-drv/cello_spmm-results-{time:}.csv".format(
    host=host,
    time=datetime.now(timezone.utc).strftime("%Y.%m.%d.%H.%M.%S")
)
result_remote_latest = "{host:}:/cse/web/homes/mrutt/results/cello-drv/cello_spmm-results-latest.csv".format(
    host=host
)
subprocess.run(
    ['scp', result_local, result_remote]
)
subprocess.run(
    ['scp', result_local, result_remote_latest]
)
