DRV_DIR ?= $(shell git rev-parse --show-toplevel)
export PYTHONPATH := $(DRV_DIR)/py

testbenches = \
	cello_bfs \
	cello_gups \
	cello_jaccard \
	cello_pr \
	cello_spmm \
	cello_tc

testbenches_run = $(addsuffix .run, $(testbenches))

.PHONY: all
all: $(testbenches_run)
$(testbenches_run): %.run:
	@echo "Running $*"
	@cd $(DRV_DIR)/examples/$* && python3 run.py --do=generate
#	@$(MAKE) -C $(DRV_DIR)/examples/$* purge
	@cd $(DRV_DIR)/examples/$* && python3 run.py --do=run
	@cd $(DRV_DIR)/examples/$* && python3 run.py --do=coalesce_results
	@cd $(DRV_DIR)/examples/$* && python3 run.py --do=upload
