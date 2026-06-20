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
#include <GLFW/glfw3.h>
#include <epoxy/gl.h>
#include <format>
#include <iostream>
#include <vector>

using namespace std;
using namespace cl;

void generateMaze(MazeManager *manager) {
    MazeGenerator *generator = manager->generator();
    MazeState &state = manager->state();
    CommandQueue &q = manager->queue();
    std::vector<Event> events;

    generator->generate(q, state, events);
    q.finish();

    cl_ulong generateNs = getProfilingTimeNs(events);
    cl_ulong generateMs = generateNs / 1'000'000;
    cout << format("Generated {}x{} maze using {} in {}ms/{}ns", state.width(), state.height(), generator->name(), generateMs, generateNs) << endl;
}

void updateTitle(GLFWwindow *win, MazeManager *manager) {
    MazeGenerator *generator = manager->generator();
    MazeSolver *solver = manager->solver();
    MazeState &state = manager->state();

    string title = format("{} [{}x{}@{}]", generator->name(), state.width(), state.height(), state.seed());
    if (manager->solving())
        title += format(" | {}", solver->name());
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
        generateMaze(manager), manager->resetSolver(true);

    updateTitle(window, manager);
}

void mazeGui(GlfwWindow &win, Context &ctx, CommandQueue &q) {
    MazeRenderer renderer(ctx);

    MazeState state(ctx, 32, 32, 6 * 7);
    MazeManager manager(ctx, q, state);

    generateMaze(&manager);
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
