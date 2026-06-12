#pragma once

#include "../gen/kernels.hpp"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <vector>

#define WALL_TOP 0x1u
#define WALL_RIGHT 0x2u
#define WALL_BOTTOM 0x4u
#define WALL_LEFT 0x8u

typedef cl_uint Vertex;
typedef cl_uint4 Neighbors;
typedef cl_uint2 Edge;

using namespace cl;

class Prim {
private:
    cl_uint w;
    cl_uint h;
    cl_uint seed;
    Program maze;
    KernelFunctor<cl_uint, cl_uint, LocalSpaceArg, LocalSpaceArg, LocalSpaceArg, Buffer> seqPrim;
    KernelFunctor<Buffer, ImageGL> render;
    Buffer mazeData;

public:
    Prim(Context &ctx, cl_uint w, cl_uint h, cl_uint seed)
        : w(w),
          h(h),
          seed(seed),
          maze(ctx, reinterpret_cast<char *>(prim_cl), true),
          seqPrim(maze, "seqPrim"),
          render(maze, "render"),
          mazeData(ctx, CL_MEM_READ_WRITE, sizeof(cl_uchar) * w * h) {}

    cl_uint width() { return w; }
    cl_uint height() { return h; }

    void generateSeq(CommandQueue &q) {
        seqPrim(
            EnqueueArgs(q, NDRange(1, 1)),
            w,
            seed,
            Local(sizeof(cl_uint) * w * h),
            Local(sizeof(Vertex) * w * h),
            Local(sizeof(cl_uchar) * w * h),
            mazeData);
    }

    void renderPar(CommandQueue &q, ImageGL &img) {
        render(EnqueueArgs(q, NDRange(w, h)), mazeData, img);
    }
};
