#include "idt.h"

static IdtPtr idt_pointer;
static IdtEntry vectors[256];

static void init_idt_entry(IdtEntry *entry, uint64_t addr, uint8_t attribute)
{
    entry->low = (uint16_t)addr;
    entry->selector = 8;  // 0000 0000 0000 1000 = 0x8 (which is offset of code segment descriptor in GDT)
    entry->attr = attribute;
    entry->mid = (uint16_t)(addr>>16);
    entry->high = (uint32_t)(addr>>32);
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

void handler(TrapFrame *tf)
{
    uint8_t isr_value;

    switch (tf->trapno) 
    {
        case 32:
            eoi();
            break;
            
        case 39:
            isr_value = read_isr();
            if ((isr_value&(1<<7)) != 0) 
            {
                eoi();
            }
            break;

        default:
            while (1) { }
    }
}

void trap(TrapFrame *tf) {
    // Simulate display update
    volatile uint8_t *video_memory = (uint8_t *)0xb8010;
    *video_memory += 1;        // Increment character at address 0xb8010
    *(video_memory + 1) = 0xe; // Set color attribute

    // Call the external handler
    handler(tf);
}

__attribute__ ((interrupt)) void vector0(TrapFrame *tf) {
    tf->trapno = 0;
    tf->errorcode = 0;
    trap(tf);
}
__attribute__ ((interrupt)) void vector1(TrapFrame *tf) {
    tf->trapno = 1;
    tf->errorcode = 0;
    trap(tf);
}
__attribute__ ((interrupt)) void vector2(TrapFrame *tf) {
    tf->trapno = 2;
    tf->errorcode = 0;
    trap(tf);
}
__attribute__ ((interrupt)) void vector3(TrapFrame *tf) {
    tf->trapno = 3;
    tf->errorcode = 0;
    trap(tf);
}
__attribute__ ((interrupt)) void vector4(TrapFrame *tf) {
    tf->trapno = 4;
    tf->errorcode = 0;
    trap(tf);
}
__attribute__ ((interrupt)) void vector5(TrapFrame *tf) {
    tf->trapno = 5;
    tf->errorcode = 0;
    trap(tf);
}
__attribute__ ((interrupt)) void vector6(TrapFrame *tf) {
    tf->trapno = 6;
    tf->errorcode = 0;
    trap(tf);
}
__attribute__ ((interrupt)) void vector7(TrapFrame *tf) {
    tf->trapno = 7;
    tf->errorcode = 0;
    trap(tf);
}


// errorcode from system
__attribute__ ((interrupt)) void vector8(TrapFrame *tf, uint64_t errorcode) {
     tf->trapno = 8;
     tf->errorcode = errorcode;
    trap(tf);
}
__attribute__ ((interrupt)) void vector10(TrapFrame *tf, uint64_t errorcode) {
     tf->trapno = 10;
     tf->errorcode = errorcode;
    trap(tf);
}
__attribute__ ((interrupt)) void vector11(TrapFrame *tf, uint64_t errorcode) {
     tf->trapno = 11;
     tf->errorcode = errorcode;
    trap(tf);
}
__attribute__ ((interrupt)) void vector12(TrapFrame *tf, uint64_t errorcode) {
     tf->trapno = 12;
     tf->errorcode = errorcode;
    trap(tf);
}
__attribute__ ((interrupt)) void vector13(TrapFrame *tf, uint64_t errorcode) {
     tf->trapno = 13;
     tf->errorcode = errorcode;
    trap(tf);
}
__attribute__ ((interrupt)) void vector14(TrapFrame *tf, uint64_t errorcode) {
     tf->trapno = 14;
     tf->errorcode = errorcode;
    trap(tf);
}
__attribute__ ((interrupt)) void vector17(TrapFrame *tf, uint64_t errorcode) {
     tf->trapno = 17;
     tf->errorcode = errorcode;
    trap(tf);
}

__attribute__ ((interrupt)) void vector16(TrapFrame *tf) {
    tf->trapno = 16;
    tf->errorcode = 0;
    trap(tf);
}
__attribute__ ((interrupt)) void vector18(TrapFrame *tf) {
    tf->trapno = 18;
    tf->errorcode = 0;
    trap(tf);
}
__attribute__ ((interrupt)) void vector19(TrapFrame *tf) {
    tf->trapno = 19;
    tf->errorcode = 0;
    trap(tf);
}
// timer interrupt vector
__attribute__ ((interrupt)) void vector32(TrapFrame *tf) {
    tf->trapno = 32;
    tf->errorcode = 0;
    trap(tf);
}
__attribute__ ((interrupt)) void vector39(TrapFrame *tf) {
    tf->trapno = 39;
    tf->errorcode = 0;
    trap(tf);
}
// vector9 is reserved
// vector15 is reserved
// vector20 is reserved
// vector31 is reserved
