#define KERNEL_CODE_TYPE        0x9a
#define KERNEL_DATA_TYPE        0x92
#define USER_CODE_TYPE          0xfa
#define USER_DATA_TYPE          0xf2
#define VIDEO_TYPE              KERNEL_DATA_TYPE

#define DESC_DEFAULT_TYPE       0xc

#define KERNEL_CODE_DESC        0x2
#define KERNEL_DATA_DESC        0x3
#define USER_CODE_DESC          0x4
#define USER_DATA_DESC          0x5
#define VIDEO_DESC              0x6

#define CODE_LIMIT              0xffffffff
#define DATA_LIMIT              CODE_LIMIT

#define CODE_BASE               0x0
#define DATA_BASE               CODE_BASE

#define VIDEO_LIMIT             0xffff
#define VIDEO_BASE              0xb8000

.set CODE_SEL, 0x08	# code segment selector in kernel mode 
.set DATA_SEL, 0x10 # data segment selector in kernel mode
.set IDT_ADDR, 0x80000	# IDT start address
.set IDT_SIZE, (256*8)	# IDT has fixed length
.set GDT_ADDR, (IDT_ADDR+IDT_SIZE)
			# GDT starts after IDT
.set GDT_ENTRIES, 5	# GDT has 5 descriptors
			# null descriptor
			# cs segment descriptor for kernel
			# ds segment descriptor for kernel
			# current process tss
			# current process ldt
.set GDT_SIZE, (8*GDT_ENTRIES)
			# GDT length
.set KERNEL_SECT, 72	# Kernel lenght, counted by sectors
