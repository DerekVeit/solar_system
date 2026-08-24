#pragma once

#include "app/color.hpp"

#include <string>
#include <vector>

namespace solar::app {

enum class SkyFigureKind { constellation, asterism };

struct SkyFigure {
    std::string id{};
    std::string name{};
    SkyFigureKind kind{SkyFigureKind::constellation};
    std::vector<std::vector<int>> polylines{};
    bool visible{true};
};

struct SkyFigureCatalog {
    static constexpr float kLineGapDeg = 0.5f;

    std::vector<SkyFigure> figures{};
    Color color{0.55f, 0.65f, 0.85f, 0.32f};
    float line_gap_deg{kLineGapDeg};
    bool visible{true};
};

} // namespace solar::app
