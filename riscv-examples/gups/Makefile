# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

THREADS := 8
CORES := 8
PODS := 1
DRV_DIR := $(shell git rev-parse --show-toplevel)
UPDATES := $(shell echo 2^16 | bc)
THREAD_UPDATES ?= $(shell echo $(UPDATES)/$(THREADS)/$(CORES)/$(PODS) | bc)
TABLE_SIZE ?= $(shell echo 128*1024*1024 | bc)
RISCV_CSOURCE := $(wildcard *.c)
RISCV_COMPILE_FLAGS += -DDRAM_BASE=$(shell echo 2^47 | bc) -DDRAM_SIZE=$(shell echo 2^30 | bc)
RISCV_COMPILE_FLAGS += -DTHREAD_UPDATES=$(THREAD_UPDATES) -DTABLE_SIZE=$(TABLE_SIZE)
SIM_OPTIONS += --pxn-pods=$(PODS) --core-threads=$(THREADS) --pod-cores=$(CORES)
include $(DRV_DIR)/mk/riscv_common.mk
