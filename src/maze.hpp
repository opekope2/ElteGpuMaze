#pragma once

#include "../gen/kernels.hpp"
#include <CL/opencl.hpp>

#define WALL_TOP 0x1u
#define WALL_RIGHT 0x2u
#define WALL_BOTTOM 0x4u
#define WALL_LEFT 0x8u

using namespace cl;

class Maze {
private:
    int w;
    int h;
    Program maze;
    KernelFunctor<Buffer> generate;
    KernelFunctor<Buffer, ImageGL> render;
    Buffer data;

public:
    Maze(Context &ctx, int w, int h)
        : w(w),
          h(h),
          maze(ctx, reinterpret_cast<char *>(maze_cl), true),
          generate(maze, "generate"),
          render(maze, "render"),
          data(ctx, CL_MEM_READ_WRITE, sizeof(unsigned char) * w * h) {}

    int width() { return w; }
    int height() { return h; }

    void generateData(CommandQueue &q) {
        generate(EnqueueArgs(q, NDRange(w, h)), data);
    }

    void renderData(CommandQueue &q, ImageGL &img) {
        render(EnqueueArgs(q, NDRange(w, h)), data, img);
    }
};
