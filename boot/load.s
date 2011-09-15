.text 
.global startup_32
startup_32:
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	#lss stack_start, %esp
	mov %ax, %ss
	movl $init_stack, %esp
	
	cld					# move the reset kernel to 0x200.
	movl $0x10200, %esi
	movl $0x200, %edi
	movl $0x4000, %ecx			# current kernel is not bigger than 64kb.
	rep movsl

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
	movl $init_stack, %esp
	#lss stack_start, %esp

        xorl %eax, %eax
1:      incl %eax
        movl %eax, 0x000000
        cmpl %eax, 0x100000
        je 1b

.align 2
	call kernel_start
2:
	jmp 2b

#.data
.align 2
new_gdt48:
        .word 8192*8 - 1
        .long new_gdt

#.data
.align 2
new_idt48:
	.word 256*8 - 1
	.long new_idt

	.fill 1024,4,0
init_stack:
	.word init_stack
