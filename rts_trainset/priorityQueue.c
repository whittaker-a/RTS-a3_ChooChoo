/*
 * priorityQueue.c
 *
 *  The purpose of this code is to create and manage the priority
 *  of each process. These functions can add and remove a process
 *  from a priority level or switch a process priority level
 *
 *  The Idle process, which reserves priority level zero (0), is defined here
 *  as it is mandatory for the software.
 *
 *  Created on: Nov 3, 2019
 *      Author: AWhit
 */

#include <stdio.h>
#include <stdlib.h>


#include "ServiceCalls.h"
#include "priorityQueue.h"
#include "process.h"
#include "registerProcess.h"
#include "UART0.h"

extern PCB *running;

//declare an array of PCB pointers to manage process priority
//Priority range from 0 to MAX PRIORITY
static PCB *priorityQueue[PRIORITY_QUEUE_SIZE];
//static PCB *priorityQueue[PRIORITY_QUEUE_SIZE] = {0,0,0,0,0};

/*  Entry way to the Queue of blocked processes */
PCB *blockedQueue;

//a process exists at this priority and
//no process has a higher priority
static unsigned short currentPriority;

void wait(){
    unsigned int i = 0;
    while(i<500000)
        i++;
}


/*
 * default process
 * lowest priority
 * writes 'writing' to UART and otherwise does nothing
 * prevents system from failing when all other processes terminate
 */
void idleWaiting(){
    UART_INPUT = 0;
    char * waiting = "Waiting";
    char * demo = "Woken up";
    char * dot = ".";
    char clear[5] = {ESC, '[', '2', 'K', NUL};
    short i;
    line_cursor idle = {ESC, '[', '0', '0', ';', '0', '0', 'H', NUL};

    bind(5);

    while(1){
        while(UART_INPUT == 0){

            writeStringToConsole((char *)&idle);
            wait();
            writeStringToConsole(waiting);
            wait();
            for(i=0;i<3;i++){
                writeStringToConsole(dot);
                wait();
            }
            writeStringToConsole((char *)clear );
        }

        UART_INPUT = 0;

        send(1,5,demo, 9);
    }
}


/*
 * The blocked queue variable is only an entry way into
 * the blocked queue is initialized to point to NULL
 * This function allocates space for the queue entry way
 */
void initBlockedQueue(){
    blockedQueue = malloc(sizeof(PCB));
    blockedQueue->PID = 0;
    blockedQueue->next = NULL;
}


/*
 * Initializes the Priority Queue
 * Priorities range from zero to the defined maximum priority.
 * Each element in the queue is a
 */
void initPriorityQueue(){
    //allocate memory for priority Queue
    //one element per priority

    unsigned short i;
    for(i = 0; i < PRIORITY_QUEUE_SIZE; i++){
        priorityQueue[i] = malloc(sizeof(PCB));
        priorityQueue[i]->PID = i;
        priorityQueue[i]->priority=i;
        priorityQueue[i]->next = 0;
        priorityQueue[i]->prev = 0;
    }

    func_name idle = &idleWaiting;
    reg_proc(idle,IDLE_PID,0);

    //allocate blocked Queue
    initBlockedQueue();
}

/*
 * Validates process priority
 * Cannot be 0
 * cannot be above maximum priority
 */
short priority_IsValid(unsigned short priority){
    return (priority && priority <= MAX_PRIORITY) ? TRUE : FALSE;
}

//takes a pointer to a PCB, gets the priority from it and
//adds it to the END of priorityQueue at given priority
//priorityQueue is an array of doubly linked list entry points
short addProcess(PCB *PCB_ptr){
    /*  Store current priority  */
    unsigned short priority = currentPriority;

    //idle process has unique priority
    if((PCB_ptr->PID == IDLE_PID) && (PCB_ptr->priority != MIN_PRIORITY)){
        //invalid process ID or Priority
        return FALSE;
    }
    else if((PCB_ptr->PID != IDLE_PID) && (!priority_IsValid(PCB_ptr->priority))){
        //Not the idle process, invalid priority
        return FALSE;
    }

    if(priorityQueue[PCB_ptr->priority]->next){
        //Some element exists in the priorityQueue at this priority

        //set added PCB_ptr->next to first element of Queue
        PCB_ptr->next = priorityQueue[PCB_ptr->priority]->next;

        //set added PCB_ptr->prev to old last element of Queue
        PCB_ptr->prev = priorityQueue[PCB_ptr->priority]->next->prev;

        //Set old end of Queue to point next to new end of queue PCB
        priorityQueue[PCB_ptr->priority]->next->prev->next = PCB_ptr;

        //Set old Front of Queue to point prev to new End of queue
        priorityQueue[PCB_ptr->priority]->next->prev = PCB_ptr;
    }
    else{
        //priority level currently empty
        //Creates a new entry point
        priorityQueue[PCB_ptr->priority]->next = PCB_ptr;

        //added PCB only element in queue, points to self
        PCB_ptr->next = PCB_ptr;
        PCB_ptr->prev = PCB_ptr;

        //if added process priority the new highest priority, update currentPriority
        currentPriority = (currentPriority > PCB_ptr->priority) ? currentPriority : PCB_ptr->priority;

        /*  priority changed  */
        if(currentPriority > priority){
            running = priorityQueue[currentPriority]->next;
        }
    }

    return TRUE;
}

