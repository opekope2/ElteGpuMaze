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
    KernelFunctor<vertex2_t, Buffer> init;
    KernelFunctor<Buffer> mark;
    KernelFunctor<Buffer, Buffer> expand;
    KernelFunctor<cl_uint, cl_uint, Buffer, Buffer, Buffer> drawPath;

    Buffer meet;

public:
    BFS(Context &ctx)
        : MazeSolver(ctx, buildProgram(ctx, cl::Program::Sources{XXD_STRING(maze_cl), XXD_STRING(solver_cl), XXD_STRING(bfs_cl)})),
          init(program, "init"),
          mark(program, "mark"),
          expand(program, "expand"),
          drawPath(program, "drawPath"),
          meet(ctx, CL_MEM_READ_WRITE, sizeof(vertex_t)) {}

    virtual bool stepSolve(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        EnqueueArgs args(q, NDRange(state.width(), state.height()));
        Event expandEvent = expand(args, state.parent(), state.mazeData());
        Event markEvent = mark(args, state.mazeData());

        events.insert(events.end(), {expandEvent, markEvent});

        return false;
    }
};

class ParallelBFS : public BFS {
public:
    ParallelBFS(Context &ctx) : BFS(ctx) {}

    string name() override { return "Parallel Breadth-First Search"; }

    void markInitialFrontiers(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        Event initEvent = init(
            EnqueueArgs(q, NDRange(1)),
            {0, VERTEX_INVALID},
            state.mazeData());

        events.push_back(initEvent);
    }

    bool stepSolve(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        BFS::stepSolve(q, state, events);

        size_type n = static_cast<size_type>(state.width()) * static_cast<size_type>(state.height());
        maze_data_t vege;
        q.enqueueReadBuffer(state.mazeData(), CL_TRUE, sizeof(maze_data_t) * (n - 1), sizeof(maze_data_t), &vege);

        return (vege & SEARCH_EXPLORED) != 0;
    }

    void showPath(CommandQueue &q, MazeState &state) override {
        q.enqueueFillBuffer<vertex_t>(meet, state.width() * state.height() - 1, 0, sizeof(vertex_t));
        drawPath(
            EnqueueArgs(q, NDRange(1)),
            state.width(),
            state.height(),
            meet,
            state.parent(),
            state.mazeData());
    }
};

class Parallel2WayBFS : public BFS {
private:
    KernelFunctor<Buffer, Buffer> vege_van;

public:
    Parallel2WayBFS(Context &ctx) : BFS(ctx), vege_van(program, "vege_van") {}

    string name() override { return "Parallel 2-Way Breadth-First Search"; }

    void markInitialFrontiers(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        Event initEvent = init(
            EnqueueArgs(q, NDRange(2)),
            {0, state.width() * state.height() - 1},
            state.mazeData());

        events.push_back(initEvent);
    }

    bool stepSolve(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        BFS::stepSolve(q, state, events);

        vertex_t vege = VERTEX_INVALID;
        q.enqueueWriteBuffer(meet, CL_FALSE, 0, sizeof(vertex_t), &vege);

        Event vege_van_event = vege_van(
            EnqueueArgs(q, NDRange(state.width(), state.height())),
            state.mazeData(),
            meet);
        events.push_back(vege_van_event);
        q.enqueueReadBuffer(meet, CL_TRUE, 0, sizeof(vertex_t), &vege);

        return ~vege;
    }

    void showPath(CommandQueue &q, MazeState &state) override {
        drawPath(
            EnqueueArgs(q, NDRange(2)),
            state.width(),
            state.height(),
            meet,
            state.parent(),
            state.mazeData());
    }
};

} // namespace bfs
