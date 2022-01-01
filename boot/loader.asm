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

; set video mode to "TextMode"
SetVideoMode:
    mov ax, 3
    int 0x10

    jmp SetupProtectedMode
    
; setup a message to print on screen
SetupMessage:
    mov si, Message
    ; screen has 25 lines, and  each line has 80 bytes (Resolution = 80 * 25)
    ; 0xb8000 is first position (top left) on screen
    ; es:di = 0xb8000
    ; 0xb800 * 16 + 0 = 0xb8000
    ; es = 0xb800 (segment) and di = 0 (offset)
    mov ax, 0xb800
    mov es, ax
    xor di, di
    ; cx is number of characters in string
    mov cx, MessageLen

; print message on screen
PrintMessage:
    ; copy a char from message to position on screen
    mov al, [si]
    mov [es:di], al
    ; each position on screen has 2 bytes
    ; position = [char:background-color|foreground-color]
    ; upper-half of position is char
    ; lower-half if position is background-color|foreground-color
    ; set char color(0xa) on 2nd byte in position
    mov byte[es:di+1], 0xa
    ; move to next position
    add di, 2
    ; each char in message has 1 byte
    ; move to next char
    add si, 1
    ; cx = loop count, each time loop, cx = cx - 1
    loop PrintMessage

; BIOSPrintMessage:
;     mov ah, 0x13
;     mov al, 1
;     mov bx, 0xa
;     xor dx, dx
;     mov bp, Message
;     mov cx, MessageLen 
;     int 0x10

; setup before entering protected mode (32-bit)
SetupProtectedMode:
    ; disable interrupts 
    cli
    ; load GDT pointer value from memory to GDTR
    lgdt [Gdt32Ptr]
    ; load IDT pinter value from memory to IDTR
    lidt [Idt32Ptr]

    ; enable protected mode=1
    mov eax, cr0
    or eax,1
    mov cr0, eax
    ; Real mode => "Segment:Offset"
    ; Protected Mode => "segment selector:Offset"
    ; jump to code descriptor in protected mode
    ; CS (code segment register) store code segment selector
    ; code segment selector => 00001000
    ; (use code segment selector to find code segment descriptor)
    ; bit-0 to bit-1 =00 (RPL, requested privilege level)
    ; bit-2 = 0 (0 for GDT, 1 for LDT)
    ; bit-3 to bit-15 = 1(index in GDT or LDT)
    ; code segment descriptor is the 2nd entry (index=1) in GDT
    ; 8 = 00001000
    jmp 8:PMEntry

ReadError:
NotAvailable:
End:
    hlt
    jmp End

DriveId:    db 0
Message:    db "Text mode is set"
MessageLen: equ $-Message
ReadPacket: times 16 db 0

; 32-bit in protected mode
[BITS 32]
PMEntry:
    ; set data segment selector(=00010000=0x10) to ds, es, ss
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x7c00
    ; print 'P' on screen
    ; the first position on screen is 0xb8000
    ; char = 'P', color = 0xa
    mov byte[0xb8000], 'P'
    mov byte[0xb8001], 0xa

PEnd:
    hlt
    jmp PEnd

; start to define GDT (each entry has 8 bytes in GDT)
; it has only 3 entries in this course...
; (GDT can be up to 65536 bytes in length (8192 entries))
; null null descriptor (the first entry in GDT)
Gdt32:
    ; dq = 8 bytes
    dq 0
; code segment descriptor, offset 8 bytes from base address of GDT
; (the 2nd entry in GDT)
Code32:
    ; limit low (bit-0 to bit-15)
    dw 0xffff
    ; base low (bit-16 to bit-31)
    ; code segement start at 0
    dw 0
    ; base middle (bit-32 to bit-39)
    db 0
    ; access
    ; 0x9a=10011010 (binary format)
    ; bit-40=0 (access, used with virtual memory)
    ; bit-41=1 (readable/writable)
    ; 0 for execute only, 1 for read and execute
    ; bit-42=0 (is conforming)
    ; 0 for "not conforming"(only executed =privilege level field), 1 for conforming (can be executed <= privilege level field)
    ; bit-43=1 (1 for code segment, 0 for data segment)
    ; bit-44=1 (0 for "system", 1 for "code/data" descriptor)
    ; bit-45 to bit-46 = 00 (privilege level, 00(Ring 0))
    ; bit-47=1 (present bit, is the segment in memory, used with virtual memory)
    db 0x9a
    ; granularity
    ; 0xcf=11001111 (binary format)
    ; bit-48 to bit-51=1111 (limit high, bit-16 to bit-19)
    ; bit-52=0 (reserved)
    ; bit-53=0 (long mode, 1 for 64 bit)
    ; bit-54=1 (segment type, 0 for 16 bit/ 1 for 32 bit)
    ; bit-55=1 (granularity, 1 for 4kb, 0 for 1byte)
    db 0xcf
    ; base high (bit-56 to bit-63)
    db 0
; data segment descriptor, offset 16 bytes from base address of GDT
; (the 3rd entry in GDT)
Data32:
    dw 0xffff
    dw 0
    db 0
    ; access
    ; 0x92=10010010 (binary format)
    ; bit-41=1 (readable/writable)
    ; 0 for read only, 1 for read and write
    ; bit-42=0 (expansion direction)
    ; 0 for the segment grows up, 1 for the segment grows down, 
    ; bit-43=0 (1 for code segment, 0 for data segment)
    db 0x92
    db 0xcf
    db 0

; length of GDT
Gdt32Len: equ $-Gdt32
; pointer to GDT
Gdt32Ptr:
    ; limit (size of GDT)
    dw Gdt32Len-1
    ; base address of GDTs
    dd Gdt32
; pointer to IDT
Idt32Ptr:
    dw 0
    dd 0

