#define FRONTIER(c) ((c & (SEARCH_EXPLORED | SEARCH_FRONTIER)) == SEARCH_FRONTIER)
#define EXPAND(c) ((c & (SEARCH_EXPLORED | SEARCH_FRONTIER)) == (SEARCH_EXPLORED | SEARCH_FRONTIER))
#define HAS_NO_WALL(c, w) ((c & w) == 0)

kernel void mark(maze_data_buffer_t mazeData) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint w = get_global_size(0);

    vertex_t v = x + y * w;
    if (FRONTIER(mazeData[v]))
        mazeData[v] |= SEARCH_EXPLORED;
}

kernel void expand(global vertex_t *parent, maze_data_buffer_t mazeData) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint w = get_global_size(0);
    uint h = get_global_size(1);

    vertex_t v = x + y * w;
    uint4 neighbors = getNeighbors(w, h, v);
    maze_data_t vertexData = mazeData[v];

    if (EXPAND(vertexData)) {
        for (int i = 0; i < 4; i++) {
            if (HAS_NO_WALL(vertexData, 1 << i) && UNEXPLORED(mazeData[neighbors[i]])) {
                mazeData[neighbors[i]] |= SEARCH_FRONTIER;
                parent[neighbors[i]] = v;
            }
        }
        mazeData[v] &= ~SEARCH_FRONTIER;
    }
}
