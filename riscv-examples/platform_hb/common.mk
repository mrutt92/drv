DRV_DIR  ?= $(shell git rev-parse --show-toplevel)
SCRIPT   := $(DRV_DIR)/tests/riscvhb.py
CFLAGS   += -nostdlib
CXXFLAGS += -nostdlib
