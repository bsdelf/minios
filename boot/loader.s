%include "adef.inc"

[bits 16]
    org PA_LOADER

    call CheckKeyboard
    call CheckMem
    call SwitchVideo

    ; disable PIC & CPU interruption
    mov al, 0xff
    out 0x21, al
    nop
    out 0xa1, al
    cli

    ; fast enable A20
    in al, 0x92
    or al, 10b
    out 0x92, al

    lgdt [GDT_PTR]

    ; update control register 0
    mov eax, cr0
    and eax, 0x7fffffff ; clear PG bit, disable paging
    or  eax, 1          ; set ET bit
    mov cr0, eax

    ; far jump to protected mode
    jmp SEL_GDT_X:Protected

; check keyboard
CheckKeyboard:
    mov ah, 0x02
    int 0x16
    mov [INFO_LEDS], al
    ret

; check memory
CheckMem:
    ; low memory
    xor ax, ax
    mov [INFO_LMEM], ax
    int 0x12
    jc .EndOfLow
    test ax, ax
    jz .EndOfLow
    mov [INFO_LMEM], ax
.EndOfLow

    ; high memory (> 1MB)
    mov ax, 0
    mov es, ax
    mov di, INFO_EHMEM

    xor ebx, ebx
    xor bp, bp
    mov edx, 0x0534D4150
    mov eax, 0xe820
    mov [es:di+20], dword 1
    mov ecx, 24
    int 0x15
    jc .EndOfHigh
    mov edx, 0x0534D4150
    cmp eax, edx
    jne .EndOfHigh
    test ebx, ebx
    je .EndOfHigh
    jmp .jmpin
.e820lp:
    mov eax, 0xe820
    mov [es:di+20], dword 1
    mov ecx, 24
    int 0x15
    jc .e820f
    mov edx, 0x0534D4150
.jmpin:
    ;jcxz .skipent
    cmp cl, 20
    jbe .notext
    test byte [es:di+20], 1
    ;je .skipent
.notext:
    mov ecx, [es:di+8]
    or ecx, [es:di+12]
    jz .skipent
    inc bp
    add di, 24
.skipent:
    test ebx, ebx
    jne .e820lp
.e820f
    mov [INFO_NHMEM], bp
    clc
    ret
.EndOfHigh
    stc
    ret

; switch video mode
SwitchVideo:
    ; check VBE available
    mov ax, 0x9000
    mov es, ax
    xor di, di

    ;mov ax, 0x4f
    ;int 0x10
    ;cmp ax, 0x004f
    ;jne .VGA320x200

    ; check VBE version
    ;mov ax, [es:di+4]
    ;cmp ax, 0x0200
    ;jb .VGA320x200

.VBEMode:
    mov cx, VBE640x480      ; check available
    mov ax, 0x4f01          ; cmd
    int 0x10
    cmp ax, 0x004f
    jne .VGA320x200

    mov bx, VBE640x480+0x4000   ; enable mode
    mov ax, 0x4f02              ; cmd
    int 0x10
    cmp ax, 0x004f              ; success?
    jne .VGA320x200

    mov byte [INFO_VMODE], 8
    mov ax, [es:di+0x12]
    mov [INFO_XPIXEL], ax
    mov ax, [es:di+0x14]
    mov [INFO_YPIXEL], ax
    mov eax, [es:di+0x28]
    mov [INFO_VRAM], eax

    jmp .Done

.VGA320x200:
    mov al, 0x13    ; VGA 320x200x8bit
    xor ah, ah
    int 0x10

    mov byte  [INFO_VMODE],  8
    mov word  [INFO_XPIXEL], 320
    mov word  [INFO_YPIXEL], 200
    mov dword [INFO_VRAM],   0xa0000

.Done
    ret

[bits 32]
Protected:
    ; now cs=SEL_GDT_X, we do not need far call/jmp any more
    mov ax, SEL_GDT_RW
    mov ds, ax
    mov es, ax
    
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, Protected

    ; init info specified by macro
    mov dword [INFO_KERN_PA], PA_KERNEL
    mov dword [INFO_KERN_VA], VA_KERNEL
    mov dword [INFO_STACK_SIZE], SIZE_KSTACK

    ; load elf kernel
    call LoadElf

    ; | kernel | ~ | stack guard | free stack | 1024 PDEs | 1024 PTEs * 1024 | ~

    ; setup kernel stack
    mov eax, [INFO_KERN_VA]
    add eax, [INFO_KERN_SIZE]
    add eax, 0xfff
    and eax, ~0xfff                 ; 4K aligned address to the end of kernel
    mov [INFO_STACK_VA], eax        ; begin address of kernel stack guard
    mov eax, [INFO_KERN_PA]
    add eax, [INFO_KERN_SIZE]
    add eax, 0xfff
    and eax, ~0xfff                 ; 4K aligned address to the end of kernel
    mov [INFO_STACK_PA], eax        ; begin address of kernel stack guard (4K)

    ; setup paging
    mov eax, [INFO_STACK_PA]
    add eax, [INFO_STACK_SIZE]
    mov [INFO_DIR_PA], eax
    mov cr3, eax                    ; set PD paddr
    mov eax, [INFO_STACK_VA]
    add eax, [INFO_STACK_SIZE]
    mov [INFO_DIR_VA], eax

    mov eax, cr3
    add eax, 0x1000
    push eax                        ; 1st 1024 PTEs begin paddr for 0~4M
    add eax, 0x1000*768
    push eax                        ; 2nd 1024 PTEs begin paddr for 3G~(3G+4M)

    ; bzero dir and tables
    mov eax, cr3
    mov ecx, 1024+1024*1024
