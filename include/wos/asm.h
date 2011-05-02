#ifndef ASM_H
#define ASM_H

#define cli()			asm("cli\n"::)
#define sti()           	asm("sti\n"::)

#define halt()			asm("cli\n\thlt\n"::)

#define inb(port)		({						\
				unsigned char ret;				\
				asm("inb %%dx, %%al\n":"=a"(ret):"d"(port));	\
				ret;						\
				})

#define outb(value, port)	asm("outb %%al, %%dx\n"::"a"(value),"d"(port))

#define set_cr3(value)		asm("movl %0, %%cr3\n"::"m"(value))
#define invalidate(value)	asm("movl %%eax,%%cr3"::"a" (value))

#endif
