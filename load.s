	.text
	.global startup_32, pg_dir
	.include "kernel.h"
	.org 0
pg_dir:
startup_32:
	movl $0x10, %eax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %ss
	movl $init_stack, %esp

	cld
	movl $0x10200, %esi
	movl $0x200, %edi
	movl $(KERNEL_SECT - 1) << 7, %ecx
	rep
	movsl

	call setup_gdt
	call setup_idt
	lgdt new_gdt48
	lidt new_idt48

	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	movl $init_stack, %esp

	cli
	call init_8259A

        xorl %eax, %eax
1:      incl %eax
        movl %eax, 0x000000
        cmpl %eax, 0x100000
        je 1b

.org 0x1000
pg0:

.org 0x2000
pg1:

.org 0x3000
pg2:

.org 0x4000
pg3:

.org 0x5000

	pushl $kernel_init
	jmp setup_paging

.align 2
setup_paging:
        movl $1024*5,%ecx 
        xorl %eax,%eax
        xorl %edi,%edi 
        cld;rep;stosl
        movl $pg0+7, pg_dir 
        movl $pg1+7, pg_dir+4
        movl $pg2+7, pg_dir+8
        movl $pg3+7, pg_dir+12
        movl $pg3+4092,%edi
        movl $0xfff007,%eax 
        std
3:      stosl             
        subl $0x1000,%eax
        jge 3b
        xorl %eax,%eax 
        movl %eax,%cr3
        movl %cr0,%eax
        orl $0x80000000,%eax
        movl %eax,%cr0
        ret

.align 8
new_gdt48:
        .word 8192*8 - 1
        .long new_gdt

.align 8
new_idt48:
	.word 256*8 - 1
	.long new_idt

	.fill 1024,4,0
init_stack:
	.word init_stack
