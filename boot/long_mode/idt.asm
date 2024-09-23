; IDT (Interrupt descriptor table)
idt_64:
    ; %rep: invoke a multi-line macro multiple times
    ; repeat this block with 256 times
    ; so, there are 256 entries
    ; the entries in the IDT are 16 bytes long (in long mode)
    ; e.g. entry 1 = IDTR Offset + 16
    %rep 256
        ; offset: bit-0 to bit-15
        ; 0000 0000 0000 0000
        dw 0
        ; selector to code segment descriptor in GDT
        ; bit-16 to bit-31
        ; 0000 0000 0000 1000 = 0x8 (which is offset of code segment descriptor in GDT)
        dw 0x8
        ; reserved (bit-32 to bit-39) = 0000 0000
        db 0
        ; 1000 1110
        ; gate type(bit-40 to bit-43)= 1110 (1110 for interrupt gate,  1111 for trap gate)
        ; 0(bit-44) = 0
        ; dpl (bit-45 to bit-46) = 00 (Ring0)
        ; present bit (bit-47) = 1
        db 0x8e
        ; offset: bit-48 to bit-63
        dw 0
        ; offset: bit-64 to bit-95 
        dd 0
        ; reserved: bit-96 to bit-128
        dd 0
    %endrep

idt_64_len: equ $-idt_64

idt_64_ptr:
    dw idt_64_len-1
    ; pointer is 8 bytes here
    dq idt_64