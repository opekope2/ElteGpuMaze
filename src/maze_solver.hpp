#pragma once

#include "maze_state.hpp"
#include <CL/opencl.hpp>
#include <vector>

using namespace cl;

class MazeSolver {
protected:
    Context &ctx;
    Program program;

public:
    MazeSolver(Context &ctx, Program program) : ctx(ctx), program(program) {}

    virtual string name() = 0;
    virtual void markInitialFrontiers(CommandQueue &q, MazeState &state, std::vector<Event> &events) = 0;
    virtual bool stepSolve(CommandQueue &q, MazeState &state, std::vector<Event> &events) = 0;
};
