DRAMSIM3_DIR ?= $(HOME)/work/AGILE/DRAMSim3
DRAMSIM3_CXXFLAGS += -I$(DRAMSIM3_DIR)/src -I$(DRAMSIM3_DIR)/ext/headers -I$(DRAMSIM3_DIR)/ext/fmt/include
DRAMSIM3_LDFLAGS  += -L$(DRAMSIM3_DIR)/build -ldramsim3 -Wl,-rpath=$(DRAMSIM3_DIR)/build
