%include "adef.inc"

[bits 16]
    org PA_BOOT
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, PA_BOOT

    ; read loader+kernel+... from floppy and write to es:bx
    ; (tracks)*(2 sides)*(18 sectors)*(sector size is 512B)
    ; 20*2*18*512=360KB
    ; 3.5 720KB: 80*2*9
    ; 3.5 1.44MB: 80*2*18
    ; 3.5 2.88MB: 80*2*36
    mov ax, PA_LOADER/16
    mov es, ax
    xor bx, bx      ; es:bx=PA_LOADER
    mov ch, 0       ; C, (track index start from 0)
    mov dh, 0       ; H, 0:upside, 1:downside
    mov cl, 2       ; S, read from second sector(index start from 1)

ReadLoop:
    mov si, 0       ; count for retry times

Retry:
    mov ah, 0x02    ; cmd read
    mov dl, 0x00    ; drive A
    mov al, 1       ; one sector
    int 0x13        ; do cmd
    jnc Next        ; success?

    inc si          ; inc retry
    cmp si, 5       ; retry 5 times
    jae Error

    xor ah, ah      ; cmd reset
    mov dl, 0x00    ; drive A
    int 0x13        ; do cmd
    jmp Retry

Next:
    mov ax, es
    add ax, 512/16
    mov es, ax      ; inc 512 for dest
    inc cl          ; next sector
    cmp cl, 18      ; all sectors were read?
    jbe ReadLoop

    mov cl, 1       ; reset sector index to 1
    inc dh          ; the other side
    cmp dh, 2       ; both sides were read?
    jb ReadLoop

    xor dh, dh      ; reset to up side
    inc ch          ; next track
    cmp ch, NCYLS   ; all tracks were read?
    jb ReadLoop

    mov [INFO_CYLS], ch     ; setup boot info(ch=NCYLS)
    jmp PA_LOADER           ; let's start from here

Error:
    xor ax, ax
    mov es, ax
    mov si, ErrorMsg

PutChar:
    mov al, [si]
    inc si
    cmp al, 0
    je Stop
    mov ah, 0x0e
    mov bx, 15
    int 0x10
    jmp PutChar

Stop:
    hlt
    jmp Stop

ErrorMsg:
    db 0x0a, 0x0a
    db "Unable to boot!"
    db 0x0a
    db 0

; padding and magic
    resb 510-($-$$)
    dw 0xaa55
