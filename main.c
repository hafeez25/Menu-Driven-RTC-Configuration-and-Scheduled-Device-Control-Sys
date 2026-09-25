/*====================================================================
	main.c
	Menu Driven RTC Configuration and Scheduled Device Control System
	Target : LPC2148  (FOSC 12 MHz, CCLK 60 MHz, PCLK 15 MHz)

	ID : V25HE11M13

	Connections
		LCD data     P0.8  - P0.15
		LCD RS/RW/EN P0.16 / P0.17 / P0.18
		Keypad rows  P1.16 - P1.19      columns P1.20 - P1.23
		Config SW    P0.1  (EINT0, active low)
		Device LED   P0.4  (active low)

	Where each project requirement is met:
		1. RTC info on LCD .......... ShowStatus()
		2. Edit RTC via keypad ...... EditRTC() + validation in GetValue()
		3. Set device ON/OFF time ... EditSchedule()
		4. Auto device control ...... UpdateDevice()
		5. Change RTC PIN ........... ChangePin()
		- ISR only records the switch press, menu runs in main() .. eint.c
		- Menu / field timeout ...... KeyScanT() in KPM.c, timer.c
		- Schedule kept in flash via IAP, last sector only ......... iap.c
		- A field is only accepted once '=' (KEY_ENTER) is pressed;
		  BACKSPACE lets the user erase the last digit typed instead
		  of clearing the whole field and starting over ............ ReadField()
====================================================================*/
#include <lpc21xx.h>
#include "delay.h"
#include "defines.h"
#include "LCD.h"     /* LCD.h brings in types.h */
#include "LCD_defines.h"
#include "KPM.h"
#include "rtc.h"
#include "eint.h"
#include "iap.h"
#include "timer.h"

/*------------------------- device output ---------------------------*/
#define DEVICE_PIN      4          /* P0.4 drives the device LED     */
#define DEVICE_ON()     SSETBIT(IOCLR0, DEVICE_PIN)   /* active LOW */
#define DEVICE_OFF()    SSETBIT(IOSET0, DEVICE_PIN)

/*------------------------- timeouts (ms) ---------------------------*/
#define MENU_TIMEOUT    10000UL    /* menu closes if no key pressed  */
#define INPUT_TIMEOUT   10000UL    /* each field entry timeout       */

/*------------------------- RTC edit PIN lock ------------------------*/
#define RTC_PIN_DEFAULT 1234       /* PIN used the first time the board
                                       ever runs / after a flash erase */
#define PIN_MAX_TRY     3          /* wrong tries allowed before lock*/
#define PIN_LOCK_MS     30000UL   /* lockout length : 30 seconds     */

/*------------------------- keypad keys not already defined ----------
	KEY_NONE / KEY_CLEAR / KEY_ENTER are assumed to already exist in
	KPM.h. KEY_BACKSPACE is new: map it in KPM.h to whichever physical
	key on your matrix should act as "erase last digit". The definition
	below is only a fallback so this file still compiles stand-alone;
	if KPM.h already defines KEY_BACKSPACE this is skipped.
----------------------------------------------------------------------*/
#ifndef KEY_BACKSPACE
#define KEY_BACKSPACE   '\\'       /* the '\' key on the matrix acts as
                                       backspace - change this if your
                                       KPM.h maps it to something else */
#endif

/*------------------------- LCD cursor shift --------------------------
	Standard HD44780 "cursor shift left" command (display unchanged).
	Used by the backspace handler to walk the cursor back over the
	digit being deleted. Skipped if LCD_defines.h already has it.
----------------------------------------------------------------------*/
#ifndef CURSOR_LEFT
#define CURSOR_LEFT     0x10
#endif

/*------------------------- globals ---------------------------------*/
SCHEDULE sched;                    /* ON / OFF schedule              */
u32      deviceState = 0;          /* 0 = OFF , 1 = ON               */
s32      hour, min, sec, date, month, year, day;

