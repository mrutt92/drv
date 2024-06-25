# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR := $(shell git rev-parse --show-toplevel)
#memsys := Latency70ns-1Greq

THREADS := 2
CORES := 16
VSIZE := $(shell echo 8*1024*1024 | bc)

###############
# cello stuff #
###############

RISCV_CXXFLAGS  += -DDRAM_BASE=$(shell echo 2^47 | bc)
RISCV_CXXFLAGS  += -DVSIZE=$(VSIZE)
RISCV_CXXSOURCE += vread.cpp
RISCV_CXXSOURCE += vsize.cpp

SIM_OPTIONS += --core-threads=$(THREADS) --pod-cores=$(CORES)


include $(DRV_DIR)/mk/riscv_common.mk
include $(DRV_DIR)/mk/dramsim3_config.mk

memsys := HBM2-1Gb-x64
include $(DRV_DIR)/mk/$(memsys).mk


#SIM_OPTIONS += --debug-memory --verbose=2

