/* sleep.c : sleep device 
   This file does not need to modified until assignment 2
 */

#include <xeroskernel.h>
#include <xeroslib.h>

static pcb      *head = NULL;

static unsigned long long time = 0;


/* Your code goes here */


unsigned int sleep ( pcb *p, unsigned int delay ) {
/*******************************/

    // convert delay from milliseconds into time slices of 10ms
    delay = delay / 10;

    unsigned int prev_delay = 0;
    p->next = NULL;
    p->state = STATE_BLOCKED;


    if( head ) {

        pcb *curr = head;
        pcb *prev = NULL;
        // find position in sleeping queue for p
        while (curr != NULL) {
            // count previous delay to find where p should be placed
            if (curr->delay <= delay - prev_delay) {
                prev_delay += curr->delay;
            } else {
                // place p into sleeping queue
                if (prev) {
                    prev->next = p;
                    p->next = curr;
                    break;
                } else {
                    // p delay smaller than head, place at front
                    p->next = head;
                    head = p;
                    break;
                }
            }

            prev = curr;
            curr = curr->next;

            // if p delay is greater than any value in queue, place at back
            if (!curr) {
                prev->next = p;
                break;
            }
        }

    } else {
        // no p in sleeping queue yet
        head = p;
    }

    p->delay = delay - prev_delay;
    // decrease relative delay of p following the inserted one
    for (pcb* n = p->next; n != NULL; n = n->next) {
        n->delay -= delay - prev_delay;
    }

    return 0;
}

void tick(void) {

    // increment system time
    ++time;
    dprintv("current system time is: ", time);

    while (head) {
        // decrement head by one tick
        head->delay -= 1;

        // sleep time complete, move process back to ready queue
        if (head->delay <= 0) {
            pcb *next = head->next;
            // call ready() from disp.c
            head->ret = 0;
            ready(head);
            head = next;
        } else {
            break;
        }

    }
}
