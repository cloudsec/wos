#include <wos/gdt.h>
#include <wos/asm.h>
#include <wos/task.h>
#include <wos/mm.h>
#include <wos/unistd.h>
#include <wos/string.h>

extern struct task_struct init_task;
extern void default_int(void);
extern void run_init_task(void);
extern void run_init_task1(void);
extern void run_init_task2(void);

struct task_struct task1;
unsigned int task1_stack0[1024];
unsigned int task1_stack3[1024];

struct task_struct task2;
unsigned int task2_stack0[1024];
unsigned int task2_stack3[1024];

extern unsigned int init_task_stack_ring3[1024];

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

void kernel_init(void)
{
	init_vga();
	init_8259A();
	init_trap();
	init_mm();
	init_keyboard(); 
	init_schedule();
	init_timer(100);

	sti();
	creat_kthread((unsigned int)&run_init_task2);

	printk("Start init_task.\n");
	asm("pushl $0x17\n\t"
                "pushl %%eax\n\t"
                "pushfl\n\t"
                "pushl $0x0f\n\t"
                "pushl $1f\n\t"
                "iret\n\t"
                "1:\n\t"
                "movw $0x17, %%ax\n\t"
                "movw %%ax, %%ds\n\t"
                "movw %%ax, %%es\n\t"
                "movw %%ax, %%fs\n\t"
                "movw %%ax, %%gs\n"
                ::"a"(init_task_stack_ring3 + sizeof(init_task_stack_ring3)));
/*
	if (!fork()) {
		write("i'm child process.\n");
		//for (;;);
	}
	write("i'm father process.\n");
	run_init_task();
*/
	creat_task((unsigned int)&run_init_task1);
	run_init_task();
}
