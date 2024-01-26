# SPDX-License-Identifier: MIT
# Copyright (c) 2024 University of Washington

# import parameters and APP_PATH
include parameters.mk
include app_path.mk

DRV_DIR := $(shell git rev-parse --show-toplevel)
include $(DRV_DIR)/mk/cello.mk

vpath %.c   $(APP_PATH)
vpath %.cpp $(APP_PATH)
vpath %.c   $(APP_PATH)/sparse_matrix_helpers
vpath %.cpp $(APP_PATH)/sparse_matrix_helpers

APP_EXE ?= $(APP_PATH)/$(test-name)/$(APP_NAME).so

SIM_OPTIONS += --core-threads=$(threads) --pod-cores=$(cores)
SIM_OPTIONS += --pxn-pods=$(pods) --num-pxn=$(pxns)
SIM_OPTIONS += --drvx-stack-in-l1sp
SIM_ARGS += $(APP_PATH)/sparse-inputs/$(graph).mtx $(start)
SIM_THREADS := 2
include $(DRV_DIR)/mk/config.mk
include $(DRV_DIR)/mk/application_common.mk

CXXFLAGS += $(CELLO_CXXFLAGS)
CXXFLAGS += -I$(APP_PATH)/sparse_matrix_helpers
CELLO_OBJECTS := $(CELLO_CXXSOURCES_DRVX:.cpp=.o)

$(APP_NAME).so: breadth_first_search_graph.o
$(APP_NAME).so: mmio.o
$(APP_NAME).so: read_graph.o
$(APP_NAME).so: transpose_graph.o
$(APP_NAME).so: $(CELLO_OBJECTS)

run: $(APP_PATH)/sparse-inputs/$(graph).mtx

$(APP_PATH)/sparse-inputs/$(graph).mtx:
	$(MAKE) -C $(dir $@)  $(graph).mtx
