#pragma once

#include "../gen/kernels.hpp"
#include "maze_generator.hpp"
#include "util/cl.hpp"
#include "util/misc.hpp"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <climits>

#define WALL_TOP 0x1u
#define WALL_RIGHT 0x2u
#define WALL_BOTTOM 0x4u
#define WALL_LEFT 0x8u

#define VERTEX_INVALID UINT_MAX

typedef cl_uint Vertex;
typedef cl_uint4 Neighbors;
typedef cl_uint2 Edge;

using namespace cl;

namespace prim {

class PrimCL {
private:
    Program primCl;

public:
    KernelFunctor<cl_uint, cl_uint, cl_uint, Buffer, Buffer, Buffer, Buffer, Buffer, Buffer> seqPrim;
    KernelFunctor<Buffer, ImageGL> render;

    PrimCL(Context &ctx)
        : primCl(buildProgram(ctx, cl::Program::Sources{XXD_STRING(maze_cl), XXD_STRING(set_cl), XXD_STRING(binary_heap_cl), XXD_STRING(prim_cl)})),
          seqPrim(primCl, "seqPrim"),
          render(primCl, "render") {}
};

class SeqPrim : public MazeGenerator {
private:
    PrimCL &primCl;

public:
    SeqPrim(Context &ctx, PrimCL &primCl) : MazeGenerator(ctx), primCl(primCl) {}

    string name() override { return "Sequential Prim"; }

    void generateAndRender(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        size_type n = static_cast<size_type>(state.width()) * static_cast<size_type>(state.height());

        // Does not fit into local memory on moderately large mazes, which resets my GPU
        Buffer cheapestEdge(state.context(), CL_MEM_READ_WRITE, sizeof(Vertex) * n);
        Buffer unexplored(state.context(), CL_MEM_READ_WRITE, sizeof(cl_uchar) * n);
        Buffer heap(state.context(), CL_MEM_READ_WRITE, sizeof(Vertex) * n);
        Buffer lookup(state.context(), CL_MEM_READ_WRITE, sizeof(Vertex) * n);
        Buffer priorities(state.context(), CL_MEM_READ_WRITE, sizeof(cl_uint) * n);

        q.enqueueFillBuffer<Vertex>(cheapestEdge, VERTEX_INVALID, 0, sizeof(Vertex) * n);
        q.enqueueFillBuffer<cl_uchar>(unexplored, 1, 0, sizeof(cl_uchar) * n);
        q.enqueueFillBuffer<cl_uchar>(state.mazeData(), WALL_TOP | WALL_RIGHT | WALL_BOTTOM | WALL_LEFT, 0, sizeof(cl_uchar) * n);

        Event generate = primCl.seqPrim(
            EnqueueArgs(q, NDRange(1)),
            state.width(),
            state.height(),
            state.seed(),
            cheapestEdge,
            unexplored,
            heap,
            lookup,
            priorities,
            state.mazeData());
        Event render = primCl.render(
            EnqueueArgs(q, NDRange(state.width(), state.height())),
            state.mazeData(),
            state.glImage());

        q.finish();
        events.insert(events.end(), {generate, render});
    }
};
} // namespace prim