#pragma once

#include "../gen/kernels.hpp"
#include "maze_solver.hpp"
#include "util/binary_heap.hpp"
#include "util/cl.hpp"
#include "util/maze.hpp"
#include "util/misc.hpp"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>

#define N_DISCOVERED(s) s.size1()
#define HEAP(s) s.vertex1()
#define LOOKUP(s) s.vertex2()
#define PRIORITIES(s) s.uint1()
#define DISTANCE_FROM_START(s) s.uint2()

namespace astar {

class AStar : public MazeSolver {
private:
    KernelFunctor<cl_uint, cl_uint, Buffer, Buffer, Buffer, Buffer, Buffer, Buffer> init;
    KernelFunctor<cl_uint, cl_uint, Buffer, Buffer, Buffer, Buffer, Buffer, Buffer, Buffer, Buffer> aStar;
    KernelFunctor<Buffer, Buffer, Buffer> drawPath;

public:
    AStar(Context &ctx)
        : MazeSolver(ctx, buildProgram(ctx, cl::Program::Sources{XXD_STRING(maze_cl), XXD_STRING(solver_cl), XXD_STRING(binary_heap_cl), XXD_STRING(astar_cl)})),
          init(program, "init"),
          aStar(program, "aStar"),
          drawPath(program, "drawPath") {}

    string name() override { return "Sequential A*"; }

    void markInitialFrontiers(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        q.enqueueFillBuffer<vertex_t>(state.meet(), VERTEX_INVALID, 0, sizeof(vertex_t));
        q.enqueueFillBuffer<heap_data_t>(LOOKUP(state), HEAP_MISSING, 0, sizeof(cl_uint) * state.width() * state.height());
        q.enqueueFillBuffer<cl_uint>(DISTANCE_FROM_START(state), CL_UINT_MAX, 0, sizeof(cl_uint) * state.width() * state.height());

        Event initEvent = init(
            EnqueueArgs(q, NDRange(1)),
            state.width(),
            state.height(),
            N_DISCOVERED(state),
            HEAP(state),
            LOOKUP(state),
            PRIORITIES(state),
            DISTANCE_FROM_START(state),
            state.mazeData());

        events.push_back(initEvent);
    }

    bool stepSolve(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        Event aStarEvent = aStar(
            EnqueueArgs(q, NDRange(1)),
            state.width(),
            state.height(),
            N_DISCOVERED(state),
            HEAP(state),
            LOOKUP(state),
            PRIORITIES(state),
            DISTANCE_FROM_START(state),
            state.parent(),
            state.mazeData(),
            state.meet());

        events.push_back(aStarEvent);

        vertex_t vege;
        q.enqueueReadBuffer(state.meet(), CL_TRUE, 0, sizeof(vertex_t), &vege);

        return ~vege;
    }

    void showPath(CommandQueue &q, MazeState &state) override {
        drawPath(
            EnqueueArgs(q, NDRange(1)),
            state.meet(),
            state.parent(),
            state.mazeData());
    }
};

} // namespace astar
