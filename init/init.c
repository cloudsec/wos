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

static inline _syscall0(int, fork)
static inline _syscall0(int, getpid)
static inline _syscall0(int, pause)
static inline _syscall1(int, write_s, char*, msg)
static inline _syscall1(int, write_i, int, msg)
static inline _syscall1(int, creat_task, unsigned int, eip)

extern char init_user_stack[PAGE_SIZE];

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

void run_task2(void);

/*
 * task1 & task2 are started by user mode process with system call support.
 */
void run_task1(void)
{
	if (!fork()) {
		write_s("cccccccc\n");
		for (;;)
			;//write_s("B");
	}
	else {
		write_s("dddddddd\n");
		for (;;)
			;//write_s("C");
	}
}

void run_task2(void)
{
	for (;;)
		write_s("D");
}

/*
 * task3 is started by kernel process, it can directly invoke printk.
 */
void run_task3(void)
{
	for (;;)
		printk("E");
}

void init(void)
{
	if (!fork()) {
		write_s("I'm child task2.\n");
		for (;;)
			;//write_s("I'm child task1.\n");
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
	//init_hd();
	init_trap();
	init_mm();
	//init_fs();
	//init_keyboard(); 
	init_schedule();
	init_timer(100);

	//setup_dpt();
	//hd_test();

	//creat_kthread((unsigned int)&run_task3);
	printk("Start idle task.\n");

	//timer_test();
	sti();
	MOVE_TO_RING3()
/*
	asm("movl $init_user_stack, %%eax\n\t"     \
		"pushl $0x17\n\t"       \
		"pushl %%eax\n\t"       \
		"pushfl\n\t"            \
		"pushl $0x0f\n\t"       \
		"pushl $1f\n\t"         \
		"iret\n\t"              \
		"1:\n\t"                \
		"movw $0x17, %%ax\n\t"  \
		"movw %%ax, %%ds\n\t"   \
		"movw %%ax, %%es\n\t"   \
		"movw %%ax, %%fs\n\t"   \
		"movw %%ax, %%gs\n"     \
		::);
*/
	//creat_task((unsigned int)&run_task2);

	if (!fork()) {
		//init();
		//write_s("I'm child task.\n");
	}

	for (;;) {
		//write_s("I'm idle task.\n");
		//pause();
	}
}
