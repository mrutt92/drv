# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington
DRV_DIR := $(shell git rev-parse --show-toplevel)
ISA_DIR := $(DRV_DIR)/riscv-examples/isa
RV64UI_DIR := $(ISA_DIR)/rv64ui

include test_name.mk

# set source files
RISCV_ASMSOURCE := $(wildcard *.S)

# no crt
RISCV_PLATFORM_CRT := no

SIM_OPTIONS += --num-pxn=1 --pod-cores=1 --core-threads=1 --drvr-isa-test
SIM_OPTIONS += --test-name="$(shell echo $(TEST_NAME) | tr [a-z] [A-Z])"

#SIM_OPTIONS += --verbose=100 --debug-clock

RISCV_COMPILE_FLAGS += -I$(ISA_DIR)/macros/scalar
RISCV_COMPILE_FLAGS += -I$(ISA_DIR)
RISCV_COMPILE_FLAGS += -DDRV_ISA_TEST

include $(DRV_DIR)/mk/riscv_common.mk


