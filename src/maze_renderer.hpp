#pragma once

#if defined(GUI)

#include "../gen/kernels.hpp"
#include "../gen/shaders.hpp"
#include "maze_state.hpp"
#include "util/cl.hpp"
#include "util/gl.hpp"
#include "util/misc.hpp"
#include <CL/opencl.hpp>
#include <epoxy/gl.h>

#define VERTEX_COUNT (4 * 2)

using namespace cl;

extern float fullQuadVerts[VERTEX_COUNT];

class MazeRenderer {
private:
    GlVertexArray _vao;
    GlBuffer _vbo;
    GlProgram _glProgram;
    Program _clProgram;
    KernelFunctor<Buffer, ImageGL> _renderMazeData;

public:
    MazeRenderer(Context &ctx)
        : _vao(createVertexArray()),
          _vbo(createBuffer()),
          _glProgram(createShaderProgram(reinterpret_cast<char *>(maze_vert), reinterpret_cast<char *>(maze_frag))),
          _clProgram(buildProgram(ctx, cl::Program::Sources{XXD_STRING(maze_cl), XXD_STRING(render_cl)})),
          _renderMazeData(_clProgram, "renderMazeData") {
        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0); // vec2 pos
        glEnableVertexAttribArray(0);

        glNamedBufferData(_vbo, sizeof(fullQuadVerts), fullQuadVerts, GL_STATIC_DRAW);
    }

    void renderMazeData(CommandQueue &q, GlMazeState &state) {
        q.enqueueAcquireGLObjects(&state.glObjs());
        _renderMazeData(
            EnqueueArgs(q, NDRange(state.width(), state.height())),
            state.mazeData(),
            state.glImage());
        q.enqueueReleaseGLObjects(&state.glObjs());
        q.finish();
    }

    void render(int w, int h, GlTexture &data) {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(_glProgram);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, data);

        glUniform1i(glGetUniformLocation(_glProgram, "data"), 0);
        glUniform2i(glGetUniformLocation(_glProgram, "res"), w, h);

        glBindVertexArray(_vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, VERTEX_COUNT);
    }
};

#endif
