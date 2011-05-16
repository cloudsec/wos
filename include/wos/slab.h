#ifndef SLAB_H
#define SLAB_H

#define SLAB_SIZE_NUM				10
#define SLAB_NUM				2
#define SLAB_MAX_OBJ				PAGE_SIZE / 8

#define SLAB_CACHE_NAME_LEN			16

struct slab_obj_cache {
	unsigned int curr_obj;
	unsigned int limit;
	void *entry[];
};

struct slab_cache {
	struct slab_obj_cache obj_cache;
	void *slab_page;
	int slab_size;
	int slab_num;
	int free_num;
	int align;
	int color_num;
	int color_next;
	char name[SLAB_CACHE_NAME_LEN];
	void (*ctor)(void);
	void (*dtor)(void);
	struct list_head list;			/* slab list head */
	struct list_head cache_list;		/* slab cache list */
};

struct slab {
	int obj_num;
	int free_num;
	int free_idx;
	void *base;
	struct list_head list;
};

#define SLAB_CACHE_SIZE		sizeof(struct slab_cache)
#define SLAB_SIZE		sizeof(struct slab)

#define ALIGN(x, a)             (((x) + (a - 1)) & (~(a - 1)))
#define DEFAULT_ALIGN		4

#endif
