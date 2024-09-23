; start to define GDT in long mode (each entry has 16 bytes in GDT)
; it has only 2 entries in this course...
; null descriptor (the first entry in GDT)

;    align 4
gdt_64:
    dq 0
; code segment descriptor, offset 8 bytes from base address of GDT
; (the 2nd entry in GDT)
gdt_64_code:
    ; limit low (bit-0 to bit-15)
    dw 0
    ; base low (bit-16 to bit-31)
    ; code segement start at 0
    dw 0
    ; base middle (bit-32 to bit-39)
    db 0
    ; access
    ; 0x98=10011000 (binary format)
    ; bit-40=0 (access, used with virtual memory)
    ; bit-41=0 (readable/writable)
    ; 0 for execute only, 1 for read and execute
    ; bit-42=0 (is conforming)
    ; 0 for "not conforming"(only executed =privilege level field), 1 for conforming (can be executed <= privilege level field)
    ; bit-43=1 (1 for code segment, 0 for data segment)
    ; bit-44=1 (0 for "system", 1 for "code/data" descriptor)
    ; bit-45 to bit-46 = 00 (privilege level, 00(Ring 0))
    ; bit-47=1 (present bit, is the segment in memory, used with virtual memory)
    db 0x98
    ; granularity
    ; 0x20=00100000 (binary format)
    ; bit-48 to bit-51=0000 (limit high, bit-16 to bit-19)
    ; bit-52=0 (reserved)
    ; bit-53=1 (long mode, 1 for 64 bit)
    ; bit-54=0 (segment type, 0 for 16 bit/ 1 for 32 bit)
    ; bit-55=0 (granularity, 1 for 4kb, 0 for 1byte)
    db 0x20
    ; ; base high (bit-56 to bit-63)
    db 0
    
gdt_64_len: equ $-gdt_64
gdt_64_ptr: 
    dw gdt_64_len-1
    dd gdt_64