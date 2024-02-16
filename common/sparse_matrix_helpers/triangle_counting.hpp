#pragma once
#include <vector>
#include <set>
#include <tuple>

namespace tc {

using triangle = std::tuple<int, int, int>;

void
triangle_counting(
 int V,
 int E,
 const std::vector<int> &offsets,
 const std::vector<int> &nonzeros,
 std::set<triangle> &triangles);


void
relabel_by_ascending_degree(
 int V,
 int E,
 const std::vector<int> &offsets_i,
 const std::vector<int> &nonzeros_i,
 std::vector<int> &offsets_o,
 std::vector<int> &nonzeros_o);

}

