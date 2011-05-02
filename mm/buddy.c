/*
 * buddy.c - Buddy alloctor.
 *
 * (c) wzt 2011		http://www.cloud-sec.org 
 *
 * +----+
 * |1024|
 * +----+
 * |512 |
 * +----+
 * |256 |
 * +----+
 * |128 | 
 * +----+
 * | 64 |
 * +----+   +-----------------------------------+   +-----------------------------------+
 * | 32 |-->|             PAGE_SIZE*32          |-->|            PAGE_SIZE*32           |
 * +----+   +-----------------------------------+   +-----------------------------------+
 * | 16 |
 * +----+
 * | 8  |
 * +----+   +---------------------------+   +----------------------------+
 * | 4  |-->|         PAGE_SIZE*4       |-->|         PAGE_SIZE*4        |
 * +----+   +---------------------------+   +----------------------------+
 * | 2  |
 * +----+   +-----------+   +-----------+
 * | 1  |-->| PAGE_SIZE |-->| PAGE_SIZE |
 * +----+   +-----------+   +-----------+
 *
 *
 * a*x + 2a*x + 4a*x + 8a*x = 512;
 * (1 + 2 + 4 + 8)ax = 512;
 * x = 512/(1 + 2 + 4 + 8)*a;
 *
 */

#include <wos/buddy.h>
#include <wos/type.h>

int buddy_size[BUDDY_CHUNK_NUM] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
int buddy_chunk_num = 0;

struct mm_buddy mm_buddy_array[BUDDY_CHUNK_NUM];

void *mm_buddy_base = NULL;

int check_buddy_size(int size)
{
	int i;

	for (i = 0; i < BUDDY_CHUNK_NUM; i++) {
		if (size < buddy_size[i])
			return i;
	}

	return -1;
}

void *alloc_buddy_chunk(int order)
{
	int i;
	void *addr;

	for (i = 0; i < mm_buddy_array[order].total_num; i++) {
		if (!mm_buddy_array[order].obj_map[i]) {
			mm_buddy_array[order].obj_map[i] = 1;
			addr = mm_buddy_array[order].obj[i];
			mm_buddy_array[order].free_num--;

			return addr;
		}
	}
}

void *__alloc_page(int old_order, int new_order)
{
	int i, next;
	void *base, *new_base, *addr;

	for (i = 0; i < mm_buddy_array[new_order].total_num; i++) {
		if (!mm_buddy_array[new_order].obj_map[i]) {
			mm_buddy_array[new_order].obj_map[i] = 1;
			next = i;
			break;
		}
	}

	if (next >= mm_buddy_array[new_order].total_num)
		return NULL;

	mm_buddy_array[new_order].free_num--;

	base = addr = mm_buddy_array[new_order].obj[next];
	new_base = base + (1 << new_order) * PAGE_SIZE;
	//printk("new_base: 0x%x\n", new_base);

	while (new_order > old_order) {
		new_order--;
		new_base -= (1 << new_order) * PAGE_SIZE;

		next = ++mm_buddy_array[new_order].total_num;
		mm_buddy_array[new_order].obj_map[next] = 0;
		mm_buddy_array[new_order].free_num++;
		mm_buddy_array[new_order].obj[next - 1] = new_base;
		//printk("new_base: 0x%x\n", new_base);
	}

	return addr;
}

void *alloc_page(int order)
{
	int idx;
	void *addr;

	if (mm_buddy_array[order].free_num) {
		addr = alloc_buddy_chunk(order);
		return addr;
	}

	for (idx = order + 1; idx < BUDDY_CHUNK_NUM; idx++) {
		if (mm_buddy_array[idx].free_num) {
			//printk("alloc page from order: %d\n", idx);
			addr = __alloc_page(order, idx);
			if (addr)
				return addr;
		}
	}

	return NULL;
}

void free_page(void *addr)
{
	int i, j;

	for (i = 0; i < BUDDY_CHUNK_NUM; i++) {
		printk("search order: %d\n", i);
		for (j = 0; j < mm_buddy_array[i].total_num; j++) {
			if (addr == mm_buddy_array[i].obj[j]) {
				printk("found obj: %d, %d, 0x%x, 0x%x\n",
					j, mm_buddy_array[i].total_num,
					addr, mm_buddy_array[i].obj[j]);
				mm_buddy_array[i].obj_map[j] = 0;
				mm_buddy_array[i].free_num++;
				return ;
			}		
		}
	}

}

void __show_buddy_list(int idx)
{
	int i;
	
	for (i = 0; i < mm_buddy_array[i].total_num; i++) {
		printk("buddy order: %d free_num: %d total_num: %d obj_addr: 0x%x\n",
			mm_buddy_array[idx].order, mm_buddy_array[idx].free_num,
			mm_buddy_array[idx].total_num, mm_buddy_array[idx].obj[i]);
	}
}

void show_buddy_list(void)
{
	int i;

	for (i = 0; i < BUDDY_CHUNK_NUM; i++)
		__show_buddy_list(i);
}

void init_buddy_list(void)
{
	void *base;
	int i, j;

	base = mm_buddy_base;
	for (i = 0; i < BUDDY_CHUNK_NUM; i++) {
		mm_buddy_array[i].size = PAGE_SIZE * (1 << i) * buddy_chunk_num;
		mm_buddy_array[i].chunk_num = buddy_size[i];
		mm_buddy_array[i].order = i;
		mm_buddy_array[i].free_num = buddy_chunk_num;
		mm_buddy_array[i].total_num = buddy_chunk_num;
		for (j = 0; j < buddy_chunk_num; j++)
			mm_buddy_array[i].obj[j] = base + j * (PAGE_SIZE * (1 << i));
		for (j = 0; j < MAX_BUDDY_CHUNK_NUM; j++)
			mm_buddy_array[i].obj_map[j] = 0;
		base += mm_buddy_array[i].size;
	}
}

int compute_buddy_num(void)
{
	int i;
	int num = 0, tmp;

	for (i = 0; i < BUDDY_CHUNK_NUM; i++)
		num += (1 << i);
	printk("%d\t0x%x\t0x%x\n", num, BUDDY_SIZE, num * PAGE_SIZE);
	
	for (i = 0;; i++) {
		if ((BUDDY_SIZE) <= (i * num * PAGE_SIZE))
			return i;
	}
	
	return -1;
}

int init_buddy_memory(void)
{
	buddy_chunk_num = compute_buddy_num();
	printk("buddy page num: 0x%x\ttotal page num: %d\n", buddy_chunk_num, PAGE_NUM);
	if (buddy_chunk_num > PAGE_NUM) {
		printk("buddy num: %d is bigger than %d\n", buddy_chunk_num, PAGE_NUM);
		return -1;
	}

	mm_buddy_base = (void *)BUDDY_MEM_BASE;

	return 0;
}

int init_buddy(void)
{
	if (init_buddy_memory() == -1) {
		printk("Init buddy system failed.\n");
		return -1;
	}
	printk("Alloc buddy memory at 0x%x\n", mm_buddy_base);

	init_buddy_list();
/*
	show_buddy_list();
	void *addr;
	addr = alloc_page(0);
	printk("allocte memory at 0x%x\n", addr);
	addr = alloc_page(0);
	printk("allocte memory at 0x%x\n", addr);
	addr = alloc_page(0);
	printk("allocte memory at 0x%x\n", addr);
	addr = alloc_page(0);
	printk("allocte memory at 0x%x\n", addr);
	free_page(addr);	
*/
}
