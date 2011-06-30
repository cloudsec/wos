AS      =as -I ..include/
LD	=ld
CC	=gcc
CFLAGS	=-pedantic -nostdlib -nostdinc -I include -fomit-frame-pointer -fno-stack-protector

KERNEL_OBJS = boot/load.o core/init.o core/gdt.o core/trap_asm.o core/traps.o core/timer.o \
	core/sched.o core/fork.o core/kthread.o core/system_call.o core/sys.o core/panic.o \
	mm/memory.o mm/slab.o mm/buddy.o mm/page_fault.o fs/fs.o \
	drivers/keyboard.o drivers/hd.o lib/printk.o drivers/vga.o core/spinlock.o lib/string.o

all: boot core mm lib kernel image
boot:
	cd boot/; make;
core:
	cd core/; make;
mm:
	cd mm/;make;

lib:
	cd lib/;make;

kernel: $(KERNEL_OBJS)
	$(AS) -o boot/boot.o boot/boot.s
	$(LD) --oformat binary -N -e _start -Ttext 0x7c00 -o boot.bin boot/boot.o
	$(LD) --oformat binary -N -e startup_32 -Ttext 0x0 -o kernel.bin $(KERNEL_OBJS) 

image:
	cat boot.bin kernel.bin > wos.img
clean:
	cd boot/;make clean;
	cd core/;make clean;
	cd mm/;make clean;
	cd lib/;make clean;
	rm -f wos.img boot.bin kernel.bin
