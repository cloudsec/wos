/*
 * page_fault.c (c) 2011	wzt
 *
 */
#include <wos/mm.h>
#include <wos/task.h>
#include <asm/asm.h>

void copy_page(unsigned int old_page, unsigned int new_page)
{
	asm("cld\n\t"
		"rep\n\t"
		"movsl\n\t"
		::"S"(old_page), "D"(new_page), "c"(1024));
}

int do_page_cow(unsigned int vaddr, struct regs *reg)
{
	pde_t *pde;
	pte_t *pte;
	unsigned int *old_page;
	unsigned int *new_page;

	pde = current->tss.cr3 + (vaddr >> 22);
	printk("pde at: 0x%x, 0x%x\n", pde, *pde);

	pte = (pte_t *)(*pde & 0xfffff000) + ((vaddr << 10) >> 22);
	printk("pte at: 0x%x, 0x%x\n", pte, *pte);
	printk("%x\n", (vaddr << 10) >> 22);

	old_page = *pte & 0xfffff000;
	printk("old page at: 0x%x\n", old_page);

	new_page = (unsigned int *)alloc_page(PAGE_ZERO);
	if (!new_page)  {
		printk("Alloc page failed.\n");
		goto cow_err;
	}
	printk("Alloc new page at: 0x%x\n", new_page);

	*pte = (unsigned int)new_page | PAGE_USER_MODE;
	printk("new pte: 0x%x\n", *pte);

	flush_cr3(current->tss.cr3);
	//memcpy((void *)new_page, (void *)old_page, 1024);
	copy_page((unsigned int)old_page, (unsigned int)new_page);
	return 0;
cow_err:
	return -1;
}

/*
 * bit 0 == 0: no page found		1: protect fault
 * bit 1 == 0: read access		1: write access
 * bit 2 == 0: kernel mode access	1: user mode access
 *
 */
void do_page_fault(unsigned int esp)
{
	struct regs *reg = (struct regs *)esp;
        unsigned int error_code, cr2;
	unsigned int esp0, ss, cs;

	asm("movl %%esp, %%eax\n":"=a"(esp0));
	printk("esp0: 0x%x\n", esp0);
	asm("movl %%ss, %%eax\n":"=a"(ss));
	printk("ss: 0x%x\n", ss);
	asm("movl %%cs, %%eax\n":"=a"(cs));
	printk("cs: 0x%x\n", cs);

        asm("movl %%cr2, %%eax\n":"=a"(cr2));
        printk("pid: %d page fault at virtual addr: 0x%x\n", current->pid, cr2);

	error_code = reg->error_code;
	printk("error code: 0x%x, eip: 0x%x, esp: 0x%x\n" 
		"cs: 0x%x, ds: 0x%x, es: 0x%x, ss: 0x%x\n",
		error_code, reg->orig_eip, reg->esp,
		reg->orig_cs, reg->ds, reg->es, reg->ss);
/*
	if (error_code & PF_WRITE) {
		printk("pid: %d write access error.\n", current->pid);	
		if (!do_page_cow(cr2, reg)) {
			printk("do page copy on write ok.\n");
			return ;
		}
	}
pf_err:
	panic("Page fault", esp);
*/
}
