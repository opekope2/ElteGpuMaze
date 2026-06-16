#pragma once

#include <CL/cl_platform.h>

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
