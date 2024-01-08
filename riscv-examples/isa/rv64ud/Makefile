# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington
DRV_DIR := $(shell git rev-parse --show-toplevel)
ISA_DIR := $(DRV_DIR)/riscv-examples/isa
RV64UD_DIR := $(ISA_DIR)/rv64ud
include $(RV64UD_DIR)/Makefrag

TESTS := $(rv64ud_sc_tests)
SKIP := 
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
