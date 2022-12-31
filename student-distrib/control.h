#ifndef _CONTROL_H
#define _CONTROL_H

#include "types.h"


#define K_SIZE 128
#define TERMINAL_SIZE 3
#define MAX_PID 6
#define OUR_VID_MAP 0x22

typedef struct terminal_t{
    /* store the keyboard buffer and status */
    uint8_t keyboard_buffer[K_SIZE];
    uint32_t buffer_size;
    volatile uint8_t terminal_enable;

    /* store the cursor */
    int screen_x;
    int screen_y;

    uint32_t video_cur_pos; // memory mapping
    uint8_t tid; // unique terminal id: 0, 1, 2
    int32_t vidmap; // vidmap status
    int32_t running_pid; // running process for the terminal
} terminal_t;


extern int32_t current_tid;
extern int32_t display_terminal;
extern uint32_t pid_cnt; // use as global couting the number of processes

terminal_t multi_terminal[TERMINAL_SIZE];

int32_t pid_st[MAX_PID];

#endif