.bzdir
    mov dword [eax], 0
    add eax, 4
    loop .bzdir

    ; init 1st PDE
    mov edi, cr3            ; to 1st PDE
    mov eax, [esp+4]        ; get related 1024 PTEs begin paddr
    and eax, ~0xfff         ; clear lower 12 bits
    or eax, 11b             ; us=0 rw=1 present=1
    mov [edi], eax          ; set 1st PDE

    ; init related 1024 PTEs, identity map 0-1M
    mov ecx, 256            ; 1M/4K=256 times
    mov esi, 0              ; target paddr
    mov edi, [esp+4]        ; to 1st 1024 PTEs begin paddr
.setlowptes
    mov eax, esi
    and eax, ~0xfff
    or eax, 11b
    mov [edi], eax          ; map
    add esi, 0x1000         ; to next paddr
    add edi, 4              ; to next PTE
    loop .setlowptes

    NPDE equ 5
    ; init 768th PDE
    mov ecx, NPDE           ; map 4M * NPDE
    mov edi, cr3
    add edi, 768*4          ; to 768th PDE
    mov esi, [esp]          ; to 2nd 1024 PTEs begin paddr
.setpdes
    mov eax, esi
    and eax, ~0xfff
    or eax, 11b
    mov [edi], eax          ; set 768th PDE
    add esi, 0x1000         ; to next 1024 PTEs begin paddr
    add edi, 4              ; to next PDE
    loop .setpdes

    ; init related 1024 PTEs, map INFO_KERN_PA~nM to 3G~(3G+4M)
    mov ecx, 1024*NPDE          ; 1024 * 4K
    mov esi, [INFO_KERN_PA]     ; target paddr
    mov edi, [esp]              ; to 2nd 1024 PTEs begin paddr
.sethigptes
    mov eax, esi
    and eax, ~0xfff
    or eax, 11b
    mov [edi], eax
    add esi, 0x1000             ; to next paddr
    add edi, 4                  ; to next PTE
    loop .sethigptes

    ; drop two tmp vars
    add esp, 4*2

    ; enable paging
    mov eax, cr0
    or eax, 0x80000000  ; set PG bit
    mov cr0, eax

    ; jump to kernel
    mov eax, [INFO_STACK_VA]
    add eax, [INFO_STACK_SIZE]
    mov esp, eax                    ; stack
    push dword PA_INFO              ; arg0
    mov eax, [INFO_KERN_ENTRY]
    call eax

LoadElf:
    mov eax, 0x464C457F             ; ELF magic
    cmp eax, [BeginOfELF]           ; verify magic
    jne Die

    mov eax, [BeginOfELF+24]        ; e_entry, virtual entry address
    mov [INFO_KERN_ENTRY], eax
    mov dword [INFO_KERN_SIZE], 0   ; clear kernel memory size

    xor esi, esi
    mov cx, [BeginOfELF+44]         ; e_phnum, program count
    movzx ecx, cx
    mov esi, [BeginOfELF+28]        ; e_phoff, offset to header
    add esi, BeginOfELF             ; e_phoff's location in memory

.LoadProgram:
    mov eax, [esi]
    cmp eax, 0
    jz .ToNext

    mov ebx, [esi+20]           ; p_memsz, program size in memory
    mov eax, [esi+28]           ; p_align
    dec eax
    add ebx, eax
    not eax
    and ebx, eax
    add [INFO_KERN_SIZE], ebx   ; add aligned program size

    mov eax, [esi+16]           ; p_filesz, program size
    push eax                    ; arg2: size
    mov eax, [esi+4]            ; p_shoff, offset to header
    add eax, BeginOfELF
    push eax                    ; arg1: src
    mov eax, [esi+8]            ; p_vaddr, program virtual address
    sub eax, [INFO_KERN_VA]
    add eax, [INFO_KERN_PA]
    push eax                    ; arg0: dest
    call MemCpy
    add esp, 4*3                ; cleanup 3 arguments

.ToNext
    add esi, 4*8                ; to next phdr, sizeof(Elf32_Phdr), see "man 5 elf"
    dec ecx
    jnz .LoadProgram            ; load next program?

    ret

MemCpy:
    push ebp
    mov ebp, esp

    push esi
    push edi
    push ecx

    mov edi, [ebp+8]
    mov esi, [ebp+12]
    mov ecx, [ebp+16]
.1:
    cmp ecx, 0
    jz .2

    mov al, [ds:esi]
    inc esi

    mov byte [es:edi], al
    inc edi

    dec ecx
    jmp .1
.2:
    mov eax, [ebp+8]

    pop ecx
    pop edi
    pop esi
    mov esp, ebp
    pop ebp
    ret

Stop:
    hlt
    jmp Stop

Die:
    mov ax, 10
    mov bx, 0
    div bx

GDT:        Descriptor  0,  0,         0                        ; dummy
GDT_X:      Descriptor  0,  0xfffff,   DA_CXR|DA_32|DA_LIMIT_4K ; 0~4G code segment
GDT_RW:     Descriptor  0,  0xfffff,   DA_DRW|DA_32|DA_LIMIT_4K ; 0~4G data segment

GDT_LEN     equ $-GDT
GDT_PTR:
    dw GDT_LEN-1
    dd GDT

SEL_GDT_X   equ     GDT_X-GDT
SEL_GDT_RW  equ     GDT_RW-GDT

BeginOfELF:
