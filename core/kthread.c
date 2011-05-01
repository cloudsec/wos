#include <wos/task.h>
#include <wos/gdt.h>
#include <wos/mm.h>

extern unsigned int pg_dir;
extern struct gdt_desc new_gdt[8192];

int creat_kthread(unsigned int eip)
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
        tsk->tss.es = KERNEL_DATA_SEL;
        tsk->tss.cs = KERNEL_CODE_SEL;
        tsk->tss.ss = KERNEL_DATA_SEL;
        tsk->tss.ds = KERNEL_DATA_SEL;
        tsk->tss.fs = KERNEL_DATA_SEL;
        tsk->tss.gs = KERNEL_DATA_SEL;
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

        set_gdt_desc(tsk->ldt, CODE_BASE, CODE_LIMIT, KERNEL_CODE_TYPE, 1);
        set_gdt_desc(tsk->ldt, DATA_BASE, DATA_LIMIT, KERNEL_DATA_TYPE, 2);

        list_add_tail(&(tsk->list), &task_list_head);

	return 0;
}
