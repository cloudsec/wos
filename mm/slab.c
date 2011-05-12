/*
 * slab.c - Slab alloctor
 *
 * (c) 2011	wzt http://www.cloud-sec.org
 *
 */

#include <wos/mm.h>
#include <wos/slab.h>
#include <wos/task.h>
#include <wos/type.h>

static int slab_size[SLAB_SIZE_NUM] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};

struct slab_cache slab_cache_array[SLAB_SIZE_NUM];

void __init_slab(int slab_idx, void *addr, int size,
                struct list_head *list_head);

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

	//for (idx = 0; idx < SLAB_SIZE_NUM; idx++) {
	for (idx = 0; idx < 1; idx++) {
		printk("slab size: %d slab num: %d free num: %d color num: %d slab_page: 0x%x\n",
			slab_cache_array[idx].slab_size, 
			slab_cache_array[idx].slab_num,
			slab_cache_array[idx].free_num,
			slab_cache_array[idx].color_num,
			slab_cache_array[idx].slab_page);
		__show_slab_list(&slab_cache_array[idx].list);
	}
}

/* bufctl just behind the slab struct. */
unsigned int *slab_bufctl(struct slab *slab)
{
	return (unsigned int *)(slab + 1);
}

/* get an obj from a slab. */
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

/* expand a new slab with PAGE_SIZE. */
void *expand_slab(int idx)
{
	void *new_slab = NULL;

	new_slab = alloc_page(0);
	if (!new_slab) {
		printk("alloc_page failed.\n");
		return NULL;
	}
	
	__init_slab(idx, new_slab, slab_cache_array[idx].slab_size,
		&(slab_cache_array[idx].list));
	
	slab_cache_array[idx].slab_num++;

	return new_slab;
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
		printk("expand slab obj in %d.\n", idx);
		if (!(s = expand_slab(idx))) {
			printk("expand slab failed.\n");
			return NULL;
		}
		obj = get_slab_obj(s, idx);
		return obj;
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

/* support for the kfree. */
int addr_to_cache_idx(void *addr)
{
	unsigned int align_addr = (unsigned int)addr & 0xfffff000;
	int idx;

	for (idx = 0; idx < SLAB_SIZE_NUM; idx++) {
		if ((unsigned int)slab_cache_array[idx].slab_page >= align_addr)
			return idx;
	}

	return -1;
}

/*
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
*/

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

/*
 * compute slab color num for hardware cache.
 */
int compute_slab_color_num(int obj_size, int slab_size)
{
	return (slab_size - sizeof(struct slab)) % (obj_size + sizeof(int));
}

int get_slab_color(int slab_idx)
{
	if (slab_cache_array[slab_idx].color_next ==
		slab_cache_array[slab_idx].color_num)
		return 0;
	else
		return slab_cache_array[slab_idx].color_next++;
}

int set_slab_base_addr(void *addr, struct slab *new_slab)
{
	return ALIGN((unsigned int)(addr + sizeof(struct slab) +
                (new_slab->obj_num * sizeof(int))), DEFAULT_ALIGN);
}

/* 
 * support for CPU hardware cache.
 */
int fix_slab_base_addr(void *addr, int slab_idx)
{
	return (unsigned int)addr + slab_cache_array[slab_idx].color_next;
}

void set_slab_cache_array(int slab_idx, void *addr, int size, 
		struct slab *new_slab)
{
	slab_cache_array[slab_idx].slab_page = addr;
	slab_cache_array[slab_idx].free_num += new_slab->free_num;
	slab_cache_array[slab_idx].color_num = 
			compute_slab_color_num(size, PAGE_SIZE);
	slab_cache_array[slab_idx].color_next = 0;
}

/* 
 * all the slab managment builtin the front of the slab, next is bufctl
 * array which is a sample link list of obj. the end of the slab maybe
 * not used, it can be used for slab color for hardware cache.
 *
 * the slab struct like this:
 *
 * +-----------------------------------------------+
 * | struct slab | bufctl | obj | obj | ...| color |
 * +-----------------------------------------------+
 * 
 */
void __init_slab(int slab_idx, void *addr, int size, 
		struct list_head *list_head)
{
	struct slab *new_slab = (struct slab *)addr;
	void *base;
	int idx;

	new_slab->obj_num = compute_slab_obj_num(size, PAGE_SIZE);
	new_slab->free_num = new_slab->obj_num;

	for (idx = 0; idx < new_slab->obj_num - 1; idx++)
		slab_bufctl(new_slab)[idx] = idx + 1;
	slab_bufctl(new_slab)[idx] = -1;

	new_slab->free_idx = 0;
	list_add_tail(&(new_slab->list), list_head);

	set_slab_cache_array(slab_idx, addr, size, new_slab);

	new_slab->base = set_slab_base_addr(addr, new_slab);	
	new_slab->base = fix_slab_base_addr(new_slab->base, slab_idx);	
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
		slab_cache_array[i].free_num = 0;
		INIT_LIST_HEAD(&slab_cache_array[i].list);
		init_slab(i, slab_size[i], &slab_cache_array[i].list);
	}

	//__show_slab_list(&slab_cache_array[4].list);
	show_slab_list();

	void *addr;
	for (i = 0; i < 35; i++) {
		addr = kmalloc(128);
		printk("alloc addr at 0x%x\n", addr);
	}
	kfree(addr);

	addr = kmalloc(128);
	printk("alloc addr at 0x%x\n", addr);
}
