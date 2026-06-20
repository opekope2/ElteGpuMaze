#pragma once

#include "maze_state.hpp"
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <vector>

using namespace cl;

class MazeGenerator {
protected:
    Context &ctx;
    Program program;
    KernelFunctor<Buffer, ImageGL> render;

public:
    MazeGenerator(Context &ctx, Program program)
        : ctx(ctx),
          program(program),
          render(program, "render") {}

    virtual string name() = 0;

    virtual void generate(CommandQueue &q, MazeState &state, std::vector<Event> &events) = 0;

    virtual void renderMazeData(CommandQueue &q, MazeState &state, std::vector<Event> &events) {
        Event renderEvent = render(
            EnqueueArgs(q, NDRange(state.width(), state.height())),
            state.mazeData(),
            state.glImage());

        q.finish();
        events.push_back(renderEvent);
    }
};
