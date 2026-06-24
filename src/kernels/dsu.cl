typedef uint dsu_size_t;
typedef uint weight_t;

typedef struct Edge {
    vertex_t u, v;
    weight_t w;
} Edge;

typedef struct DSU {
    dsu_size_t n;
    global dsu_size_t *size;
    global vertex_t *parent;
} DSU;

void dsu_init(DSU *dsu) {
    global dsu_size_t *size = dsu->size;
    global vertex_t *parent = dsu->parent;
    dsu_size_t n = dsu->n;

    for (dsu_size_t i = 0; i < n; i++) {
        size[i] = 1;
        parent[i] = i;
    }
}

vertex_t dsu_find(DSU *dsu, vertex_t v) {
    global vertex_t *parent = dsu->parent;
    vertex_t root = v;

    while (parent[root] != root)
        root = parent[root];

    for (vertex_t cur = v, par; cur != root; cur = par) {
        par = parent[cur];
        parent[cur] = root;
    }

    return root;
}

uint dsu_union(DSU *dsu, vertex_t u, vertex_t v) {
    u = dsu_find(dsu, u);
    v = dsu_find(dsu, v);

    if (u == v)
        return 0;

    if (dsu->size[u] < dsu->size[v]) {
        vertex_t tmp = u;
        u = v;
        v = tmp;
    }

    dsu->parent[v] = u;
    dsu->size[u] += dsu->size[v];
    return 1;
}
