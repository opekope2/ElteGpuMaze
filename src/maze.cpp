#include "maze.hpp"
#include "maze_generator.hpp"
#include "maze_manager.hpp"
#include "maze_renderer.hpp"
#include "maze_solver.hpp"
#include "maze_state.hpp"
#include "util/cl.hpp"
#include "util/glfw.hpp"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <iostream>
#include <numeric>
#include <vector>

#if defined(GUI)
#include <GLFW/glfw3.h>
#include <epoxy/gl.h>
#endif

using namespace std;
using namespace cl;

cl_ulong generateMaze(MazeManager *manager) {
    MazeGenerator *generator = manager->generator();
    MazeState &state = manager->state();
    CommandQueue &q = manager->queue();
    std::vector<Event> events;

    generator->generate(q, state, events);
    q.finish();

    return getProfilingTimeNs(events);
}

#if defined(GUI)
void updateTitle(GLFWwindow *win, MazeManager *manager) {
    MazeGenerator *generator = manager->generator();
    MazeSolver *solver = manager->solver();
    MazeState &state = manager->state();

    string title = format("{} [{}x{}@{}]", generator->name(), state.width(), state.height(), state.seed());
    if (manager->solving())
        title += format(" | {} [x{}]", solver->name(), manager->solvingSpeed());
    glfwSetWindowTitle(win, title.c_str());
}

// FIXME only this method can correctly manage the internal state of MazeManager.
void handleInput(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto *manager = static_cast<MazeManager *>(glfwGetWindowUserPointer(window));
    auto &state = manager->state();

    cl_uint dw = 0, dh = 0, seed = 0;
    bool regenerate = false;
    uint8_t speed = manager->solvingSpeed();

    if (key == GLFW_KEY_LEFT && action != GLFW_RELEASE)
        dw -= AMOUNT(mods);
    if (key == GLFW_KEY_RIGHT && action != GLFW_RELEASE)
        dw += AMOUNT(mods);
    if (key == GLFW_KEY_UP && action != GLFW_RELEASE)
        dh += AMOUNT(mods);
    if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE)
        dh -= AMOUNT(mods);

    if (key == GLFW_KEY_EQUAL && action != GLFW_RELEASE)
        seed += AMOUNT(mods);
    if (key == GLFW_KEY_MINUS && action != GLFW_RELEASE)
        seed -= AMOUNT(mods);

    if (key == GLFW_KEY_P && action != GLFW_RELEASE)
        manager->generator(manager->sequentialPrim()), regenerate = true;
    if (key == GLFW_KEY_K && action != GLFW_RELEASE)
        manager->generator(manager->parallelSortedKruskal()), regenerate = true;
    if (key == GLFW_KEY_B && action != GLFW_RELEASE)
        manager->generator(manager->sequentialBoruvka()), regenerate = true;
    if (key == GLFW_KEY_W && action != GLFW_RELEASE && !manager->solving() && !manager->solved())
        manager->startSolving(manager->parallelBfs());

    if (key == GLFW_KEY_SPACE && action != GLFW_RELEASE)
        speed = !speed;
    if (key == GLFW_KEY_LEFT_BRACKET && action != GLFW_RELEASE)
        speed -= AMOUNT(mods);
    if (key == GLFW_KEY_RIGHT_BRACKET && action != GLFW_RELEASE)
        speed += AMOUNT(mods);

    if (key == GLFW_KEY_Q && action != GLFW_RELEASE)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (seed)
        state.seed(seed + state.seed()), regenerate = true;
    if (dw || dh)
        state.resize(dw, dh), regenerate = true;
    if (speed != manager->solvingSpeed())
        manager->solvingSpeed(speed);

    if (regenerate) {
        cl_ulong generateNs = generateMaze(manager);
        cl_ulong generateMs = generateNs / 1'000'000;
        cout << format("Generated {}x{} maze using {} in {}ms/{}ns", state.width(), state.height(), manager->generator()->name(), generateMs, generateNs) << endl;

        manager->resetSolver(true);
    }

    updateTitle(window, manager);
}

