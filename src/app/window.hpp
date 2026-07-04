#pragma once

#include <GL/gl.h>
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

    [[nodiscard]] GLFWwindow* handle() const { return window_; }
    [[nodiscard]] int framebuffer_width() const;
    [[nodiscard]] int framebuffer_height() const;

    void set_clear_color(float r, float g, float b, float a);

  private:
    GLFWwindow* window_{nullptr};
};

}  // namespace solar::app