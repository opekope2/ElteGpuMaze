#pragma once

#include "maze_state.hpp"
#include <CL/opencl.hpp>
#include <vector>

using namespace cl;

class MazeGenerator {
protected:
    Context &ctx;
    Program program;

public:
    MazeGenerator(Context &ctx, Program program) : ctx(ctx), program(program) {}

    virtual string name() = 0;
    virtual void generate(CommandQueue &q, MazeState &state, std::vector<Event> &events) = 0;
};
