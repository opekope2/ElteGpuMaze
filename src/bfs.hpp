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

#define PREV_WAVEFRONT_SIZE(s) s.size1()
#define WAVEFRONT_SIZE(s) s.size2()
#define PREV_WAVEFRONT(s) s.vertex1()
#define WAVEFRONT(s) s.vertex2()

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

public:
    BFS(Context &ctx, string vegeVanKernel, size_type parallelism)
        : MazeSolver(ctx, buildProgram(ctx, cl::Program::Sources{XXD_STRING(maze_cl), XXD_STRING(solver_cl), XXD_STRING(bfs_cl)})),
          parallelism(parallelism),
          init(program, "init"),
          mark(program, "mark"),
          expand(program, "expand"),
          drawPath(program, "drawPath"),
          vege_van(program, vegeVanKernel) {}

    virtual vertex2_t getInitialFrontiers(MazeState &state) = 0;

    void markInitialFrontiers(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        q.enqueueFillBuffer<vertex_t>(state.meet(), VERTEX_INVALID, 0, sizeof(vertex_t));

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
            state.meet());

        events.insert(events.end(), {expandEvent, markEvent, vegeVanEvent});

        vertex_t vege;
        q.enqueueReadBuffer(state.meet(), CL_TRUE, 0, sizeof(vertex_t), &vege);

        return ~vege;
    }

    void showPath(CommandQueue &q, MazeState &state) override {
        drawPath(
            EnqueueArgs(q, NDRange(parallelism)),
            state.width(),
            state.height(),
            state.meet(),
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

class WavefrontBFS : public MazeSolver {
private:
    size_type parallelism;

    KernelFunctor<vertex2_t, Buffer, Buffer, Buffer> initWavefront;
    KernelFunctor<cl_uint, cl_uint, Buffer, Buffer, Buffer, Buffer, Buffer, Buffer> expandWavefront;
    KernelFunctor<cl_uint, cl_uint, Buffer, Buffer, Buffer> drawPath;
    KernelFunctor<cl_uint, cl_uint, Buffer, Buffer, Buffer, Buffer> vege_van;

public:
    WavefrontBFS(Context &ctx, string vegeVanKernel, size_type parallelism)
        : MazeSolver(ctx, buildProgram(ctx, cl::Program::Sources{XXD_STRING(maze_cl), XXD_STRING(solver_cl), XXD_STRING(bfs_cl)})),
          parallelism(parallelism),
          initWavefront(program, "init_wavefront"),
          expandWavefront(program, "expand_wavefront"),
          drawPath(program, "drawPath"),
          vege_van(program, vegeVanKernel) {}

    virtual vertex2_t getInitialFrontiers(MazeState &state) = 0;

    void markInitialFrontiers(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        q.enqueueFillBuffer<vertex_t>(state.meet(), VERTEX_INVALID, 0, sizeof(vertex_t));

        Event initEvent = initWavefront(
            EnqueueArgs(q, NDRange(parallelism)),
            getInitialFrontiers(state),
            WAVEFRONT_SIZE(state),
            WAVEFRONT(state),
            state.mazeData());

        events.push_back(initEvent);

        state.updateWavefrontSize(q);
        state.swapWavefronts();
    }

    bool stepSolve(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        q.enqueueFillBuffer<vertex_t>(WAVEFRONT_SIZE(state), 0, 0, sizeof(vertex_t));

        EnqueueArgs args(q, NDRange(nextPowerOf2(state.cachedWavefrontSize())));
        Event expandEvent = expandWavefront(
            args,
            state.width(),
            state.height(),
            PREV_WAVEFRONT_SIZE(state),
            PREV_WAVEFRONT(state),
            WAVEFRONT_SIZE(state),
            WAVEFRONT(state),
            state.parent(),
            state.mazeData());
        Event vegeVanEvent = vege_van(
            args,
            state.width(),
            state.height(),
            PREV_WAVEFRONT_SIZE(state),
            PREV_WAVEFRONT(state),
            state.mazeData(),
            state.meet());

        events.insert(events.end(), {expandEvent, vegeVanEvent});

        vertex_t vege;
        q.enqueueReadBuffer(state.meet(), CL_TRUE, 0, sizeof(vertex_t), &vege);

        state.updateWavefrontSize(q);
        state.swapWavefronts();

        return ~vege;
    }

    void showPath(CommandQueue &q, MazeState &state) override {
        drawPath(
            EnqueueArgs(q, NDRange(parallelism)),
            state.width(),
            state.height(),
            state.meet(),
            state.parent(),
            state.mazeData());
    }
};

class ParallelWavefrontBFS : public WavefrontBFS {
public:
    ParallelWavefrontBFS(Context &ctx) : WavefrontBFS(ctx, "wege_wan", 1) {}
    string name() override { return "Parallel Wavefront Breadth-First Search"; }
    vertex2_t getInitialFrontiers(MazeState &state) override { return {0, VERTEX_INVALID}; }
};

class Parallel2WayWavefrontBFS : public WavefrontBFS {
public:
    Parallel2WayWavefrontBFS(Context &ctx) : WavefrontBFS(ctx, "wege_wan_2", 2) {}
    string name() override { return "Parallel 2-Way Wavefront Breadth-First Search"; }
    vertex2_t getInitialFrontiers(MazeState &state) override { return {0, state.width() * state.height() - 1}; }
};

} // namespace bfs
