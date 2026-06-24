#define EXPLORED(c) ((c & SEARCH_EXPLORED) != 0)
#define UNEXPLORED(c) ((c & SEARCH_EXPLORED) == 0)

kernel void clearPath(maze_data_buffer_t mazeData) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint w = get_global_size(0);

    vertex_t v = x + y * w;
    mazeData[v] &= ~SEARCH_PATH;
}
