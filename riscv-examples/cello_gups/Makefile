# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR := $(shell git rev-parse --show-toplevel)
include $(DRV_DIR)/mk/cello.mk

THREADS := 1
CORES := 1
GUPS_UPDATES = $(shell echo 2^10 | bc)
GUPS_TABLE_SIZE = $(shell echo 2^26 | bc)

###############
# cello stuff #
###############
CELLO_DIR := $(DRV_DIR)/cello
RISCV_CXXFLAGS += -DGUPS_UPDATES=$(GUPS_UPDATES) -DGUPS_TABLE_SIZE=$(GUPS_TABLE_SIZE)
RISCV_CXXFLAGS  += $(CELLO_CXXFLAGS_DRVR)
RISCV_CXXSOURCE += $(CELLO_CXXSOURCES_DRVR)
RISCV_CXXSOURCE += $(wildcard *.cpp)


SIM_OPTIONS += --core-threads=$(THREADS) --pod-cores=$(CORES)
include $(DRV_DIR)/mk/riscv_common.mk
