#include "maze_renderer.hpp"

#if defined(GUI)
float fullQuadVerts[VERTEX_COUNT] = {
    // clang-format off
    -1.0f, 1.0f,
    -1.0f, -1.0f,
    1.0f, -1.0f,
    1.0f, 1.0f,
    // clang-format on
};
#endif