void mazeGui(GlfwWindow &win, Context &ctx, MazeManager &manager) {
    MazeRenderer renderer(ctx);

    GlMazeState &state = dynamic_cast<GlMazeState &>(manager.state());
    CommandQueue &q = manager.queue();

    cl_ulong generateNs = generateMaze(&manager);
    cl_ulong generateMs = generateNs / 1'000'000;
    cout << format("Generated {}x{} maze using {} in {}ms/{}ns", state.width(), state.height(), manager.generator()->name(), generateMs, generateNs) << endl;
    updateTitle(win, &manager);

    glfwSetWindowUserPointer(win, &manager);
    glfwSetKeyCallback(win, handleInput);

    while (!glfwWindowShouldClose(win)) {
        int w, h;
        glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);

        renderer.renderMazeData(q, state);
        renderer.render(w, h, state.texture());

        glfwSwapBuffers(win);
        glfwPollEvents();

        if (manager.stepSolve()) {
            cl_ulong solveNs = manager.solveNs();
            cl_ulong solveMs = solveNs / 1'000'000;
            cout << format("Solved {}x{} maze using {} in {}ms/{}ns", state.width(), state.height(), manager.solver()->name(), solveMs, solveNs) << endl;
            manager.resetSolver(false);
        }
    }
}
#endif

void dumpStatsHeader(Platform &platform, Device &device, string benchmark) {
#if defined(__linux__)
    cout << "Target OS: Linux" << endl;
#elif defined(_WIN32) || defined(_WIN64)
    cout << "Target OS: Windows" << endl;
#else
#error Operating system not supported
#endif

    cout << "Platform: " << platform.getInfo<CL_PLATFORM_NAME>()
         << "; Vendor: " << platform.getInfo<CL_PLATFORM_VENDOR>()
         << "; Version: " << platform.getInfo<CL_PLATFORM_VERSION>()
         << "; Profile: " << platform.getInfo<CL_PLATFORM_PROFILE>()
         << endl;
    cout << "Device: " << device.getInfo<CL_DEVICE_NAME>()
         << "; Vendor: " << device.getInfo<CL_DEVICE_VENDOR>()
         << "; Version: " << device.getInfo<CL_DEVICE_VERSION>()
         << "; Profile: " << device.getInfo<CL_DEVICE_PROFILE>()
         << "; Driver version: " << device.getInfo<CL_DRIVER_VERSION>()
         << endl;
    cout << "Benchmark: " << benchmark << endl;
    cout << "Sample size: " << BENCHMARK_SAMPLE_SIZE << endl;
    cout << endl;
    cout << "width\theight\tmin\tq1\tmedian\tmean\tq3\tmax\tstddev" << endl;
}

void dumpStats(std::vector<cl_ulong> stats, cl_uint width, cl_uint height) {
    sort(stats.begin(), stats.end());
    auto n = stats.size();
    auto min = stats[0];
    auto q1 = (stats[n / 4 - 1] + stats[n / 4]) / 2.0;
    auto median = (stats[n / 2 - 1] + stats[n / 2]) / 2.0;
    auto mean = accumulate(stats.begin(), stats.end(), 0.0) / n;
    auto q3 = (stats[n * 3 / 4 - 1] + stats[n * 3 / 4]) / 2.0;
    auto max = stats[n - 1];
    std::vector<double> varHelper;
    for (auto time : stats)
        varHelper.push_back((time - mean) * (time - mean));
    auto var = accumulate(varHelper.begin(), varHelper.end(), 0.0) / varHelper.size();
    auto stddev = sqrt(var);
    cout << format("{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}", width, height, min, q1, median, mean, q3, max, stddev) << endl;
}

void mazeBenchmarkGenerator(MazeManager &manager) {
    std::vector<cl_ulong> ns;
    for (MazeState &state = manager.state(); state.width() <= state.maxWidth() && state.height() <= state.maxHeight(); state.resize(state.width(), state.height())) {
        for (int i = 0; i < BENCHMARK_SAMPLE_SIZE; i++)
            state.seed(i), ns.push_back(generateMaze(&manager));

        dumpStats(ns, state.width(), state.height());
        ns.clear();
    }
}

void mazeBenchmarkSolver(MazeManager &manager) {
    std::vector<cl_ulong> ns;
    auto solver = manager.solver();

    for (MazeState &state = manager.state(); state.width() <= state.maxWidth() && state.height() <= state.maxHeight(); state.resize(state.width(), state.height())) {
        for (int i = 0; i < BENCHMARK_SAMPLE_SIZE; i++) {
            state.seed(i);
            generateMaze(&manager);
            manager.startSolving(solver);

            while (!manager.stepSolve())
                ;

            ns.push_back(manager.solveNs());
            manager.resetSolver(true);
        }

        dumpStats(ns, state.width(), state.height());
        ns.clear();
    }
}
