#ifndef SLAB_H
#define SLAB_H

#define SLAB_SIZE_NUM				10
#define SLAB_NUM				2
#define SLAB_MAX_OBJ				PAGE_SIZE / 8

struct slab_cache {
	void *slab_page;
	int slab_size;
	int slab_num;
	int free_num;
	struct list_head list;
};

struct slab {
	int obj_num;
	int free_num;
	int free_idx;
	void *base;
	struct list_head list;
};

#endif
