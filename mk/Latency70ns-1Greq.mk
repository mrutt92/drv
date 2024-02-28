ifndef MEMSYS_MK
MEMSYS_MK=1
SIM_OPTIONS += --pxn-dram-banks=1 --pod-l2sp-banks=2
SIM_OPTIONS += --dram-access-time=70ns
SIM_OPTIONS += --pxn-dram-clock=1GHz
endif
