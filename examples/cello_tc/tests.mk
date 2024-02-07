####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[threads],[cores],[pods],[pxns],[graph])
TESTS += $(call test-name,1,1,1,1,u10k16)
TESTS += $(call test-name,16,8,1,1,u10k16)

