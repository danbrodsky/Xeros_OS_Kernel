/* msg.c : messaging system 
   This file does not need to modified until assignment 2
 */

#include <xeroskernel.h>
#include <stdarg.h>

/* Your code goes here */

int send(pcb *p){

    va_list ap = (va_list) p->args;
    PID_t receiver_pid = va_arg(ap, PID_t);

    int send_val = va_arg(ap, int);


    // If the process tries to receive from is itself then −3 is returned immediately.
    if(receiver_pid == p->pid){
        p->ret = SELF_REF_ERR;
        return FALSE;
    }


    // check if blocking
    int i;
    int flag = FALSE;
    for( i = 0; i < MAX_PROC; i++ ) {
        if(proctab[i].pid != NULL_PROC_PID && proctab[i].pid != p->pid && proctab[i].state != STATE_STOPPED){
            flag = TRUE;
        }

        if( proctab[i].pid == receiver_pid) {
            if(proctab[i].state == STATE_BLOCKED) {
                ap = (va_list) proctab[i].args;
                PID_t *sender_pid = va_arg(ap, PID_t*);
                int *rec_val = va_arg(ap, int*);

                if (p->pid == *sender_pid || *sender_pid == ANY_SYS_CALL) {
                    // set receive value
                    *rec_val = send_val;

                    // set ret value of receipt process and put it on ready queue
                    proctab[i].ret = TRANSFER_SUCCESS;
                    proctab[i].state = STATE_READY;
                    ready(&proctab[i]);

                    // set ret value of sender process and return
                    p->ret = TRANSFER_SUCCESS;
                    return FALSE;
                } else{
                    p->state = STATE_BLOCKED;
                    return TRUE;
                }
            } else if(proctab[i].state == STATE_READY){
                p->state = STATE_BLOCKED;
                return TRUE;
            } else if(proctab[i].state == STATE_STOPPED){
                p->ret = PROC_DOES_NOT_EXIST_ERR;
                return FALSE;
            }
        }
    }

    if(flag){
        p->ret = PROC_DOES_NOT_EXIST_ERR;
        return FALSE;
    } else{
        p->ret = NO_PROC_RUNNING_ERR;
        return FALSE;
    }
}

int receive(pcb *p){
    va_list ap = (va_list) p->args;
    PID_t *sender_pid = va_arg(ap, PID_t*);
    int *rec_val = va_arg(ap, int*);

    if(!is_valid_address(sender_pid)){
        p->ret = INVALID_FROM_PID_ERR;
        return FALSE;
    }

    if(!is_valid_address( rec_val)){
        p->ret = INVALID_NUM_ERR;
        return FALSE;
    }

    // If the process tries to receive from is itself then −3 is returned immediately.
    if(*sender_pid == p->pid){
        p->ret = SELF_REF_ERR;
        return FALSE;
    }

    int i;
    int flag = FALSE;
    for( i = 0; i < MAX_PROC; i++ ) {
        if(proctab[i].pid != NULL_PROC_PID && proctab[i].pid != p->pid && proctab[i].state != STATE_STOPPED){
            flag = TRUE;
        }

        if( proctab[i].pid == *sender_pid || *sender_pid == ANY_SYS_CALL) {
            if(proctab[i].state == STATE_BLOCKED) {
                ap = (va_list) proctab[i].args;
                PID_t receiver_pid = va_arg(ap, PID_t);
                int send_val = va_arg(ap, int);

                if (p->pid == receiver_pid) {
                    // set receive value
                    *sender_pid = proctab[i].pid;
                    *rec_val = send_val;

                    // set ret value of sender process and put it on ready queue
                    proctab[i].ret = TRANSFER_SUCCESS;
                    proctab[i].state = STATE_READY;
                    ready(&proctab[i]);

                    // set ret value of sender process and return
                    p->ret = TRANSFER_SUCCESS;
                    return FALSE;
                } else if (*sender_pid != ANY_SYS_CALL) {
                    p->state = STATE_BLOCKED;
                    return TRUE;
                }
            } else if(proctab[i].state == STATE_READY){
                p->state = STATE_BLOCKED;
                return TRUE;
            } else if(*sender_pid != 0 && proctab[i].state == STATE_STOPPED){
                p->ret = PROC_DOES_NOT_EXIST_ERR;
                return FALSE;
            }
        }
    }

    if (*sender_pid == ANY_SYS_CALL) {
        p->state = STATE_BLOCKED;
        return TRUE;
    }

    if(flag){
        p->ret = PROC_DOES_NOT_EXIST_ERR;
        return FALSE;
    } else{
        p->ret = NO_PROC_RUNNING_ERR;
        return FALSE;
    }
}
