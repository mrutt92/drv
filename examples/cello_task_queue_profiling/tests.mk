####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[app],[threads],[cores],[pods],[pxns])
TESTS += $(call test-name,bfs,1,1,1,1)
TESTS += $(call test-name,gemm,1,1,1,1)
TESTS += $(call test-name,gups,1,1,1,1)
TESTS += $(call test-name,jaccard,1,1,1,1)
TESTS += $(call test-name,pr,1,1,1,1)
TESTS += $(call test-name,spmm,1,1,1,1)
TESTS += $(call test-name,tc,1,1,1,1)
TESTS += $(call test-name,vadd,1,1,1,1)


