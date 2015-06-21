#include "sched.h"
#include "heap.h"
#include "panic.h"
#include "asm.h"
#include "screen.h"
#include <stand.h>
#include <queue.h>

/* 4K stack */
#define STACK_SIZE  0x1000

typedef struct {
    uint32 esp, ebp, ebx, esi, edi, eflags;
    IrqRegs regs;
    uint32* stack;
    uint32 id;
} ThreadContext;

typedef struct ThreadNode {
    bool kern;
    ThreadContext ctx;
    LIST_ENTRY(ThreadNode) __node__;
} ThreadNode;

typedef struct ThreadList {
    uint32 count;
    ThreadNode* cursor;
    LIST_HEAD(__list__, ThreadNode) __list__;
} ThreadList;

static ThreadList _threadList = { 0, NULL };
static int _maxID = 0;

static void OnThreadExit(void);

extern void SwitchContext(ThreadContext* old, ThreadContext* next);
extern void SwitchUserContext(ThreadContext* old, ThreadContext* next);
extern void LoadContext(ThreadContext* next);

void InitThreading(void)
{
    _threadList.count = 0;
    _threadList.cursor = NULL;
    LIST_INIT(&_threadList.__list__);

    /* Main thread id is 0 */
    ThreadNode* node = heap_calloc(sizeof(ThreadNode));
    if (node == NULL)
        panic("InitThreading() OOM");
    node->kern = true;
    node->ctx.id = _maxID++;

    _threadList.count++;
    _threadList.cursor = node;
    LIST_INSERT_HEAD(&_threadList.__list__, node, __node__);
}

void ThreadCreate(ThreadRoutine fn, void* arg, bool isKernel)
{
    __asm__ volatile ("cli");

    void* mem = NULL;
    if (isKernel)
        mem = heap_calloc(sizeof(ThreadNode) + STACK_SIZE);
    else
        mem = (void*)0x1000;

    if (mem == NULL)
        panic("ThreadCreate() OOM");

    /* Memory Layout
     *
     *             low                                              high
     * memory:      | fn     | OnThreadExit  | arg   | 0xdeadc0de    |
     *                ^        ^
     * scheduler:     esp      esp+4
     *                         ^               ^
     * thread:                 esp             esp+4
     *
     * */
    uint32* stack = mem + sizeof(ThreadNode) + STACK_SIZE;
    *--stack = 0xdeadc0de;
    *--stack = (uint32)arg;
    *--stack = (uint32)OnThreadExit;
    *--stack = (uint32)fn;

    ThreadNode* node = mem;
    node->kern = isKernel;
    node->ctx.esp = (uint32)stack;
    node->ctx.ebp = (uint32)stack;
    node->ctx.eflags = (1 << 9); // set IF flag
    node->ctx.stack = stack;
    node->ctx.id = _maxID++;

    LIST_INSERT_AFTER(_threadList.cursor, node, __node__);
    _threadList.count++;

    __asm__ volatile ("sti");
}

uint32 ThreadID(void)
{
    return _threadList.cursor->ctx.id;
}

void Schedule(IrqRegs* regs)
{
    ThreadNode* old = _threadList.cursor;
    ThreadNode* tmp = old;
    ThreadNode* next = NULL;

    next = LIST_NEXT(tmp, __node__);
    if (next == NULL)
        next = LIST_FIRST(&_threadList.__list__);

    if (!next->kern) {
        next->kern = true;
        _threadList.cursor = next;
        SwitchUserContext(&old->ctx, &next->ctx);
    } else {
        _threadList.cursor = next;
        SwitchContext(&old->ctx, &next->ctx);
    }
}

static void OnThreadExit(void)
{
    __asm__ volatile ("cli");

    ThreadNode* old = _threadList.cursor;
    ThreadNode* next = LIST_NEXT(old, __node__);
    if (next == NULL)
        next = LIST_FIRST(&_threadList.__list__);

    LIST_REMOVE(old, __node__);
    heap_release(old);
    _threadList.count--;
    _threadList.cursor = next;

    LoadContext(&next->ctx);
    /*NOTREACHED*/
}

uint32 GetThreadCount(void)
{
    return _threadList.count;
}
