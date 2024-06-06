ifndef CELLO_MK
CELLO_MK=1
DRV_DIR ?= $(shell git rev-parse --show-toplevel)

CELLO_BACKEND  ?=  DRVX
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
CELLO_CXXSOURCES_DRVR += pandohammer_allocator.cpp

# compile flags
CELLO_CXXFLAGS_DRVR += $(CELLO_CXXFLAGS)
CELLO_CXXFLAGS_DRVR += -DRISCV -DCORE_THREADS=$(THREADS)
CELLO_CXXFLAGS_DRVR += -std=c++17

COMMAND_PROCESSOR_PLATFORM_LOADER := no
COMMAND_PROCESSOR_COMPILE_FLAGS += -I$(DRV_DIR)/cello
COMMAND_PROCESSOR_CXXSOURCE += cello_core_drvr_commandprocessor.cpp
COMMAND_PROCESSOR_CXXSOURCE += cello_core_drvr_makeapp.cpp

# link flags
CELLO_LDFLAGS_DRVX +=
CELLO_LDFLAGS_DRVR +=

# select core clock
CELLO_CORE_CLOCK_DRXX := --core-clock=125MHz
CELLO_CORE_CLOCK_DRVR := --core-clock=1GHz

# common model options for the simulator
#SIM_OPTIONS += --drvx-stack-in-l1sp
SIM_OPTIONS += --core-stats --stats-load-level=3


SIM_OPTIONS += $(CELLO_CORE_CLOCK_$(CELLO_BACKEND))

memsys ?= HBM2-1Gb-x64
include $(DRV_DIR)/mk/$(memsys).mk

endif
