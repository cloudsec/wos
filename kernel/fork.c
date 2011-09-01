/*
 * fork.c
 *
 * (c) 2011	wzt http://www.cloud-sec.org
 *
 */

#include <wos/gdt.h>
#include <asm/asm.h>
#include <wos/type.h>
#include <wos/list.h>
#include <wos/task.h>
#include <wos/mm.h>
#include <wos/debug.h>

extern void ret_from_sys_call(void);
extern struct task_struct init_task;
extern struct list_head task_list_head;
extern unsigned int pg_dir;

extern struct gdt_desc new_gdt[8192];
extern unsigned int gdt_desc_idx;
extern int task_flag;

int last_pid = 0;

int get_pid(void)
{
	int pid;

	if (last_pid > MAX_PID)
		return -1;

	pid = ++last_pid;

	return pid;
}

int task_clone(unsigned int eip)
{
	struct task_struct *tsk;
        int pid;

        pid = get_pid();
        if (pid == -1) {
                DbgPrint("Get pid failed.\n");
                return -1;
        }
        DbgPrint("pid: %d\n", pid);

        tsk = (struct task_struct *)alloc_page(1);
        if (!tsk) {
                DbgPrint("Alloc tsk page failed.\n");
                return -1;
        }
        DbgPrint("Alloc tsk page at: 0x%x\n", tsk);

	memcpy(tsk, &init_task, sizeof(struct task_struct));

        tsk->tss.esp0 = (unsigned int)alloc_page(1) + PAGE_SIZE;
        tsk->tss.eip = eip;
        tsk->tss.eflags = 0x200;
        tsk->tss.ldt_sel = TSS_SEL(pid);
        tsk->tss.cr3 = pg_dir;
        tsk->tss.esp = (unsigned int)alloc_page(1) + PAGE_SIZE;

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

        return 0;
}

int do_fork(unsigned int esp)
{
        struct task_struct *tsk;
	struct regs *reg = (struct regs *)(esp + 4);
        int pid;

	DbgPrint("cs: 0x%x, eip: 0x%x, ds: 0x%x, eax: 0x%x, ebx: 0x%x\n"
		"ecx: 0x%x, es: 0x%x, fs: 0x%x, eflags: 0x%x\n"
		"ss: 0x%x, esp: 0x%x, orig_eax: 0x%x\n",
		reg->orig_cs, reg->orig_eip, reg->ds, reg->eax, reg->ebx, 
		reg->ecx, reg->es, reg->fs, reg->eflags, 
		reg->ss, reg->esp, reg->error_code);

        pid = get_pid();
        if (pid == -1) {
                printk("Get new pid failed.\n");
                return -1;
        }
        printk("Get new pid: %d\n", pid);

        tsk = (struct task_struct *)alloc_page(1);
        if (!tsk) {
                DbgPrint("Alloc tsk page failed.\n");
                return -1;
        }
        printk("Alloc task_struct page at: 0x%x\n", tsk);

	*tsk = *current;
        tsk->tss.prev_task_link = 0;
        tsk->tss.esp0 = tsk + PAGE_SIZE * 2;
        tsk->tss.ss0 = KERNEL_DATA_SEL;
        tsk->tss.esp1 = 0;
        tsk->tss.ss1 = 0;
        tsk->tss.esp2 = 0;
        tsk->tss.ss2 = 0;
        tsk->tss.eip = reg->orig_eip;
        tsk->tss.eflags = reg->eflags;
        tsk->tss.eax = 0;
        tsk->tss.ebx = reg->ebx;
        tsk->tss.ecx = reg->ecx;
        tsk->tss.edx = reg->edx;
        tsk->tss.esp = reg->esp;
        tsk->tss.ebp = reg->ebp;
        tsk->tss.esi = reg->esi;
        tsk->tss.edi = reg->edi;
        tsk->tss.es = reg->es;
        tsk->tss.cs = reg->orig_cs;
        tsk->tss.ss = reg->ss;
        tsk->tss.ds = reg->ds;
        tsk->tss.fs = reg->fs;
        tsk->tss.gs = USER_DATA_SEL;
        tsk->tss.ldt_sel = LDT_SEL(pid);
        tsk->tss.io_map = 0x80000000;

        tsk->pid = pid;
        tsk->tss_sel = TSS_SEL(pid);
        tsk->ldt_sel = LDT_SEL(pid);
        tsk->state = TASK_RUNABLE;
        tsk->counter = DEFAULT_COUNTER;
        tsk->priority = DEFAULT_PRIORITY;

        set_tss_desc(new_gdt, (unsigned int)&(tsk->tss), TSS_LIMIT, TSS_TYPE, TSS_IDX(pid));
        set_ldt_desc(new_gdt, (unsigned int)&(tsk->ldt), LDT_LIMIT, LDT_TYPE, LDT_IDX(pid));

        set_gdt_desc(tsk->ldt, CODE_BASE, USER_CODE_LIMIT, USER_CODE_TYPE, 1);
        set_gdt_desc(tsk->ldt, DATA_BASE, USER_DATA_LIMIT, USER_DATA_TYPE, 2);

	setup_task_pages(tsk);
	//copy_page_tables(tsk);

        list_add_tail(&(tsk->list), &task_list_head);

	return pid;
}


