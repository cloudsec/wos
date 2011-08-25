/*
 * memory.c - core memory management.
 *
 * (c) 2011	wzt http://wwww.cloud-sec.org
 *
 */

#include <wos/gdt.h>
#include <wos/asm.h>
#include <wos/task.h>
#include <wos/mm.h>
#include <wos/type.h>
#include <wos/debug.h>

extern void page_fault();

/* set kernel page dectory at 0x100000(1MB). */
unsigned int pg_dir = 0x100000;

void setup_kernel_pte(void)
{
        unsigned int *kernel_pde = (unsigned int *)0x100000;
        unsigned int *kernel_pte = (unsigned int *)0x101000;
        unsigned int pte_addr = 0x101000, py_addr = 0;
	int i, j;
	
	/* mmap kernel to all the 64MB memory. */
	for (i = 0; i < KERNEL_PDE_NUM; i++) {
		*(kernel_pde + i) = pte_addr | PAGE_USER_MODE;
		kernel_pte = (unsigned int*)pte_addr;
		DbgPrint("0x%x, 0x%x\n", *(kernel_pde + i), kernel_pte);
		for (j = 0; j < 1024; j++) {
			*(kernel_pte + j) = py_addr | PAGE_USER_MODE;
			py_addr += PAGE_SIZE;
			//DbgPrint("0x%x, 0x%x\n", *(kernel_pte + j), kernel_pte + j);
		}
		pte_addr += PAGE_SIZE;
	}

	asm("movl %%eax, %%cr3\n\t"
		"movl %%cr0, %%eax\n\t"
		"orl $0x80000000, %%eax\n\t"
		"movl %%eax, %%cr0\n"::"a"(pg_dir));
}

int setup_kernel_pte1(void)
{
        unsigned int *kernel_pde, *kernel_pte;
        unsigned int py_addr = 0;
        int i, j;

        kernel_pde = (unsigned int *)alloc_page(0);
        if (!kernel_pde) {
                DbgPrint("Alloc page failed.\n");
                return -1;
        }
        DbgPrint("Alloc task cr3 page table addr at: 0x%x\n", kernel_pde);

	//kernel_pde = (unsigned int *)0x100000;
	//kernel_pte = (unsigned int *)0x101000;

        /* user task mmap to first 64MB memory. */
        for (i = 0; i < KERNEL_PDE_NUM; i++) {
                kernel_pte = (unsigned int *)alloc_page(0);
                if (!kernel_pte) {
                        DbgPrint("Alloc pte failed.\n");
                        return -1;
                }

                *(kernel_pde + i) = (unsigned int)kernel_pte | PAGE_USER_MODE;
                DbgPrint("Alloc pte %d at 0x%x\t0x%x\n", i, *(kernel_pde + i), kernel_pte);
                for (j = 0; j < 1024; j++) {
                        *(kernel_pte + j) = py_addr | PAGE_USER_MODE;
                        py_addr += PAGE_SIZE;
                }
		//kernel_pte += 1024;
        }

        asm("movl %%eax, %%cr3\n\t"
                "movl %%cr0, %%eax\n\t"
                "orl $0x80000000, %%eax\n\t"
                "movl %%eax, %%cr0\n"::"a"((unsigned int)kernel_pde));
}

int setup_task_pages(struct task_struct *tsk)
{
	unsigned int *tsk_pde, *tsk_pte;
	unsigned int py_addr = 0;
	int i, j;

	tsk_pde = (unsigned int *)alloc_page(0);
	if (!tsk_pde) {
		DbgPrint("Alloc page failed.\n");
		return -1;
	}
	printk("Alloc task cr3 page table addr at: 0x%x\n", tsk_pde);

	/* user task mmap to all the 64MB memory. */
	for (i = 0; i < 16; i++) {
		tsk_pte = (unsigned int *)alloc_page(0);
		if (!tsk_pte) {
			DbgPrint("Alloc pte failed.\n");
			return -1;
		}
		*(tsk_pde + i) = (unsigned int)tsk_pte | PAGE_USER_MODE;
		DbgPrint("Alloc pte %d at 0x%x\t0x%x\n", i, tsk_pte, *(tsk_pde + i));
		for (j = 0; j < 1024; j++) {
			*(tsk_pte + j) = py_addr | PAGE_USER_MODE;
			DbgPrint("0x%x\n", *(tsk_pte + i));
			py_addr += PAGE_SIZE;
		}
	}	
	
	tsk->tss.cr3 = (unsigned int)tsk_pde;
	printk("task cr3 addr: 0x%x\n", tsk->tss.cr3);
	return 0;
}

void init_mm(void)
{
	int i;

	setup_kernel_pte();
	set_trap_gate((unsigned int)page_fault, 14);

	/* kernel memory is reserved. */
	for (i = 0; i < KERNEL_MEM_MAP; i++)
		mem_map[i] = MEM_RESERVED;

	i++;
        for (; i < PAGE_NUM; i++)
                mem_map[i] = MEM_UNUSED;

	init_buddy();
	init_general_slab_cache();
	init_kmem_cache();
	//DbgPrint("init ok.\n");

	//mm_test();
}
