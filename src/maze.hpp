#pragma once

#include "util/glfw.hpp"
#include <CL/opencl.hpp>

void mazeGui(GlfwWindow &win, cl::Context &ctx, cl::CommandQueue &q);
