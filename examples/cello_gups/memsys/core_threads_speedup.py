# TESTS += $(call test-name,[table-size],[updates],[memsys],[ports],[threads],[cores],[pods],[pxns])
import subprocess
import re
import itertools
import numpy as np
from datetime import datetime, timezone

CORES      = [1, 2, 4, 8]
THREADS    = [1, 2, 3, 4]
MEMSYS     = ['HBM2-1Gb-x64','HBM2-1Gb-x128','LPDDR4-1Gb-x16-2400']
TABLE_SIZE = (2**30)//8
UPDATES    = 10**3

def test(threads, cores, table_size, updates, memsys, ports):
    return "TESTS += $(call test-name,{},{},{},{},{},{},{},{})\n".format(
        table_size, updates, memsys, ports, threads, cores, 1, 1
    )

def test_dir(threads, cores, table_size, updates, mesys, ports):
    return "table-size_{}__updates_{}__memsys_{}__ports_{}__threads_{}__cores_{}__pods_1__pxns_1".format(
        table_size, updates, memsys, ports, threads, cores, 1, 1
    )

def result_header():
    return "Application,Memsys,Input,Sim Options,Core Clock Hz,PXN,Pods,Cores,Threads,Seconds\n"

def result(threads, cores, table_size, updates, memsys, ports, seconds, sim_options, core_clock_hz):
    return "{Application:},{Memsys:},{Input:},{SimOptions:},{CoreClockHz:},{PXN:},{Pods:},{Cores:},{Threads:},{Seconds:1.12f}\n".format(
        Application="cello_gups",
        Memsys=memsys,
        Input="table-size_{}__updates_{}".format(table_size, updates),
        SimOptions=sim_options,
        CoreClockHz=core_clock_hz,
        PXN=1,
        Pods=1,
        Cores=cores,
        Threads=threads,
        Seconds=seconds,
    )

with open("tests.mk", "w") as f:
    for memsys, cores, threads in itertools.product(MEMSYS, CORES, THREADS):
        f.write(test(threads, cores, TABLE_SIZE, UPDATES, memsys, 1))

subprocess.run(['make', '-j', '8', 'run'])

result_local = "gups-memsys-results.csv"
with open(result_local, "w") as results:
    results.write(result_header())
    for memsys, cores, threads in itertools.product(MEMSYS, CORES, THREADS):
        tdir = test_dir(threads, cores, TABLE_SIZE, UPDATES, memsys, 1)
        with open(tdir + '/run.log', 'r') as f:
            for line in f:
                match = re.search(r'gups: Elapsed time: ([0-9.]+) seconds', line)
                if not match:
                    continue
                seconds = float(match.group(1))

        with open(tdir + '/sim_options.log','r') as f:
            sim_options = f.read().strip()

        core_clock_hz = re.search(r'--core-clock=([0-9]+[KMG]?Hz)', sim_options).group(1)
        results.write(result(threads, cores, TABLE_SIZE, UPDATES, memsys, 1, seconds, sim_options, core_clock_hz))

result_remote = "{host:}:/cse/web/homes/mrutt/results/cello-drv/gups-memsys-results-{time:}.csv".format(
    host="bicycle.cs.washington.edu",
    time=datetime.now(timezone.utc).strftime("%Y.%m.%d.%H.%M.%S")
)
subprocess.run(
    ['scp', result_local, result_remote]
)
