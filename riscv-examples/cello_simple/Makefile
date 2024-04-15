# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR := $(shell git rev-parse --show-toplevel)

THREADS := 1
CORES := 2

###############
# cello stuff #
###############
CELLO_DIR := $(DRV_DIR)/cello
RISCV_CXXFLAGS += -DRISCV
RISCV_CXXFLAGS += -DCORE_THREADS=$(THREADS)
RISCV_CXXFLAGS += -std=c++17
RISCV_CXXFLAGS += -I$(CELLO_DIR)
vpath %.cpp $(CELLO_DIR)
vpath %.c   $(CELLO_DIR)
RISCV_CXXSOURCE += cello_core_drvr.cpp
RISCV_CXXSOURCE += $(wildcard *.cpp)


SIM_OPTIONS += --core-threads=$(THREADS) --pod-cores=$(CORES)
include $(DRV_DIR)/mk/riscv_common.mk
