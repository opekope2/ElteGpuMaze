#pragma once

#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <utility>
#include <vector>

inline cl_ulong getProfilingTimeNs(std::vector<cl::Event> &events) {
    cl_ulong ns = 0;
    for (auto &&e : events)
        ns += e.getProfilingInfo<CL_PROFILING_COMMAND_END>() - e.getProfilingInfo<CL_PROFILING_COMMAND_START>();

    return ns;
}

// Because Program doesn't have a constructor that accepts multiple sources while also builds it
template <typename... Args>
cl::Program buildProgram(Args &&...args) {
    cl::Program program(std::forward<Args>(args)...);
    program.build();
    return program;
}
