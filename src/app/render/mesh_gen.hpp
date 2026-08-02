#pragma once

#include <vector>

namespace solar::app {

struct MeshData {
    int stride;
    int index_count;
    std::vector<float> interleaved;
    std::vector<unsigned> indices;
};

MeshData generate_unit_sphere(int slices, int stacks);

} // namespace solar::app
