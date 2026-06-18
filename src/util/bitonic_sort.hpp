#pragma once

#include "dsu.hpp"
#include <CL/opencl.hpp>

using namespace cl;

inline void parallelBitonicMergeSort(CommandQueue &q, KernelFunctor<dsu_size_t, dsu_size_t, Buffer> &bitonicSwap, dsu_size_t size, Buffer &edges, std::vector<Event> &events) {
    for (dsu_size_t count = 2; count <= size; count <<= 1) {
        for (dsu_size_t stride = count >> 1; stride > 0; stride >>= 1) {
            Event swapEvent = bitonicSwap(
                EnqueueArgs(q, NDRange(size)),
                stride,
                count,
                edges);
            events.push_back(swapEvent);
        }
    }
}
