#pragma once

#include "maze_state.hpp"
#include <CL/opencl.hpp>
#include <vector>

using namespace cl;

class MazeSolver {
protected:
    Context &ctx;

public:
    MazeSolver(Context &ctx) : ctx(ctx) {}

    virtual string name() = 0;
    virtual bool stepSolve(CommandQueue &q, MazeState &state, std::vector<Event> &events) = 0;
};
