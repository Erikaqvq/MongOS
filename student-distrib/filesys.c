#include "filesys.h"


/* helper function */
void * mycopy(void *dst,const void *src,uint32_t num);

/* int32_t fileSystem_init(uint32_t fs_start)
    input: fs_start - pointer to the start of boot block
    output: -
    return: FAIL / SUCC
    Description: read the dentry according to the file name
*/
int32_t fileSystem_init(uint32_t fs_start){
    int size_of_inodes;
    if(0 == fs_start){
        printf("ERROR! NULL POINTER!\n");
        return SFAIL;
    }
    boot_b = (btblk_t*) fs_start;
    if(boot_b->num_dir_entries > MAX_FILES){
        printf("ERROR! TOO MANY FILES\n");
        return SFAIL;
    }
    inode_st = (inode_t*) ((btblk_t*) fs_start + 1); /* 1: according to Appendix A */
    size_of_inodes = boot_b->num_inodes;
    datab_st = (datablk_t*) (inode_st + size_of_inodes);
    return SUCC;
}

/* int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
    input: fname - name of the file, dentry - position to save the corresponding dentry
    output: -
    return: FAIL / SUCC
    Description: read the dentry according to the file name
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    int i, j;
    uint32_t length_of_name, length_of_d;
    dentry_t tmp;
    if(0 == fname){ /* invalid file name */
        printf("ERROR! INVALID FILE NAME!\n");
        return SFAIL;
    }
    if(0 == boot_b){ /* not initialize */
        printf("ERROR! HAVEN'T INITIALIZE!\n");
        return SFAIL;
    }
    length_of_name = strlen((const int8_t*)fname);
	if(fname[length_of_name - 1] == enter){
		length_of_name--;
	}
	for(i = 0; i < length_of_name; i++){
		if(fname[i] == NULL) break;
	}
	length_of_name = i;
    if(length_of_name > FILE_NAME){/* invalid file name length*/
        printf("ERROR! FILE NAME IS TOO LONG!\n");
        return SFAIL;
    }
    for(i = 0; i < boot_b->num_dir_entries; i++){
        tmp = boot_b->dir_entries[i];
        length_of_d = strlen((const int8_t*)tmp.filename);
		for(j = 0; j < length_of_d; j++){
			if(tmp.filename[j] == NULL) break;
		}
		length_of_d = j;
		if(length_of_d > FILE_NAME) length_of_d = FILE_NAME;
		if(length_of_name != length_of_d) continue;

        if(0 != strncmp(tmp.filename, (const int8_t*)fname, length_of_name)) continue;
		if(NULL == dentry){
			return SFAIL;
		}
		*dentry = tmp;
        return SUCC;
    }
    printf("ERROR! FILE NAME DOES NOT MATCH!\n");
    return SFAIL;
}

/* int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
    input: index - index of the file, dentry - position to save the corresponding dentry
    output: -
    return: FAIL / SUCC
    Description: read the dentry according to the file index
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    if(0 == boot_b){ /* not initialize */
        printf("ERROR! HAVEN'T INITIALIZE!\n");
        return SFAIL;
    }
    if(index >= boot_b->num_dir_entries){ /* invalid index */
        printf("ERROR! INVALID INDEX!\n");
        return SFAIL;
    }
    *dentry = boot_b->dir_entries[index];
    return SUCC;
}

