# TESTS += $(call test-name,[table-size],[updates],[memsys],[ports],[threads],[cores],[pods],[pxns])
def output(threads, cores, table_size, updates, memsys, ports):
    return "TESTS += $(call test-name,{},{},{},{},{},{},{},{})".format(
        table_size, updates, memsys, ports, threads, cores, 1, 1
    )

for memsys in ['HBM2-1Gb-x64','HBM2-1Gb-x128','LPDDR4-1Gb-x16-2400']:
    for cores in [1, 2, 4, 8]:
        for threads in [1, 2, 3, 4]:
            print(output(threads, cores, (2**30)//8, 10**3, memsys, 1))
