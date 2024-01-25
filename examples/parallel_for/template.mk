# SPDX-License-Identifier: MIT
# Copyright (c) 2024 University of Washington

# import parameters and APP_PATH
include parameters.mk
include app_path.mk

DRV_DIR := $(shell git rev-parse --show-toplevel)
include $(DRV_DIR)/mk/cello.mk

vpath %.c   $(APP_PATH)
vpath %.cpp $(APP_PATH)

APP_EXE ?= $(APP_PATH)/$(test-name)/$(APP_NAME).so

SIM_OPTIONS += --core-threads=$(threads) --pod-cores=$(cores)
SIM_OPTIONS += --pxn-pods=$(pods) --num-pxn=$(pxns)
SIM_OPTIONS += --drvx-stack-in-l1sp
SIM_ARGS += $(start) $(stop) $(step) $(grain)

include $(DRV_DIR)/mk/config.mk
include $(DRV_DIR)/mk/application_common.mk

CXXFLAGS += $(CELLO_CXXFLAGS)
CELLO_OBJECTS := $(CELLO_CXXSOURCES_DRVX:.cpp=.o)

$(APP_NAME).so: $(CELLO_OBJECTS)

.DEFAULT_GOAL := help


.PHONY: debug
debug:
	@echo "APP_PATH: $(APP_PATH)"
	@echo "APP_NAME: $(APP_NAME)"
