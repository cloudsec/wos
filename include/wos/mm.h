#ifndef MM_H
#define MM_H

#include <wos/list.h>

#define DEFAULT_MEM_SIZE		(64*1024*1024)			/* 64MB */
#define PAGE_SIZE			(4*1024)			/* 4KB */
#define PAGE_NUM			DEFAULT_MEM_SIZE / PAGE_SIZE	
#define PAGE_SHIFT			12

#define KERNEL_MEM_SIZE			(2*1024*1024)			/* 1MB */
#define KERNEL_MEM_MAP			KERNEL_MEM_SIZE >> PAGE_SHIFT
#define KERNEL_PDE_NUM			16	

#define MEM_RESERVED			-1
#define MEM_UNUSED			0
#define MEM_USED			1

#define PAGE_PRESENT                    1
#define PAGE_WRITE                      (1<<1)
#define PAGE_USER                       (1<<2)
#define PAGE_KERNEL                     (0<<2)

#define PAGE_USER_MODE			(PAGE_PRESENT | PAGE_WRITE | PAGE_USER)
#define PAGE_KERNEL_MODE		(PAGE_PRESENT | PAGE_WRITE | PAGE_KERNEL)

#define MAX_BUDDY_CHUNK_NUM		10

struct mm_chunk {
	void *chunk_pos;
	int size;
	int inuse;
	struct list_head list;
};

struct mm_buddy {
	int size;
	int chunk_num;
	int order;
	int free_num;
	int total_num;
	int obj_map[MAX_BUDDY_CHUNK_NUM];
	void *obj[MAX_BUDDY_CHUNK_NUM];
};

struct mm_buddy_chunk {
	void *chunk_pos;
	int inuse;
	struct list_head list;
};

#define MM_CHUNK_SIZE			sizeof(struct mm_chunk)
#define MM_BUDDY_SIZE			sizeof(struct mm_buddy)
#define MM_BUDDY_CHUNK_SIZE		sizeof(struct mm_buddy_chunk)

#define BUDDY_CHUNK_NUM         	11
#define BUDDY_SIZE			(DEFAULT_MEM_SIZE - KERNEL_MEM_SIZE)
#define BUDDY_SUB_CHUNK_NUM		2
#define BUDDY_MEM_BASE			KERNEL_MEM_SIZE
			
unsigned int mem_map[PAGE_NUM];

void *alloc_page(int order);

#endif
