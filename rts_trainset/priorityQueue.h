/*
 * priorityQueue.h
 *
 *  Created on: Nov 3, 2019
 *      Author: AWhit
 */

#ifndef PRIORITYQUEUE_H_
#define PRIORITYQUEUE_H_

#include "Process.h"

//define maximum priority a process can obtain
#define MAX_PRIORITY    4
#define MIN_PRIORITY    0
#define PRIORITY_QUEUE_SIZE MAX_PRIORITY+1

/*  Function Declarations   */
void setRunning();
void initPriorityQueue();
short addProcess(PCB *PCB_ptr);
void terminate_self();
void removeProcess(PCB *PCB_ptr);
void nextProcess();
void startNewRunning();
short priority_IsValid(unsigned short priority);
void newPriorityQueue(PCB * PCB_ptr);
void block(PCB * PCB_ptr);
void unblock(PCB * PCB_ptr);

char UART_INPUT;

#endif /* PRIORITYQUEUE_H_ */
