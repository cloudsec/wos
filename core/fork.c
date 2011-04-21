#include <wos/gdt.h>
#include <wos/asm.h>
#include <wos/type.h>
#include <wos/list.h>
#include <wos/task.h>
#include <wos/mm.h>

extern struct task_struct init_task;
extern struct list_head task_list_head;
extern unsigned int pg_dir;

extern struct gdt_desc new_gdt[8192];
extern unsigned int gdt_desc_idx;
extern int task_flag;
extern void run_init_task1(void);

void run_task1(void);

int last_pid = 0;

struct task_struct task1;

int get_pid(void)
{
	int pid;

	if (last_pid > MAX_PID)
		return -1;

	pid = ++last_pid;

	return pid;
}

/*
int do_fork(unsigned int esp)
{
	struct regs *reg = (struct regs *)(esp + 4);
	struct task_struct *tsk;
	int pid;

	printk("cs: 0x%x, eip: 0x%x, ds: 0x%x\n", reg->orig_cs, reg->orig_eip, reg->ds);

	pid = get_pid();
	if (pid == -1) {
		printk("Get pid failed.\n");
		return -1;
	}
	printk("pid: %d\n", pid);

	tsk = (struct task_struct *)alloc_page(2);
	if (!tsk) {
		printk("Alloc tsk page failed.\n");
		return -1;
	}
	printk("Alloc tsk page at: 0x%x\n", tsk);

	tsk->tss.esp0 = (unsigned int)tsk + PAGE_SIZE * 2;
	tsk->tss.ss0 = KERNEL_DATA_SEL;
	//tsk->tss.eip = reg->orig_eip;
	tsk->tss.eip = run_init_task1;
        tsk->tss.esp1 = 0;
        tsk->tss.ss1 = 0;
        tsk->tss.esp2 = 0;
        tsk->tss.ss2 = 0;
	//tsk->tss.eflags = reg->eflags;
	tsk->tss.eflags = 0x200;
	tsk->tss.ldt_sel = TSS_SEL(pid);
	tsk->tss.prev_task_link = 0;
	tsk->tss.cr3 = pg_dir;
	tsk->tss.eax = 0;
	tsk->tss.ebx = reg->ebx;
	tsk->tss.ecx = reg->ecx;
	tsk->tss.edx = reg->edx;
	//tsk->tss.esp = reg->esp;
	tsk->tss.esp = (unsigned int)alloc_page(1) + PAGE_SIZE;
        tsk->tss.ebp = 0;
	tsk->tss.esi = reg->esi;
	tsk->tss.edi = reg->edi;
	tsk->tss.cs = reg->orig_cs;
	tsk->tss.ds = reg->ds;
	tsk->tss.es = reg->es;
	tsk->tss.fs = reg->fs;
        tsk->tss.gs = USER_DATA_SEL;
	tsk->tss.ss = reg->ss;
        tsk->tss.io_map = 0x80000000;

	printk("cs: 0x%x, ss: 0x%x\n", tsk->tss.cs, tsk->tss.ss);
	printk("eax: 0x%x, ebx: 0x%x\n", tsk->tss.eax, tsk->tss.ebx);
	printk("ldt_sel: 0x%x, cr3: 0x%x\n", tsk->tss.ldt_sel, tsk->tss.cr3);
	printk("esp0: 0x%x, esp: 0x%x\n", tsk->tss.esp0, tsk->tss.esp);

	tsk->tss_sel = TSS_SEL(pid);
	tsk->ldt_sel = LDT_SEL(pid);
	tsk->pid = pid;
	tsk->state = TASK_STOP;
	tsk->counter = DEFAULT_COUNTER;
	tsk->priority = DEFAULT_PRIORITY;

        set_tss_desc(new_gdt, (unsigned int)&(tsk->tss), TSS_LIMIT, TSS_TYPE, 
		TSS_IDX(pid));
        set_ldt_desc(new_gdt, (unsigned int)&(tsk->ldt), LDT_LIMIT, LDT_TYPE, 
		LDT_IDX(pid));

        set_gdt_desc(tsk->ldt, CODE_BASE, USER_CODE_LIMIT, USER_CODE_TYPE, 1);
        set_gdt_desc(tsk->ldt, DATA_BASE, USER_DATA_LIMIT, USER_DATA_TYPE, 2);

	list_add_tail(&(tsk->list), &task_list_head);
	tsk->state = TASK_RUNABLE;

	return 0;
}
*/

