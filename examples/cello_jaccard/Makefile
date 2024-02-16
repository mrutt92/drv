DRV_DIR = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = pxns_$(1)__pods_$(2)__cores_$(3)__threads_$(4)__graph_$(5)

# call to get parameter from test name
get-pxns = $(lastword $(subst _, ,$(filter pxns_%,$(subst __, ,$(1)))))
get-pods = $(lastword $(subst _, ,$(filter pods_%,$(subst __, ,$(1)))))
get-cores = $(lastword $(subst _, ,$(filter cores_%,$(subst __, ,$(1)))))
get-threads = $(lastword $(subst _, ,$(filter threads_%,$(subst __, ,$(1)))))
get-graph = $(lastword $(subst _, ,$(filter graph_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo pxns = $(call get-pxns,$*) >> $@
	@echo pods = $(call get-pods,$*) >> $@
	@echo cores = $(call get-cores,$*) >> $@
	@echo threads = $(call get-threads,$*) >> $@
	@echo graph = $(call get-graph,$*) >> $@

include $(DRV_DIR)/mk/testbench_common.mk

# for regression
EXECUTION_DIRS := (addprefix $(APP_PATH)/,$(TESTS_DIRS))


