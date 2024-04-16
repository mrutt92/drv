# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR := $(shell git rev-parse --show-toplevel)
include $(DRV_DIR)/mk/cello.mk

THREADS := 2
CORES := 4
N := 9

###############
# cello stuff #
###############
CELLO_DIR := $(DRV_DIR)/cello
RISCV_CXXFLAGS += -DFIB_N=$(N)
RISCV_CXXFLAGS  += $(CELLO_CXXFLAGS_DRVR)
RISCV_CXXSOURCE += $(CELLO_CXXSOURCES_DRVR)
RISCV_CXXSOURCE += $(wildcard *.cpp)


SIM_OPTIONS += --core-threads=$(THREADS) --pod-cores=$(CORES)
include $(DRV_DIR)/mk/riscv_common.mk
