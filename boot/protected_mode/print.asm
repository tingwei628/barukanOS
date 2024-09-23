BITS 32

print_screen_string32:
    pushad      
    mov edx, vga_start
    xor edi, edi

print_screen_string32_loop:
    mov al, [esi]  
    mov [edx + edi], al
    mov byte[edx + edi + 1], 0xa
    add edi, 2
    add esi, 1
    loop print_screen_string32_loop

    popad
    ret


clear_screen_string32:
    pushad
    mov edx, vga_start
    xor edi, edi
    mov ecx, vga_extent

clear_screen_string32_loop:
    
    mov al, space_char
    mov [edx + edi], al
    mov byte[edx + edi + 1], 0x0
    
    add edi, 2
    loop clear_screen_string32_loop

    popad
    ret


space_char: equ 0x20 ; space
vga_start:  equ 0xb8000
vga_extent: equ 80 * 25 ; VGA Memory is 80 chars wide by 25 chars tall (one char is 2 bytes)