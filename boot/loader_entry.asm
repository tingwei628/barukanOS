section .text
extern EMain
global start

start:
    mov rsp, 0xffff800000200000
    call EMain

    mov rax, 0xffff800000200000
    jmp rax
    
End:
    hlt
    jmp End


