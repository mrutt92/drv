# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington
DRV_DIR := $(shell git rev-parse --show-toplevel)
ISA_DIR := $(DRV_DIR)/riscv-examples/isa
RV64UA_DIR := $(ISA_DIR)/rv64ua
include $(RV64UA_DIR)/Makefrag

TESTS := $(rv64ua_sc_tests)
SKIP := lrsc amomax_d amomaxu_d amomin_d amominu_d amoor_d amoxor_d amoand_d
SKIP +=      amomax_w amomaxu_w amomin_w amominu_w amoor_w amoxor_w amoand_w
TESTS := $(filter-out $(SKIP),$(TESTS))

$(TESTS): %: %.S template.mk
	@mkdir -p $@
	@cp $*.S $@
	@cp template.mk $@/Makefile
	$(eval TEST_NAME="$(shell echo $* | tr [a-z] [A-Z])")
	@echo "TEST_NAME=$(TEST_NAME)" > $@/test_name.mk
	@$(MAKE) -C $@ run | tee run.log | grep "$(TEST_NAME) TEST"

.PHONY: tests purge

tests: $(TESTS)

purge:
	@rm -rf $(TESTS)
