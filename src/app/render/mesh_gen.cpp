#include "app/render/mesh_gen.hpp"

#include "core/constants.hpp"

#include <cmath>

namespace solar::app {

MeshData generate_unit_sphere(int slices, int stacks) {
    MeshData mesh{};
    mesh.stride = 5 * static_cast<int>(sizeof(float));

    // u = longitude 0..1, v = latitude 0..1 (from top to bottom)
    for (int stack = 0; stack <= stacks; ++stack) {
        const float v = static_cast<float>(stack) / static_cast<float>(stacks);
        const float phi = v * static_cast<float>(solar::core::kPi);
        const float z = std::cos(phi);
        const float ring_radius = std::sin(phi);

        for (int slice = 0; slice <= slices; ++slice) {
            const float u = static_cast<float>(slice) / static_cast<float>(slices);
            const float theta = u * (2.0f * static_cast<float>(solar::core::kPi));
            const float x = ring_radius * std::cos(theta);
            const float y = ring_radius * std::sin(theta);
            mesh.interleaved.push_back(x);
            mesh.interleaved.push_back(y);
            mesh.interleaved.push_back(z);
            mesh.interleaved.push_back(u);
            mesh.interleaved.push_back(v);
        }
    }

    const int ring_stride = slices + 1;
    for (int stack = 0; stack < stacks; ++stack) {
        for (int slice = 0; slice < slices; ++slice) {
            //  i0 --- i0+1
            //   |   /  |
            //   |  /   |
            //  i1 --- i1+1
            const unsigned int i0 = static_cast<unsigned int>(stack * ring_stride + slice);
            const unsigned int i1 = i0 + static_cast<unsigned int>(ring_stride);
            mesh.indices.push_back(i0);
            mesh.indices.push_back(i1);
            mesh.indices.push_back(i0 + 1);
            mesh.indices.push_back(i0 + 1);
            mesh.indices.push_back(i1);
            mesh.indices.push_back(i1 + 1);
        }
    }

    mesh.index_count = static_cast<int>(mesh.indices.size());
    return mesh;
}

MeshData generate_ring_disc(int slices) {
    MeshData mesh{};
    mesh.stride = 4 * static_cast<int>(sizeof(float));

    // Center
    mesh.interleaved.push_back(0.0f);
    mesh.interleaved.push_back(0.0f);
    mesh.interleaved.push_back(0.0f);
    mesh.interleaved.push_back(0.0f);

    const float two_pi = 2.0f * static_cast<float>(solar::core::kPi);
    for (int slice = 0; slice <= slices; ++slice) {
        const float u = static_cast<float>(slice) / static_cast<float>(slices);
        const float theta = u * two_pi;
        mesh.interleaved.push_back(std::cos(theta));
        mesh.interleaved.push_back(std::sin(theta));
        mesh.interleaved.push_back(0.0f);
        mesh.interleaved.push_back(1.0f);
    }

    for (int slice = 0; slice < slices; ++slice) {
        const unsigned int i = static_cast<unsigned int>(slice + 1);
        mesh.indices.push_back(0);
        mesh.indices.push_back(i);
        mesh.indices.push_back(i + 1);
    }

    mesh.index_count = static_cast<int>(mesh.indices.size());
    return mesh;
}

} // namespace solar::app
