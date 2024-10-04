#ifndef IDT_H
#define IDT_H


#define TRAP_GATE_FLAG  0x8f
#define INT_GATE_FLAG   0x8e

#include "stdint.h"

typedef struct IdtEntry {
    uint16_t low;
    uint16_t selector;
    uint8_t res0;
    uint8_t attr;
    uint16_t mid;
    uint32_t high;
    uint32_t res1;
} IdtEntry;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t addr;
} IdtPtr;


typedef struct TrapFrame {
    int64_t r15;
    int64_t r14;
    int64_t r13;
    int64_t r12;
    int64_t r11;
    int64_t r10;
    int64_t r9;
    int64_t r8;
    int64_t rbp;
    int64_t rdi;
    int64_t rsi;
    int64_t rdx;
    int64_t rcx;
    int64_t rbx;
    int64_t rax;
    int64_t trapno;
    int64_t errorcode;
    int64_t rip;
    int64_t cs;
    int64_t rflags;
    int64_t rsp;
    int64_t ss;
} TrapFrame;

//gcc -mgeneral-regs-only
__attribute__((interrupt)) void vector0(TrapFrame *tf);
__attribute__((interrupt)) void vector1(TrapFrame *tf);
__attribute__((interrupt)) void vector2(TrapFrame *tf);
__attribute__((interrupt)) void vector3(TrapFrame *tf);
__attribute__((interrupt)) void vector4(TrapFrame *tf);
__attribute__((interrupt)) void vector5(TrapFrame *tf);
__attribute__((interrupt)) void vector6(TrapFrame *tf);
__attribute__((interrupt)) void vector7(TrapFrame *tf);

// void vector8(void);
__attribute__ ((interrupt)) void vector8(TrapFrame *tf, uint64_t errorcode);
__attribute__ ((interrupt)) void vector10(TrapFrame *tf, uint64_t errorcode);
__attribute__ ((interrupt)) void vector11(TrapFrame *tf, uint64_t errorcode);
__attribute__ ((interrupt)) void vector12(TrapFrame *tf, uint64_t errorcode);
__attribute__ ((interrupt)) void vector13(TrapFrame *tf, uint64_t errorcode);
__attribute__ ((interrupt)) void vector14(TrapFrame *tf, uint64_t errorcode);
__attribute__ ((interrupt)) void vector17(TrapFrame *tf, uint64_t errorcode);

__attribute__((interrupt)) void vector16(TrapFrame *tf);
__attribute__((interrupt)) void vector18(TrapFrame *tf);
__attribute__((interrupt)) void vector19(TrapFrame *tf);
__attribute__((interrupt)) void vector32(TrapFrame *tf);
__attribute__((interrupt)) void vector39(TrapFrame *tf);

void init_idt(void);
void eoi(void);
void load_idt(IdtPtr *ptr);
uint8_t read_isr(void);
void load_cr3(uint64_t map);

#endif