/*
 * Kernel code: Supervisor call (SVC) handler example
 * ECED 4402
 *
 * 17 Oct 2019 - Fix to SVCall comments
 * 04 Oct 2017 - Comments in SVCHandler()'s "second call"
 * 11 Mar 2013 - Handle passing argument to kernel by register
 * 05 Mar 2013 - Fix first SVC call to access PSP correctly
 *             - Updated comments
 * 20 Feb 2012 - First version
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ServiceCalls.h"
#include "process.h"
#include "sysTick.h"
#include "priorityQueue.h"
#include "Kernel_Calls.h"
#include "UART0.h"

PCB *running;
extern PCB *blockedQueue;

/*
 *   Declare a list of Mailboxes that only kernel communicates with
 *   A mailbox is a LETTER struct that points to NULL if empty
 *   A mailbox ADDR field stores the process ID of the owner
 *
 */
static MAILBOX mailbox_list[NUMBER_OF_MAILBOXES];

volatile void init_PendSVPriority(){
    NVIC_SYS_PRI3_R |= PENDSV_LOWEST_PRIORITY;
}

/*
 * Taken from Assignment 2 hand-out
 *
 * Performs a context switch
 * !! CAN BE INTERRUPTED !!
 */
void PendSV_Handler()
{
    disable();
    /* Save running process */
//    save_registers(); /* Save active CPU registers in PCB */
    nextProcess();
//    restore_registers(); /* Restore process’s registers */

    enable();
}

/*
 * Copies the contents of one letter to another,
 * Does not save the next / prev pointers as
 * both letters are not in the same Queue
 */
void copyLetter(LETTER * dst, LETTER * src){
    dst->ADDR = src->ADDR;
    dst->RTRN = src->RTRN;
    dst->msg_sz = src->msg_sz;

    /*  free msg in dst and allocate new memory, insures correct size   */
    dst->msg = (char *)realloc(dst->msg, sizeof(char) * src->msg_sz);
    /*  Copy Contents of message    */
    strcpy(dst->msg,src->msg);
}


/*
 *  Removes a letter from a mailbox and free's the memory
 *  the letter stores its own address / mailbox ID
 */
void removeLetter(LETTER *letter,  enum letterState letter_location){

    if(letter_location == IN_MAILBOX){
        letter->prev->next = letter->next;

        if(!(letter->next == NULL)){
            letter->next->prev = letter->prev;
        }
    }

    free(letter->msg);
    free(letter);
}

/*
 * Allocates memory for the Mailbox List
 * and initializes values for each element
 * initialized with empty contents and no currently bound process
 *
 * Each mailbox has an incremental ID starting from 1
 *
 * Requesting mailbox 0 requests any mailbox
 */
void initMailBoxList(){

    unsigned short i;

    for(i=0;i<NUMBER_OF_MAILBOXES;i++){
        /*  Initialize letter pointer   */
        mailbox_list[i].letter = malloc(sizeof(LETTER));
        mailbox_list[i].letter->next = NUL;
        mailbox_list[i].letter->ADDR = NUL;
        mailbox_list[i].letter->RTRN = NUL;
        mailbox_list[i].letter->msg_sz = 0;
        /*  Mailbox is not Bound    */
        mailbox_list[i].Bound = 0;
        /*  Mailbox ID starts from 1    */
        mailbox_list[i].ID = i+1;
    }
}
/*
 *  Checks if process is waiting on this letter
 *  This letter can have a specific sender or any sender
 *  Return  TRUE    if process blocked and waiting for this letter
 *          FALSE   if process not waiting for this letter
 */
short is_LetterReceiver(PCB * process, LETTER * msg){

   return ((mailbox_list[msg->ADDR-1].Bound == process->PID) &&
           ((msg->RTRN == process->message->RTRN) ||(process->message->RTRN == ANY)))
           ? TRUE : FALSE;
}


/*
 * Kernel Function to Query Blocked Queue
 * Checks if owner of just received letter is blocked, waiting on that letter
 * if found, return process to priority Queue
 * Iterates through the blocked Queue of blocked processes
 *
 */
