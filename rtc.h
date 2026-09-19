//rtc.h
#ifndef _RTC_H_
#define _RTC_H_
/* types.h is included by the .c file before this header */

#define SUN 0
#define MON 1
#define TUE 2
#define WED 3
#define THU 4
#define FRI 5
#define SAT 6

extern const u8 week[7][4];

void RTC_Init(void);

void SetRTCTimeInfo(u32 hour, u32 minute, u32 second);
void GetRTCTimeInfo(s32 *hour, s32 *minute, s32 *second);

void SetRTCDateInfo(u32 date, u32 month, u32 year);
void GetRTCDateInfo(s32 *date, s32 *month, s32 *year);

void SetRTCDay(u32 dow);
void GetRTCDay(s32 *dow);

u32  IsLeapYear(u32 year);
u32  DaysInMonth(u32 month, u32 year);
u32  IsRTCValid(void);

#endif
