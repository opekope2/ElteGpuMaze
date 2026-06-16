#pragma once

#include "../gen/kernels.hpp"
#include "maze_solver.hpp"
#include "maze_state.hpp"
#include "util/cl.hpp"
#include "util/maze.hpp"
#include "util/misc.hpp"
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <sys/types.h>

using namespace cl;

namespace bfs {

class ParallelBFS : public MazeSolver {
private:
    Program program;
    KernelFunctor<Buffer> mark;
    KernelFunctor<Buffer, Buffer> expand;
    KernelFunctor<Buffer, ImageGL> render;
    KernelFunctor<cl_uint, cl_uint, Buffer, Buffer> drawPath;

public:
    ParallelBFS(Context &ctx)
        : MazeSolver(ctx),
          program(buildProgram(ctx, cl::Program::Sources{XXD_STRING(maze_cl), XXD_STRING(solver_cl), XXD_STRING(bfs_cl)})),
          mark(program, "mark"),
          expand(program, "expand"),
          render(program, "render"),
          drawPath(program, "drawPath") {}

    string name() override { return "Parallel Breadth-First Search"; }

    bool stepSolve(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        size_type n = static_cast<size_type>(state.width()) * static_cast<size_type>(state.height());

        EnqueueArgs args(q, NDRange(state.width(), state.height()));
        Event markEvent = mark(args, state.mazeData());
        Event expandEvent = expand(args, state.parent(), state.mazeData());
        Event drawPathEvent = drawPath(
            EnqueueArgs(q, NDRange(1)),
            state.width(),
            state.height(),
            state.parent(),
            state.mazeData());
        Event renderEvent = render(args, state.mazeData(), state.glImage());

        maze_data_t lastCell;
        q.enqueueReadBuffer(state.mazeData(), CL_FALSE, sizeof(maze_data_t) * (n - 1), sizeof(maze_data_t), &lastCell);

        q.finish();
        events.insert(events.end(), {markEvent, expandEvent, drawPathEvent, renderEvent});

        return (lastCell & SEARCH_EXPLORED) != 0;
    }
};

} // namespace bfs
