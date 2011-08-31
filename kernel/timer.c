#include <wos/gdt.h>
#include <asm/asm.h>
#include <wos/task.h>
#include <wos/timer.h>
#include <wos/type.h>
#include <wos/debug.h>

extern void io_delay(void);

unsigned int timer_tick = 0;

struct list_head timer_list_head;
struct timer_list timer_list[MAX_TIMER_REQ];

void add_timer(unsigned int jiffies, void (*fn)(void))
{
	struct timer_list *new_timer = NULL;

	if (!fn)
		return ;

	if (jiffies <= 0) {
		fn();
		goto out;
	}

	cli();
	new_timer = (struct timer_list *)kmalloc(sizeof(struct timer_list));
	if (!new_timer)
		goto out;
	printk("allocte new_timer struct at 0x%x\n", new_timer);

	new_timer->jiffies = jiffies;
	new_timer->fn = fn;
	list_add_tail(&(new_timer->list), &timer_list_head);

out:
	sti();
	return ;
}

void del_timer(void (*fn)(void))
{
	struct timer_list *s = NULL;
	struct list_head *p = NULL;

	cli();
	list_for_each(p, (&timer_list_head)) {
		s = list_entry(p, struct timer_list, list);
		if (s->fn == fn) {
			/* we will not break here, beacuse maybe 
			 * there are many same timers in the list. */
			list_del(p);
		}
	}
	sti();
}

void do_timer(unsigned int esp)
{
	struct regs *reg = (struct regs *)esp;
	struct timer_list *s = NULL;
	struct list_head *p = NULL;

	list_for_each(p, (&timer_list_head)) {
		s = list_entry(p, struct timer_list, list);
		if (s && --(s->jiffies) <= 0) {
			s->fn();
			list_del(p);
		}
	}

	timer_tick++;

	if (current && --current->counter > 0)
		return ;
/*
	if (!(reg->orig_cs & 0x3)) {
		return ;
	}
*/

	DbgPrint("pid: %d counter is 0.\n", current->pid);
	//show_task_status();
	schedule();
}

void init_timer(int hz)
{
        unsigned int divisor = 1193180 / hz;

	INIT_LIST_HEAD(&timer_list_head);

        outb(0x36, 0x43);
        outb(divisor&0xff, 0x40);
        outb(divisor>>8, 0x40);
        outb(inb(0x21)&0xfe, 0x21);
}

void __timer_test(void)
{
	printk("hello, world.\n");
}

void __timer_test1(void)
{
	printk("hello, wos.\n");
}

void __timer_test2(void)
{
	printk("hello, hot girl.\n");
}

void timer_test(void)
{
	add_timer(500, __timer_test);
	add_timer(500, __timer_test1);
	add_timer(200, __timer_test2);
}
