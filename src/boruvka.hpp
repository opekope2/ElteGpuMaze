#pragma once

#include "../gen/kernels.hpp"
#include "maze_generator.hpp"
#include "maze_state.hpp"
#include "util/cl.hpp"
#include "util/dsu.hpp"
#include "util/maze.hpp"
#include "util/misc.hpp"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <vector>

using namespace cl;

namespace boruvka {

#define EMPLACE_EDGE(vector, u, v, stride, seed) \
    vector.emplace_back(u, v, weight(seed, stride, u, v))

class SequentialBoruvka : public MazeGenerator {
private:
    Program program;
    KernelFunctor<dsu_size_t, cl_uint, Buffer, Buffer, Buffer, Buffer, Buffer> boruvka;
    KernelFunctor<Buffer, ImageGL> render;

public:
    SequentialBoruvka(Context &ctx)
        : MazeGenerator(ctx),
          program(buildProgram(ctx, cl::Program::Sources{XXD_STRING(maze_cl), XXD_STRING(dsu_cl), XXD_STRING(boruvka_cl)})),
          boruvka(program, "boruvka"),
          render(program, "render") {}

    string name() override { return "Sequential Boruvka"; }

    void generateEdges(MazeState &state, std::vector<Edge> &edges) {
        cl_uint w = state.width(), h = state.height(), s = state.seed();
        edges.reserve(2 * w * h - w - h);
        for (cl_uint j = 1; j < w; j++)
            EMPLACE_EDGE(edges, j - 1, j, w, s);
        for (cl_uint i = 1; i < h; i++)
            EMPLACE_EDGE(edges, (i - 1) * w, i * w, w, s);
        for (cl_uint i = 1; i < h; i++)
            for (cl_uint j = 1; j < w; j++) {
                cl_uint ij = i * w + j;
                EMPLACE_EDGE(edges, ij - 1, ij, w, s);
                EMPLACE_EDGE(edges, ij - w, ij, w, s);
            }
    }

    void generateAndRender(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        size_type n = static_cast<size_type>(state.width()) * static_cast<size_type>(state.height());
        std::vector<Edge> edges_vector;
        generateEdges(state, edges_vector);

        // Does not fit into local memory on moderately large mazes, which resets my GPU
        Buffer dsu_size(ctx, CL_MEM_READ_WRITE, sizeof(dsu_size_t) * n);
        Buffer dsu_parent(ctx, CL_MEM_READ_WRITE, sizeof(dsu_vertex_t) * n);
        Buffer minout(ctx, CL_MEM_READ_WRITE, sizeof(cl_uint) * n);
        Buffer edges_buffer(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(Edge) * edges_vector.size(), edges_vector.data());

        q.enqueueFillBuffer<maze_data_t>(state.mazeData(), WALL_TOP | WALL_RIGHT | WALL_BOTTOM | WALL_LEFT, 0, sizeof(maze_data_t) * n);

        Event generateEvent = boruvka(
            EnqueueArgs(q, NDRange(1)),
            n,
            edges_vector.size(),
            dsu_size,
            dsu_parent,
            minout,
            edges_buffer,
            state.mazeData());
        Event renderEvent = render(
            EnqueueArgs(q, NDRange(state.width(), state.height())),
            state.mazeData(),
            state.glImage());

        q.finish();
        events.insert(events.end(), {generateEvent, renderEvent});
    }
};

} // namespace boruvka
