set_handler:
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

    ret


; interrupt handler for "divided by 0"
handler_divided_by_0:
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

    ; mov byte[0xb8000], 'D'
    ; mov byte[0xb8001], 0xc

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
handler_timer:
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

    ; mov byte[0xb8020], 'T'
    ; mov byte[0xb8021], 0xe
    
    ; inc byte[0xb8020]
    ; mov byte[0xb8021], 0xe
    ; jmp End

    ; write to PIC
    ; 0x20 "End of Interrupt"（EOI）
    mov al, 0x20
    out 0x20, al
   
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

; IRQ7 handler
set_irq7:
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

    ; check if it is spurious interrupt
    ; write to PIC
    mov al, 11
    out 0x20, al
    ; read from PIC
    in al, 0x20
    ; ISR bit-7 = 1 (regular interrupt)
    ; ISR bit-7 = 0 (spurious interrupt)
    test al, (1<<7)
    jz .is_spurious_interrupt

    ; write to PIC
    ; 0x20 "End of Interrupt"（EOI）
    mov al, 0x20
    out 0x20, al

.is_spurious_interrupt
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