# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

APP_NAME := pointer_v2
DRV_DIR := $(shell git rev-parse --show-toplevel)
SIM_OPTIONS += --num-pxn=1 --pxn-pods=1 --pod-cores=1 --core-threads=1
include $(DRV_DIR)/mk/config.mk
include $(DRV_DIR)/mk/application_common.mk
