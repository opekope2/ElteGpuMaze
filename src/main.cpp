#include "../gen/kernels.hpp"
#include "../gen/shaders.hpp"
#include "maze.hpp"
#include "maze_renderer.hpp"
#include "util/gl.hpp"
#include "util/glfw.hpp"
#include <CL/opencl.hpp>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <epoxy/gl.h>
#include <iostream>

using namespace std;
using namespace cl;

void maze(GlfwWindow &win, Context &ctx, CommandQueue &q) {
    Maze maze(ctx, 16, 16);
    MazeRenderer renderer;

    auto tex = createTexture<GL_TEXTURE_2D>();
    glTextureStorage2D(tex, 1, GL_R8UI, maze.width(), maze.height());
    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    ImageGL img(ctx, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, tex);
    std::vector<Memory> glObjs = {img};

    while (!glfwWindowShouldClose(win)) {
        int w, h;
        glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);

        q.enqueueAcquireGLObjects(&glObjs);
        maze.generateData(q);
        maze.renderData(q, img);
        q.enqueueReleaseGLObjects(&glObjs);
        q.finish();

        renderer.render(w, h, tex);

        glfwSwapBuffers(win);
        glfwPollEvents();
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
        CommandQueue q(ctx, dev);

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
