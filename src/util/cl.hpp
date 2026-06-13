#pragma once

#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <utility>

cl_ulong getProfilingTimeNs(cl::Event &event);

// Because Program doesn't have a constructor that accepts multiple sources while also builds it
template <typename... Args>
cl::Program buildProgram(Args &&...args) {
    cl::Program program(std::forward<Args>(args)...);
    program.build();
    return program;
}
