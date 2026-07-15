#pragma once

#include <string_view>

namespace solar::app {

unsigned int compile_shader(unsigned int type, std::string_view source);
unsigned int link_program(unsigned int vertex_shader, unsigned int fragment_shader);

} // namespace solar::app