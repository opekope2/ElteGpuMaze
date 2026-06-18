typedef Edge bitonic_data_t;

kernel void bitonicSwap(dsu_size_t stride, dsu_size_t count, global bitonic_data_t *e) {
    dsu_size_t low = get_global_id(0);
    dsu_size_t pair = low ^ stride;

    if (pair <= low)
        return;

    bool inc = (low & count) == 0;

    if ((inc && e[low].w > e[pair].w) || (!inc && e[low].w < e[pair].w)) {
        bitonic_data_t tmp = e[low];
        e[low] = e[pair];
        e[pair] = tmp;
    }
}