unsigned short checkBlockedProcesses(LETTER * letter){

    /*
     * BlockedQueue is an entry-way to the list of blocked queues
     */
    PCB * blocked_iterator = blockedQueue->next;

    while(blocked_iterator){
        if(is_LetterReceiver(blocked_iterator, letter)){
            /*  Blocked Process is waiting on msg   */
            copyLetter(blocked_iterator->message, letter);
            removeLetter(letter, NOT_IN_MAILBOX);
            unblock(blocked_iterator);
            return TRUE;
        }
        else{
            blocked_iterator = blocked_iterator->next;
        }
    }

    /*  No block processes found waiting on this letter */
    return FALSE;
}

/*
 * Checks if mailbox id a valid mailbox
 */
unsigned short mailbox_isValid(unsigned short id){
    return (id!=0 && id<=NUMBER_OF_MAILBOXES) ? TRUE : FALSE;
}



/*
 * checks if mailbox with id MID is owned by
 * the process with id PID
 */
unsigned short is_MailboxOwner(unsigned int MID, unsigned int PID){
    return (mailbox_list[MID-1].Bound == PID) ? TRUE : FALSE;
}

/*
 * The functionality for attempting to bind a mailbox to a process
 */
void k_bind(struct kcallargs *kcaptr){
    // set the iterator to top of mailbox list
    unsigned int i = NUMBER_OF_MAILBOXES - 1;

    if(kcaptr->arg1){
        /*  process requests bind to specific mailbox   */
        if(mailbox_isValid(kcaptr->arg1)){
            if(mailbox_list[kcaptr->arg1-1].Bound){
                kcaptr->rtnvalue = QUEUE_IN_USE;
            }
            else{
                //set bound to the binding process ID
                mailbox_list[kcaptr->arg1-1].Bound=kcaptr->arg2;
                kcaptr->arg1 = kcaptr->arg1-1;
                kcaptr->rtnvalue = BOUND;
            }
        }
        else{
            kcaptr->rtnvalue = INVALID_QUEUE;
        }
    }
    else{
        /*  process is attempting to bind any mailbox   */
        /*  Loop until an unbound mailbox is found  */
        while(i && !mailbox_list[i].Bound){
            i--;
        }

        //if i not zero then a free mailbox was found
        if(i){
            mailbox_list[i].Bound = kcaptr->arg2;
            kcaptr->arg1 = i;
            kcaptr->rtnvalue = BOUND;
        }
        else{
            kcaptr->rtnvalue = NO_FREE_QUEUE;
        }
    }
}


/*
 * A process unbinds from a mailbox
 * all letters in mailbox are deleted
 */
void k_unbind(struct kcallargs *kcaptr){
    /*  Is the requested Mailbox Owned by requesting process    */
    if(is_MailboxOwner(kcaptr->arg1, kcaptr->arg2)){
//        LETTER * iterator = malloc(sizeof(LETTER));
        LETTER * iterator;
        LETTER * temp;
        /*  iterator equals the first letter in message Queue   */
        iterator = mailbox_list[kcaptr->arg1-1].letter->next;

        /*  While there are messages in the mailbox, free */
        while(iterator){
            //store the value in iterator
            temp = iterator;

            //increment the iterator along the linked lists
            if(iterator != iterator->next){
                iterator = iterator->next;
            }
            else{
                iterator = NULL;
            }

            //delete the letter
            free(temp->msg);
            free(temp);
        }
        kcaptr->rtnvalue = PASS;
        mailbox_list[kcaptr->arg1-1].Bound = 0;
        mailbox_list[kcaptr->arg1-1].letter->next= NULL;
    }
    else{
        kcaptr->rtnvalue = ERROR;
    }
}


/*
 * Validates the Sender, copies the contents of the letter into the mailbox.
 * The value, kcaptr, stores the address of the letter in arg1
 * the lettevoiv    vfrr44r contains destination, sender, the Message, and the Message size.
 *
 */
