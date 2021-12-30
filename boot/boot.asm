[BITS 16] ; 16-bit in real mode
[ORG 0x7c00] ; start to run at 0x7c00

; initialization of segment registers and stack pointer 
; now, ax is 0
start:
    xor ax,ax   
    mov ds,ax
    mov es,ax  
    mov ss,ax
    mov sp,0x7c00

PrintMessage:
    mov ah,0x13
    mov al,1
    mov bx,0xa
    xor dx,dx
    mov bp,Message
    mov cx,MessageLen 
    int 0x10 ; BIOS interrupt number to call BIOS service

End:
    hlt    
    jmp End
     
Message:    db "Hello barukanOS"
MessageLen: equ $-Message

; 0x1be = 446 bytes, which is bootloader code size
; $-$$ is current section size (in bytes)
times (0x1be-($-$$)) db 0

; here is partition table entries (which is total 64 bytes)
; there're 4 entries and only define the first entry
    ; db 80h
    db 0x80
    db 0,2,0
    ; db 0f0h
    db 0xf0
    ; db 0ffh,0ffh,0ffh
    db 0xff, 0xff, 0xff
    dd 1
    dd (20*16*63-1)
	
    times (16*3) db 0
    
    ; BIOS identifier
    ; here is the same as dw 0aa55h
    db 0x55
    db 0xaa

	
