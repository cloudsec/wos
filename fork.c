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

int last_pid = 1;

int get_pid(void)
{
	int pid;

	if (last_pid > MAX_PID)
		return -1;

	pid = ++last_pid;

	return pid;
}

int do_fork(unsigned int esp)
{
	struct regs *reg = (struct regs *)(esp + 4);
	struct task_struct *tsk;
	int pid;

	printk("cs: 0x%x, eip: 0x%x, error_code: 0x%x, ds: 0x%x\n", 
		reg->orig_cs, reg->orig_eip, reg->error_code, reg->ds);
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

	memcpy((void *)tsk, (void *)&init_task, sizeof(struct task_struct));

	tsk->tss.esp0 = (unsigned int)tsk + PAGE_SIZE - 1;
	tsk->tss.eip = reg->orig_eip;
	tsk->tss.eflags = reg->eflags;
	tsk->tss.ldt_sel = TSS_SEL(pid);
	tsk->tss.cr3 = 0;
	tsk->tss.eax = 0;
	tsk->tss.ebx = reg->ebx;
	tsk->tss.ecx = reg->ecx;
	tsk->tss.edx = reg->edx;
	tsk->tss.edx = reg->edx;
	tsk->tss.esp = reg->esp;
	tsk->tss.esi = reg->esi;
	tsk->tss.edi = reg->edi;
	tsk->tss.cs = reg->orig_cs;
	tsk->tss.ds = reg->ds;
	tsk->tss.es = reg->es;
	tsk->tss.fs = reg->fs;
	
	tsk->tss_sel = TSS_SEL(pid);
	tsk->ldt_sel = LDT_SEL(pid);
	tsk->pid = pid;
	tsk->state = TASK_STOP;
	tsk->counter = DEFAULT_COUNTER;
	tsk->priority = DEFAULT_PRIORITY;

        /* setup tss & ldt in the gdt.*/
        set_tss_desc(new_gdt, (unsigned int)&(tsk->tss), TSS_LIMIT, TSS_TYPE, 
		TSS_IDX(pid));
        set_ldt_desc(new_gdt, (unsigned int)&(tsk->ldt), LDT_LIMIT, LDT_TYPE, 
		LDT_IDX(pid));

        /* setup code & data segment selector in the ldt. */
        set_gdt_desc(tsk->ldt, CODE_BASE, USER_CODE_LIMIT, USER_CODE_TYPE, 1);
        set_gdt_desc(tsk->ldt, DATA_BASE, USER_DATA_LIMIT, USER_DATA_TYPE, 2);

	tsk->state = TASK_RUNABLE;
	list_add_tail(&(tsk->list), &task_list_head);

	invalidate();
	return 0;
}
