.text
.globl	_start
.include "include/wos/kernel.h"
.code16
_start:
	jmpl $0x0, $code
code:
	xorw %ax, %ax
	movw %ax, %ds	# ds = 0x0000
	movw %ax, %ss	# stack segment = 0x0000
	movw $0x1000, %sp	# arbitrary value 
				# used before pmode

	movw $3, %ax
	int $0x10

	## read rest of kernel to 0x10000
	movw $0x1000, %ax
	movw %ax, %es
	xorw %bx, %bx	# es:bs destination address
	movw $72, %cx
	movw $1, %si	# 0 is boot sector
rd_kern:
	call read_sect
	addw $512, %bx
	incw %si
	loop rd_kern

	cli
		
	## move first 512 bytes of kernel to 0x0000
	## it will move rest of kernel to 0x0200,
	## that is, next to this sector
	cld
	movw $0x1000, %ax
	movw %ax, %ds
	movw $0x0000, %ax
	movw %ax, %es
	xorw %si, %si
	xorw %di, %di
	movw $512>>2, %cx
	rep
	movsl

	xorw %ax, %ax
	movw %ax, %ds	# reset ds to 0x0000
	## move	gdt 
	movw $GDT_ADDR>>4,%ax
	movw %ax, %es
	movw $gdt, %si
	xorw %di, %di
	movw $GDT_SIZE>>2, %cx
	rep
	movsl

enable_a20:		
	## The Undocumented PC
	inb $0x64, %al	
	testb $0x2, %al
	jnz enable_a20
	movb $0xdf, %al
	outb %al, $0x64

	lgdt gdt_48

	## enter pmode
	movl %cr0, %eax
	orl $0x1, %eax
	movl %eax, %cr0

	ljmp $0x08, $0x0

	## in:	ax:	LBA address, starts from 0
	##		es:bx address for reading sector
read_sect:
	pushw %ax
	pushw %cx
	pushw %dx
	pushw %bx

	movw %si, %ax		
	xorw %dx, %dx
	movw $18, %bx	# 18 sectors per track 
					# for floppy disk
	divw %bx
	incw %dx
	movb %dl, %cl	# cl=sector number
	xorw %dx, %dx
	movw $2, %bx	# 2 headers per track 
				# for floppy disk
	divw %bx

	movb %dl, %dh	# head
	xorb %dl, %dl	# driver
	movb %al, %ch	# cylinder
	popw %bx		# save to es:bx
rp_read:
	movb $0x1, %al	# read 1 sector
	movb $0x2, %ah
	int $0x13
	jc rp_read
	popw %dx
	popw %cx
	popw %ax
	ret
		
gdt:
	.quad 0x0000000000000000 # null descriptor
	.quad 0x00cf9a000000ffff # cs
	.quad 0x00cf92000000ffff # ds
	.quad 0x0000000000000000 # reserved for further use
gdt_48:
	.word   .-gdt-1
	.long   GDT_ADDR

.org	0x1fe, 0x90
.word	0xaa55
