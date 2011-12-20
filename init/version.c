#include <wos/gdt.h>
#include <asm/asm.h>
#include <wos/task.h>
#include <wos/string.h>
#include <wos/version.h>

void wos_version(void)
{
	printk("Welcome to WOS-%s\n\n", WOS_VERSION);
}
