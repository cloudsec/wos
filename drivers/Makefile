CC	=gcc
CFLAGS	= -I ../include -pedantic -nostdlib -nostdinc -I include -fomit-frame-pointer -fno-stack-protector

all:
	$(CC) $(CFLAGS) -c keyboard.c
	$(CC) $(CFLAGS) -c vga.c

clean:
	rm -f *.o
