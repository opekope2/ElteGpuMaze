#pragma once

#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <algorithm>
#include <utility>
#include <vector>

template <cl_profiling_info name>
bool compareEventsBy(cl::Event lhs, cl::Event rhs) {
    return lhs.getProfilingInfo<name>() < rhs.getProfilingInfo<name>();
}

inline cl_ulong getProfilingTimeNs(std::vector<cl::Event> &events) {
    if (events.empty())
        return 0;

    auto start = std::min_element(events.begin(), events.end(), compareEventsBy<CL_PROFILING_COMMAND_START>);
    auto end = std::min_element(events.begin(), events.end(), compareEventsBy<CL_PROFILING_COMMAND_END>);

    return end->getProfilingInfo<CL_PROFILING_COMMAND_END>() - start->getProfilingInfo<CL_PROFILING_COMMAND_START>();
}

// Because Program doesn't have a constructor that accepts multiple sources while also builds it
template <typename... Args>
cl::Program buildProgram(Args &&...args) {
    cl::Program program(std::forward<Args>(args)...);
    program.build();
    return program;
}

template <typename vertex_t>
cl_uint weight(cl_uint seed, vertex_t a, vertex_t b) {
    cl_uint hash = seed;

    // xxHash primes
    hash ^= min(a, b) * 0x9e3779b1;
    hash ^= max(a, b) * 0x85ebca77;

    // xxHash avalanche
    hash ^= hash >> 15;
    hash *= 0x85ebca77;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae3d;
    hash ^= hash >> 16;

    return hash;
}
