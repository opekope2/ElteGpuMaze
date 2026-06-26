#pragma once

#include "../gen/kernels.hpp"
#include "maze_generator.hpp"
#include "maze_state.hpp"
#include "util/cl.hpp"
#include "util/maze.hpp"
#include "util/misc.hpp"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>

#define HEAP(s) s.vertex1()
#define LOOKUP(s) s.vertex2()
#define PRIORITIES(s) s.uint1()
#define UNEXPLORED(s) s.uint2()

using namespace cl;

namespace prim {

class SequentialPrim : public MazeGenerator {
private:
    KernelFunctor<cl_uint, cl_uint, cl_uint, Buffer, Buffer, Buffer, Buffer, Buffer, Buffer> seqPrim;

public:
    SequentialPrim(Context &ctx)
        : MazeGenerator(ctx, buildProgram(ctx, cl::Program::Sources{XXD_STRING(maze_cl), XXD_STRING(set_cl), XXD_STRING(binary_heap_cl), XXD_STRING(prim_cl)})),
          seqPrim(program, "seqPrim") {}

    string name() override { return "Sequential Prim"; }

    void generate(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        size_type n = static_cast<size_type>(state.width()) * static_cast<size_type>(state.height());

        // Does not fit into local memory on moderately large mazes, which resets my GPU
        Buffer cheapestEdge(ctx, CL_MEM_READ_WRITE, sizeof(vertex_t) * n);

        q.enqueueFillBuffer<vertex_t>(cheapestEdge, VERTEX_INVALID, 0, sizeof(vertex_t) * n);
        q.enqueueFillBuffer<cl_uchar>(UNEXPLORED(state), 1, 0, sizeof(cl_uchar) * n);
        q.enqueueFillBuffer<maze_data_t>(state.mazeData(), WALL_TOP | WALL_RIGHT | WALL_BOTTOM | WALL_LEFT, 0, sizeof(maze_data_t) * n);

        Event generateEvent = seqPrim(
            EnqueueArgs(q, NDRange(1)),
            state.width(),
            state.height(),
            state.seed(),
            cheapestEdge,
            UNEXPLORED(state),
            HEAP(state),
            LOOKUP(state),
            PRIORITIES(state),
            state.mazeData());

        events.push_back(generateEvent);
    }
};

} // namespace prim
