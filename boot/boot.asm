[BITS 16] ; 16-bit in real mode
[ORG 0x7c00] ; start to run at 0x7c00

; initialization of segment registers and stack pointer 
; now, ax=0
start:
    xor ax,ax   
    mov ds,ax
    mov es,ax  
    mov ss,ax
    mov sp,0x7c00

PrintMessage:
    ; function code, which is "Write string"
    mov ah,0x13
    ; cursor at the end of string
    mov al,1
    ; bh is page number
    ; bl is color
    ; bh is higher part of bx and bl is lower part of bx
    ; set bl=0xa (0xa=bright green which is character color)
    mov bx,0xa
    ; dx is string position
    ; dh is row (higher part of dx)
    ; dl is column (lower part of dx)
    ; set dh=0 and dl=0
    xor dx,dx
    ;ES:BP = Offset of string
    mov bp,Message
    ; cx is number of characters in string
    mov cx,MessageLen
    ; BIOS interrupt call
    ; 0x10 = BIOS interrupt number
    int 0x10

End:
    hlt    
    jmp End
     
Message:    db "Hello barukanOS"
MessageLen: equ $-Message

; 0x1be = 446 bytes, which is bootloader code size
; $-$$ is current section size (in bytes)
times (0x1be-($-$$)) db 0

; here are partition table entries (which is total 64 bytes)
; there are 4 entries and only define the first entry
; each entry has 16 bytes (64/4=16)
    ; this is the first partition entry format
    ; boot indicator, 0x80=bootable partition
    db 0x80
    ; starting CHS
    ; C=cylinder, H=head, S=sector
    ; the range for cylinder is 0 through 1023
    ; the range for head is 0 through 255 inclusive
    ; The range for sector is 1 through 63
    ; address of the first sector in partition
    ; 0(head),2(sector),0(cylinder)
    db 0,2,0
    ; partition type
    db 0xf0
    ; ending of CHS
    ; address of last absolute sector in partition
    db 0xff, 0xff, 0xff
    ; starting sector (LBA)
    dd 1
    ; size (number of sectors in partition)
    ; here is (20*16*63-1) * 512 bytes/ (1024 * 1024) = 10 mb
    dd (20*16*63-1)
	
    ; other 3 entries are set t0 0
    times (16*3) db 0
    
    ; Boot Record signature
    ; here is the same as dw 0aa55h
    db 0x55
    db 0xaa

	
