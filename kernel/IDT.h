#ifndef IDT_H
#define IDT_H

#include <types.h>

/*
 * Size of IDT table
 */
#define NIDT    256     /* 32 reserved, 0x80 syscall, most are h/w */
#define NRSVIDT 32      /* reserved entries for cpu exceptions */

/*
 * Entries in the Interrupt Descriptor Table (IDT)
 */
#define IDT_DE      0   /* #DE: Divide Error */
#define IDT_DB      1   /* #DB: Debug */
#define IDT_NMI     2   /* Nonmaskable External Interrupt */
#define IDT_BP      3   /* #BP: Breakpoint */
#define IDT_OF      4   /* #OF: Overflow */
#define IDT_BR      5   /* #BR: Bound Range Exceeded */
#define IDT_UD      6   /* #UD: Undefined/Invalid Opcode */
#define IDT_NM      7   /* #NM: No Math Coprocessor */
#define IDT_DF      8   /* #DF: Double Fault */
#define IDT_FPUGP   9   /* Coprocessor Segment Overrun */
#define IDT_TS      10  /* #TS: Invalid TSS */
#define IDT_NP      11  /* #NP: Segment Not Present */
#define IDT_SS      12  /* #SS: Stack Segment Fault */
#define IDT_GP      13  /* #GP: General Protection Fault */
#define IDT_PF      14  /* #PF: Page Fault */
#define IDT_MF      16  /* #MF: FPU Floating-Point Error */
#define IDT_AC      17  /* #AC: Alignment Check */
#define IDT_MC      18  /* #MC: Machine Check */
#define IDT_XF      19  /* #XF: SIMD Floating-Point Exception */
// 15, 17~31 reserved
#define IDT_IRQ0    32
#define IDT_IRQ1    33
#define IDT_IRQ2    34
#define IDT_IRQ3    35
#define IDT_IRQ4    36
#define IDT_IRQ5    37
#define IDT_IRQ6    38
#define IDT_IRQ7    39
#define IDT_IRQ8    40
#define IDT_IRQ9    41
#define IDT_IRQ10   42
#define IDT_IRQ11   43
#define IDT_IRQ12   44
#define IDT_IRQ13   45
#define IDT_IRQ14   46
#define IDT_IRQ15   47

#define IDT_IO_INTS NRSVIDT /* Base of IDT entries for I/O interrupts. */
#define IDT_SYSCALL 0x80    /* System Call Interrupt Vector */

#pragma pack(push, 1)

typedef struct {
    uint16 limit;
    uint32 base;
} IDTPtr;

/* No error code with privilege transition
 *
 *  ------------   -> ss:esp from tss
 *      | old ss
 *  ------------
 *    old esp
 *  ------------
 *   old eflags
 *  ------------
 *      | old cs
 *  ------------
 *    old eip
 *  ------------  -> stack top
 *
 */

/* No error code without privilege transition
 *
 *  ------------   -> ss:esp
 *   old eflags
 *  ------------
 *      | old cs
 *  ------------
 *    old eip
 *  ------------  -> stack top
 *
 */

typedef struct {
    uint32 gs, fs, es, ds;
    uint32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32 eip, cs, eflags;
    uint32 useresp, ss;
} SyscallRegs;

typedef struct {
    uint32 gs, fs, es, ds;
    uint32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32 intno;
    uint32 err_code;
    uint32 eip, cs, eflags;
    uint32 useresp, ss;
} IsrRegs;

typedef struct {
    uint32 gs, fs, es, ds;
    uint32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32 intno;
    uint32 eip, cs, eflags;
    uint32 useresp, ss;
} IrqRegs;

#pragma pack(pop)

typedef void (*IsrHandler)(IsrRegs);
typedef void (*IrqHandler)(IrqRegs*);

void InitIDT(void);

void IsrRouter(IsrRegs regs);
void IrqRouter(IrqRegs* regs);
void SyscallRouter(SyscallRegs* regs);

void SetIsrHandler(uint8 intno, IsrHandler h);
void SetIrqHandler(uint8 intno, IrqHandler h);

#endif
