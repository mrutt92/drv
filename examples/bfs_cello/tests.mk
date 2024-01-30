####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[threads],[cores],[pods],[pxns],[graph],[start])
TESTS += $(call test-name,1,1,1,1,u10k16,0)
# TESTS += $(call test-name,2,1,1,1,u10k16,0)
# TESTS += $(call test-name,4,1,1,1,u10k16,0)
# TESTS += $(call test-name,8,1,1,1,u10k16,0)
TESTS += $(call test-name,16,1,1,1,u10k16,0)
TESTS += $(call test-name,16,8,1,1,u10k16,0)
TESTS += $(call test-name,16,8,1,1,u12k16,0)
TESTS += $(call test-name,16,8,1,1,u16k16,0)
TESTS += $(call test-name,16,16,1,1,u16k16,0)
TESTS += $(call test-name,16,16,1,1,g10k16,0)
TESTS += $(call test-name,16,16,1,1,g16k16,0)



