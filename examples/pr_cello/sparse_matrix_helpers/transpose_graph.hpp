// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#pragma once
#include <vector>
int transpose_graph(int V,
                    int E,
                    const std::vector<int>&fwd_offsets,
                    const std::vector<int>&fwd_nonzeros,
                    /* outputs */
                    std::vector<int>&rev_offsets,
                    std::vector<int>&rev_nonzeros);

int transpose_sparse_matrix
(int V,
 int E,
 const std::vector<int>&fwd_offsets,
 const std::vector<std::pair<int,float>>&fwd_nonzeros,
 /* outputs */
 std::vector<int>&rev_offsets,
 std::vector<std::pair<int,float>>&rev_nonzeros);
