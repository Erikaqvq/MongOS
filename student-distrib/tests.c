#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "keyboard.h"
#include "filesys.h"
#include "syscall.h"
#include "sys_w.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here
/* division_zero_excepction_test
 * 
 * test the functionality to  raise exception of division zero
 * Inputs: None
 * Outputs: PASS/FAIL
 */
int division_zero_excepction_test() {
    TEST_HEADER;
	int num1 = 10;
	int num2 = 0;
	num1 = num1 / num2;
    return 0; 
}

/* null_pointer_test
 * 
 * test if dereferencing null pointer raises exception
 * Inputs: None
 * Outputs: PASS/FAIL
 */
int null_pointer_test() {
    TEST_HEADER;
    int* null_pointer = 0;
	int num;
	num = *null_pointer;
    return 0;
}

/* kernel_mem_edge_test1
 * 
 * test 0-4 mb, should occur exception
 * Inputs: None
 * Outputs: PASS/FAIL
 */
int kernel_mem_edge_test1() {
    TEST_HEADER;

    int num;
    int* ptr = (int*) 0x3FFFFF;
    num = *ptr;
    return 0;
}

/* kernel_mem_edge_test2
 * 
 * test >=8 mb, should occur exception
 * Inputs: None
 * Outputs: PASS/FAIL
 */
int kernel_mem_edge_test2() {
    TEST_HEADER;

    int num;
    int* ptr = (int*) 0x800000;
    num = *ptr;
    return 0;
}

/* kernel_mem_edge_test3
 * 
 * test a liitle smaller than video memory, should occur exception
 * Inputs: None
 * Outputs: PASS/FAIL
 */
int kernel_mem_edge_test3() {
    TEST_HEADER;

    int num;
    int* ptr = (int*) 0xB7FFF;
    num = *ptr;
    return 0;
}

/* kernel_mem_edge_test4
 * 
 * test a little larger than video memory, should occur exception
 * Inputs: None
 * Outputs: PASS/FAIL
 */
int kernel_mem_edge_test4() {
    TEST_HEADER;

    int num;
    int* ptr = (int*) 0xB9000;
    num = *ptr;
    return 0;
}

/* kernel_valid_test1
 * 
 * test a pos in kernel, should be PASS
 * Inputs: None
 * Outputs: PASS/FAIL
 */
int kernel_valid_test1() {
    TEST_HEADER;

    int num;
    int* ptr = (int*) 0x410000;
    num = *ptr;
    return 1;
}

/* kernel_valid_test2
 * 
 * test a pos in video memory, should be PASS
 * Inputs: None
 * Outputs: PASS/FAIL
 */
int kernel_valid_test2() {
    TEST_HEADER;

    int num;
    int* ptr = (int*) 0xB8100;
    num = *ptr;
    return 1;
}

/* rtc_test
 * 
 * set rtc freq
 * Inputs: None
 * Outputs: None
 */
void rtc_test(){
	TEST_HEADER;

	RTC_init();
}

/* paging_test
 * 
 * randomly choose some positon and test the .present
 * Inputs: None
 * Outputs: None
 */
int paging_test() {
	TEST_HEADER;
	
	if(page_directory[0].kb.present!=1 || page_directory[1].mb.present!=1){
		printf("wrong paging directory. \n");
		return FAIL;
	}

	if(page_table[0xB8].present!=1){
		printf("wrong paging table. \n");
		return FAIL;
	}

	return PASS;
}

/* system_test
 * 
 * system call
 * Inputs: None
 * Outputs: None
 */
void system_test(){
	TEST_HEADER;
	
	asm volatile("int $128");
}

/* ------------------------------------------------------------------------------------------------------------------ */

/* Checkpoint 2 tests */
/* 
 * print_one_file
 * Description: test the function of read_data
 * Inputs: -
 * Outputs: -
 */
void print_one_file(){
	/* the second argument of read_f is used for limiting the length to read(in order to see the ELF in the beginning)*/
	//read_f("frame0.txt",-1); // -1 means read the whole file
	//read_f("frame1.txt",-1);
	//read_f("ls",-1);
	//read_f("grep",-1);
	//read_f("grep",100); // it is a test for showing "ELF" in the beginning
	//read_f("verylargetextwithverylongname.txt",-1); // it should be error
	//read_f("verylargetextwithverylongname.tx",-1);  // this should match
	//read_f("fish",-1);
	//read_f("hibunnie",-1); // it is a garbage input
}

/* 
 * print_dir_list
 * Description: test the function of read_dir lists
 * Inputs: -
 * Outputs: -
 */
