BITS 16 ; 16-bit in real mode
ORG 0x7c00 ; start to run at 0x7c00

; initialization of segment registers and stack pointer 
; now, ax=0
start:
    xor ax, ax   
    mov ds, ax
    mov es, ax  
    mov ss, ax
    mov sp, 0x7c00

    mov byte[DriveId], dl

; load bootloader
LoadBootLoader:
    mov si, DiskReadPacket      
    mov dl, [DriveId]
    mov ah, 0x42
    int 0x13
    ; Set On Error, Clear If No Error
    jc  ReadError
    mov dl, [DriveId]
    ; jump to bootloader
    jmp 0x7e00

NotAvailable:
ReadError:
    mov bp, ErrorMessage
    mov cx, ErrorMessageLen
    call print_bios_string16

End:
    hlt
    jmp $
     
%include "boot/real_mode/print.asm"

DriveId:    db 0 ; variable to save drive id
ErrorMessage:    db "An error happened in boot process"
ErrorMessageLen: equ $-ErrorMessage
; Disk Address Packet has 16 bytes
DiskReadPacket:
    dw 0x10 ; size of Disk Address Packet (set this to 0x10)
    dw 30 ; number of sectors(loader) to read
    dw 0x7e00 ; 16 bit offset=0x7e00
    dw 0x0000 ; 16 bit segment=0
    ; address => 0 * 16 + 0x7e00 = 0x7e00
    dd 1 ; LBA=1 (the 2nd sector) is the start of sector to be read
    dd 0 ;


; $-$$ is current section size (in bytes)
; 0x1be = 446 = 512 - 66 (66 is total size of descriptions of four partitions)
times (0x1be - ($-$$)) db 0

    ; we only used first partition
    ; this is the description of first partition
    db 0x80 ; flag of boot partition
    db 1, 1, 0
    db 0x06 ; type of partition is FAT16
    db 0x0f, 0x3f, 0xca
    dd 0x3f ; initial address of partition in LBA
    db 0x11, 0x1f, 0x03, 0

    ; we do not use other partitions
    times (16 * 3) db 0

    ; Boot Record signature
    ; here is the same as dw 0aa55h
    ; little endian
    db 0x55
    db 0xaa

	
