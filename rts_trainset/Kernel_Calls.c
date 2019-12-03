/*
 * These functions occupy entry points to 'Kernel Space'
 * Used to manage the Many running processes
 * These functions allow a process to:
 *      Bind, unbind, Send a message, receive a message
 *      Terminate, nice, get their unique process id
 *
 *  A Mailbox List holds a finite number of Mailboxes
 *  A Mailbox can hold an unlimited number of letters
 *  A letter contains an address, return address, message size,
 *  and message
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ServiceCalls.h"
#include "Process.h"
#include "priorityQueue.h"
#include "Kernel_Calls.h"


/*
 * R0 is used to pass the first arg to this function
 * R0 holds volatile unsigned long data
 * move to R7 from R0
 */
void set_Register7(volatile unsigned long data){
    __asm("     mov     r7,r0");
}

/*
 * Returns process ID
 * kernel gets id request and gets ID from process PCB
 */
short get_pid(){
    volatile struct kcallargs kcall;

    /*  Set kernel command  */
    kcall.code = GETID;
    /*  Store command in Register 7 */
    set_Register7((unsigned long) &kcall);

    SVC();
    /*  Return Process ID   */
    return kcall.rtnvalue;
}


/*
 * Running process is to be terminated
 * A non running process cannot reach end of process lifetime
 *
 */
void terminate_self(){

    volatile struct kcallargs kcall;

    //kill command
    kcall.code = TERMINATE;
    //Process for kernel to kill
    kcall.arg1 = (unsigned int) running;

    //set R7 to a pointer to kcall
    //tells SVCHandler what to do
    set_Register7((unsigned long) &kcall);

    //alive
    SVC();
    //dead
}

/*
 * Trap to Kernel to allow process to switch priority
 */
void nice(unsigned short priority){

    volatile struct kcallargs kcall;
    if(priority_IsValid(priority) && priority!=running->priority){
        //allowed to make K Call
        kcall.code = NICE;
        kcall.arg1 = priority;
        kcall.arg2 = (unsigned int) running;

        //set R7 to a pointer to kcall
        //tells SVCHandler what to do
        set_Register7((unsigned long) &kcall);

        SVC();
    }
}


/*
 * A process becomes associated with a mailbox after calling bind
 * an ID of 0 signifies the process will take any mailbox number
 *
 * return value:    1   Successful Bind
 *                 -1   Queue in Use
 *                 -2   No Queues Available
 *                 -3   Invalid Queue Number
 */
unsigned int bind(unsigned short mQueue_id){

    volatile struct kcallargs kcall;
    /*  The purpose is to BIND  */
    kcall.code = BIND;
    /*  Initialize kcall pass/fail code */
    kcall.rtnvalue = 0;
    /*  Pass the arguments to the kernel    */
    kcall.arg1 = mQueue_id;
    kcall.arg2 = kcall.arg2 = (unsigned int) running->PID;

    //set R7 to a pointer to kcall
    //tells SVCHandler what to do
    set_Register7((unsigned long) &kcall);

    SVC();

    /*
     * returns to this function after the kernel call
     * needs to report back to the process calling bind
     */

    return kcall.rtnvalue;
}

/*
 * A process can unbind itself from a message Queue
 *
 * return value:    1   Successful unbind
 *                 -1   Process was not bound to Queue
 */
unsigned short unbind(unsigned short mQueue_id){
    volatile struct kcallargs kcall;

    /*  The purpose is to BIND  */
    kcall.code = UNBIND;
    /*  Initialize kcall pass/fail code */
    kcall.rtnvalue = 0;
    /*  Pass the arguments to the kernel  */
    kcall.arg1 = mQueue_id;
    kcall.arg2 = kcall.arg2 = (unsigned int) running->PID;

    //set R7 to a pointer to kcall
    //tells SVCHandler what to do
    set_Register7((unsigned long) &kcall);

    SVC();

    /*
     * returns to this function after the kernel call
     * needs to report back to the process calling bind
     */
    return kcall.rtnvalue;
}

/*
 * Process requests to send a message to another process
 * Validates process level permissions: message size is valid
 * Creates a struct called letter and passes the address to the kernel
 * args:    to      destination mailbox
 *          frm     sending mailbox
 *          *msg    the message to be sent
 *          sz      The size of the message
 *
 * returns: sz      If successful
 *          -1      Destination does not exist
 *          -2      Message Queue is Full
 *          -3      Size is invalid
 *           0      Failure for other any other reason
 */
short send(unsigned long to, unsigned long frm, char * msg, unsigned short sz){

    volatile struct kcallargs kcall;

    static short first_send = TRUE;
    //This function always reserves space for a letter
    static LETTER * temp;
    /*
     * only need to allocate memory for temp letter once
     * temp stores the message being sent from a process
     * to the kernel
     */
    if(first_send){
        temp = malloc(sizeof(LETTER));
        first_send = FALSE;
    }

    // Validate message size
    if(sz > 0 && sz < MAX_LETTER_SIZE){

        //the letter contains the message from the sender
        temp->ADDR = to;
        temp->RTRN = frm;
        temp->msg = msg;
        temp->msg_sz = sz;

        kcall.code = SEND;
        kcall.arg1 = (unsigned int)temp;
        kcall.arg2 = (unsigned int)running->PID;

        //set R7 to a pointer to kcall
        //tells SVCHandler what to do
        set_Register7((unsigned long) &kcall);

        SVC();
        /*
         * Return here after message sent
         */
        return kcall.rtnvalue;
    }

    return SZ_INVALID;
}

/*
 * A process checks if they have an item in their mailbox
 * if not, they are blocked until the message arrives
 * sz is the maximum number of bits the process will accept
 *
 * args:        to      owned mailbox
 *              frm     sending mailbox
 *             *msg     the message to be returned
 *              sz      The size of the message
 *
 * Returns:     sz      if received
 *              -1      process not bound to Queue
 *              -2      size is invalid
 */
short recv(unsigned long to, unsigned long frm, char * msg, unsigned short sz){

    volatile struct kcallargs kcall;

    kcall.code = RECV;
    kcall.arg1 = to;
    kcall.arg2 = frm;

    set_Register7((unsigned long) &kcall);

    SVC();


    /*  Validate Caller mailbox is valid    */
    if(kcall.rtnvalue == NO_MAILBOX){
        return NO_MAILBOX;
    }

    /*  Validate message size   */
    if(((LETTER *)kcall.arg1)->msg_sz > sz){
        return RECV_SIZE_INV;
    }

    /*  arg1 of kcall now stores the address of the returned letter */

    /*  Copy Contents of the received letter into the process message */
//    memcpy(msg, ((LETTER *)kcall.arg1)->msg, ((LETTER *)kcall.arg1)->msg_sz);
    strcpy(msg, ((LETTER *)kcall.arg1)->msg);
    /*  return from */
    return ((LETTER *)kcall.arg1)->RTRN;
}



