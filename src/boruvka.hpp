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

#define DSU_SIZE(s) s.uint1()
#define DSU_PARENT(s) s.vertex1()
#define MINOUT(s) s.uint2()

using namespace cl;

namespace boruvka {

class SequentialBoruvka : public MazeGenerator {
private:
    KernelFunctor<Buffer, Buffer> dsu_init;
    KernelFunctor<cl_uint, Buffer> generateEdges;
    KernelFunctor<dsu_size_t, cl_uint, Buffer, Buffer, Buffer, Buffer, Buffer> boruvka;

public:
    SequentialBoruvka(Context &ctx)
        : MazeGenerator(ctx, buildProgram(ctx, cl::Program::Sources{XXD_STRING(maze_cl), XXD_STRING(dsu_cl), XXD_STRING(generator_cl), XXD_STRING(boruvka_cl)})),
          dsu_init(program, "dsu_init"),
          generateEdges(program, "generateEdges"),
          boruvka(program, "boruvka") {}

    string name() override { return "Sequential Boruvka"; }

    void generate(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        size_type n = static_cast<size_type>(state.width()) * static_cast<size_type>(state.height());
        dsu_size_t m = 2 * state.width() * state.height() - state.width() - state.height();

        // Does not fit into local memory on moderately large mazes, which resets my GPU
        Buffer edges_buffer(ctx, CL_MEM_READ_WRITE, sizeof(Edge) * m);

        q.enqueueFillBuffer<maze_data_t>(state.mazeData(), WALL_TOP | WALL_RIGHT | WALL_BOTTOM | WALL_LEFT, 0, sizeof(maze_data_t) * n);

        Event dsuInitEvent = dsu_init(
            EnqueueArgs(q, NDRange(n)),
            DSU_SIZE(state),
            DSU_PARENT(state));
        Event generateEdgesEvent = generateEdges(
            EnqueueArgs(q, NDRange(state.width(), state.height())),
            state.seed(),
            edges_buffer);
        Event generateEvent = boruvka(
            EnqueueArgs(q, NDRange(1)),
            n,
            m,
            DSU_SIZE(state),
            DSU_PARENT(state),
            MINOUT(state),
            edges_buffer,
            state.mazeData());

        events.insert(events.end(), {dsuInitEvent, generateEdgesEvent, generateEvent});
    }
};

} // namespace boruvka
