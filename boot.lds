OUTPUT_ARCH(i386)
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
ENTRY(_start)
SECTIONS
{
        . = 0x7c00;
        .text : {
                *(.text)
                *(.rodata)
        }

        .data : {
                *(.data)
        }
}
