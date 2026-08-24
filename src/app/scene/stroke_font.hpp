#pragma once

#include <glm/ext/vector_double3.hpp>

#include <cstddef>
#include <string_view>
#include <vector>

namespace solar::app {

/// Cap height in font units (baseline y = 0). Glyphs occupy x in [0, 4].
inline constexpr double kStrokeFontCapHeight = 6.0;
inline constexpr double kStrokeFontAdvance = 5.0;

/// GL_LINES vertex count (two per stroke) for `text` in this font.
[[nodiscard]] std::size_t stroke_text_line_vertex_count(std::string_view text);

/// Append screen-plane stroke segments for `text`. `axis_x` / `axis_y` are the
/// world vectors of one font unit along the baseline and the cap-height axis.
/// Endpoints are pushed in pairs for `GL_LINES`.
void append_stroke_text(std::string_view text, const glm::dvec3& origin, const glm::dvec3& axis_x,
                        const glm::dvec3& axis_y, std::vector<glm::dvec3>& endpoints);

} // namespace solar::app
