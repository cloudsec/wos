CC	=gcc
CFLAGS	= -I include/ -pedantic -nostdlib -nostdinc -I include -fomit-frame-pointer -fno-stack-protector

all:
	$(CC) $(CFLAGS) -c printk.c
	$(CC) $(CFLAGS) -c string.c

clean:
	rm -f *.o