static u32 pinLocked    = 0;       /* 1 while RTC editing is locked  */
static u32 pinLockStart = 0;       /* tick value when lock began     */
static u32 rtcPin       = RTC_PIN_DEFAULT;   /* current RTC PIN, changeable
                                                 at runtime via ChangePin().
                                                 NOTE: this lives in RAM only,
                                                 so it reverts to
                                                 RTC_PIN_DEFAULT after a power
                                                 cycle unless it is also saved
                                                 to flash the way sched is
                                                 (see IAP_SaveSchedule) -
                                                 hook that up in iap.c if you
                                                 need the PIN to survive a
                                                 reset.                     */

/*====================================================================
	Small LCD helpers
====================================================================*/
/* StrLCD() takes u8* : this wrapper keeps the string literals clean */
static void Str(const char *p)
{
	StrLCD((u8*)p);
}

static void Print2(u32 v)                 /* always two digits */
{
	WRITE_LCD_DATA((v/10)%10 + '0');
	WRITE_LCD_DATA(v%10 + '0');
}

static void Message(const char *l1, const char *l2, u32 ms)
{
	WRITE_LCD_CMD(CLEAR_LCD);
	Str(l1);
	WRITE_LCD_CMD(GOTO_LINE2_POS0);
	Str(l2);
	delay_ms(ms);
}

/*====================================================================
	Power on banner
	Line 1 : student ID
	Line 2 : project title, scrolled because it is longer than 16 chars
====================================================================*/
static void ShowBanner(void)
{
	u8  title[] = "                 MENU DRIVEN RTC AND SCHEDULED DEVICE CONTROL";
	u32 i, j, len = 0;

	while(title[len]) len++;

	WRITE_LCD_CMD(CLEAR_LCD);
	WRITE_LCD_CMD(GOTO_LINE1_POS0);
	Str("ID : V25HE11M13");            /* ID first, on line 1      */

	for(i=0; i<=len-16; i++)              /* project name on line 2   */
	{
		WRITE_LCD_CMD(GOTO_LINE2_POS0);
		for(j=0; j<16; j++)
			WRITE_LCD_DATA(title[i+j]);
		delay_ms(120);
	}
	delay_ms(300);
}

/*====================================================================
	Keypad input with echo, timeout and validation

	- Digits typed are only echoed/buffered; the field is NOT auto
	  accepted once "digits" characters have been entered any more.
	  The user must press '=' (KEY_ENTER) to confirm the field.
	- BACKSPACE erases the last digit typed (cursor back, blank it,
	  cursor back again) instead of forcing a full re-entry.
	- 'C' (KEY_CLEAR) still throws the whole field away and lets the
	  caller re-prompt from scratch.

	returns  >=0 : accepted value (user pressed '=')
	          -1 : timed out
	          -2 : cancelled with the 'C' key
====================================================================*/
static s32 ReadField(u8 digits)
{
	u32 key, i = 0, j, val = 0;
	u8  buf[5];

	while(1)
	{
		key = KeyScanT(INPUT_TIMEOUT);

		if(key == KEY_NONE)                       /* timeout          */
			return -1;

		if(key == KEY_CLEAR)                      /* erase last digit;
		                                              if nothing is left
		                                              to erase, cancel
		                                              the field         */
		{
			if(i > 0)
			{
				i--;
				WRITE_LCD_CMD(CURSOR_LEFT);        /* step back        */
				WRITE_LCD_DATA(' ');               /* blank the digit  */
				WRITE_LCD_CMD(CURSOR_LEFT);        /* step back again  */
				continue;
			}
			return -2;
		}

		if(key == KEY_BACKSPACE)                  /* erase last digit */
		{
			if(i > 0)
			{
				i--;
				WRITE_LCD_CMD(CURSOR_LEFT);        /* step back        */
				WRITE_LCD_DATA(' ');               /* blank the digit  */
				WRITE_LCD_CMD(CURSOR_LEFT);        /* step back again  */
			}
			continue;
		}

		if(key == KEY_ENTER)                      /* '=' : confirm field */
		{
			if(i == 0) continue;                  /* nothing typed    */
			break;
		}

		if(key>='0' && key<='9')
		{
			if(i < digits)
			{
				buf[i++] = (u8)key;
				WRITE_LCD_DATA((u8)key);
			}
			/* field is deliberately NOT auto-accepted here any more -
			   it now only commits once '=' is pressed above          */
		}
	}

	for(j=0; j<i; j++)
		val = val*10 + (buf[j]-'0');

	delay_ms(400);         /* let the finished value sit on screen a
	                           moment before the caller clears it out,
	                           so the last digit is actually seen      */

	return (s32)val;
}

