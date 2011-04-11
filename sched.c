#include "gdt.h"
#include "asm.h"
#include "task.h"
#include "mm.h"
#include "type.h"

extern unsigned int pg_dir;
extern struct gdt_desc new_gdt[8192];

unsigned int init_task_stack[4096] = {0};
void run_init_task(void);

struct task_struct init_task = {
                {0,
                (unsigned int)&init_task_stack + PAGE_SIZE, KERNEL_DATA_SEL,
                0, 0, 0, 0,                     /* esp1, ss1, esp2, ss2 */
                (unsigned int)&pg_dir, 0, 0,    /* cr3, eip, eflags */
                0, 0, 0, 0,                     /* eax, ecx, edx, ebx */
                0, 0, 0, 0,                     /* esp, ebp, esi, edi */
                USER_DATA_SEL, USER_CODE_SEL, USER_DATA_SEL,
                USER_DATA_SEL, USER_DATA_SEL, USER_DATA_SEL,/* es, cs, ss, ds, fs, gs */
                LDT_SEL(0),                     /* ldt sel */
                0x80000000},
		TSS_SEL(0), LDT_SEL(0),		/* tss_sel, ldt_sel */
		{{0,0},				/* ldt[0] */
		{0x9f,0xc0fa00},		/* ldt[1] */
		{0x9f,0xc0f200}},		/* ldt[2] */
		0, TASK_RUNABLE,		/* pid, state */
		DEFAULT_COUNTER,		/* counter, priority */
		DEFAULT_PRIORITY,
		NULL};			/* task_name */

void run_init_task(void)
{
        char c = 'A';
        int x = 0;

        for (x = 0; ; x += 2) {
                if (x == 3840) {
                        x = 0;
                        continue;
                }

                asm("movw $0x18, %%ax\n\t"
                        "movw %%ax, %%gs\n\t"
                        "movb $0x07, %%ah\n\t"
                        "movb %0, %%al\n\t"
                        "movl %1, %%edi\n\t"
                        "movw %%ax, %%gs:(%%edi)\n\t"
                        ::"m"(c),"m"(x));
        }
}

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
				//printk("%s:%d\n", tsk->task_name, tsk->counter);
				if (tsk->counter > counter) {
					counter = tsk->counter;
					next = tsk;
				}
			}
		}
		//printk("choose pid: %d:%d\n", tsk->pid, tsk->counter);
		if (counter > 0)
			break;

		list_for_each(p, (&task_list_head)) {
			tsk = list_entry(p, struct task_struct, list);
			if (tsk) {
				tsk->counter = DEFAULT_COUNTER;
			}
		}
		counter = -1;
	}
	
	printk("Choose pid: %d\ttss: 0x%x\n", next->pid, next->tss_sel);
/*
        struct {long a, b;} tmp; 
        if (current == next) 
                return ;
        current = next;

        asm("movw %%dx, %1\n\t" 
                "ljmp %0\n"
                ::"m"(*&tmp.a), "m"(*&tmp.b), "d"(next->tss_sel));
*/
}

void init_schedule(void)
{
        INIT_LIST_HEAD(&task_list_head);
	
        /* setup tss & ldt in the gdt.*/
        set_tss_desc(&new_gdt, &(init_task.tss), TSS_LIMIT, TSS_TYPE, TSS_IDX(0));
        set_ldt_desc(&new_gdt, &(init_task.ldt), LDT_LIMIT, LDT_TYPE, LDT_IDX(0));

        /* setup code & data segment selector in the ldt. */
        set_gdt_desc(&(init_task.ldt), CODE_BASE, USER_CODE_LIMIT, USER_CODE_TYPE, 1);
        set_gdt_desc(&(init_task.ldt), DATA_BASE, USER_DATA_LIMIT, USER_DATA_TYPE, 2);

        /* move init_task to the task list. */
        list_add_tail(&(init_task.list), &task_list_head);

        current = &init_task;

        /* loading tss register & ldt register. */
        asm("ltrw %%ax\n"::"a"(TSS_SEL(0)));
        asm("lldt %%ax\n"::"a"(LDT_SEL(0)));
}
