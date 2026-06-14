#include "cl.hpp"
#include <CL/cl.h>
#include <CL/opencl.hpp>
#include <algorithm>

cl_ulong getProfilingTimeNs(std::vector<cl::Event> &events) {
    if (events.empty())
        return 0;

    auto start = std::min_element(events.begin(), events.end(), compareEventsBy<CL_PROFILING_COMMAND_START>);
    auto end = std::min_element(events.begin(), events.end(), compareEventsBy<CL_PROFILING_COMMAND_END>);

    return end->getProfilingInfo<CL_PROFILING_COMMAND_END>() - start->getProfilingInfo<CL_PROFILING_COMMAND_START>();
}
