/**
 *
 * This Software allows for multiple processes, at given priority levels,
 * to operate pseudo-simultaneously.
 * Each process must be of type void with no return value.
 * These processes can:
 *      Request their process ID
 *      Change priority Levels
 *      terminate
 *
 * and this software supports inter-process communication where
 * each process can request a mailbox from the kernel, once
 * a mailbox is allocated to the process, the process can
 *      Send Messages
 *      Receive Messages
 *      relieve control over their mailbox
 *
 * Author: Alexander Whittaker
 * Fall 2019
 */

#include <stdio.h>
#include <stdlib.h>

#include "UART0.h"
#include "sysTick.h"

#include "Process.h"
#include "registerProcess.h"
#include "testProcess.h"
#include "priorityQueue.h"
#include "ServiceCalls.h"

//Initializes the UART interrupts and SysTick
void main (void){


    /* initializes each priority level  */
    initPriorityQueue();
    /*  Initializes Mailbox Messaging Structure   */
    initMailBoxList();

    /*  Context Switching   */
    func_name writeA = &writeAtoConsole;
    reg_proc(writeA, 9, 2);
    func_name writeB = &writeBtoConsole;
    reg_proc(writeB, 10, 3);
    func_name writeC = &writeCtoConsole;
    reg_proc(writeC,11, 3);

//    func_name writeX = &spamDisplay1;
//    reg_proc(writeX, 70, 3);
//
//    func_name writey = &spamDisplay2;
//    reg_proc(writey, 72, 3);
//
//    func_name writeZ = &spamDisplay3;
//    reg_proc(writeZ, 72, 3);

    /*  Nice    */
//    func_name writen1 = &demo_nice1;
//    reg_proc(writen1, 66, 3);
//    func_name writen2 = &demo_nice2;
//    reg_proc(writen2, 65, 3);

//    func_name writeX = &demo_nice1;
//    reg_proc(writeX, 70, 3);
//    func_name writeZ = &spamDisplayX;
//    reg_proc(writeZ, 71, 3);

//    func_name testPID = &writePID;
//    reg_proc(testPID, 3, 3);


//    func_name test3 = &t43;
//    reg_proc(test3,2,1);
//    func_name test2 = &t42;
//    reg_proc(test2,3,3);
//    func_name test1 = &t41;
//    reg_proc(test1,4,4);

//    func_name testZ = &spamDisplay3;
//    reg_proc(testZ,7,2);

    func_name communication = &communications;
    reg_proc(communication, 1, 4);

//    func_name proc = &testProcWrite;
//    reg_proc(proc, 12, 2);

    UART0_Init();                                // Initialize UART0
    InterruptEnable(INT_VEC_UART0);              // Enable UART0 interrupts
    UART0_IntEnable(UART_INT_RX | UART_INT_TX);  // Enable Receive and Transmit interrupts

    /* Sets the period at which the System Clock Increments*/
    SysTickPeriod(MAX_WAIT);

    /*  Initializes PendSV For Context Switching between processes  */
    init_PendSVPriority();

    /*  Traps to the kernel to instance first process   */
    SVC();

    /*  Never Reaches This point   */

}






