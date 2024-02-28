ifndef MEMSYS_MK
MEMSYS_MK=1
SIM_OPTIONS += --pxn-dram-banks=1 --pod-l2sp-banks=2
SIM_OPTIONS += --pxn-dram-clock=1GHz
SIM_OPTIONS += --dram-backend=dramsim3
SIM_OPTIONS += --dram-backend-config=$(DRAMSIM3_DIR)/configs/$(subst -,_,$(memsys)).ini
endif
