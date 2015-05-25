[bits 32]

[global LoadGdt]
LoadGdt:
    mov eax, [esp+4]    ; gdt pointer
    lgdt [eax]

    mov ax, [esp+12]    ; data selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov eax, [esp+8]    ; code selector
    mov ebx, [esp]      ; eip
    add esp, 4          ; clear return address
    push eax
    push ebx
    retf                ; far jump directly back to C

[global LoadTss]
LoadTss
    mov ax, [esp+4]
    ltr ax
    ret
