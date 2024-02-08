#include "triangle_counting.hpp"

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
}
