#define WALL_TOP 0x1u
#define WALL_RIGHT 0x2u
#define WALL_BOTTOM 0x4u
#define WALL_LEFT 0x8u

// TODO Prim

// TODO Kruskal

// TODO remove once proper generation is added
__kernel void generate(__global uchar *data) {
    int x = get_global_id(0);
    int y = get_global_id(1);
    int w = get_global_size(0);

    uchar walls = (x + y) % 2 ? WALL_TOP | WALL_LEFT : WALL_BOTTOM | WALL_RIGHT;

    data[y * w + x] = walls;
}

__kernel void render(__global uchar *data, __write_only image2d_t tex) {
    int x = get_global_id(0);
    int y = get_global_id(1);
    int w = get_global_size(0);

    write_imageui(tex, (int2)(x, y), (uint4)(data[y * w + x], 0, 0, 0));
}
