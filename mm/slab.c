#include <wos/mm.h>
#include <wos/slab.h>
#include <wos/task.h>
#include <wos/type.h>

static int slab_size[SLAB_SIZE_NUM] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};

struct slab_cache slab_cache_array[SLAB_SIZE_NUM];

void __show_slab_list(struct list_head *list_head)
{
	struct slab *slab;
	struct list_head *p;

	if (list_empty(list_head))
		return ;

	list_for_each(p, list_head) {
		slab = list_entry(p, struct slab, list);
		if (slab) {
			printk("obj num: %d\tfree_num: %d\tbase: 0x%x\n",
				slab->obj_num, slab->free_num, slab->base);
		}
	}
}

void show_slab_list(void)
{
	int idx;

	for (idx = 0; idx < SLAB_SIZE_NUM; idx++) {
		printk("slab size: %d\tslab num: %d\tfree num: %d\tslab_page: 0x%x\n",
			slab_cache_array[idx].slab_size, 
			slab_cache_array[idx].slab_num,
			slab_cache_array[idx].free_num,
			slab_cache_array[idx].slab_page);
		__show_slab_list(&slab_cache_array[idx].list);
	}
}

unsigned int *slab_bufctl(struct slab *slab)
{
	return (unsigned int *)(slab + 1);
}

void *get_slab_obj(struct slab *slab, int idx)
{
	void *obj;

	obj = slab->base + slab_cache_array[idx].slab_size * slab->free_idx;

	slab->free_idx = slab_bufctl(slab)[slab->free_idx];

	slab->free_num--;
	slab_cache_array[idx].free_num--;

	return obj;
}

int check_slab_size(int size)
{
        int i;

        for (i = 0; i < SLAB_SIZE_NUM; i++) {
                if (size < slab_size[i])
                        return i;
        }

        return -1;
}

void *kmalloc(int size)
{
	struct slab *s = NULL;
	struct list_head *p = NULL;
	void *obj;
	int idx;

	if (size < 0 || size > 1024)
		return NULL;

	idx = check_slab_size(size);

	if (!slab_cache_array[idx].free_num) {
		printk("alloc slab obj in %d failed.\n", idx);
		return NULL;
	}

	list_for_each(p, (&slab_cache_array[idx].list)) {
		s = list_entry(p, struct slab, list);
		if (s && s->free_num) {
			obj = get_slab_obj(s, idx);
			return obj;
		}
	}

	return NULL;
}

int addr_to_cache_idx(void *addr)
{
	unsigned int align_addr = (unsigned int)addr & 0xfffff000;
	unsigned int base;
	unsigned int idx;

	printk("align addr: 0x%x\n", align_addr);
	base = (unsigned int)(slab_cache_array[0].slab_page);
	printk("base addr: 0x%x\n", base);

	base = align_addr - base;
	printk("base addr: 0x%x\n", base);

	idx = base / (SLAB_NUM * PAGE_SIZE);
	printk("cache idx: %d\n", idx);

	return idx;
}

struct slab *search_slab(void *addr, struct list_head *list_head)
{
	struct slab *slab;
	struct list_head *p;

	if (list_empty(list_head))
		return NULL;

	list_for_each(p, list_head) {
		slab = list_entry(p, struct slab, list);
		if (slab && slab->base <= addr)
			return slab;
	}

	return NULL;
}

void *put_slab_obj(struct slab *slab, void *obj, int idx)
{
	int obj_idx;

	obj_idx = (obj - slab->base) / slab_cache_array[idx].slab_size;
	printk("obj_idx: %d\n", obj_idx);
	
	slab_bufctl(slab)[obj_idx] = slab->free_idx;
	slab->free_idx = obj_idx;

	slab->free_num++;
	slab_cache_array[idx].free_num++;
}

void *kfree(void *addr)
{
	struct slab *slab;
	int cache_idx;

	if (!addr)
		return ;

	cache_idx = addr_to_cache_idx(addr);
	if (cache_idx < 0 || cache_idx >= SLAB_SIZE_NUM)
		return ;

	slab = search_slab(addr, &slab_cache_array[cache_idx].list);
	if (!slab)
		return ;

	put_slab_obj(slab, addr, cache_idx);
}

int compute_slab_obj_num(int obj_size, int slab_size)
{
	return (slab_size - sizeof(struct slab)) / (obj_size + sizeof(int));
}

void __init_slab(int slab_idx, void *addr, int size, 
		struct list_head *list_head)
{
	struct slab *new_slab = (struct slab *)addr;
	void *base;
	int idx;

	new_slab->obj_num = compute_slab_obj_num(size, PAGE_SIZE);
	new_slab->free_num = new_slab->obj_num;
	new_slab->base = addr + sizeof(struct slab) + 
		(new_slab->obj_num * sizeof(int));

	for (idx = 0; idx < new_slab->obj_num - 1; idx++)
		slab_bufctl(new_slab)[idx] = idx + 1;
	slab_bufctl(new_slab)[idx] = -1;

	new_slab->free_idx = 0;
	list_add_tail(&(new_slab->list), list_head);

	if (!slab_cache_array[slab_idx].slab_page)
		slab_cache_array[slab_idx].slab_page = addr;
	slab_cache_array[slab_idx].free_num = 
			slab_cache_array[slab_idx].slab_num * new_slab->free_num;
}

int init_slab(int idx, int size, struct list_head *list_head)
{
	int i;

	for (i = 0; i < SLAB_NUM; i++) {
		void *addr;

		addr = alloc_page(0);
		if (!addr) {
			printk("alloc page failed.\n");
			return -1;
		}
		//printk("slab alloc page at 0x%x\n", addr);

		__init_slab(idx, addr, size, list_head);
	}	

	return 0;
}

void init_slab_cache(void)
{
	int i;

	printk("Init slab cache.\n");

	for (i = 0; i < SLAB_SIZE_NUM; i++) {
		slab_cache_array[i].slab_size = slab_size[i];
		slab_cache_array[i].slab_num = SLAB_NUM;
		INIT_LIST_HEAD(&slab_cache_array[i].list);
		init_slab(i, slab_size[i], &slab_cache_array[i].list);
	}

	//__show_slab_list(&slab_cache_array[0].list);
	//show_slab_list();
	void *addr;
	addr = kmalloc(4);
	printk("alloc addr at 0x%x\n", addr);
	addr = kmalloc(4);
	printk("alloc addr at 0x%x\n", addr);
	addr = kmalloc(64);
	printk("alloc addr at 0x%x\n", addr);
	addr = kmalloc(64);
	printk("alloc addr at 0x%x\n", addr);
	addr = kmalloc(64);
	printk("alloc addr at 0x%x\n", addr);
	kfree(addr);
	addr = kmalloc(64);
	printk("alloc addr at 0x%x\n", addr);
	addr = kmalloc(64);
	printk("alloc addr at 0x%x\n", addr);
	addr = kmalloc(64);
	printk("alloc addr at 0x%x\n", addr);
	kfree(addr);
	addr = kmalloc(64);
	printk("alloc addr at 0x%x\n", addr);
}
