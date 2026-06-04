#include "../gen/shaders.hpp"
#include "util/gl.hpp"
#include <epoxy/gl.h>

#define VERTEX_COUNT (4 * 2)

using namespace std;

extern float fullQuadVerts[VERTEX_COUNT];

class MazeRenderer {
private:
    GlVertexArray vao;
    GlBuffer vbo;
    GlProgram program;

public:
    MazeRenderer()
        : vao(createVertexArray()),
          vbo(createBuffer()),
          program(createShaderProgram(reinterpret_cast<char *>(maze_vert), reinterpret_cast<char *>(maze_frag))) {
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0); // vec2 pos
        glEnableVertexAttribArray(0);

        glNamedBufferData(vbo, sizeof(fullQuadVerts), fullQuadVerts, GL_STATIC_DRAW);
    }

    void render(int w, int h, GlTexture &data) {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, data);

        glUniform1i(glGetUniformLocation(program, "data"), 0);
        glUniform2i(glGetUniformLocation(program, "res"), w, h);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, VERTEX_COUNT);
    }
};
