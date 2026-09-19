//timer.c
#include <lpc21xx.h>
#include "types.h"
#include "timer.h"

#define PCLK 15000000UL      /* CCLK = 60MHz, VPBDIV = 4 */

void InitTimer0(void)
{
	T0TCR = 1<<1;                 /* reset timer counter          */
	T0PR  = (PCLK/1000UL)-1;      /* prescaler -> TC ticks = 1 ms */
	T0MCR = 0;                    /* no match action, free run    */
	T0TCR = 1<<0;                 /* start timer                  */
}

u32 GetTicks(void)
{
	return T0TC;
}

void ResetTicks(void)
{
	T0TCR = 1<<1;      /* hold in reset */
	T0TCR = 1<<0;      /* release, run  */
}

u32 Elapsed(u32 start)
{
	return (T0TC - start);        /* unsigned wrap-around safe */
}
