LOAD_OFFSET = 0x0;
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(startup_32)
SECTIONS
{
	. = (LOAD_OFFSET + 0x100000);
  	_text = .;			
  	.text : AT(ADDR(.text) - LOAD_OFFSET) {
		*(.text)
	} = 0x9090
	_etext = .;			

	.rodata : AT(ADDR(.rodata) - LOAD_OFFSET) {
		*(.rodata)
		*(.rodata.*)
	}

	. = ALIGN(16);
	.data : AT(ADDR(.data) - LOAD_OFFSET) {	
		*(.data)
		CONSTRUCTORS
	}
	_edata = .;

	. = ALIGN(16);
	__bss_start = .;
	.bss : AT(ADDR(.bss) - LOAD_OFFSET) {
		*(.bss)
	}
	_end = .;
}