/* prompt, read and range check one field                            */
static s32 GetValue(const char *prompt, u32 lo, u32 hi, u8 digits)
{
	s32 v;

	while(1)
	{
		WRITE_LCD_CMD(CLEAR_LCD);
		Str(prompt);
		WRITE_LCD_CMD(GOTO_LINE2_POS0);
		Str("> ");

		v = ReadField(digits);

		if(v == -1)                                /* timeout         */
			return -1;
		if(v == -2)                                /* C pressed       */
			continue;                              /* enter it again  */

		if((u32)v < lo || (u32)v > hi)
		{
			Message("INVALID ENTRY  ", "TRY AGAIN...   ", 800);
			continue;
		}
		return v;
	}
}

/*====================================================================
	PIN check that guards RTC editing
	returns 1 : PIN accepted, caller may proceed to edit the RTC
	        0 : cancelled / timed out / locked out after 3 wrong tries
====================================================================*/
static u32 VerifyPin(void)
{
	s32 pin;
	u32 tries;

	if(pinLocked)                              /* still inside a lock */
	{
		if(Elapsed(pinLockStart) < PIN_LOCK_MS)
		{
			u32 remain = (PIN_LOCK_MS - Elapsed(pinLockStart))/1000 + 1;
			WRITE_LCD_CMD(CLEAR_LCD);
			Str("RTC LOCKED      ");
			WRITE_LCD_CMD(GOTO_LINE2_POS0);
			Str("WAIT ");
			U32LCD(remain);
			Str(" SEC    ");
			delay_ms(1000);
			return 0;
		}
		pinLocked = 0;                         /* lock period is over */
	}

	for(tries = 0; tries < PIN_MAX_TRY; tries++)
	{
		pin = GetValue("ENTER RTC PIN-4D", 0, 9999, 4);

		if(pin < 0)                             /* timeout : back to menu */
			return 0;

		if((u32)pin == rtcPin)
			return 1;                           /* correct                */

		Message("WRONG PIN       ", "TRY AGAIN       ", 800);
	}

	/* three wrong attempts in a row : lock RTC editing for one minute */
	pinLocked    = 1;
	pinLockStart = GetTicks();
	Message("TOO MANY TRIES  ", "LOCKED 30 SEC   ", 900);
	return 0;
}

/*====================================================================
	Menu option : change the RTC PIN
	Must re-enter the CURRENT pin first (VerifyPin, same lockout rules
	apply), then the new pin twice to guard against a typo locking the
	user out of their own RTC menu.
====================================================================*/
static void ChangePin(void)
{
	s32 newPin, confirmPin;

	if(!VerifyPin())                 /* prove you know the old PIN first */
		return;

	newPin = GetValue("NEW PIN(4 DIG) ", 0, 9999, 4);
	if(newPin < 0) return;

	confirmPin = GetValue("CONFIRM NEW PIN", 0, 9999, 4);
	if(confirmPin < 0) return;

	if(newPin != confirmPin)
	{
		Message("PINS DID NOT    ", "MATCH - ABORTED ", 900);
		return;
	}

	rtcPin = (u32)newPin;
	Message("PIN CHANGED     ", "SUCCESSFULLY    ", 800);
}

