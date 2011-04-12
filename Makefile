AS	=as
LD	=ld
CC	=gcc
CFLAGS	=-pedantic -nostdlib -nostdinc -I include -fomit-frame-pointer -fno-stack-protector

KERNEL_OBJS = load.o init.o fork.o sched.o memory.o traps.o trap_asm.o keyboard.o timer.o	\
		printk.o vga.o string.o

.s.o:
	${AS} -a $< -o $*.o >$*.map

all: kernel.img
kernel.img: boot kernel
	cat boot kernel > kernel.img

boot:boot.o
	$(LD) --oformat binary -N -e _start -Ttext 0x7c00 -o boot boot.o

kernel: $(KERNEL_OBJS)
	$(LD) --oformat binary -N -e startup_32 -Ttext 0x0 -o kernel $(KERNEL_OBJS) 

clean:
	rm -f kernel.img boot init kernel *.o
