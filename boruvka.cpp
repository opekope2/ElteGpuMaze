#include <iostream>
#include <vector>

struct Edge {
    int u, v, w;
};

class DisjointSet {
    std::vector<int> parent, rank;

public:
    DisjointSet(int n) : parent(n), rank(n, 1) {
        for (int i = 0; i < n; i++)
            parent[i] = i;
    }

    int find(int v) {
        if (parent[v] == v)
            return v;
        return parent[v] = find(parent[v]);
    }

    int unio(int u, int v) {
        u = find(u);
        v = find(v);
        if (u == v)
            return 0;
        if (rank[u] < rank[v])
            std::swap(u, v);
        parent[v] = u;
        rank[u] += rank[v];
        return 1;
    }
};

void boruvka(int n, const std::vector<Edge> &e) {
    DisjointSet dsu(n);
    std::vector<int> minout(n);
    int comp = n;
    while (comp > 1) {
        minout.assign(n, -1);
        for (int i = 0; i < e.size(); i++) {
            int u = dsu.find(e[i].u);
            int v = dsu.find(e[i].v);
            if (u == v)
                continue;
            if (!~minout[u] || e[i].w < e[minout[u]].w)
                minout[u] = i;
            if (!~minout[v] || e[i].w < e[minout[v]].w)
                minout[v] = i;
        }
        for (int i : minout) {
            if (~i) {
                int u = dsu.find(e[i].u);
                int v = dsu.find(e[i].v);
                if (dsu.unio(u, v)) {
                    comp--;
                    std::cout << e[i].u << ' ' << e[i].v << std::endl;
                }
            }
        }
    }
}

int main() {
    int n, m;
    std::cin >> n >> m;
    std::vector<Edge> e(m);
    for (Edge &edge : e)
        std::cin >> edge.u >> edge.v >> edge.w;
    boruvka(n, e);
}
