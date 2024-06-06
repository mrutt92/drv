#include <cstdio>
#include <algorithm>
#include <tuple>
#include <cello.hpp>
#include "cello_sort.hpp"
#include <pandohammer/storage.h>

constexpr bool parallel = false;

l2sp_storage(sort_config) sort_cfg;

void serial_sort(vector_type &&v) {
    pointer<value_type> begin = &v[0];
    pointer<value_type> end = begin+v.size();
    std::sort(begin, end);
}


/**
 * @brief find the first element on the k-th diagonal of the merge matrix
 */
std::tuple<idx_type, idx_type>
first_on_diagonal(const vector_type &v0, const vector_type &v1, idx_type diag) {
    idx_type i_start = std::min(diag, v0.size()-1);
    idx_type j_start = diag - i_start;
    return {i_start, j_start};
}

/**
 * @brief find the coordinate of n-th element on the k-th diagonal of the merge matrix
 */
std::tuple<idx_type, idx_type>
nth_on_diagonal(const vector_type &v0, const vector_type &v1, idx_type n, idx_type diag) {
    idx_type i_start, j_start;
    std::tie(i_start, j_start) = first_on_diagonal(v0, v1, diag);
    return {i_start-n, j_start+n};
}

/**
 * @brief find the size of the search space for the k-th diagonal of the merge matrix
 */
idx_type diagonal_search_space_size(const vector_type &v0, const vector_type &v1, idx_type diag) {
    idx_type i,j;
    std::tie(i, j) = first_on_diagonal(v0, v1, diag);
    return std::min(i, v1.size()-j);
}

/**
 * @brief find the intersection of the k-th diagonal of the merge matrix
 * with the merge path
 */
std::tuple<idx_type, idx_type>
find_merge_path_intersection(const vector_type &v0, const vector_type &v1, idx_type diag) {
    idx_type hi = diagonal_search_space_size(v0, v1, diag);
    idx_type lo = 0;
    while (lo < hi) {
        idx_type m = (lo+hi)/2;
        idx_type i,j;
        std::tie(i,j) = nth_on_diagonal(v0, v1, m, diag);
        if (v0[i-1] > v1[j]) {
            lo = m+1;
        } else {
            hi = m;
        }
    }
    return nth_on_diagonal(v0, v1, lo, diag);
}


/**
 * @brief merge two sorted vectors v0 and v1 into out
 */
void do_merge
(vector_type &&v0, vector_type &&v1, vector_type &&out, idx_type blocks = 8) {
    idx_type diagonals = v0.size() + v1.size() - 1;
    idx_type block_size = diagonals / blocks;
    if (diagonals % blocks) {
        block_size++;
    }
    common::foreach_block{parallel}(0, v0.size()+v1.size(), block_size, [=, &v0, &v1, &out](idx_type start, idx_type end) mutable {
        idx_type i,j;
        // 1. binary search for the intersection of the merge path along the i-th diagonal of the merge matrix
        std::tie(i,j) = find_merge_path_intersection(v0, v1, start);
        // 2. begin merge starting at v0[i] and v1[j] to out[start:end]
        while (i+j < end && i < v0.size() && j < v1.size()) {
            if (v0[i] < v1[j]) {
                out[i+j] = v0[i];
                i++;
            } else {
                out[i+j] = v1[j];
                j++;
            }
        }
        while (i < v0.size()) {
            out[i+j] = v0[i];
            i++;
        }
        while (j < v1.size()) {
            out[i+j] = v1[j];
            j++;
        }
    });
}

void do_copy
(vector_type &&v0, vector_type &&out) {
    idx_type block_size = 64;
    common::foreach_block{parallel}(0, v0.size(), block_size, [=, &v0, &out](idx_type start, idx_type end) mutable {
        for (idx_type i = start; i < end; i++) {
            out[i] = v0[i];
        }
    });
}


struct bound {
    bound(idx_type start, idx_type end) : start(start), end(end) {}
    idx_type start;
    idx_type end;
    idx_type size() const { return end - start; }
};

int CelloMain(int argc, char *argv[])
{
    printf("hello, from cello sort\n");
    vector_type v = *sort_cfg.vec();

    idx_type block_size = 4;
    idx_type blocks = (v.size() + block_size - 1) / block_size;

    pointer<bound> bounds = (pointer<bound>)allocate_dram(sizeof(bound)*blocks);
    pointer<bound> merge_bounds = (pointer<bound>)allocate_dram(sizeof(bound)*blocks);
    vector_type merge;
    merge.init(v.size());

    common::foreach_block{parallel}(0, v.size(), block_size, [=, &v](idx_type start, idx_type end) mutable {
        serial_sort(v.slice(start, end));
        idx_type block = start / block_size;
        bounds[block] = {start, end};
        merge_bounds[block] = {0,0};
    });    

    while (blocks > 1) {
        common::foreach_block{parallel}(0, blocks, 2, [=, &merge, &v](idx_type start, idx_type end) mutable {
            idx_type b0 = start;
            idx_type b1 = end-1;
            idx_type new_block = b0 / 2;
            if (b0 != b1) {
                // merge blocks b0 and b1
                do_merge(v.slice(bounds[b0].start, bounds[b0].end),
                         v.slice(bounds[b1].start, bounds[b1].end),
                         merge.slice(bounds[b0].start, bounds[b1].end));
                merge_bounds[new_block] = {bounds[b0].start, bounds[b1].end};
            } else {
                // copy block b0 to new_block
                do_copy(v.slice(bounds[b0].start, bounds[b0].end),
                        merge.slice(bounds[b0].start, bounds[b0].end));
                merge_bounds[new_block] = bounds[b0];
            }
        });
        blocks = (blocks + 1) / 2;
        std::swap(bounds, merge_bounds);
        std::swap(merge, v);
    }
    
    *sort_cfg.vec() = v;
    
    return 0;
}
