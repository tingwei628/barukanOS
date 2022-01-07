[BITS 64]
[ORG 0x200000]

start:

    mov rdi, Idt64
    mov rax, Handler0
    ; set Handler0 as handler after triggering "divided by 0" (which is the first entry in IDT)
    ; copy first part of offset (bit-0 to bit-15 in Handler0) to IDT
    ; ax = 2 bytes
    mov [rdi], ax
    ; shift right 16 bit to get second part of offset (bit-48 to bit-63 in Handler0)
    shr rax, 16
    ; copy offset to address rdi+6
    mov [rdi + 6], ax
    ; shift right 16 bit to get third part of offset (bit-64 to bit-95 in Handler0)
    shr rax, 16
    ; copy offset to address rdi+8
    mov [rdi + 8], eax

    ; set IDT entry for Timer
    ; Timer(PIT) trigger IRQ0 of master
    mov rax, Timer
    ; the vector number of Timer is 32 in the PIC
    ; since each entry has 16 bytes in IDT
    add rdi, 32*16
    mov [rdi], ax
    shr rax, 16
    mov [rdi + 6], ax
    shr rax, 16
    mov [rdi + 8], eax

    ; load Gdt64 to GDTR
    lgdt [Gdt64Ptr]
    ; load Idt64 to IDTR
    lidt [Idt64Ptr]

    ; since rsp=0x7c00
    ; code segment descriptor is 8 bytes offest from start of Gdt64
    push 8
    push KernelEntry
    db 0x48
    ; load code segment descriptor into code segment register
    ; retf = far return
    ; retf => pop RIP, then pop CS
    ; CS:IP => RIP=KernelEntry, CS=8 => 8:KernelEntry
    ; why add this prefix "db 0x48"
    ; The default operand size of retf is 32bits, and
    ; this will pop invalid data into rip and cs registers 
    ; since we pushed 64bits value on stack.
    ; in short, adjust operand size from 32 bits to 64 bits
    ; Source: https://wiki.osdev.org/X86-64_Instruction_Encoding
    ; In  "REX prefix" section,
    ; 0x48 = 01001000b
    ; Fixed bit pattern=0100b
    ; W=1(1, a 64-bit operand size is used)
    retf

KernelEntry:
    mov byte[0xb8000], 'K'
    mov byte[0xb8001], 0xa

    ; ; set rbx=0
    ; xor rbx, rbx
    ; ; divided by 0 to cause excetpion
    ; div rbx

; PIT(Programmable Interval Timer)
; here we use only channel 0 
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
    ; selecting mode
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
    push 0x2
    ; code segement selector in Ring3 to load into cs register (after iretq)
    ; offset, 0x10 = 16 bytes, 3rd entry in GDT
    ; "| 3" (set RPL=11b which is Ring3)
    ; load "0x18 | 3" into code segment register  (which has CPL in bit-0 to bit-1)
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
    mov ax, cs
    and al, 11b
    ; is it in Ring3
    cmp al, 3
    jne UEnd

    mov byte[0xb8010],'U'
    mov byte[0xb8011],0xE

UEnd:
    ; no hlt in user mode
    jmp UEnd

; interrupt handler for "divided by 0"
Handler0:
    ; save the state of cpu when interrupt or exception occurs
    push rax
    push rbx  
    push rcx
    push rdx  	  
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov byte[0xb8000], 'D'
    mov byte[0xb8001], 0xc

    ; stop kernel if error happened
    jmp End

    ; restore the state of cpu when interrupt or exception is done
    ; NOTE stack is LIFO
    pop	r15
    pop	r14
    pop	r13
    pop	r12
    pop	r11
    pop	r10
    pop	r9
    pop	r8
    pop	rbp
    pop	rdi
    pop	rsi  
    pop	rdx
    pop	rcx
    pop	rbx
    pop	rax

    ; interrupt return (64-bit)
    iretq

; Timer Handler
Timer:
    push rax
    push rbx  
    push rcx
    push rdx  	  
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov byte[0xb8020], 'T'
    mov byte[0xb8021], 0xe
    jmp End
   
    pop	r15
    pop	r14
    pop	r13
    pop	r12
    pop	r11
    pop	r10
    pop	r9
    pop	r8
    pop	rbp
    pop	rdi
    pop	rsi  
    pop	rdx
    pop	rcx
    pop	rbx
    pop	rax

    iretq

Gdt64:
    ; null descriptor
    dq 0
    ; Ring0 cs descriptor is the same value as "Code64" in loader.asm
    dq 0x0020980000000000
    ; Ring3 cs descriptor (DPL: from 00 to 11)
    dq 0x0020f80000000000
    ; Ring3 data segment scriptor (DPL: from 00 to 11)
    dq 0x0000f20000000000

Gdt64Len: equ $-Gdt64


Gdt64Ptr:
    dw Gdt64Len-1
    ; pointer is 8 bytes here
    dq Gdt64

; IDT (Interrupt descriptor table)
Idt64:
    ; %rep: invoke a multi-line macro multiple times
    ; repeat this block with 256 times
    ; so, there are 256 entries
    ; the entries in the IDT are 16 bytes long (in long mode)
    ; e.g. entry 1 = IDTR Offset + 16
    %rep 256
        ; offset: bit-0 to bit-15
        ; 0000 0000 0000 0000
        dw 0
        ; selector to code segment descriptor in GDT
        ; bit-16 to bit-31
        ; 0000 0000 0000 1000 = 0x8 (which is offset of code segment descriptor in GDT)
        dw 0x8
        ; reserved (bit-32 to bit-39) = 0000 0000
        db 0
        ; 1000 1110
        ; gate type(bit-40 to bit-43)= 1110 (1110 for interrupt gate,  1111 for trap gate)
        ; 0(bit-44) = 0
        ; dpl (bit-45 to bit-46) = 00 (Ring0)
        ; present bit (bit-47) = 1
        db 0x8e
        ; offset: bit-48 to bit-63
        dw 0
        ; offset: bit-64 to bit-95 
        dd 0
        ; reserved: bit-96 to bit-128
        dd 0
    %endrep

Idt64Len: equ $-Idt64

Idt64Ptr:
    dw Idt64Len-1
    ; pointer is 8 bytes here
    dq Idt64