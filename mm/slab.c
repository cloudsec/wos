/*
 * slab.c - Slab alloctor
 *
 * (c) 2011 wzt
 *
 * 
 *  -------     ------     ------
 *  |cache|-->  |slab| --> |slab|
 *  -------     ------     ------
 *  |cache|
 *  -----
 *  |cache| ...
 *  -----
 *  |cache| ...
 *  -----
 *  |cache| ...
 *  -------    ------     ------
 *  |cache|--> |slab| --> |slab|
 *  -------    ------     ------
 *  |cache|-->|slab|-->|slab|
 *  -------   ------   ------
 *
 *
 * current support:
 *
 * - basic implement for slab alloctor.
 * - hardware cache support.
 * - slab expand support.
 * - genernal slab and slab cache support.
 *
 * todo:
 *
 * - smp cpu support.
 * - slab obj cache support.
 * 
 */

#include <wos/mm.h>
#include <wos/slab.h>
#include <wos/buddy.h>
#include <wos/task.h>
#include <wos/string.h>
#include <wos/type.h>

static int slab_size[SLAB_SIZE_NUM] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
static struct slab_cache slab_cache_array[SLAB_SIZE_NUM];

struct slab_cache kmem_cache_st;
struct list_head kmem_list_head;

void __init_slab(struct slab_cache *slab_cache, void *addr, int size);

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
		printk("slab size: %d slab num: %d free num: %d color num: %d"
			"slab_page: 0x%x\n",
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
void *get_slab_obj(struct slab *slab, struct slab_cache *slab_cache)
{
	void *obj;

	obj = slab->base + slab_cache->slab_size * slab->free_idx;

	slab->free_idx = slab_bufctl(slab)[slab->free_idx];

	slab->free_num--;
	slab_cache->free_num--;

	return obj;
}

void set_slab_obj_cache(struct slab *slab, struct slab_cache *slab_cache)
{
	void *obj;
	int idx;

	slab_cache->obj_cache.curr_obj = 0;
	slab_cache->obj_cache.limit += slab->obj_num;

	for (idx = 0; idx < slab->obj_num - 1; idx++)
		slab_cache->obj_cache.entry[idx] = 
			get_slab_obj(slab, slab_cache);
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

/* 
 * expand a new slab with PAGE_SIZE. 
 */
void *expand_slab(struct slab_cache *slab_cache)
{
	void *new_slab = NULL;

	new_slab = alloc_page(PAGE_ORDER_ZERO);
	if (!new_slab) {
		printk("alloc_page failed.\n");
		return NULL;
	}
	
	__init_slab(slab_cache, new_slab, slab_cache->slab_size);
	
	slab_cache->slab_num++;

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
		if (!(s = expand_slab(&slab_cache_array[idx]))) {
			printk("expand slab failed.\n");
			return NULL;
		}
		obj = get_slab_obj(s, &(slab_cache_array[idx]));
		return obj;
	}

	list_for_each(p, (&slab_cache_array[idx].list)) {
		s = list_entry(p, struct slab, list);
		if (s && s->free_num) {
			obj = get_slab_obj(s, &(slab_cache_array[idx]));
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
 * support for kfree & kmem_cache_free.
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

void *put_slab_obj(struct slab *slab, void *obj, 
		struct slab_cache *slab_cache)
{
	int obj_idx;

	printk("obj: %x, slab->base: %x slab size: %d\n", 
		obj, slab->base, slab_cache->slab_size);

	obj_idx = (obj - slab->base) / slab_cache->slab_size;
	printk("obj_idx: %d\n", obj_idx);
	
	slab_bufctl(slab)[obj_idx] = slab->free_idx;
	slab->free_idx = obj_idx;

	slab->free_num++;
	slab_cache->free_num++;
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

	put_slab_obj(slab, addr, &slab_cache_array[cache_idx]);
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

int get_slab_color(struct slab_cache *slab_cache)
{
	if (slab_cache->color_next >= slab_cache->color_num)
		return 0;
	else
		return slab_cache->color_next++;
}

int set_slab_base_addr(void *addr, struct slab *new_slab)
{
	return ALIGN((unsigned int)(addr + sizeof(struct slab) +
                (new_slab->obj_num * sizeof(int))), DEFAULT_ALIGN);
}

/* 
 * support for CPU hardware cache.
 */
int fix_slab_base_addr(void *addr, int color)
{
	return (unsigned int)addr + color;
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
void __init_slab(struct slab_cache *slab_cache, void *addr, int size)
{
	struct slab *new_slab = (struct slab *)addr;
	void *base;
	int idx;

	new_slab->obj_num = compute_slab_obj_num(size, PAGE_SIZE);
	new_slab->free_num = new_slab->obj_num;

	for (idx = 0; idx < new_slab->obj_num - 1; idx++)
		slab_bufctl(new_slab)[idx] = idx + 1;
	slab_bufctl(new_slab)[idx] = -1;

        if (slab_cache->ctor)
                slab_cache->ctor();

        slab_cache->slab_page = addr;
        slab_cache->free_num += new_slab->free_num;
        slab_cache->color_next = get_slab_color(slab_cache);
	
	//set_slab_obj_cache(new_slab, slab_cache);

	new_slab->free_idx = 0;
	list_add_tail(&(new_slab->list), &(slab_cache->list));

	new_slab->base = set_slab_base_addr(addr, new_slab);	
	//printk("slab base: 0x%x\n", new_slab->base);
	new_slab->base = fix_slab_base_addr(new_slab->base, 
		slab_cache->color_next);
	//printk("slab base: 0x%x\n", new_slab->base);
}

int init_slab(struct slab_cache *slab_cache, int size)
{
	int i;

	for (i = 0; i < SLAB_NUM; i++) {
		void *addr;

		addr = alloc_page(PAGE_ORDER_ZERO);
		if (!addr) {
			printk("alloc page failed.\n");
			return -1;
		}
		//printk("slab alloc page at 0x%x\n", addr);

		__init_slab(slab_cache, addr, size);
	}	

	return 0;
}

void init_general_slab_cache(void)
{
	int i;

	printk("Init genernal slab cache.\n");
	for (i = 0; i < SLAB_SIZE_NUM; i++) {
		slab_cache_array[i].slab_size = slab_size[i];
		slab_cache_array[i].slab_num = SLAB_NUM;
		slab_cache_array[i].free_num = 0;
		slab_cache_array[i].ctor = NULL;
		slab_cache_array[i].dtor = NULL;
        	slab_cache_array[i].color_num = 
			compute_slab_color_num(slab_size[i], PAGE_SIZE);
        	slab_cache_array[i].color_next = 0;
		slab_cache_array[i].obj_cache.limit = 0;
		INIT_LIST_HEAD(&slab_cache_array[i].list);
		init_slab(&slab_cache_array[i], slab_size[i]);
	}
}

void *kmem_cache_alloc(struct slab_cache *slab_cache)
{
	struct slab *s = NULL;
	struct list_head *p = NULL;
	void *obj = NULL;

	if (!slab_cache->free_num) {
		if (!(s = expand_slab(slab_cache))) {
			printk("expand slab failed.\n");
			return NULL;
		}
		obj = get_slab_obj(s, slab_cache);
		return obj;
	}

	if (list_empty(&(slab_cache->list)))
		return NULL;

	list_for_each(p, (&(slab_cache->list))) {
		s = list_entry(p, struct slab, list);
		if (s && s->free_num) {
			obj = get_slab_obj(s, slab_cache);
			return obj;
		}
	}

	return NULL;
}

struct slab_cache *creat_kmem_cache(char *name, int size)
{
	struct slab_cache *cachep;
	int algin_size;

	cachep = (struct slab_cache *)kmem_cache_alloc(&kmem_cache_st);
	if (!cachep) {
		printk("create kmem cache failed.\n");
		return NULL;
	}
	printk("kmem cache alloc at 0x%x\n", cachep);

	cachep->slab_size = ALIGN(size, DEFAULT_ALIGN);
	cachep->slab_num = SLAB_NUM;
	cachep->free_num = 0;
	cachep->ctor = NULL;
	cachep->dtor = NULL;
	cachep->obj_cache.limit = 0;

	strcpy(cachep->name, name);

	INIT_LIST_HEAD(&(cachep->list));
	init_slab(cachep, cachep->slab_size);
	list_add_tail(&(cachep->cache_list), &kmem_list_head);

	return cachep;
}

struct slab_cache *search_slab_cache(char *name)
{
	struct slab_cache *s = NULL;
	struct list_head *p = NULL;

	list_for_each(p, (&kmem_list_head)) {
		s = list_entry(p, struct slab_cache, cache_list);
		if (s && !strcmp(name, s->name))
			return s;
	}

	return NULL;
}

void *kmem_cache_free(struct slab_cache *slab_cache, void *addr)
{
	struct slab *slab = NULL;
	struct list_head *p = NULL;
	
	if (!slab_cache || !addr)
		return ;

	slab = search_slab(addr, (&(slab_cache->list)));
	if (!slab) {
		printk("not found slab.\n");
		return ;
	}
	printk("found slab.\n");

	put_slab_obj(slab, addr, slab_cache);
}

void *kmem_cache_destory(struct slab_cache *slab_cache)
{
	int i;

	kmem_cache_free(&kmem_cache_st, (void *)slab_cache);
	
	for (i = 0; i < slab_cache->slab_num; i++)
		;	
}

void print_kmem_cache_list(void)
{
	struct slab_cache *s = NULL;
	struct list_head *p = NULL;

	list_for_each(p, (&kmem_list_head)) {
		s = list_entry(p, struct slab_cache, cache_list);
		if (s) {
			printk("cache name: %s slab size: %d slab num: %d "
				"free num: %d color num: %d slab_page: 0x%x\n",
				s->name, s->slab_size, s->slab_num, 
				s->free_num, s->color_num, s->slab_page); 
			__show_slab_list(&(s->list));
		}
	}
}

void init_kmem_cache(void)
{
	printk("init kmem cache.\n");

	INIT_LIST_HEAD(&kmem_list_head);

	kmem_cache_st.slab_size = SLAB_CACHE_SIZE;
	kmem_cache_st.slab_num = SLAB_NUM;
	kmem_cache_st.free_num = 0;
	kmem_cache_st.ctor = NULL;
	kmem_cache_st.dtor = NULL;
	kmem_cache_st.obj_cache.limit = 0;

	strcpy(kmem_cache_st.name, "kmem_cache_st");

	INIT_LIST_HEAD(&(kmem_cache_st.list));
	init_slab(&kmem_cache_st, SLAB_CACHE_SIZE);
	list_add_tail(&(kmem_cache_st.cache_list), &kmem_list_head);
}

void mm_test(void)
{
	int i;
	void *addr;

	//__show_slab_list(&slab_cache_array[4].list); show_slab_list();
	//show_slab_list();

	for (i = 0; i < 35; i++) {
		addr = kmalloc(128);
		printk("alloc addr at 0x%x\n", addr);
	}
	kfree(addr);

	addr = kmalloc(128);
	printk("alloc addr at 0x%x\n", addr);

	struct slab_cache *test;
	test = creat_kmem_cache("test", 32);

	print_kmem_cache_list();
	
	addr = kmem_cache_alloc(test);
	printk("kmem cache alloc at 0x%x\n", addr);
	addr = kmem_cache_alloc(test);
	printk("kmem cache alloc at 0x%x\n", addr);
	print_kmem_cache_list();
	kmem_cache_free(test, addr);
	addr = kmem_cache_alloc(test);
	printk("kmem cache alloc at 0x%x\n", addr);
	kmem_cache_destory(test);
}
