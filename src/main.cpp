#include "kernels.hpp"
#include "maze.hpp"
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <CL/opencl.hpp>
#include <EGL/egl.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GLFW/glfw3.h>
#include <exception>
#include <format>
#include <iostream>
#include <stdexcept>
#include <vector>

#define CHECK(v, e)                                                            \
  if (!v)                                                                      \
    throw runtime_error(e " failed");

using namespace std;
using namespace cl;

std::vector<cl_context_properties> getContextProperties(GLFWwindow *w) {
  auto eglCtx = eglGetCurrentContext();
  auto glxCtx = glXGetCurrentContext();

  if (eglCtx != EGL_NO_CONTEXT) {
    return {CL_EGL_DISPLAY_KHR, (cl_context_properties)eglGetCurrentDisplay(),
            CL_GL_CONTEXT_KHR, (cl_context_properties)eglCtx, 0};
  } else if (glxCtx != nullptr) {
    return {CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
            CL_GL_CONTEXT_KHR, (cl_context_properties)glxCtx, 0};
  } else {
    throw runtime_error("Current context not available");
  }
}

int main(int argc, char **argv) {
  try {
    CHECK(glfwInit(), "glfwInit");

    auto w = glfwCreateWindow(800, 480, argv[0], nullptr, nullptr);
    CHECK(w, "glfwCreateWindow");
    glfwMakeContextCurrent(w);

    // TODO handle multiple platforms, multiple devices
    Device dev = Device::getDefault();
    auto props = getContextProperties(w);
    Context ctx(dev, props.data());
    MazeApp maze(dev, ctx);

    while (!glfwWindowShouldClose(w)) {
      maze.run();

      glClear(GL_COLOR_BUFFER_BIT);
      glfwSwapBuffers(w);
      glfwPollEvents();
    }

    glfwDestroyWindow(w);
    glfwTerminate();
    return 0;
  } catch (const Error &e) {
    cerr << format("OpenCL error: {} ({})", e.what(), e.err()) << endl;
  } catch (const exception &e) {
    cerr << format("Fatal error: {}", e.what()) << endl;
  }

  glfwTerminate();
  return 1;
}
