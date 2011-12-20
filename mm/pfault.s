.global page_fault

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

page_fault:
        SAVE_ALL
        pushl %eax
        movw $0x10, %ax
        movw %ax, %ds
        movw %ax, %es
        movw %ax, %fs
        popl %eax
        pushl %esp
        call do_page_fault
        addl $4, %esp
        RESTORE_ALL
        iret
