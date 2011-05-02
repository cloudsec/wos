#ifndef MM_H
#define MM_H

#include <wos/list.h>

#define DEFAULT_MEM_SIZE		(64*1024*1024)			/* 64MB */
#define PAGE_SIZE			(4*1024)			/* 4KB */
#define PAGE_NUM			DEFAULT_MEM_SIZE / PAGE_SIZE	
#define PAGE_SHIFT			12

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

#define PAGE_USER_MODE			(PAGE_PRESENT | PAGE_WRITE | PAGE_USER)
#define PAGE_KERNEL_MODE		(PAGE_PRESENT | PAGE_WRITE | PAGE_KERNEL)

unsigned int mem_map[PAGE_NUM];

#endif
