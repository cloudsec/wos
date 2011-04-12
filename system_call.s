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
      
/*
system_call:
	cmpl $nr_system_calls - 1, %eax
	ja bad_sys_call
	pushl %eax
	SAVE_ALL
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	call sys_call_table(, %eax, 4)
	RESTORE_ALL
	addl $4, %esp
	iret
*/
system_call:
        SAVE_ALL
        movw $0x10, %ax
        movw %ax, %ds
        movw %ax, %es
        movw %ax, %fs
	pushl $test_msg
	call printk
	addl $4, %esp
        RESTORE_ALL
        iret

bad_sys_call:
	movl $-1, %eax
	iret

sys_call_table:
	.long sys_write
	.long 0

test_msg:
	.asciz "in system call."
