#include <wos/mm.h>
#include <wos/fs.h>

int init_fs(void)
{
	printk("hd blocks: %d inode blocks: %d dmap_blocks: %d data_blocks: %d\n",
		TOTAL_BLKS, INODE_BLKS, DATA_MAP_BLKS, DATA_BLKS);

}
