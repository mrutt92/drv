####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[threads],[cores],[pods],[pxns],[graph],[relabel])
TESTS += $(call test-name,1,1,1,1,u10k16,yes)
TESTS += $(call test-name,1,1,1,1,u10k16,no)
TESTS += $(call test-name,1,1,1,1,g10k16,yes)
TESTS += $(call test-name,1,1,1,1,g10k16,no)


