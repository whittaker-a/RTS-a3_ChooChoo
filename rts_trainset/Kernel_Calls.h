/*
 * K_Space.h
 *
 *  Created on: Nov 5, 2019
 *      Author: AWhit
 */

#ifndef KERNEL_CALLS_H_
#define KERNEL_CALLS_H_

void terminate_self();
void nice(unsigned short priority);

unsigned int bind(unsigned short mQueue_id);
unsigned short unbind(unsigned short mQueue_id);

short recv(unsigned long to, unsigned long frm, char * msg, unsigned short sz);
short send(unsigned long to, unsigned long frm, char * msg, unsigned short sz);

short get_pid();

#define MAX_LETTER_SIZE         16
#define NUMBER_OF_MAILBOXES     8


/*
 * A letter contains an address, a return address,
 * a message, and the size of that message
 *
 * A letter is stored in a mailbox
 * A mailbox is a linked list queue of letters
 * A mailbox exists in the mailbox list
 *
 */
typedef struct letter{
    /*  Address of Mailbox  */
    unsigned long ADDR;
    /*  Address of Return Mailbox   */
    unsigned long RTRN;
    /*  Message Being Sent  */
    char *msg;
    /*  Size of Message  */
    unsigned short msg_sz;
    /* Next Letter in Mailbox Queue */
    struct letter *next;
    struct letter *prev;

}LETTER;

/*
 * A mailbox contains a pointer to a letter, an unsigned ID, and
 * an unsigned bound indicating if a process has bound to this mailbox
 * mailbox is free if bound = 0
 * mailbox is bound and bound = process id
 */
typedef struct mailbox{

    LETTER * letter;
    unsigned short ID;
    unsigned short Bound;

//    /*  If a process owns multiple mailboxes they are seen as a linked list */
//    mailbox * next;

}MAILBOX;


#endif /* KERNEL_CALLS_H_ */
