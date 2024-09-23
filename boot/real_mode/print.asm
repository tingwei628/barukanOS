BITS 16

print_bios_string16:
    push ax
    push bx
    push dx

    mov ah, 0x13
    mov al, 1
    mov bx, 0xa
    xor dx, dx
    int 0x10
    
    pop dx
    pop bx
    pop ax

    ret


print_screen_string16:
    pusha
    ; screen has 25 lines, and  each line has 80 bytes (Resolution = 80 * 25)
    ; 0xb8000 is first position (top left) on screen
    ; es:di = 0xb8000
    ; 0xb800 * 16 + 0 = 0xb8000
    ; es = 0xb800 (segment) and di = 0 (offset)       
    mov ax, vga_base
    mov es, ax
    xor di, di

print_screen_string16_loop:
    ; copy a char from message to position on screen
    mov al, [si]  
    mov [es:di], al
    ; position = [char:background-color|foreground-color]
    ; upper-half of position is char
    ; lower-half if position is background-color|foreground-color
    ; set char color(0xa) on 2nd byte in position
    mov byte[es:di+1], 0xa
    ; each position on screen has 2 bytes
    ; move to next position on screen
    add di, 2
    ; each char in message has 1 byte
    ; move to next char
    add si, 1
    ; cx = loop count, each time loop, cx = cx - 1
    loop print_screen_string16_loop

    popa
    ret


clear_screen_string16:
    pusha
    mov ax, vga_base
    mov es, ax
    xor di, di
    mov cx, vga_extent

clear_screen_string16_loop:
    
    mov al, space_char
    mov [es:di], al
    mov byte [es:di+1], 0x0
    
    add di, 2
    loop clear_screen_string16_loop

    popa
    ret


space_char: equ 0x20 ; space
vga_base: equ 0xb800
vga_start:  equ 0xb8000
vga_extent: equ 80 * 25 ; VGA Memory is 80 chars wide by 25 chars tall (one char is 2 bytes)