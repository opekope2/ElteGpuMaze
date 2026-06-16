#pragma once

#include <CL/cl_platform.h>
#include <algorithm>

#define WALL_TOP 0x01u
#define WALL_RIGHT 0x02u
#define WALL_BOTTOM 0x04u
#define WALL_LEFT 0x08u

#define SEARCH_EXPLORED 0x10u
#define SEARCH_FRONTIER 0x20u
#define SEARCH_PATH 0x40u

#define DEBUG 0x80u

typedef cl_uint vertex_t;
typedef cl_uchar maze_data_t;

template <typename vertex_t>
cl_uint weight(cl_uint seed, cl_uint stride, vertex_t a, vertex_t b) {
    cl_uint x = std::min(a, b);
    cl_uint y = std::max(a, b);

    cl_uint hash = seed;

    // xxHash primes
    hash ^= (x % stride) * 0x9e3779b1;
    hash ^= (x / stride) * 0x85ebca77;
    hash ^= (y % stride) * 0xc2b2ae3d;
    hash ^= (y / stride) * 0x27d4eb2f;

    // xxHash avalanche
    hash ^= hash >> 15;
    hash *= 0x85ebca77;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae3d;
    hash ^= hash >> 16;

    return hash;
}
