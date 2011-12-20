CC      =gcc
LD	=ld
CFLAGS  = -I ../include -pedantic -nostdlib -nostdinc -I include -fomit-frame-pointer -fno-stack-protector

memory: memory.o
	$(CC) $(CFLAGS) -c memory.c
	$(CC) $(CFLAGS) -c buddy.c
	$(CC) $(CFLAGS) -c slab.c
	$(CC) $(CFALGS) -o pfault.o pfault.s
clean:
	rm -f *.o *.map
