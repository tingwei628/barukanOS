#ifndef PROCESS_H
#define PROCESS_H

#include "idt.h"
#include "list.h"

#define STACK_SIZE      (2 * 1024 * 1024) // 2MB
#define NUM_PROC        10

typedef enum {
    UNUSED,
    INIT,
    RUNNING,
    READY,
    SLEEP,
    KILLED
} ProcessState;

// Process inherits from List
typedef struct Process {
	List *next;
    int32_t pid;
	int32_t state;
	int32_t wait;	// identifier for sleep or wake up
	uint64_t context; // saving kernel stack value of process before context switch
	uint64_t page_map;	
	uint64_t stack; // kernel mode
	TrapFrame *tf;
} Process;

// save task state into tss before context switch
// after context switch, it will restore from tss
typedef struct __attribute__((packed)) {
    uint32_t res0;
    uint64_t rsp0; // rsp0 = stack top of kernel stack
    uint64_t rsp1;
    uint64_t rsp2;
	uint64_t res1;
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint64_t res2;
	uint16_t res3;
	uint16_t iopb;
} TSS;


typedef struct ProcessControl {
	Process *current_process;
	HeadList ready_list;
	HeadList wait_list;
	HeadList kill_list;
} ProcessControl;

void init_process(void);
void launch(void);
void yield(void);
void sleep(int32_t wait);
void wake_up(int32_t wait);
void exit(void);
void wait(void);

List* remove_list(HeadList *list, int32_t wait);

#endif