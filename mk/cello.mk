ifndef CELLO_MK
CELLO_MK=1
DRV_DIR ?= $(shell git rev-parse --show-toplevel)

CELLO_BACKEND  :=  DRVX
CELLO_CXXFLAGS += -I$(DRV_DIR)/cello

vpath %.cpp $(DRV_DIR)/cello
vpath %.c   $(DRV_DIR)/cello

########
# DRVX #
########
CELLO_CXXSOURCES_DRVX += cello_core_drvx.cpp
CELLO_CXXSOURCES_DRVX += cello_core_drvx_allocator.cpp

########
# DRVR #
########
# sources
CELLO_CXXSOURCES_DRVR += cello_core_drvr.cpp
# compile flags
CELLO_CXXFLAGS_DRVR += $(CELLO_CXXFLAGS)
CELLO_CXXFLAGS_DRVR += -DRISCV -DCORE_THREADS=$(THREADS)
CELLO_CXXFLAGS_DRVR += -std=c++17
# link flags
CELLO_LDFLAGS_DRVX +=
CELLO_LDFLAGS_DRVR +=

# common model options for the simulator
#SIM_OPTIONS += --drvx-stack-in-l1sp
SIM_OPTIONS += --core-stats --stats-load-level=3
SIM_OPTIONS += --core-clock=125MHz

memsys ?= HBM2-1Gb-x64
include $(DRV_DIR)/mk/$(memsys).mk

endif
