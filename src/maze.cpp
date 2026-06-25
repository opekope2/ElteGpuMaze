#include "maze.hpp"
#include "benchmark.hpp"
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
#include <cstdint>
#include <format>
#include <iostream>
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
void step(MazeManager *manager) {
    if (manager->stepSolve()) {
        manager->solver()->showPath(manager->queue(), manager->state());
        cl_ulong solveNs = manager->solveNs();
        cl_ulong solveMs = solveNs / 1'000'000;
        cout << format("Solved {}x{} maze using {} in {}ms/{}ns", manager->state().width(), manager->state().height(), manager->solver()->name(), solveMs, solveNs) << endl;
        manager->resetSolver(false);
    }
}

void updateTitle(GLFWwindow *win, MazeManager *manager) {
    MazeGenerator *generator = manager->generator();
    MazeSolver *solver = manager->solver();
    MazeState &state = manager->state();

    string title = format("{} [{}x{}@{}]", generator->name(), state.width(), state.height(), state.seed());
    if (manager->solving())
        title += format(" | {} (x{})", solver->name(), manager->solvingSpeed());
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
    if (key == GLFW_KEY_S && action != GLFW_RELEASE && !manager->solving() && !manager->solved())
        manager->startSolving(manager->parallel2WayBfs());

    if (key == GLFW_KEY_SPACE && action != GLFW_RELEASE)
        speed = !speed;
    if (key == GLFW_KEY_LEFT_BRACKET && action != GLFW_RELEASE)
        speed -= AMOUNT(mods);
    if (key == GLFW_KEY_RIGHT_BRACKET && action != GLFW_RELEASE)
        speed += AMOUNT(mods);
    if (key == GLFW_KEY_PERIOD && action != GLFW_RELEASE && !speed)
        manager->solvingSpeed(AMOUNT(mods)), step(manager), manager->solvingSpeed(0);

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

        step(&manager);
    }
}
#endif

void benchmarkMazeGenerator(MazeManager &manager, benchmark::Benchmark &benchmark, bool warmup) {
    MazeState &state = manager.state();

    cl_ulong ns = generateMaze(&manager);
    if (!warmup)
        benchmark::printResult(cout, benchmark, state.seed(), ns);

    state.seed(state.seed() + 1);
}

void benchmarkMazeGenerator(MazeManager &manager, benchmark::Benchmark &benchmark) {
    cout << benchmark::HEADER << endl;

    for (int i = 0; i < benchmark.warmup; i++)
        benchmarkMazeGenerator(manager, benchmark, true);
    for (int i = 0; i < benchmark.samples; i++)
        benchmarkMazeGenerator(manager, benchmark, false);
}

void benchmarkMazeSolver(MazeManager &manager, benchmark::Benchmark &benchmark, MazeSolver *solver, bool warmup) {
    MazeState &state = manager.state();

    generateMaze(&manager);

    manager.resetSolver(true);
    manager.startSolving(solver);
    while (!manager.stepSolve())
        ;

    cl_ulong ns = manager.solveNs();
    if (!warmup)
        benchmark::printResult(cout, benchmark, state.seed(), ns);

    state.seed(state.seed() + 1);
}

void benchmarkMazeSolver(MazeManager &manager, benchmark::Benchmark &benchmark) {
    cout << benchmark::HEADER << endl;

    MazeSolver *solver = manager.solver();

    for (int i = 0; i < benchmark.warmup; i++)
        benchmarkMazeSolver(manager, benchmark, solver, true);
    for (int i = 0; i < benchmark.samples; i++)
        benchmarkMazeSolver(manager, benchmark, solver, false);
}
