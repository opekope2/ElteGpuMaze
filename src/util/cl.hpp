#pragma once

#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <utility>
#include <vector>

template <cl_profiling_info name>
bool compareEventsBy(cl::Event lhs, cl::Event rhs) {
    return lhs.getProfilingInfo<name>() < rhs.getProfilingInfo<name>();
}

cl_ulong getProfilingTimeNs(std::vector<cl::Event> &events);

// Because Program doesn't have a constructor that accepts multiple sources while also builds it
template <typename... Args>
cl::Program buildProgram(Args &&...args) {
    cl::Program program(std::forward<Args>(args)...);
    program.build();
    return program;
}
