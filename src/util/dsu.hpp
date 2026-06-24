#pragma once

#include "maze.hpp"
#include <CL/cl_platform.h>

typedef cl_uint dsu_size_t;
typedef cl_uint weight_t;

typedef struct Edge {
    vertex_t u, v;
    weight_t w;
} Edge;
