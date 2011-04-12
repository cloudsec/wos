#include "gdt.h"
#include "asm.h"

int sys_write(char *msg)
{
	printk("%s", msg);
}
