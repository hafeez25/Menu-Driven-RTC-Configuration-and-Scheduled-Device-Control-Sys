//timer.h  -- Timer0 used as a free running 1ms tick for input timeouts
#ifndef _TIMER_H_
#define _TIMER_H_
/* types.h is included by the .c file before this header */

void  InitTimer0(void);      /* free running, TC increments every 1 ms */
u32   GetTicks(void);        /* current 1ms tick value                 */
void  ResetTicks(void);      /* restart the tick counter from 0        */
u32   Elapsed(u32 start);    /* ms elapsed since 'start'               */

#endif
