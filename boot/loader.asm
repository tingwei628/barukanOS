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

LoadKernel:
    mov si, ReadPacket
    mov word[si], 0x10
    ; number of sectors to read
    mov word[si+2], 1
    ; load kernel start at 0x10000
    ; transfer buffer (16 bit segment:16 bit offset)
    ; 16 bit offset=0 (stored in word[si+4])
    ; 16 bit segment=0x1000 (stored in word[si+6])
    ; address => 0x1000 * 16 + 0 = 0x10000
    mov word[si+4], 0
    mov word[si+6], 0x1000
    mov dword[si+8], 1
    mov dword[si+0xc], 0
    mov dl, [DriveId]
    ; function code, 0x42 = Extended Read Sectors From Drive
    mov ah, 0x42
    int 0x13
    jc  ReadError

; INT 15H = Miscellaneous system services
; get memory map
GetMemInfoStart:
    ; note that the upper 16-bits of eax should be set to 0
    ; function code, 0xe820
    mov eax, 0xe820
    ; set edx to the magic number 0x534D4150
    ; smap
    mov edx, 0x534d4150
    ; The length of memory block in bytes 
    mov ecx, 20
    ; ES:DI
    ; start address of memory block
    mov edi, 0x9000
    ; ebx = Continuation (the first call => ebx must be 0)
    xor ebx, ebx
    ; 0xe820 function that can detect memory areas above 4G
    ; check if 0xe820 service is available
    int 0x15
    jc NotAvailable

GetMemInfo:
    ; move to next memory block (each memory block has 20 bytes)
    add edi, 20
    ; ebx = Continuation
    ; ebx must be preserved and do not change ebx
    mov eax, 0xe820
    mov edx, 0x534d4150
    mov ecx, 20
    int 0x15
    ; jump if there is NO next memory block
    jc GetMemDone
    ; test => ebx & ebx, if result!=0, ZF=0, then jump
    test ebx, ebx
    jnz GetMemInfo

; test whether A20 line is enabled or not
GetMemDone:
TestA20:
    ; if A20 line enabled, bit-20=1
    ; if not enabled, bit-20=0
    ; e.g. 0x107c00 (A20 line enabled) => 0x107c00
    ;      0x107c00 (A20 line NOT enabled) => 0x007c00
    ; 0xffff = 0b1111111111111111 (binary format)
    mov ax, 0xffff
    mov es, ax
    ; segement ds = 0, offset = 0x7c00 
    ; 0 * 16 + 0x7c00 = 0x7c00 (address)
    ; it has value 0xa200 at address 0x7c00 
    mov word[ds:0x7c00], 0xa200
    ; segment es = 0xffff, offset = 0x7c10
    ; es:0x7c10 => 0xffff * 16 + 0x7c10 = 0x107C00
    cmp word[es:0x7c10], 0xa200
    ; when address is NOT the same, then jmp SetA20LineDone
    jne SetA20LineDone
    ; because 0x107C00 may just has value 0xa200 before
    ; that's why we need the double check
    ; let's assign different random value "0xb200"
    mov word[0x7c00], 0xb200
    cmp word[es:0x7c10], 0xb200
    ; if they are same, A20 line is not enabled
    je End
    
SetA20LineDone:
    xor ax,ax
    mov es,ax
    
PrintMessage:
    mov ah, 0x13
    mov al, 1
    mov bx, 0xa
    xor dx, dx
    mov bp, Message
    mov cx, MessageLen 
    int 0x10

ReadError:
NotAvailable:
End:
    hlt
    jmp End

DriveId:    db 0
Message:    db "a20 line is on"
MessageLen: equ $-Message
ReadPacket: times 16 db 0