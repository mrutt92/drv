# SPDX-License-Identifier: MIT
# Copyright (c) 2024 University of Washington

# import parameters and APP_PATH
n = 128
m = 128
k = 128
include app_path.mk

DRV_DIR := $(shell git rev-parse --show-toplevel)

include $(DRV_DIR)/mk/cello.mk
include $(DRV_DIR)/mk/eigen_config.mk

vpath %.c   $(DRV_DIR)/examples/cello_gemm
vpath %.cpp $(DRV_DIR)/examples/cello_gemm
vpath %.c   $(DRV_DIR)/common/util
vpath %.cpp $(DRV_DIR)/common/util

APP_EXE ?= $(APP_PATH)/$(test-name)/$(APP_NAME).so

INPUT_DIR := $(DRV_DIR)/inputs/sparse-inputs

SIM_OPTIONS += --core-threads=$(threads) --pod-cores=$(cores)
SIM_OPTIONS += --pxn-pods=$(pods) --num-pxn=$(pxns)
SIM_OPTIONS += --core-stats --stats-load-level=3

SIM_THREADS := 1
include $(DRV_DIR)/mk/config.mk
include $(DRV_DIR)/mk/application_common.mk

CXXFLAGS += -DGEMM_D0=$(n) -DGEMM_D1=$(m) -DGEMM_D2=$(k)
CXXFLAGS += $(EIGEN_CXXFLAGS)
CXXFLAGS += $(CELLO_CXXFLAGS)
CXXFLAGS += -I$(DRV_DIR)/common/
CELLO_OBJECTS := $(CELLO_CXXSOURCES_DRVX:.cpp=.o)

$(APP_NAME).so: $(DRV_DIR)/examples/cello_gemm/cello_gemm.o
$(APP_NAME).so: $(CELLO_OBJECTS)
