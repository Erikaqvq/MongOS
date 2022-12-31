#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "lib.h"
#include "ops.h"
#include "filesys.h"


#define USER_BASE           0x08000000 // from instruction
#define KERNEL_STACK_BUTTOM 0x0800000 //8mb
#define PROCCESS_STACK_SIZE 0x2000    //8kb
#define EXCEPTION           0xFF      //status for exceptions
#define EXCEPT_RET          256

#define SYS_SUCC 0
#define SYS_FAIL -1
#define MAX_ARG_LENGTH 128 // from instruction
#define MAGIC_LEN 40 // from instruction
#define MAGIC_START 4 // from instruction
#define FARRAY_SIZE 8 // from instruction
#define PDE_INDEX 0x20 // calculate according to instruction
#define USER_STACK 0x400000
#define FENCE_FOUR 0x4
#define EXE_ESP 0x08400000 - FENCE_FOUR
#define PROGRAM_ADDR_OFF 0x48000
#define REAL_FILE 2
#define EIP_TOT 4
#define EIP_POS 24
#define BYTE_LEN 8
#define MAX_PROCESS 6
#define PDE_OFFSET 22
#define CAL_PCB 1

#define bitmask 0xFF

#define INIT_POS 0
#define FREE 0
#define OPEN 1
#define FD_ARRAY_ST 0
#define FD_ARRAY_MIN 2
#define FD_ARRAY_MAX 8
#define DEFAULT_INODE 0

#define RTC_JUMPTABLE 0
#define DIR_JUMPTABLE 1
#define FILE_JUMPTABLE 2
#define STDIN_JUMPTABLE 3
#define STDOUT_JUMPTABLE 4
#define NULL_JUMPTABLE 5

#define PDE_ST 2
#define ROOT_FD 2
#define FILE_FD 3

static const uint8_t magic_nums[MAGIC_START] = {0x7f, 0x45, 0x4c, 0x46}; // from instruction

typedef struct f_array_t{ 
    ops_t* fotp; /* file operation table pointer - 4B */
    uint32_t inode;
    uint32_t file_pos;
    uint32_t flags;
}f_array_t;

typedef struct Pcb_t{
    uint32_t pid;
    uint32_t par_pid;
    f_array_t farray[FARRAY_SIZE]; /* Each task can have up to 8 open files */
    uint32_t saved_esp;
    uint32_t saved_ebp;
    uint32_t esp0;
    uint32_t active;
    uint8_t args[MAX_ARG_LENGTH];
    uint8_t name[FILE_NAME];
    uint32_t vidmap;
    volatile int32_t rtc_trigger;
    int32_t rtc_counter;
    int32_t rtc_maxcount;
} Pcb_t;


/*
Prototypes appear below. Unless otherwise specified, successful calls should return 0, and failed calls should return -1.
*/
int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);

int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);

int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (void);
int32_t sigreturn (void);

/* helper functions */
Pcb_t* get_ptr(uint8_t pid);
void handle_command(const uint8_t* command, uint8_t* fname, uint8_t* farg);
uint32_t handle_execuable_check(dentry_t* dentry);
void handle_exe_paging(int32_t pid);
void context_switch();
void paging_halt(uint32_t parent_id);
int32_t init(f_array_t* fd_array);
void halt_asm(uint32_t esp,uint32_t ebp,uint16_t return_value);
int32_t get_pid();
void free_pid(int32_t pid);

void process_switch(int32_t tid);
void handle_switch_paging(int32_t pid);

#endif
