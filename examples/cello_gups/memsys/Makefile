DRV_DIR = $(shell git rev-parse --show-toplevel)

.PHONY: all
all: generate

# call to generate a test name
test-name = table-size_$(1)__updates_$(2)__memsys_$(3)__ports_$(4)__threads_$(5)__cores_$(6)__pods_$(7)__pxns_$(8)

# call to get parameter from test name
get-table-size = $(lastword $(subst _, ,$(filter table-size_%,$(subst __, ,$(1)))))
get-updates = $(lastword $(subst _, ,$(filter updates_%,$(subst __, ,$(1)))))
get-memsys = $(lastword $(subst _, ,$(filter memsys_%,$(subst __, ,$(1)))))
get-ports = $(lastword $(subst _, ,$(filter ports_%,$(subst __, ,$(1)))))
get-threads = $(lastword $(subst _, ,$(filter threads_%,$(subst __, ,$(1)))))
get-cores = $(lastword $(subst _, ,$(filter cores_%,$(subst __, ,$(1)))))
get-pods = $(lastword $(subst _, ,$(filter pods_%,$(subst __, ,$(1)))))
get-pxns = $(lastword $(subst _, ,$(filter pxns_%,$(subst __, ,$(1)))))

# defines tests
TESTS =
include tests.mk

TESTS_DIRS = $(TESTS)

$(addsuffix /parameters.mk,$(TESTS_DIRS)): %/parameters.mk:
	@echo Creating $@
	@mkdir -p $(dir $@)
	@touch $@
	@echo test-name  = $* >> $@
	@echo table-size = $(call get-table-size,$*) >> $@
	@echo updates = $(call get-updates,$*) >> $@
	@echo memsys = $(call get-memsys,$*) >> $@
	@echo ports = $(call get-ports,$*) >> $@
	@echo threads = $(call get-threads,$*) >> $@
	@echo cores = $(call get-cores,$*) >> $@
	@echo pods = $(call get-pods,$*) >> $@
	@echo pxns = $(call get-pxns,$*) >> $@

include $(DRV_DIR)/mk/testbench_common.mk

# for regression
EXECUTION_DIRS := (addprefix $(APP_PATH)/,$(TESTS_DIRS))


