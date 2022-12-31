#include "syscall.h"
#include "paging.h"
#include "keyboard.h"
#include "rtc.h"
#include "control.h"

#define VIDEO_OF12 0xB8
#define Fourmb     0x400000
#define OUR_MAP     0x22
#define SCREENST   (USER_VIDMEM_INDEX * Fourmb + 0x22000)

int32_t current_tid = 0;
int32_t display_terminal = 0;
uint32_t pid_cnt = 0; // use as global couting the number of processes

// set terminal operation 
static ops_t terminal_opss = {
    .open = open_terminal,
    .close = close_terminal,
    .read = read_terminal,
    .write = write_terminal
};

// set rtc operation 
static ops_t rtc_ops = {
    .open = rtc_open,
    .close = rtc_close,
    .read = rtc_read,
    .write = rtc_write
};

// set file operation 
static ops_t file_ops = {
    .open = open_f,
    .close = close_f,
    .read = read_f,
    .write = write_f
};

// set directory operation 
static ops_t dir_ops = {
    .open = open_d,
    .close = close_d,
    .read = read_d,
    .write = write_d
};
/* int32_t halt(uint8_t status)
The halt system call terminates a process, returning the specified value to its parent process. The system call handler itself is responsible for expanding the 8-bit argument from BL into the 32-bit return value to the parent program’s execute system call. Be careful not to return all 32 bits from EBX. This call should never return to the caller.
* INPUT: status - the status of halt
* OUTPUT: -
* return to execute
* RETURN: SUCC/FAIL
*/
int32_t halt (uint8_t status){
    cli();
	Pcb_t * cur_pcb; 
    Pcb_t* par_pcb;
    int32_t pid;
	// determine the return value
	uint16_t return_value = (uint16_t)status; 
	if (status == EXCEPTION) return_value = EXCEPT_RET;
	// calculate the address of pcb 
    terminal_t* current_terminal = &multi_terminal[current_tid];
    pid = current_terminal->running_pid;
    cur_pcb = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * pid);
    
    // close the process and deal with the counter
    free_pid(cur_pcb->pid);
    // close all file in fd array
    int i;
    for(i = REAL_FILE; i < FARRAY_SIZE; i++){
        if(cur_pcb->farray[i].flags == FREE) continue;
        close(i);
    }
	// if is shell restart
    if (cur_pcb->par_pid == -1) {  // -1: current process is base shell
        current_terminal->running_pid = -1;
        execute ((uint8_t*)"shell"); // restart
	// if not, back to the parent
    } else {
        par_pcb = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * cur_pcb->par_pid);
        uint32_t ebp = par_pcb->saved_ebp;
        uint32_t esp = par_pcb->saved_esp;
		//restore the tss for the processes
        current_terminal->running_pid = cur_pcb->par_pid;
        tss.esp0 = KERNEL_STACK_BUTTOM - (cur_pcb->par_pid - CAL_PCB) * PROCCESS_STACK_SIZE - FENCE_FOUR;
        tss.ss0 = KERNEL_DS;
        // deal with the paging mapping address
        paging_halt(cur_pcb->par_pid);
        current_terminal->vidmap = par_pcb->vidmap;
        halt_asm(esp, ebp, return_value);
    }
    return SYS_SUCC;
}