void k_send(struct kcallargs *kcaptr){
    /*
     * Check if destination is valid Mailbox number
     * mailbox ID's are start at 1 and are stored in an array
     */
    LETTER * new_letter;
//    LETTER * iterator;
//    short i;

    /*  Check if Sender Owns Return Address */
    if(is_MailboxOwner(((LETTER *)kcaptr->arg1)->RTRN, kcaptr->arg2)){
        /*  Is the ADDR Valid   */
        if(((LETTER *)kcaptr->arg1)->ADDR-1 < NUMBER_OF_MAILBOXES){
            /*  Check if Mailbox Has an Owner  */
            if(mailbox_list[((LETTER *)kcaptr->arg1)->ADDR-1].Bound){
                /*  All things Valid    */
                /*  Allocate Memory for the new Letter  */
                new_letter = malloc(sizeof(LETTER));
                new_letter->msg = (char *)malloc(sizeof(char) * new_letter->msg_sz);

                /*
                 * Copy letter sent by running into new_letter
                 * and add new_letter to the appropriate message Queue
                 * or send to the appropriate Process and unblock
                 */
                copyLetter(new_letter,((LETTER *)kcaptr->arg1));

                if(checkBlockedProcesses(new_letter)){
                    /*  process was blocked waiting on this letter */
                    kcaptr->rtnvalue = PASS;
                }
                else{
                    if(mailbox_list[((LETTER *)kcaptr->arg1)->ADDR-1].letter->next){
                        /*  Mailbox currently has Letter in Queue */
                        new_letter->next = mailbox_list[((LETTER *)kcaptr->arg1)->ADDR-1].letter->next;
                        new_letter->prev = mailbox_list[((LETTER *)kcaptr->arg1)->ADDR-1].letter->next->prev;
                        mailbox_list[((LETTER *)kcaptr->arg1)->ADDR-1].letter->next = new_letter;
                    }else{
                        /*  Mailbox Empty   */
                        mailbox_list[((LETTER *)kcaptr->arg1)->ADDR-1].letter->next = new_letter;
//                        new_letter->next = NULL;
                        new_letter->prev = new_letter;
                        new_letter->next = new_letter;
                    }
                    kcaptr->rtnvalue = PASS;
                }
            }
            else{
                kcaptr->rtnvalue = DST_NOT_EXIST;
            }
        }
        else{
            kcaptr->rtnvalue = DST_NOT_EXIST;
        }
    }
    else{
        /*  Sender Does not Own RTRN Mailbox    */
        kcaptr->rtnvalue = REQUEST_ERROR;
    }
}

/*
 * mailbox_list is an entry-way to the message queue for that mailbox
 * and therefore points to the first letter in that mailbox
 *
 * if rtrn_MID is zero, return the first found letter
 */
LETTER * searchLetters(unsigned short my_mID, unsigned short rtrn_mID){

    LETTER * msg = mailbox_list[my_mID-1].letter->next;

    /*
     * Iterate msg over mailbox of ID my_mID
     * msg is a letter and iterates over each letter in the mailbox
     * until the RTRN address matches the desired sender
     * desired sender could be any sender
     */
    while((msg->RTRN != rtrn_mID) && (msg != NULL)){
        if(rtrn_mID == ANY){
            /*  Returning any letter from mailbox   */
            return msg;
        }
        else{
            msg=msg->next;
        }
    }
    /*  either msg points to the desired letter or it points to NULL    */
    return msg;
}

/*
 * Kernel validates process request
 * if valid request, and mailbox empty, block
 * if valid and data, load data
 * if invalid, return error message
 */
