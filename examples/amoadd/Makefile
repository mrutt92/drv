DRV_DIR := $(shell git rev-parse --show-toplevel)
include $(DRV_DIR)/mk/config.mk
APP_NAME := amoadd
include $(DRV_DIR)/mk/application_common.mk

run-%: $(APP_NAME).so
	$(eval saddr=$(word 1,$(subst -, ,$*)))
	$(eval baddr=$(word 2,$(subst -, ,$*)))
	sst $(DRV_DIR)/tests/drv-multicore-bus-dramsim-test.py -- $(APP_PATH)/$(APP_NAME).so $(baddr) $(saddr)

run-sim: run-0x80000000-0x80000010
