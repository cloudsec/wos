/*
 * memory.c - core memory management.
 *
 * (c) 2011	wzt
 *
 */

#include <wos/gdt.h>
#include <wos/idt.h>
#include <asm/asm.h>
#include <wos/task.h>
#include <wos/mm.h>
#include <wos/type.h>
#include <wos/debug.h>

extern void page_fault();

/* set kernel page dectory at 0x200000(2MB). */
pde_t pg_dir = 0x200000;

void setup_kernel_pte(void)
{
        pde_t *kernel_pde = (pde_t *)0x200000;
        pte_t *kernel_pte = (pte_t *)0x201000;
        pte_t pte_addr = 0x201000, py_addr = 0;
	unsigned int pde_idx, pte_idx;
	
	/* mmap kernel to all the 64MB memory. */
	for (pde_idx = 0; pde_idx < KERNEL_PDE_NUM; pde_idx++) {
		*(kernel_pde + pde_idx) = pte_addr | PAGE_USER_MODE;
		kernel_pte = (pte_t *)pte_addr;
		DbgPrint("0x%x, 0x%x\n", *(kernel_pde + pde_idx), kernel_pte);
		for (pte_idx = 0; pte_idx < PAGE_PTE_NUM; pte_idx++) {
			*(kernel_pte + pte_idx) = py_addr | PAGE_USER_MODE;
			py_addr += PAGE_SIZE;
			DbgPrint("0x%x, 0x%x\n", *(kernel_pte + pte_idx), 
				kernel_pte + pte_idx);
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
	pde_t *kernel_pde = (pde_t *)0x200000;
	pte_t *kernel_pte = (pte_t *)0x201000;
	pte_t pte_addr = 0x201000, py_addr = 0;
	int pde_idx, pte_idx;

	/* mmap kernel to all the 64MB memory. */
	for (pde_idx = 0; pde_idx < KERNEL_PDE_NUM; pde_idx++) {
                *(kernel_pde + pde_idx) = ((pte_addr >> 12) << 12) | PAGE_USER_MODE;
                kernel_pte = (pte_t *)pte_addr;
                DbgPrint("0x%x, 0x%x\n", *(kernel_pde + pde_idx), kernel_pte);
                for (pte_idx = 0; pte_idx < PAGE_PTE_NUM; pte_idx++) {
                        *(kernel_pte + pte_idx) = ((py_addr >> 12) << 12) | PAGE_USER_MODE;
                        py_addr += PAGE_SIZE;
                        DbgPrint("0x%x, 0x%x\n", *(kernel_pte + pte_idx), 
				kernel_pte + pte_idx);
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
	pde_t *tsk_pde;
	pte_t *tsk_pte;
	pte_t py_addr = 0;
	int pde_idx, pte_idx;

	tsk_pde = (unsigned int *)alloc_page(PAGE_ZERO);
	if (!tsk_pde) {
		DbgPrint("Alloc page failed.\n");
		return -1;
	}
	printk("Alloc task cr3 page table addr at: 0x%x\n", tsk_pde);

	/* user task mmap to all the 64MB physics memory. */
	for (pde_idx = 0; pde_idx < KERNEL_PDE_NUM; pde_idx++) {
		tsk_pte = (unsigned int *)alloc_page(PAGE_ZERO);
		if (!tsk_pte) {
			DbgPrint("Alloc pte failed.\n");
			return -1;
		}
		*(tsk_pde + pde_idx) = (pte_t)tsk_pte | PAGE_USER_MODE;
		DbgPrint("Alloc pte %d at 0x%x\t0x%x\n", pde_idx, tsk_pte, *(tsk_pde + pde_idx));
		for (pte_idx = 0; pte_idx < PAGE_PTE_NUM; pte_idx++) {
			*(tsk_pte + pte_idx) = py_addr | PAGE_USER_MODE;
			DbgPrint("0x%x\n", *(tsk_pte + pte_idx));
			py_addr += PAGE_SIZE;
		}
	}	
	
	tsk->tss.cr3 = (pde_t)tsk_pde;
	printk("task cr3 addr: 0x%x\n", tsk->tss.cr3);
	return 0;
}

int copy_page_tables(struct task_struct *new_tsk) {
	pde_t *old_pde, *new_pde;
	pte_t *old_pte, *new_pte;
	pde_t old_cr3;
	pte_t tmp_pte;
	unsigned int pde_idx, pte_idx;

	old_cr3 = (pde_t)(current->tss.cr3);
	printk("old cr3: 0x%x\n", old_cr3);

	new_pde = (unsigned int *)alloc_page(PAGE_ZERO);
	if (!new_pde) {
		printk("Alloc page failed.\n");
		return -1;
	}
	printk("Alloc task pid: %d cr3 page table addr at: 0x%x\n", 
		new_tsk->pid, new_pde);

	for (pde_idx = 0; pde_idx < PAGE_PDE_NUM; pde_idx++) {
		old_pde = (pde_t *)old_cr3 + pde_idx;
		if (PDE_IS_PRESENT(*old_pde)) {
			new_pte = (pte_t *)alloc_page(PAGE_ZERO);
			if (!new_pte) {
				DbgPrint("Alloc page failed.\n");
				return -1;
			}
			DbgPrint("Alloc new pte addr at 0x%x\n", new_pte);

			/* need to keep old pte attr? */
			*(new_pde + pde_idx) = (pte_t)new_pte | PAGE_USER_MODE;
			old_pte = *old_pde & 0xfffff000;
			DbgPrint("old pte at: 0x%x\n", old_pte);
			for (pte_idx = 0; pte_idx < PAGE_PTE_NUM; pte_idx++) {
				DbgPrint("pte_idx: 0%x, pte: 0x%x\n", pte_idx,
					*(old_pte + pte_idx));
				if (PTE_IS_PRESENT(*(old_pte + pte_idx))) {
					/* set new pte write pertect. */
					tmp_pte = *(old_pte + pte_idx);
					tmp_pte &= 0xfffffffd;
					*(new_pte + pte_idx) = tmp_pte; 
					/* kernel space is shared to all task, 
					 * so the pte is writable. The idle task
					 * is in the kernel memory, so make it
					 * writable. */
					if (tmp_pte > KERNEL_MEM_SIZE) {
						*(old_pte + pte_idx) = tmp_pte;
					}
				}
			}
		}
	}

	new_tsk->tss.cr3 = (pde_t)new_pde;
	printk("new task cr3 addr: 0x%x\n", new_tsk->tss.cr3);
	/* flush current task cr3, beacuse we have set shared pte write pretect. */
	flush_cr3(current->tss.cr3);
	
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
