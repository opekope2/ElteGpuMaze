#define FRONTIER(c) ((c & (SEARCH_EXPLORED | SEARCH_FRONTIER)) == SEARCH_FRONTIER)
#define EXPAND(c) ((c & (SEARCH_EXPLORED | SEARCH_FRONTIER)) == 0)
#define EXPAND_FROM(c) ((c & (SEARCH_EXPLORED | SEARCH_FRONTIER)) == (SEARCH_EXPLORED | SEARCH_FRONTIER))
#define HAS_NO_WALL(c, w) ((c & w) == 0)

#define BFS_ORIGIN_END 0x80u

kernel void init(vertex2_t frontiers, global maze_data_t *mazeData) {
    uint id = get_global_id(0);
    mazeData[frontiers[id]] |= SEARCH_EXPLORED | SEARCH_FRONTIER | (id * BFS_ORIGIN_END);
}

kernel void init_wavefront(vertex2_t frontiers, global uint *wavefrontSize, global vertex_t *wavefront, global maze_data_t *mazeData) {
    uint id = get_global_id(0);
    if (id == 0)
        *wavefrontSize = get_global_size(0);
    mazeData[frontiers[id]] |= SEARCH_FRONTIER | (id * BFS_ORIGIN_END);
    wavefront[id] = frontiers[id];
}

kernel void expand(global vertex_t *parent, global maze_data_t *mazeData) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint w = get_global_size(0);
    uint h = get_global_size(1);

    vertex_t v = x + y * w;
    vertex4_t neighbors = getNeighbors(w, h, v);
    maze_data_t vertexData = mazeData[v];

    if (EXPLORED(vertexData))
        return;

    for (int i = 0; i < 4; i++) {
        vertex_t neighbor = neighbors[i];
        if (neighbor == VERTEX_INVALID)
            continue;

        maze_data_t neighborData = mazeData[neighbor];

        if (HAS_NO_WALL(vertexData, 1 << i) && EXPAND_FROM(neighborData)) {
            mazeData[v] |= SEARCH_FRONTIER | (neighborData & BFS_ORIGIN_END);
            parent[v] = neighbor;
            break;
        }
    }
}

kernel void expand_wavefront(uint width,
                             uint height,
                             const global uint *prevFrontierSize,
                             const global vertex_t *prevFrontiers,
                             global uint *frontierSize,
                             global vertex_t *frontiers,
                             global vertex_t *parent,
                             global maze_data_t *mazeData) {
    uint id = get_global_id(0);
    if (id >= *prevFrontierSize)
        return;

    vertex_t v = prevFrontiers[id];
    vertex4_t neighbors = getNeighbors(width, height, v);
    maze_data_t vertexData = mazeData[v];
    mazeData[v] |= SEARCH_EXPLORED;
    mazeData[v] &= ~SEARCH_FRONTIER;

    vertex_t newFrontiers[4];
    uint newFrontiersSize = 0;

    for (int i = 0; i < 4; i++) {
        vertex_t neighbor = neighbors[i];
        if (neighbor == VERTEX_INVALID)
            continue;

        maze_data_t neighborData = mazeData[neighbor];

        if (HAS_NO_WALL(vertexData, 1 << i) && EXPAND(neighborData)) {
            mazeData[neighbor] |= SEARCH_FRONTIER | (vertexData & BFS_ORIGIN_END);
            parent[neighbor] = v;
            newFrontiers[newFrontiersSize++] = neighbor;
        }
    }

    uint index = atomic_add(frontierSize, newFrontiersSize);
    for (int i = 0; i < newFrontiersSize; i++)
        frontiers[i + index] = newFrontiers[i];
}

kernel void mark(global maze_data_t *mazeData) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint w = get_global_size(0);

    vertex_t v = x + y * w;
    if (FRONTIER(mazeData[v]))
        mazeData[v] |= SEARCH_EXPLORED;
    else if (EXPAND_FROM(mazeData[v]))
        mazeData[v] &= ~SEARCH_FRONTIER;
}

kernel void vege_van(uint width, uint height, const global maze_data_t *mazeData, global vertex_t *meet) {
    uint n = width * height;
    maze_data_t vege = mazeData[n - 1];
    if (vege & SEARCH_EXPLORED)
        *meet = n - 1;
}

kernel void vege_van_2(uint width, uint height, const global maze_data_t *mazeData, global vertex_t *meet) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    vertex_t v = x + y * width;

    vertex4_t neighbors = getNeighbors(width, height, v);
    maze_data_t vertexData = mazeData[v];

    if (UNEXPLORED(vertexData))
        return;

    for (int i = 0; i < 4; i++) {
        vertex_t neighbor = neighbors[i];
        if (neighbor == VERTEX_INVALID)
            continue;

        maze_data_t neighborData = mazeData[neighbor];

        if (HAS_NO_WALL(vertexData, 1 << i) && EXPLORED(neighborData) && ((vertexData ^ neighborData) & BFS_ORIGIN_END))
            atomic_cmpxchg(meet, VERTEX_INVALID, v);
    }
}

kernel void wege_wan(uint width,
                     uint height,
                     const global uint *frontierSize,
                     const global vertex_t *frontiers,
                     const global maze_data_t *mazeData,
                     global vertex_t *meet) {
    uint id = get_global_id(0);
    if (id >= *frontierSize)
        return;

    uint n = width * height;
    if (frontiers[id] == n - 1)
        *meet = n - 1;
}

kernel void wege_wan_2(uint width,
                       uint height,
                       const global uint *frontierSize,
                       const global vertex_t *frontiers,
                       const global maze_data_t *mazeData,
                       global vertex_t *meet) {
    uint id = get_global_id(0);
    if (id >= *frontierSize)
        return;

    vertex_t v = frontiers[id];
    vertex4_t neighbors = getNeighbors(width, height, v);
    maze_data_t vertexData = mazeData[v];

    for (int i = 0; i < 4; i++) {
        vertex_t neighbor = neighbors[i];
        if (neighbor == VERTEX_INVALID)
            continue;

        maze_data_t neighborData = mazeData[neighbor];

        if (HAS_NO_WALL(vertexData, 1 << i) && EXPLORED(neighborData) && ((vertexData ^ neighborData) & BFS_ORIGIN_END))
            atomic_cmpxchg(meet, VERTEX_INVALID, v);
    }
}

kernel void drawPath(uint width, uint height, const global vertex_t *meet, const global vertex_t *parent, global maze_data_t *mazeData) {
    uint id = get_global_id(0);
    vertex2_t start = (vertex2_t)(*meet, VERTEX_INVALID);
    if (UNEXPLORED(mazeData[start.x]))
        return;

    maze_data_t vertexData = mazeData[start.x];
    vertex4_t neighbors = getNeighbors(width, height, start.x);

    for (int i = 0; i < 4; i++) {
        vertex_t neighbor = neighbors[i];
        if (neighbor == VERTEX_INVALID)
            continue;

        maze_data_t neighborData = mazeData[neighbor];

        if (HAS_NO_WALL(vertexData, 1 << i) && EXPLORED(neighborData) && ((vertexData ^ neighborData) & BFS_ORIGIN_END))
            start.y = neighbor;
    }

    for (vertex_t v = start[id]; ~v; v = parent[v])
        mazeData[v] |= SEARCH_PATH;
}
