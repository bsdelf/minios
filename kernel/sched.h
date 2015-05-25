#ifndef SCHED_H
#define SCHED_H

#include <types.h>
#include "IDT.h"

typedef int (*ThreadRoutine)(void*);

void InitThreading(void);
void Schedule(IrqRegs* regs);
uint32 GetThreadCount(void);

void ThreadCreate(ThreadRoutine fn, void* arg, bool isKernel);
uint32 ThreadID(void);

#endif
