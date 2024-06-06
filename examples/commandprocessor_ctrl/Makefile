# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR := $(shell git rev-parse --show-toplevel)
include $(DRV_DIR)/mk/config.mk
APP_NAME := commandprocessor_ctrl
SIM_OPTIONS += --with-command-processor=$(APP_PATH)/$(APP_NAME).so
SIM_OPTIONS += --pod-cores=8
SIM_OPTIONS += --core-threads=16
SIM_OPTIONS += --debug-mmio
include $(DRV_DIR)/mk/application_common.mk
