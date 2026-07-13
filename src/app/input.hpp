#pragma once

#include "app/context.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace solar::app {

void key_callback(GLFWwindow* glfw_window, int key, int /*scancode*/, int action, int mods);
void register_key_handlers(AppContext& app_context);

} // namespace solar::app
