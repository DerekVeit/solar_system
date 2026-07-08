#include "app/window.hpp"

#include "app/color.hpp"

#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <stdexcept>

namespace solar::app {

namespace {

void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
}

} // namespace

Window::Window(const WindowConfig& config) {
    if (!glfwInit()) {
        throw std::runtime_error("failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    GLFWmonitor* monitor = nullptr;
    int width = config.windowed_width;
    int height = config.windowed_height;

    if (config.fullscreen) {
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (mode == nullptr) {
            throw std::runtime_error("failed to query primary monitor video mode");
        }
        width = mode->width;
        height = mode->height;
    }

    window_ = glfwCreateWindow(width, height, config.title.c_str(), monitor, nullptr);
    if (window_ == nullptr) {
        glfwTerminate();
        throw std::runtime_error("failed to create GLFW window");
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);

    int framebuffer_width = 0;
    int framebuffer_height = 0;
    glfwGetFramebufferSize(window_, &framebuffer_width, &framebuffer_height);
    glViewport(0, 0, framebuffer_width, framebuffer_height);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.02f, 0.02f, 0.06f, 1.0f);
}

Window::~Window() {
    if (window_ != nullptr) {
        glfwDestroyWindow(window_);
    }
    glfwTerminate();
}

bool Window::should_close() const { return glfwWindowShouldClose(window_) != 0; }

void Window::poll_events() { glfwPollEvents(); }

void Window::swap_buffers() { glfwSwapBuffers(window_); }

int Window::framebuffer_width() const {
    int width = 0;
    glfwGetFramebufferSize(window_, &width, nullptr);
    return width;
}

int Window::framebuffer_height() const {
    int height = 0;
    glfwGetFramebufferSize(window_, nullptr, &height);
    return height;
}

void Window::set_clear_color(solar::app::Color c) { glClearColor(c.r, c.g, c.b, c.a); }

void Window::set_user_pointer(void* data) { glfwSetWindowUserPointer(window_, data); }

void Window::set_key_callback(GLFWkeyfun callback) { glfwSetKeyCallback(window_, callback); }

void Window::clear_frame() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

void Window::request_close() { glfwSetWindowShouldClose(window_, GLFW_TRUE); }

} // namespace solar::app