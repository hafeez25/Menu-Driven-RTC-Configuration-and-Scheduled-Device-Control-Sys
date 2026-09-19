//eint.h  -- configuration switch on EINT0 (P0.1)
#ifndef _EINT_H_
#define _EINT_H_
/* types.h is included by the .c file before this header */

extern volatile u32 menuRequest;   /* set by the ISR, cleared by main */

void EINT0_Enable(void);

#endif
