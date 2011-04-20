#include <wos/gdt.h>
#include <wos/asm.h>

int sys_write(char *msg)
{
	printk("%s", msg);
}
