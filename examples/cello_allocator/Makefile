# SPDX-License-Identifier: MIT
# Copyright (c) 2024 University of Washington

# import parameters and APP_PATH

DRV_DIR := $(shell git rev-parse --show-toplevel)
APP_PATH ?= $(DRV_DIR)/examples/cello_allocator
APP_NAME ?= cello_allocator

include $(DRV_DIR)/mk/cello.mk

vpath %.c   $(APP_PATH)
vpath %.cpp $(APP_PATH)
vpath %.c   $(DRV_DIR)/common/util
vpath %.cpp $(DRV_DIR)/common/util

APP_EXE ?= $(APP_PATH)/$(test-name)/$(APP_NAME).so

INPUT_DIR := $(DRV_DIR)/inputs/sparse-inputs

threads ?= 1
cores   ?= 1
pods    ?= 1
pxns    ?= 1

SIM_OPTIONS += --core-threads=$(threads) --pod-cores=$(cores)
SIM_OPTIONS += --pxn-pods=$(pods) --num-pxn=$(pxns)
SIM_OPTIONS += --core-stats --stats-load-level=3
TAG_BREAKDOWN_OPTIONS += --start-tag=breadth_first_search_start
TAG_BREAKDOWN_OPTIONS += --end-tag=breadth_first_search_end
SIM_ARGS += $(INPUT_DIR)/$(graph).mtx $(start)
SIM_THREADS := 1
include $(DRV_DIR)/mk/config.mk
include $(DRV_DIR)/mk/application_common.mk

CXXFLAGS += $(CELLO_CXXFLAGS)
CXXFLAGS += -I$(DRV_DIR)/common/sparse_matrix_helpers
CXXFLAGS += -I$(DRV_DIR)/common/
CELLO_OBJECTS := $(CELLO_CXXSOURCES_DRVX:.cpp=.o)

$(APP_NAME).so: $(CELLO_OBJECTS)

