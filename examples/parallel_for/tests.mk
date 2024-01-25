####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[threads],[cores],[pods],[pxns],[start],[stop],[step],[grain])
TESTS += $(call test-name,1,1,1,1,1,10,1,1)
TESTS += $(call test-name,1,1,1,1,0,16,1,1)
TESTS += $(call test-name,2,1,1,1,0,16,1,1)
TESTS += $(call test-name,2,1,1,1,0,16,3,1)
TESTS += $(call test-name,2,1,1,1,0,16,3,0)
TESTS += $(call test-name,2,1,1,1,0,1024,1,0)




