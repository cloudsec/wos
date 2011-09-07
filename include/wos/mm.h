#ifndef MM_H
#define MM_H

#include <wos/list.h>

#define DEFAULT_MEM_SIZE		(64*1024*1024)			/* 64MB */

#define PAGE_SIZE			(4*1024)			/* 4KB */
#define PAGE_NUM			DEFAULT_MEM_SIZE / PAGE_SIZE	
#define PAGE_SHIFT			12
#define PAGE_PDE_NUM			1024
#define PAGE_PTE_NUM			PAGE_PDE_NUM		
#define PAGE_ZERO			0

#define KERNEL_MEM_SIZE			(2*1024*1024)			/* 2MB */
#define KERNEL_MEM_MAP			KERNEL_MEM_SIZE >> PAGE_SHIFT
#define KERNEL_PDE_NUM			16

#define MEM_RESERVED			-1
#define MEM_UNUSED			0
#define MEM_USED			1

#define PAGE_PRESENT                    (1<<0)
#define PAGE_WRITE                      (1<<1)
#define PAGE_USER                       (1<<2)
#define PAGE_KERNEL                     (0<<2)
#define PAGE_READ			(0<<1)

#define PAGE_USER_MODE			(PAGE_PRESENT | PAGE_WRITE | PAGE_USER)
#define PAGE_KERNEL_MODE		(PAGE_PRESENT | PAGE_WRITE | PAGE_KERNEL)

#define PF_PROT				(1<<0)				/* page fault error code. */
#define PF_WRITE			(1<<1)
#define PF_KERNEL			(1<<2)

unsigned int mem_map[PAGE_NUM];

#define PDE_IS_NONE(addr)		(!(addr & 0x00000fff))
#define PTE_IS_NONE(addr)		PDE_IS_NONDE(addr)

#define PDE_IS_PRESENT(addr)		((addr & 0x00000001) ? 1 : 0)

#define PTE_IS_PRESENT(addr)		PDE_IS_PRESENT(addr)

#define get_pde_index(addr)		(addr >> 22)

typedef unsigned int			pde_t;
typedef unsigned int			pte_t;

#endif
