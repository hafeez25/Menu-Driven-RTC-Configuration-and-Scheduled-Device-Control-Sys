//rtc.c
#include <lpc21xx.h>
#include "types.h"
#include "defines.h"
#include "rtc.h"

/* RTC runs from PCLK through the built-in prescaler by default. This
   needs no external hardware, so it works even on boards where the
   32.768 kHz RTC crystal is missing or not wired up -- the single most
   common reason the clock "doesn't run" (SEC/MIN/HOUR stay frozen).

   If your board DOES have a working RTC crystal fitted and you'd rather
   use it, define USE_RTC_XTAL below.                                    */
/* #define USE_RTC_XTAL */

#define RTC_ENABLE   0
#define RTC_RESET    1
#define RTC_CLKSRC   4

#define PCLK        15000000UL      /* FOSC 12MHz, CCLK 60MHz, VPBDIV 4 */
#define PREINT_VAL  ((PCLK/32768UL)-1)
#define PREFRAC_VAL (PCLK-((PREINT_VAL+1)*32768UL))

const u8 week[7][4] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};

void RTC_Init(void)
{
	SETBIT(CCR, RTC_RESET);                /* disable + reset the RTC   */

#ifdef USE_RTC_XTAL
	CCR = 0;
	SETBIT(CCR, RTC_ENABLE);
	SETBIT(CCR, RTC_CLKSRC);               /* clock from 32.768kHz xtal */
#else
	PREINT  = PREINT_VAL;
	PREFRAC = PREFRAC_VAL;
	CCR = 0;
	SETBIT(CCR, RTC_ENABLE);               /* clock from PCLK           */
#endif

	CIIR = 0;                              /* no counter increment int  */
	AMR  = 0xFF;                           /* mask all alarms           */
}

/* ------------------- time ---------------------------------------- */
void SetRTCTimeInfo(u32 hour, u32 minute, u32 second)
{
	HOUR = hour;
	MIN  = minute;
	SEC  = second;
}

void GetRTCTimeInfo(s32 *hour, s32 *minute, s32 *second)
{
	*hour   = HOUR;
	*minute = MIN;
	*second = SEC;
}

/* ------------------- date ---------------------------------------- */
void SetRTCDateInfo(u32 date, u32 month, u32 year)
{
	DOM   = date;
	MONTH = month;
	YEAR  = year;
}

void GetRTCDateInfo(s32 *date, s32 *month, s32 *year)
{
	*date  = DOM;
	*month = MONTH;
	*year  = YEAR;
}

/* ------------------- day of week --------------------------------- */
void SetRTCDay(u32 dow)
{
	DOW = dow;
}

void GetRTCDay(s32 *dow)
{
	*dow = DOW;
}

/* ------------------- calendar helpers ---------------------------- */
/* Needed to validate the Edit RTC menu: reject 30 Feb, 31 Apr, etc,
   and only accept 29 Feb on a leap year.                            */
u32 IsLeapYear(u32 year)
{
	if((year%400)==0) return 1;
	if((year%100)==0) return 0;
	if((year%4)==0)   return 1;
	return 0;
}

u32 DaysInMonth(u32 month, u32 year)
{
	const u8 dim[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
	if(month<1 || month>12) return 0;
	if(month==2 && IsLeapYear(year)) return 29;
	return dim[month];
}

/* A plausible calendar means the RTC has actually been configured.
   Used at power-up so the device stays OFF until the RTC and the
   schedule are both valid, as required by the project spec.        */
u32 IsRTCValid(void)
{
	if(HOUR>23 || MIN>59 || SEC>59)            return 0;
	if(MONTH<1 || MONTH>12)                    return 0;
	if(YEAR<2000 || YEAR>2099)                 return 0;
	if(DOM<1 || DOM>DaysInMonth(MONTH,YEAR))   return 0;
	if(DOW>6)                                  return 0;
	return 1;
}
