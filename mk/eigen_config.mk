# SPDX-License-Identifier: MIT
# Copyright (c) 2024 University of Washington
ifndef _EIGEN_CONFIG_MK_
_EIGEN_CONFIG_MK_ = 1
EIGEN_INSTALL_DIR=$(DRV_ROOT)
EIGEN_CXXFLAGS += -I$(EIGEN_INSTALL_DIR)/include/eigen3
endif
