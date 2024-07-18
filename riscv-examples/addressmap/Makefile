# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR := $(shell git rev-parse --show-toplevel)
RISCV_CSOURCE := $(wildcard *.c)
THREADS := 1
CORES := 8
ALL_THREADS := $(shell echo $(THREADS)*$(CORES) | bc)
SIM_OPTIONS += --core-threads=$(THREADS) --pod-cores=$(CORES)
RISCV_COMPILE_FLAGS += -DTHREADS=$(ALL_THREADS)
RISCV_COMPILE_FLAGS += -DBARRIER

include $(DRV_DIR)/mk/riscv_common.mk

LDSCRIPT = script.ld
$(RISCV_TARGET): $(LDSCRIPT)

$(LDSCRIPT): $(DRV_DIR)/py/addressmap.py
	python3 $^ > $@

