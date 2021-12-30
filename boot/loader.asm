[BITS 16]
[ORG 0x7e00]

start:
    ; dl from boot.asm
    mov [DriveId], dl
    ; "cpuid" retrieve the information about your cpu
    ; eax=0x80000000: Get Highest Extended Function Implemented (only in long mode)
    ; 0x80000000 = 2^31
    mov eax, 0x80000000
    cpuid
    ; Long mode can only be detected using the extended functions of CPUID (> 0x80000000)
    ; It is less, there is no long mode.
    cmp eax, 0x80000001
    ; if eax < 0x80000001 => CF=1 => jb
    ; jb, jump if below for unsigned number
    ; (jl, jump if less for signed number)
    jb NotAvailable
    ; eax=0x80000001: Extended Processor Info and Feature Bits
    mov eax, 0x80000001
    cpuid
    ; check if long mode is supported
    ; test => edx & (1<<29), if result=0, ZF=1, then jump
    ; long mode is at bit-29
    test edx, (1<<29)
    ; jz, jump if zero => ZF=1
    jz NotAvailable
    ; check if 1g huge page support
    ; test => edx & (1<<26), if result=0, ZF=1, then jump
    ; Gigabyte pages is at bit-26
    test edx, (1<<26)
    jz NotAvailable

PrintMessage:
    mov ah, 0x13
    mov al, 1
    mov bx, 0xa
    xor dx, dx
    mov bp, Message
    mov cx, MessageLen 
    int 0x10

NotAvailable:
End:
    hlt
    jmp End

DriveId:    db 0
Message:    db "long mode is supported"
MessageLen: equ $-Message