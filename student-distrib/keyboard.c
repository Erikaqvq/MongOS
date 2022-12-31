
#include "keyboard.h"
#include "scancode.h"
#include "cursor.h"

#define FULL_BUFFER 127
#define ENTER_ASCII 13

int CapsLock, LRShift, LCtrl, LAlt, Caps_on;

/* void keyboard_init(void)
    in: -
    out: -
    return: -
    description: initialize variables used in keyboard and enable irq
*/
void keyboard_init(){
    CapsLock = 0; // cap's status
    LRShift = 0; // consider two shift together
    LCtrl = 0; // ctrl key
    LAlt = 0; // alt key, not used in cp2
    Caps_on = 0; // for cap press/release
    enable_irq(KEYBOARD_PICNUM); // from cp1
}

/* void keyboard_irq(void)
    in: -
    out: -
    return: -
    description: get irq from keyboard using inb
                 check special character
                 put the character and save it in buffer
*/
void keyboard_irq(){
    cli();
    send_eoi(KEYBOARD_PICNUM);
    terminal_t* tds = &multi_terminal[display_terminal];
    uint8_t ch; // the input char
    unsigned char sch; // the corresponding char in scancode table
    /* get the typed character from port */
    ch = inb(KEYBOARD_PORT);
    if(K_TRUE == get_command_char(ch)){ // is command char
        // it has been checked in get_command_char, so return directly
        sti();
        return;
    }
    if(ch < CHARACTER_MAX){ // is a character in the scancode table
        sch = scancode_table[ch][0];
        if(LCtrl){ // deal with ctrl+l
            if(sch == 'l'){
                clear();
                tds->buffer_size = 0;
                sti();
                return;
            }
        }
        if (sch >= 'a' && sch <= 'z'){ // deal with a-z
            if (CapsLock != LRShift){ //  need to consider both
                sch = scancode_table[ch][1];
            }
        }
        else{ // not in alphabet, then just chect shift
            sch = (LRShift) ? scancode_table[ch][1] : scancode_table[ch][0];
        }
    }
    else{ // not in the table -- without this, each character will be printed twice
        // see buglog
        sti();
        return;
    }
    if(tds->buffer_size == FULL_BUFFER ){ // buffer is full
        
		if (sch == ENTER_ASCII){
			display_putc('\n');
			tds->keyboard_buffer[FULL_BUFFER] = ENTER_ASCII;
			tds->terminal_enable = 1;
            tds->buffer_size = 0; // clear the buffer
            sti();
            return;
		}
		if(sch == '\b'){ // deal with back
			display_putc(sch); // call putc first
			if(tds->buffer_size){ // check tab condition and null buffer
				if(tds->keyboard_buffer[tds->buffer_size - 1] == '\t'){  
				display_putc(sch); display_putc(sch); display_putc(sch);}
				tds->buffer_size--;
			}
            sti();
			return;
		}
        sti();
        return;
    }
    if(sch == '\b'){ // deal with back
        if(tds->buffer_size == 0){ // cannot delete ******************* add by jiahe 11/11
            sti();
            return;
        }
        display_putc(sch); // call putc first
        if(tds->buffer_size){ // check tab condition and null buffer
            if(tds->keyboard_buffer[tds->buffer_size - 1] == '\t'){  
			display_putc(sch); display_putc(sch); display_putc(sch);}
            tds->buffer_size--;
        }
        sti();
        return;
    }
    if(sch == '\t'){ // deal with tab
        display_putc(' '); display_putc(' '); display_putc(' '); display_putc(' '); // four spaces
        tds->keyboard_buffer[tds->buffer_size] = '\t';
        tds->buffer_size++;
        sti();
        return;
    }
    // finally it's a normal characters
    tds->keyboard_buffer[tds->buffer_size] = sch;
    tds->buffer_size++;
    display_putc(sch);
    if(sch == ENTER_ASCII) {tds->terminal_enable = 1; tds->buffer_size = 0;}
    sti();
    return;
}

