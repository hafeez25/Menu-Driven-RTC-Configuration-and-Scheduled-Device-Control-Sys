//iap.h  -- In Application Programming : store the schedule in on chip flash
#ifndef _IAP_H_
#define _IAP_H_
/* types.h is included by the .c file before this header */

/* -------------------------------------------------------------------
   Flash map for a 256 KB device (matches IROM1 Size = 0x40000 in your
   Target options):
       sector  0 - 7   : 4 KB   0x00000000 - 0x00007FFF
       sector  8 - 14  : 32 KB  0x00008000 - 0x0003FFFF
   The application binary starts at 0x00000000, so the LAST user sector
   (sector 14, the final 32 KB block) is used for the non volatile
   settings. Keep the compiled code size well below 0x00038000 so the
   two areas never overlap. If your chip actually has 512 KB flash,
   using this 256 KB-safe sector still works fine, it just doesn't use
   the very last physical sector.
-------------------------------------------------------------------*/
/* USE_IAP = 0 (default): the schedule is kept in RAM. Saving is instant
   and can never hang the board. It resets to blank on power-off.

   USE_IAP = 1: the schedule is written to flash with IAP, so it
   survives a reset -- this is what the project brief asks for. Make
   sure CFG_SECTOR/CFG_ADDR above actually exist on your chip's real
   flash size before enabling this -- writing to a sector beyond the
   end of physical flash causes a Data Abort, and this startup file's
   DAbt_Handler is just "B DAbt_Handler" (an infinite loop), which is
   exactly what a mismatched sector looks like: the board freezes right
   after the schedule is confirmed.                                     */
#define USE_IAP      0

#define CFG_SECTOR   14
#define CFG_ADDR     0x00038000UL

typedef struct
{
	u32 magic;      /* 0xA5A55A5A when a valid record is present */
	u32 onHour;
	u32 onMin;
	u32 offHour;
	u32 offMin;
	u32 valid;      /* 1 = schedule configured by the user       */
	u32 csum;       /* simple additive checksum                  */
}SCHEDULE;

#define CFG_MAGIC 0xA5A55A5AUL

u32  IAP_SaveSchedule(SCHEDULE *s);   /* 0 = success */
u32  IAP_LoadSchedule(SCHEDULE *s);   /* 1 = a valid record was found */

#endif
