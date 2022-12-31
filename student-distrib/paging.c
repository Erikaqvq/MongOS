#include "paging.h"
#include "control.h"


// paging_dir: PDE structure array, size = 1024
/* Set parameters in 4kb page directory entry */
void set_paramter_pde_kb(int i ,int flag);
/* Set parameters in 4mb page directory entry */
void set_paramter_pde_mb(int i, int flag);
/* Set parameters in page table entry */
void set_paramter_pte(int i, int flag);
/* Set parameters in page table user mapping for init */
void init_user_mapping(int i, int flag);


/* paging_init()
    input: -
    output: -
    Description: initialize page_directory and page_table
*/
void paging_init(){
    // initialize
    int index;
    // set up pde first
    for (index = 0; index < PDE_TOT_NUM; index++){
        if (index == 0){ // 0 - 4mb, seperate as kbs
            set_paramter_pde_kb(index,1);
        }
        else if(index == 1 || index == USER_IMAGE){ // 4 - 8mb, kernel, 128mb user image
            set_paramter_pde_mb(index,1);
        }
        else{
            set_paramter_pde_mb(index,0); // dne
			if(index == USER_VIDMEM_INDEX){
				page_directory[index].kb.present = 1;
                page_directory[index].kb.user_supervisor = 1;  //1:it is user
                page_directory[index].kb.address=(int)page_table_usermapping >> paging_offset;//not sure
			}
        }
    }
    // then set up pte
    for (index = 0; index < PTE_TOT_NUM; index++){
        init_user_mapping(index,0);
        if(index == OUR_VID_MAP){
            page_table_usermapping[index].present = 1;
            page_table_usermapping[index].address = VIDEO_MEM >> paging_offset;
        }
        if(index >= VIDEO_MEM >> paging_offset && index <= VIDEO_BUFFER_MAX >> paging_offset) set_paramter_pte(index,1);
		else set_paramter_pte(index,1);
    }
	index = VIDEO_MEM >> paging_offset; // set video memory
	page_table[index].present = 1;
	page_table[index].cache_disable = 0;
	
    set_regs(page_directory); // set registers, in _asm.S
}

/* set_paramter_pde_kb(int i, int flag)
    input: i - index in dir, flag - 0/1, used to set some args
    output: -
    Description: initialize page_directory - kb
    Reference: I32 reference book & osdev
*/
void set_paramter_pde_kb(int i, int flag){
    page_directory[i].kb.present = flag;
    page_directory[i].kb.read_write = 1; //not sure
    page_directory[i].kb.user_supervisor = 0;
    page_directory[i].kb.write_through = 0;
    page_directory[i].kb.cache_disable = 0;
    page_directory[i].kb.accessed = 0;
    page_directory[i].kb.reserved = 0; //set to 0
    page_directory[i].kb.page_size = 0; // 0 for 4kb
    page_directory[i].kb.global_page = 1;
    page_directory[i].kb.available = 0; //not sure
    page_directory[i].kb.address = (int)page_table >> paging_offset;//not sure
}

/* set_paramter_pde_mb(int i, int flag)
    input: i - index in dir, flag - 0/1, used to set some args
    output: -
    Description: initialize page_directory - mb
    Reference: I32 reference book & osdev
*/
void set_paramter_pde_mb(int i, int flag){
    page_directory[i].mb.present = flag;
    page_directory[i].mb.read_write = 1; //not sure
    page_directory[i].mb.user_supervisor = 0;  //geuess it is supervisor
    page_directory[i].mb.write_through = 0;
    page_directory[i].mb.cache_disable = 0;
    page_directory[i].mb.accessed = 0;
    page_directory[i].mb.dirty = 0; //set to 0
    page_directory[i].mb.page_size = flag; // 1 for 4mb
    page_directory[i].mb.global = 1;
    page_directory[i].mb.available = 0; //not sure
    page_directory[i].mb.pat = flag; //not sure
    page_directory[i].mb.reserved = 0; //must set to 0
    page_directory[i].mb.address = i;//not sure
}

/* set_paramter_pte(int i, int flag)
    input: i - index in dir, flag - 0/1, used to set some args
    output: -
    Description: initialize page_table
    Reference: I32 reference book & osdev
*/
void set_paramter_pte(int i, int flag){
    page_table[i].present = flag;
    page_table[i].read_write = 1; //not sure
    page_table[i].user_supervisor = 0;  //geuess it is supervisor
    page_table[i].write_through = 0;
    page_table[i].cache_disable = 0;
    page_table[i].accessed = 0;
    page_table[i].dirty = 0; //set to 0
    page_table[i].pat = 0; //not sure
    page_table[i].global_page = 1;
    page_table[i].available = 0; //not sure
    page_table[i].address = i;
}

void init_user_mapping(int i, int flag){
    page_table_usermapping[i].present = 0;
    page_table_usermapping[i].read_write = 1; 
    page_table_usermapping[i].user_supervisor = 1; //1:it is user
    page_table_usermapping[i].write_through = 0;
    page_table_usermapping[i].cache_disable = 0;
    page_table_usermapping[i].accessed = 0;
    page_table_usermapping[i].dirty = 0; 
    page_table_usermapping[i].pat = 0; 
    page_table_usermapping[i].global_page = 1;
    page_table_usermapping[i].available = 0; 
    page_table_usermapping[i].address = i;
}


