; start to define GDT in protected mode (each entry has 8 bytes in GDT)
; it has only 3 entries in this course...
; (GDT can be up to 65536 bytes in length (8192 entries))
; null descriptor (the first entry in GDT)

gdt_32:
    ; dq = 8 bytes
    dq 0
; code segment descriptor, offset 8 bytes from base address of GDT
; (the 2nd entry in GDT)
gdt_32_code:
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
gdt_32_data:
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
gdt_32_len: equ $-gdt_32
; pointer to GDT
gdt_32_ptr:
    ; limit (size of GDT)
    dw gdt_32_len-1
    ; base address of GDTs
    dd gdt_32
; pointer to IDT
idt_32_ptr:
    dw 0
    dd 0