int task_clone(unsigned int eip)
{
	struct task_struct *tsk;
        int pid;

        pid = get_pid();
        if (pid == -1) {
                printk("Get pid failed.\n");
                return -1;
        }
        printk("pid: %d\n", pid);

        tsk = (struct task_struct *)alloc_page(1);
        if (!tsk) {
                printk("Alloc tsk page failed.\n");
                return -1;
        }
        printk("Alloc tsk page at: 0x%x\n", tsk);

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

int task_clone1(struct task_struct *tsk, unsigned int eip,
                unsigned int stack0, unsigned int stack3)
{
	int pid;

        pid = get_pid();
        if (pid == -1) {
                printk("Get pid failed.\n");
                return -1;
        }
        printk("pid: %d\n", pid);

        tsk->tss.prev_task_link = 0;
        tsk->tss.esp0 = stack0; 
        tsk->tss.ss0 = KERNEL_DATA_SEL;
        tsk->tss.esp1 = 0;
        tsk->tss.ss1 = 0;
        tsk->tss.esp2 = 0;
        tsk->tss.ss2 = 0;
        tsk->tss.cr3 = pg_dir;
        tsk->tss.eip = eip;
        tsk->tss.eflags = 0x200;
        tsk->tss.eax = 0;
        tsk->tss.ebx = 0;
        tsk->tss.ecx = 0;
        tsk->tss.edx = 0;
        tsk->tss.esp = stack3;
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
        tsk->counter = DEFAULT_COUNTER;
        tsk->priority = DEFAULT_PRIORITY;

        set_tss_desc(new_gdt, (unsigned int)&(tsk->tss), TSS_LIMIT, TSS_TYPE, TSS_IDX(pid));
        set_ldt_desc(new_gdt, (unsigned int)&(tsk->ldt), LDT_LIMIT, LDT_TYPE, LDT_IDX(pid));

        set_gdt_desc(tsk->ldt, CODE_BASE, USER_CODE_LIMIT, USER_CODE_TYPE, 1);
        set_gdt_desc(tsk->ldt, DATA_BASE, USER_DATA_LIMIT, USER_DATA_TYPE, 2);

        list_add_tail(&(tsk->list), &task_list_head);
        tsk->state = TASK_RUNABLE;
}

int task_clone2(unsigned int eip, unsigned int stack0, unsigned int stack3) 
{
	struct task_struct *tsk;
	int pid;

        pid = get_pid();
        if (pid == -1) {
                printk("Get pid failed.\n");
                return -1;
        }
        printk("pid: %d\n", pid);

        tsk = (struct task_struct *)alloc_page(1);
        if (!tsk) {
                printk("Alloc tsk page failed.\n");
                return -1;
        }
        printk("Alloc tsk page at: 0x%x\n", tsk);
	
        tsk->tss.prev_task_link = 0;
        tsk->tss.esp0 = stack0; 
        tsk->tss.ss0 = KERNEL_DATA_SEL;
        tsk->tss.esp1 = 0;
        tsk->tss.ss1 = 0;
        tsk->tss.esp2 = 0;
        tsk->tss.ss2 = 0;
        tsk->tss.cr3 = pg_dir;
        tsk->tss.eip = eip;
        tsk->tss.eflags = 0x200;
        tsk->tss.eax = 0;
        tsk->tss.ebx = 0;
        tsk->tss.ecx = 0;
        tsk->tss.edx = 0;
        tsk->tss.esp = stack3;
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
        tsk->priority = DEFAULT_COUNTER;

        set_tss_desc(new_gdt, (unsigned int)&(tsk->tss), TSS_LIMIT, TSS_TYPE, TSS_IDX(pid));
        set_ldt_desc(new_gdt, (unsigned int)&(tsk->ldt), LDT_LIMIT, LDT_TYPE, LDT_IDX(pid));

        set_gdt_desc(tsk->ldt, CODE_BASE, USER_CODE_LIMIT, USER_CODE_TYPE, 1);
        set_gdt_desc(tsk->ldt, DATA_BASE, USER_DATA_LIMIT, USER_DATA_TYPE, 2);

        list_add_tail(&(tsk->list), &task_list_head);
}

int task_clone3(unsigned int eip)
{
        struct task_struct *tsk;
        int pid;

        pid = get_pid();
        if (pid == -1) {
                printk("Get pid failed.\n");
                return -1;
        }
        printk("pid: %d\n", pid);

        tsk = (struct task_struct *)alloc_page(2);
        if (!tsk) {
                printk("Alloc tsk page failed.\n");
                return -1;
        }
        printk("Alloc tsk page at: 0x%x\n", tsk);

        tsk->tss.prev_task_link = 0;
        tsk->tss.esp0 = tsk + PAGE_SIZE * 2;
        tsk->tss.ss0 = KERNEL_DATA_SEL;
        tsk->tss.esp1 = 0;
        tsk->tss.ss1 = 0;
        tsk->tss.esp2 = 0;
        tsk->tss.ss2 = 0;
        tsk->tss.cr3 = pg_dir;
        tsk->tss.eip = eip;
        tsk->tss.eflags = 0x200;
        tsk->tss.eax = 0;
        tsk->tss.ebx = 0;
        tsk->tss.ecx = 0;
        tsk->tss.edx = 0;
        tsk->tss.esp = (unsigned int)alloc_page(1) + PAGE_SIZE;
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
        tsk->priority = DEFAULT_COUNTER;

        set_tss_desc(new_gdt, (unsigned int)&(tsk->tss), TSS_LIMIT, TSS_TYPE, TSS_IDX(pid));
        set_ldt_desc(new_gdt, (unsigned int)&(tsk->ldt), LDT_LIMIT, LDT_TYPE, LDT_IDX(pid));

        set_gdt_desc(tsk->ldt, CODE_BASE, USER_CODE_LIMIT, USER_CODE_TYPE, 1);
        set_gdt_desc(tsk->ldt, DATA_BASE, USER_DATA_LIMIT, USER_DATA_TYPE, 2);

        list_add_tail(&(tsk->list), &task_list_head);
}

int do_fork(unsigned int esp)
{
        struct task_struct *tsk;
	struct regs *reg = (struct regs *)(esp + 4);
        int pid;

        pid = get_pid();
        if (pid == -1) {
                printk("Get pid failed.\n");
                return -1;
        }
        printk("pid: %d\n", pid);

        tsk = (struct task_struct *)alloc_page(2);
        if (!tsk) {
                printk("Alloc tsk page failed.\n");
                return -1;
        }
        printk("Alloc tsk page at: 0x%x\n", tsk);

        tsk->tss.prev_task_link = 0;
        tsk->tss.esp0 = tsk + PAGE_SIZE * 2;
        tsk->tss.ss0 = KERNEL_DATA_SEL;
        tsk->tss.esp1 = 0;
        tsk->tss.ss1 = 0;
        tsk->tss.esp2 = 0;
        tsk->tss.ss2 = 0;
        tsk->tss.cr3 = pg_dir;
        //tsk->tss.eip = reg->orig_eip;
        tsk->tss.eip = run_init_task1;
        tsk->tss.eflags = 0x200;
        tsk->tss.eax = 0;
        tsk->tss.ebx = 0;
        tsk->tss.ecx = 0;
        tsk->tss.edx = 0;
        tsk->tss.esp = (unsigned int)alloc_page(1) + PAGE_SIZE;
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
        tsk->priority = DEFAULT_COUNTER;

        set_tss_desc(new_gdt, (unsigned int)&(tsk->tss), TSS_LIMIT, TSS_TYPE, TSS_IDX(pid));
        set_ldt_desc(new_gdt, (unsigned int)&(tsk->ldt), LDT_LIMIT, LDT_TYPE, LDT_IDX(pid));

        set_gdt_desc(tsk->ldt, CODE_BASE, USER_CODE_LIMIT, USER_CODE_TYPE, 1);
        set_gdt_desc(tsk->ldt, DATA_BASE, USER_DATA_LIMIT, USER_DATA_TYPE, 2);

        list_add_tail(&(tsk->list), &task_list_head);

	return 0;
}


int sys_creat_task(unsigned int eip)
{
        struct task_struct *tsk;
        int pid;

        pid = get_pid();
        if (pid == -1) {
                printk("Get pid failed.\n");
                return -1;
        }
        printk("pid: %d\n", pid);

        tsk = (struct task_struct *)alloc_page(2);
        if (!tsk) {
                printk("Alloc tsk page failed.\n");
                return -1;
        }
        printk("Alloc tsk page at: 0x%x\n", tsk);

        tsk->tss.prev_task_link = 0;
        tsk->tss.esp0 = tsk + PAGE_SIZE * 2;
        tsk->tss.ss0 = KERNEL_DATA_SEL;
        tsk->tss.esp1 = 0;
        tsk->tss.ss1 = 0;
        tsk->tss.esp2 = 0;
        tsk->tss.ss2 = 0;
        tsk->tss.cr3 = pg_dir;
        tsk->tss.eip = eip;
        tsk->tss.eflags = 0x200;
        tsk->tss.eax = 0;
        tsk->tss.ebx = 0;
        tsk->tss.ecx = 0;
        tsk->tss.edx = 0;
        tsk->tss.esp = (unsigned int)alloc_page(1) + PAGE_SIZE;
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
        tsk->priority = DEFAULT_COUNTER;

        set_tss_desc(new_gdt, (unsigned int)&(tsk->tss), TSS_LIMIT, TSS_TYPE, TSS_IDX(pid));
        set_ldt_desc(new_gdt, (unsigned int)&(tsk->ldt), LDT_LIMIT, LDT_TYPE, LDT_IDX(pid));

        set_gdt_desc(tsk->ldt, CODE_BASE, USER_CODE_LIMIT, USER_CODE_TYPE, 1);
        set_gdt_desc(tsk->ldt, DATA_BASE, USER_DATA_LIMIT, USER_DATA_TYPE, 2);

        list_add_tail(&(tsk->list), &task_list_head);

	return 0;
}