void k_recv(struct kcallargs *kcaptr){

    unsigned short mailbox_total = 0;
    short i = 0;
    /*  Store the ADDR and RTRN of the recv process request in case of block and for improved readability */
    running->message->ADDR = kcaptr->arg1;
    running->message->RTRN = kcaptr->arg2;

    if(running->message->ADDR == ANY){
        /* search all mailboxes for    letter*/
        kcaptr->arg1 == NULL;
        while((i < NUMBER_OF_MAILBOXES) && kcaptr->arg1 == NULL){
            /* insure mailbox owned by calling process */
            if(mailbox_list[i].Bound == running->PID){

                /*  Count number of owned mailboxes */
                mailbox_total++;

                /*
                 * Search the letters on the mailbox, store returned letter in kcaptr
                 * if No Letter found, returns NULL
                 */
                kcaptr->arg1 = (unsigned long)searchLetters(i+1,  running->message->RTRN);
            }
            i++;
        }
        if(!mailbox_total){
            /*  process bound to zero mailboxes */
            kcaptr->rtnvalue = NO_MAILBOX;
        }
    }
    else if(mailbox_list[running->message->ADDR - 1].Bound == running->PID){
        /*
         *  Calling Process owns Mailbox
         *  Search all the letters in mailbox with requested ID
         *  if a letter is found, store pointer to letter in arg1
         */
        kcaptr->arg1 = (unsigned long) searchLetters(running->message->ADDR, running->message->RTRN);
    }
    else{
        /*  Calling Process Does not Own Requested Mailbox    */
        kcaptr->rtnvalue = NO_MAILBOX;

        /*  Reset message in process block, invalid contents  */
        running->message = NULL;
    }

    if(kcaptr->arg1 == NULL){
        /*  Block the Process if no Letter found   */
        block(running);
    }
    else{
        /*
         * Letter Found, return pointer to message in arg1. Process otherwise
         * cannot access message as it is stored in PCB
         * Letter is to be removed from mailbox and free'd
         */
        copyLetter(running->message, (LETTER *)kcaptr->arg1);
        removeLetter((LETTER *)kcaptr->arg1, IN_MAILBOX);

        /*  Store a pointer to the process message  */
        kcaptr->arg1 = (unsigned int)running->message;
    }
}



void SVCall(void)
{
    /* Supervisor call (trap) entry point
     * Using MSP - trapping process either MSP or PSP (specified in LR)
     * Source is specified in LR: F1 (MSP) or FD (PSP)
     * Save r4-r11 on trapping process stack (MSP or PSP)
     * Restore r4-r11 from trapping process stack to CPU
     * SVCHandler is called with r0 equal to MSP or PSP to access any arguments
     */

    /* Save LR for return via MSP or PSP */
    __asm("     PUSH    {LR}");

    /* Trapping source: MSP or PSP? */
    __asm("     TST     LR,#4");    /* Bit #3 (0100b) indicates MSP (0) or PSP (1) */
    __asm("     BNE     RtnViaPSP");

    /* Trapping source is MSP - save r4-r11 on stack (default, so just push) */
    __asm("     PUSH    {r4-r11}");
    __asm("     MRS r0,msp");
    __asm("     BL  SVCHandler");   /* r0 is MSP */
    __asm("     POP {r4-r11}");
    __asm("     POP     {PC}");

    /* Trapping source is PSP - save r4-r11 on psp stack (MSP is active stack) */
    __asm("RtnViaPSP:");
    __asm("     mrs     r0,psp");
    __asm("     stmdb   r0!,{r4-r11}"); /* Store multiple, decrement before */
    __asm("     msr psp,r0");
    __asm("     BL  SVCHandler");   /* r0 Is PSP */

    /* Restore r4..r11 from trapping process stack  */
    __asm("     mrs     r0,psp");
    __asm("     ldmia   r0!,{r4-r11}"); /* Load multiple, increment after */
    __asm("     msr psp,r0");
    __asm("     POP     {PC}");

}

