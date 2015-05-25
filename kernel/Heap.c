#include "Heap.h"
#include "panic.h"
#include <stand.h>

#ifdef USERSPACE
#include <stdio.h>
#else
#define printf(...)
#endif

#pragma pack(push, 1)
typedef struct Chunk {
    // header (payload size)
    uint32 size;
    // payload
    struct Chunk* prev;
    struct Chunk* next;
} Chunk; // 4+4+4=12B
#pragma pack(pop)

#define CHUNK_HEADER_SIZE (sizeof(uint32))
#define CHUNK_PAYLOAD(chunk) ((void*)&(chunk->prev))

typedef struct {
    uint32 total;
    uint32 avail;
    Chunk* first;
} HeapStats;

static HeapStats _stats = { 0, 0, NULL };

void HeapInit(uint32 addr, uint32 size)
{
    if (size < sizeof(Chunk)) {
        return;
    }

    Chunk* chunk = (Chunk*)addr;
    chunk->size = size - CHUNK_HEADER_SIZE;
    chunk->next = NULL;
    chunk->prev = NULL;

    _stats.total = size;
    _stats.avail = chunk->size;
    _stats.first = chunk;
}

void* Malloc(uint32 size, bool align, uint32* paddr)
{
    if (size < sizeof(Chunk))
        size = sizeof(Chunk);

    if (size > _stats.avail) return NULL;

    // find first suitable chunk
    Chunk* chunk = _stats.first;
    while (chunk->size < size) {
        chunk = chunk->next;
        // fragment
        if (chunk == NULL) return NULL;
    }

    // update linkage
    const uint32 resetChunkSize = chunk->size - size;
    if (resetChunkSize >= sizeof(Chunk)) {
        // split
        Chunk* rest = (Chunk*)(CHUNK_PAYLOAD(chunk) + size);
        rest->size = resetChunkSize - CHUNK_HEADER_SIZE;
        rest->prev = chunk->prev;
        rest->next = chunk->next;

        chunk->size = size;
        if (chunk->prev != NULL)
            chunk->prev->next = rest;
        else
            _stats.first = rest;
        if (chunk->next != NULL)
            chunk->next->prev = rest;

        _stats.avail -= size + CHUNK_HEADER_SIZE;
    } else {
        // whole
        if (chunk->prev != NULL)
            chunk->prev->next = chunk->next;
        else
            _stats.first = chunk->next;
        if (chunk->next != NULL)
            chunk->next->prev = chunk->prev;

        _stats.avail -= chunk->size;
    }

    return CHUNK_PAYLOAD(chunk);
}

void* Calloc(uint32 size, bool align, uint32* paddr)
{
    void* mem = Malloc(size, align, paddr);
    if (mem != NULL)
        bzero(mem, size);
    return mem;
}

void Free(void* mem)
{
    Chunk* chunk = (Chunk*)(mem - CHUNK_HEADER_SIZE);

    printf("free: %u\n", chunk->size);

    _stats.avail += chunk->size;

    if (_stats.first == NULL) {
        chunk->prev = NULL;
        chunk->next = NULL;
        _stats.first = chunk;
        return;
    }

    Chunk* pos = _stats.first;
    while (pos < chunk) {
        printf(".\n");
        pos = pos->next;
        // end & append
        if (pos == NULL) {
            chunk->prev = pos;
            chunk->next = NULL;
            pos->next = chunk;
            goto merge;
        }
    }
    // insert: ...=>chunk=>pos=>...
    chunk->prev = pos->prev;
    chunk->next = pos;
    if (pos->prev != NULL) {
        pos->prev->next = chunk;
    } else {
        _stats.first = chunk;
    }
    pos->prev = chunk;

merge:
    // merge ---<o---
    for (Chunk* prev = chunk->prev; prev != NULL; ) {
        printf("<? %p - %p %u\n", (CHUNK_PAYLOAD(prev) + prev->size), (void*)chunk, prev->size);

        if ((CHUNK_PAYLOAD(prev) + prev->size) != (void*)chunk)
            break;

        printf("<\n");

        prev->size += CHUNK_HEADER_SIZE + chunk->size;
        prev->next = chunk->next;

        if (chunk->next != NULL)
            chunk->next->prev = prev;

        _stats.avail += CHUNK_HEADER_SIZE;

        chunk = prev;
        prev = chunk->prev;
    }
    // merge ---o>---
    for (Chunk* next = chunk->next; next != NULL; ) {
        printf(">? %p - %p %u\n", (CHUNK_PAYLOAD(chunk) + chunk->size), next, chunk->size);

        if ((CHUNK_PAYLOAD(chunk) + chunk->size) != (void*)next)
            break;

        printf(">\n");

        chunk->size += CHUNK_HEADER_SIZE + next->size;
        chunk->next = next->next;

        if (next->next != NULL)
            next->next->prev = chunk;

        _stats.avail += CHUNK_HEADER_SIZE;

        next = chunk->next;
    }

    printf("avail: %u\n", _stats.avail);
}

uint32 KHeapAvail(void)
{
    return _stats.avail;
}

uint32 KHeapTotal(void)
{
    return _stats.total;
}

void PrintKHeap(void)
{
    int i = 0;
    for (Chunk* chunk = _stats.first; chunk != NULL; chunk = chunk->next) {
        printf("%d:  %p %u\n", i++, chunk, chunk->size);
    }
}
