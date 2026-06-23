#pragma once

#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <ostream>

#define TAB "\t"

using namespace std;
using namespace cl;

namespace benchmark {

const string HEADER = "os" TAB

                      "platform.name" TAB
                      "platform.vendor" TAB
                      "platform.version" TAB
                      "platform.profile" TAB

                      "device.name" TAB
                      "device.vendor" TAB
                      "device.version" TAB
                      "device.profile" TAB
                      "driver.version" TAB

                      "maze.size" TAB
                      "maze.seed" TAB

                      "time";

struct Benchmark {
    int size;
    int warmup;
    int samples;
    int seed;
    string &name;
    Platform &platform;
    Device &device;
};

void printResult(ostream &stream, Benchmark &benchmark, cl_uint seed, cl_ulong time);

}; // namespace benchmark
