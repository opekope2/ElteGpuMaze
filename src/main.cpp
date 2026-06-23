#include "benchmark.hpp"
#include "maze.hpp"
#include "maze_manager.hpp"
#include "maze_state.hpp"
#include "util/cl.hpp"
#include "util/glfw.hpp"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <cstdlib>
#include <format>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(GUI)
#include <GLFW/glfw3.h>
#include <epoxy/gl.h>
#endif

using namespace std;
using namespace cl;

void dumpPlatformsAndDevices() {
    std::vector<Platform> platforms;
    Platform::get(&platforms);

    for (auto &&p : platforms) {
        cout << "Platform: " << p.getInfo<CL_PLATFORM_NAME>() << endl;
        std::vector<Device> devices;
        p.getDevices(CL_DEVICE_TYPE_ALL, &devices);
        for (auto &&d : devices)
            cout << "  Device: " << d.getInfo<CL_DEVICE_NAME>() << endl;
    }
}

Platform getPlatform(string platformName) {
    std::vector<Platform> platforms;
    Platform::get(&platforms);

    for (auto &&platform : platforms) {
        if (platform.getInfo<CL_PLATFORM_NAME>() == platformName)
            return platform;
    }

    throw runtime_error(format("No such platform: {}", platformName));
}

Device getDevice(Platform &platform, string deviceName) {
    std::vector<Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    for (auto &&device : devices) {
        if (device.getInfo<CL_DEVICE_NAME>() == deviceName)
            return device;
    }

    throw runtime_error(format("No such device: {}", deviceName));
}

string getEnvOr(const char *name, const char *def) {
    char *env = getenv(name);
    return string(env == nullptr ? def : env);
}

int main(int argc, char **argv) {
    try {
        string platformName = getEnvOr("PLATFORM", ""), deviceName = getEnvOr("DEVICE", ""), benchmarkName = getEnvOr("BENCHMARK", "");
        bool useDefaults = platformName == "" || deviceName == "";
#if defined(GUI)
        bool gui = benchmarkName == "";
#else
        bool gui = false;
#endif
        if (platformName == "list" && deviceName == "list")
            return dumpPlatformsAndDevices(), 0;

#if defined(GUI)
        optional<GlfwWindow> win;
        if (gui) {
            win = Glfw::instance().createWindow(1440, 900, argv[0], nullptr, nullptr);
            glfwMakeContextCurrent(*win);
        }
#endif

        Platform platform = useDefaults ? Platform::getDefault() : getPlatform(platformName);
        Device dev = useDefaults ? Device::getDefault() : getDevice(platform, deviceName);
        auto props = getContextProperties(platform, !gui);
        Context ctx(dev, props.data());
        CommandQueue q(ctx, dev, CL_QUEUE_PROFILING_ENABLE);

#if defined(GUI)
        if (gui) {
            GlMazeState state(ctx);
            state.seed(6 * 7), state.size(32, 32);
            MazeManager manager(ctx, q, state);

            mazeGui(*win, ctx, manager);
            return 0;
        }
#endif

        if (benchmarkName == "list") {
            MazeState state(ctx);
            MazeManager manager(ctx, q, state);

            for (auto generator : manager.generators())
                cout << generator->name() << endl;
            for (auto solver : manager.solvers())
                cout << solver->name() << endl;
            return 0;
        }

        benchmark::Benchmark benchmark{
            stoi(getEnvOr("BENCHMARK_SIZE", "256")),
            stoi(getEnvOr("BENCHMARK_WARMUP", "256")),
            stoi(getEnvOr("BENCHMARK_SAMPLES", "256")),
            stoi(getEnvOr("BENCHMARK_SEED_START", "0")),
            benchmarkName,
            platform,
            dev};

        MazeState state(ctx);
        state.seed(benchmark.seed), state.size(benchmark.size, benchmark.size);
        MazeManager manager(ctx, q, state);

        for (auto generator : manager.generators()) {
            if (generator->name() != benchmarkName)
                continue;

            manager.generator(generator);
            benchmarkMazeGenerator(manager, benchmark);
            return 0;
        }

        for (auto solver : manager.solvers()) {
            if (solver->name() != benchmarkName)
                continue;

            manager.startSolving(solver);
            benchmarkMazeSolver(manager, benchmark);
            return 0;
        }

        throw runtime_error("No such benchmark: " + benchmarkName);
    } catch (const BuildError &e) {
        cerr << format("OpenCL error: {} ({})", e.what(), e.err()) << endl;

        for (auto &&[dev, log] : e.getBuildLog()) {
            cerr << format("Build log for {}:\n{}", dev.getInfo<CL_DEVICE_NAME>(), log) << endl;
        }
    } catch (const Error &e) {
        cerr << format("OpenCL error: {} ({})", e.what(), e.err()) << endl;
    } catch (const exception &e) {
        cerr << format("Fatal error: {}", e.what()) << endl;
    }

    return 1;
}
