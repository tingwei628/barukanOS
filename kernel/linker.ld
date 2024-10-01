ENTRY(start)

SECTIONS
{
    . = 0xffff800000200000;  /* kernel base address at high memory location */
    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    . = ALIGN(16);
    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }
}