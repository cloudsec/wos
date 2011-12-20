/*
 * init.c - kernel init functions.
 *
 * (c) 2011	wzt
 *
 */

#include <wos/gdt.h>
#include <asm/asm.h>
#include <wos/task.h>
#include <wos/mm.h>
#include <wos/unistd.h>
#include <wos/string.h>

static _syscall0(int, fork)
static _syscall0(int, exit)
static _syscall0(int, getpid)
static _syscall0(int, pause)
static _syscall1(int, write_s, char*, msg)
static _syscall1(int, write_i, int, msg)
static _syscall1(int, creat_task, unsigned int, eip)

void run_task1(void)
{
/*
	int i = 0;

	for (; i < 100; i++)
		write_s("A");
	//exit();
*/
	for (;;)
		write_s("A");
}

void run_task2(void)
{
	write_s("\nHello, World!");
	for (;;);
}

void run_task3(void)
{
	for (;;)
		write_s("B");
}

void init(void)
{
	if (!fork()) {
		write_s("I'm child task2.\n");
		for (;;)
			;
	}
	else {
		write_s("I'm father task1.\n");
	}

	for (;;);
}

void kernel_start(void)
{
	init_vga();
	init_8259A();
	init_trap();

	wos_version();

	init_mm();
	init_keyboard(); 
	init_schedule();
	init_timer(100);

	//creat_kthread((unsigned int)&run_task3);
	printk("Move to Ring3.\n");

	//timer_test();
	sti();
	move_to_user_mode();
	creat_task((unsigned int)&run_task1);
	creat_task((unsigned int)&run_task3);

/*
	if (!fork()) {
	
	}

	for (;;) {
		write_s("I'm idle task.\n");
		//pause();
	}
*/
}
