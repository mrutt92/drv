# SPDX-License-Identifier: MIT
# Copyright (c) 2024 University of Washington

# import parameters and APP_PATH
include parameters.mk
include app_path.mk

DRV_DIR ?= $(shell git rev-parse --show-toplevel)

vpath %.c   $(APP_PATH)
vpath %.cpp $(APP_PATH)

include $(APP_PATH)/template-$(app).mk

CXXFLAGS += -DCELLO_ENABLE_TASK_QUEUE_PROFILER
