#include "idt.h"
#include "process.h"
#include "syscall.h"
#include "print.h"

static IdtPtr idt_pointer;
static IdtEntry vectors[256];
static uint64_t ticks;

static void init_idt_entry(IdtEntry *entry, uint64_t addr, uint8_t attribute)
{
    entry->low = (uint16_t)addr;
    entry->selector = 8;  // 0000 0000 0000 1000 = 0x8 (which is offset of code segment descriptor in GDT)
    entry->attr = attribute;
    entry->mid = (uint16_t)(addr>>16);
    entry->high = (uint32_t)(addr>>32);
}

static void timer_handler(void)
{
    ticks++;
    wake_up(-1);
}

void init_idt(void)
{
    init_idt_entry(&vectors[0],(uint64_t)vector0,INT_GATE_FLAG);
    init_idt_entry(&vectors[1],(uint64_t)vector1,INT_GATE_FLAG);
    init_idt_entry(&vectors[2],(uint64_t)vector2,INT_GATE_FLAG);
    init_idt_entry(&vectors[3],(uint64_t)vector3,INT_GATE_FLAG);
    init_idt_entry(&vectors[4],(uint64_t)vector4,INT_GATE_FLAG);
    init_idt_entry(&vectors[5],(uint64_t)vector5,INT_GATE_FLAG);
    init_idt_entry(&vectors[6],(uint64_t)vector6,INT_GATE_FLAG);
    init_idt_entry(&vectors[7],(uint64_t)vector7,INT_GATE_FLAG);
    init_idt_entry(&vectors[8],(uint64_t)vector8,INT_GATE_FLAG);
    init_idt_entry(&vectors[10],(uint64_t)vector10,INT_GATE_FLAG);
    init_idt_entry(&vectors[11],(uint64_t)vector11,INT_GATE_FLAG);
    init_idt_entry(&vectors[12],(uint64_t)vector12,INT_GATE_FLAG);
    init_idt_entry(&vectors[13],(uint64_t)vector13,INT_GATE_FLAG);
    init_idt_entry(&vectors[14],(uint64_t)vector14,INT_GATE_FLAG);
    init_idt_entry(&vectors[16],(uint64_t)vector16,INT_GATE_FLAG);
    init_idt_entry(&vectors[17],(uint64_t)vector17,INT_GATE_FLAG);
    init_idt_entry(&vectors[18],(uint64_t)vector18,INT_GATE_FLAG);
    init_idt_entry(&vectors[19],(uint64_t)vector19,INT_GATE_FLAG);
    init_idt_entry(&vectors[32],(uint64_t)vector32,INT_GATE_FLAG);
    init_idt_entry(&vectors[39],(uint64_t)vector39,INT_GATE_FLAG);
    init_idt_entry(&vectors[0x80],(uint64_t)sysint,0xee);

    idt_pointer.limit = sizeof(vectors)-1;
    idt_pointer.addr = (uint64_t)vectors;
    load_idt(&idt_pointer);
}

void eoi(void)
{
    __asm__ __volatile__ (
        ".intel_syntax noprefix\n"  // intel syntax
        "mov al, 0x20\n"
        "out 0x20, al\n" 
        ".att_syntax prefix\n"
        : 
        : 
        : "al"
    );
}
void load_idt(IdtPtr *ptr)
{
    __asm__ __volatile__ ("lidt (%0)" : : "r"(ptr));
}
uint8_t read_isr(void)
{
    uint8_t al;

    __asm__ __volatile__ (
        ".intel_syntax noprefix\n"  // intel syntax
        "mov al, 11\n"
        "out 0x20, al\n"
        "in al, 0x20\n"
        ".att_syntax prefix\n" // switch to  AT&T syntax
        : "=a" (al)
        : 
    );

    return al;
}

void load_cr3(uint64_t map)
{
    __asm__ __volatile__ (
        "movq %0, %%rax\n\t"  // set rax = map
        "movq %%rax, %%cr3\n\t" // set cr3 = rax
        :
        : "r"(map)
        : "%rax"
    );
}

uint64_t read_cr2(void)
{
    uint64_t cr2;
    __asm__ __volatile__ (
        "movq %%cr2, %0\n\t"
        : "=r"(cr2)
    );
    return cr2;
}

uint64_t get_ticks(void)
{
    return ticks;
}

__attribute__((naked)) void swap(uint64_t *prev, uint64_t next)
{
        __asm__ __volatile__ (
        "pushq %rbx\n\t"
        "pushq %rbp\n\t"
        "pushq %r12\n\t"
        "pushq %r13\n\t"
        "pushq %r14\n\t"
        "pushq %r15\n\t"
        "movq %rsp, (%rdi)\n\t"
        "movq %rsi, %rsp\n\t"
        "popq %r15\n \t"
        "popq %r14\n\t"
        "popq %r13\n\t"
        "popq %r12\n\t"
        "popq %rbp\n\t"
        "popq %rbx\n\t"
        "ret\n\t"
    );
}
  __attribute__((naked)) void pstart(TrapFrame *tf)
{
    __asm__ __volatile__ (
        "movq %rdi, %rsp\n\t"
        "jmp TrapReturn\n\t"
    );
}