int sys_creat_task(unsigned int eip)
{
        struct task_struct *tsk;
        int pid;

        pid = get_pid();
        if (pid == -1) {
                DbgPrint("Get pid failed.\n");
                return -1;
        }
        printk("Get new pid: %d\n", pid);

        tsk = (struct task_struct *)alloc_page(1);
        if (!tsk) {
                DbgPrint("Alloc tsk page failed.\n");
                return -1;
        }
        DbgPrint("Alloc tsk page at: 0x%x\n", tsk);

	*tsk = *current;
        tsk->tss.prev_task_link = 0;
        tsk->tss.esp0 = tsk + PAGE_SIZE * 2;
        tsk->tss.ss0 = KERNEL_DATA_SEL;
        tsk->tss.esp1 = 0;
        tsk->tss.ss1 = 0;
        tsk->tss.esp2 = 0;
        tsk->tss.ss2 = 0;
        tsk->tss.eip = eip;
        tsk->tss.eflags = 0x200;
        tsk->tss.eax = 0;
        tsk->tss.ebx = 0;
        tsk->tss.ecx = 0;
        tsk->tss.edx = 0;
        tsk->tss.esp = (unsigned int)alloc_page(0) + PAGE_SIZE;
	DbgPrint("Alloc tsk ring3 stack at 0x%x\n", tsk->tss.esp - PAGE_SIZE);
        tsk->tss.ebp = 0;
        tsk->tss.esi = 0;
        tsk->tss.edi = 0;
        tsk->tss.es = USER_DATA_SEL;
        tsk->tss.cs = USER_CODE_SEL;
        tsk->tss.ss = USER_DATA_SEL;
        tsk->tss.ds = USER_DATA_SEL;
        tsk->tss.fs = USER_DATA_SEL;
        tsk->tss.gs = USER_DATA_SEL;
        tsk->tss.ldt_sel = LDT_SEL(pid);
        tsk->tss.io_map = 0x80000000;

        tsk->pid = pid;
        tsk->tss_sel = TSS_SEL(pid);
        tsk->ldt_sel = LDT_SEL(pid);
        tsk->state = TASK_RUNABLE;
        tsk->counter = DEFAULT_COUNTER;
        tsk->priority = DEFAULT_PRIORITY;

        set_tss_desc(new_gdt, (unsigned int)&(tsk->tss), TSS_LIMIT, TSS_TYPE, TSS_IDX(pid));
        set_ldt_desc(new_gdt, (unsigned int)&(tsk->ldt), LDT_LIMIT, LDT_TYPE, LDT_IDX(pid));

        set_gdt_desc(tsk->ldt, CODE_BASE, USER_CODE_LIMIT, USER_CODE_TYPE, 1);
        set_gdt_desc(tsk->ldt, DATA_BASE, USER_DATA_LIMIT, USER_DATA_TYPE, 2);

	setup_task_pages(tsk);
	//copy_page_tables(tsk);

        list_add_tail(&(tsk->list), &task_list_head);

	return 0;
}