/* int32_t execute (const uint8_t* command)
The execute system call attempts to load and execute a new program, handing off the processor to the new program until it terminates. The command is a space-separated sequence of words. The first word is the file name of the program to be executed, and the rest of the command—stripped of leading spaces—should be provided to the new program on request via the getargs system call. The execute call returns -1 if the command cannot be executed, for example, if the program does not exist or the filename specified is not an executable, 256 if the program dies by an exception, or a value in the range 0 to 255 if the program executes a halt system call, in which case the value returned is that given by the program’s call to halt.
* INPUT: command - the command (filename and arg)
* OUTPUT: -
* RETURN: SUCC/FAIL
*/
int32_t execute (const uint8_t* command){
    cli();
    uint8_t fname[FILE_NAME];
    uint8_t farg[MAX_ARG_LENGTH];
    dentry_t tmpdentry;
    inode_t* tmpinode;
    uint32_t pid;
    uint32_t ent_add, exe_ret;
    int i;
    printf("[execute program on terminal:%d]\n",current_tid);
    /*
    1. Paging Helpers  ( optional, but very helpful )
    Map Virtual & Physical Memory (optional, needed for CP5)
    Unmap  Virtual & Physical Memory (optional, needed for CP5)
        Mapping Phys to Virtual func
        Unmap( optional)
    */
    /*2. Parse cmd */
    if(NULL == command){
        printf("Invalid command!\n");
        return 0;
    }
    for(i = 0; i < FILE_NAME; i++) fname[i] = 0; // clear the buffers
    for(i = 0; i < MAX_ARG_LENGTH; i++) farg[i] = 0;
    handle_command(command, fname, farg);
 
    /*3. File Checks
        Does the file exist?
        Is the file an EXE?
        Hint: look in MP3 documentation for some “magic numbers”
        Is the file valid?
        Remember to get prog_eip from valid files: 
        Look in MP3 documentation for how to do this
    */
    if(SYS_FAIL == read_dentry_by_name(fname, &tmpdentry)){
        return SYS_FAIL;
    }
    exe_ret = (handle_execuable_check(&tmpdentry));
    if(SYS_FAIL == exe_ret){
        return SYS_FAIL;
    }
    /*
    4. Create new PCB
        Give pcb memory
        Set active
        Set file descriptor
    */
    pid = get_pid();
    if(pid == -1){
        printf("CANNOT HAVE MORE PROCESS!\n");
		return 0;
    }
    
    Pcb_t* tmpPCB = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * pid );
    Pcb_t* parPCB;
    /* initialize tmpPCB */
    tmpPCB->pid = pid;
    tmpPCB->active = OPEN; //set it to active
    terminal_t* current_terminal = &multi_terminal[current_tid];
    tmpPCB->par_pid = current_terminal->running_pid;

    current_terminal->running_pid = pid;
    tmpPCB->vidmap = 0;
    for(i = 0; i < MAX_ARG_LENGTH; i++) tmpPCB->args[i] = 0;
    for(i = 0; i < MAX_ARG_LENGTH; i++) tmpPCB->args[i] = farg[i];
    for(i = 0 ; i < FILE_NAME ; i++ ) tmpPCB->name[i] = fname[i];


    for(i = 0; i < FARRAY_SIZE; i++){ // initialize fd array
        tmpPCB->farray[i].flags = (i < REAL_FILE) ? 1 : 0; // only stdin and stdout are valid at first
        tmpPCB->farray[i].inode = 0;
        tmpPCB->farray[i].file_pos = 0;
        if(i < REAL_FILE){
            tmpPCB->farray[i].fotp = &terminal_opss; // for stdin and stdout
        }
    }
    /*
    5. Setup memory (aka paging)
        Setup paging( you should use some helper functions)
            REMEMBER TO FLUSH TLB (OSDev is very helpful for this)
        Set save_ebp
        Set save_esp (to  a fixed value)
    */
	//handle with paging
    handle_exe_paging(pid);
	//save ebp and esp
    if(tmpPCB->par_pid != -1){
	register uint32_t saved_esp asm("esp");
	register uint32_t saved_ebp asm("ebp");
    parPCB = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * tmpPCB->par_pid );
    parPCB->saved_esp = saved_esp;
	parPCB->saved_ebp = saved_ebp;}
    tmpPCB->esp0 =  KERNEL_STACK_BUTTOM - (tmpPCB->pid - CAL_PCB) * PROCCESS_STACK_SIZE - FENCE_FOUR;
    
    /*
    6. Read exe data
    */
	// calculate the pointer of inode
    tmpinode = &inode_st[tmpdentry.index_node];
   //read data from files if fails return -1
    if(SYS_FAIL == read_data((int32_t*)tmpinode, 0, (char*) (USER_BASE + PROGRAM_ADDR_OFF), PROCCESS_STACK_SIZE)){
        printf("Cannot read data!\n");
        return SYS_FAIL;
    }
    /*
    7. Setup old stack & eip
        Hint: You can save current esp and ebp values by:
        register uint32 t saved_ebp asm("ebp");
        Or you can just use normal inline asm 
    8. Goto usermode
        OSdev Getting_to_Ring_3
        Switching to usermode requires pushing certain things to stack
        user_ds
        user_esp
        user_cs
        prog_eip  
        Look in MP3 Documentation for some of the above values
    */
   sti();
    // store the sip into ent_add
    ent_add = exe_ret;
	// restore the tss for context switch
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNEL_STACK_BUTTOM - (pid - CAL_PCB) * PROCCESS_STACK_SIZE - FENCE_FOUR;
	// context switch
	asm volatile(
        "MOVW %%AX, %%DS;"
		"PUSHL %%EAX;"
        "PUSHL %%EBX;"
        "PUSHFL;"
        "PUSHL %%ECX;"
        "PUSHL %%EDX;"
        "IRET;"
        "exe_return:"
        "leave;"
        "ret;"
        :
		: "a"(USER_DS), "b"(EXE_ESP), "c"(USER_CS), "d"(ent_add)
        : "cc", "memory"
    );
    return SYS_FAIL;
}


