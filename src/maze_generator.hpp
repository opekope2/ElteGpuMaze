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
    virtual void generateAndRender(CommandQueue &q, MazeState &state, std::vector<Event> &events) = 0;
};
