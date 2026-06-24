kernel void renderMazeData(const global maze_data_t *mazeData, write_only image2d_t tex) {
    int x = get_global_id(0);
    int y = get_global_id(1);
    int w = get_global_size(0);

    write_imageui(tex, (int2)(x, y), (uint4)(mazeData[x + y * w], 0, 0, 0));
}
