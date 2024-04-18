# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

THREADS := 1
CORES := 1
PODS := 1
DRV_DIR := $(shell git rev-parse --show-toplevel)

RISCV_CSOURCE := $(wildcard *.c)
RISCV_COMPILE_FLAGS += -DWAIT_N=1000000

SIM_OPTIONS += --pxn-pods=$(PODS) --core-threads=$(THREADS) --pod-cores=$(CORES)
include $(DRV_DIR)/mk/riscv_common.mk
