BITS 16

test_A20:
    pusha
    push ds
    push es

    ; if A20 line enabled, bit-20=1
    ; if not enabled, bit-20=0
    ; e.g. 0x107c00 (A20 line enabled) => 0x107c00
    ;      0x107c00 (A20 line NOT enabled) => 0x007c00
    ; 0xffff = 0b1111111111111111 (binary format)
    ; segement ds = 0, offset = 0x7c00 
    ; 0 * 16 + 0x7c00 = 0x7c00 (address)
    ; it has value 0xa200 at address 0x7c00

    mov ax, 0xffff
    mov es, ax
    xor ax, ax
    mov ds, ax

    ; segment es = 0xffff, offset = 0x7c10
    ; es:0x7c10 => 0xffff * 16 + 0x7c10 = 0x107C00

    mov ax, 0xa200
    mov word[ds:0x7c00], ax
    cmp word[es:0x7c10], ax
    jne end_test_A20

    ; because 0x107C00 may just has value 0xa200 before
    ; that's why we need the double check
    ; let's assign different random value "0xb200"
    mov ax, 0xb200
    mov word [ds:0x7c00], ax
    cmp word [es:0x7c10], ax
    je A20_is_disabled
    
A20_is_disabled:
end_test_A20:
    xor ax, ax
    mov es, ax

    pop es
    pop ds
    popa
    ret

get_memory_info:
    pusha
    mov eax, 0xe820 ; 
    mov edx, 0x534d4150 ; SMAP
    mov ecx, 20 ; size of each entry is 20 bytes
    mov dword[0x9000], 0 ; the number of entries will be stored at 0x9000
    mov edi, 0x9008 ; entries stored start at 0x9008
    xor ebx, ebx ; ebx must be 0 to start
    int 0x15
    jc get_mem_done

get_mem_info_loop:
    add edi, 20 ; next entry start
    inc dword[0x9000] ; add entry number
    test ebx, ebx
    jz get_mem_done ; test is end of memeory map

    mov eax, 0xe820
    mov edx, 0x534d4150 ; SMAP
    mov ecx, 20 ; size of each entry is 20 bytes
    int 0x15
    jnc get_mem_info_loop

get_mem_done:
    popa
    ret