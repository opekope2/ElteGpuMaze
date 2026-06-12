#pragma once

#include <CL/cl_platform.h>
#include <CL/opencl.hpp>

cl_ulong getProfilingTimeNs(cl::Event &event);
