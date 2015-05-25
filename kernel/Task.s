[bits 32]

[global SwitchUserContext]
SwitchUserContext:
    ; save registers for old task
    mov eax, [esp+4] ; arg0

    mov [eax], esp
    mov [eax+4], ebp
    mov [eax+8], ebx
    mov [eax+12], esi
    mov [eax+16], edi

    pushfd
    pop ecx
    mov [eax+20], ecx

    mov eax, [esp+8] ; arg1

    mov esp, [eax]
    mov ebp, [eax+4]
    mov ebx, [eax+8]
    mov esi, [eax+12]
    mov edi, [eax+16]

    mov ecx, [eax+20]
    push ecx
    popfd

    mov bx, 0x23
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    mov ebx, [eax]

    push 0x23           ; ss
    push ebx            ; esp
    pushf               ; eflag
    push 0x1b           ; cs
    mov ebx, [ebx]
    push ebx            ; eip

    iret

[global SwitchContext]
SwitchContext:
    ; save context
    mov eax, [esp+4]    ; arg0

    mov [eax],    esp   ; store current esp, and [esp] is the return address
    mov [eax+4],  ebp
    mov [eax+8],  ebx
    mov [eax+12], esi
    mov [eax+16], edi

    pushfd
    pop ecx
    mov [eax+20], ecx   ; store eflags
    
    ; restore context
    mov eax, [esp+8]    ; arg1
    jmp DoLoadContext

[global LoadContext]
LoadContext:
    mov eax, [esp+4]    ; arg0

DoLoadContext:
    mov esp, [eax]      ; load previous esp, and [esp] was ret address
    mov ebp, [eax+4]
    mov ebx, [eax+8]
    mov esi, [eax+12]
    mov edi, [eax+16]

    mov ecx, [eax+20]
    push ecx
    popfd               ; restore eflags

    ret
