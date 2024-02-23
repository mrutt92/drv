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
INPUTS     = [('u7k1','0')]
APP = "cello_bfs"

def test(threads, cores, graph, root):
    return "TESTS += $(call test-name,{threads:},{cores:},{pods:},{pxns:},{graph:},{root:})\n".format(
        threads=threads, cores=cores, pods=1, pxns=1, graph=graph, root=root
    )

def test_dir(threads, cores, graph, root):
    return "threads_{}__cores_{}__pods_{}__pxns_{}__graph_{}__start_{}".format(
        threads, cores, 1, 1, graph, root
    )

def result_header():
    return "Application,Input,Sim Options,PXN,Pods,Cores,Threads,Seconds\n"

def result(threads, cores, graph, root, sim_options, seconds):
    return "{Application:},{Input:},{SimOptions:},{PXN:},{Pods:},{Cores:},{Threads:},{Seconds:1.12f}\n".format(
        Application=APP,
        Input="graph_{}__root_{}".format(graph, root),
        SimOptions=sim_options,
        PXN=1,
        Pods=1,
        Cores=cores,
        Threads=threads,
        Seconds=seconds,
    )

with open("tests.mk", "w") as f:
    for inputs, cores, threads in itertools.product(INPUTS, CORES, THREADS):
        (graph, root) = inputs
        f.write(test(threads, cores, graph, root))

subprocess.run(['make', '-j', '8', 'run'])

result_local = "{}-results.csv".format(APP)
with open(result_local, "w") as results:
    results.write(result_header())
    for inputs, cores, threads in itertools.product(INPUTS, CORES, THREADS):
        tdir = test_dir(threads, cores, inputs[0], inputs[1])
        with open(tdir + '/run.log', 'r') as f:
            for line in f:
                match = re.search(r'bfs: Elapsed time: ([0-9.]+) seconds', line)
                if match:
                    seconds = float(match.group(1))
        
        with open(tdir + '/sim_options.log','r') as f:
            sim_options = f.read().strip()

        results.write(result(threads, cores, inputs[0], inputs[1], sim_options, seconds))

host = "bicycle.cs.washington.edu"
result_remote = "{host:}:/cse/web/homes/mrutt/results/cello-drv/{app:}-results-{time:}.csv".format(
    app=APP,
    host=host,
    time=datetime.now(timezone.utc).strftime("%Y.%m.%d.%H.%M.%S")
)
result_remote_latest = "{host:}:/cse/web/homes/mrutt/results/cello-drv/{app:}-results-latest.csv".format(
    app=APP,
    host=host
)
subprocess.run(
    ['scp', result_local, result_remote]
)
subprocess.run(
    ['scp', result_local, result_remote_latest]
)
