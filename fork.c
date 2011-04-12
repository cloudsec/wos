#include "gdt.h"
#include "asm.h"
#include "type.h"
#include "list.h"
#include "task.h"
#include "mm.h"

extern struct task_struct init_task;
extern struct list_head task_list_head;

extern struct gdt_desc new_gdt[8192];
extern unsigned int gdt_desc_idx;
extern int task_flag;

void run_task1(void);

int last_pid = 0;

int get_pid(void)
{
	int pid;

	if (last_pid > MAX_PID)
		return -1;

	pid = ++last_pid;

	return pid;
}

int fork_task(unsigned int eip)
{
	struct task_struct *tsk;
	int pid;

	pid = get_pid();
	if (pid == -1) {
		printk("Get pid failed.\n");
		return -1;
	}
	printk("pid: %d\n", pid);

	tsk = (struct task_struct *)alloc_page();
	if (!tsk) {
		printk("Alloc tsk page failed.\n");
		return -1;
	}
	printk("Alloc tsk page at: 0x%x\n", tsk);

	*tsk = *current;
	tsk->tss.esp0 = (unsigned int)tsk + PAGE_SIZE - 1;
	tsk->tss.esp = (unsigned int)alloc_page() + PAGE_SIZE - 1;
	tsk->tss.eip = eip;
	tsk->tss.eflags = 0x3202;
	tsk->tss.ldt_sel = TSS_SEL(pid);
	printk("Alloc stack page at: 0x%x\n", tsk->tss.esp - PAGE_SIZE + 1);
	printk("%d\n", tsk->tss.cr3);
	printk("0x%x, 0x%x\n", tsk->tss.cs, tsk->tss.ds);

	tsk->pid = pid;

	tsk->tss_sel = TSS_SEL(pid);
	tsk->ldt_sel = LDT_SEL(pid);
	tsk->state = TASK_STOP;
	tsk->counter = DEFAULT_COUNTER;
	tsk->priority = DEFAULT_PRIORITY;

	//setup_task_pages(tsk);

        /* setup tss & ldt in the gdt.*/
        set_tss_desc(new_gdt, (unsigned int)&(tsk->tss), TSS_LIMIT, TSS_TYPE, TSS_IDX(pid));
        set_ldt_desc(new_gdt, (unsigned int)&(tsk->ldt), LDT_LIMIT, LDT_TYPE, LDT_IDX(pid));

        /* setup code & data segment selector in the ldt. */
        set_gdt_desc(tsk->ldt, CODE_BASE, USER_CODE_LIMIT, USER_CODE_TYPE, 1);
        set_gdt_desc(tsk->ldt, DATA_BASE, USER_DATA_LIMIT, USER_DATA_TYPE, 2);

	list_add_tail(&(tsk->list), &task_list_head);
	tsk->state = TASK_RUNABLE;
}
