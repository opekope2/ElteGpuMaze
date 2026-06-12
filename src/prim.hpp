#pragma once

#include "../gen/kernels.hpp"
#include "maze_generator.hpp"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>

#define WALL_TOP 0x1u
#define WALL_RIGHT 0x2u
#define WALL_BOTTOM 0x4u
#define WALL_LEFT 0x8u

typedef cl_uint Vertex;
typedef cl_uint4 Neighbors;
typedef cl_uint2 Edge;

using namespace cl;

class PrimCL {
private:
    Program primCl;

public:
    KernelFunctor<cl_uint, cl_uint, cl_uint, LocalSpaceArg, LocalSpaceArg, LocalSpaceArg, Buffer> seqPrim;
    KernelFunctor<Buffer, ImageGL> render;

    PrimCL(Context &ctx)
        : primCl(ctx, reinterpret_cast<char *>(prim_cl), true),
          seqPrim(primCl, "seqPrim"),
          render(primCl, "render") {}
};

class SeqPrim : public MazeGenerator {
private:
    PrimCL &primCl;

public:
    SeqPrim(Context &ctx, PrimCL &primCl)
        : MazeGenerator(ctx),
          primCl(primCl) {}

    Event generate(CommandQueue &q, MazeState &state) override {
        return primCl.seqPrim(
            EnqueueArgs(q, NDRange(1, 1)),
            state.width(),
            state.height(),
            state.seed(),
            Local(sizeof(cl_uint) * state.width() * state.height()),
            Local(sizeof(Vertex) * state.width() * state.height()),
            Local(sizeof(cl_uchar) * state.width() * state.height()),
            state.mazeData());
    }

    Event render(CommandQueue &q, MazeState &state) override {
        return primCl.render(
            EnqueueArgs(q, NDRange(state.width(), state.height())),
            state.mazeData(),
            state.glImage());
    }
};
