#include "process.h"
#include "mem.h"
#include "memory.h"
#include "vm.h"
#include "debug.h"
#include "print.h"
#include "../fs/fat16/impl.h"

extern TSS tss_64_ptr;
static Process process_table[NUM_PROC];
static int32_t pid_num = 1;
static ProcessControl pc;

static void set_tss(Process *proc)
{
    // rsp0 = stack top of kernel stack
    // rsp0 is used when jumping from ring3 to ring0
    tss_64_ptr.rsp0 = proc->stack + STACK_SIZE;
}

static Process* find_unused_process(void)
{
    Process *process = NULL;

    for (int32_t i = 0; i < NUM_PROC; i++)
    {
        if (process_table[i].state == UNUSED)
        {
            process = &process_table[i];
            break;
        }
    }

    return process;
}

static void set_process_tf_regs(Process *proc)
{
    // setup for from ring0 to ring3
    // 0x10 is 2nd entry of gdt
    // 3 is RPL =3 (ring3)
    proc->tf->cs = 0x10 | 3;
    proc->tf->rip = 0x400000; // base address (virtual memory) of program of user space
    // 0x18 is 3rd entry of gdt
    // 3 is RPL =3 (ring3)
    proc->tf->ss = 0x18 | 3;
    proc->tf->rsp = 0x400000 + PAGE_SIZE; // end of address (virtual memory) of program of user space
    // 0x202
    // bit-9 (Interrupt Enable Flag) is 1 which means enable interrupt
    // bit-1 is 1 which is reserved
    proc->tf->rflags = 0x202;
}

// set process in kernel space
static Process *set_process_entry(Process *proc)
{
    uint64_t stack_top;

    proc->state = INIT;
    proc->pid = pid_num++;

    // alloc page to stack (in kernel space) for program
    proc->stack = (uint64_t)kmalloc();
    if (proc->stack == 0)
    {
        return NULL;
    }    

    kmemset((void*)proc->stack, 0, PAGE_SIZE);
    stack_top = proc->stack + STACK_SIZE;
     // 7 = number of registers + return address
    proc->context = stack_top - sizeof(TrapFrame) - 7 * 8;
    // 6 = number of registers
    // which are rbx, rbp, r12, r13, r14, r15 in swap
    // TrapReturn is return address for returning to ring3
    *(uint64_t*)(proc->context + 6 * 8) = (uint64_t)TrapReturn;
    /*
        --low address--
            r15
            r14
            r13
            r12
            rbp
            rbx
            return address (TrapReturn)
        --high address--
    
    */

    proc->tf = (TrapFrame*)(stack_top - sizeof(TrapFrame));

    // kernel stack
    /*
        --low address--
            rip (0x400000)
            cs
            rflags
            rsp
            ss

        --high address--
    
    */
    set_process_tf_regs(proc);

    // setup virtual memory for kernel space of process
    proc->page_map = setup_kvm();
    
    if (proc->page_map == 0)
    {
        kfree(proc->stack);
        kmemset(proc, 0, sizeof(Process));
        return NULL;
    }

    return proc;
}

static Process* alloc_new_process(void)
{
    Process *proc = find_unused_process();

    if (proc == NULL)
    {
        return NULL;
    }

    return set_process_entry(proc);
}

static void init_idle_process(void)
{
    Process *process;
    ProcessControl *process_control;

    process = find_unused_process();
    ASSERT(process == &process_table[0]);

    process->pid = 0;
    // since idle process only run in kernel mode
    // use current kernel stack for idle process
    // read pml4 table base address (physical address) from cr3
    process->page_map = P2V(read_cr3());
    process->state = RUNNING;

    process_control = get_pc();
    process_control->current_process = process;
}

static void init_user_process(void)
{
    ProcessControl *process_control;
    Process *process;
    HeadList *list;

    process_control = get_pc();
    list = &process_control->ready_list;

    process = alloc_new_process();
    ASSERT(process != NULL);
    // setup virtual memory for user space of process (which runs program)
    // size of user space is 5120 bytes = 10 * 512 bytes (10 sectors for each program)
    // user program is loaded at 0x30000 (physical address)
    ASSERT(setup_uvm(process->page_map, P2V(0x30000), 5120));
    process->state = READY;
    append_list_tail(list, (List*)process);
}

