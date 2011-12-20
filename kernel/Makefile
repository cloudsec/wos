AS      =as -I ..include/
CC	=gcc
CFLAGS	= -I ../include -pedantic -nostdlib -nostdinc -I include -fomit-frame-pointer -fno-stack-protector

all:
	$(CC) $(CFLAGS) -c fork.c
	$(CC) $(CFLAGS) -c exit.c
	$(CC) $(CFLAGS) -c kthread.c
	$(CC) $(CFLAGS) -c init.c
	$(CC) $(CFLAGS) -c gdt.c
	$(CC) $(CFLAGS) -c sched.c
	$(CC) $(CFLAGS) -c sys.c
	$(CC) $(CFLAGS) -c spinlock.c
	$(CC) $(CFLAGS) -c timer.c
	$(CC) $(CFLAGS) -c traps.c
	$(CC) $(CFLAGS) -c panic.c
	$(AS) -o system_call.o system_call.s
	$(AS) -o trap_asm.o trap_asm.s

clean:
	rm -f *.o
