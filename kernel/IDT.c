#include "IDT.h"
#include "Segments.h"
#include "screen.h"
#include "asm.h"
#include "env.h"
#include "panic.h"

/* PIC definitions */
#define PIC1_CMD      0x20
#define PIC1_DATA     0x21
#define PIC2_CMD      0xa0
#define PIC2_DATA     0xa1
#define PIC_EOI       0x20    // End-of-Interrupt command code

extern void asm_idt_flush(uint32);

typedef void (*IntCallback)(void);

extern void OnSyscall();

extern void OnEmptyIsr();
extern void OnIsr0();
extern void OnIsr1();
extern void OnIsr2();
extern void OnIsr3();
extern void OnIsr4();
extern void OnIsr5();
extern void OnIsr6();
extern void OnIsr7();
extern void OnIsr8();
extern void OnIsr9();
extern void OnIsr10();
extern void OnIsr11();
extern void OnIsr12();
extern void OnIsr13();
extern void OnIsr14();
extern void OnIsr15();
extern void OnIsr16();
extern void OnIsr17();
extern void OnIsr18();
extern void OnIsr19();
extern void OnIsr20();
extern void OnIsr21();
extern void OnIsr22();
extern void OnIsr23();
extern void OnIsr24();
extern void OnIsr25();
extern void OnIsr26();
extern void OnIsr27();
extern void OnIsr28();
extern void OnIsr29();
extern void OnIsr30();
extern void OnIsr31();

extern void OnIrq0();
extern void OnIrq1();
extern void OnIrq2();
extern void OnIrq3();
extern void OnIrq4();
extern void OnIrq5();
extern void OnIrq6();
extern void OnIrq7();
extern void OnIrq8();
extern void OnIrq9();
extern void OnIrq10();
extern void OnIrq11();
extern void OnIrq12();
extern void OnIrq13();
extern void OnIrq14();
extern void OnIrq15();

static GateDescriptor _idtEntries[NIDT] = { 0 };

static IDTPtr _idtPtr = {
    .limit = sizeof(_idtEntries) - 1,
    .base = (uint32)&_idtEntries
};

static IsrHandler _isrhandles[NIDT] = { NULL };
static IrqHandler _irqhandles[NIDT] = { NULL };

static inline void SetIDTEntry(uint8 num, IntCallback fn, uint16 selector, uint8 type, uint8 dpl)
{
    uint32 offset = (uint32)fn;
    _idtEntries[num].off_low = (offset & 0xffff);
    _idtEntries[num].off_high = ((offset >> 16) & 0xffff);
    _idtEntries[num].selector = selector;
    _idtEntries[num].s = 0;
    _idtEntries[num].type = type;
    _idtEntries[num].dpl = dpl;
    _idtEntries[num].present = 1;
}

void InitIDT(void)
{
    // Remapping the PIC
    OutB(PIC1_CMD, 0x11);
    WaitIO();
    OutB(PIC2_CMD, 0x11);
    WaitIO();
    OutB(PIC1_DATA, 0x20);
    WaitIO();
    OutB(PIC2_DATA, 0x28);
    WaitIO();
    OutB(PIC1_DATA, 0x04);
    WaitIO();
    OutB(PIC2_DATA, 0x02);
    WaitIO();
    OutB(PIC1_DATA, 0x01);
    WaitIO();
    OutB(PIC2_DATA, 0x01);
    WaitIO();
    OutB(PIC1_DATA, 0xff);
    WaitIO();
    OutB(PIC2_DATA, 0xff);
    WaitIO();

    /*
     * 0-32: used by the CPU to report conditions, both normal and error
     * 255: used by system calls
     */
    for (int i = 0; i < NIDT; ++i) {
        SetIDTEntry(i, OnEmptyIsr, 0x08, INT_GATE, SEL_KPL);
    }
    // ISR
    SetIDTEntry(IDT_DE,    OnIsr0,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_DB,    OnIsr1,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_NMI,   OnIsr2,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_BP,    OnIsr3,  0x08, INT_GATE, SEL_UPL);
    SetIDTEntry(IDT_OF,    OnIsr4,  0x08, INT_GATE, SEL_UPL);
    SetIDTEntry(IDT_BR,    OnIsr5,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_UD,    OnIsr6,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_NM,    OnIsr7,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_DF,    OnIsr8,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_FPUGP, OnIsr9,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_TS,    OnIsr10, 0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_NP,    OnIsr11, 0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_SS,    OnIsr12, 0x08, INT_GATE, SEL_KPL);
    //SetIDTEntry(IDT_GP,    OnIsr13, 0x08, INT_GATE, SEL_KPL);
    //SetIDTEntry(IDT_PF,    OnIsr14, 0x08, INT_GATE, SEL_KPL);

    SetIDTEntry(IDT_MF,    OnIsr16, 0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_AC,    OnIsr17, 0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_MC,    OnIsr18, 0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_XF,    OnIsr19, 0x08, INT_GATE, SEL_KPL);

    // IRQ
    SetIDTEntry(IDT_IRQ0,  OnIrq0,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_IRQ1,  OnIrq1,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_IRQ2,  OnIrq2,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_IRQ3,  OnIrq3,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_IRQ4,  OnIrq4,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_IRQ5,  OnIrq5,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_IRQ6,  OnIrq6,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_IRQ7,  OnIrq7,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_IRQ8,  OnIrq8,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_IRQ9,  OnIrq9,  0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_IRQ10, OnIrq10, 0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_IRQ11, OnIrq11, 0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_IRQ12, OnIrq12, 0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_IRQ13, OnIrq13, 0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_IRQ14, OnIrq14, 0x08, INT_GATE, SEL_KPL);
    SetIDTEntry(IDT_IRQ15, OnIrq15, 0x08, INT_GATE, SEL_KPL);

    // syscall
    SetIDTEntry(IDT_SYSCALL, OnSyscall, 0x08, INT_GATE, SEL_UPL);

    asm_idt_flush((uint32)&_idtPtr);
}

void IsrRouter(IsrRegs regs)
{
    if (_isrhandles[regs.intno] != NULL) {
        _isrhandles[regs.intno](regs);
    } else {
        panic("unhandled ISR!\n"
              "int number: %d\n"
              "error code: %d",
              regs.intno, regs.err_code);
    }
}

void IrqRouter(IrqRegs* regs)
{
    // send EOI
    if (regs->intno >= 40)
        OutB(PIC2_CMD, PIC_EOI);
    OutB(PIC1_CMD, PIC_EOI);

    if (_irqhandles[regs->intno] != NULL) {
        _irqhandles[regs->intno](regs);
    } else {
        panic("unhandled IRQ\n"
              "int number %d",
              regs->intno);
    }
}

void SyscallRouter(SyscallRegs* regs)
{
    uint32 a = regs->eax;

    //DrawString(0, 0, "    ", COLOR_White, COLOR_Black);
    if (a >= 100) {
    } else {
        //DrawUnsigned(FONT_WIDTH*2, 0, a, COLOR_White, COLOR_Black);
    }
}

void SetIsrHandler(uint8 intno, IsrHandler h)
{
    //disable_irq(intno);
    _isrhandles[intno] = h;
}

void SetIrqHandler(uint8 intno, IrqHandler h)
{
    _irqhandles[intno] = h;
}
