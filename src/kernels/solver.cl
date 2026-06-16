#define UNEXPLORED(c) ((c & SEARCH_EXPLORED) == 0)

kernel void clearPath(maze_data_buffer_t mazeData) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint w = get_global_size(0);

    vertex_t v = x + y * w;
    mazeData[v] &= ~SEARCH_PATH;
}

kernel void drawPath(uint width, uint height, global vertex_t *parent, maze_data_buffer_t mazeData) {
    vertex_t last = width * height - 1;
    if (UNEXPLORED(mazeData[last]))
        return;
    for (vertex_t v = last; v != 0; v = parent[v])
        mazeData[v] |= SEARCH_PATH;
    mazeData[0] |= SEARCH_PATH;
}
