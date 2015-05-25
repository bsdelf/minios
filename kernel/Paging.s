[bits 32]

[global LoadPageDirectory]
LoadPageDirectory:
    mov eax, [esp+4]
    mov cr3, eax        ; page directory's physical 4K aligned address
    ret

[global EnablePaging]
EnablePaging:
    mov eax, cr0
    or eax, 0x80000000  ; set PG bit
    mov cr0, eax
    ret

[global GetPageFaultAddress]
GetPageFaultAddress:
    mov eax, cr2
    ret
