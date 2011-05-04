CC      =gcc
CFLAGS  = -I ../include -pedantic -nostdlib -nostdinc -I include -fomit-frame-pointer -fno-stack-protector

fs: fs.o
	$(CC) $(CFLAGS) -c fs.c

clean:
	rm -f *.o *.map
