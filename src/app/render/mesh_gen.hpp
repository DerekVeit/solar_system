#pragma once

#include <vector>

namespace solar::app {

struct MeshData {
    int stride{};
    int index_count{};
    std::vector<float> interleaved;
    std::vector<unsigned> indices;
};

/// Unit sphere: interleaved x,y,z,u,v (u/v in 0..1; last longitude column duplicates u=1).
MeshData generate_unit_sphere(int slices, int stacks);

/// Unit disc in XY: interleaved x,y,z,radial (radial 0 at center, 1 at rim).
MeshData generate_ring_disc(int slices);

} // namespace solar::app
