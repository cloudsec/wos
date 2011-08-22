/*
 * init.c - kernel init functions.
 *
 * (c) 2011	wzt http://wwww.cloud-sec.org
 *
 */

#include <wos/gdt.h>
#include <wos/asm.h>
#include <wos/task.h>
#include <wos/mm.h>
#include <wos/unistd.h>
#include <wos/string.h>

static inline _syscall0(int, fork)
static inline _syscall0(int, pause)
static inline _syscall1(int, write, char*, msg)
static inline _syscall1(int, creat_task, unsigned int, eip)

void run_init_task(void)
{
        char c = 'A';
        int x = 0;

        for (x = 0; ; x += 2) {
                if (x == 3840) {
                        x = 0;
                        continue;
                }

		x = 2;
                asm("movw $0x18, %%ax\n\t"
                        "movw %%ax, %%gs\n\t"
                        "movb $0x07, %%ah\n\t"
                        "movb %0, %%al\n\t"
                        "movl %1, %%edi\n\t"
                        "movw %%ax, %%gs:(%%edi)\n\t"
                        ::"m"(c),"m"(x));
        }
}

/*
 * task1 & task2 are started by user mode process with system call support.
 */
void run_task1(void)
{
	for (;;)
		write("B");
}

void run_task2(void)
{
	for (;;)
		write("C");
}

/*
 * task3 is started by kernel process, it can directly invoke printk.
 */
void run_task3(void)
{
	for (;;)
		printk("D");
}

void kernel_init(void)
{
	init_vga();
	init_8259A();
	//init_hd();
	init_trap();
	init_mm();
	//init_fs();
	init_keyboard(); 
	init_schedule();
	init_timer(100);

	sti();
	//setup_dpt();
	//hd_test();

	creat_kthread((unsigned int)&run_task3);
	printk("Move to ring3.\n");

	timer_test();

	/* start move to ring3 mode. */
	MOVE_TO_RING3()
	
	creat_task((unsigned int)&run_task1);

	/* the below code is our init task. */
	for (;;) {
		write("in idle task.\n");
		pause();
	}
}
