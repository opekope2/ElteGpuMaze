#include "cl.hpp"
#include <CL/cl_platform.h>

cl_ulong getProfilingTimeNs(cl::Event &event) {
    return event.getProfilingInfo<CL_PROFILING_COMMAND_END>() - event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
}