/*====================================================================
	Menu option 1 : edit RTC time and calendar
====================================================================*/
static void EditRTC(void)
{
	s32 h, m, s, dw, dd, mo, yy;

	if(!VerifyPin())            /* PIN required before any RTC edit  */
		return;

	h = GetValue("SET HOUR(0-23)", 0, 23, 2);    if(h  < 0) return;
	m = GetValue("SET MIN(0-59)", 0, 59, 2);    if(m  < 0) return;
	s = GetValue("SET SEC(0-59)", 0, 59, 2);    if(s  < 0) return;
	dw= GetValue("DAY(0-SUN/6-SAT)", 0,  6, 1);    if(dw < 0) return;
	dd= GetValue("SET DATE(1-31)", 1, 31, 2);    if(dd < 0) return;
	mo= GetValue("SET MONTH(1-12)", 1, 12, 2);    if(mo < 0) return;
	yy= GetValue("YEAR(2000-2030)", 2000, 2030, 4); if(yy < 0) return;

	/* the date can only be checked once month and year are known,
	   this also covers February in a leap year                      */
	while((u32)dd > DaysInMonth(mo, yy))
	{
		Message("DATE INVALID FOR", "THIS MONTH/YEAR ", 900);
		dd = GetValue("SET DATE       ", 1, DaysInMonth(mo, yy), 2);
		if(dd < 0) return;
	}

	SetRTCTimeInfo(h, m, s);
	SetRTCDateInfo(dd, mo, yy);
	SetRTCDay(dw);

	Message("RTC UPDATED     ", "SUCCESSFULLY    ", 800);
}

/*====================================================================
	Menu option 2 : edit device ON / OFF times
====================================================================*/
static void EditSchedule(void)
{
	s32 onH, onM, offH, offM;
	SCHEDULE tmp;

	onH  = GetValue("ON HOUR(0-23)", 0, 23, 2);  if(onH  < 0) return;
	onM  = GetValue("ON MIN(0-59)", 0, 59, 2);  if(onM  < 0) return;
	offH = GetValue("OFF HOUR(0-23)", 0, 23, 2);  if(offH < 0) return;
	offM = GetValue("OFF MIN(0-59)", 0, 59, 2);  if(offM < 0) return;

	/* identical ON and OFF times give an ambiguous period            */
	if(onH == offH && onM == offM)
	{
		Message("ON = OFF TIME   ", "NOT ALLOWED     ", 900);
		return;
	}

	tmp.onHour  = onH;
	tmp.onMin   = onM;
	tmp.offHour = offH;
	tmp.offMin  = offM;
	tmp.valid   = 1;

	sched = tmp;                         /* usable immediately either way */

	if(IAP_SaveSchedule(&tmp) == 0)
		Message("SCHEDULE SAVED  ", "SUCCESSFULLY    ", 800);
	else
		Message("SAVED (RAM ONLY)", "LOST ON RESET   ", 900);
}

/*====================================================================
	Menu driven interface, entered from the main loop after the
	configuration switch interrupt has set menuRequest
====================================================================*/
static void ShowMenu(void)
{
	s32 key;

	WRITE_LCD_CMD(DISP_ON_CUR_BLINK);   /* visible cursor for the whole
	                                        menu, so every keypad digit
	                                        is clearly seen as it is typed */

	while(1)
	{
		WRITE_LCD_CMD(CLEAR_LCD);
		Str("1:RTC 2:SCHD");
		WRITE_LCD_CMD(GOTO_LINE2_POS0);
		Str("3:PIN 4:EXIT S:");

		/* one digit, same rules as every other field: 'C' / backspace
		   erase it, and it only takes effect once '=' is pressed - no
		   more acting on the very first key pressed                  */
		key = ReadField(1);

		if(key == -1)                                /* timeout exit   */
		{
			Message("NO INPUT        ", "EXITING MENU... ", 700);
			WRITE_LCD_CMD(DISP_ON_CUR_OFF);
			return;
		}

		if(key == -2)                    /* cleared with nothing typed,
		                                     just redraw and ask again  */
			continue;

		switch(key)
		{
			case 1 : EditRTC();        break;
			case 2 : EditSchedule();   break;
			case 3 : ChangePin();      break;
			case 4 : WRITE_LCD_CMD(DISP_ON_CUR_OFF); return;
			default: Message("INVALID OPTION  ",
			                 "USE 1,2,3 OR 4  ", 800);
		}
	}
}

