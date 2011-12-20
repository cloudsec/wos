AS      =as -I ..include/
CC	=gcc
CFLAGS	= -I ../include -pedantic -nostdlib -nostdinc -I include -fomit-frame-pointer -fno-stack-protector

all:
	$(CC) $(CFLAGS) -c init.c
	$(CC) $(CFLAGS) -c version.c

clean:
	rm -f *.o
