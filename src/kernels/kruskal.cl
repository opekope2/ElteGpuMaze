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
        deleteWall(mazeData, edge.u, edge.v);
    }
}
