# SPDX-License-Identifier: MIT
# Copyright (c) 2024 University of Washington

# import parameters and APP_PATH
include parameters.mk
include app_path.mk

DRV_DIR := $(shell git rev-parse --show-toplevel)
include $(DRV_DIR)/mk/cello.mk

vpath %.c   $(APP_PATH)
vpath %.cpp $(APP_PATH)
vpath %.c   $(DRV_DIR)/common/util
vpath %.cpp $(DRV_DIR)/common/util

APP_EXE ?= $(APP_PATH)/$(test-name)/$(APP_NAME).so

SIM_OPTIONS += --core-threads=$(threads) --pod-cores=$(cores)
SIM_OPTIONS += --pxn-pods=$(pods) --num-pxn=$(pxns)
SIM_OPTIONS += --drvx-stack-in-l1sp
SIM_OPTIONS += --core-stats --stats-load-level=3

SIM_ARGS += $(table-size) $(updates)
SIM_THREADS := 1
include $(DRV_DIR)/mk/config.mk
include $(DRV_DIR)/mk/application_common.mk

CXXFLAGS += $(CELLO_CXXFLAGS)
CXXFLAGS += -I$(DRV_DIR)/common/
CELLO_OBJECTS := $(CELLO_CXXSOURCES_DRVX:.cpp=.o)

$(APP_NAME).so: $(CELLO_OBJECTS)
