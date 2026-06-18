kernel void boruvka(dsu_size_t n,
                    uint m,
                    global dsu_size_t *dsu_size,
                    global dsu_vertex_t *dsu_parent,
                    global uint *minout,
                    global Edge *e,
                    maze_data_buffer_t maze_data) {
    DSU dsu = {n, dsu_size, dsu_parent};
    dsu_init(&dsu);
    dsu_size_t comp = n;
    while (comp > 1) {
        for (dsu_size_t i = 0; i < n; i++)
            minout[i] = -1;
        for (uint i = 0; i < m; i++) {
            dsu_vertex_t u = dsu_find(&dsu, e[i].u);
            dsu_vertex_t v = dsu_find(&dsu, e[i].v);
            if (u == v)
                continue;
            if (!~minout[u] || e[i].w < e[minout[u]].w)
                minout[u] = i;
            if (!~minout[v] || e[i].w < e[minout[v]].w)
                minout[v] = i;
        }
        for (dsu_size_t _ = 0; _ < n; _++) {
            uint i = minout[_];
            // uint3 *ei = (*uint3)(e + i);
            if (~i) {
                if (dsu_union(&dsu, e[i].u, e[i].v)) {
                    comp--;
                    deleteWall(maze_data, e[i].u, e[i].v);
                }
            }
        }
    }

    maze_data[0] |= SEARCH_FRONTIER;
}
