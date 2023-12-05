# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR := $(shell git rev-parse --show-toplevel)
include $(DRV_DIR)/mk/config.mk
APP_NAME := to_address
SIM_OPTIONS += --pod-cores=1 --core-threads=1
SIM_OPTIONS += --drvx-stack-in-l1sp
include $(DRV_DIR)/mk/application_common.mk
