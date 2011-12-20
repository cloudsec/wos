#include <wos/gdt.h>
#include <asm/asm.h>
#include <wos/task.h>

void panic(char *msg, unsigned int esp)
{
        struct regs *reg = (struct regs *)esp;

	printk("\n------------------------------------------\n");
        printk("Kernel Panic: %s\tPid: %d\n", msg, current->pid);
	printk("--------------------------------------------\n");
        printk("error code: 0x%x eip: 0x%x esp: 0x%x eflags: 0x%x\n", 
		reg->error_code, reg->orig_eip, reg->esp, reg->eflags);
        printk("\nGeneral Registers:\n");
        printk("eax: 0x%x ebx: 0x%x ecx: 0x%x edx: 0x%x\n",
                reg->eax, reg->ebx, reg->ecx, reg->edx);
        printk("\nSegment Registers:\n");
        printk("cs: 0x%x ds: 0x%x es: 0x%x ss: 0x%x fs: 0x%x\n", 
		reg->orig_cs, reg->ds, reg->es, reg->ss, reg->fs);
	printk("--------------------------------------------\n");

        halt();
}

void oops(char *msg)
{
	printk("%s\n", msg);

	halt();
}
