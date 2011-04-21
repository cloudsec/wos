#include <wos/gdt.h>
#include <wos/asm.h>
#include <wos/task.h>
#include <wos/mm.h>
#include <wos/type.h>

unsigned int pg_dir = 0x100000;
unsigned int pg0 = 0x101000;
unsigned int pg1 = 0x102000;
unsigned int pg2 = 0x103000;
unsigned int pg3 = 0x104000;

void setup_kernel_pte(void)
{
        unsigned int *kernel_pde = (unsigned int *)0x100000;
        unsigned int *kernel_pte = (unsigned int *)0x101000;
        unsigned int pte_addr = 0x101000, py_addr = 0;
	int i, j;
	
	//printk("!0x%x, 0x%x, 0x%x\n", *kernel_pde, kernel_pde, kernel_pte);
	/* mmap kernel to first 64MB in memory. */
	for (i = 0; i < KERNEL_PDE_NUM; i++) {
		*(kernel_pde + i) = pte_addr | PAGE_USER_MODE;
		kernel_pte = (unsigned int*)pte_addr;
		printk("0x%x, 0x%x, 0x%x\n", *(kernel_pde + i), kernel_pde + i, kernel_pte);
		for (j = 0; j < 1024; j++) {
			*(kernel_pte + j) = py_addr | PAGE_USER_MODE;
			py_addr += PAGE_SIZE;
			//printk("0x%x, 0x%x\n", *(kernel_pte + j), kernel_pte + j);
		}
		pte_addr += PAGE_SIZE;
	}

	asm("movl %%eax, %%cr3\n\t"
		"movl %%cr0, %%eax\n\t"
		"orl $0x80000000, %%eax\n\t"
		"movl %%eax, %%cr0\n"::"a"(pg_dir));
}

int setup_task_pages(struct task_struct *tsk)
{
	unsigned int *tsk_pde, *tsk_pte;
	unsigned int py_addr = 0, cr3;
	int i, j;

	tsk_pde = (unsigned int *)alloc_page(1);
	if (!tsk_pde) {
		printk("Alloc page failed.\n");
		return -1;
	}
	printk("Alloc task cr3 page table addr at: 0x%x\n", tsk_pde);

	/* user task mmap 16MB memory. */
	for (i = 0; i < 4; i++) {
		tsk_pte = (unsigned int *)alloc_page(1);
		if (!tsk_pte) {
			printk("Alloc pte failed.\n");
			return -1;
		}
		printk("Alloc pte %d at 0x%x\n", i, tsk_pte);
		tsk_pde[i] = (unsigned int)tsk_pte | PAGE_USER_MODE;
		for (j = 0; j < 1024; j++) {
			tsk_pte[i] = py_addr | PAGE_USER_MODE;
			py_addr += PAGE_SIZE;
		}
	}	
	
	tsk->tss.cr3 = (unsigned int)tsk_pde;
	return 0;
}

void init_mm(void)
{
	int i;

	setup_kernel_pte();

	/* kernel memory is reserved. */
	for (i = 0; i < KERNEL_MEM_MAP; i++)
		mem_map[i] = MEM_RESERVED;

        for (; i < PAGE_NUM; i++)
                mem_map[i] = MEM_UNUSED;

	init_buddy();
	//init_chunk();
}
