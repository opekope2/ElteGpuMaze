#define WALL_TOP 0x01u
#define WALL_RIGHT 0x02u
#define WALL_BOTTOM 0x04u
#define WALL_LEFT 0x08u
#define DEBUG 0x80u

#define VERTEX_INVALID UINT_MAX

typedef uint Vertex;
typedef uint4 Neighbors;
typedef uint2 Edge;

Neighbors getNeighbors(uint w, uint h, uint id) {
    uint left = id % w == 0 ? VERTEX_INVALID : id - 1;
    uint right = id % w == w - 1 ? VERTEX_INVALID : id + 1;
    uint top = id < w ? VERTEX_INVALID : id - w;
    uint bottom = id >= w * (h - 1) ? VERTEX_INVALID : id + w;

    return (Neighbors)(left, right, top, bottom);
}

uint weight(uint seed, Vertex a, Vertex b) {
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

// TODO parallel
kernel void seqPrim(uint width,
                    uint height,
                    uint seed,
                    global Vertex *cheapestEdge,
                    global uchar *unexplored,
                    global Vertex *heap,
                    global Vertex *lookup,
                    global uint *priorities,
                    global uchar *mazeData) {
    uint n = width * height;
    Heap h = {0, heap, lookup, priorities}; // TODO Fibonacci heap

    for (uint i = 0; i < n; i++)
        heapInsert(&h, i, UINT_MAX);

    Vertex startVertex = 0;
    heapDecrease(&h, startVertex, 0);

    while (h.size > 0) {
        Vertex currentVertex = heapExtract(&h);

        SET_REMOVE(unexplored, currentVertex);

        Neighbors neighbors = getNeighbors(width, height, currentVertex);
        for (uint i = 0; i < 4; i++) {
            Vertex neighbor = neighbors[i];
            if (neighbor == VERTEX_INVALID)
                continue;

            uint w = weight(seed, currentVertex, neighbor);
            if (SET_CONTAINS(unexplored, neighbor) && w < heapPriority(&h, neighbor)) {
                cheapestEdge[neighbor] = currentVertex;
                heapDecrease(&h, neighbor, w);
            }
        }
    }

    for (Vertex i = 0; i < n; i++) {
        Vertex j = cheapestEdge[i];
        if (j == VERTEX_INVALID)
            continue;

        Vertex u = min(i, j), v = max(i, j);

        if (u == v - 1) { // Horizontal
            mazeData[u] &= ~WALL_RIGHT;
            mazeData[v] &= ~WALL_LEFT;
        } else { // Vertical
            mazeData[u] &= ~WALL_BOTTOM;
            mazeData[v] &= ~WALL_TOP;
        }
    }
}

kernel void render(global uchar *mazeData, write_only image2d_t tex) {
    int x = get_global_id(0);
    int y = get_global_id(1);
    int w = get_global_size(0);

    write_imageui(tex, (int2)(x, y), (uint4)(mazeData[y * w + x], 0, 0, 0));
}
