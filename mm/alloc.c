#include <wos/mm.h>
#include <wos/type.h>
#include <wos/list.h>

#define CHUNK_NUM		8

int chunk_size[CHUNK_NUM] = {32, 64, 128, 256, 512, 1024, 2048, 4096};
struct list_head chunk_list_head[CHUNK_NUM];

void *chunk_base;

/*
void *alloc_page(int order)
{
        int i, j;

        for (i = KERNEL_MEM_MAP + 2; i < PAGE_NUM - 1; i++) {
                for (j = i; j <= i + order; j++) {
                        if (mem_map[j] != MEM_UNUSED)
                                break;
                }
                if (j >= i + order) {
                        for (j = i; j <= i + order; j++)
                                mem_map[j] = MEM_USED;
                        return (void *)(i << PAGE_SHIFT);
                }
        }

        return NULL;
}
*/

int check_chunk_size(int size)
{
	int i;

	for (i = 0; i < CHUNK_NUM; i++) {
		if (size < chunk_size[i])
			return i;
	}

	return -1;
}

void *kmalloc(int size)
{
	struct mm_chunk *s = NULL;
	struct list_head *p = NULL;
	int idx;

	if (size < 0 || size > PAGE_SIZE)
		return NULL;

	idx = check_chunk_size(size);
	if (idx == -1)
		return NULL;
	//printk("%d\n", idx);

	if (list_empty((&chunk_list_head[idx])))
		return NULL;

	list_for_each(p, (&chunk_list_head[idx])) {
		s = list_entry(p, struct mm_chunk, list);
		if (s && !(s->inuse))
			return s->chunk_pos;
	}

	return NULL;
}

void kfree(void *addr)
{


}

void __show_chunk_list(struct list_head *list_head)
{
	struct mm_chunk *s = NULL;
	struct list_head *p = NULL;

	if (list_empty(list_head))
		return NULL;

	list_for_each(p, list_head) {
		s = list_entry(p, struct mm_chunk, list);
		if (s) {
			printk("chunk size: %d\tpos: 0x%x\n", s->size, s->chunk_pos);
		}
	}
}

void show_chunk_list(void)
{
	int i;

	for (i = 0; i < CHUNK_NUM; i++)
		__show_chunk_list(&chunk_list_head[i]);
}

void init_sub_chunk(void *base, int size, int page_num, int index)
{
	int total_size = PAGE_SIZE * page_num;
	int chunk_num = total_size / (MM_CHUNK_SIZE + size);
	int idx;

	printk("Create chunk size: %d\tchunk num: %d\n", size, chunk_num);
	for (idx = 0; idx < chunk_num; idx++) {
		struct mm_chunk *mm_chunk;

		mm_chunk = (struct mm_chunk *)(base + idx * (size + MM_CHUNK_SIZE)); 
		mm_chunk->size = size;
		mm_chunk->inuse = 0;
		mm_chunk->chunk_pos = mm_chunk;
		list_add_tail(&(mm_chunk->list), &chunk_list_head[index]);
	}
}

void init_chunk_list(void)
{
	int i;

	for (i = 0; i < CHUNK_NUM; i++)
		INIT_LIST_HEAD(&chunk_list_head[i]);
}

int init_chunk(void)
{
	int i, size;
	void *base;

	chunk_base = alloc_page(CHUNK_NUM * 2);
	if (!chunk_base) {
		printk("Allocte chunk failed.\n");
		return -1;
	}
	printk("Allocte chunk at 0x%x\t%d(bytes) ok.\n", chunk_base, 
		CHUNK_NUM * 2 * PAGE_SIZE);

	init_chunk_list();

	base = chunk_base;
	for (i = 0; i < CHUNK_NUM; i++) {
		init_sub_chunk(base, chunk_size[i], 2, i);
		base += PAGE_SIZE * 2;
	}

	//__show_chunk_list(&chunk_list_head[0]);
	//show_chunk_list();
}
