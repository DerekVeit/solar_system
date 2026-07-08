#pragma once

#include "app/color.hpp"

#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <string>

struct GLFWwindow;

namespace solar::app {

struct WindowConfig {
    std::string title{"Solar System"};
    bool fullscreen{true};
    int windowed_width{1920};
    int windowed_height{1200};
};

class Window {
  public:
    explicit Window(const WindowConfig& config);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    [[nodiscard]] bool should_close() const;
    void poll_events();
    void swap_buffers();

    [[nodiscard]] int framebuffer_width() const;
    [[nodiscard]] int framebuffer_height() const;

    void set_clear_color(solar::app::Color c);

    void set_user_pointer(void* data);

    void set_key_callback(GLFWkeyfun callback);

    void clear_frame();

    void request_close();

  private:
    GLFWwindow* window_{nullptr};
};

} // namespace solar::app