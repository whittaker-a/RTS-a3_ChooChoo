/*
 - SysTick sample code
 - Originally written for the Stellaris (2013)
 - Will need to use debugger to "see" interrupts
 - Code uses bit-operations to access SysTick bits
 *
 * sysTick.h
 *
 * This header file is used to define
 * required registers locations in memory
 * and to set required bits in enabling / disable
 * and checking data
 *
 * Edited by: Alexander Whittaker
 *
 * Updated to enable processes switching via a PendSV call
 * Used to support pseudo-simultaneous process execution
 *   Switching Occurs at every instance of a systic interrupt
 *   Maximum wait time reduced to increase context switching occurrence rate
 * Edits by: Alexander Whittaker
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_

// SysTick Registers
// SysTick Control and Status Register (STCTRL)
#define ST_CTRL_R   (*((volatile unsigned long *)0xE000E010))
// Systick Reload Value Register (STRELOAD)
#define ST_RELOAD_R (*((volatile unsigned long *)0xE000E014))

// SysTick defines
#define ST_CTRL_COUNT      0x00010000  // Count Flag for STCTRL
#define ST_CTRL_CLK_SRC    0x00000004  // Clock Source for STCTRL
#define ST_CTRL_INTEN      0x00000002  // Interrupt Enable for STCTRL
#define ST_CTRL_ENABLE     0x00000001  // Enable for STCTRL

#define TRUE  1
#define FALSE 0

// Maximum period
#define MAX_WAIT           16000

/*   Function Declarations   */
void SysTickPeriod(unsigned long Period);
void SysTickIntEnable();
void SysTickStart();
void IntMasterEnable();
void SysTick_init();


#endif /* SYSTICK_H_ */
