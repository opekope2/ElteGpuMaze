#include "benchmark.hpp"
#include "util/misc.hpp"
#include <CL/cl_platform.h>
#include <ostream>

namespace benchmark {

void printResult(ostream &stream, Benchmark &benchmark, cl_uint seed, cl_ulong time) {
    stream << STR(OS) << TAB;

    stream << benchmark.platform.getInfo<CL_PLATFORM_NAME>() << TAB;
    stream << benchmark.platform.getInfo<CL_PLATFORM_VENDOR>() << TAB;
    stream << benchmark.platform.getInfo<CL_PLATFORM_VERSION>() << TAB;
    stream << benchmark.platform.getInfo<CL_PLATFORM_PROFILE>() << TAB;

    stream << benchmark.device.getInfo<CL_DEVICE_NAME>() << TAB;
    stream << benchmark.device.getInfo<CL_DEVICE_VENDOR>() << TAB;
    stream << benchmark.device.getInfo<CL_DEVICE_VERSION>() << TAB;
    stream << benchmark.device.getInfo<CL_DEVICE_PROFILE>() << TAB;
    stream << benchmark.device.getInfo<CL_DRIVER_VERSION>() << TAB;

    stream << benchmark.size << TAB;
    stream << seed << TAB;

    stream << time << endl;
}

} // namespace benchmark
