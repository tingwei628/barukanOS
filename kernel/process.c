#include "process.h"
#include "memory.h"
#include "vm.h"
#include "debug.h"
#include "print.h"

extern TSS tss_64_ptr;
static Process process_table[NUM_PROC];
static int32_t pid_num = 1;
static ProcessControl pc;

static void set_tss(Process *proc)
{
    tss_64_ptr.rsp0 = proc->stack + STACK_SIZE;
}

static Process* find_unused_process(void)
{
    Process *process = NULL;

    for (int i = 0; i < NUM_PROC; i++)
    {
        if (process_table[i].state == UNUSED)
        {
            process = &process_table[i];
            break;
        }
    }

    return process;
}

static void set_process_entry(Process *proc, uint64_t addr)
{
    uint64_t stack_top;

    proc->state = INIT;
    proc->pid = pid_num++;

    // alloc page to stack (in kernel space) for program
    proc->stack = (uint64_t)kmalloc();
    ASSERT(proc->stack != 0);

    kmemset((void*)proc->stack, 0, PAGE_SIZE);   
    stack_top = proc->stack + STACK_SIZE;

    proc->context = stack_top - sizeof(TrapFrame) - 7 * 8;   
    *(uint64_t*)(proc->context + 6 * 8) = (uint64_t)TrapReturn;

    proc->tf = (TrapFrame*)(stack_top - sizeof(TrapFrame)); 
    proc->tf->cs = 0x10 | 3;
    proc->tf->rip = 0x400000; // base address (virtual memory) of program of user space
    proc->tf->ss = 0x18 | 3;
    proc->tf->rsp = 0x400000 + PAGE_SIZE; // end of address (virtual memory) of program of user space
    proc->tf->rflags = 0x202;
    
    proc->page_map = setup_kvm();
    ASSERT(proc->page_map != 0);
    ASSERT(setup_uvm(proc->page_map, P2V(addr), 5120));
    proc->state = READY;    
}

static ProcessControl* get_pc(void)
{
    return &pc;
}

void init_process(void)
{
    ProcessControl *process_control;
    Process *process;
    HeadList *list;
    uint64_t addr[3] = {0x20000, 0x30000, 0x40000};

    process_control = get_pc();
    list = &process_control->ready_list;
    for (int i = 0; i < 3; i++)
    {
        process = find_unused_process();
        set_process_entry(process, addr[i]);
        append_list_tail(list, (List*)process);
    }
}

void launch(void)
{
    ProcessControl *process_control;
    Process *process;

    process_control = get_pc();
    process = (Process*)remove_list_head(&process_control->ready_list);
    process->state = RUNNING;
    process_control->current_process = process;
    set_tss(process);
    switch_vm(process->page_map);
    pstart(process->tf);
    
}

// context switch between processes
static void switch_process(Process *prev, Process *current)
{
    set_tss(current);
    switch_vm(current->page_map);
    swap(&prev->context, current->context);
}

// swap


static void schedule(void)
{
    Process *prev_proc;
    Process *current_proc;
    ProcessControl *process_control;
    HeadList *list;

    process_control = get_pc();
    prev_proc = process_control->current_process;
    list = &process_control->ready_list;
    ASSERT(!is_list_empty(list));
    
    current_proc = (Process*)remove_list_head(list);
    current_proc->state = RUNNING;   
    process_control->current_process = current_proc;

    switch_process(prev_proc, current_proc);   
}

void yield(void)
{
    ProcessControl *process_control;
    Process *process;
    HeadList *list;
    
    process_control = get_pc();
    list = &process_control->ready_list;

    if (is_list_empty(list))
    {
        return;
    }

    process = process_control->current_process;
    process->state = READY;
    append_list_tail(list, (List*)process);
    schedule();
}

void sleep(int32_t wait)
{
    ProcessControl *process_control;
    Process *process;
    
    process_control = get_pc();
    process = process_control->current_process;
    process->state = SLEEP;
    process->wait = wait;

    append_list_tail(&process_control->wait_list, (List*)process);
    schedule();
}

void wake_up(int32_t wait)
{
    ProcessControl *process_control;
    Process *process;
    HeadList *ready_list;
    HeadList *wait_list;

    process_control = get_pc();
    ready_list = &process_control->ready_list;
    wait_list = &process_control->wait_list;
    process = (Process*)remove_list(wait_list, wait);
    while (process != NULL)
    {   
        process->state = READY;
        append_list_tail(ready_list, (List*)process);
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

    list = &process_control->kill_list;
    // add killed process to tail of kill_list
    append_list_tail(list, (List*)process);

    // other process could be in sleep state
    // so it should wake up the process of pid = 1
    // 1 means pid = 1
    wake_up(1);
    schedule();
}

void wait(void)
{
    ProcessControl *process_control;
    Process *process;
    HeadList *list;

    process_control = get_pc();
    list = &process_control->kill_list;

    while (true)
    {
        if (!is_list_empty(list))
        {
            process = (Process*)remove_list_head(list); 
            ASSERT(process->state == KILLED);

            kfree(process->stack);
            free_vm(process->page_map);            
            kmemset(process, 0, sizeof(Process));   
        }
        else
        {
            // 1 means process of pid = 1
            sleep(1);
        }
    }
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