void SVCHandler(struct stack_frame *argptr)
{
    /*
     * Supervisor call handler
     * Handle startup of initial process
     * Handle all other SVCs such as getid, terminate, etc.
     * Assumes first call is from startup code
     * Argptr points to (i.e., has the value of) either:
       - the top of the MSP stack (startup initial process)
       - the top of the PSP stack (all subsequent calls)
     * Argptr points to the full stack consisting of both hardware and software
       register pushes (i.e., R0..xPSR and R4..R10); this is defined in type
       stack_frame
     * Argptr is actually R0 -- setup in SVCall(), above.
     * Since this has been called as a trap (Cortex exception), the code is in
       Handler mode and uses the MSP
     */
    static int firstSVCcall = TRUE;
    PCB * temp;
    struct kcallargs *kcaptr;

    if (firstSVCcall)
    {
        /*
         * Force a return using PSP
         * This will be the first process to run, so the eight "soft pulled" registers
           (R4..R11) must be ignored otherwise PSP will be pointing to the wrong
           location; the PSP should be pointing to the registers R0..xPSR, which will
           be "hard pulled"by the BX LR instruction.
         * To do this, it is necessary to ensure that the PSP points to (i.e., has) the
           address of R0; at this moment, it points to R4.
         * Since there are eight registers (R4..R11) to skip, the value of the sp
           should be increased by 8 * sizeof(unsigned int).
         * sp is increased because the stack runs from low to high memory.
        */
        set_PSP(running -> sp + 8 * sizeof(unsigned int));

        firstSVCcall = FALSE;

        /* Start SysTick */
        SysTick_init();

        /*
         - Change the current LR to indicate return to Thread mode using the PSP
         - Assembler required to change LR to FFFF.FFFD (Thread/PSP)
         - BX LR loads PC from PSP stack (also, R0 through xPSR) - "hard pull"
        */
        __asm(" movw    LR,#0xFFFD");  /* Lower 16 [and clear top 16] */
        __asm(" movt    LR,#0xFFFF");  /* Upper 16 only */
        __asm(" bx  LR");              /* Force return to PSP */
    }
    else /* Subsequent SVCs */
    {
        /*
         * kcaptr points to the arguments associated with this kernel call
         * argptr is the value of the PSP (passed in R0 and pointing to the TOS)
         * the TOS is the complete stack_frame (R4-R10, R0-xPSR)
         * in this example, R7 contains the address of the structure supplied by
            the process - the structure is assumed to hold the arguments to the
            kernel function.
         * to get the address and store it in kcaptr, it is simply a matter of
           assigning the value of R7 (arptr -> r7) to kcaptr
         */
        kcaptr=(struct kcallargs *)argptr->r7;

        //SVC restores registers of what ever was running afterwards
        //must handle new running AFTER return
        switch(kcaptr->code)
        {
        case GETID:
            kcaptr->rtnvalue = running->PID;
            break;
        case NICE:
            //process wishes to change priority
            //PCB stored in kcaptr->arg2
            ((PCB *)kcaptr->arg2)->priority = kcaptr->arg1;
            //rearranges priorities
            newPriorityQueue(((PCB *)kcaptr->arg2));

            if(((PCB *)kcaptr->arg2) != running){
                ((PCB *)kcaptr->arg2)->sp = get_PSP();
                set_PSP(running->sp);
            }

            break;
        case TERMINATE:
            //preserves running when destroying terminating function
            //remove PCB from Queue, sets proper running

            temp = running;
            removeProcess(temp); //also defines new running
            free(&(temp->base_sp));
            free(temp);
            set_PSP(running->sp);

            break;
        case SEND:
            temp = running;
            k_send(kcaptr);
            if(temp != running){
                /*  process was blocked, save PSP   */
                temp->sp = get_PSP();
                /*  Set PSP to now running PSP  */
                set_PSP(running->sp);
            }
            break;
        case RECV:
            temp = running;
            k_recv(kcaptr);
            kcaptr->arg1 = (unsigned int)temp->message;
            if(temp != running){
                /*  process was blocked, save PSP   */
                temp->sp = get_PSP();
                /*  Set PSP to now running PSP  */
                set_PSP(running->sp);
            }
            break;
        case BIND:
            k_bind(kcaptr);
            break;
        case UNBIND:
            k_unbind(kcaptr);
            break;
        }
    }
}

