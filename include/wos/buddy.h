#ifndef BUDDY_H
#define BUDDY_H

#include <wos/mm.h>

#define BUDDY_CHUNK_NUM         	11
#define MAX_BUDDY_CHUNK_NUM             100

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

#define MM_CHUNK_SIZE                   sizeof(struct mm_chunk)
#define MM_BUDDY_SIZE                   sizeof(struct mm_buddy)
#define MM_BUDDY_CHUNK_SIZE             sizeof(struct mm_buddy_chunk)

#define BUDDY_SIZE                      (DEFAULT_MEM_SIZE - KERNEL_MEM_SIZE)
#define BUDDY_MEM_BASE                  KERNEL_MEM_SIZE

#define PAGE_ORDER_ZERO			0

void *alloc_page(int order);
void free_page(void *addr);

#endif
