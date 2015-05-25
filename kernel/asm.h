#ifndef ASM_H
#define ASM_H

#include <types.h>

/*
asm ( assembler template 
           : output operands                  // optional
           : input operands                   // optional
           : list of clobbered registers      // optional
           );
*/

static inline void Hlt(void)
{
    __asm__ volatile("hlt");
}

static inline void Nop(void)
{
    __asm__ volatile("nop");
}

static inline void HltLoop(void)
{
    while (true) {
        __asm__ volatile("hlt");
    }
}

static inline void OutB(uint16 port, uint8 data)
{
    __asm__ volatile(
        "outb %0, %1"
        :
        : "a"(data), "Nd"(port));
}

static inline uint8 InB(uint16 port)
{
    uint8 data;
    __asm__ volatile(
        "inb %1, %0"
        : "=a"(data)
        : "Nd"(port));
    return data;
}

static inline void WaitIO(void)
{
    __asm__ volatile(
        "outb %%al, $0x80"
        :
        : "a"(0));
}

#endif
