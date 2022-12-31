#ifndef _FILE_SYS
#define _FILE_SYS

#include "lib.h"

#define BLOCK_SIZE 4096 /* Appendix A: 4kB */
#define INODE_BLK (BLOCK_SIZE / 4 - 1) /* lenth of lenth is at most 4B */
#define MAX_FILES 63 /* Appendix A: 63 files */
#define FILE_NAME 32  /* Appendix A: up to 32 characters */
#define SFAIL -1
#define SUCC 0
#define MAX_FILE_SIZE 65536 /* Temporarily. Because the max file size is 36164 now(cp2) */
#define enter 13

typedef struct dentry_t{
    char filename[FILE_NAME];
    uint32_t type; /* 0 for a file giving user-level access to the real-time clock (RTC), 1 for the directory, and 2 for a regular file */
    uint32_t index_node; /* only meaningful for regular files */
    uint8_t reserved[24]; /* Appendix A: reserved 24 bits */
}dentry_t;

/* boot block */
typedef struct btblk_t{ /* boot block */
    uint32_t num_dir_entries;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    uint8_t reserved[52]; /* Appendix A: reserved 52 bits */
    dentry_t dir_entries[MAX_FILES]; /* Appendix A: 63 files */
}btblk_t;

typedef struct inode_t{ /* index nodes */
    uint32_t size;
    uint32_t data_blocks[INODE_BLK];
}inode_t;

typedef struct datablk_t{ /* data blocks */
    uint8_t datas[BLOCK_SIZE];
}datablk_t;

btblk_t* boot_b; /* pointer to the start of boot block (0) */
inode_t* inode_st; /* pointer to the start of index node (1) */
datablk_t* datab_st; /* pointer to the start of data blocks (N+1) */
char mybuf[MAX_FILE_SIZE]; /* used for print functions (test) */

/* Open and close will always be successful in this file system */
int32_t open_f(const uint8_t* filename);
int32_t close_f(int32_t fd);
int32_t write_f(int32_t fd, const void* buf, int32_t nbytes);
int32_t read_f(int32_t inode, uint32_t* offst, void* buf, int32_t nbytes); // read file: call print_one

int32_t open_d(const uint8_t* filename);
int32_t close_d(int32_t fd);
int32_t write_d(int32_t fd, const void* buf, int32_t nbytes);
int32_t read_d(int32_t inode, uint32_t* offst, void* buf, int32_t nbytes); // read directory list: call print_all


/* Inside the file system */
int32_t fileSystem_init(uint32_t fs_start);
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (int32_t* inode, uint32_t off, char* buf, uint32_t length);
int32_t dir_read(int32_t fd, void * buf, int32_t nbytes);
int32_t real_dir_read(uint32_t offst, void* buf, int32_t nbytes);
void print_all();
void print_one(char* name,int32_t offsete);


#endif

