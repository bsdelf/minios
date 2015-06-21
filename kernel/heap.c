#include "heap.h"
#include "panic.h"
#include <stand.h>

#ifdef USERSPACE
#include <stdio.h>
#else
#define printf(...)
#endif

#pragma pack(push, 1)
typedef struct chunk {
    // header (payload size)
    uint32 size;
    // payload
    struct chunk* prev;
    struct chunk* next;
} chunk_t; // 4+4+4=12B
#pragma pack(pop)

#define CHUNK_HEADER_SIZE (sizeof(uint32))
#define CHUNK_PAYLOAD(chunk) ((void*)&(chunk->prev))

struct {
    uint32 occup;
    uint32 avail;
    chunk_t* first;
} _stats = { 0, 0, NULL };

void heap_init(uint32 va, uint32 size)
{
    if (size < sizeof(chunk_t)) {
        return;
    }

    chunk_t* chunk = (chunk_t*)va;
    chunk->size = size - CHUNK_HEADER_SIZE;
    chunk->next = NULL;
    chunk->prev = NULL;

    _stats.occup = size;
    _stats.avail = chunk->size;
    _stats.first = chunk;
}

void* heap_alloc(uint32 size)
{
    if (size < sizeof(chunk_t))
        size = sizeof(chunk_t);

    if (size > _stats.avail) return NULL;

    // find first suitable chunk
    chunk_t* chunk = _stats.first;
    while (chunk->size < size) {
        chunk = chunk->next;
        // fragment
        if (chunk == NULL) return NULL;
    }

    // update linkage
    const uint32 resetChunkSize = chunk->size - size;
    if (resetChunkSize >= sizeof(chunk_t)) {
        // split
        chunk_t* rest = (chunk_t*)(CHUNK_PAYLOAD(chunk) + size);
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

void* heap_calloc(uint32 size)
{
    void* va = heap_alloc(size);
    if (va != NULL)
        bzero(va, size);
    return va;
}

void heap_release(void* va)
{
    chunk_t* chunk = (chunk_t*)(va - CHUNK_HEADER_SIZE);

    printf("heap_release: %u\n", chunk->size);

    _stats.avail += chunk->size;

    if (_stats.first == NULL) {
        chunk->prev = NULL;
        chunk->next = NULL;
        _stats.first = chunk;
        return;
    }

    chunk_t* pos = _stats.first;
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
    // merge forward ---<o---
    for (chunk_t* prev = chunk->prev; prev != NULL; ) {
        printf("<? %p - %p %u\n", (CHUNK_PAYLOAD(prev) + prev->size), (void*)chunk, prev->size);

        // give up if there is a gap
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
    // merge backward ---o>---
    for (chunk_t* next = chunk->next; next != NULL; ) {
        printf(">? %p - %p %u\n", (CHUNK_PAYLOAD(chunk) + chunk->size), next, chunk->size);

        // give up if there is a gap
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

uint32 heap_avail(void)
{
    return _stats.avail;
}

uint32 heap_occup(void)
{
    return _stats.occup;
}

void heap_status(void)
{
    int i = 0;
    for (chunk_t* chunk = _stats.first; chunk != NULL; chunk = chunk->next) {
        printf("%d:  %p %u\n", i++, chunk, chunk->size);
    }
}
