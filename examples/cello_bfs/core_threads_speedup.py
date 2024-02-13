def output(threads, cores, graph, start):
    return "TESTS += $(call test-name,{},{},1,1,{},{})".format(threads, cores, graph, start)

for cores in [1, 2, 4, 8]:
    for threads in [1, 2, 3, 4]:
        print(output(threads, cores, "u12k16", 0))