/* read (int32_t fd, void* buf, int32_t nbytes)
 * used to read files
 * Inputs: fd buf nbytes
 * Outputs: SUCC/FAIL
 * Side Effects: read
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes){
    int32_t pid;
    terminal_t* current_terminal = &multi_terminal[current_tid];
    pid = current_terminal->running_pid;
    Pcb_t* cur_pcb = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * pid );
    if(cur_pcb->name[0] == 'g' && cur_pcb->name[1] == 'r'){
        if(fd != ROOT_FD){ // force the id
            fd = FILE_FD;
        }
    }

    if(fd < FD_ARRAY_ST || fd >= FD_ARRAY_MAX){
        printf("invalid fd\n");
        return SYS_FAIL;
    }

    // check for null buffer
    if (buf == NULL){
        printf("invalid buffer\n");
       return SYS_FAIL;
    }

    // check for invalid nbytes
    if (nbytes < 0){
        printf("invalid nbytes\n");
        return SYS_FAIL;
    }    
	if(fd == 1) return SYS_FAIL;

    // get PCB for current PID
    //check if file has been opened
    if(cur_pcb->farray[fd].flags == FREE) {
        printf("file has not been openned!\n");
        return SYS_FAIL;
    }
    uint32_t cur_inode = (uint32_t) &inode_st[cur_pcb->farray[fd].inode];
    //uint32_t cur_pos = cur_pcb->farray[fd].file_pos;
    // use jump table to call read function
    // rtc read blocked until interrupted, and always return 0
    int32_t ret = cur_pcb->farray[fd].fotp->read(cur_inode,&cur_pcb->farray[fd].file_pos,buf,nbytes);
    return ret;
}


/* int32_t write (int32_t fd, const void* buf, int32_t nbytes)
 * used to write in files
 * Inputs: fd buf nbytes
 * Outputs: SUCC/FAIL
 * Side Effects: write
 */
int32_t write (int32_t fd, const void* buf, int32_t nbytes){
    // check if fd is valid
	if(fd < FD_ARRAY_ST || fd >= FD_ARRAY_MAX){
        printf("invalid fd\n");
        return SYS_FAIL;
    }
    // check if buffer is valid
    if (buf == NULL){
        printf("invalid buff\n");
       return SYS_FAIL;
    }
    // check of nbytes is valid
    if (nbytes < 0){
        printf("invalid nbytes\n");
        return SYS_FAIL;
    }
	if(fd == 0) return SYS_FAIL;
    // get current pcb
    int32_t pid;
    terminal_t* current_terminal = &multi_terminal[current_tid];
    pid = current_terminal->running_pid;
    Pcb_t* cur_pcb = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * pid);
    // check if file is opened
    if(cur_pcb->farray[fd].flags == FREE) {
        printf("file has not been openned!\n");
        return SYS_FAIL;
    }

    // return nbytes written
    return cur_pcb->farray[fd].fotp->write(fd, buf, nbytes);
}

/* int32_t open (const uint8_t* filename)
 * used to close files
 * Inputs: filenname
 * Outputs: SUCC/FAIL
 * Side Effects: close
 */
