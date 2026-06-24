kernel void boruvka(dsu_size_t n,
                    uint m,
                    global dsu_size_t *dsu_size,
                    global vertex_t *dsu_parent,
                    global uint *minout,
                    const global Edge *e,
                    global maze_data_t *maze_data) {
    DSU dsu = {n, dsu_size, dsu_parent};
    dsu_init(&dsu);
    dsu_size_t comp = n;
    while (comp > 1) {
        for (dsu_size_t i = 0; i < n; i++)
            minout[i] = -1;
        for (uint i = 0; i < m; i++) {
            Edge edge = e[i];
            vertex_t u = dsu_find(&dsu, edge.u);
            vertex_t v = dsu_find(&dsu, edge.v);
            if (u == v)
                continue;
            if (!~minout[u] || edge.w < e[minout[u]].w)
                minout[u] = i;
            if (!~minout[v] || edge.w < e[minout[v]].w)
                minout[v] = i;
        }
        for (dsu_size_t _ = 0; _ < n; _++) {
            uint i = minout[_];
            if (~i) {
                Edge edge = e[i];
                if (dsu_union(&dsu, edge.u, edge.v)) {
                    comp--;
                    deleteWall(maze_data, edge.u, edge.v);
                }
            }
        }
    }
}