/* int32_t get_command_char(uint8_t input)
    in: an input char (uint8_t)
    out: -
    return: K_TRUE - is an accepted command
            K_FALSE - is not an accepted command
    description: using switch to check the command / or not command
*/
int32_t get_command_char(uint8_t input){
    switch(input){
        case 0x2A: /* left shift 
		press */
            LRShift = 1;
            return K_TRUE;
        case 0x36: /* right shift press */
            LRShift = 1;
            return K_TRUE;
        case 0xAA: /* left shift release */
            LRShift = 0;
            return K_TRUE;
        case 0xB6: /* right shift release */
            LRShift = 0;
            return K_TRUE;
        case 0x1D: /* left ctrl press */
            LCtrl = 1;
            return K_TRUE;
        case 0x9D: /* left ctrl release */
            LCtrl = 0;
            return K_TRUE;
        case 0x3A: /* capslock press */
            Caps_on = 1;
            CapsLock = 1 - CapsLock;
            return K_TRUE;
        case 0xBA: /* capslock release */
            Caps_on = 0;
            return K_TRUE;
        case 0x38: /* alt press */
            LAlt = 1;
            return K_TRUE;
        case 0xB8: /* alt release */
            LAlt = 0;
            return K_TRUE;
        case 0x3B: /* F1 press */
            if (LAlt){
                terminal_switch(0);
            }
            return K_TRUE;
        case 0x3C: /* F2 press */
            if (LAlt){
                terminal_switch(1);
            }
            return K_TRUE;
        case 0x3D: /* F3 press */
            if (LAlt){
                terminal_switch(2);
            }
            return K_TRUE;
        case 0x01: /* ESC press */
            return K_TRUE;
        default: /* not a command */
            return K_FALSE;
    }
}

/* void init_terminal(void)
    in: -
    out: -
    return: -
    description: initialize variables used in keyboard and enable irq
*/
void init_terminal(){
    int i;
    for(i = 0; i < TERMINAL_SIZE; i++){
        memset(multi_terminal[i].keyboard_buffer, 0, KEYBOARD_SIZE);
        multi_terminal[i].buffer_size = 0;
        multi_terminal[i].terminal_enable = 0;
        multi_terminal[i].screen_x = 0;
        multi_terminal[i].screen_y = 0;
        multi_terminal[i].video_cur_pos = VID_MEM + (i + 1) * numfour * KB;
        multi_terminal[i].tid = i;
        multi_terminal[i].vidmap = 0;
        multi_terminal[i].running_pid = -1;
    }
    screen_xt = &multi_terminal[0].screen_x;
    screen_yt = &multi_terminal[0].screen_y;
    for(i = 0; i < MAX_PID; i++){
        pid_st[i] = 0;
    }
    return;
}
/* void open_terminal(void)
    in: -
    out: -
    return: -
    description: open the terminal, always success
*/
int32_t open_terminal(const uint8_t* filename){
    return 0;
}

/* void close_terminal(void)
    in: -
    out: -
    return: -
    description: close the terminal, always success
*/
int32_t close_terminal(int32_t fd){
    return 0;
}

/* int32_t read_terminal(uint32_t offs, void* buf, uint32_t nbytes)
    in: offs - ignored, buf - the buffer to read, nbytes - the length to read
    out: -
    return: -
    description: terminal should read the buffer
*/
int32_t read_terminal(int32_t inode, uint32_t* offse, void* buff, int32_t nbytes){
    terminal_t* tds = &multi_terminal[current_tid];
	if(nbytes > MAX_BUFFER_SIZE) {
        nbytes = MAX_BUFFER_SIZE;
	}
    if(0 == buff){ // invalid buffer
		display_printf("Invalid buffer pointer!\n");
		return T_FAIL;
	}
    while (!(tds->terminal_enable)); // and terminal start
    cli();
	tds->terminal_enable = 0; // when enter in keyboard set this flag to 1
	char * buf = buff;
    int kb_index; // index in keyboard buffer
    int cp_cnt = 0;   // used for counting how mant characters we have copied
    int i;

    for(kb_index = 0; kb_index < nbytes; kb_index++){
        if(kb_index >= LOG_ROWS){ // deal with overflow
            sti();
            return cp_cnt;
        }
        // if newline character copy it and set every character after it as null
        if(tds->keyboard_buffer[kb_index] == ENTER_ASCII){
           cp_cnt++;
           ((char*)buf)[kb_index] = '\n'; //copy keyboard buffer to buf
           kb_index++;
           for(i = kb_index;i < nbytes; i++){
            ((char*)buf)[i] = NULL;
           }
           sti();
           return cp_cnt; //return how many things we have copied
        }
        else{
            cp_cnt++;
           ((char*)buf)[kb_index] = tds->keyboard_buffer[kb_index];
        }

    }
    sti();
    return cp_cnt;
}

