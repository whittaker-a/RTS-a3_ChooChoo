/*
 * svc.h
 *
 *  Created on: Nov 4, 2019
 *      Author: AWhit
 */

#ifndef SERVICECALLS_H_
#define SERVICECALLS_H_

enum kernelcallcodes {GETID, NICE, TERMINATE,SEND, RECV, BIND, UNBIND};
enum niceRtrnCodes {FAIL, SUCCEED};

#define NVIC_INT_CTRL_R     (*((volatile unsigned long *) 0xE000ED04))
#define TRIGGER_PENDSV      0x10000000
#define NVIC_SYS_PRI3_R (*((volatile unsigned long *) 0xE000ED20))
#define PENDSV_LOWEST_PRIORITY 0x00E00000

#define BOUND           1   //Successful Bind
#define QUEUE_IN_USE   -1   //Queue in Use
#define NO_FREE_QUEUE  -2   //No Queue Available
#define INVALID_QUEUE  -3   //Invalid Queue Number

#define REQUEST_ERROR   0
#define DST_NOT_EXIST  -1
#define DST_FULL       -2
#define SZ_INVALID     -3

#define ANY             0
#define NO_MAILBOX     -1
#define RECV_SIZE_INV  -2

#define PASS            1
#define ERROR          -1

enum letterState {NOT_IN_MAILBOX, IN_MAILBOX};


volatile void init_PendSVPriority();
void initMailBoxList();
void SVCall(void);
void PendSV_Handler();


struct kcallargs
{
    unsigned int code;
    int rtnvalue;   /*  signed for error values */
    unsigned int arg1;
    unsigned int arg2;
};

#endif /*SERVICECALLS_H_*/
