.globl system_call
.align 2

nr_system_calls = 2

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
reschedule:
	pushl $ret_from_sys_call
	jmp schedule

.align 2
system_call:
	cmpl $nr_system_calls - 1, %eax
	ja bad_sys_call
	pushl $0
	SAVE_ALL
	pushl %eax
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	popl %eax
	call sys_call_table(, %eax, 4)
	xchgl %eax, 24(%esp)
	movl current, %eax
	cmpl $0, 140(%eax)
	jne reschedule
	cmpl $0, 144(%eax)
	je reschedule
ret_from_sys_call:
	RESTORE_ALL
	addl $4, %esp
	iret

bad_sys_call:
	movl $-1, %eax
	iret

sys_fork:
	pushl %esp
	call do_fork
	addl $4, %esp
	ret

sys_call_table:
	.long sys_write
	.long sys_fork
	.long sys_pause
	.long 0

test_msg:
	.asciz "in system call."
