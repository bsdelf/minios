[bits 32]

[global asm_idt_flush]
asm_idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret

;============================================
; System call
;============================================
extern SyscallRouter
[global OnSyscall]
OnSyscall:
    ; store edi, esi, ebp, esp, ebx, edx, ecx, eax, etc.
    pushad
    push ds
    push es
    push fs
    push gs

    ; load the kernel data segment selector (see GDT.c)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; call C function, pass args by pointer
    push esp
    call SyscallRouter
    add esp, 4

    ; restore edi, esi, ebp, esp, ebx, edx, ecx, eax, etc.
    pop gs
    pop fs
    pop es
    pop ds
    popad
    
    ; restore cs, eip, eflags(, esp, ss)
    iret

;============================================
; ISR stuff
;============================================

[global OnEmptyIsr]
OnEmptyIsr:
    iret

%macro ISR_NOERRCODE 1
[global OnIsr%1]
OnIsr%1:
    push 0xfaceb00b     ; dummy error code
    push dword %1       ; interrupt number
    jmp IsrCommonStub
%endmacro

%macro ISR_ERRCODE 1
[global OnIsr%1]
OnIsr%1:
    push dword %1       ; interrupt number
    jmp IsrCommonStub
%endmacro

; This is our common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
extern IsrRouter
IsrCommonStub:
    ; store edi, esi, ebp, esp, ebx, edx, ecx, eax, etc.
    pushad
    push ds
    push es
    push fs
    push gs

    ; load the kernel data segment selector (see GDT.c)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; call C function, pass args by pointer
    call IsrRouter

    ; restore edi, esi, ebp, esp, ebx, edx, ecx, eax, etc.
    pop gs
    pop fs
    pop es
    pop ds
    popad

    ; remove error code and ISR number
    add esp, 8

    ; restore cs, eip, eflags(, esp, ss)
    iret

;============================================
; IRQ stuff
;============================================

%macro IRQ 2
[global OnIrq%1]
OnIrq%1:
    push dword %2       ; IRQ number
    jmp IrqCommonStub
%endmacro

; This is our common IRQ stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
extern IrqRouter
IrqCommonStub:
    ; store edi, esi, ebp, esp, ebx, edx, ecx, eax, etc.
    pushad
    push ds
    push es
    push fs
    push gs

    ; load the kernel data segment selector (see GDT.c)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; call C function, pass args by pointer
    push esp
    call IrqRouter
    add esp, 4

    ; restore edi, esi, ebp, esp, ebx, edx, ecx, eax, etc.
    pop gs
    pop fs
    pop es
    pop ds
    popad

    ; remove ISR number
    add esp, 4

    ; restore cs, eip, eflags(, esp, ss)
    iret

INT_M_CTLMASK        equ     0x21    ; setting bits in this port disables ints   <Master>
INT_S_CTLMASK        equ     0xA1    ; setting bits in this port disables ints   <Slave>
; ========================================================================
;                  void disable_irq(int irq);
; ========================================================================
; Disable an interrupt request line by setting an 8259 bit.
; Equivalent code:
;   if(irq < 8){
;       out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) | (1 << irq));
;   }
;   else{
;       out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) | (1 << irq));
;   }
[global disable_irq]
disable_irq:
    mov     ecx, [esp + 4]          ; irq
    pushf
    cli
    mov     ah, 1
    rol     ah, cl                  ; ah = (1 << (irq % 8))
    cmp     cl, 8
    jae     disable_8               ; disable irq >= 8 at the slave 8259
disable_0:
    in      al, INT_M_CTLMASK
    test    al, ah
    jnz     dis_already             ; already disabled?
    or      al, ah
    out     INT_M_CTLMASK, al       ; set bit at master 8259
    popf
    mov     eax, 1                  ; disabled by this function
    ret
disable_8:
    in      al, INT_S_CTLMASK
    test    al, ah
    jnz     dis_already             ; already disabled?
    or      al, ah
    out     INT_S_CTLMASK, al       ; set bit at slave 8259
    popf
    mov     eax, 1                  ; disabled by this function
    ret
dis_already:
    popf
    xor     eax, eax                ; already disabled
    ret

; ========================================================================
;                  void enable_irq(int irq);
; ========================================================================
; Enable an interrupt request line by clearing an 8259 bit.
; Equivalent code:
;       if(irq < 8){
;               out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) & ~(1 << irq));
;       }
;       else{
;               out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) & ~(1 << irq));
;       }
;
[global enable_irq]
enable_irq:
    mov     ecx, [esp + 4]          ; irq
    pushf
    cli
    mov     ah, ~1
    rol     ah, cl                  ; ah = ~(1 << (irq % 8))
    cmp     cl, 8
    jae     enable_8                ; enable irq >= 8 at the slave 8259
enable_0:
    in      al, INT_M_CTLMASK
    and     al, ah
    out     INT_M_CTLMASK, al       ; clear bit at master 8259
    popf
    ret
enable_8:
    in      al, INT_S_CTLMASK
    and     al, ah
    out     INT_S_CTLMASK, al       ; clear bit at slave 8259
    popf
    ret

; ========================================================================
;                   Apply those macros
; ========================================================================

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19

IRQ    0,   32
IRQ    1,   33
IRQ    2,   34
IRQ    3,   35
IRQ    4,   36
IRQ    5,   37
IRQ    6,   38
IRQ    7,   39
IRQ    8,   40
IRQ    9,   41
IRQ   10,   42
IRQ   11,   43
IRQ   12,   44
IRQ   13,   45
IRQ   14,   46
IRQ   15,   47



