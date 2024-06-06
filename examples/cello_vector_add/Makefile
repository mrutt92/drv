DRV_DIR = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = threads_$(1)__cores_$(2)__pods_$(3)__pxns_$(4)__n_$(5)

# call to get parameter from test name
get-threads = $(lastword $(subst _, ,$(filter threads_%,$(subst __, ,$(1)))))
get-cores = $(lastword $(subst _, ,$(filter cores_%,$(subst __, ,$(1)))))
get-pods = $(lastword $(subst _, ,$(filter pods_%,$(subst __, ,$(1)))))
get-pxns = $(lastword $(subst _, ,$(filter pxns_%,$(subst __, ,$(1)))))
get-n = $(lastword $(subst _, ,$(filter n_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo threads = $(call get-threads,$*) >> $@
	@echo cores = $(call get-cores,$*) >> $@
	@echo pods = $(call get-pods,$*) >> $@
	@echo pxns = $(call get-pxns,$*) >> $@
	@echo n = $(call get-n,$*) >> $@

include $(DRV_DIR)/mk/testbench_common.mk

# for regression
EXECUTION_DIRS := (addprefix $(APP_PATH)/,$(TESTS_DIRS))


