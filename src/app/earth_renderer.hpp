#pragma once

#include "core/types.hpp"

namespace solar::app {

/// Draws Earth as a single GL point in heliocentric ecliptic xy (km scaled to clip space).
class EarthRenderer {
  public:
    bool init();
    void draw(const core::Displacement& earth_position) const;

  private:
    unsigned int program_{0};
    unsigned int vao_{0};
    int u_pos_location_{-1};
    int u_size_location_{-1};
    int u_color_location_{-1};
};

} // namespace solar::app