void handler(TrapFrame *tf)
{
    uint8_t isr_value;

    switch (tf->trapno) 
    {
        case 32:
            timer_handler();
            eoi();
            break;
            
        case 39:
            isr_value = read_isr();
            if ((isr_value&(1<<7)) != 0) 
            {
                eoi();
            }
            break;
        
        case 0x80:
            system_call(tf);
            break;

        default:
            // check if trap or interrupt happened in kernel mode or user mode
            // ring0 = 000 (kernel mode)
            // ring3 = 011 (user mode)
            // user mode
            if (tf->cs & 3)
            {
                printk("Exception is %d in user mode\n", tf->trapno);
                exit();
            }
            // kernel mode
            else
            {
                printk("Exception is %d in kernel mode\n", tf->trapno);
                while(1) {}
            }
    }

    if (tf->trapno == 32)
    {   
        yield();
    }
}

//Divide by 0
__attribute__ ((interrupt)) void vector0(TrapFrame *tf) {
    tf->trapno = 0;
    tf->errorcode = 0;
    handler(tf);
}
//Reserved
__attribute__ ((interrupt)) void vector1(TrapFrame *tf) {
    tf->trapno = 1;
    tf->errorcode = 0;
    handler(tf);
}
//NMI Interrupt
__attribute__ ((interrupt)) void vector2(TrapFrame *tf) {
    tf->trapno = 2;
    tf->errorcode = 0;
    handler(tf);
}
//Breakpoint (INT3)
__attribute__ ((interrupt)) void vector3(TrapFrame *tf) {
    tf->trapno = 3;
    tf->errorcode = 0;
    handler(tf);
}
//Overflow (INTO)
__attribute__ ((interrupt)) void vector4(TrapFrame *tf) {
    tf->trapno = 4;
    tf->errorcode = 0;
    handler(tf);
}
//Bounds range exceeded (BOUND)
__attribute__ ((interrupt)) void vector5(TrapFrame *tf) {
    tf->trapno = 5;
    tf->errorcode = 0;
    handler(tf);
}
//Invalid opcode (UD2)
__attribute__ ((interrupt)) void vector6(TrapFrame *tf) {
    tf->trapno = 6;
    tf->errorcode = 0;
    handler(tf);
}
//Device not available (WAIT/FWAIT)
__attribute__ ((interrupt)) void vector7(TrapFrame *tf) {
    tf->trapno = 7;
    tf->errorcode = 0;
    handler(tf);
}


// errorcode from system
// Double fault
__attribute__ ((interrupt)) void vector8(TrapFrame *tf, uint64_t errorcode) {
     tf->trapno = 8;
     tf->errorcode = errorcode;
    handler(tf);
}
//Invalid TSS
__attribute__ ((interrupt)) void vector10(TrapFrame *tf, uint64_t errorcode) {
     tf->trapno = 10;
     tf->errorcode = errorcode;
    handler(tf);
}
//Segment not present
__attribute__ ((interrupt)) void vector11(TrapFrame *tf, uint64_t errorcode) {
     tf->trapno = 11;
     tf->errorcode = errorcode;
    handler(tf);
}
//Stack-segment fault
__attribute__ ((interrupt)) void vector12(TrapFrame *tf, uint64_t errorcode) {
     tf->trapno = 12;
     tf->errorcode = errorcode;
    handler(tf);
}

//General protection fault
__attribute__ ((interrupt)) void vector13(TrapFrame *tf, uint64_t errorcode) {
     tf->trapno = 13;
     tf->errorcode = errorcode;
    handler(tf);
}

//Page fault
__attribute__ ((interrupt)) void vector14(TrapFrame *tf, uint64_t errorcode) {
     tf->trapno = 14;
     tf->errorcode = errorcode;
    handler(tf);
}
//Alignment check
__attribute__ ((interrupt)) void vector17(TrapFrame *tf, uint64_t errorcode) {
     tf->trapno = 17;
     tf->errorcode = errorcode;
    handler(tf);
}
//x87 FPU error
__attribute__ ((interrupt)) void vector16(TrapFrame *tf) {
    tf->trapno = 16;
    tf->errorcode = 0;
    handler(tf);
}
//Machine check
__attribute__ ((interrupt)) void vector18(TrapFrame *tf) {
    tf->trapno = 18;
    tf->errorcode = 0;
    handler(tf);
}
//SIMD Floating-Point Exception
__attribute__ ((interrupt)) void vector19(TrapFrame *tf) {
    tf->trapno = 19;
    tf->errorcode = 0;
    handler(tf);
}
// timer interrupt vector
__attribute__ ((interrupt)) void vector32(TrapFrame *tf) {
    tf->trapno = 32;
    tf->errorcode = 0;
    handler(tf);
}
__attribute__ ((interrupt)) void vector39(TrapFrame *tf) {
    tf->trapno = 39;
    tf->errorcode = 0;
    handler(tf);
}

// vector9 is reserved
// vector15 is reserved
// vector20 is reserved
// vector31 is reserved
