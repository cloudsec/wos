#.include "gdt.h"
.align 2
.global divide_error, debug, nmi, int3, overflow, bounds
.global invalid_op, coprocessor_segment_overrun, reserved, irq13
.global double_fault, invalid_TSS, segment_not_present
.global device_not_available, page_fault, timer_interrupt
.global stack_segment, general_protection, parallel_interrupt
.global keyboard_interrupt, hd_interrupt, default_int

.macro SAVE_ALL
	push %fs
	push %es
	push %ds
	pushl %eax
        pushl %ebp
        pushl %edi
        pushl %esi
        pushl %edx
        pushl %ecx
        pushl %ebx
.endm

.macro RESTORE_ALL
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
	pop %ds
	pop %es
	pop %fs
.endm

.align 2
divide_error:
	push $0
	SAVE_ALL
	pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	call do_divide_error
	addl $4, %esp
	RESTORE_ALL
	addl $4, %esp
	iret

.align 2
debug:
        push $0
        SAVE_ALL
	pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_debug
        addl $4, %esp
        RESTORE_ALL
        addl $4, %esp
        iret

.align 2
nmi:
        push $0
        SAVE_ALL
	pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_nmi
        addl $4, %esp
        RESTORE_ALL
        addl $4, %esp
        iret

.align 2
int3:
        push $0
        SAVE_ALL
	pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_int3
        addl $4, %esp
        RESTORE_ALL
        addl $4, %esp
        iret

.align 2
overflow:
        push $0
        SAVE_ALL
	pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_overflow
        addl $4, %esp
        RESTORE_ALL
        addl $4, %esp
        iret

.align 2
bounds:
        push $0
        SAVE_ALL
	pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_bounds
        addl $4, %esp
        RESTORE_ALL
        addl $4, %esp
        iret

.align 2
invalid_op:
        push $0
        SAVE_ALL
	pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_invalid_op
        addl $4, %esp
        RESTORE_ALL
        addl $4, %esp
        iret

.align 2
coprocessor_segment_overrun:
        push $0
        SAVE_ALL
	pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_coprocessor_segment_overrun
        addl $4, %esp
        RESTORE_ALL
        addl $4, %esp
        iret

.align 2
reserved:
        push $0
        SAVE_ALL
	pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_reserved
        addl $4, %esp
        RESTORE_ALL
        addl $4, %esp
        iret

.align 2
irq13:
        push $0
        SAVE_ALL
	pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_irq13
        addl $4, %esp
        RESTORE_ALL
        addl $4, %esp
        iret

.align 2
device_not_available:
        push $0
        SAVE_ALL
        pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_device_not_available
        addl $4, %esp
        RESTORE_ALL
        addl $4, %esp
	iret

.align 2
timer_interrupt:
	push $0
        SAVE_ALL
	pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        movb $0x20, %al
	outb %al, $0x20
	call do_timer
	add $4, %esp
	RESTORE_ALL
	add $4, %esp
        iret

.align 2
keyboard_interrupt:
        SAVE_ALL
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	call do_keyboard
	RESTORE_ALL
	#iret

.align 2
hd_interrupt:
	SAVE_ALL
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movb $0x20, %al
	outb %al, $0xa0
	jmp 1f
1:	jmp 1f
1:	call do_hd
	RESTORE_ALL
	iret

.align 2
parallel_interrupt:
        pushl %eax
        movb $0x20,%al
        outb %al,$0x20
        popl %eax
        iret

.align 2
double_fault:
        SAVE_ALL
        pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_double_fault
        addl $4, %esp
        RESTORE_ALL
        iret

.align 2
invalid_TSS:
        SAVE_ALL
        pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_invalid_tss
        addl $4, %esp
        RESTORE_ALL
        iret

.align 2
segment_not_present:
        SAVE_ALL
        pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_segment_not_present
        addl $4, %esp
        RESTORE_ALL
        iret

.align 2
stack_segment:
        SAVE_ALL
        pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_stack_segment
        addl $4, %esp
        RESTORE_ALL
        iret

.align 2
general_protection:
        SAVE_ALL
        pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_general_protection
        addl $4, %esp
        RESTORE_ALL
        iret

.align 2
page_fault:
        SAVE_ALL
        pushl %esp
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
        call do_page_fault
        addl $4, %esp
        RESTORE_ALL
        iret

.align 2
default_int:
        pushl %eax
        pushl %ecx
        pushl %edx
        push %ds
        push %es
        push %fs
        movl $0x10, %eax
        mov %ax, %ds
        mov %ax, %es
        mov %ax, %fs
        pushl $default_int_msg
        call printk
        popl %eax
        pop %fs
        pop %es
        pop %ds
        popl %edx
        popl %ecx
        popl %eax
        iret

.align 2
default_int_msg:
	.asciz "Unkown Interrupt.\n"
