# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington
DRV_DIR := $(shell git rev-parse --show-toplevel)
ISA_DIR := $(DRV_DIR)/riscv-examples/isa
RV64UI_DIR := $(ISA_DIR)/rv64ui
include $(RV64UI_DIR)/Makefrag

TESTS := $(rv64ui_sc_tests)

$(TESTS): %: %.S template.mk
	@mkdir -p $@
	cp $*.S $@
	cp template.mk $@/Makefile
	echo "TEST_NAME=$*" > $@/test_name.mk
	$(MAKE) -C $@ run

purge:
	rm -rf $(TESTS)
