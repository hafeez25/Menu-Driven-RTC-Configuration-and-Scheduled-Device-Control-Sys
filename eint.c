//eint.c
#include <lpc21xx.h>
#include "types.h"
#include "defines.h"
#include "eint.h"

#define EINT0_CHNO 14
#define SW_PIN     1        /* P0.1 = EINT0, the config switch         */

volatile u32 menuRequest = 0;

/* The ISR ONLY records the request. All menu / keypad work happens in
   main(), so the interrupt stays as short as possible (per spec).     */
void eint0_isr(void) __irq
{
	menuRequest = 1;
	SETBIT(EXTINT, 0);      /* clear the EINT0 flag  */
	VICVectAddr = 0;        /* end of interrupt      */
}

void EINT0_Enable(void)
{
	/* cfg P0.1 as EINT0 (function 3, so both PINSEL0 bits =1) */
	PINSEL0 |= 3<<(SW_PIN*2);

	SETBIT(EXTMODE, 0);     /* edge sensitive                  */
	CLRBIT(EXTPOLAR, 0);    /* falling edge (switch to GND)    */
	SETBIT(EXTINT, 0);      /* clear any flag left over        */

	CLRBIT(VICIntSelect, EINT0_CHNO);   /* EINT0 as IRQ, not FIQ  */
	VICVectAddr0 = (u32)eint0_isr;      /* ISR address in slot 0  */
	VICVectCntl0 = (1<<5)|EINT0_CHNO;   /* enable slot 0          */
	SETBIT(VICIntEnable, EINT0_CHNO);   /* enable EINT0 source    */
}
