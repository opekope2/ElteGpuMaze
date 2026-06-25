#pragma once

#include "../gen/kernels.hpp"
#include "maze_solver.hpp"
#include "maze_state.hpp"
#include "util/cl.hpp"
#include "util/maze.hpp"
#include "util/misc.hpp"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <sys/types.h>

using namespace cl;

namespace bfs {

class BFS : public MazeSolver {
protected:
    size_type parallelism;

    KernelFunctor<vertex2_t, Buffer> init;
    KernelFunctor<Buffer> mark;
    KernelFunctor<Buffer, Buffer> expand;
    KernelFunctor<cl_uint, cl_uint, Buffer, Buffer, Buffer> drawPath;
    KernelFunctor<cl_uint, cl_uint, Buffer, Buffer> vege_van;

    Buffer meet;

public:
    BFS(Context &ctx, string vegeVanKernel, size_type parallelism)
        : MazeSolver(ctx, buildProgram(ctx, cl::Program::Sources{XXD_STRING(maze_cl), XXD_STRING(solver_cl), XXD_STRING(bfs_cl)})),
          parallelism(parallelism),
          init(program, "init"),
          mark(program, "mark"),
          expand(program, "expand"),
          drawPath(program, "drawPath"),
          vege_van(program, vegeVanKernel),
          meet(ctx, CL_MEM_READ_WRITE, sizeof(vertex_t)) {}

    virtual vertex2_t getInitialFrontiers(MazeState &state) = 0;

    void markInitialFrontiers(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        q.enqueueFillBuffer<vertex_t>(meet, VERTEX_INVALID, 0, sizeof(vertex_t));

        Event initEvent = init(
            EnqueueArgs(q, NDRange(parallelism)),
            getInitialFrontiers(state),
            state.mazeData());

        events.push_back(initEvent);
    }

    bool stepSolve(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        EnqueueArgs args(q, NDRange(state.width(), state.height()));
        Event expandEvent = expand(args, state.parent(), state.mazeData());
        Event markEvent = mark(args, state.mazeData());
        Event vegeVanEvent = vege_van(
            EnqueueArgs(q, parallelism == 1 ? NDRange(1) : NDRange(state.width(), state.height())),
            state.width(),
            state.height(),
            state.mazeData(),
            meet);

        events.insert(events.end(), {expandEvent, markEvent, vegeVanEvent});

        vertex_t vege;
        q.enqueueReadBuffer(meet, CL_TRUE, 0, sizeof(vertex_t), &vege);

        return ~vege;
    }

    void showPath(CommandQueue &q, MazeState &state) override {
        drawPath(
            EnqueueArgs(q, NDRange(parallelism)),
            state.width(),
            state.height(),
            meet,
            state.parent(),
            state.mazeData());
    }
};

class ParallelBFS : public BFS {
public:
    ParallelBFS(Context &ctx) : BFS(ctx, "vege_van", 1) {}
    string name() override { return "Parallel Naive Breadth-First Search"; }
    vertex2_t getInitialFrontiers(MazeState &state) override { return {0, VERTEX_INVALID}; }
};

class Parallel2WayBFS : public BFS {
public:
    Parallel2WayBFS(Context &ctx) : BFS(ctx, "vege_van_2", 2) {}
    string name() override { return "Parallel 2-Way Naive Breadth-First Search"; }
    vertex2_t getInitialFrontiers(MazeState &state) override { return {0, state.width() * state.height() - 1}; }
};

} // namespace bfs
