#include "app/context.hpp"
#include <GLFW/glfw3.h>

namespace solar::app {

class Window;

namespace sim { class SolarSystem; }

void key_callback(GLFWwindow* glfw_window, int key, int /*scancode*/, int action, int mods);
void register_key_handlers(solar::app::AppContext& app_context);

}
