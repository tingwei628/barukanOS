; Task State segment
tss_64_ptr:
    dd 0
    dq 0xffff800000190000 ; at high memory location
    times 88 db 0
    ; IOPB may get the value sizeof(TSS) (which is 104)
    ; if you don't plan to use this io-bitmap further
    dd tss_64_len

tss_64_len: equ $-tss_64_ptr