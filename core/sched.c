#include <wos/gdt.h>
#include <wos/asm.h>
#include <wos/task.h>
#include <wos/sched.h>
#include <wos/mm.h>
#include <wos/unistd.h>
#include <wos/type.h>

extern void timer_interrupt();
extern void system_call();
extern void run_init_task1(void);
extern unsigned int pg_dir;
extern struct gdt_desc new_gdt[8192];

static inline _syscall0(int, fork)
static inline _syscall0(int, pause)
static inline _syscall1(int, write, char*, msg)

unsigned int init_task_stack[1024] = {0};
unsigned int init_task_stack_ring3[1024] = {0};
struct task_struct init_task;

unsigned int init_task1_stack[1024] = {0};
unsigned int init_task1_stack_ring3[1024] = {0};
struct task_struct init_task1;

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
	int counter = 0;

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
	
	//printk("Choose pid: %d\ttss: 0x%x\n", next->pid, next->tss_sel);
	switch_task(next)
	/* schedule can never get here. */
}

void setup_init_task(void)
{
	init_task.tss.prev_task_link = 0;
	init_task.tss.esp0 = (unsigned int)&init_task_stack + PAGE_SIZE - 1;
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
        init_task1.tss.esp = (unsigned int)&init_task_stack_ring3 + PAGE_SIZE;
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
        set_gdt_desc(init_task.ldt, CODE_BASE, USER_CODE_LIMIT, USER_CODE_TYPE, 1);
        set_gdt_desc(init_task.ldt, DATA_BASE, USER_DATA_LIMIT, USER_DATA_TYPE, 2);

	init_task.state = TASK_RUNABLE;
        /* move init_task to the task list. */
        list_add_tail(&(init_task.list), &task_list_head);
}

void setup_init_task1(void)
{
        init_task1.tss.prev_task_link = 0;
        init_task1.tss.esp0 = (unsigned int)&init_task1_stack + PAGE_SIZE;
        init_task1.tss.ss0 = KERNEL_DATA_SEL;
        init_task1.tss.esp1 = 0;
        init_task1.tss.ss1 = 0;
        init_task1.tss.esp2 = 0;
        init_task1.tss.ss2 = 0;
        init_task1.tss.cr3 = pg_dir;
        init_task1.tss.eip = run_init_task1;
        init_task1.tss.eflags = 0x200;
        init_task1.tss.eax = 0;
        init_task1.tss.ebx = 0;
        init_task1.tss.ecx = 0;
        init_task1.tss.edx = 0;
        init_task1.tss.esp = (unsigned int)&init_task1_stack_ring3 + PAGE_SIZE;
        init_task1.tss.ebp = 0;
        init_task1.tss.esi = 0;
        init_task1.tss.edi = 0;
        init_task1.tss.es = USER_DATA_SEL;
        init_task1.tss.cs = USER_CODE_SEL;
        init_task1.tss.ss = USER_DATA_SEL;
        init_task1.tss.ds = USER_DATA_SEL;
        init_task1.tss.fs = USER_DATA_SEL;
        init_task1.tss.gs = USER_DATA_SEL;
        init_task1.tss.ldt_sel = LDT_SEL(1);
        init_task1.tss.io_map = 0x80000000;

        init_task1.tss_sel = TSS_SEL(1);
        init_task1.ldt_sel = LDT_SEL(1);
        init_task1.pid = 1;
        init_task1.state = TASK_RUNABLE;
        init_task1.counter = DEFAULT_COUNTER;
        init_task1.priority = DEFAULT_PRIORITY;

        /* setup tss & ldt in the gdt.*/
        set_tss_desc(new_gdt, (unsigned int)&(init_task1.tss), TSS_LIMIT, TSS_TYPE, TSS_IDX(1));
        set_ldt_desc(new_gdt, (unsigned int)&(init_task1.ldt), LDT_LIMIT, LDT_TYPE, LDT_IDX(1));

        /* setup code & data segment selector in the ldt. */
        set_gdt_desc(init_task1.ldt, CODE_BASE, USER_CODE_LIMIT, USER_CODE_TYPE, 1);
        set_gdt_desc(init_task1.ldt, DATA_BASE, USER_DATA_LIMIT, USER_DATA_TYPE, 2);

        /* move init_task to the task list. */
        list_add_tail(&(init_task1.list), &task_list_head);
}

void init_schedule(void)
{
        INIT_LIST_HEAD(&task_list_head);
	setup_init_task();
	//setup_init_task1();
	
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

int sys_pause(void)
{
	current->state = TASK_STOP;
	schedule();
	return 0;
}
