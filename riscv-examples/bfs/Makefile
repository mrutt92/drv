# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR := $(shell git rev-parse --show-toplevel)

THREADS_PER_CORE := 1
CORES_PER_POD    := 1

# pando hammer source files
RISCV_CXXSOURCE += bfs.cpp

# command processor compile flags
COMMAND_PROCESSOR_COMPILE_FLAGS += -DCOMMAND_PROCESSOR -DTHREADS_PER_CORE=$(THREADS_PER_CORE)
# command processor source files
COMMAND_PROCESSOR_CXXSOURCE += bfs_cp.cpp

SIM_OPTIONS += --core-threads=$(THREADS_PER_CORE)
SIM_OPTIONS += --pod-cores=$(CORES_PER_POD)

include $(DRV_DIR)/mk/riscv_common.mk
include $(DRV_DIR)/mk/command_processor.mk
