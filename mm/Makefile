CC      =gcc
CFLAGS  = -I ../include -pedantic -nostdlib -nostdinc -I include -fomit-frame-pointer -fno-stack-protector

memory: memory.o
	$(CC) $(CFLAGS) -c memory.c

clean:
	rm -f *.o *.map