int32_t open (const uint8_t* filename){
    int i;
    dentry_t dentry;
    // check if file name is empty
    if(!strlen((char*)filename)){
        printf("file name can't be empty!\n");
        return SYS_FAIL;  
    }
    // read dentry
    if(read_dentry_by_name(filename, &dentry) == SYS_FAIL){
        printf("read_dentry_by_name failed\n");
        return SYS_FAIL;  
    }
    // get current pcb
    int32_t pid;
    terminal_t* current_terminal = &multi_terminal[current_tid];
    pid = current_terminal->running_pid;
    Pcb_t* cur_pcb = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * pid);
    // find a free spot and write fd
    for(i = FD_ARRAY_MIN; i < FD_ARRAY_MAX; i++) {
        if(cur_pcb->farray[i].flags == FREE) { // free is 0

            cur_pcb->farray[i].file_pos = INIT_POS;
            cur_pcb->farray[i].inode = dentry.index_node;
            cur_pcb->farray[i].flags = OPEN;
            // fill into corresponding jump table
            if(dentry.type == 0){ // 0: rtc
                cur_pcb->farray[i].fotp = &rtc_ops;
            }
            else if(dentry.type == 1){ // 1:dir
                cur_pcb->farray[i].fotp = &dir_ops;
            }
            else{ // 2: regular file
                cur_pcb->farray[i].fotp = &file_ops;
            }
            // no need to call open from jump table
            // return fd for success
            if(SYS_FAIL != cur_pcb->farray[i].fotp->open(filename)) return i; 
            return SYS_FAIL;
        }
    }
	return SYS_FAIL;
}

/* int32_t close (int32_t fd)
 * used to close files
 * Inputs: fd
 * Outputs: SUCC/FAIL
 * Side Effects: close
 */
int32_t close (int32_t fd){
    int32_t pid;
    terminal_t* current_terminal = &multi_terminal[current_tid];
    pid = current_terminal->running_pid;
    Pcb_t* tmpPCB = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * pid);
    if(tmpPCB->name[0] == 'g' && tmpPCB->name[1] == 'r'){
        if(fd != ROOT_FD){ // force the id
            fd = FILE_FD;
        }
    }
    if(fd < FD_ARRAY_MIN || fd >= FD_ARRAY_MAX){
        printf("invalid fd\n");
        return SYS_FAIL;
    }
    
    if(tmpPCB->farray[fd].fotp == NULL){
        printf("invalid fd\n");
        return SYS_FAIL;
    }
    if(tmpPCB->farray[fd].flags == FREE){
        printf("can't close file that is not openned\n");
        return SYS_FAIL; 
    }
    if(tmpPCB->farray[fd].fotp->close(fd) == SYS_SUCC){
        tmpPCB->farray[fd].fotp = NULL;
        tmpPCB->farray[fd].inode = DEFAULT_INODE;
        tmpPCB->farray[fd].file_pos = INIT_POS;
        tmpPCB->farray[fd].flags = FREE;
        return SYS_SUCC;
    } else{
        printf("UNABLE TO CLOSE FILE!\n");
        return SYS_FAIL;
    }
}


/* void handle_command(uint8_t* command)
 * The command is a space-separated sequence of words. 
 * The first word is the file name of the program to be executed, 
 * and the rest of the command—stripped of leading spaces—should be 
 * provided to the new program on request via the getargs system call.
 * Inputs: the command to translate, desc pointers to name and arg
 * Outputs: - 
 * Side Effects: fill fname and fagr
 */
void handle_command(const uint8_t* command, uint8_t* fname, uint8_t* farg){
    int i, j, len;
	char* strcmd = (char*) command;
	len = strlen(strcmd);
    for(i = 0; (i < FILE_NAME && i < len); i++){
        if(' ' == command[i]) break;
        fname[i] = command[i];
    }
    j = 0;
    while(' ' == command[i]) i++; // remove ALL the space before argument
    for(; (i < MAX_ARG_LENGTH && i < len); i++){
        farg[j++] = command[i];
    }
    return;
}

/* int32_t handle_execuable_check(dentry_t* dentry)
 * check whether it is an execuable file
 * Inputs: target dentry (not NULL)
 * Outputs: SUCC/FAIL
 * Side Effects: -
 */
