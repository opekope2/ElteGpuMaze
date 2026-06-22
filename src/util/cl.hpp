#pragma once

#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <stdexcept>
#include <utility>
#include <vector>

#if defined(GUI)

#if defined(__linux__)
#include <epoxy/egl.h>
#include <epoxy/glx.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#endif

inline cl_ulong getProfilingTimeNs(std::vector<cl::Event> &events) {
    cl_ulong ns = 0;
    for (auto &&e : events)
        ns += e.getProfilingInfo<CL_PROFILING_COMMAND_END>() - e.getProfilingInfo<CL_PROFILING_COMMAND_START>();

    return ns;
}

// Because Program doesn't have a constructor that accepts multiple sources while also builds it
template <typename... Args>
cl::Program buildProgram(Args &&...args) {
    cl::Program program(std::forward<Args>(args)...);
    program.build();
    return program;
}

inline std::vector<cl_context_properties> getContextProperties(cl::Platform &p, bool headless) {
#if defined(GUI)
    if (headless)
#endif
        return {CL_CONTEXT_PLATFORM, (cl_context_properties)p(),
                0};

#if defined(GUI)

#if defined(__linux__)
    auto eglCtx = eglGetCurrentContext();
    auto glxCtx = glXGetCurrentContext();

    if (eglCtx != EGL_NO_CONTEXT) {
        return {CL_EGL_DISPLAY_KHR, (cl_context_properties)eglGetCurrentDisplay(),
                CL_GL_CONTEXT_KHR, (cl_context_properties)eglCtx,
                CL_CONTEXT_PLATFORM, (cl_context_properties)p(),
                0};
    } else if (glxCtx != nullptr) {
        return {CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
                CL_GL_CONTEXT_KHR, (cl_context_properties)glxCtx,
                CL_CONTEXT_PLATFORM, (cl_context_properties)p(),
                0};
    } else {
        throw std::runtime_error("Current context not available");
    }
#elif defined(_WIN32) || defined(_WIN64)
    return {CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
            CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
            CL_CONTEXT_PLATFORM, (cl_context_properties)p(),
            0};
#else
#error Operating system not supported
#endif

#endif
}
