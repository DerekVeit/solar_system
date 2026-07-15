#pragma once

#include "app/color.hpp"

namespace solar::app {

struct PointInstance {
    float x_ndc{0.0f};
    float y_ndc{0.0f};
    Color color{};
    float point_size{1.0f};
};

} // namespace solar::app