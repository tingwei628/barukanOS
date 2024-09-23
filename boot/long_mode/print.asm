BITS 64

print_screen_string64:
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    
    mov rdx, vga_start
    xor rdi, rdi

print_screen_string64_loop:
    mov al, [rsi]  
    mov [rdx + rdi], al
    mov byte[rdx + rdi + 1], 0xa
    add rdi, 2
    add rsi, 1
    loop print_screen_string64_loop

    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    ret


clear_screen_string64:
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi 
    
    mov rdx, vga_start
    xor rdi, rdi
    mov rcx, vga_extent

clear_screen_string64_loop:
    
    mov al, space_char
    mov [rdx + rdi], al
    mov byte[rdx + rdi + 1], 0x0
    
    add rdi, 2
    loop clear_screen_string64_loop

    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    ret


space_char: equ 0x20 ; space
vga_start:  equ 0xb8000
vga_extent: equ 80 * 25 ; VGA Memory is 80 chars wide by 25 chars tall (one char is 2 bytes)