#include <wos/gdt.h>
#include <wos/asm.h>
#include <wos/task.h>

void panic(char *msg, unsigned int esp)
{
        struct regs *reg = (struct regs *)esp;

        printk("Interrupt [%s] Error:\n===============================\n", msg);
        printk("Error code: 0x%x\n", reg->error_code);
        printk("EIP: 0x%x, EFLAGS: 0x%x\n", reg->orig_eip, reg->eflags);
        printk("General Registers:\n==================================\n");
        printk("eax: 0x%x, ebx: 0x%x, ecx: 0x%x, edx: 0x%x\n",
                reg->eax, reg->ebx, reg->ecx, reg->edx);
        printk("Segment Registers:\n===================================\n");
        printk("cs: 0x%x ds: 0x%x es: 0x%x fs: 0x%x\n", reg->orig_cs, reg->ds,
                reg->es, reg->fs);

        halt();
}

void oops(char *msg)
{
	printk("%s\n", msg);

	halt();
}
