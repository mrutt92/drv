# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR := $(shell git rev-parse --show-toplevel)
#memsys := Latency70ns-1Greq
CELLO_BACKEND := DRVR
include $(DRV_DIR)/mk/cello.mk
include $(DRV_DIR)/mk/eigen_config.mk

THREADS := 1
CORES := 1
N := 32

###############
# cello stuff #
###############

CELLO_DIR := $(DRV_DIR)/cello
RISCV_CXXFLAGS  += $(CELLO_CXXFLAGS_DRVR)
RISCV_CXXFLAGS  += -I$(DRV_DIR)/riscv-examples/cello_common
RISCV_CXXSOURCE += $(CELLO_CXXSOURCES_DRVR)
RISCV_CXXSOURCE += cello_sort.cpp

SIM_OPTIONS += --core-threads=$(THREADS) --pod-cores=$(CORES)
SIM_ARGS += $(N)

vpath %.cpp $(DRV_DIR)/riscv-examples/cello_common
vpath %.c   $(DRV_DIR)/riscv-examples/cello_common


COMMAND_PROCESSOR_CXXSOURCE += cello_sort_cp.cpp

COMMAND_PROCESSOR_COMPILE_FLAGS += -I$(DRV_DIR)/riscv-examples/cello_common
COMMAND_PROCESSOR_COMPILE_FLAGS += $(EIGEN_CXXFLAGS)

include $(DRV_DIR)/mk/riscv_common.mk

