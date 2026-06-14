#pragma once

#include "maze_state.hpp"
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <vector>

using namespace cl;

class MazeGenerator {
protected:
    Context &ctx;

public:
    MazeGenerator(Context &ctx) : ctx(ctx) {}

    virtual string name() = 0;
    virtual std::vector<Event> generateAndRender(CommandQueue &q, MazeState &state) = 0;
};
