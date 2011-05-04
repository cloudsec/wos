#ifndef FS_H
#define FS_H

#define DEFAULT_HD_SIZE			100*1024*1024
#define BLOCK_SIZE			512

struct super_block {
	unsigned char sb_magic;
	unsigned int sb_start;
	unsigned int sb_blocks;
	unsigned int sb_dmap_blks;
	unsigned int sb_imap_blks;
	unsigned int sb_inode_blks;
};

struct inode {
	unsigned int i_mode;
	unsigned int i_size;
	unsigned int i_block[8];
};

#define FS_BOOT_BLK(sb)			((sb).sb_start)
#define FS_SUPER_BLK(sb)		(FS_BOOT_BLK(sb) + 1)
#define FS_DMAP_BLK(sb)			(FS_SUPER_BLK(sb) + 1)
#define FS_IMAP_BLK(sb)			(FS_DMAP_BLK(sb) + sb.sb_dmap_blks)
#define FS_INODE_BLK(sb)		(FS_IMAP_BLK(sb) + sb.sb_imap_blks)
#define FS_DATA_BLK(sb)			(FS_DATA_BLK(sb) + sb.sb_inode_blks)

#define INODE_SIZE			(sizeof(struct inode))
#define INODE_MAP_BLKS			1	/* 512 * 8 = 4096 */
#define INODES_PER_BLK			(BLOCK_SIZE / INODE_SIZE)
#define INODE_BLKS			((INODE_MAP_BLKS*BLOCK_SIZE*8*INODE_SIZE) / BLOCK_SIZE)

// (1 + 1 + 1 + inode + x + x*512*8) = TOTAL_BLKS
#define TOTAL_BLKS			(DEFAULT_HD_SIZE / BLOCK_SIZE)
#define DATA_BLKS1			(TOTAL_BLKS - INODE_BLKS - INODE_MAP_BLKS - 1 - 1)
#define DATA_MAP_BLKS			((TOTAL_BLKS - INODE_BLKS - INODE_MAP_BLKS \
						 - 1 - 1) / (BLOCK_SIZE*8 + 1))
#define DATA_BLKS			(TOTAL_BLKS - DATA_MAP_BLKS - INODE_BLKS \
						 - INODE_MAP_BLKS - 1 - 1)


#endif
