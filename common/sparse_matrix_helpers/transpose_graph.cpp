// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#include "transpose_graph.hpp"
#include <algorithm>
#include <tuple>
int transpose_graph(int V,
                    int E,
                    const std::vector<int>&fwd_offsets,
                    const std::vector<int>&fwd_nonzeros,
                    /* outputs */
                    std::vector<int>&rev_offsets,
                    std::vector<int>&rev_nonzeros) {
    rev_offsets = std::vector<int>(V+1);
    rev_nonzeros = std::vector<int>(E);
    std::vector<std::vector<int>> columns(V+1);
    for (int s = 0; s < V; s++) {
        int start = fwd_offsets[s];
        int stop = fwd_offsets[s+1];
        for (int k = start; k < stop; k++) {
            int d = fwd_nonzeros[k];
            columns[d].push_back(s);
        }
    }
    int sum = 0;
    for (size_t d = 0; d < rev_offsets.size(); d++) {
        rev_offsets[d] = sum;
        sum += columns[d].size();
    }
    int n = 0;
    for (int d = 0; d < V; d++) {
        std::sort(columns[d].begin(), columns[d].end());
        for (int s : columns[d]) {
            rev_nonzeros[n++] = s;
        }
    }
    return 0;
}

int transpose_sparse_matrix(int V,
                    int E,
                    const std::vector<int>&fwd_offsets,
                    const std::vector<std::pair<int,float>>&fwd_nonzeros,
                    /* outputs */
                    std::vector<int>&rev_offsets,
                    std::vector<std::pair<int,float>>&rev_nonzeros) {
    rev_offsets = std::vector<int>(V+1);
    rev_nonzeros = std::vector<std::pair<int,float>>(E);
    std::vector<std::vector<std::pair<int,float>>> columns(V+1);
    for (int s = 0; s < V; s++) {
        int start = fwd_offsets[s];
        int stop = fwd_offsets[s+1];
        for (int k = start; k < stop; k++) {
            int d; float w;
            std::tie(d,w) = fwd_nonzeros[k];
            columns[d].push_back({s,w});
        }
    }
    int sum = 0;
    for (size_t d = 0; d < rev_offsets.size(); d++) {
        rev_offsets[d] = sum;
        sum += columns[d].size();
    }
    int n = 0;
    for (int d = 0; d < V; d++) {
        std::sort(columns[d].begin(), columns[d].end(), [](const std::pair<int,float>&a, const std::pair<int,float>&b) {
            return a.first < b.first;
        });
        for (std::pair<int,float> e : columns[d]) {
            rev_nonzeros[n++] = e;
        }
    }
    return 0;
}
