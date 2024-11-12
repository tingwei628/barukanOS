BITS 16
ORG 0x7e00

start:
    ; dl from boot.asm
    mov [DriveId], dl

; ; get memory info
; call get_memory_info

    mov ax, 0x2000
    mov es, ax ; es = 0x2000

; get memory info
GetMemInfoStart:
    mov eax, 0xe820
    ; SMAP
    mov edx, 0x534d4150
    ; size of each entry is 20 bytes
    mov ecx, 20
    ; the number of entries will be stored at 0x20000 = 0x2000 * 16 + 0
    mov dword[es:0], 0

    mov edi, 8 ; entries stored start at 0x20008 = es:di = 0x2000 * 16 + 8
    xor ebx, ebx ; ebx must be 0
    int 0x15
    jc NotAvailable

GetMemInfo:
    ; check if the type of memory is 1 = free memory region
    ; when it is not 1 it means this is not free memory region
    ; then continue to find next block of memory info
    ; typedef struct __attribute__((packed)) {
    ;   uint64_t address;
    ;   uint64_t length;
    ;   uint32_t type;
    ; } E820;
    ; type is at 16 bytes offset
    cmp dword[es:di + 16], 1
    jne Cont
    ; check if higer part of the base address of region is larger than 4GB
    ; when it is not 0 it means this is larger than 4GB 
    ; then continue to find next block of memory info
    ; ref: E820
    cmp dword[es:di + 4], 0
    jne Cont
    ; we want address of free region < 0x30000000
    ; since os.img is from 0x30000000 to 0x30000000 + 100MB
    ; check if address of memory is larger than 0x30000000
    mov eax, [es:di]
    ; when [es:di] > 0x30000000
    ; then continue to find next block of memory info
    cmp eax, 0x30000000
    ja Cont ; ja = Jump if Above
    ; check if length of memory at least >= 4GB (=0x100000000) = 12 bytes
    ; when it is not 0 it means this is the region to load image
    ; then jump to Find
    cmp dword[es:di + 12], 0
    jne Find
    ; the address (eax) add length of memory
    add eax, [es:di + 8]
    ; the size of os.img = 100MB
    ; when eax < (0x30000000 + 100 * 1024 * 1024)
    ; since os.img is from 0x30000000 to 0x30000000 + 100MB
    ; then continue to find next block of memory info
    cmp eax, 0x30000000 + 100 * 1024 * 1024 ; 0x30000000 + 100MB
    jb Cont ; jb = Jump if Less

Find:
    ; set LoadImage = 1
    mov byte[LoadImage], 1

Cont:
    add edi, 20 ; next entry start
    inc dword[es:0] ; add entry number
    test ebx, ebx
    jz GetMemDone ; test is end of memeory map

    mov eax, 0xe820
    mov edx, 0x534d4150 ; SMP
    mov ecx, 20 ; size of each entry is 20 bytes
    int 0x15
    jnc GetMemInfo

GetMemDone:
    ; check if LoadImage is 1
    ; if not 1, then jump ReadError
    cmp byte[LoadImage], 1
    jne ReadError

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
SetupProtectedModeToLoadGDT:
    ; disable interrupts 
    cli
    ; load GDT pointer value from memory to GDTR
    lgdt [gdt_32_ptr]

    ; enable protected mode=1
    mov eax, cr0
    or eax, 1
    mov cr0, eax

; setup for loading os.img
LoadFS:
    ; extend to 4GB limit of segment register fs
    ; 0x10 = 16, which is descriptor of data segment (3rd descriptor = gdt_32_data)
    mov ax, 0x10
    mov fs, ax

    ; switch to real mode
    mov eax, cr0
    ; bit-0 in cr0 is PE (Protection Enable)
    ; 0xfe = 11111110
    ; clear bit-0 of al
    ; if bit-0 = 1 (protection mode), bit-0 = 0 (real mode)
    and al, 0xfe
    mov cr0, eax

; since originally it only access 1MB memory
; now for loading image to access 4GB memory in real mode
BigRealMode:
    sti
    ; os.img CHS = (203/16/63)
    ; total sectors = 203 * 16 * 63
    ; read 100 sectors each time
    ; cx as counter
    mov cx, 203 * 16 * 63 / 100
    ; ebx is start from which sector to read
    xor ebx, ebx
    ; edi is the start address of loading image
    mov edi, 0x30000000
    xor ax, ax
    mov fs, ax