/* int32_t read_data (uint32_t inode, uint32_t offset, char* buf, uint32_t length)
    input: inode - #inode to read, offset - offset from the start of file, buf - buffer to save the chars, length - length to read
    output: -
    return: FAIL / read bytes
    Description: read the data in the file (according the inode number)
*/
int32_t read_data (int32_t* inode, uint32_t off, char* buf, uint32_t length){
    uint32_t first_index,last_index;
    inode_t* inode_f;
    uint32_t begin_position;
    uint32_t end_position;
    uint32_t i;
    uint32_t cur_bytes = 0; /* initialize to 0 */
    uint32_t flag1 = 1; /* flags: 0/1 to mark the integrity of blocks */
    uint32_t flag2 = 1; /* flags: 0/1 to mark the integrity of blocks */
    uint32_t index_data;
    datablk_t* cur_pointer;
   
    inode_f = (inode_t*) inode;

    if(0 == boot_b){ /* not initialize */
        printf("ERROR! HAVEN'T INITIALIZE!\n");
        return SFAIL;
    }
    if(off >= inode_f->size) return 0; /* ljh: why success here??? */
    if(inode_f == NULL) return SFAIL;
    
    uint32_t end_tmp = off + length; /* end position to read */
    if(end_tmp > inode_f->size) end_tmp = inode_f->size; /* too large? */

    first_index = off / BLOCK_SIZE; /* start block */
    last_index  = end_tmp / BLOCK_SIZE; /* end block */
   
    if (first_index * BLOCK_SIZE == off) flag1 = 0;
    if ((last_index + 1) * BLOCK_SIZE == end_tmp) flag2 = 0;

    for(i = first_index;i <= last_index;i++){
        begin_position = 0;
        end_position = BLOCK_SIZE;
        index_data = inode_f->data_blocks[i];
        cur_pointer = &datab_st[index_data];
        if (flag1 && (i == first_index)) begin_position = off - i * BLOCK_SIZE;
        if (flag2 && (i == last_index)) end_position = end_tmp - i * BLOCK_SIZE;
        memcpy(&buf[cur_bytes],(char*)cur_pointer+begin_position,end_position-begin_position);
        cur_bytes += end_position - begin_position;
    }
    return cur_bytes;
}

/* int32_t dir_read(int32_t fd, void * buf, int32_t nbytes)
    input: fd - offset from the beginning (now always 0), buf - buffer to save the chars, nbytes - #bytes to read
    output: -
    return: the bytes read
    Description: Reads directory entries from the bootblock one at a time
    The file descriptor will have a file position entry that you use to determine which directory entry you’re reading
    Grab the dentry
    Increment the file position by 1 each time you read a directory
    Copy the file name from the dentry to the buffer you passed in
*/
int32_t dir_read(int32_t fd, void * buf, int32_t nbytes){
   dentry_t* dir_temp;
   
   if(0 == boot_b){ /* not initialize */
        printf("ERROR! HAVEN'T INITIALIZE!\n");
        return SFAIL;
    }
    if(fd >= boot_b->num_dir_entries) return SFAIL; /* invalid index */
    if(fd >= MAX_FILES) return SFAIL; 

    dir_temp = &(boot_b->dir_entries[fd]);
    
    if(nbytes > strlen(dir_temp->filename)) nbytes = strlen(dir_temp->filename);
    mycopy((char*)buf,(char*)dir_temp->filename,nbytes);
    return nbytes;
}

/* void print_all()   ---- cp2 test function
    input: -
    output: -
    Description: for test1_cp2 -- list all files
*/
void print_all(){
    // for test1 -- list all files 
    int i, j;
    dentry_t tmp;
    inode_t* tmpnode;
    printf("\n\n");
    printf("           -------TEST #1: list all files-------\n");
    if(0 == boot_b){ // 0: not initialize 
        printf("ERROR! HAVEN'T INITIALIZE!\n");
        return;
    }
    for(i = 0; i < boot_b->num_dir_entries; i++){
        tmp = boot_b->dir_entries[i];
        printf("file_name:");
        for(j = 0; j < FILE_NAME; j++){
            putc(tmp.filename[j]);
        }
        printf(", file_type: ");
        printf("%d",tmp.type);
        printf(", file_size: ");
        tmpnode = (inode_t*) &inode_st[tmp.index_node];
        printf("%d\n",tmpnode->size);
    }
}

/* void print_one(char* name)   ---- cp2 test function
    input: name - file's name in string
    output: -
    Description: for test2_cp2 -- read file by name
*/
void print_one(char* name, int32_t offset){
    int i;
	int len;
    dentry_t tmp;
    inode_t* tmpnode;
    if(0 == boot_b){ // 0: not initialize 
        printf("ERROR! HAVEN'T INITIALIZE!\n");
        return;
    }
    if(SFAIL == read_dentry_by_name((const uint8_t* )name, &tmp)) return;
    tmpnode = (inode_t*) &inode_st[tmp.index_node];
	len = (offset == -1) ? tmpnode->size : offset;
    for(i = 0; i < tmpnode->size; i++){
        if(0 == mybuf[i]) continue;
        printf("%c",mybuf[i]);
    }
    printf("\n");
    printf("filename: ");
    printf("%s",name);
    printf("\n");
}

