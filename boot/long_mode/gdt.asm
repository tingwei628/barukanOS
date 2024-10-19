section .data
global tss_64_ptr
gdt_64:
    ; null descriptor
    dq 0
    ; Ring0 cs descriptor is the same value as "Code64" in loader.asm
    dq 0x0020980000000000
    ; Ring3 cs descriptor (DPL: from 00 to 11)
    dq 0x0020f80000000000
    ; Ring3 data segment scriptor (DPL: from 00 to 11)
    dq 0x0000f20000000000

tss_desc:
    ; tss limit
    dw tss_64_len-1
    ; base: bit-16 to bi-31
    dw 0
    ; base: bit-32 to bit-39
    db 0
    ; Present=1, DPL = 00, type=01001 (64bit-tss)
    db 0x89
    db 0
    ; base: bit-56 to bit-63
    db 0
    ; base: bit-64 to bit-95
    ; reserve: bit-96 to bit-127
    dq 0

gdt_64_len: equ $-gdt_64

gdt_64_ptr:
    dw gdt_64_len-1
    ; pointer is 8 bytes here
    dq gdt_64