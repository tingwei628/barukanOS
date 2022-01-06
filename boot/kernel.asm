[BITS 64]
[ORG 0x200000]

start:

    mov rdi, Idt64
    mov rax, Handler0
    ; set Handler0 as handler after triggering "divided by 0" (which is the first entry in IDT)
    ; copy first part of offset (bit-0 to bit-15 in Handler0) to IDT
    ; ax = 2 bytes
    mov [rdi], ax
    ; shift right 16 bit to get second part of offset (bit-48 to bit-63 in Handler0)
    shr rax, 16
    ; copy offset to address rdi+6
    mov [rdi + 6], ax
    ; shift right 16 bit to get third part of offset (bit-64 to bit-95 in Handler0)
    shr rax, 16
    ; copy offset to address rdi+8
    mov [rdi + 8], eax


    ; load Gdt64 to GDTR
    lgdt [Gdt64Ptr]
    ; load Idt64 to IDTR
    lidt [Idt64Ptr]

    ; since rsp=0x7c00
    ; code segment descriptor is 8 bytes offest from start of Gdt64
    push 8
    push KernelEntry
    db 0x48
    ; load code segment descriptor into code segment register
    ; retf = far return
    ; retf => pop RIP, then pop CS
    ; CS:IP => RIP=KernelEntry, CS=8 => 8:KernelEntry
    ; why add this prefix "db 0x48"
    ; The default operand size of retf is 32bits, and
    ; this will pop invalid data into rip and cs registers 
    ; since we pushed 64bits value on stack.
    ; in short, adjust operand size from 32 bits to 64 bits
    ; Source: https://wiki.osdev.org/X86-64_Instruction_Encoding
    ; In  "REX prefix" section,
    ; 0x48 = 01001000b
    ; Fixed bit pattern=0100b
    ; W=1(1, a 64-bit operand size is used)
    retf

KernelEntry:
    mov byte[0xb8000], 'K'
    mov byte[0xb8001], 0xa

    ; set rbx=0
    xor rbx, rbx
    ; divided by 0 to cause excetpion
    div rbx


End:
    hlt
    jmp End

Handler0:
    mov byte[0xb8000], 'D'
    mov byte[0xb8001], 0xc

    ; stop kernel if error happened
    jmp End
    ; interrupt return (64-bit)
    iretq

Gdt64:
    ; null descriptor
    dq 0
    ; cs descriptor is the same value as "Code64" in loader.asm
    dq 0x0020980000000000

Gdt64Len: equ $-Gdt64


Gdt64Ptr:
    dw Gdt64Len-1
    ; pointer is 8 bytes here
    dq Gdt64

; IDT (Interrupt descriptor table)
Idt64:
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

Idt64Len: equ $-Idt64

Idt64Ptr:
    dw Idt64Len-1
    ; pointer is 8 bytes here
    dq Idt64