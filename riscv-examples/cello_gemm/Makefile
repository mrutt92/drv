# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR := $(shell git rev-parse --show-toplevel)
#memsys := Latency70ns-1Greq
CELLO_BACKEND := DRVR
include $(DRV_DIR)/mk/cello.mk
include $(DRV_DIR)/mk/eigen_config.mk

THREADS := 1
CORES := 16
M := 256
N := 256
K := 256

###############
# cello stuff #
###############

CELLO_DIR := $(DRV_DIR)/cello
RISCV_CXXFLAGS  += $(CELLO_CXXFLAGS_DRVR)
RISCV_CXXFLAGS  += -I$(DRV_DIR)/riscv-examples/cello_common
RISCV_CXXFLAGS  += -DGEMM_M=$(M) -DGEMM_N=$(N) -DGEMM_K=$(K)
RISCV_CXXSOURCE += $(CELLO_CXXSOURCES_DRVR)
RISCV_CXXSOURCE += cello_gemm.cpp


SIM_OPTIONS += --core-threads=$(THREADS) --pod-cores=$(CORES)
#SIM_OPTIONS += --debug-memory --verbose=2
SIM_ARGS += $(M) $(N) $(K)

vpath %.cpp $(DRV_DIR)/common/sparse_matrix_helpers
vpath %.c   $(DRV_DIR)/common/sparse_matrix_helpers
vpath %.cpp $(DRV_DIR)/riscv-examples/cello_common
vpath %.c   $(DRV_DIR)/riscv-examples/cello_common

COMMAND_PROCESSOR_CXXSOURCE += cello_gemm_cp.cpp
COMMAND_PROCESSOR_COMPILE_FLAGS += -I$(DRV_DIR)/riscv-examples/cello_common
COMMAND_PROCESSOR_COMPILE_FLAGS += -DGEMM_M=$(M) -DGEMM_N=$(N) -DGEMM_K=$(K)
COMMAND_PROCESSOR_COMPILE_FLAGS += $(EIGEN_CXXFLAGS)

include $(DRV_DIR)/mk/riscv_common.mk

