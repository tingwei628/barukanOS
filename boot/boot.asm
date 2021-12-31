[BITS 16] ; 16-bit in real mode
[ORG 0x7c00] ; start to run at 0x7c00

; initialization of segment registers and stack pointer 
; now, ax=0
start:
    xor ax, ax   
    mov ds, ax
    mov es, ax  
    mov ss, ax
    mov sp, 0x7c00

; Check to see if the INT13 extensions are available for your BIOS
; carry will be clear on return if extensions are there
CheckExtensionsPresent:
    mov [DriveId], dl
    mov ah, 0x41
    mov bx, 0x55aa
    int 0x13
    ; INT13 extensions not found if carry is set
    ; jc , jump if carry (CF=1)
    jc NotAvailable
    cmp bx, 0xaa55
    jne NotAvailable

; load bootloader
LoadLoader:
    ; DS:SI (segment:offset pointer to the DAP, Disk Address Packet)
    ; DS is Data Segment, SI is Source Index
    mov si, ReadPacket
    ; size of Disk Address Packet (set this to 0x10)
    mov word[si], 0x10
    ; number of sectors to read
    mov word[si+2], 1
    ; number of sectors to transfer
    ; transfer buffer (16 bit segment:16 bit offset)
    ; 16 bit offset=0x7e00 (stored in word[si+4])
    ; 16 bit segment=0 (stored in word[si+6])
    ; address => 0 * 16 + 0x7e00 = 0x7e00
    mov word[si+4], 0x7e00
    mov word[si+6], 0
    ; absolute number of the start of the sectors to be read
    ; LBA=1 (the 2nd sector) is the start of sector to be read
    ; lower part of 64-bit starting LBA
    mov dword[si+8], 1
    ; upper part of 64-bit starting LBA
    mov dword[si+12], 0
    ; dl=drive id
    mov dl, [DriveId]
    ; function code, 0x42 = Extended Read Sectors From Drive
    mov ah, 0x42
    int 0x13
    ; Set On Error, Clear If No Error
    jc  ReadError
    mov dl, [DriveId]
    ; jump to bootloader
    jmp 0x7e00

NotAvailable:
ReadError:
    ; function code, which is "Write string"
    mov ah, 0x13
    ; cursor at the end of string
    mov al, 1
    ; bh is page number
    ; bl is color
    ; bh is higher part of bx and bl is lower part of bx
    ; set bl=0xa (0xa=bright green which is character color)
    mov bx, 0xa
    ; dx is string position
    ; dh is row (higher part of dx)
    ; dl is column (lower part of dx)
    ; set dh=0 and dl=0
    xor dx, dx
    ;ES:BP = Offset of string
    mov bp, ErrorMessage
    ; cx is number of characters in string
    mov cx, ErrorMessageLen
    ; BIOS interrupt call
    ; 0x10 = BIOS interrupt number
    int 0x10

End:
    hlt    
    jmp End
     
DriveId:    db 0 ; variable to save drive id
; Message:    db "INT13 extensions are available"
; MessageLen: equ $-Message
ErrorMessage:    db "An error happened in boot process"
ErrorMessageLen: equ $-ErrorMessage
; Disk Address Packet has 16 bytes
ReadPacket: times 16 db 0

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
    db 0, 2, 0
    ; partition type
    db 0xf0
    ; ending of CHS
    ; address of last absolute sector in partition
    db 0xff, 0xff, 0xff
    ; LBA of the first absolute sector
    ; LBA = (C × HPC + H) × SPT + (S − 1)
    ; S(sector)=2, C(cylinder)=0, H(head)=0 => LBA=1
    dd 1
    ; size (number of sectors in partition)
    ; here is (20*16*63-1) * 512 bytes/ (1024 * 1024) = 10 mb
    dd (20*16*63-1)
	
    ; other 3 entries are set to 0
    times (16*3) db 0
    
    ; Boot Record signature
    ; here is the same as dw 0aa55h
    db 0x55
    db 0xaa

	
