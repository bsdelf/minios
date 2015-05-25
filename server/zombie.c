#include <stand.h>

int Main()
{
    uint32 a = 0;
    while (true) {
        ///*
        __asm__ volatile("movl %0, %%eax"
                         :
                         : "r" (a)
                         : "%eax"
                        );
        __asm__ volatile("int $0x80");
        __asm__ volatile("cli");
        //*/
        if (++a > 200)
            a = 0;
    }
    return 0;
}
