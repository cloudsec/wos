#include "gdt.h"
#include "asm.h"
#include "task.h"
#include "mm.h"
#include "type.h"

unsigned int mem_map[PAGE_NUM];

void setup_kernel_pte(void)
{
	unsigned int *kernel_pde = (unsigned int *)KERNEL_PDE_ADDR;
	unsigned int *kernel_pte = (unsigned int *)KERNEL_PTE_ADDR;
	unsigned int pte_addr = KERNEL_PTE_ADDR, py_addr = 0;
	int i, j;
	
	printk("0x%x, 0x%x\n", *kernel_pde, kernel_pde);
	/* mmap kernel to first 4MB in memory. */
	for (i = 0; i < KERNEL_PDE_NUM; i++) {
		*(kernel_pde + i) = pte_addr | PAGE_USER_MODE;
		printk("0x%x, 0x%x\n", *(kernel_pde + i), kernel_pde + i);
		for (j = 0; j < 1024; j++) {
			*(kernel_pte + j) = py_addr | PAGE_USER_MODE;
			py_addr += PAGE_SIZE;
			//printk("0x%x, 0x%x\n", *(kernel_pte + j), kernel_pte + j);
		}
		kernel_pte += PAGE_SIZE;
		pte_addr += PAGE_SIZE;
	}

	asm("movl %%eax, %%cr3\n\t"
		"movl %%cr0, %%eax\n\t"
		"orl $0x80000000, %%eax\n\t"
		"movl %%eax, %%cr0\n"::"a"(KERNEL_PDE_ADDR));
}

int setup_task_pages(struct task_struct *tsk)
{
	unsigned int *tsk_pde, *tsk_pte;
	unsigned int py_addr = 0, cr3;
	int i, j;

	tsk_pde = (unsigned int *)alloc_page();
	if (!tsk_pde) {
		printk("Alloc page failed.\n");
		return -1;
	}
	printk("Alloc task cr3 page table addr at: 0x%x\n", tsk_pde);

	/* user task mmap 16MB memory. */
	for (i = 0; i < 4; i++) {
		tsk_pte = (unsigned int *)alloc_page();
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

void *alloc_page(void)
{
	int i;

	for (i = KERNEL_MEM_MAP + 1; i < PAGE_NUM - 1; i++) {
		if (mem_map[i] == MEM_UNUSED) {
			mem_map[i] = MEM_USED;
			return (void *)(i << PAGE_SHIFT);
		}
	}

	return NULL;
}

void do_page_fault(unsigned int esp)
{
	unsigned int cr2;

	asm("movl %%cr2, %%eax\n":"=a"(cr2));
	printk("Page fault at: 0x%x\n", cr2);

        panic("page_fault", esp);
}

void init_mm(void)
{
	int i;

	/* kernel memory is reserved. */
	for (i = 0; i < KERNEL_MEM_MAP; i++)
		mem_map[i] = MEM_RESERVED;

	for (; i < PAGE_NUM; i++)
		mem_map[i] = MEM_UNUSED;
}
