#pragma once

#include "maze_state.hpp"
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>

using namespace cl;

class MazeGenerator {
protected:
    Context &ctx;

public:
    MazeGenerator(Context &ctx) : ctx(ctx) {}

    virtual Event generate(CommandQueue &q, MazeState &state) = 0;
    virtual Event render(CommandQueue &q, MazeState &state) = 0;
};
