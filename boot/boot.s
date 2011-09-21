.text
.globl	_start
.code16
_start:
	xorw %ax, %ax		# boot.lds has linked code at 0x7c00.
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss
	movw $0x2000, %sp	# set stack for 8kb.

	movw $0x03, %ax		# set screen mode. ah = 0x0, al = 0x03, 80*25 text mode.
	int $0x10

	movw $boot_msg, %ax	# msg addr at es:bp
	movw %ax, %bp
	movw $17, %cx		# cx = msg length
	movw $0x1301, %ax	# ah = 0x13, al = 0x01
	movw $0x07, %bx		# attr.
	movw $0, %dx		# coordinate (0, 0)
	int $0x10

	movw $0x00, %ax		# reset hard disk.
	int $0x13
	orb %ah, %ah
	jnz failed

        movw $0x1000, %ax	# load kernel to 0x10000:0x0(64kb).
        movw %ax, %es
        xorw %bx, %bx   	# es:bs destination address
        movw $81, %cx		# 80 sectors is enough.
        movw $1, %si    	# 0 is boot sector
rd_kern:
        call read_sect
        addw $512, %bx
        incw %si
        loop rd_kern

        xorw %ax, %ax		# clear registers. ds,es has been changed before.
        movw %ax, %ds
        movw %ax, %es
        movw %ax, %ss

enable_a20:			# enable a20.
	inb $0x64, %al  
	testb $0x2, %al
	jnz enable_a20
	movb $0xdf, %al
	outb %al, $0x64

	cli			# close interrupt.
	lgdt gdt_48		# load gdtr.

	movl %cr0, %eax		# enter protect mode.
	orl $0x1, %eax
	movl %eax, %cr0

	ljmp $0x08, $start_pm

failed:
	movw $boot_failed, %ax	# msg addr at es:bp.
	movw %ax, %bp
	movw $12, %cx		# cx = msg length.
	movw $0x1301, %ax	# ah = 0x13, al = 0x01.
	movw $0x07, %bx		# attr.
	movw $0x10, %dx		# coordinate (1, 0).
	int $0x10

die:
	jmp die

read_sect:
        pushw %ax
        pushw %cx
        pushw %dx
        pushw %bx

        movw %si, %ax           
        xorw %dx, %dx
        movw $18, %bx   		# 18 sectors per track for floppy disk
        divw %bx
        incw %dx
        movb %dl, %cl   		# cl=sector number
        xorw %dx, %dx
        movw $2, %bx    		# 2 headers per track for floppy disk
        divw %bx

        movb %dl, %dh			# head
        xorb %dl, %dl			# driver
        movb %al, %ch			# cylinder
        popw %bx			# save to es:bx
rp_read:
        movb $0x1, %al			# read 1 sector
        movb $0x2, %ah			
        int $0x13
        jc rp_read
        popw %dx
        popw %cx
        popw %ax
        ret

.code32
start_pm:
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %gs
	mov %ax, %ss
	movl $0x2000, %esp		# set kernel stack 8KB.

	cld				# move kernel from 0x10000(1KB) to 0x100000(1MB).
	movl $0x10000, %esi
	movl $0x100000, %edi
	movl $0x10000, %ecx
	rep movsb

	ljmp $0x08, $0x100000	 	# jmp to startup_32

gdt:
	.quad 0x0000000000000000	# null descriptor
	.quad 0x00cf9a000000ffff	# cs
	.quad 0x00cf92000000ffff	# ds
	.quad 0x0000000000000000	# reserved for further use

gdt_48:
	.word 32
	.long gdt

boot_msg:
	.asciz "Loading kernel..."
boot_failed:
	.asciz "Boot failed."

.org 510
	.word 0xaa55
