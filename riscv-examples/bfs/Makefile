# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR := $(shell git rev-parse --show-toplevel)

# pando hammer source files
RISCV_CXXSOURCE += bfs.cpp

# command processor compile flags
COMMAND_PROCESSOR_COMPILE_FLAGS += -DCOMMAND_PROCESSOR
# command processor source files
COMMAND_PROCESSOR_CXXSOURCE += bfs_cp.cpp

SIM_OPTIONS += --core-threads=1 --pod-cores=1
#SIM_OPTIONS += --verbose=1 --debug-memory
#SIM_OPTIONS += --cp-verbose=10 --cp-verbose-requests

include $(DRV_DIR)/mk/riscv_common.mk
include $(DRV_DIR)/mk/command_processor.mk
