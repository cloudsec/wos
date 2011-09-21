AS      =as -I ..include/
CC	=gcc
CFLAGS	= -I ../include -pedantic -nostdlib -nostdinc -I include -fomit-frame-pointer -fno-stack-protector

all:
	$(CC) $(CFLAGS) -c init.c

clean:
	rm -f *.o
