####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[n],[threads],[cores],[pods],[pxns])
 TESTS += $(call test-name,2,1,1,1,1)
# TESTS += $(call test-name,16,1,1,1,1)
# TESTS += $(call test-name,16,2,1,1,1)
# TESTS += $(call test-name,16,4,1,1,1)
# TESTS += $(call test-name,16,8,1,1,1)
TESTS += $(call test-name,16,16,1,1,1)
TESTS += $(call test-name,16,2,1,1,1)
TESTS += $(call test-name,16,2,2,1,1)
# TESTS += $(call test-name,16,1,2,1,1)
# TESTS += $(call test-name,16,1,4,1,1)
# TESTS += $(call test-name,16,1,8,1,1)
