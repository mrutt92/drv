def output(threads, cores, table_size, updates):
    return "TESTS += $(call test-name,{},{},1,1,{},{})".format(threads, cores, table_size, updates)

for cores in [1, 2, 4, 8]:
    for threads in [1, 2, 3, 4]:
        print(output(threads, cores, (2**30)//8, 10**3))
