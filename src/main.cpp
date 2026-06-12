#include "../gen/kernels.hpp"
#include "../gen/shaders.hpp"
#include "maze_generator.hpp"
#include "maze_renderer.hpp"
#include "maze_state.hpp"
#include "prim.hpp"
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

using namespace std;
using namespace cl;

void handleInput(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto *state = static_cast<MazeState *>(glfwGetWindowUserPointer(window));

    cl_uint dw = 0, dh = 0, seed = 0;

    if (key == GLFW_KEY_LEFT && action != GLFW_RELEASE)
        dw--;
    if (key == GLFW_KEY_RIGHT && action != GLFW_RELEASE)
        dw++;
    if (key == GLFW_KEY_UP && action != GLFW_RELEASE)
        dh++;
    if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE)
        dh--;

    if (key == GLFW_KEY_EQUAL && action != GLFW_RELEASE)
        seed++;
    if (key == GLFW_KEY_MINUS && action != GLFW_RELEASE)
        seed--;

    if (seed)
        state->seed(seed + state->seed());
    if (dw || dh)
        state->resize(dw, dh);
}

void generateMaze(MazeGenerator *maze, MazeState &state, CommandQueue &q) {
    q.enqueueAcquireGLObjects(&state.glObjs());
    auto generate = maze->generate(q, state);
    maze->render(q, state);
    q.enqueueReleaseGLObjects(&state.glObjs());
    q.finish();

    auto generateNs = getProfilingTimeNs(generate);
    cout << format("Generated {}x{} maze using {} in {}ms/{}ns", state.width(), state.height(), maze->name(), generateNs / 1000000, generateNs) << endl;
}

void generateMazeAndUpdateTitle(GlfwWindow &win, MazeGenerator *maze, MazeState &state, CommandQueue &q) {
    generateMaze(maze, state, q);

    string title = format("{} [{}x{}@{}]", maze->name(), state.width(), state.height(), state.seed());
    glfwSetWindowTitle(win, title.c_str());
}

void maze(GlfwWindow &win, Context &ctx, CommandQueue &q) {
    PrimCL primCl(ctx);
    SeqPrim seqPrim(ctx, primCl);

    MazeRenderer renderer;

    MazeState state(ctx, 32, 32, 6 * 7);

    MazeGenerator *maze = &seqPrim;
    generateMazeAndUpdateTitle(win, maze, state, q);

    glfwSetWindowUserPointer(win, &state);
    glfwSetKeyCallback(win, handleInput);

    while (!glfwWindowShouldClose(win)) {
        int w, h;
        glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);

        renderer.render(w, h, state.texture());

        glfwSwapBuffers(win);
        glfwPollEvents();

        bool regenerate = false;

        if (state.changed())
            regenerate = true;

        if (glfwGetKey(win, GLFW_KEY_P) != GLFW_RELEASE)
            maze = &seqPrim, regenerate = true;

        if (regenerate)
            generateMazeAndUpdateTitle(win, maze, state, q);
    }
}

int main(int argc, char **argv) {
    try {
        auto win = Glfw::instance().createWindow(800, 480, argv[0], nullptr, nullptr);
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
