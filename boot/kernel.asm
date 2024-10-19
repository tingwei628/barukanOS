%include "boot/long_mode/gdt.asm"
%include "boot/long_mode/tss.asm"

section .text
extern kernal_main
global start

start:
    ; load high memory location
    mov rax, gdt_64_ptr
    ; load Gdt64 to GDTR
    lgdt [rax]

; copy tss address to tss descriptor
SetTss:
    mov rax, tss_64_ptr
    ; tss_desc at high memory location
    mov rdi, tss_desc
    ; copy lower 16 bits of address to 3rd bytes of base of tss descriptor
    mov [rdi + 2], ax
    shr rax, 16
    ; copy "bit-16 to bit-23" of address to 5th bytes of base of tss descriptor
    mov [rdi + 4], al
    shr rax, 8
    ; copy "bit-24 to bit-31" of address to 8th bytes of base of tss descriptor
    mov [rdi + 7], al
    shr rax, 8
    ; copy "bit-32 to bit-63" of address to 9th bytes of base of tss descriptor
    mov [rdi + 8], eax
    
    ; load tss(task state segment) selector into task register 
    ; offset = 0x20 = 32 bytes which is 5th entry(tss descriptor) in GDT
    ; tss selector = 0x20
    mov ax, 0x20
    ; ltr, load into task register
    ltr ax

; PIT(Programmable Interval Timer)
; here we use only channel 0 
; https://wiki.osdev.org/Programmable_Interval_Timer
InitPIT:
    ; set al = 0011 0100
    ; bit 0 = 0 (0 = 16-bit binary mode)
    ; bit-1 to bit-3 = 010 (Mode 2, rate generator)
    ; bit-4 to bit-5 = 11 (Access mode, lobyte/hibyte)
    ; bit-6 to bit-7 = 00 (Channel 0)
    mov al, (1 << 2) | (3 << 4)
    ; out =  output to port
    ; the address of mode command register is 0x43
    ; "OUT imm8, AL" => output byte in al to I/O port address imm8
    out 0x43, al
    ; set interval value
    ; count value = 1193182 times -> 11931 times
    ; (1193182 / 100 = 11931)
    mov ax, 11931
    ; the address of data resgister of channel 0 = 0x40
    ; write lower bytes of ax first, then write higher bytes of ax
    ; write al
    out 0x40, al
    ; write ah
    mov al, ah
    out 0x40, al
; PIC(Programmable Interrupt Controller)
InitPIC:
    ; initialise command word(ICW) 1=0x11
    mov al, 0x11
    ; master PIC - command register, i/o port = 0x20
    out 0x20, al
    ; Slave PIC - command register, i/o port = 0xa0
    out 0xa0, al

    ; ICW2: Master PIC vector offset
    ; IRQ number=32 (user defined interrupt vector from 32 to 255)
    ; starting vector of master is 32 (it has 8 IRQ in master)
    mov al, 32
    ; master PIC - data	register, i/o port = 0x21
    out 0x21, al

    ; ICW2: Slave PIC vector offset
    ; starting vector of slave is 40 (it has 8 IRQ in slave)
    mov al, 40
    ; slave PIC - data register, i/o port = 0xa1
    out 0xa1, al
    
    ; ICW3 => tell Master PIC that there is a slave PIC at IRQ2
    ; (0000 0100 =4, bit-2 is set, IRQ2 is used)
    mov al, 4
    ; master PIC - data	register, i/o port = 0x21
    out 0x21, al

    ; ICW3 => tell Slave PIC its cascade identity
    ; (0000 0010 =2, bit-1 is set, IRQ1 is used)
    mov al, 2
    ; slave PIC - data	register, i/o port = 0xa1
    out 0xa1, al

    ; ICW4 => Gives additional information about the environment.
    ; selecting mode (8086)
    mov al, 1
    out 0x21, al
    out 0xa1, al

    ; masking all IRQs in master except IRQ0
    ; (only IRQ0 of master which PIT used fire interrupt)
    mov al, 11111110b
    out 0x21, al
    ; masking all IRQs of slave
    mov al, 11111111b
    out 0xa1, al


    ; since rsp=0x7c00
    ; code segment descriptor is 8 bytes offest from start of Gdt64
    ; KernelEntry is now at high memory location
    mov rax, KernelEntry
    push 8
    push rax
    db 0x48
    ; load code segment descriptor into code segment register
    ; retf = far return
    ; retf => pop RIP, then pop CS
    ; CS:IP => RIP=KernelEntry, CS=8 => 8:KernelEntry
    ; why add this prefix "db 0x48" (REX.W)
    ; The default operand size of retf is 32bits, and
    ; this will pop invalid data into rip and cs registers 
    ; since we pushed 64bits value on stack.
    ; in short, adjust operand size from 32 bits to 64 bits
    retf
  
KernelEntry:
    ; stack is at high memory location
    mov rsp, 0xffff800000200000
    call kernal_main

End:
    hlt
    jmp End

; %include "boot/long_mode/print.asm"
