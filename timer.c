#include "gdt.h"
#include "asm.h"
#include "task.h"

extern void io_delay(void);

unsigned int timer_tick = 0;

void init_timer(int hz)
{
        unsigned int divisor = 1193180 / hz;

	outb(0x36, 0x43);
	io_delay();

	outb(divisor&0xff, 0x40);
	io_delay();

	outb(divisor>>8, 0x40);
	io_delay();
	
	outb(inb(0x21)&0xfe, 0x21);
	io_delay();
}

void do_timer(unsigned int esp)
{
	struct regs *reg = (struct regs *)esp;

	timer_tick++;

	//printk("%d", timer_tick);
	if (--current->counter > 0)
		return ;
	current->counter = 0;

	if (!(reg->orig_cs & 0x3)) {
		//printk("cpl: %d\n", reg->orig_cs & 0x3);
		return ;
	}

	//printk("%s counter is 0.\n", current->task_name);
	schedule();
}
