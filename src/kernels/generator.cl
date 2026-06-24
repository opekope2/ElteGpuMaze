kernel void generateEdges(uint seed, global Edge *e) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint w = get_global_size(0);
    uint h = get_global_size(1);
    uint o = (w - 1) * h;

    vertex_t v = x + y * w;

    uint u1 = x - 1 + y * (w - 1);
    uint u2 = x + (y - 1) * w;

    Edge e1 = {v - 1, v, weight(seed, w, v - 1, v)};
    Edge e2 = {v - w, v, weight(seed, w, v - w, v)};

    if (x != 0)
        e[u1] = e1;
    if (y != 0)
        e[u2 + o] = e2;
}
