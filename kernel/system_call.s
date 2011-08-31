/*
 * system_call.s (c) wzt 2011
 *
 */

.globl system_call, ret_from_sys_call
.align 2

nr_system_calls 	= 7
state_offset 		= 140
counter_offset 		= 144

EAX			= 24
ORIG_CS			= 48
ORIG_SS			= 56

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
	cmpl $nr_system_calls, %eax
	ja bad_sys_call
	pushl %eax				# fill error_code.
	SAVE_ALL
	pushl %eax
	movw $0x10, %ax				# set ds, es for kernel data space.
	movw %ax, %ds
	movw %ax, %es
	movw $0x17, %ax				# set fs for user data sapce.
	movw %ax, %fs
	popl %eax
	call *sys_call_table(, %eax, 4)
	xchgl %eax, EAX(%esp)			# save return value to eax in the stack.
	movl current, %eax
	cmpl $0, state_offset(%eax)		# the task can't runable.
	jne reschedule
	cmpl $0, counter_offset(%eax)		# task's counter is zero.
	je reschedule

.align 2
ret_from_sys_call:
	cmpl $0x0f, ORIG_CS(%esp)		# is kernel code.
	jne 1f
	cmpl $0x17, ORIG_SS(%esp)		# is kernel data.
	jne 1f
						# TODO: handle singals.
1:
	RESTORE_ALL
	addl $4, %esp				# for error_code.
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
	.long sys_write_s
	.long sys_fork
	.long sys_pause
	.long sys_creat_task
	.long sys_getpid
	.long sys_write_i
	.long 0

test_msg:
	.asciz "in system call."
