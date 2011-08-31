#include <wos/gdt.h>
#include <asm/asm.h>
#include <wos/printk.h>

int sys_write_s(char *msg)
{
	printk("%s", msg);
}

int sys_write_i(int msg)
{
	printk("!%d\n", msg);
}

int sys_printf(char *format, ...)
{
	va_list arg;
	va_start(arg, format);

	return vfprintf(format, arg);
}
