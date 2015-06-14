#include "timer.h"
#include "IDT.h"
#include "screen.h"
#include "asm.h"
#include "panic.h"
#include "sched.h"
#include "Heap.h"

#include "TimerReg.h"

#define TIMER_FREQ   1193182

static uint8 _tick = 0;

static void on_timer(IrqRegs* regs);

void timer_init(uint32 frequency)
{
    SetIrqHandler(IDT_IRQ0, &on_timer);

    uint16 divisor = TIMER_FREQ / frequency; 
    uint8 l = (divisor & 0xff);
    uint8 h = (divisor >> 8);

    outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
    outb(TIMER_CNTR0, l);
    outb(TIMER_CNTR0, h);
}

uint8 timer_ticks()
{
    return _tick;
}

static void on_timer(IrqRegs* regs)
{
    _tick++;

    Schedule(regs);
}
