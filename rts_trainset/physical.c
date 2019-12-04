/*
 * physical.c
 *
 *  The purpose of this code is to send packets to or
 *  Created on: Dec 3, 2019
 *      Author: AWhit
 */

#include "dataLink.h"

#define STX 0x02
#define ETX 0x03
#define DLE 0x10

/*
 * This function receives a packet to be transferred
 * and prepares a frame
 *
 * a frame consists of a start bit: STX
 * and an end bit: ETX
 *
 */
void outFrame(PACKET data){
    int chksum = 0;
    unsigned short len = data.length;
    int i;

    /*  Send STX    */


    for(i=0 ;i<len ;i++){

    }


}
