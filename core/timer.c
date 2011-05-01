#include <wos/gdt.h>
#include <wos/asm.h>
#include <wos/task.h>

extern void io_delay(void);

unsigned int timer_tick = 0;

void do_timer(unsigned int esp)
{
	struct regs *reg = (struct regs *)esp;

	timer_tick++;

	if (--current->counter > 0)
		return ;
/*
	if (!(reg->orig_cs & 0x3)) {
		return ;
	}
*/

	//printk("pid: %d counter is 0.\n", current->pid);
	schedule();
}

void init_timer(int hz)
{
        unsigned int divisor = 1193180 / hz;

        outb(0x36, 0x43);
        outb(divisor&0xff, 0x40);
        outb(divisor>>8, 0x40);
        outb(inb(0x21)&0xfe, 0x21);
}