ProcessControl *get_pc(void)
{
    return &pc;
}

void init_process(void)
{
    init_idle_process();
    init_user_process();
}

// process example
// void init_process(void)
// {
//     ProcessControl *process_control;
//     Process *process;
//     HeadList *list;
//     // addr are physical memory addresses of programs
//     uint64_t addr[3] = {0x20000, 0x30000, 0x40000};

//     process_control = get_pc();
//     list = &process_control->ready_list;
//     for (int i = 0; i < 3; i++)
//     {
//         process = find_unused_process();
//         // initialize processes
//         set_process_entry(process, addr[i]);
//         // add to ready list
//         append_list_tail(list, (List*)process);
//     }
// }
// void launch(void)
// {
//     ProcessControl *process_control;
//     Process *process;
//     process_control = get_pc();
//     // remove from ready list
//     process = (Process*)remove_list_head(&process_control->ready_list);
//     process->state = RUNNING;
//     process_control->current_process = process;
//     set_tss(process);
//     switch_vm(process->page_map);
//     // run the program of process in user space
//     pstart(process->tf);
// }

// context switch between processes
static void switch_process(Process *prev, Process *current)
{
    set_tss(current);
    switch_vm(current->page_map);
    swap(&prev->context, current->context);
}

static void schedule(void)
{
    Process *prev_proc;
    Process *current_proc;
    ProcessControl *process_control;
    HeadList *list;

    process_control = get_pc();
    prev_proc = process_control->current_process;
    list = &process_control->ready_list;

    // if ready list is empty, run idle process
    if (is_list_empty(list))
    {
        // idle process is not in ready list
        // check current process is idle process (pid = 0)
        ASSERT(process_control->current_process->pid != 0);
        current_proc = &process_table[0];
    }
    else
    {
        // remove from ready list
        current_proc = (Process*)remove_list_head(list);
    }

    current_proc->state = RUNNING;   
    process_control->current_process = current_proc;

    switch_process(prev_proc, current_proc);   
}

// after timer interrupt to do context switch 
void yield(void)
{
    ProcessControl *process_control;
    Process *process;
    HeadList *list;
    
    process_control = get_pc();
    list = &process_control->ready_list;

    // if ready list is empty
    if (is_list_empty(list))
    {
        return;
    }

    process = process_control->current_process;
    process->state = READY;

    // if process is not idle process(pid=0), then add to ready list
    if (process->pid != 0)
    {
        // add to ready list
        append_list_tail(list, (List*)process);
    }

    schedule();
}

// "wait" is identifier to target process to sleep
void sleep(int32_t wait)
{
    ProcessControl *process_control;
    Process *process;
    
    process_control = get_pc();
    process = process_control->current_process;
    process->state = SLEEP;
    process->wait = wait;
    
    // add to wait list
    append_list_tail(&process_control->wait_list, (List*)process);
    schedule();
}
// "wait" is identifier to target process to wake up
void wake_up(int32_t wait)
{
    ProcessControl *process_control;
    Process *process;
    HeadList *ready_list;
    HeadList *wait_list;

    process_control = get_pc();
    ready_list = &process_control->ready_list;
    wait_list = &process_control->wait_list;
    // remove the process which has identifier "wait" from wait list
    process = (Process*)remove_list(wait_list, wait);
    
    // continue to find all processes which have the same identifier "wait" in wait list
    while (process != NULL)
    {   
        process->state = READY;
        // add to ready list
        append_list_tail(ready_list, (List*)process);
        // remove the process which has identifier "wait" from wait list
        process = (Process*)remove_list(wait_list, wait);
    }
}

