#define HAS_WALL(c, w) ((c & w) == w)

uint distanceFromEnd(uint width, uint height, vertex_t v) {
    uint x = v % width;
    uint y = v / width;

    return (width - 1 - x) + (height - 1 - y);
}

kernel void init(uint width,
                 uint height,
                 global uint *nDiscovered,
                 global vertex_t *heap,
                 global vertex_t *lookup,
                 global uint *priorities,
                 global uint *distanceFromStart,
                 global maze_data_t *mazeData) {
    vertex_t startVertex = 0;

    mazeData[startVertex] |= SEARCH_FRONTIER;

    Heap discovered = {0, heap, lookup, priorities};
    heapInsert(&discovered, startVertex, distanceFromEnd(width, height, startVertex));
    distanceFromStart[startVertex] = 0;

    *nDiscovered = discovered.size;
}

kernel void aStar(uint width,
                  uint height,
                  global uint *nDiscovered,
                  global vertex_t *heap,
                  global vertex_t *lookup,
                  global uint *priorities,
                  global uint *distanceFromStart,
                  global vertex_t *parent,
                  global maze_data_t *mazeData,
                  global vertex_t *meet) {
    uint n = width * height;
    vertex_t end = n - 1;
    Heap discovered = {*nDiscovered, heap, lookup, priorities};

    if (discovered.size == 0)
        return;

    vertex_t currentVertex = heapExtract(&discovered);
    maze_data_t vertexData = mazeData[currentVertex];
    vertexData &= ~SEARCH_FRONTIER;
    vertexData |= SEARCH_EXPLORED;
    mazeData[currentVertex] = vertexData;

    if (currentVertex == end) {
        *meet = currentVertex;
        return;
    }

    vertex4_t neighbors = getNeighbors(width, height, currentVertex);
    for (int i = 0; i < 4; i++) {
        vertex_t neighbor = neighbors[i];
        if (HAS_WALL(vertexData, 1 << i) || neighbor == VERTEX_INVALID)
            continue;

        uint newDistanceFromStart = distanceFromStart[currentVertex] + 1;
        if (newDistanceFromStart < distanceFromStart[neighbor]) {
            parent[neighbor] = currentVertex;
            distanceFromStart[neighbor] = newDistanceFromStart;

            newDistanceFromStart += distanceFromEnd(width, height, neighbor);
            if (heapContains(&discovered, neighbor))
                heapDecrease(&discovered, neighbor, newDistanceFromStart);
            else
                heapInsert(&discovered, neighbor, newDistanceFromStart);

            mazeData[neighbor] |= SEARCH_FRONTIER;
        }
    }

    *nDiscovered = discovered.size;
}

kernel void drawPath(const global vertex_t *meet, const global vertex_t *parent, global maze_data_t *mazeData) {
    for (vertex_t v = *meet; ~v; v = parent[v])
        mazeData[v] |= SEARCH_PATH;
}
