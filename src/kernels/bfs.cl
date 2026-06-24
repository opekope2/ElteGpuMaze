#define FRONTIER(c) ((c & (SEARCH_EXPLORED | SEARCH_FRONTIER)) == SEARCH_FRONTIER)
#define EXPAND_FROM(c) ((c & (SEARCH_EXPLORED | SEARCH_FRONTIER)) == (SEARCH_EXPLORED | SEARCH_FRONTIER))
#define HAS_NO_WALL(c, w) ((c & w) == 0)

#define BFS_ORIGIN_END 0x80u

kernel void init(uint2 frontiers, maze_data_buffer_t mazeData) {
    uint id = get_global_id(0);
    mazeData[frontiers[id]] |= SEARCH_EXPLORED | SEARCH_FRONTIER | (id * BFS_ORIGIN_END);
}

kernel void expand(global vertex_t *parent, maze_data_buffer_t mazeData) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint w = get_global_size(0);
    uint h = get_global_size(1);

    vertex_t v = x + y * w;
    uint4 neighbors = getNeighbors(w, h, v);
    maze_data_t vertexData = mazeData[v];

    if (EXPLORED(vertexData))
        return;

    for (int i = 0; i < 4; i++) {
        vertex_t neighbor = neighbors[i];
        maze_data_t neighborData = mazeData[neighbor];

        if (HAS_NO_WALL(vertexData, 1 << i) && EXPAND_FROM(neighborData)) {
            mazeData[v] |= SEARCH_FRONTIER | (neighborData & BFS_ORIGIN_END);
            parent[v] = neighbor;
            break;
        }
    }
}

kernel void mark(maze_data_buffer_t mazeData) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint w = get_global_size(0);

    vertex_t v = x + y * w;
    if (FRONTIER(mazeData[v]))
        mazeData[v] |= SEARCH_EXPLORED;
    else if (EXPAND_FROM(mazeData[v]))
        mazeData[v] &= ~SEARCH_FRONTIER;
}

kernel void vege_van(maze_data_buffer_t mazeData, global vertex_t *meet) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint w = get_global_size(0);
    uint h = get_global_size(1);
    vertex_t v = x + y * w;

    uint4 neighbors = getNeighbors(w, h, v);
    maze_data_t vertexData = mazeData[v];

    if (UNEXPLORED(vertexData))
        return;

    for (int i = 0; i < 4; i++) {
        vertex_t neighbor = neighbors[i];
        maze_data_t neighborData = mazeData[neighbor];

        if (HAS_NO_WALL(vertexData, 1 << i) && EXPLORED(neighborData) && ((vertexData ^ neighborData) & BFS_ORIGIN_END))
            atomic_cmpxchg(meet, VERTEX_INVALID, v);
    }
}

kernel void drawPath(uint width, uint height, global vertex_t *meet, global vertex_t *parent, maze_data_buffer_t mazeData) {
    uint id = get_global_id(0);
    uint2 start = (uint2)(*meet, VERTEX_INVALID);
    if (UNEXPLORED(mazeData[start.x]))
        return;

    maze_data_t vertexData = mazeData[start.x];
    uint4 neighbors = getNeighbors(width, height, start.x);

    for (int i = 0; i < 4; i++) {
        vertex_t neighbor = neighbors[i];
        maze_data_t neighborData = mazeData[neighbor];

        if (HAS_NO_WALL(vertexData, 1 << i) && EXPLORED(neighborData) && ((vertexData ^ neighborData) & BFS_ORIGIN_END))
            start.y = neighbor;
    }

    for (vertex_t v = start[id]; ~v; v = parent[v])
        mazeData[v] |= SEARCH_PATH;
}
