#pragma once

#include <CL/cl_platform.h>

typedef cl_uint dsu_size_t;
typedef cl_uint dsu_vertex_t;
typedef cl_uint weight_t;

typedef struct Edge {
    dsu_vertex_t u, v;
    weight_t w;
} Edge;
