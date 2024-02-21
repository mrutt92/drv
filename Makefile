# SPDX-License-Identifier: MIT
# Copyright (c) 2023 University of Washington

DRV_DIR ?= $(shell git rev-parse --show-toplevel)

all: install examples examples-run

EXAMPLES += $(DRV_DIR)/examples/addrmap
EXAMPLES += $(DRV_DIR)/examples/allocator
EXAMPLES += $(DRV_DIR)/examples/amoadd
EXAMPLES += $(DRV_DIR)/examples/argv
EXAMPLES += $(DRV_DIR)/examples/atomic
#EXAMPLES += $(DRV_DIR)/examples/bfs
EXAMPLES += $(DRV_DIR)/examples/bitrangehandle
EXAMPLES += $(DRV_DIR)/examples/cas
#EXAMPLES += $(DRV_DIR)/examples/cello_bfs
#EXAMPLES += $(DRV_DIR)/examples/cello_gups
#EXAMPLES += $(DRV_DIR)/examples/cello_jaccard
#EXAMPLES += $(DRV_DIR)/examples/cello_pr
#EXAMPLES += $(DRV_DIR)/examples/cello_spmm
#EXAMPLES += $(DRV_DIR)/examples/cello_tc
EXAMPLES += $(DRV_DIR)/examples/comm
EXAMPLES += $(DRV_DIR)/examples/commandprocessor
EXAMPLES += $(DRV_DIR)/examples/commandprocessor_allocator
EXAMPLES += $(DRV_DIR)/examples/commandprocessor_ctrl
EXAMPLES += $(DRV_DIR)/examples/fence
EXAMPLES += $(DRV_DIR)/examples/fibonacci
EXAMPLES += $(DRV_DIR)/examples/globals
EXAMPLES += $(DRV_DIR)/examples/gups
EXAMPLES += $(DRV_DIR)/examples/gups-latency
EXAMPLES += $(DRV_DIR)/examples/gups_multi_node
#oEXAMPLES += $(DRV_DIR)/examples/IDM
EXAMPLES += $(DRV_DIR)/examples/info
EXAMPLES += $(DRV_DIR)/examples/latency
EXAMPLES += $(DRV_DIR)/examples/lock
EXAMPLES += $(DRV_DIR)/examples/mem
EXAMPLES += $(DRV_DIR)/examples/multicore
EXAMPLES += $(DRV_DIR)/examples/multicoreamoadd
EXAMPLES += $(DRV_DIR)/examples/multimem
EXAMPLES += $(DRV_DIR)/examples/nop
EXAMPLES += $(DRV_DIR)/examples/parallel_for
EXAMPLES += $(DRV_DIR)/examples/pointer
EXAMPLES += $(DRV_DIR)/examples/pointer_v2
EXAMPLES += $(DRV_DIR)/examples/simple
EXAMPLES += $(DRV_DIR)/examples/stream
EXAMPLES += $(DRV_DIR)/examples/super_simple_task_runtime
EXAMPLES += $(DRV_DIR)/examples/thread
EXAMPLES += $(DRV_DIR)/examples/to_address
EXAMPLES += $(DRV_DIR)/examples/to_native

.PHONY: all install install-element install-api install-interpreter install-py
.PHONY: clean  examples $(EXAMPLES)

install: install-api install-element install-interpreter install-py

install-api:
	$(MAKE) -C $(DRV_DIR)/api/ install

install-interpreter:
	$(MAKE) -C $(DRV_DIR)/interpreter/ install

install-element: install-api install-interpreter
	$(MAKE) -C $(DRV_DIR)/element/ install

install-py:
	$(MAKE) -C $(DRV_DIR)/py/ install

$(foreach example, $(EXAMPLES), $(example)-clean): %-clean:
	$(MAKE) -C $* clean

$(foreach example, $(EXAMPLES), $(example)-run): %-run:
	$(MAKE) -C $* clean
	$(MAKE) -C $* run

clean: #$(foreach example, $(EXAMPLES), $(example)-clean)
	$(MAKE) -C $(DRV_DIR)/element/ clean
	$(MAKE) -C $(DRV_DIR)/api/ clean
	$(MAKE) -C $(DRV_DIR)/interpreter/ clean
	$(MAKE) -C $(DRV_DIR)/py/ clean
	rm -rf install/

$(EXAMPLES): install-api install-element install-interpreter

examples: $(EXAMPLES)

examples-run: $(foreach example, $(EXAMPLES), $(example)-run)

$(EXAMPLES):
	$(MAKE) -C $@ all
