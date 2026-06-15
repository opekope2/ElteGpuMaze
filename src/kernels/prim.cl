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
