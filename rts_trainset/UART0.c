/*
 *
 * UART test code:
 * Echo's characters back that are inputted.
 *
 * Author: Emad Khan, ECED4402 TA
 * Summer 2017
 *
 * The software that controls the UART ISR
 *
 * Provided and tweaked to add
 * Queues,
 * Error Messages,
 * And Output Functionality
 *
 * Updated for optimization towards specific software by
 * Alexander Whittaker
 *
 */


/* Globals */
volatile char Data;     /* Input data from UART receive */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "UART0.h"
#include "Process.h"

extern PCB *running;

/*
 * This function queue's a string to the output buffer
 * Then Enqueue's a new line character and
 * forces the first character
 * The rest happens during the UART interrupt
 */
void writeCharToConsole(char data){
    /*  Checks if UART FIFO Full    */
    while(UART0_FR_R & UART_FR_TXFF);
    UART0_DR_R = data;
}

void writeStringToConsole(char *index){
    while(*index){
        writeCharToConsole(*index);
        index++;
    }
}



void UART0_Init(void)
{
    volatile int wait;

    /* Initialize UART0 */
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCUART_GPIOA;   // Enable Clock Gating for UART0
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCGPIO_UART0;   // Enable Clock Gating for PORTA
    wait = 0; // give time for the clocks to activate

    UART0_CTL_R &= ~UART_CTL_UARTEN;        // Disable the UART
    wait = 0;   // wait required before accessing the UART config regs

    // Setup the BAUD rate
    UART0_IBRD_R = 8;   // IBRD = int(16,000,000 / (16 * 115,200)) = 8.680555555555556
    UART0_FBRD_R = 44;  // FBRD = int(.680555555555556 * 64 + 0.5) = 44.05555555555556

    UART0_LCRH_R = (UART_LCRH_WLEN_8);  // WLEN: 8, no parity, one stop bit, without FIFOs)

    GPIO_PORTA_AFSEL_R = 0x3;        // Enable Receive and Transmit on PA1-0
    GPIO_PORTA_PCTL_R = (0x01) | ((0x01) << 4);         // Enable UART RX/TX pins on PA1-0
    GPIO_PORTA_DEN_R = EN_DIG_PA0 | EN_DIG_PA1;        // Enable Digital I/O on PA1-0

    UART0_CTL_R = UART_CTL_UARTEN;        // Enable the UART
    wait = 0; // wait; give UART time to enable itself.
}

void InterruptEnable(unsigned long InterruptIndex)
{
/* Indicate to CPU which device is to interrupt */
if(InterruptIndex < 32)
    NVIC_EN0_R = 1 << InterruptIndex;       // Enable the interrupt in the EN0 Register
else
    NVIC_EN1_R = 1 << (InterruptIndex - 32);    // Enable the interrupt in the EN1 Register
}

void UART0_IntEnable(unsigned long flags)
{
    /* Set specified bits for interrupt */
    UART0_IM_R |= flags;
}

void InterruptMasterDisable(){
    /* disable CPU interrupts */
    __asm(" cpsid   i");
}

void InterruptMasterEnable(void)
{
    /* enable CPU interrupts */
    __asm(" cpsie   i");
}

extern short UART_INPUT;

void UART0_IntHandler(void)
{
/*
 * Simplified UART ISR - handles receive and xmit interrupts
 * Application signaled when data received
 */
    if (UART0_MIS_R & UART_INT_RX)
    {
        /* RECV done - clear interrupt and make char available to application */
        UART0_ICR_R |= UART_INT_RX;
        UART_INPUT = UART0_DR_R;
    }

    if (UART0_MIS_R & UART_INT_TX)
    {
        /* XMIT done - clear interrupt */
        UART0_ICR_R |= UART_INT_TX;
    }
}



