/*
 * testProcess.c
 *
 *
 * These functions are the process which run synchronously
 * they are used for testing the other software
 * Each process is a C function of type void
 *
 *
 *  Created on: Nov 4, 2019
 *      Author: AWhit
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "UART0.h"
#include "Kernel_Calls.h"
#include "ServiceCalls.h"
#include "testProcess.h"


//this function writes the char 'X' to the console
void writeXtoConsole(){
    char* temp = "Process X: %i";
    char * out = malloc(sizeof(char) * 13);
    unsigned int i;
    bind(2);
    for(i=0;i<100;i++){
        sprintf(out, temp, i);
        while(send(1,2,out,13)<0);
    }
    bind(2);
}


void writeYtoConsole(){
    char* temp = "Process Y: %i";
    char * out = malloc(sizeof(char) * 13);
    unsigned int i;
    bind(4);

    for(i=0;i<100;i++){
        sprintf(out, temp, i);
        while(send(1,4,out,13)<0);
    }

}


void writeZtoConsole(){
    char* temp = "Process Z: %i";
    char * out = malloc(sizeof(char) * 13);
    unsigned int i;
    bind(3);
    for(i=0;i<10;i++){
        sprintf(out, temp, i);
        while(send(1,3,out,13)<0);
    }
    unbind(3);
}


void prioritySwitchTo2(){
    int i = 0;
    while(i<1000){
        i++;
    }
    nice(2);

    //just stay here
    while(1);
}

void writeAtoConsole(){
    char *out = "A\n";
    int i;
    for(i=0;i<100;i++){
//        writeStringToConsole("\n");
        writeStringToConsole(out);
    }
}
void writeBtoConsole(){
    char *out = "B\n";
    char *priority = "change from 3 to 4\n";
    int i;
    for(i=0;i<50;i++){
//        writeStringToConsole("\n");
        writeStringToConsole(out);
    }
    nice(2);
    writeStringToConsole(priority);

    for(i=0;i<50;i++){
//        writeStringToConsole("\n");
        writeStringToConsole(out);
    }
}
void writeCtoConsole(){
    char *out = "C\n";
    int i;
    for(i=0;i<100;i++){
//        writeStringToConsole("\n");
        writeStringToConsole(out);
    }
}


void spamDisplay1(){
    char *out = "A";
    int i;
    for(i=0;i<10;i++){
        writeStringToConsole("\n");
        writeStringToConsole(out);
    }
}
void spamDisplay2(){
    char *out = "B";
    int i;
    for(i=0;i<10;i++){
        writeStringToConsole("\n");
        writeStringToConsole(out);
    }
    nice(4);
    for(i=0;i<10;i++){
        writeStringToConsole("\n");
        writeStringToConsole(out);
    }
}

void spamDisplay3(){
    int i;
    char *out = "C";
    bind(2);
    for(i=0;i<10;i++){
        writeStringToConsole("\n");
        writeStringToConsole(out);
    }
//    recv(2,4, out, 1);
//
//    while(1){
//        writeStringToConsole(out);
//    }
}

void spamDisplayX(){
    char *out = "1";
    while(1){
        writeStringToConsole(out);
    }
}

void spamDisplay_nice(){
    int i;
    char *out = "_";
    for(i=0;i<10000;i++){
        writeStringToConsole(out);
    }
    nice(1);
    while(1){
        writeStringToConsole(out);
    }
}

void spamDisplayX_DIE(){
    unsigned int i;
    char *out = "X";
    for(i=0;i<100;i++){
        writeStringToConsole(out);
    }
}

void t41(){
    bind(2);
    char * bound = "\nBound\n";
    char * level = "Level 2\n";
    writeStringToConsole(bound);
    nice(2);
    writeStringToConsole(level);
    unbind(2);
}

void t42(){
    bind(3);
    char * letter = "Letter Sent\n";
    send(2,3,letter,13);
    writeStringToConsole(letter);

}

void t43(){
    char * fail = "Fail\n";
    char * out = malloc(sizeof(char) * 12);
    if(bind(2) == 1){
        recv(ANY, ANY,out, 12 );
        writeStringToConsole(out);
    }
    else{
        writeStringToConsole(fail);
    }

}

void cascadeNice(){
    nice(4);
    nice(3);
    nice(2);
    nice(1);
    while(1);
}

void testBind(){

    bind(2);
    char temp = 'B';
    while (1){
        writeStringToConsole(&temp);
    }

}

int has_sent = 0;

void receiveAndWrite(){

    bind(3);
    char *msg = malloc(12 * sizeof(char));

    while(1){
        recv(3, 1, msg, 12);
        writeStringToConsole(msg);
    }
}

void sendAndWait(){

    bind(1);
    char* msg = malloc(6 * sizeof(char));
    strcpy(msg, "HELLO");

    short sent = 0;

    while(sent <=0){
        sent = send(3, 1, msg, 6);
    }
//    has_sent = 1;

    nice(1);
    while(1);
}

void demo_nice1(){
    bind(2);
    int i, j;
    char * temp = "n1=level %i";
    char * out = malloc(sizeof(char)*16);
    for(j=0;j<3;j++){
        for(i=4;i>0;i--){
            nice(i);
            sprintf(out, temp, i);
            send(1, 2, out, 12);
        }
    }
}

void demo_nice2(){
    bind(3);
    int i,j;
    char * temp = "n2=level %i";
    char * out = malloc(sizeof(char)*16);
    for(j=0;j<3;j++){
        for(i=4;i>0;i--){
            nice(i);
            sprintf(out, temp, i);
            send(1, 3, out, 12);
        }
    }
}

void vt_100_proc(){

    struct CUP cup = {ESC, '[', '0', '0', ';', '0', '0', 'H', NUL};

    struct CUP g1 = {ESC, ')', NULL};
//    g1.esc = ESC;
//    g1.sqrbrkt = '(';
    writeStringToConsole((char *)&g1);

    cup.line[0]=0;
    cup.line[1]=4;
    cup.col[0] = 3;
    cup.col[1] = 3;
    cup.cmdchar = 'm';



    writeStringToConsole((char*)&cup);

    cup . line [0] = '1';
    cup . line [1] = '9';
    cup . col [0] = '0';
    cup . col [1] = '5';
    cup.cmdchar = 'H';
    /*
     - To test on screen - change ESC to 'E'
     - If removed when using a VT-100 emulator, cursor will be reposition
    */
//    cup . esc = 'E';

//    printf("\u001b[31mHelloWorld");
//    printf("%s\n", (char *)&cup);
    writeStringToConsole((char*)&cup);
    writeStringToConsole("Hello");

}
void testProcWrite(){
    bind(5);
    char * msg = "Great";
    send(1, 5, msg, 5);
}

void writePID(){
    unsigned long pid;
    pid = get_pid();

    char * out = malloc(sizeof(char)*2);
    bind(4);
    sprintf(out, "%i", pid);
    send(1, 4, out, 2);
//    writeStringToConsole(out);
}

/*
 * Receives from any, writes contents to screen
 */

void communications(){
    bind(1);
    line_cursor line = {ESC, '[', '0', '1', ';', '0', '0', 'H', NUL};
    char * out = malloc(sizeof(char) * MAX_LETTER_SIZE);
    int cur_line = 1;
    while(1){
        if(recv(ANY, ANY, out, MAX_LETTER_SIZE ) > 0){
//            writeStringToConsole("\n");
            writeStringToConsole((char *)&line);
            writeStringToConsole(out);
            cur_line++;
            line.line[1] = '0' + cur_line%10;
            line.line[0] = '0' + cur_line/10;
        }
    }
}

