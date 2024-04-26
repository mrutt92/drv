# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR := $(shell git rev-parse --show-toplevel)
#memsys := Latency70ns-1Greq
CELLO_BACKEND := DRVR
include $(DRV_DIR)/mk/cello.mk

THREADS := 8
CORES := 4
GRAPH := u12k16

###############
# cello stuff #
###############

CELLO_DIR := $(DRV_DIR)/cello
RISCV_CXXFLAGS  += $(CELLO_CXXFLAGS_DRVR)
RISCV_CXXFLAGS  += -I$(DRV_DIR)/riscv-examples/cello_common
RISCV_CXXSOURCE += $(CELLO_CXXSOURCES_DRVR)
RISCV_CXXSOURCE += cello_pr.cpp

SIM_OPTIONS += --core-threads=$(THREADS) --pod-cores=$(CORES)
SIM_ARGS += $(DRV_DIR)/inputs/sparse-inputs/$(GRAPH).mtx

vpath %.cpp $(DRV_DIR)/common/sparse_matrix_helpers
vpath %.c   $(DRV_DIR)/common/sparse_matrix_helpers
vpath %.cpp $(DRV_DIR)/riscv-examples/cello_common
vpath %.c   $(DRV_DIR)/riscv-examples/cello_common

COMMAND_PROCESSOR_CXXSOURCE += read_graph.cpp
COMMAND_PROCESSOR_CXXSOURCE += transpose_graph.cpp
COMMAND_PROCESSOR_CSOURCE += mmio.c
COMMAND_PROCESSOR_COMPILE_FLAGS += -I$(DRV_DIR)/common/sparse_matrix_helpers
COMMAND_PROCESSOR_COMPILE_FLAGS += -I$(DRV_DIR)/riscv-examples/cello_common

COMMAND_PROCESSOR_CXXSOURCE += cello_pr_cp.cpp

include $(DRV_DIR)/mk/riscv_common.mk

