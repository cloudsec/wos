AS      =as -I ..include/
LD	=ld
CC	=gcc
CFLAGS	=-pedantic -nostdlib -nostdinc -I include -fomit-frame-pointer -fno-stack-protector

KERNEL_OBJS = boot/load.o init/init.o kernel/gdt.o kernel/trap_asm.o kernel/traps.o kernel/timer.o \
	kernel/sched.o kernel/system_call.o kernel/kthread.o kernel/fork.o kernel/sys.o kernel/panic.o \
	mm/memory.o mm/slab.o mm/buddy.o mm/page_fault.o fs/fs.o \
	drivers/keyboard.o drivers/hd.o lib/printk.o drivers/vga.o kernel/spinlock.o lib/string.o

all: boot init kernel mm lib wos image
boot:
	cd boot/; make;
init:
	cd init/; make;
kernel:
	cd kernel/; make;
mm:
	cd mm/;make;

lib:
	cd lib/;make;

wos: $(KERNEL_OBJS)
	$(AS) -o boot/boot.o boot/boot.s
	#$(LD) --oformat binary -N -e _start -Ttext 0x7c00 -o boot.bin boot/boot.o
	#$(LD) --oformat binary -N -e startup_32 -Ttext 0x0 -o kernel.bin $(KERNEL_OBJS) 
	$(LD) -o boot.bin boot/boot.o -N -m elf_i386 -T boot.lds
	$(LD) -o kernel.bin $(KERNEL_OBJS) -N -s -x -M -m elf_i386 -T kernel.lds > System.map

image:
	objcopy -O binary boot.bin boot.img
	objcopy -O binary kernel.bin kernel.img
	cat boot.img kernel.img > wos.img
clean:
	cd boot/;make clean;
	cd kernel/;make clean;
	cd mm/;make clean;
	cd lib/;make clean;
	rm -f wos.img boot.bin kernel.bin
