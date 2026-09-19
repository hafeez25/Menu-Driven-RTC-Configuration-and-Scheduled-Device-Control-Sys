//KPM.h
#ifndef _KPM_H_
#define _KPM_H_
/* types.h is included by the .c file before this header */

/* keypad key codes returned by the scan routines */
#define KEY_NONE   0      /* returned only on timeout */
#define KEY_ENTER  '='
#define KEY_CLEAR  'C'

void InitKPM(void);
u32  ColScan(void);
u32  RowCheck(void);
u32  ColCheck(void);
u32  KeyScan(void);                 /* blocking scan (original)          */
u32  KeyScanT(u32 timeout_ms);      /* scan with timeout, 0 => timed out */
u32  ReadNum(void);

#endif
