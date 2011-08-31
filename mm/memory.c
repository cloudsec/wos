/*
 * memory.c - core memory management.
 *
 * (c) 2011	wzt http://wwww.cloud-sec.org
 *
 */

#include <wos/gdt.h>
#include <asm/asm.h>
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
	printk("Init kernel pte ok.\n");
}

void setup_kernel_pte_new(void)
{
        unsigned int *kernel_pde = (unsigned int *)0x100000;
        unsigned int *kernel_pte = (unsigned int *)0x101000;
        unsigned int pte_addr = 0x101000, py_addr = 0;
        int i, j;

        /* mmap kernel to all the 64MB memory. */
        for (i = 0; i < KERNEL_PDE_NUM; i++) {
                *(kernel_pde + i) = ((pte_addr >> 12) << 12) | PAGE_USER_MODE;
                kernel_pte = (unsigned int *)pte_addr;
                DbgPrint("0x%x, 0x%x\n", *(kernel_pde + i), kernel_pte);
                for (j = 0; j < 1024; j++) {
                        *(kernel_pte + j) = ((py_addr >> 12) << 12) | PAGE_USER_MODE;
                        py_addr += PAGE_SIZE;
                        //DbgPrint("0x%x, 0x%x\n", *(kernel_pte + j), kernel_pte + j);
                }
                pte_addr += PAGE_SIZE;
        }

        asm("movl %%eax, %%cr3\n\t"
                "movl %%cr0, %%eax\n\t"
                "orl $0x80000000, %%eax\n\t"
                "movl %%eax, %%cr0\n"::"a"(pg_dir));
        printk("Init kernel pte ok.\n");
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

	/* user task mmap to all the 64MB physics memory. */
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
int copy_page_tables(struct task_struct *new_tsk) {
	unsigned int *new_pde, *new_pte;
	unsigned int *old_pde, *old_pte;
	unsigned int old_cr3;
	int i, j;

	old_cr3 = current->tss.cr3;

	new_pde = (unsigned int *)alloc_page(0);
	if (!new_pde) {
		printk("Alloc page failed.\n");
		return -1;
	}
	printk("Alloc task pid: %d cr3 page table addr at: 0x%x\n", new_tsk->pid, new_pde);

	for (i = 0; i < PAGE_PDE_NUM; i++) {
	//for (i = 0; i < 1; i++) {
		old_pde = (unsigned int *)old_cr3 + i;
		if (PDE_IS_PRESENT(*old_pde)) {
			//printk("old pde is present 0x%x, 0x%x\n", old_pde, *old_pde);
			*(new_pde + i) = *old_pde;
			//printk("new pde is present 0x%x, 0x%x\n", new_pde + i, *(new_pde + i));
			new_pte = (unsigned int *)alloc_page(0);
			if (!new_pte) {
				printk("Alloc page failed.\n");
				return -1;
			}
			//printk("Alloc new pte addr at 0x%x\n", new_pte);
			old_pte = *old_pde & 0xfffff000;
			//printk("old pte at: 0x%x\n", old_pte);
			for (j = 0; j < PAGE_PTE_NUM; j++) {
				if (PTE_IS_PRESENT(*(old_pte + j))) {
					//printk("old pte is present 0x%x, 0x%x\n", old_pte + j, *(old_pte + j));
					*(new_pte + j) = *(old_pte + j); 
					//printk("new pte is present 0x%x, 0x%x\n", new_pte + j, *(new_pte + j));
				}
			}
		}
	}

	new_tsk->tss.cr3 = (unsigned int)new_pde;
	printk("new task cr3 addr: 0x%x\n", new_tsk->tss.cr3);
}

void init_mm(void)
{
	int i;

	setup_kernel_pte_new();
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
