DRV_DIR := $(shell git rev-parse --show-toplevel)
TARGET := exe.riscv

PLATFORM ?= ph
VERBOSE ?= 0

SIM_OPTIONS := --harts 2
SIM_OPTIONS += --verbose-core $(VERBOSE)

include $(DRV_DIR)/mk/riscv_common.mk