/* int32_t write_terminal(uint32_t offs, void* buf, uint32_t nbytes)
    in: offs - ignored, buf - the buffer to read, nbytes - the length to read
    out: -
    return: -
    description: terminal should write the buffer to the screen
*/
int32_t write_terminal(int32_t fd, const void* buf, int32_t nbytes){
    int i;
    uint8_t* buf_tmp = (uint8_t*)buf;
    if(0 == buf) return T_FAIL;
    for(i = 0; i < nbytes; i++ ){
        if(buf_tmp[i] == ENTER_ASCII){ // enter -- return directly
            putc(buf_tmp[i]);
        } 
        else putc(buf_tmp[i]);
    }
    return i;
}

/* --------- multi terminal functions---------*/
/* void terminal_switch(int32_t tid)
    in: tid - target terminal id
    out: -
    return: -
    description: display - switch to no.tid terminal (0, 1, 2)
*/
void terminal_switch(int32_t tid){
    if (tid == display_terminal){
        return;
    }

    terminal_t* dis_terminal = &multi_terminal[display_terminal];
    terminal_t* new_t = &multi_terminal[tid];
    screen_xt = &new_t->screen_x;
    screen_yt = &new_t->screen_y;
    update_cursor(*screen_xt, *screen_yt);
    vid_mem(display_terminal);
    // switch actual video memory
    memcpy((void*)(dis_terminal->video_cur_pos), (void*)VID_MEM, numfour * KB);
    memcpy((void*)VID_MEM, (void*)(multi_terminal[tid].video_cur_pos), numfour * KB);

    display_terminal = tid; // update new terminal id on display
    vid_mem(current_tid); // switch back to curent running terminal
}

/* void vid_mem(int32_t tid)
    in: tid - target terminal id
    out: -
    return: -
    description: switch video memory to no.tid terminal (0, 1, 2)
*/
void vid_mem(int32_t tid){

    if(tid != display_terminal){ 
        page_table_usermapping[OUR_VID_MAP].present = multi_terminal[tid].vidmap;
        page_table_usermapping[OUR_VID_MAP].address = multi_terminal[tid].video_cur_pos >> 12;
        page_table[VID_MEM >> offset].address = multi_terminal[tid].video_cur_pos >> 12;
    }
    else{ // switch back to current terminal, same as function vidmap
        page_table_usermapping[OUR_VID_MAP].present = multi_terminal[tid].vidmap;
        page_table_usermapping[OUR_VID_MAP].address = VID_MEM >> offset;
        page_table[VID_MEM >> offset].address = VID_MEM >> offset;
    }
   screen_xt = &multi_terminal[tid].screen_x;
   screen_yt = &multi_terminal[tid].screen_y;

    //flush the TLB here
    //https://wiki.osdev.org/TLB
    asm volatile (
        "movl %%cr3,%%eax           ;"
        "movl %%eax,%%cr3           ;"

        ::: "eax", "cc"
    );
}


/* --------- helper function---------*/
/* void display_putc(uint8_t key)
    in: key - putc character
    out: -
    return: -
    description: deal with putc for multi-terminal
*/
void display_putc(uint8_t key){
    vid_mem(display_terminal);
    putc(key);
    update_cursor(*screen_xt, *screen_yt);
    vid_mem(current_tid);
    return;
}

/* void display_printf(uint8_t* key)
    in: *key - printf string
    out: -
    return: -
    description: deal with printf for multi-terminal
*/
void display_printf(int8_t* key){
    vid_mem(display_terminal);
    printf(key);
    update_cursor(*screen_xt, *screen_yt);
    vid_mem(current_tid);
}
