/*
 * load.s (c) 2011	wzt
 *
 */
.text 
.global startup_32
startup_32:
	call setup_gdt
	call setup_idt
	lgdt new_gdt48
	lidt new_idt48

	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	movl $0x4000, %esp

        xorl %eax, %eax
1:      incl %eax
        movl %eax, 0x000000
        cmpl %eax, 0x100000
        je 1b

	call kernel_start
2:
	jmp 2b

.data
.align 2
new_gdt48:
        .word 8192*8 - 1
        .long new_gdt

.data
.align 2
new_idt48:
	.word 256*8 - 1
	.long new_idt
