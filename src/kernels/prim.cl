#define VERTEX_INVALID UINT_MAX

// TODO parallel
kernel void seqPrim(uint width,
                    uint height,
                    uint seed,
                    global vertex_t *cheapestEdge,
                    global uchar *unexplored,
                    global vertex_t *heap,
                    global vertex_t *lookup,
                    global uint *priorities,
                    maze_data_buffer_t mazeData) {
    uint n = width * height;
    Heap h = {0, heap, lookup, priorities}; // TODO Fibonacci heap

    for (uint i = 0; i < n; i++)
        heapInsert(&h, i, UINT_MAX);

    vertex_t startVertex = 0;
    heapDecrease(&h, startVertex, 0);

    while (h.size > 0) {
        vertex_t currentVertex = heapExtract(&h);

        SET_REMOVE(unexplored, currentVertex);

        uint4 neighbors = getNeighbors(width, height, currentVertex);
        for (uint i = 0; i < 4; i++) {
            vertex_t neighbor = neighbors[i];
            if (neighbor == VERTEX_INVALID)
                continue;

            uint w = weight(seed, currentVertex, neighbor);
            if (SET_CONTAINS(unexplored, neighbor) && w < heapPriority(&h, neighbor)) {
                cheapestEdge[neighbor] = currentVertex;
                heapDecrease(&h, neighbor, w);
            }
        }
    }

    for (vertex_t i = 0; i < n; i++) {
        vertex_t j = cheapestEdge[i];
        if (j == VERTEX_INVALID)
            continue;

        vertex_t u = min(i, j), v = max(i, j);

        if (u == v - 1) { // Horizontal
            mazeData[u] &= ~WALL_RIGHT;
            mazeData[v] &= ~WALL_LEFT;
        } else { // Vertical
            mazeData[u] &= ~WALL_BOTTOM;
            mazeData[v] &= ~WALL_TOP;
        }
    }
}
