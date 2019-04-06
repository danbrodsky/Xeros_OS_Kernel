/* signal.c - support for signal handling
   This file is not used until Assignment 3
 */

#include <xeroskernel.h>
#include <xeroslib.h>

#define STARTING_EFLAGS         0x00003000
#define ARM_INTERRUPTS          0x00000200

void sigtramp(void (*handler)(void *), void *cntx){
    handler(cntx);
    syssigreturn(cntx);
}

void signal(pcb *p){
    int i;
    int signum = -1;
    int last_signum = -1;

    for(i = MAX_SIG -1 ; i >= 0 ; i--){
        if(p->signal_mask[i]){
            last_signum = i;
            break;
        }
    }

    for(i = 0 ; i < MAX_SIG ; i++){
        if(p->pending_signals[i]){
            signum = i;
            break;
        }
    }

    // If last_signum is greater or equal signum this means that no signals can be executed so return.
    // If signum is equal to -1 this means that no signals are pending
    if(signum <= last_signum || signum == -1){
        return;
    }

    int *esp = (int *)p->esp;

    // push last signum onto stack
    esp--;
    *esp = last_signum;

    // push old_sp onto stack
    esp--;
    *esp = p->esp;

    // push handler
    esp--;
    *esp = p->signal_table[signum];

    // push place holder return address
    esp--;
    *esp = NULL;

    // push branch new context frame onto stack
    context_frame *cf = (context_frame *) esp;
    cf--;
    memset(cf, 0xA5, sizeof( context_frame ));
    cf->iret_cs = getCS();
    cf->iret_eip = (unsigned int) &sigtramp;
    cf->eflags = STARTING_EFLAGS | ARM_INTERRUPTS;
    cf->esp = (int)(cf + 1);
    cf->ebp = cf->esp;
    cf->stackSlots[0] = (int) sysstop;

    // set new sp to point to the top of the stack
    p->esp = (void*) cf;

    // flush pending signal
    p->pending_signals[signum] = FALSE;
    // set signal mask
    for(i = 0 ; i <= signum; i++){
        p->signal_mask[i] = 1;
    }

    // recursively add signal context onto the stack
    // if no signals are left to be handled this function will return
    signal(p);
}
