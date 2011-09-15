/* OUTPUT_ARCH(i386) */
OUTPUT_FORMAT("binary")
ENTRY(startup_32)
SECTIONS
{
        . = 0x0;
        .text : {
                *(.text)
                . = ALIGN(4096);
        }

        .data : {
                *(.rodata)
                *(.data)
                . = ALIGN(4096);
        }

        .bss : {
                *(.bss)
                . = ALIGN(4096);
        }
}
