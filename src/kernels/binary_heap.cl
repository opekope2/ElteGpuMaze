#define HEAP_TYPE uint
#define HEAP_TYPE_INVALID UINT_MAX
#define HEAP_MISSING UINT_MAX

typedef struct Heap {
    uint size;
    global HEAP_TYPE *heap, *lookup;
    global uint *priorities;
} Heap;

#define HEAP_PARENT(i) (i - 1) / 2
#define HEAP_CHILD1(i) 2 * i + 1
#define HEAP_CHILD2(i) 2 * i + 2

void upHeap(Heap *heap, uint i) {
    global HEAP_TYPE *h = heap->heap, *lut = heap->lookup;
    global uint *prio = heap->priorities;

    uint p = HEAP_PARENT(i);
    HEAP_TYPE key = h[i];
    uint priority = prio[i];

    while (i > 0 && priority < prio[p]) {
        // Swap up
        h[i] = h[p];
        prio[i] = prio[p];
        lut[h[p]] = i;

        // Swap down
        h[p] = key;
        prio[p] = priority;
        lut[key] = p;

        i = p;
        p = HEAP_PARENT(i);
    }
}

void downHeap(Heap *heap, uint i) {
    global HEAP_TYPE *h = heap->heap, *lut = heap->lookup;
    global uint *prio = heap->priorities;
    uint size = heap->size;

    uint c1 = HEAP_CHILD1(i), c2 = HEAP_CHILD2(i);
    HEAP_TYPE key = h[i];
    uint priority = prio[i];

    while ((c1 < size && prio[c1] < priority) || (c2 < size && prio[c2] < priority)) {
        uint c = c2 >= size || prio[c1] < prio[c2] ? c1 : c2;

        // Swap up
        h[i] = h[c];
        prio[i] = prio[c];
        lut[h[c]] = i;

        // Swap down
        h[c] = key;
        prio[c] = priority;
        lut[key] = c;

        i = c;
        c1 = HEAP_CHILD1(i), c2 = HEAP_CHILD2(i);
    }
}

void heapInsert(Heap *heap, HEAP_TYPE key, uint priority) {
    uint i = heap->size++;
    heap->heap[i] = key;
    heap->priorities[i] = priority;
    heap->lookup[key] = i;

    upHeap(heap, i);
}

HEAP_TYPE heapExtract(Heap *heap) {
    global HEAP_TYPE *h = heap->heap, *lut = heap->lookup;
    global uint *prio = heap->priorities;

    if (heap->size == 0)
        return HEAP_TYPE_INVALID;

    HEAP_TYPE root = h[0];

    // Move last to root
    h[0] = h[--heap->size];
    prio[0] = prio[heap->size];
    lut[h[0]] = 0;

    // Clear last
    h[heap->size] = HEAP_TYPE_INVALID;
    prio[heap->size] = UINT_MAX;
    lut[root] = HEAP_MISSING;

    downHeap(heap, 0);

    return root;
}

bool heapContains(Heap *heap, HEAP_TYPE key) {
    return heap->lookup[key] != HEAP_MISSING;
}

uint heapPriority(Heap *heap, HEAP_TYPE key) {
    if (!heapContains(heap, key))
        return UINT_MAX;

    uint i = heap->lookup[key];
    return heap->priorities[i];
}

void heapDecrease(Heap *heap, HEAP_TYPE key, uint priority) {
    if (!heapContains(heap, key))
        return;

    uint i = heap->lookup[key];
    heap->priorities[i] = priority;

    upHeap(heap, i);
}
