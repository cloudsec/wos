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

void run_init_task1(void)
{
        char c = 'B';
        int x = 0;

        for (x = 0; ; x += 2) {
                if (x == 3840) {
                        x = 0;
                        continue;
                }

		x = 4;
                asm("movw $0x18, %%ax\n\t"
                        "movw %%ax, %%gs\n\t"
                        "movb $0x07, %%ah\n\t"
                        "movb %0, %%al\n\t" "movl %1, %%edi\n\t"
                        "movw %%ax, %%gs:(%%edi)\n\t"
                        ::"m"(c),"m"(x));
        }
}

void run_init_task2(void)
{
        char c = 'C';
        int x = 0;

        for (x = 0; ; x += 2) {
                if (x == 3840) {
                        x = 0;
                        continue;
                }

                x = 6;
                asm("movw $0x18, %%ax\n\t"
                        "movw %%ax, %%gs\n\t"
                        "movb $0x07, %%ah\n\t"
                        "movb %0, %%al\n\t" "movl %1, %%edi\n\t"
                        "movw %%ax, %%gs:(%%edi)\n\t"
                        ::"m"(c),"m"(x));
        }
}

void run_init_task3(void)
{
        char c = 'D';
        int x = 0;

        for (x = 0; ; x += 2) {
                if (x == 3840) {
                        x = 0;
                        continue;
                }

                x = 8;
                asm("movw $0x18, %%ax\n\t"
                        "movw %%ax, %%gs\n\t"
                        "movb $0x07, %%ah\n\t"
                        "movb %0, %%al\n\t" "movl %1, %%edi\n\t"
                        "movw %%ax, %%gs:(%%edi)\n\t"
                        ::"m"(c),"m"(x));
        }
}

void kernel_init(void)
{
	init_vga();
	init_8259A();
	init_hd();
	init_trap();
	init_mm();
	init_keyboard(); 
	init_schedule();
	//init_timer(100);

	sti();
	//setup_dpt();
	hd_test();
	for (;;);
/*
	creat_kthread((unsigned int)&run_init_task3);

	printk("Move to ring3.\n");
	MOVE_TO_RING3()

	creat_task((unsigned int)&run_init_task1);
	creat_task((unsigned int)&run_init_task2);
	run_init_task();
*/
}
