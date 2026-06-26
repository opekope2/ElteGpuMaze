#pragma once

#include "../gen/kernels.hpp"
#include "maze_generator.hpp"
#include "util/bitonic_sort.hpp"
#include "util/cl.hpp"
#include "util/dsu.hpp"
#include "util/misc.hpp"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <climits>
#include <vector>

#define DSU_SIZE(s) s.uint1()
#define DSU_PARENT(s) s.vertex1()

using namespace cl;

namespace kruskal {

class ParallelSortedKruskal : public MazeGenerator {
private:
    KernelFunctor<Buffer, Buffer> dsuInit;
    KernelFunctor<cl_uint, Buffer> generateEdges;
    KernelFunctor<dsu_size_t, dsu_size_t, Buffer> bitonicSwap;
    KernelFunctor<dsu_size_t, cl_uint, Buffer, Buffer, Buffer, Buffer> kruskal;

public:
    ParallelSortedKruskal(Context &ctx)
        : MazeGenerator(ctx, buildProgram(ctx, cl::Program::Sources{XXD_STRING(maze_cl), XXD_STRING(dsu_cl), XXD_STRING(generator_cl), XXD_STRING(bitonic_sort_cl), XXD_STRING(kruskal_cl)})),
          dsuInit(program, "dsu_init"),
          generateEdges(program, "generateEdges"),
          bitonicSwap(program, "bitonicSwap"),
          kruskal(program, "kruskal") {}

    string name() override { return "Parallel-Sorted Kruskal"; }

    void generate(CommandQueue &q, MazeState &state, std::vector<Event> &events) override {
        size_type n = static_cast<size_type>(state.width()) * static_cast<size_type>(state.height());
        dsu_size_t m = 2 * state.width() * state.height() - state.width() - state.height();
        dsu_size_t m2 = nextPowerOf2(m);

        // Does not fit into local memory on moderately large mazes, which resets my GPU
        Buffer edges(ctx, CL_MEM_READ_WRITE, sizeof(Edge) * m2);

        q.enqueueFillBuffer<cl_uint>(edges, UINT_MAX, sizeof(Edge) * m, sizeof(Edge) * (m2 - m));
        q.enqueueFillBuffer<maze_data_t>(state.mazeData(), WALL_TOP | WALL_RIGHT | WALL_BOTTOM | WALL_LEFT, 0, sizeof(maze_data_t) * n);

        Event dsuInitEvent = dsuInit(
            EnqueueArgs(q, NDRange(n)),
            DSU_SIZE(state),
            DSU_PARENT(state));
        Event generateEdgesEvent = generateEdges(
            EnqueueArgs(q, NDRange(state.width(), state.height())),
            state.seed(),
            edges);
        events.insert(events.end(), {dsuInitEvent, generateEdgesEvent});
        parallelBitonicMergeSort(q, bitonicSwap, m2, edges, events);
        Event generateEvent = kruskal(
            EnqueueArgs(q, NDRange(1)),
            n,
            m2,
            DSU_SIZE(state),
            DSU_PARENT(state),
            edges,
            state.mazeData());

        events.push_back(generateEvent);
    }
};

} // namespace kruskal
