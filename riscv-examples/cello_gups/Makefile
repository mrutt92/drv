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
CELLO_DIR := $(DRV_DIR)/cello
RISCV_CXXFLAGS += -DGUPS_UPDATES=$(GUPS_UPDATES) -DGUPS_TABLE_SIZE=$(GUPS_TABLE_SIZE)
RISCV_CXXFLAGS  += $(CELLO_CXXFLAGS_DRVR)
RISCV_CXXSOURCE += $(CELLO_CXXSOURCES_DRVR)
RISCV_CXXSOURCE += cello_gups.cpp cello_gups_main.cpp

SIM_OPTIONS += --core-threads=$(THREADS) --pod-cores=$(CORES)

COMMAND_PROCESSOR_CXXSOURCE += cello_gups_commandprocessor.cpp

include $(DRV_DIR)/mk/riscv_common.mk

