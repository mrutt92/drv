# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR := $(shell git rev-parse --show-toplevel)
#memsys := Latency70ns-1Greq
CELLO_BACKEND := DRVR
include $(DRV_DIR)/mk/cello.mk

THREADS := 8
CORES := 1
GUPS_UPDATES = $(shell echo 2^16 | bc)
GUPS_TABLE_SIZE = $(shell echo 2^26 | bc)

###############
# cello stuff #
###############
GRAPH := $(DRV_DIR)/inputs/sparse-inputs/u7k1.mtx

CELLO_DIR := $(DRV_DIR)/cello
RISCV_CXXFLAGS  += $(CELLO_CXXFLAGS_DRVR)
RISCV_CXXSOURCE += $(CELLO_CXXSOURCES_DRVR)
RISCV_CXXSOURCE += cello_pr.cpp

SIM_OPTIONS += --core-threads=$(THREADS) --pod-cores=$(CORES)
SIM_ARGS += 
COMMAND_PROCESSOR_CXXSOURCE += cello_pr_cp.cpp

include $(DRV_DIR)/mk/riscv_common.mk

