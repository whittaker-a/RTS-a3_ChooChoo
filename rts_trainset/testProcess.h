/*
 * testProcess.h
 *
 *  Created on: Nov 4, 2019
 *      Author: AWhit
 */

#ifndef TESTPROCESS_H_
#define TESTPROCESS_H_

void writeXtoConsole();
void writeYtoConsoleAndDie();
void prioritySwitchTo2();
void writeZtoConsole();
void writeYtoConsole();

void cascadeNice();
void testBind();
void sendAndWait();
void receiveAndWrite();

void vt_100_proc();
void blink();

void demo_nice2();
void demo_nice1();

void testProcWrite();
void communications();
void testWriteto3();
void testWriteto2();

void writePID();

/*  T1  */
void spamDisplay1();
void spamDisplay2();
void spamDisplay3();

/*  T2  */
void spamDisplay_nice();
void spamDisplayX();

/*  T3  */
void spamDisplayX_DIE();

/*  T4  */
void t43();
void t42();
void t41();

void writeAtoConsole();
void writeBtoConsole();
void writeCtoConsole();

typedef struct cursor
{
    char esc;
    char sqrbrkt;
    char line[2];   /* 01 through 24 */
    char semicolon;
    char col[2];    /* 01 through 80 */
    char cmdchar;
    char nul;

}LINE;


#endif /* TESTPROCESS_H_ */