uint32_t handle_execuable_check(dentry_t* dentry){
    uint32_t read_len, ret_val;
 int32_t* inode_ptr;
    uint8_t mybuf[MAGIC_LEN];
    int i;

    if(2 != dentry->type){
        printf("Invalid file type!\n");
        return SYS_FAIL;
    }
 inode_ptr = (int32_t*)(&inode_st[dentry->index_node]);
    read_len = read_data(inode_ptr, 0, (char*)mybuf, MAGIC_LEN);
    if(MAGIC_LEN != read_len){
        printf("Wrong length! Not an execuable file!\n");
        return SYS_FAIL;
    }
    for(i = 0; i < MAGIC_START; i++){
        if(magic_nums[i] != mybuf[i]){
            printf("Wrong magic number! Not an execuable file!\n");
            return SYS_FAIL;
        }
    }
    ret_val = 0;
    for(i = EIP_TOT-1; i >= 0; i--){ // get eip from 24-27 byte in buffer
        ret_val <<= BYTE_LEN;
  ret_val |= (mybuf[i + EIP_POS] & bitmask); 
    }
    return ret_val;
}

/* int32_t handle_exe_paging(dentry_t* dentry)
 * handle with paging while execute opening the directory
 * Inputs: NULL
 * Outputs: NULL
 * Side Effects: -
 */
void handle_exe_paging(int32_t pid){ 
    page_directory[PDE_INDEX].mb.user_supervisor=1;  
    page_directory[PDE_INDEX].mb.address= PDE_ST + pid - 1;

    //flush the TLB here
    //https://wiki.osdev.org/TLB
    asm volatile (
        "movl %%cr3,%%eax           ;"
        "movl %%eax,%%cr3           ;"

        ::: "eax", "cc"
    );
}


/* int32_t paging_halt(dentry_t* dentry)
 * handle with halt paging
 * Inputs: pid
 * Outputs: NULL
 * Side Effects: -
 */
void paging_halt(uint32_t parent_pid){
    page_directory[PDE_INDEX].mb.address = PDE_ST + parent_pid - 1;
    if(display_terminal == current_tid){
        page_table_usermapping[OUR_MAP].present = 0;
    }
    else  page_table_usermapping[OUR_MAP + 1 + current_tid].present = 0;
    //flush the TLB here
    //https://wiki.osdev.org/TLB
    asm volatile (
        "movl %%cr3,%%eax           ;"
        "movl %%eax,%%cr3           ;"

        ::: "eax", "cc"
    );
}

/* void handle_switch_paging()
 * handle with paging while execute opening the directory
 * Inputs: NULL
 * Outputs: NULL
 * Side Effects: -
 */
void handle_switch_paging(int32_t pid){
    page_directory[PDE_INDEX].mb.address= PDE_ST + pid - 1;

    //flush the TLB here
    //https://wiki.osdev.org/TLB
    asm volatile (
        "movl %%cr3,%%eax           ;"
        "movl %%eax,%%cr3           ;"

        ::: "eax", "cc"
    );
}


/* int32_t getargs (uint8_t* buf, int32_t nbytes)
 * used by execute, after handling command, the part other than name will be handled here
 * input: buf - the rest of the command - stripped of leading spaces - should be provided to the new program on reauest via the getargs system call
 * nbytes - length to copy
 * output: / 
 * return: SYS_FAIL / SYS_SUCC
*/
int32_t getargs (uint8_t* buf, int32_t nbytes){
    uint8_t tmpargs[MAX_ARG_LENGTH];
    uint32_t tmplen;
    int32_t i;
    int32_t pid;
    if(NULL == buf) return SYS_FAIL;
    terminal_t* current_terminal = &multi_terminal[current_tid];
    pid = current_terminal->running_pid;
    Pcb_t* cur_pcb = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * pid);
    for(i = 0; i < MAX_ARG_LENGTH; i++) tmpargs[i] = cur_pcb->args[i];
    tmplen = strlen((char*)tmpargs);
    if (tmplen > nbytes) return SYS_FAIL;
    i = tmplen - 1;
    while(tmpargs[i] == '\n' || tmpargs[i] == 0) i--;
    if(i == SYS_FAIL) return SYS_FAIL;
    strncpy((char*)buf,(char*)tmpargs, nbytes);
    return SYS_SUCC;
}

