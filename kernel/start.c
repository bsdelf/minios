#include <stand.h>
#include "asm.h"
#include "env.h"
#include "screen.h"
#include "GDT.h"
#include "IDT.h"
#include "timer.h"
#include "vm.h"
#include "pm.h"
#include "panic.h"
#include "sched.h"
#include "heap.h"

extern void enable_irq(int);
extern void EnableUserMode(void);

int task1(void* data);
int task2(void* data);
int task3(void* data);

void start(void* env)
{
    /* NOTE: loader already did "cli" */
    env_init(env);
    screen_init();
    screen_clear();
    InitGDT();
    InitIDT();
    vm_init();

    //screen_clear();
    screen_printf("%d\n", bootinfo.video_size);
    screen_printf("kernsz: %d, stack: 0x%X, 0x%X, video: 0x%X, 0x%X\n", 
            bootinfo.kern_size,
            bootinfo.stack_va,
            bootinfo.stack_size,
            bootinfo.video_va,
            bootinfo.video_size);

    hltloop();

    InitThreading();
    timer_init(100);
    enable_irq(0);
    __asm__ volatile("sti");

    //pm_status();
    /*
    int a = test(100);
    panic("test")
    uint32 p = 0x1600000;
    *(int*)p = a;
    */

    //ThreadCreate(task1, NULL, true);
    //ThreadCreate(task2, NULL, true);
    //ThreadCreate(task3, "xx", true);
    //ThreadCreate(task3, "yy", true);

    /*
    unsigned char bin[] = {
        0x55,
        0x89, 0xe5,
        0xeb, 0xfe,
        0xeb, 0xfe
    };
    */
    unsigned char bin[] = { 
        0x55, 
        0x89, 0xe5,
        0x31, 0xc9,
        0xeb, 0x04,
        0xcd, 0x80, 
        0x89, 0xc1,
        0x89, 0xc8,
        0x41,
        0x81, 0xf9, 0xc8, 0x00, 0x00, 0x00,
        0xb8, 0x00, 0x00, 0x00, 0x00,
        0x77, 0xec,
        0x89, 0xc8,
        0xeb, 0xe8
    };

    //memcpy(0, bin, sizeof(bin));
    //ThreadCreate(0, NULL, false);

    //TODO: need a scheduler to replace it
    hltloop();
}

int task1(void* data)
{
    while (1) {
        screen_setfg(COLOR_Yellow);
        screen_printf("%u ", ThreadID());

        __asm__ volatile("hlt");
        //ThreadCreate(task3, "000000000000");
    }
    return 0;
}

int task2(void* data)
{
    while (1) {
        screen_setfg(COLOR_Blue);
        screen_printf("%u ", ThreadID());

        /*
        uint32 a = ch;
        __asm__ volatile("movl %0, %%eax"
                         :
                         : "r" (a)
                         : "%eax"
                         );
                         */
        //__asm__ volatile("int $0x80");
        __asm__ volatile("hlt");
        //ThreadCreate(task3, "111111111111");
    }
    return 0;
}

int task3(void* data)
{
    char* str = (char*)data;
    for (int i = 0; i < 10; ++i) {
    //while (true) {
        screen_setfg(COLOR_Red);
        screen_printf(str);
        __asm__ volatile("hlt");
    }
    screen_setfg(COLOR_Red);
    screen_printf("#%u#\n", ThreadID());
    return 0;
}
