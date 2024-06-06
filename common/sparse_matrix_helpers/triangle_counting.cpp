#include "triangle_counting.hpp"
#include <algorithm>
#include <numeric>
namespace tc
{

void
insert(
  std::set<triangle> &triangles,
  int u,
  int v,
  int w) {
    if (u > v) std::swap(u, v);
    if (v > w) std::swap(v, w);
    if (u > v) std::swap(u, v);
    triangles.insert(triangle{u, v, w});
}

void
triangle_counting(
  int V,
  int E,
  const std::vector<int> &offsets,
  const std::vector<int> &nonzeros,
  std::set<triangle> &triangles) {
    // brute force triangle counting
    for (int u = 0; u < V; u++) {
        for (int i = offsets[u]; i < offsets[u + 1]; i++) {
            int v = nonzeros[i];
            if (v != u) {
                for (int j = offsets[v]; j < offsets[v + 1]; j++) {
                    int w = nonzeros[j];
                    if (w != u && w != v) {
                        for (int k = offsets[w]; k < offsets[w + 1]; k++) {
                            int x = nonzeros[k];
                            if (x == u) {
                                insert(triangles, u, v, w);
                            }
                        }
                    }
                }
            }
        }
    }
}

int binary_search(std::vector<int> v, int key) {
    int low = 0;
    int high = v.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (v[mid] == key) {
            return mid;
        }
        if (v[mid] < key) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return v.size();
}

void
relabel_by_ascending_degree(
 int V,
 int E,
 const std::vector<int> &offsets_i,
 const std::vector<int> &nonzeros_i,
 std::vector<int> &offsets_o,
 std::vector<int> &nonzeros_o) {
    std::vector<int> degrees(V, 0);
    for (int i = 0; i < V; i++) {
        degrees[i] = offsets_i[i + 1] - offsets_i[i];
    }

    std::vector<int> order(V);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&degrees](int u, int v) {
        return degrees[u] < degrees[v];
    });

    std::vector<int> newlabel(V);
    for (int i = 0; i < V; i++) {
        newlabel[order[i]] = i;
    }
    std::vector<int> offsets(V + 1);
    std::vector<int> nonzeros(E);

    int k = 0;
    for (int i = 0; i < V; i++) {
        int u = order[i];
        offsets[i] = k;
        int start = k;
        for (int j = offsets_i[u]; j < offsets_i[u + 1]; j++) {
            nonzeros[k++] = newlabel[nonzeros_i[j]];
        }
        int end = k;
        std::sort(&nonzeros[start], &nonzeros[end]);
    }
    offsets[V] = k;
    offsets_o = std::move(offsets);
    nonzeros_o = std::move(nonzeros);
}

}

