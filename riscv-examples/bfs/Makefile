# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR := $(shell git rev-parse --show-toplevel)

# pando hammer source files
RISCV_CXXSOURCE += bfs.cpp

# command processor source files
COMMAND_PROCESSOR_CXXSOURCE += bfs_cp.cpp

SIM_OPTIONS += --core-threads=16 --pod-cores=1

include $(DRV_DIR)/mk/riscv_common.mk
include $(DRV_DIR)/mk/command_processor.mk
