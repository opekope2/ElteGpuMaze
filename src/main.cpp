#include "maze.hpp"
#include "maze_manager.hpp"
#include "maze_state.hpp"
#include "util/gl.hpp"
#include "util/glfw.hpp"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <epoxy/gl.h>
#include <format>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <vector>

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

Platform getPlatform(char *platformName) {
    std::vector<Platform> platforms;
    Platform::get(&platforms);

    for (auto &&platform : platforms) {
        if (platform.getInfo<CL_PLATFORM_NAME>() == platformName)
            return platform;
    }

    throw runtime_error(format("No such platform: {}", platformName));
}

Device getDevice(Platform &platform, char *deviceName) {
    std::vector<Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    for (auto &&device : devices) {
        if (device.getInfo<CL_DEVICE_NAME>() == deviceName)
            return device;
    }

    throw runtime_error(format("No such device: {}", deviceName));
}

int main(int argc, char **argv) {
    try {
        char *platformName = getenv("PLATFORM"), *deviceName = getenv("DEVICE"), *benchmark = getenv("BENCHMARK");
        bool useDefaults = platformName == nullptr || deviceName == nullptr;
        bool gui = benchmark == nullptr;
        if (useDefaults && gui)
            dumpPlatformsAndDevices();

        optional<GlfwWindow> win;
        if (gui) {
            win = Glfw::instance().createWindow(1440, 900, argv[0], nullptr, nullptr);
            glfwMakeContextCurrent(*win);
        }

        Platform platform = useDefaults ? Platform::getDefault() : getPlatform(platformName);
        Device dev = useDefaults ? Device::getDefault() : getDevice(platform, deviceName);
        auto props = getContextProperties(platform, !gui);
        Context ctx(dev, props.data());
        CommandQueue q(ctx, dev, CL_QUEUE_PROFILING_ENABLE);

        if (gui) {
            GlMazeState state(ctx);
            state.seed(6 * 7), state.size(32, 32);
            MazeManager manager(ctx, q, state);

            mazeGui(*win, ctx, manager);
            return 0;
        }

        MazeState state(ctx);
        state.seed(6 * 7), state.size(state.minWidth(), state.minHeight());
        MazeManager manager(ctx, q, state);

        if (benchmark == string("list")) {
            for (auto generator : manager.generators())
                cout << generator->name() << endl;
            for (auto solver : manager.solvers())
                cout << solver->name() << endl;
            return 0;
        }

        for (auto generator : manager.generators()) {
            if (generator->name() != benchmark)
                continue;

            manager.generator(generator);
            mazeBenchmarkGenerator(manager);
            return 0;
        }

        for (auto solver : manager.solvers()) {
            if (solver->name() != benchmark)
                continue;

            manager.startSolving(solver);
            mazeBenchmarkSolver(manager);
            return 0;
        }

        throw runtime_error(format("No such generator or solver to benchmark: {}", benchmark));
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
