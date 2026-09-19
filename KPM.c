//KPM.c
#include <lpc21xx.h>
#include "KPM_defines.h"
#include "types.h"
#include "defines.h"
#include "KPM.h"
#include "delay.h"
#include "timer.h"

/*  4x4 keypad look-up table
	7 8 9 /
	4 5 6 *
	1 2 3 -
	C 0 = +      ( C = CLEAR/CANCEL , = = ENTER )                 */
u8 kpmLUT[4][4]={{'7','8','9','/'},
                 {'4','5','6','*'},
                 {'1','2','3','-'},
                 {'C','0','=','+'}};

void InitKPM(void)
{
	/* rows P1.16 - P1.19 as output, columns P1.20 - P1.23 as input */
	IODIR1 |= 15<<ROW0;
	IOCLR1  = 15<<ROW0;          /* drive all rows low */
}

u32 ColScan(void)
{
	if(((IOPIN1>>COL0)&15)<15)
		return 0;                /* some key is pressed */
	else
		return 1;                /* no key pressed      */
}

u32 RowCheck(void)
{
	u32 rno;
	for(rno=0; rno<4; rno++)
	{
		IOPIN1 = (IOPIN1 & ~(15<<ROW0)) | ((~(1<<rno))<<ROW0);
		if(ColScan()==0)
			break;
	}
	IOCLR1 = 15<<ROW0;           /* restore all rows low */
	return rno;
}

u32 ColCheck(void)
{
	u32 cno;
	for(cno=0; cno<4; cno++)
	{
		if(((IOPIN1>>(COL0+cno))&1)==0)
			break;
	}
	return cno;
}

u32 KeyScan(void)
{
	u32 rno, cno, key;

	while(ColScan());            /* wait for press   */
	delay_ms(10);                /* debounce         */
	rno = RowCheck();
	cno = ColCheck();
	if(rno>3 || cno>3)           /* spurious / released early */
	{
		while(!ColScan());
		return KEY_NONE;
	}
	key = kpmLUT[rno][cno];
	while(!ColScan());           /* wait for release */
	delay_ms(20);
	return key;
}

/* Non blocking version : returns KEY_NONE (0) ONLY on a real timeout.
   Contact bounce or a key released during the scan makes it keep waiting,
   it must never report a timeout while the user is still typing.        */
u32 KeyScanT(u32 timeout_ms)
{
	u32 rno, cno, key, start;

	start = GetTicks();

	while(1)
	{
		/* wait for a press, give up only when the timeout really expires */
		while(ColScan())
		{
			if(Elapsed(start) >= timeout_ms)
				return KEY_NONE;
		}

		delay_ms(10);                /* debounce                          */
		if(ColScan())
			continue;                /* it was only a bounce, keep waiting */

		rno = RowCheck();
		cno = ColCheck();

		if(rno>3 || cno>3)           /* released during the scan          */
		{
			while(!ColScan());
			delay_ms(10);
			continue;                /* do NOT abort the field entry       */
		}

		key = kpmLUT[rno][cno];

		while(!ColScan());           /* wait for release                  */
		delay_ms(10);
		return key;
	}
}

u32 ReadNum(void)
{
	u8  key;
	u32 sum=0;
	while(1)
	{
		key = KeyScan();
		if(key>='0' && key<='9')
			sum = (sum*10)+(key-48);
		else
			break;
	}
	return sum;
}
