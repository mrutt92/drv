# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

ifndef _RAMULATOR_CONFIG_MK_
_RAMULATOR_CONFIG_MK_ := 1
RAMULATOR := yes
RAMULATOR_DIR := $(HOME)/work/AGILE/ramulator
RAMULATOR_CXXFLAGS += -isystem $(RAMULATOR_DIR)/src
RAMULATOR_LDFLAGS  +=
RAMULATOR_LIBS     +=
endif
