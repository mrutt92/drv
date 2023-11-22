# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR := $(shell git rev-parse --show-toplevel)
include $(DRV_DIR)/mk/config.mk
SIM_OPTIONS += --dram-backend=ramulator
SIM_OPTIONS += --dram-backend-config=$(RAMULATOR_DIR)/configs/HBM-config.cfg
APP_NAME := multicoreamoadd
APP_PATH := $(DRV_DIR)/examples/multicoreamoadd_ramulator
SIM_OPTIONS += --dram-backend=ramulator
include $(DRV_DIR)/mk/application_common.mk
