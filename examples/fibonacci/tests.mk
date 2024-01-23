####################
# CHANGE ME: TESTS #
####################
# TESTS += $(call test-name,[n],[threads],[cores],[pods],[pxns])
TESTS += $(call test-name,1,1,1,1,1)
TESTS += $(call test-name,2,1,1,1,1)

TESTS += $(call test-name,10,1,1,1,1)
TESTS += $(call test-name,10,2,1,1,1)
TESTS += $(call test-name,10,4,1,1,1)
TESTS += $(call test-name,10,8,1,1,1)
TESTS += $(call test-name,10,16,1,1,1)

TESTS += $(call test-name,12,1,1,1,1)
TESTS += $(call test-name,12,2,1,1,1)
TESTS += $(call test-name,12,4,1,1,1)
TESTS += $(call test-name,12,8,1,1,1)
TESTS += $(call test-name,12,16,1,1,1)

TESTS += $(call test-name,14,1,1,1,1)
TESTS += $(call test-name,14,2,1,1,1)
TESTS += $(call test-name,14,4,1,1,1)
TESTS += $(call test-name,14,8,1,1,1)
TESTS += $(call test-name,14,16,1,1,1)