void exit(void)
{
    ProcessControl *process_control;
    Process* process;
    HeadList *list;

    process_control = get_pc();
    process = process_control->current_process;
    process->state = KILLED;
    process->wait = process->pid;

    list = &process_control->kill_list;
    // add killed process to kill list
    append_list_tail(list, (List*)process);

    // other process could be in sleep state, so wake it up first
    // it should wake up the process which has identifier = -3
    // processes which have identifier = -3 caused by wait(pid)
    wake_up(-3);
    schedule();
}

void wait(int32_t pid)
{
    ProcessControl *process_control;
    Process *process;
    HeadList *list;

    process_control = get_pc();
    list = &process_control->kill_list;

    while (true)
    {
        // free resources of process
        if (!is_list_empty(list))
        {
            // remove pid from kill list
            process = (Process*)remove_list(list, pid);
            if (process != NULL)
            {
                ASSERT(process->state == KILLED);
                // free kernel stack of proces
                kfree(process->stack);
                // free page of process
                free_vm(process->page_map);

                // handling fd/fcb of process
                for (int32_t i = 0; i < 100; i++)
                {
                    if (process->file[i] != NULL)
                    {
                        process->file[i]->fcb->count--;
                        process->file[i]->count--;

                        if (process->file[i]->count == 0)
                        {
                            process->file[i]->fcb = NULL;
                        }
                    }
                }

                kmemset(process, 0, sizeof(Process));
                break;
            }
        }

        // put process (which has identifier = -3) into sleep
        // it means we wait for process to exit
        sleep(-3);
    }
}

// A parent process calls fork to create a child process, a duplicate of the parent
// sharing the same code and memory space
int32_t fork(void)
{
    ProcessControl *process_control;
    Process *process;
    Process *current_process;
    HeadList *list;

    process_control = get_pc();
    current_process = process_control->current_process;
    list = &process_control->ready_list;

    process = alloc_new_process();
    if (process == NULL)
    {
        ASSERT(0);
        return -1;
    }

    // copy virtual memory in user space from current process to new process
    if (copy_uvm(process->page_map, current_process->page_map, PAGE_SIZE) == false)
    {
        ASSERT(0);
        return -1;
    }

    // copy fd from currnet process to new process
    kmemcpy(process->file, current_process->file, 100 * sizeof(FileDesc*));

    for (int32_t i = 0; i < 100; i++)
    {
        if (process->file[i] != NULL)
        {
            process->file[i]->count++;
            process->file[i]->fcb->count++;
        }
    }

    // copy trapfram from current process to new process
    kmemcpy(process->tf, current_process->tf, sizeof(TrapFrame));
    // return value is 0 after returning to user mode
    process->tf->rax = 0;
    process->state = READY;
    // add to ready list
    append_list_tail(list, (List*)process);

    return process->pid;
}
// The child process calls exec to replace its memory space with a new program.
// process is the the child process
// name is the name of program
int32_t exec(Process *process, char *name)
{
    int32_t fd;
    uint32_t size;
    
    fd = open_file(process, name);

    if (fd == -1)
    {
        exit();
    }

    kmemset((void*)0x400000, 0, PAGE_SIZE);
    // copy data and program into 
    size = get_file_size(process, fd);
    size = read_file(process, fd, (void*)0x400000, size);
    
    if (size == UINT32_MAX)
    {
        exit();
    }

    close_file(process, fd);

    kmemset(process->tf, 0, sizeof(TrapFrame));
    set_process_tf_regs(process);

    return 0;
}


// *list is dummy pointer of HeadList
// list->next is head of HeadList
List* remove_list(HeadList *list, int32_t wait)
{
    List *current = list->next;
    List *prev = (List*)list;
    List *item = NULL;

    while (current != NULL)
    {
        if (((Process*)current)->wait == wait)
        {
            prev->next = current->next;
            item = current;

            if (is_list_empty(list))
            {
                // set tail of HeadList
                list->tail = NULL;
            }
            else if (current->next == NULL)
            {
                list->tail = prev;
            }

            break;
        }

        prev = current;
        current = current->next;    
    }
    return item;
}
