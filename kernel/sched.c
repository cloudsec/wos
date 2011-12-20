#include <wos/gdt.h>
#include <wos/idt.h>
#include <asm/asm.h>
#include <wos/task.h>
#include <wos/sched.h>
#include <wos/mm.h>
#include <wos/unistd.h>
#include <wos/type.h>
#include <wos/debug.h>

extern void timer_interrupt();
extern void system_call();
extern void run_task1(void);
extern unsigned int pg_dir;
extern struct gdt_desc new_gdt[8192];

static inline _syscall0(int, fork)
static inline _syscall0(int, pause)
static inline _syscall1(int, write_s, char*, msg)

unsigned int init_task_stack[PAGE_SIZE] = {0};
struct task_struct init_task;

char user_stack[PAGE_SIZE] = {0};
char init_user_stack[PAGE_SIZE] = {0};

/*
struct {
	long * a;
	short b;
}stack_start __attribute__((__section__(".data.user_stack"))) = {&user_stack[PAGE_SIZE >> 2], 0x10};
*/

struct {
	long *a;
	short b;
}stack_start = {&user_stack, 0x10};

void show_task_status(void)
{
	struct task_struct *tsk = NULL;
	struct list_head *p = NULL;

	list_for_each(p, (&task_list_head)) {
		tsk = list_entry(p, struct task_struct, list);
		if (tsk) {
			printk("Task pid: %d\ttss: 0x%x\tldt: 0x%x\t"
				"counter: %d is running.\n", 
				tsk->pid, tsk->tss_sel, tsk->ldt_sel, tsk->counter); 
		}
	}
}

void schedule(void)
{
        struct task_struct *tsk = NULL, *next = NULL;
        struct list_head *p = NULL;
	int counter = -1;

	for (;;) {
        	list_for_each(p, (&task_list_head)) {
                	tsk = list_entry(p, struct task_struct, list);
                	if (tsk) {
				if ((tsk->state == TASK_RUNABLE) && 	
					(tsk->counter > counter)) {
					counter = tsk->counter;
					next = tsk;
				}
			}
		}
		if (counter == -1) {
			//printk("starting init task.\n");
			next = &init_task;
			break;
		}

		if (counter > 0)
			break;

		//printk("all task counter is zero.\n");
		list_for_each(p, (&task_list_head)) {
			tsk = list_entry(p, struct task_struct, list);
			if (tsk) {
				tsk->counter = DEFAULT_COUNTER;
			}
		}
		counter = 0;
	}
	
	if (next->pid == 1) {
		//printk("choose child process.\n");
	}
	//printk("Choose pid: %d\ttss: 0x%x\n", next->pid, next->tss_sel);
	switch_task(next)
	/* schedule can never get here. */
}

/* 
 * init_task aren't add into the task list, it will not join in the process schedule,
 * it runs run only at the time there is no process choosed by scheduler(). 
 */
void setup_init_task(void)
{
	init_task.tss.prev_task_link = 0;
	init_task.tss.esp0 = init_task_stack + PAGE_SIZE;
	init_task.tss.ss0 = KERNEL_DATA_SEL;
	init_task.tss.esp1 = 0;
	init_task.tss.ss1 = 0;
	init_task.tss.esp2 = 0;
	init_task.tss.ss2 = 0;
	init_task.tss.cr3 = pg_dir;
	init_task.tss.eip = 0;
	init_task.tss.eflags = 0;
	init_task.tss.eax = 0;
	init_task.tss.ebx = 0;
	init_task.tss.ecx = 0;
	init_task.tss.edx = 0;
        init_task.tss.esp = 0;
	init_task.tss.ebp = 0;
	init_task.tss.esi = 0;
	init_task.tss.edi = 0;
	init_task.tss.es = USER_DATA_SEL;
	init_task.tss.cs = USER_CODE_SEL;
	init_task.tss.ss = USER_DATA_SEL;
	init_task.tss.ds = USER_DATA_SEL;
	init_task.tss.fs = USER_DATA_SEL;
	init_task.tss.gs = USER_DATA_SEL;
	init_task.tss.ldt_sel = LDT_SEL(0);
	init_task.tss.io_map = 0x80000000;

	init_task.tss_sel = TSS_SEL(0);
	init_task.ldt_sel = LDT_SEL(0);
	init_task.pid = 0;
	init_task.counter = DEFAULT_COUNTER;
	init_task.priority = DEFAULT_PRIORITY;

        /* setup tss & ldt in the gdt.*/
        set_tss_desc(new_gdt, (unsigned int)&(init_task.tss), TSS_LIMIT, TSS_TYPE, TSS_IDX(0));
        set_ldt_desc(new_gdt, (unsigned int)&(init_task.ldt), LDT_LIMIT, LDT_TYPE, LDT_IDX(0));

        /* setup code & data segment selector in the ldt. */
        set_gdt_desc(init_task.ldt, CODE_BASE, 0x9ffff, USER_CODE_TYPE, 1);
        set_gdt_desc(init_task.ldt, DATA_BASE, 0x9ffff, USER_DATA_TYPE, 2);

	/* the init task's state is not impornt, we nerver use it. */
	init_task.state = TASK_RUNABLE;
}

void init_schedule(void)
{
        INIT_LIST_HEAD(&task_list_head);
	setup_init_task();
	
        current = &init_task;

	asm("pushfl\n\t"
		"andl $0xffffbfff, (%%esp)\n\t"
		"popfl"::);

        /* loading tss register & ldt register. */
        asm("ltrw %%ax\n"::"a"(TSS_SEL(0)));
        asm("lldt %%ax\n"::"a"(LDT_SEL(0)));

	set_intr_gate((unsigned int)timer_interrupt, 32);
	set_system_gate((unsigned int)system_call, 0x85);
}

int sys_getpid(void)
{
	return current->pid;
}

int sys_pause(void)
{
	current->state = TASK_STOP;
	schedule();
	return 0;
}
