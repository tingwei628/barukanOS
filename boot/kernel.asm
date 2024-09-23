BITS 64
ORG 0x200000

start:

    mov rdi, idt_64
    mov rax, handler_divided_by_0
    call set_handler

    ; set IDT entry for Timer
    ; Timer(PIT) trigger IRQ0 of master
    mov rax, handler_timer
    ; the vector number of Timer is 32 in the PIC
    ; since each entry has 16 bytes in IDT
    mov rdi, idt_64 + 32*16
    call set_handler

    ; spurious_interrupt
    ; the vector number of IRQ7 is 32 + 7
    ; set IDT entry for IRQ7
    mov rdi, idt_64 + 32*16 + 7*16
    mov rax, set_irq7
    call set_handler


    ; load Gdt64 to GDTR
    lgdt [gdt_64_ptr]
    ; load Idt64 to IDTR
    lidt [idt_64_ptr]

; copy tss address to tss descriptor
SetTss:
    mov rax, tss_64_ptr
    ; copy lower 16 bits of address to 3rd bytes of base of tss descriptor
    mov [tss_desc + 2], ax
    shr rax, 16
    ; copy "bit-16 to bit-23" of address to 5th bytes of base of tss descriptor
    mov [tss_desc + 4], al
    shr rax, 8
    ; copy "bit-24 to bit-31" of address to 8th bytes of base of tss descriptor
    mov [tss_desc + 7], al
    shr rax, 8
    ; copy "bit-32 to bit-63" of address to 9th bytes of base of tss descriptor
    mov [tss_desc + 8], eax
    
    ; load tss(task state segment) selector into task register 
    ; offset = 0x20 = 32 bytes which is 5th entry(tss descriptor) in GDT
    ; tss selector = 0x20
    mov ax, 0x20
    ; ltr, load into task register
    ltr ax

    ; since rsp=0x7c00
    ; code segment descriptor is 8 bytes offest from start of Gdt64
    push 8
    push KernelEntry
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
    ; mov byte[0xb8000], 'K'
    ; mov byte[0xb8001], 0xa

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

    ; sti(set interrupt flag), enable interrupts
    ; sti

    ; ss(stack segment) selector (after iretq)
    ; offset (0x18=24 bytes) is 4th entry in GDT
    ; "| 3" (set RPL=11b which is Ring3)
    ; load "0x18 | 3" into stack segment register (which has CPL in bit-0 to bit-1)
    push 0x18 | 3
    ; 0x7c00 load into rsp (after iretq)
    push 0x7c00
    ; value(state of cpu) to load into rflags register (after iretq)
    ; set bit-1=1
    ; enable interrupt (bit-9 =1)
    push 0x202
    ; code segement selector in Ring3 to load into cs register (after iretq)
    ; offset, 0x10 = 16 bytes, 3rd entry in GDT
    ; "| 3" (set RPL=11b which is Ring3)
    ; load "0x10 | 3" into code segment register  (which has CPL in bit-0 to bit-1)
    push 0x10 | 3
    ; UserEntry load into rip (after iretq)
    push UserEntry
    ; interrupt return jump from Ring0 to Ring3
    iretq

End:
    hlt
    jmp End

; rip=UserEntry, jump here in Ring3
UserEntry:
    ; mov ax, cs
    ; and al, 11b
    ; is it in Ring3
    ; cmp al, 3
    ; jne UEnd

    ; mov byte[0xb8010],'U'
    ; mov byte[0xb8011],0xE

    ; inc byte[0xb8010]
    ; mov byte[0xb8011],0xE
    ; jmp UserEntry

UEnd:
    ; no hlt in user mode
    jmp UEnd

%include "boot/long_mode/gdt.asm"
%include "boot/long_mode/idt.asm"
%include "boot/long_mode/tss.asm"
%include "boot/long_mode/print.asm"
%include "boot/long_mode/handler.asm"

times (512 * 100 -($-$$)) db 0