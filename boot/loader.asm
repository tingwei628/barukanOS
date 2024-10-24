BITS 16
ORG 0x7e00

start:
    ; dl from boot.asm
    mov [DriveId], dl

LoadKernel:
    mov si, DiskReadPacket
    mov dl, [DriveId]
    mov ah, 0x42 ; function code, 0x42 = Extended Read Sectors From Drive
    int 0x13
    jc  ReadError

; read user program in examples
LoadUser1:
    mov si,ReadPacket
    mov word[si], 0x10
    mov word[si+2], 10
    ; 16 bit segment=0x2000
    ; address => 0x2000 * 16 + 0 = 0x20000,  load user1 start at 0x20000
    mov word[si+4], 0
    mov word[si+6], 0x2000 ; start
    mov dword[si+8], 106 ; LBA
    mov dword[si+0xc], 0
    mov dl, [DriveId]
    mov ah,0x42
    int 0x13
    jc  ReadError

LoadUser2:
    mov si, ReadPacket
    mov word[si], 0x10
    mov word[si+2], 10
    ; 16 bit segment=0x3000
    ; address => 0x3000 * 16 + 0 = 0x30000,  load user2 start at 0x30000
    mov word[si+4], 0
    mov word[si+6], 0x3000
    mov dword[si+8], 116 ; LBA
    mov dword[si+0xc], 0
    mov dl, [DriveId]
    mov ah, 0x42
    int 0x13
    jc  ReadError

LoadUser3:
    mov si, ReadPacket
    mov word[si], 0x10
    mov word[si+2], 10
    mov word[si+4], 0
    mov word[si+6], 0x4000
    ; 16 bit segment=0x4000
    ; address => 0x4000 * 16 + 0 = 0x40000,  load user3 start at 0x40000
    mov dword[si+8], 126 ; LBA
    mov dword[si+0xc], 0
    mov dl, [DriveId]
    mov ah, 0x42
    int 0x13
    jc  ReadError

; get memory info
call get_memory_info
; test whether A20 line is enabled or not
call test_A20

; set video mode to "TextMode"
SetVideoMode:
    mov ax, 3
    int 0x10
    ; mov si, Message
    ; mov cx, MessageLen
    ; call print_screen_string16
    ; call clear_screen_string16


; setup before entering protected mode (32-bit)
SetupProtectedMode:
    ; disable interrupts 
    cli
    ; load GDT pointer value from memory to GDTR
    lgdt [gdt_32_ptr]
    ; load IDT pinter value from memory to IDTR
    lidt [idt_32_ptr]

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
    jmp $

%include "boot/real_mode/print.asm"
%include "boot/real_mode/setup.asm"
%include "boot/real_mode/gdt.asm"

DriveId:    db 0
ReadPacket: times 16 db 0
Message:    db "Text mode is set"
MessageLen: equ $-Message
DiskReadPacket:
    dw 0x10
    dw 100 ; number of sectors(kernel) to read
    dw 0  ; 16 bit offset=0
    dw 0x1000  ; 16 bit segment=0x1000
    ; address => 0x1000 * 16 + 0 = 0x10000,  load kernel start at 0x10000
    dd 6  ; LBA=6 is the start of kernel sector
    dd 0

; 32-bit in protected mode
BITS 32
PMEntry:
    ; set data segment selector(=00010000=0x10) to ds, es, ss
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x7c00


    ; mov esi, Message32
    ; mov ecx, Message32Len
    ; call print_screen_string32
    ; call clear_screen_string32

