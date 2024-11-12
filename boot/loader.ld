OUTPUT_FORMAT("elf64-x86-64")
ENTRY(start)

SECTIONS
{
    . = 0xffff800000100000;
	
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
