[BITS 64]
[ORG 0x200000]

start:
    ; load Gdt64 to GDTR
    lgdt [Gdt64Ptr]
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

End:
    hlt
    jmp End

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