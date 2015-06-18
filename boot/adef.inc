%ifndef ADEF_INC
%define ADEF_INC

NCYLS   equ 20

INFO_CYLS   equ PA_INFO          ; 1B
INFO_LEDS   equ INFO_CYLS+1      ; 1B
INFO_LMEM   equ INFO_LEDS+1      ; 2B 0~0x500~nKB
INFO_NHMEM  equ INFO_LMEM+2      ; 2B
INFO_EHMEM  equ INFO_NHMEM+2     ; 10 entry, each takes 24B, see BootInfo.h

INFO_VMODE  equ INFO_EHMEM+24*10 ; 2B
INFO_XPIXEL equ INFO_VMODE+2     ; 2B
INFO_YPIXEL equ INFO_XPIXEL+2    ; 2B
INFO_VRAM   equ INFO_YPIXEL+2    ; 4B

INFO_KERN_PA      equ INFO_VRAM+4         ; 4B kernel physical address
INFO_KERN_VA      equ INFO_KERN_PA+4      ; 4B kernel virtual address
INFO_KERN_ENTRY   equ INFO_KERN_VA+4      ; 4B kernel entry's virtual address
INFO_KERN_SIZE    equ INFO_KERN_ENTRY+4   ; 4B kernel in-memory space
INFO_STACK_PA     equ INFO_KERN_SIZE+4    ; 4B kernel stack physical address
INFO_STACK_VA     equ INFO_STACK_PA+4     ; 4B kernel stack virtual address
INFO_STACK_SIZE   equ INFO_STACK_VA+4     ; 4B
INFO_DIR_PA       equ INFO_STACK_SIZE+4   ; 4B page directory physical address
INFO_DIR_VA       equ INFO_DIR_PA+4       ; 4B page directory virtual adress

;ASSETS  equ INFO_STACK_SIZE+4   ; 4+4*nB

VBE640x480  equ 0x101   ; 640*480*8bit
VBE800x600  equ 0x103   ; 800*600*8bit
VBE1024x768 equ 0x105   ; 1024*768*8bit

%macro Descriptor 3
    dw %2 & 0xffff
    dw %1 & 0xffff
    db (%1 >> 16) & 0xff
    dw ((%2 >> 8) & 0xf00) | (%3 & 0xf0ff)
    db (%1 >> 24) & 0xff
%endmacro

DA_32       equ 0x4000
DA_LIMIT_4K equ 0x8000

DA_DPL0 equ 0x00
DA_DPL1 equ 0x20
DA_DPL2 equ 0x40
DA_DPL3 equ 0x60

DA_DR   equ 0x90 ; ds r
DA_DRW  equ 0x92 ; ds rw
DA_DRWA equ 0x93 ; ds rw accessed
DA_CX   equ 0x98 ; cs x
DA_CXR  equ 0x9a ; cs xr
DA_CCO  equ 0x9c
DA_CCOR equ 0x9e

%endif