SetupLongMode:
    ; IA-32e pageing: CR0.PG = 1, CR4.PAE = 1, and IA32_EFER.LME = 1
    ; which translates 48-bit linear addresses to 52-bit physical addresses.
    ; initialize the paging structure (IA-32e paging model)
    ; cld (clear direction flag, beginning from lowest to highest address )
    cld
    mov edi, 0x70000
    ; set eax=0
    xor eax, eax
    ; 0x10000 = 64kb
    ; 0x10000/4 = 16kb (if total 4 tables)
    ; each table is 4kb size
    mov ecx, 0x10000/4
    ; write "eax" value into "edi" with "ecx" times
    rep stosd
    ; set the first entry in PML4
    ; page map level4(PML4) table -> page directory pointer(PDP) table -> page directory(PD) table -> page(P) table -> physical address
    ; PML4 table: each table has 4kb size and 512 entries, each enrty has 8 bytes
    ; PDP table: each table has 4kb size and 512 entries, each enrty has 8 bytes
    ; PD table: each table has 4kb size and 512 entries, each enrty has 8 bytes
    ; P table: each table has 4kb size and 512 entries, each enrty has 8 bytes
    ; (4kb = 4096 bytes = 512 * 8)
    ; 1 PML4 table has 512 entries which map to 512 PDP tables individually
    ; virtual address has "Page offset"
    ; 4k pages (Page offset => bit-0 to bit-11 = 2^12 (=4k page size), 4-level page table): PML4T, PDPT. PDT and PT
    ; 2m pages (Page offset => bit-0 to bit-20 = 2^21 (=2m page size), 3-level page table): PML4T, PDPT and PDT
    ; 1g pages (Page offset => bit-0 to bit-29 = 2^30 (=1g page size), 2-level page table): PML4T, PDPT
    ; NOTE: we use 1g pages in IA-32e paging model
    ; 0x70000 is PML4 base address
    ; 0x71007 = 1110001 0000 0000 0111 (binary format)
    ; 0x71003 = 1110001 0000 0000 0011 (binary format)
    ; bit-0= 1 (P)
    ; bit-1= 1 (R/W)
    ; bit-2= 0 (U/S)
    ; bit-12 to bit-31 = 1110001 = 0x71 ("page directory pointer table" base address)
    ; set only first entry in PML4
    mov dword[0x70000], 0x71003
    ; 0x71000 is PDP base address
    ; 10000111 = 1000 0111
    ; bit-0= 1 (P)
    ; bit-1= 1 (R/W)
    ; bit-2= 0 (U/S)
    ; bit-7= 1 (1 for 1g pages PDP table entry, 0 for 2m or 4k PDP table entry)
    ; set only first entry in PDP (base address is 0x71000)
    mov dword[0x71000], 10000011b
    ; for high canonical range 
    ; 0xffff800000000000 is lowest part of high canonical range (0xffff800000000000 ~ 0xffffffffffffffff)
    ; >> 39 move to PML4 to lower bits
    mov eax, (0xffff800000000000 >> 39)
    ; 0x1ff = 0000 0000 0001 1111 1111
    ; "and" to get lower 9 bits and save it to eax
    and eax, 0x1ff
    ; now eax is PML4 part of virtual address of 0xffff800000000000
    ; set nth entry in PML4 (n = eax, each entry is 8 bytes)
    mov dword[0x70000 + eax * 8], 0x72003
    ; bit-0= 1 (P)
    ; bit-1= 1 (R/W)
    ; bit-2= 0 (U/S)
    ; bit-7= 1
    ; set first entry (index = 0) in another PDP table (base address is 0x72000)
    ; PDP index = 0 from 0xffff800000000000
    mov dword[0x72000], 10000011b

    lgdt [gdt_64_ptr]
    ; for supporting physical address to 2^36 bytes (=64 gb)
    ; enable the bit-5 which is PAE(physical address extension) in CR4
    mov eax, cr4
    or eax, (1<<5)
    mov cr4, eax
    ; load CR3 with the base physical address of the PML4 (Level 4 Page Map)
    ; the base physical address of PML4 = 0x70000
    mov eax, 0x70000
    mov cr3, eax

    ; enable long mode by setting the LME flag (bit 8) in MSR 0xC0000080 (aka EFER)
    ; LME (Long Mode Enable)
    ; MSR number of IA32_EFER = 0xc0000080
    mov ecx, 0xc0000080
    rdmsr
    or eax, (1<<8)
    wrmsr
    ; enable the paging
    mov eax, cr0
    or eax, (1<<31)
    mov cr0, eax

    jmp 8:LMEntry

PEnd:
    hlt
    jmp PEnd


%include "boot/protected_mode/gdt.asm"
%include "boot/protected_mode/print.asm"

Message32:    db "Protected mode is set"
Message32Len: equ $-Message32

BITS 64
LMEntry:
    mov rsp, 0x7c00

    ; mov rsi, Message64
    ; mov rcx, Message64Len
    ; call print_screen_string64
    ; call clear_screen_string64

SetupKernel:
    cld
    ; since memory address below 0x100000 has some ranges reserved
    ; so copy kernel from source to destination in memory
    ; source address =0x10000
    mov rdi, 0x200000
    mov rsi, 0x10000
    ; kernel
    ; times = 512 (bytes) * 100 (sectors) / 8 (qword)
    mov rcx, 512 * 100 /8
    ; "rep": repeat "rcx" times
    ; "movsq": move qword from address (R|E)SI to (R|E)DI.
    rep movsq

    ; kernel base address = 0xffff800000200000 at high memory location
    mov rax, 0xffff800000200000
    jmp rax

LEnd:
    hlt
    jmp LEnd

%include "boot/long_mode/print.asm"

Message64:    db "Long mode is set"
Message64Len: equ $-Message64