/* int32_t vidmap (uint8_t** screen_start)
 * the viamap call maps the text-mode video into user space at a preset virtual address
 * input: screen_start - start position
 * output: / 
 * return: SYS_FAIL / SYS_SUCC
*/
int32_t vidmap (uint8_t** screen_start){
    int32_t pid;
    if(NULL == screen_start){
        return SYS_FAIL;
    }
    uint32_t st_add = (uint32_t) screen_start;
    if(PDE_INDEX != (st_add >> PDE_OFFSET)){
        return SYS_FAIL;
    }
    terminal_t* current_terminal = &multi_terminal[current_tid];
    pid = current_terminal->running_pid;
    Pcb_t* cur_pcb = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * pid);
    cur_pcb->vidmap = OPEN;
    current_terminal->vidmap = OPEN;
    int32_t open_map = OUR_MAP + 1 + current_tid;
    if(display_terminal == current_tid){
        page_table_usermapping[OUR_MAP].present = OPEN;
        page_table_usermapping[OUR_MAP].address = VIDEO_OF12;
    }
    else{
        page_table_usermapping[open_map].present = OPEN;
        page_table_usermapping[open_map].address = VIDEO_OF12;
    }

    //flush the TLB here
    //https://wiki.osdev.org/TLB
    asm volatile (
        "movl %%cr3,%%eax           ;"
        "movl %%eax,%%cr3           ;"

        ::: "eax", "cc"
    );
    *screen_start = (uint8_t*) (SCREENST);
    return SYS_SUCC;
}

/* extra credit */
int32_t set_handler (void){
    return SYS_FAIL;
}

/* extra credit */
int32_t sigreturn (void){
    return SYS_FAIL;
}

/* void halt_asm(uint32_t esp,uint32_t ebp,uint16_t return_value)
 * the end of halt function, ret to execute
 * Inputs: saved eso, ebp and return_value
 * Outputs: NULL
 * Side Effects: -
 */
void process_switch(int32_t tid){
    terminal_t* current_terminal = &multi_terminal[current_tid];
    int32_t old_process = current_terminal->running_pid;
    Pcb_t* tmpPCB;
    if(old_process > 0 && old_process <= MAX_PROCESS){ 

    tmpPCB = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * old_process);
    register uint32_t saved_esp asm("esp");
	register uint32_t saved_ebp asm("ebp");
    tmpPCB->saved_ebp = saved_ebp;
    tmpPCB->saved_esp = saved_esp;
    tmpPCB->esp0 = tss.esp0;
    }
    
    // now switch to the new terminal
    terminal_t* new_terminal = &multi_terminal[tid];
    int32_t new_process = new_terminal->running_pid;
    tmpPCB = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * new_process);
    if(new_process == -1){ // initialize the terminal
       current_tid = tid;
        execute ((uint8_t*)"shell");
        return;
    }
    // else, load new ebp, esp, tss, paging
    handle_switch_paging(new_process);
    int32_t s_esp = tmpPCB->saved_esp;
    int32_t s_ebp = tmpPCB->saved_ebp;
    tmpPCB = (Pcb_t*)(KERNEL_STACK_BUTTOM - PROCCESS_STACK_SIZE * new_process);
    tss.ss0 = KERNEL_DS;
    tss.esp0 = tmpPCB->esp0;
    current_tid = tid;
    
    asm volatile(
        "movl %0,%%esp   ;"
        "movl %1,%%ebp   ;"
        "leave  ;"
        "ret    ;"
        :
        : "r" (s_esp), "r" (s_ebp)
        : "esp","ebp"
    );
}

/* void halt_asm(uint32_t esp,uint32_t ebp,uint16_t return_value)
 * the end of halt function, ret to execute
 * Inputs: saved eso, ebp and return_value
 * Outputs: NULL
 * Side Effects: -
 */
void halt_asm(uint32_t esp,uint32_t ebp,uint16_t return_value){
    asm volatile(
    "movl %%edx,%%esp  ;"
    "movl %%ecx,%%ebp  ;"
    "movl %%ebx,%%eax  ;"
    "jmp exe_return"
   :
   : "d"(esp), "c"(ebp), "b"(return_value)
   : "esp", "ebp", "eax");
}

/* int32_t get_pid()
 * arrange a new pid to current process
 * Inputs: -
 * Outputs: a not used pid or -1 for full
 * Side Effects: -
 */
int32_t get_pid(){
    int i;
    for(i = 0; i < MAX_PID; i++){
        if(pid_st[i] == 0){
            pid_cnt++;
            pid_st[i] = 1;
            return i + 1; // our pid starts from 1
        }
    }
    return -1;
}

/* void free_pid(int32_t pid)
 * free one pid in halt
 * Inputs: pid
 * Outputs: -
 * Side Effects: -
 */
void free_pid(int32_t pid){
    pid_st[pid - 1] = 0;
    pid_cnt--;
}
