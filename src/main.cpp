#include "maze_generator.hpp"
#include "maze_manager.hpp"
#include "maze_renderer.hpp"
#include "maze_solver.hpp"
#include "maze_state.hpp"
#include "util/cl.hpp"
#include "util/gl.hpp"
#include "util/glfw.hpp"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <epoxy/gl.h>
#include <format>
#include <iostream>
#include <vector>

using namespace std;
using namespace cl;

cl_ulong generateMaze(MazeManager *manager) {
    MazeGenerator *generator = manager->generator();
    MazeState &state = manager->state();
    CommandQueue &q = manager->queue();
    std::vector<Event> events;

    q.enqueueAcquireGLObjects(&state.glObjs());
    generator->generate(q, state, events);
    generator->renderMazeData(q, state, events);
    q.enqueueReleaseGLObjects(&state.glObjs());
    q.finish();

    return getProfilingTimeNs(events);
}

void generateMazeAndUpdateTitle(GLFWwindow *win, MazeManager *manager) {
    MazeGenerator *generator = manager->generator();
    MazeState &state = manager->state();

    cl_ulong generateNs = generateMaze(manager);
    cl_ulong generateMs = generateNs / 1'000'000;
    cout << format("Generated {}x{} maze using {} in {}ms/{}ns", state.width(), state.height(), generator->name(), generateMs, generateNs) << endl;

    string title = format("{} [{}x{}@{}]", generator->name(), state.width(), state.height(), state.seed());
    glfwSetWindowTitle(win, title.c_str());
}

// FIXME only this method can correctly manage the internal state of MazeManager.
void handleInput(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto *manager = static_cast<MazeManager *>(glfwGetWindowUserPointer(window));
    auto &state = manager->state();

    cl_uint dw = 0, dh = 0, seed = 0;
    bool regenerate = false;

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

    if (key == GLFW_KEY_Q && action != GLFW_RELEASE)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (seed)
        state.seed(seed + state.seed()), regenerate = true;
    if (dw || dh)
        state.resize(dw, dh), regenerate = true;

    if (regenerate)
        generateMazeAndUpdateTitle(window, manager), manager->resetSolver(true);
}

void maze(GlfwWindow &win, Context &ctx, CommandQueue &q) {
    MazeRenderer renderer;

    MazeState state(ctx, 32, 32, 6 * 7);
    MazeManager manager(ctx, q, state);

    generateMazeAndUpdateTitle(win, &manager);

    glfwSetWindowUserPointer(win, &manager);
    glfwSetKeyCallback(win, handleInput);

    while (!glfwWindowShouldClose(win)) {
        int w, h;
        glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);

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

int main(int argc, char **argv) {
    try {
        auto win = Glfw::instance().createWindow(1440, 900, argv[0], nullptr, nullptr);
        glfwMakeContextCurrent(win);

        // TODO handle multiple platforms, multiple devices
        Platform platform = Platform::getDefault();
        Device dev = Device::getDefault();
        auto props = getContextProperties(platform);
        Context ctx(dev, props.data());
        CommandQueue q(ctx, dev, CL_QUEUE_PROFILING_ENABLE);

        maze(win, ctx, q);

        return 0;
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