void setRunning(){
    running = priorityQueue[currentPriority]->next;
}

/*
 * This process sets the PSP and
 * pulls registers for a newly running process
 *
 */
void pull_Process(){
    //restore PSP from PCB
    set_PSP(running->sp);

    //restore Registers
    restore_registers();
}

/*
 * pushes the contents of a process onto the process tack
 * and stores the process stack pointer
 */
void push_Process(){
    //Save Running Reg's on Stack
    save_registers();

    //Save PSP in PCB
    running->sp=get_PSP();
}

/*
 * This function switches the active running process
 * The two processes :
 * (one running -> waiting to run )
 * (one waiting to run -> running )
 * if the running process only element in array
 * then that process keeps priority and does not change state
 */
void nextProcess(){
    //check next running process
    if(running->next == running){
        //running process is the only process in Queue
        return;
    }
    //There is another process on the Queue

    //Saves the PSP in the PCB and Pushes Registers to Process Stack
    push_Process();

    //get NEXT from PCB
    running = running->next;

    //Gets PSP from PCB and Pulls Registers from Process Stack
    pull_Process();
}


/*
 * Searches for a process at the next lowest priority
 * Performs a process switch
 * called when process removed from priority queue
 */
void decrementPriority(){

    //Null if Queue at priority = currentPriority is empty
    while(!(priorityQueue[currentPriority]->next)){
        currentPriority--;
    }

    //set the next running process
    running = priorityQueue[currentPriority]->next;

}

/*
 * The formerly running process is no longer running
 * and not due to a process switch within a priority queue
 *
 * Either terminated or changed priority
 */
//void startNewRunning(){
//    pull_Process();
//}

/*
 * Removes a process from priority queue
 */
void removeProcess(PCB *PCB_ptr){
    //do not want an interrupt to occur while managing Queue
    disable();

    if(PCB_ptr->next == PCB_ptr){
        //only item in Queue
        priorityQueue[currentPriority]->next=NUL;
        //find new process at lower priority
        decrementPriority();
    }
    else{
        //another Process in Queue

        //Link the adjacent PCB's
        PCB_ptr->next->prev = PCB_ptr->prev;
        PCB_ptr->prev->next = PCB_ptr->next;

        //is this PCB the entry point to the Queue?
        if(priorityQueue[currentPriority]->next == PCB_ptr){
            //create new entry point to Queue
            priorityQueue[currentPriority]->next = PCB_ptr->next;
        }

        running = priorityQueue[currentPriority]->next;

    }

    //Queue has been managed, enable interrupts
    enable();
}

/*  Kernel Function
 *
 * This function Blocks a running process
 * removes process from priority queue, adds to blocked queue
 *
 */
void block(PCB * PCB_ptr){

    PCB * temp = blockedQueue;

    removeProcess(PCB_ptr);

    /*  Go to end of Queue  */
    while(temp->next){
        temp=temp->next;
    }

    /*  Temp is a pointer to the last element in the blocked Queue  */
    temp->next = PCB_ptr;

    /*  PCB at end of blocked Queue points to Null  */
    PCB_ptr->next = NULL;

    /* And points back to the previous blocked PCB    */
    PCB_ptr->prev = temp;

}

/*
 * Removes a process from the Blocked Queue and adds it to
 * the appropriate priority Queue
 */
void unblock(PCB *PCB_ptr){

    /*  PCB_ptr is in the Blocked Queue */
    PCB_ptr->prev->next = PCB_ptr->next;

    if(!(PCB_ptr->next==NULL)){
        PCB_ptr->next->prev = PCB_ptr->prev;
    }

    /*  PCB_ptr no longer in Blocked Queue  */
    addProcess(PCB_ptr);


}

/*
 * Takes a function and moves it to a new priority Queue
 * if the priority goes down, removeProcess catches it
 * if the priority goes up, addProcess catches it
 */
void newPriorityQueue(PCB * PCB_ptr){

    removeProcess(PCB_ptr);
    addProcess(PCB_ptr);

}