void print_dir_list(){
	//read_d();
} 


/* 
 * test_rtc_virtualization
 * Test RTC at a given frequency
 * Inputs: uint32_t set_freq
 * Outputs: None
 */
/*int test_rtc_virtualization() {
    TEST_HEADER;
    uint32_t i, j;
	uint32_t set_freq = 2;
	for(i = 1; i <= 10; i++){ // from 2^1 to 2^10 (2,4,8,16 ... 1024)
		rtc_write(NULL, &set_freq, sizeof(uint32_t));     // set frequency
		printf("RTC running at frequency: %u", set_freq);
		putc('\n');
    	for(j = 0; j < 10; j++) { // 60: random number
      	  rtc_read(NULL, NULL, NULL);                   // if read, print indicator
       	 putc('>');
		 }
		set_freq = set_freq * 2;
		putc('\n');
	}
	return PASS;
}*/

/* 
 * terminal_test1
 * Description: test the terminal functions: open, read, write, close
 * Inputs: -
 * Outputs: -
 */
/*int terminal_test1(){
	TEST_HEADER;
	
	int32_t cnt_r, cnt_w; // used to count the num of in/out characters
	char mybuf[MAX_BUFFER_SIZE];
	
	open_terminal();
	printf("MongOS > Hello! This is terminal test for checkpoint2!\nMongOS > How are you?\nMongOS > You can enter q to quit.\n");
	while(0 != strncmp("q",mybuf,1)){ // 1: comare length = 1
		memset(mybuf, 0, MAX_BUFFER_SIZE); // 0: not used for cp2
		cnt_r = read_terminal(0, mybuf, MAX_BUFFER_SIZE);
		if(0 == strncmp("q",mybuf,1)){ // 1: comare length = 1
			printf("MongOS > Bye!\n");
		}
		else{
			printf("MongOS > You say:\n");
			cnt_w = write_terminal(0, mybuf, MAX_BUFFER_SIZE); // 0: not used for cp2
			printf("MongOS > You read %d characters, write %d characters successfully!\n", cnt_r, cnt_w);
		}
	}
	close_terminal();
	return PASS;
}*/
/* Checkpoint 3 tests */
/* 
 * int32_t execute_test
 * Description: test execute for command = "testprint" - from instruction
 * Inputs: -
 * Outputs: -
 * return PASS/FAIL
 */
int32_t execute_test(){
	TEST_HEADER;
	execute((uint8_t*)"shell");
	return PASS;
}
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* launch_tests
 * 
 * Test suite entry point
 * Inputs: None
 * Outputs: None
 */
void launch_tests(){
	/* Test 1: idt test */
	// TEST_OUTPUT("idt_test", idt_test());
	
	/* Test 2: divide by 0 */
	// TEST_OUTPUT("division_zero_excepction_test", division_zero_excepction_test());
	
	/* Test 3: null pointer */
	//TEST_OUTPUT("null_pointer_test", null_pointer_test());
	
	/* Test 4,5,6 7: memory edge test -- all should be FAIL */
	//TEST_OUTPUT("kernel_mem_edge_test1", kernel_mem_edge_test1());
	//TEST_OUTPUT("kernel_mem_edge_test2", kernel_mem_edge_test2());
	//TEST_OUTPUT("kernel_mem_edge_test3", kernel_mem_edge_test3());
	//TEST_OUTPUT("kernel_mem_edge_test4", kernel_mem_edge_test4());

	/* Test 8,9: memory valid test -- all should be PASS */
	//TEST_OUTPUT("kernel_valid_test1", kernel_valid_test1());
	//TEST_OUTPUT("kernel_valid_test2", kernel_valid_test2());

	/* Test 10: set rtc frequency */
	//rtc_test();

	/* Test 11: rtc test provided by lib */
	//test_interrupts();

	/* Test 12: paging test */
	//TEST_OUTPUT("paging_test", paging_test());

	/* Test 13:system call */
	//system_test();

	/* ------------------------------------------------------------------------------------------- */
	
	/* checkpoint 2 */
	/* test 1: list all files */
	//print_dir_list();

	/* test 2: read file by name */
	//print_one_file();

	/* test 3: RTC - Test frequency at 8HZ */
	//TEST_OUTPUT("RTC_test", test_rtc_virtualization());
	
	/* test 4: Terminal - test the terminal */
	//TEST_OUTPUT("terminal_test", terminal_test1());
	
	/* ------------------------------------------------------------------------------------------- */
	
	/* checkpoint 3 */
	TEST_OUTPUT("exe", execute_test()); // then we can reach the tests in shell
}

