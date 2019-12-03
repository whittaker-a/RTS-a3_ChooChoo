/*
 *
 *
 *
 *
 *
 */
#ifndef PROCESS_H_
#define PROCESS_H_


#include "Kernel_Calls.h"


#define TRUE    1
#define FALSE   0
#define PRIVATE static


#define SVC()   __asm(" SVC #0")
#define disable()   __asm(" cpsid i")
#define enable()    __asm(" cpsie i")

#define MSP_RETURN 0xFFFFFFF9    //LR value: exception return using MSP as SP
#define PSP_RETURN 0xFFFFFFFD    //LR value: exception return using PSP as SP


/*  Function Declarations   */
void set_LR(volatile unsigned long);
unsigned long get_PSP();
void set_PSP(volatile unsigned long);
unsigned long get_MSP(void);
void set_MSP(volatile unsigned long);
unsigned long get_SP();

void volatile save_registers();
void volatile restore_registers();


#define STACKSIZE        512
#define STACKFRAMESIZE   15

#define THUMB_MODE  0x01000000;



/* Cortex default stack frame
 *
 * Ordered as they are to be pushed onto the stack
 */
struct stack_frame{
    /* Registers saved by s/w (explicit) */
    /* There is no actual need to reserve space for R4-R11, other than
     * for initialization purposes.  Note that r0 is the h/w top-of-stack.
     */
    unsigned long r4;
    unsigned long r5;
    unsigned long r6;
    unsigned long r7;
    unsigned long r8;
    unsigned long r9;
    unsigned long r10;
    unsigned long r11;

    /* Stacked by hardware (implicit)*/
    unsigned long r0;
    unsigned long r1;
    unsigned long r2;
    unsigned long r3;
    unsigned long r12;
    unsigned long lr;   // -> LINK REGISTER
    unsigned long pc;   // -> PROGRAM COUNTER
    unsigned long psr;  // -> PROGRAM STATUS REGISTER
};



/* Process control block */
typedef struct pcb{

    /* Stack pointer - r13 (PSP) */
    unsigned long sp;
    //points to base of stack always
    unsigned long base_sp;
    /* Links to adjacent PCBs */
    struct pcb *next;
    struct pcb *prev;

    // The process unique id
    unsigned long PID;
    // The process priority
    unsigned short priority;
    //the entry way for the process to begin

    /*  A pointer to message received from kernel when requested   */
    LETTER * message;

    /*  Each Process writes data to its own line  */
    unsigned int UART0_line;

}PCB;

extern PCB *running;

#endif /*PROCESS_H_*/