/*====================================================================
	Device control
	The ON time is included in the operating period and the OFF time is
	excluded. If ON is later than OFF the period runs across midnight.
====================================================================*/
static void UpdateDevice(void)
{
	u32 now, on, off, want;

	if(!sched.valid || !IsRTCValid())      /* nothing valid yet       */
	{
		DEVICE_OFF();
		deviceState = 0;
		return;
	}

	now = (u32)hour*3600 + (u32)min*60 + (u32)sec;
	on  = sched.onHour*3600  + sched.onMin*60;
	off = sched.offHour*3600 + sched.offMin*60;

	if(on < off)
		want = (now >= on && now < off);            /* same day       */
	else
		want = (now >= on || now < off);            /* across midnight*/

	if(want)
		DEVICE_ON();
	else
		DEVICE_OFF();

	deviceState = want;
}

/*====================================================================
	Normal run screen
	Line 1 : HH:MM:SS DDD ON / OF
	Line 2 : alternates between the date and the programmed schedule
====================================================================*/
u32 refreshAll = 1;                 /* forces one full redraw */

static void ShowStatus(void)
{
	static s32 pSec = -1, pState = -1, pPage = -1;
	s32 page = (sec/3) % 2;

	/* line 1 : rewritten only when the second or the device state changes */
	if(sec != pSec || (s32)deviceState != pState || refreshAll)
	{
		WRITE_LCD_CMD(GOTO_LINE1_POS0);
		Print2(hour);  WRITE_LCD_DATA(':');
		Print2(min);   WRITE_LCD_DATA(':');
		Print2(sec);   WRITE_LCD_DATA(' ');
		Str((const char*)week[(day<0||day>6) ? 0 : day]);
		WRITE_LCD_DATA(' ');
		Str(deviceState ? "ON " : "OFF");
		pSec   = sec;
		pState = (s32)deviceState;
	}

	/* line 2 : rewritten only when the page flips, every 3 seconds       */
	if(page != pPage || refreshAll)
	{
		WRITE_LCD_CMD(GOTO_LINE2_POS0);

		if(page)                                  /* schedule page        */
		{
			if(sched.valid)
			{
				Str("ON");  Print2(sched.onHour);  WRITE_LCD_DATA(':');
				Print2(sched.onMin);
				Str(" OFF"); Print2(sched.offHour); WRITE_LCD_DATA(':');
				Print2(sched.offMin);
			}
			else
				Str("SCHEDULE NOT SET");
		}
		else                                      /* calendar page        */
		{
			Print2(date);  WRITE_LCD_DATA('/');
			Print2(month); WRITE_LCD_DATA('/');
			U32LCD(year);
			Str("      ");
		}
		pPage = page;
	}

	refreshAll = 0;
}

/*====================================================================
	main
====================================================================*/
int main(void)
{
	/* ---------------- initialisation ------------------------------*/
	SETBIT(IODIR0, DEVICE_PIN);         /* device pin as output       */
	DEVICE_OFF();                       /* device stays OFF at start  */

	Init_LCD();
	InitKPM();
	InitTimer0();
	RTC_Init();
	EINT0_Enable();

	ShowBanner();                       /* ID then project title      */

	/* recover the schedule written to the last flash sector by IAP  */
	if(IAP_LoadSchedule(&sched) == 0)
	{
		sched.valid   = 0;
		sched.onHour  = 0;  sched.onMin  = 0;
		sched.offHour = 0;  sched.offMin = 0;
	}

	/* if the RTC has never been set, start from a known calendar    */
	if(!IsRTCValid())
	{
		SetRTCTimeInfo(0, 0, 0);
		SetRTCDateInfo(1, 1, 2026);
		SetRTCDay(THU);
		Message("RTC NOT SET     ", "PRESS SW TO SET ", 900);
	}

	WRITE_LCD_CMD(CLEAR_LCD);

	/* ---------------- main loop -----------------------------------*/
	while(1)
	{
		GetRTCTimeInfo(&hour, &min, &sec);
		GetRTCDateInfo(&date, &month, &year);
		GetRTCDay(&day);

		UpdateDevice();
		ShowStatus();

		if(menuRequest)                 /* set by the EINT0 handler   */
		{
			menuRequest = 0;
			ShowMenu();                 /* keypad work done here      */
			WRITE_LCD_CMD(CLEAR_LCD);
			refreshAll  = 1;            /* redraw the whole screen    */
			menuRequest = 0;            /* ignore switch bounce       */
		}
	}
}
