#pragma once

#include "app.hpp"
#include "kernels.hpp"
#include <CL/cl.h>
#include <CL/opencl.hpp>
#include <cassert>

using namespace cl;

class MazeApp : public App {
private:
    Program maze;
    // TODO KernelFunctor

public:
    MazeApp(Device &dev, Context &ctx)
        : App(dev, ctx),
          maze(buildProgramFromSource(reinterpret_cast<char *>(maze_cl))) {}

    void run() override {}
};
