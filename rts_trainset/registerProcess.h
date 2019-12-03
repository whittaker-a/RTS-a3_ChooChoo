/*
 * registerProcess.h
 *
 *  Created on: Nov 3, 2019
 *      Author: AWhit
 */

#ifndef REGISTERPROCESS_H_
#define REGISTERPROCESS_H_

/*
 *  Process has max assignable ID of 100
 *  Each process can write data through UART0,
 *  that data is written to line number matching their id
 */
#define MAX_PID     100

/* The default ID given to the idle waiting process at priority 0 */
#define IDLE_PID 0

#define IDLE_PRIORITY 0

//create typedef of function pointer void void
typedef void (*func_name)();

//int reg_proc(void (*func_name)(), unsigned short pid, unsigned short priority);
int reg_proc(func_name func_ptr, unsigned long pid, unsigned long priority);

#endif /* REGISTERPROCESS_H_ */
