#define WALL_TOP 0x01u
#define WALL_RIGHT 0x02u
#define WALL_BOTTOM 0x04u
#define WALL_LEFT 0x08u

#define SEARCH_EXPLORED 0x10u
#define SEARCH_FRONTIER 0x20u
#define SEARCH_PATH 0x40u

#define VERTEX_INVALID UINT_MAX
#define NEIGHBOR_INVALID UINT_MAX

typedef uint vertex_t;
typedef uchar maze_data_t;
typedef global maze_data_t *maze_data_buffer_t;

uint weight(uint seed, uint stride, uint a, uint b) {
    uint x = min(a, b);
    uint y = max(a, b);

    uint hash = seed;

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

uint4 getNeighbors(uint w, uint h, uint id) {
    uint left = id % w == 0 ? NEIGHBOR_INVALID : id - 1;
    uint right = id % w == w - 1 ? NEIGHBOR_INVALID : id + 1;
    uint top = id < w ? NEIGHBOR_INVALID : id - w;
    uint bottom = id >= w * (h - 1) ? NEIGHBOR_INVALID : id + w;

    return (uint4)(top, right, bottom, left);
}

void deleteWall(maze_data_buffer_t mazeData, vertex_t a, vertex_t b) {
    vertex_t u = min(a, b), v = max(a, b);

    if (u == v - 1) { // Horizontal
        mazeData[u] &= ~WALL_RIGHT;
        mazeData[v] &= ~WALL_LEFT;
    } else { // Vertical
        mazeData[u] &= ~WALL_BOTTOM;
        mazeData[v] &= ~WALL_TOP;
    }
}
