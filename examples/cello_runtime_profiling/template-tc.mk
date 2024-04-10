# SPDX-License-Identifier: MIT
# Copyright (c) 2024 University of Washington

# import parameters and APP_PATH
graph := u12k16
include app_path.mk

DRV_DIR := $(shell git rev-parse --show-toplevel)
include $(DRV_DIR)/mk/cello.mk

vpath %.c   $(DRV_DIR)/examples/cello_tc
vpath %.cpp $(DRV_DIR)/examples/cello_tc
vpath %.c   $(DRV_DIR)/common/sparse_matrix_helpers
vpath %.cpp $(DRV_DIR)/common/sparse_matrix_helpers
vpath %.c   $(DRV_DIR)/common/util
vpath %.cpp $(DRV_DIR)/common/util

APP_EXE ?= $(APP_PATH)/$(test-name)/$(APP_NAME).so

INPUT_DIR := $(DRV_DIR)/inputs/sparse-inputs

SIM_OPTIONS += --core-threads=$(threads) --pod-cores=$(cores)
SIM_OPTIONS += --pxn-pods=$(pods) --num-pxn=$(pxns)
SIM_OPTIONS += --core-stats --stats-load-level=3
TAG_BREAKDOWN_OPTIONS += --start-tag=breadth_first_search_start
TAG_BREAKDOWN_OPTIONS += --end-tag=breadth_first_search_end
SIM_ARGS += $(INPUT_DIR)/$(graph).mtx
SIM_THREADS := 1
include $(DRV_DIR)/mk/config.mk
include $(DRV_DIR)/mk/application_common.mk

CXXFLAGS += $(CELLO_CXXFLAGS)
CXXFLAGS += -I$(DRV_DIR)/common/sparse_matrix_helpers
CXXFLAGS += -I$(DRV_DIR)/common/
CELLO_OBJECTS := $(CELLO_CXXSOURCES_DRVX:.cpp=.o)

$(APP_NAME).so: cello_tc.o
$(APP_NAME).so: mmio.o
$(APP_NAME).so: read_graph.o
$(APP_NAME).so: transpose_graph.o
$(APP_NAME).so: triangle_counting.o
$(APP_NAME).so: $(CELLO_OBJECTS)

run.log: $(INPUT_DIR)/$(graph).mtx

$(INPUT_DIR)/$(graph).mtx:
	$(MAKE) -C $(dir $@)  $(graph).mtx
