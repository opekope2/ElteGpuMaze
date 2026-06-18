kernel void generateEdges(uint seed, global Edge *e) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint w = get_global_size(0);
    uint h = get_global_size(1);
    uint o = (w - 1) * h;

    dsu_vertex_t v = x + y * w;

    uint u1 = x - 1 + y * (w - 1);
    uint u2 = x + (y - 1) * w;

    Edge e1 = {v - 1, v, weight(seed, w, v - 1, v)};
    Edge e2 = {v - w, v, weight(seed, w, v - w, v)};

    if (x != 0)
        e[u1] = e1;
    if (y != 0)
        e[u2 + o] = e2;
}

kernel void kruskal(dsu_size_t n,
                    uint m,
                    global dsu_size_t *dsuSize,
                    global dsu_vertex_t *dsuParent,
                    global Edge *e,
                    maze_data_buffer_t mazeData) {
    DSU dsu = {n, dsuSize, dsuParent};
    dsu_init(&dsu);

    for (dsu_size_t i = 0; i < m; i++) {
        Edge edge = e[i];
        if (edge.u == VERTEX_INVALID || edge.v == VERTEX_INVALID) {
            // Parallel bitonic merge sort is not stable, so invalid padding edges can get in the middle
            // TODO run parallel stream compaction before Kruskal
            continue;
        }

        dsu_vertex_t u = dsu_find(&dsu, edge.u);
        dsu_vertex_t v = dsu_find(&dsu, edge.v);
        if (u == v)
            continue;

        dsu_union(&dsu, u, v);
        if (edge.u > edge.v)
            edge.u ^= edge.v ^= edge.u ^= edge.v;

        if (edge.u == edge.v - 1) { // Horizontal
            mazeData[edge.u] &= ~WALL_RIGHT;
            mazeData[edge.v] &= ~WALL_LEFT;
        } else { // Vertical
            mazeData[edge.u] &= ~WALL_BOTTOM;
            mazeData[edge.v] &= ~WALL_TOP;
        }
    }

    mazeData[0] |= SEARCH_FRONTIER;
}
