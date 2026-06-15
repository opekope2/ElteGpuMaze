#pragma once

#include "../gen/kernels.hpp"
#include "maze_generator.hpp"
#include "maze_state.hpp"
#include "util/cl.hpp"
#include "util/misc.hpp"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <vector>

#define WALL_TOP 0x1u
#define WALL_RIGHT 0x2u
#define WALL_BOTTOM 0x4u
#define WALL_LEFT 0x8u

using namespace cl;

namespace boruvka {

typedef cl_uint dsu_size_t;
typedef cl_uint dsu_vertex_t;
typedef cl_uint weight_t;

typedef struct Edge {
    dsu_vertex_t u, v;
    weight_t w;

    Edge(dsu_vertex_t u, dsu_vertex_t v, cl_uint seed) : u(u), v(v), w(weight(seed, u, v)) {}
} Edge;

class BoruvkaCL {
private:
    Program boruvkaCl;

public:
    KernelFunctor<dsu_size_t, cl_uint, Buffer, Buffer, Buffer, Buffer, Buffer> seqBoruvka;
    KernelFunctor<Buffer, ImageGL> render;

    BoruvkaCL(Context &ctx)
        : boruvkaCl(buildProgram(ctx, cl::Program::Sources{XXD_STRING(maze_cl), XXD_STRING(dsu_cl), XXD_STRING(boruvka_cl)})),
          seqBoruvka(boruvkaCl, "boruvka"),
          render(boruvkaCl, "render") {}

    void generateEdges(MazeState &state, std::vector<Edge> &edges) {
        cl_uint w = state.width(), h = state.height(), s = state.seed();
        edges.reserve(2 * w * h - w - h);
        for (cl_uint j = 1; j < w; j++)
            edges.emplace_back(j - 1, j, s);
        for (cl_uint i = 1; i < h; i++)
            edges.emplace_back((i - 1) * w, i * w, s);
        for (cl_uint i = 1; i < h; i++)
            for (cl_uint j = 1; j < w; j++) {
                cl_uint ij = i * w + j;
                edges.emplace_back(ij - 1, ij, s);
                edges.emplace_back(ij - w, ij, s);
            }
    }
};

class SeqBoruvka : public MazeGenerator {
private:
    BoruvkaCL &boruvkaCl;

public:
    SeqBoruvka(Context &ctx, BoruvkaCL &boruvkaCl) : MazeGenerator(ctx), boruvkaCl(boruvkaCl) {}

    string name() override { return "Sequential Boruvka"; }

    void generateAndRender(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        size_type n = static_cast<size_type>(state.width()) * static_cast<size_type>(state.height());
        std::vector<Edge> edges_vector;
        boruvkaCl.generateEdges(state, edges_vector);

        // Does not fit into local memory on moderately large mazes, which resets my GPU
        Buffer dsu_size(state.context(), CL_MEM_READ_WRITE, sizeof(dsu_size_t) * n);
        Buffer dsu_parent(state.context(), CL_MEM_READ_WRITE, sizeof(dsu_vertex_t) * n);
        Buffer minout(state.context(), CL_MEM_READ_WRITE, sizeof(cl_uint) * n);
        Buffer edges_buffer(state.context(), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(Edge) * edges_vector.size(), edges_vector.data());

        q.enqueueFillBuffer<maze_data_t>(state.mazeData(), WALL_TOP | WALL_RIGHT | WALL_BOTTOM | WALL_LEFT, 0, sizeof(maze_data_t) * n);

        Event generate = boruvkaCl.seqBoruvka(
            EnqueueArgs(q, NDRange(1)),
            n,
            edges_vector.size(),
            dsu_size,
            dsu_parent,
            minout,
            edges_buffer,
            state.mazeData());
        Event render = boruvkaCl.render(
            EnqueueArgs(q, NDRange(state.width(), state.height())),
            state.mazeData(),
            state.glImage());

        q.finish();
        events.insert(events.end(), {generate, render});
    }
};
} // namespace boruvka