; os.img range from 0x30000000 to 0x40000000(=1GB) in physical memory
; read sectors in os.img
ReadFAT:
    push ecx
    push ebx
    push edi
    push fs

    mov ax, 100 ; read 100 sectors
    call ReadSectors
    ; test al is 0 or not
    ; jnz = if al is not 0, then jump ReadError
    test al, al
    jnz  ReadError

    pop fs
    pop edi
    pop ebx

    ; 512 * 100 = total bytes of 100 sectors
    ; copy 4 bytes each time
    mov cx, 512 * 100 / 4
    ; read from 0x60000 = 0x6000 * 16 + 0
    mov esi, 0x60000

CopyData:
    ; copy data from  [fs:esi] to [fs:edi]
    ; esi = source, edi = destination
    mov eax, [fs:esi]
    mov [fs:edi], eax

    ;  move address to next 4 bytes each time
    add esi, 4
    add edi, 4
    loop CopyData

    ; restore ecx counter for ReadFAT
    pop ecx

    ; ebx is the start of sector to read
    ; move to next 100 sectors
    add ebx, 100
    loop ReadFAT

ReadRemainingSectors:
    push edi
    push fs
    
    ; os.img CHS = (203/16/63)
    ; total sectors = 203 * 16 * 63
    ; ax = remaining sectors % 100
    mov ax, (203 * 16 * 63) % 100
    call ReadSectors
    test al, al
    jnz  ReadError

    pop fs
    pop edi
    
    ; 512 * (203 * 16 * 63) % 100 = total bytes of remaining sectors
    ; copy 4 bytes each time
    mov cx, (((203 * 16 * 63) % 100) * 512) / 4
    mov esi, 0x60000

CopyRemainingData: 
    mov eax, [fs:esi]
    mov [fs:edi], eax

    add esi, 4
    add edi, 4
    loop CopyRemainingData

SetupProtectedModeToLoadIDT:
    cli
    ; load IDT pinter value from memory to IDTR
    lidt [idt_32_ptr]

    ; enable protected mode=1
    mov eax, cr0
    or eax, 1
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

ReadSectors:
    mov si, ReadPacket
    mov word[si], 0x10 ; packet size
    mov word[si+2], ax ; number of sectors(kernel) to read
    mov word[si+4], 0 ; 16 bit offset=0
    mov word[si+6], 0x6000 ; loading sectors into this address
    mov dword[si+8], ebx ; ebx is the start of kernel sector to read
    mov dword[si+0xc], 0
    mov dl, [DriveId]
    mov ah, 0x42 ; function code, 0x42 = Extended Read Sectors From Drive
    int 0x13
    
    ; if read successfully, CF = 0
    ; if read unsuccessfully, CF = 1
    ; setc = Set if Carry
    ; If CF = 1, then set al = 1; if CF=0, then set al=0
    setc al
    ret

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
LoadImage:  db 0
Message:    db "Text mode is set"
MessageLen: equ $-Message

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
    ; bit-2= 1 (U/S)
    ; bit-12 to bit-31 = 1110001 = 0x71 ("page directory pointer table" base address)
    ; set only first entry in PML4
    mov dword[0x70000], 0x71007
    ; 0x71000 is PDP base address
    ; 10000111 = 1000 0111
    ; bit-0= 1 (P)
    ; bit-1= 1 (R/W)
    ; bit-2= 1 (U/S)
    ; bit-7= 1 (1 for 1g pages PDP table entry, 0 for 2m or 4k PDP table entry)
    ; set only first entry in PDP (base address is 0x71000)
    mov dword[0x71000], 10000111b
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
    ; source address = CModule
    ; destination address = 0x100000
    mov rdi, 0x100000
    mov rsi, CModule
    ; kernel
    ; times = 512 (bytes) * 30 (sectors) / 8 (qword)
    ; 30 sectors = size of CModule
    mov rcx, 512 * 30 / 8
    ; "rep": repeat "rcx" times
    ; "movsq": move qword from address (R|E)SI to (R|E)DI.
    rep movsq

    ; kernel base address = 0xffff800000100000 at high memory location
    mov rax, 0xffff800000100000
    jmp rax

LEnd:
    hlt
    jmp LEnd

%include "boot/long_mode/print.asm"

Message64:      db "Long mode is set"
Message64Len:   equ $-Message64

; setup kernel from os.img
CModule:
