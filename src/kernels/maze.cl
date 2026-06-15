#define WALL_TOP 0x01u
#define WALL_RIGHT 0x02u
#define WALL_BOTTOM 0x04u
#define WALL_LEFT 0x08u

#define SEARCH_EXPLORED 0x10u
#define SEARCH_FRONTIER 0x20u
#define SEARCH_PATH 0x40u

#define DEBUG 0x80u

#define NEIGHBOR_INVALID UINT_MAX

typedef uchar maze_data_t;
typedef global maze_data_t *maze_data_buffer_t;

uint weight(uint seed, uint a, uint b) {
    uint hash = seed;

    // xxHash primes
    hash ^= min(a, b) * 0x9e3779b1;
    hash ^= max(a, b) * 0x85ebca77;

    // xxHash avalanche
    hash ^= hash >> 15;
    hash *= 0x85ebca77;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae3d;
    hash ^= hash >> 16;

    return hash;
}

uint4 getNeighbors(uint w, uint h, uint id) {
    uint left = id % w == 0 ? NEIGHBOR_INVALID : id - 1;
    uint right = id % w == w - 1 ? NEIGHBOR_INVALID : id + 1;
    uint top = id < w ? NEIGHBOR_INVALID : id - w;
    uint bottom = id >= w * (h - 1) ? NEIGHBOR_INVALID : id + w;

    return (uint4)(top, right, bottom, left);
}

kernel void render(maze_data_buffer_t mazeData, write_only image2d_t tex) {
    int x = get_global_id(0);
    int y = get_global_id(1);
    int w = get_global_size(0);

    write_imageui(tex, (int2)(x, y), (uint4)(mazeData[y * w + x], 0, 0, 0));
}
