#pragma once

#if defined(GUI)

#include "handle.hpp"
#include "misc.hpp"
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <CL/opencl.hpp>
#include <epoxy/gl.h>
#include <format>

using namespace std;
using namespace cl;

#define GL_DELETE_1(f) \
    inline void f##Wrapper(GLuint id) { f(id); }

#define GL_DELETE_2(f) \
    inline void f##Wrapper(GLuint id) { f(1, &id); }

GL_DELETE_2(glDeleteBuffers);
GL_DELETE_1(glDeleteProgram);
GL_DELETE_1(glDeleteShader);
GL_DELETE_2(glDeleteTextures);
GL_DELETE_2(glDeleteVertexArrays);

using GlBuffer = Handle<GLuint, glDeleteBuffersWrapper>;
using GlProgram = Handle<GLuint, glDeleteProgramWrapper>;
using GlShader = Handle<GLuint, glDeleteShaderWrapper>;
using GlTexture = Handle<GLuint, glDeleteTexturesWrapper>;
using GlVertexArray = Handle<GLuint, glDeleteVertexArraysWrapper>;

inline GlBuffer createBuffer() {
    GLuint id = 0;
    glCreateBuffers(1, &id);
    CHECK(id, glCreateBuffers);

    return GlBuffer(id);
}

inline GlProgram createProgram() {
    GLuint id = glCreateProgram();
    CHECK(id, glCreateProgram);

    return GlProgram(id);
}

template <GLenum type>
inline GlShader createShader() {
    GLuint id = glCreateShader(type);
    CHECK(id, glCreateShader);

    return GlShader(id);
}

template <GLenum target>
inline GlTexture createTexture() {
    GLuint id = 0;
    glCreateTextures(target, 1, &id);
    CHECK(id, glCreateTextures);

    return GlTexture(id);
}

inline GlVertexArray createVertexArray() {
    GLuint id = 0;
    glCreateVertexArrays(1, &id);
    CHECK(id, glCreateVertexArrays);

    return GlVertexArray(id);
}

template <GLenum type>
inline GlShader createShader(string shaderType, const char *source) {
    auto shader = createShader<type>();
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE)
        return shader;

    GLchar log[1024];
    glGetShaderInfoLog(shader, 1024, NULL, log);

    throw runtime_error(format("{} shader compilation error:\n{}", shaderType, log));
}

inline GlProgram createShaderProgram(const GLchar *vertexShader, const GLchar *fragmentShader) {
    auto vs = createShader<GL_VERTEX_SHADER>("Vertex", vertexShader);
    auto fs = createShader<GL_FRAGMENT_SHADER>("Fragment", fragmentShader);

    auto program = createProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_TRUE)
        return program;

    GLchar log[1024];
    glGetProgramInfoLog(program, 1024, NULL, log);

    throw runtime_error(format("Shader program linking error:\n{}", log));
}

#endif
