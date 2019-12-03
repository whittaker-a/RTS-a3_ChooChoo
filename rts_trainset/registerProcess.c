


#include <stdio.h>
#include <stdlib.h>


#include "Kernel_Calls.h"
#include "Process.h"
#include "registerProcess.h"
#include "priorityQueue.h"
/*
 * Allocate memory for a process stack and
 * process control block.
 * Initialize a process to"thumb" mode
 *
 * Takes a pointer to a function, process id, and priority
 */
int reg_proc(func_name func_ptr, unsigned long pid, unsigned long priority){

    /*
     * The first process to be registered is idle waiting
     * Idle waiting has priority 0, registered when
     * priority Queue's are initialized
     * No other process can have priority 0
     */
    static int firstRegistration = TRUE;
    if(firstRegistration && priority == IDLE_PRIORITY){
        firstRegistration = FALSE;
    }
    else if(firstRegistration && priority != IDLE_PRIORITY){
        //first registration, priority not 0
        return FALSE;
    }
    else if(!firstRegistration && (priority == IDLE_PRIORITY || priority > MAX_PRIORITY)){
        //invalid priority for registered process
        return FALSE;
    }
    if(pid >= MAX_PID || (pid == IDLE_PID) && (priority != IDLE_PRIORITY)){
        /*  invalid process ID  */
        return FALSE;
    }

    PCB *temp = (PCB*)malloc(sizeof(PCB));

    //assign pcb an ID and initial Priority
    temp->PID = pid;
    temp->priority = priority;

    /*  If process calls recv, message copied to PCB  */
    temp->message = (LETTER *)malloc(sizeof(LETTER));

    /*  Reserve data for message contents   */
    temp->message->msg = (char *)malloc(sizeof(char)*MAX_LETTER_SIZE );

    /*  When outputting Data to UART0, Process writes to own line    */
    temp->UART0_line = pid;

    //declare a stack and allocate memory
    unsigned long *stack =  ( unsigned long *)malloc(STACKSIZE* sizeof(unsigned long));

    //stack grows from high to low
    //Initial stack pointer points to top of memory
    temp->sp = (unsigned long)(stack+sizeof(unsigned long)*(STACKSIZE-1));

    //get base of stack -> used to free stack
    temp->base_sp = (unsigned long)(stack);

    //needs the registers initial value
    struct stack_frame initialRegister;

    //set the process to thumb mode
    initialRegister.psr = THUMB_MODE;

    //initialize program counter
    initialRegister.pc = (unsigned long)func_ptr;

    //set LR to address of terminate process routine
    void (*terminate)(PCB *) = &terminate_self;
    initialRegister.lr = (unsigned long)terminate;

    //create a pointer to initialized registers
    unsigned long* registers = (unsigned long *) &initialRegister;
    unsigned short length = STACKFRAMESIZE;

    unsigned short i;
    //push initialized values of register onto process stack
    for( i = length; i > 0 ; i-- ){
        //sp stores the memory address of a location in the process stack
        *((unsigned long*)(temp->sp)) = registers[i];
        //point to the next stack element
        temp->sp = temp->sp - sizeof(long);
    }

    //add the PCB to required priority queue
    addProcess(temp);

    return TRUE;
}




