#pragma once

#if defined(GUI)

#include "gl.hpp"
#include "misc.hpp"
#include <GLFW/glfw3.h>

using GlfwWindow = Handle<GLFWwindow *, glfwDestroyWindow>;

class Glfw {
private:
    Glfw() { CHECK(glfwInit(), glfwInit); }

    ~Glfw() { glfwTerminate(); }

    Glfw(const Glfw &) = delete;
    Glfw &operator=(const Glfw &) = delete;

    Glfw(Glfw &) noexcept = delete;
    Glfw &operator=(Glfw &&) = delete;

public:
    GlfwWindow createWindow(int width, int height, const char *title, GLFWmonitor *monitor, GLFWwindow *share) {
        auto win = glfwCreateWindow(width, height, title, monitor, share);
        CHECK(win, glfwCreateWindow);
        return GlfwWindow(win);
    }

    static Glfw &instance() {
        static Glfw instance;
        return instance;
    }
};

#endif
