/*
 * exit.c	(c) wzt 2011
 *
 */
#include <wos/gdt.h>
#include <asm/asm.h>
#include <wos/type.h>
#include <wos/list.h>
#include <wos/task.h>
#include <wos/mm.h>
#include <wos/debug.h>

int do_exit(void)
{
        struct task_struct *tsk = NULL;
        struct list_head *p = NULL;
	int pid;

	pid = current->pid;
	printk("Pid %d exiting.\n", pid);

	if (pid == 0)
		oops("Init process try to exit.\n");

	cli();
        list_for_each(p, (&task_list_head)) {
                tsk = list_entry(p, struct task_struct, list);
                if (tsk) {
			list_del(&p);
			sti();
			printk("Del pid %d from task list ok.\n", pid);
			return 0;
                }
        }
	sti();
	printk("Del pid %d from task list failed.\n", pid);

	return -1;
}