/* int32_t open_f(char* name)
    input: name - file's name in string
    output: FAIL / SUCC
    Description: should always be true once system is initialized and name exists
*/
int32_t open_f(const uint8_t* name){
    dentry_t tmp;
    if(0 == boot_b){ /* 0: not initialize */
        printf("ERROR! HAVEN'T INITIALIZE!\n");
        return SFAIL;
    }
    if(SFAIL == read_dentry_by_name((const uint8_t* )name, &tmp)){
        printf("ERROR! FILE DOES NOT EXITS!\n");
        return SFAIL;
    }
    if(2 != tmp.type){ /* 2 for a regular file  */
        printf("ERROR! NOT A REGULAR FILE!\n");
        return SFAIL;
    }
    return SUCC;
}

/* int32_t close_f(char* name)
    input: name - file's name in string
    output: FAIL / SUCC
    Description: should always be true once system is initialized
*/
int32_t close_f(int32_t fd){
    if(0 == boot_b){ /* 0: not initialize */
        printf("ERROR! HAVEN'T INITIALIZE!\n");
        return SFAIL;
    }
    return SUCC;
}

/* int32_t open_d(char* name)
    input: name - dir's name in string
    output: FAIL / SUCC
    Description: should always be true once system is initialized and name exists
*/
int32_t open_d(const uint8_t* name){
    if(0 == boot_b){ /* not initialize */
        printf("ERROR! HAVEN'T INITIALIZE!\n");
        return SFAIL;
    }
    return SUCC;
}

/* int32_t close_d(char* name)
    input: name - dir's name in string
    output: FAIL / SUCC
    Description: should always be true once system is initialized
*/
int32_t close_d(int32_t fd){
    if(0 == boot_b){ /* not initialize */
        printf("ERROR! HAVEN'T INITIALIZE!\n");
        return SFAIL;
    }
    return SUCC;
}

/* int32_t write_f(char* name)
    input: name - file's name in string
    output: FAIL 
    Description: write always fail
*/
int32_t write_f(int32_t fd, const void* buf, int32_t nbytes){
    printf("READ ONLY FILE SYSTEM!\n");
    return SFAIL;
}

/* int32_t write_d(char* name)
    input: name - dir's name in string
    output: FAIL 
    Description: write always fail
*/
int32_t write_d(int32_t fd, const void* buf, int32_t nbytes){
    printf("READ ONLY FILE SYSTEM!\n");
    return SFAIL;
}

/*  void read_f(char* name)
    input: name - file's name in string
    output: -
    return: length of reading
    Description: read file: call print_one
*/
int32_t read_f(int32_t inode, uint32_t* offset, void* buf, int32_t nbytes){
	int32_t len;
    if(0 == boot_b){ /* 0: not initialize */
        printf("ERROR! HAVEN'T INITIALIZE FILESYSTEM!\n");
        return SFAIL;
    }
    len = read_data((int32_t*)inode, *offset, buf, nbytes);
    if(len >= 0){*offset = *offset + len;}
   return len;
}

/*  void read_d(void)
    input: -
    output: -
    Description: read directory list: call print_all
*/
int32_t read_d(int32_t inode, uint32_t* offset, void* buf, int32_t nbytes){
   int32_t retval;
   retval = real_dir_read(*offset, buf, nbytes);
   if(SFAIL == retval) return SFAIL;
   if(0 == retval) return 0;
   *offset = *(offset) + 1;
   return retval;
}

int32_t real_dir_read(uint32_t offset, void* buf, int32_t nbytes){
	int32_t retval;
	if(offset >= MAX_FILES || offset < 0) return SFAIL;
	if(NULL == buf) return SFAIL;
	dentry_t tmp;
	tmp = boot_b->dir_entries[offset];
	if(0 == strlen(tmp.filename)) return 0;
	mycopy((char*)buf, (char*)tmp.filename, nbytes);
	retval = strlen(tmp.filename);
	if(retval > FILE_NAME) retval = FILE_NAME;
	return FILE_NAME;
}

/*--------------helper function------------*/

/*  void * mycopy(void *dst,const void *src, uint32_t num)
    input: dst - positon copy to, src - position copy from, num - bit of copy
    output: -
    Description: copy src[] to dst[], a function like mymcpy
    Reference: https://blog.csdn.net/goodwillyang/article/details/45559925#
*/
void * mycopy(void *dst,const void *src, uint32_t num){
    int nchunks = num / sizeof(dst);   
    int slice =   num % sizeof(dst);  
    
    unsigned long * s = (unsigned long *)src;
    unsigned long * d = (unsigned long *)dst;
    
    while(nchunks--)
        *d++ = *s++;
        
    while (slice--)
        *((char *)d++) =*((char *)s++);
        
    return dst;
}

