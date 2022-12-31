/*
 * Header file used to initialize paging
 */

#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"
#include "x86_desc.h"

// Some constants
// Table address offset
#define paging_offset     12 /* from osdev */
#define VIDEO_MEM         0xB8000
#define VIDEO_BUFFER_MAX  0xB8000
#define USER_IMAGE        32
#define USER_VIDMEM_INDEX 33 //128/4+1


// function used to initial paging
void paging_init();

extern void set_regs(void* ptr);

#endif
