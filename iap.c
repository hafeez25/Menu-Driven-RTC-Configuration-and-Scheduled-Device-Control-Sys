//iap.c
#include <lpc21xx.h>
#include "types.h"
#include "iap.h"

#define IAP_LOCATION        0x7FFFFFF1UL   /* thumb entry point of the bootloader */
#define IAP_PREPARE         50
#define IAP_COPY_RAM2FLASH  51
#define IAP_ERASE           52
#define IAP_BLANK_CHECK     53

#define CCLK_KHZ            60000          /* 60 MHz */
#define IAP_BLOCK           256            /* smallest programmable block */

typedef void (*IAP_FN)(u32[], u32[]);

#if (USE_IAP == 1)
static IAP_FN iap_entry = (IAP_FN)IAP_LOCATION;

static u32 command[5];
static u32 result[5];

/* the source buffer for command 51 must be word aligned */
static u32 iap_buf[IAP_BLOCK/4];
#endif

static u32 CheckSum(SCHEDULE *s)
{
	return (s->magic + s->onHour + s->onMin + s->offHour + s->offMin + s->valid);
}

u32 IAP_SaveSchedule(SCHEDULE *s)
{
#if (USE_IAP == 0)
	s->magic = CFG_MAGIC;
	s->csum  = CheckSum(s);
	return 1;                    /* flash writing disabled */
#else
	u32 i;
	u32 *src;
	u32 vic_save;

	s->magic = CFG_MAGIC;
	s->csum  = CheckSum(s);

	/* fill the 256 byte page : record first, 0xFF padding after */
	for(i=0; i<IAP_BLOCK/4; i++)
		iap_buf[i] = 0xFFFFFFFFUL;
	src = (u32*)s;
	for(i=0; i<sizeof(SCHEDULE)/4; i++)
		iap_buf[i] = src[i];

	/* flash operations must not be interrupted */
	vic_save    = VICIntEnable;
	VICIntEnClr = 0xFFFFFFFFUL;

	/* 1. prepare the sector for erase */
	command[0] = IAP_PREPARE;
	command[1] = CFG_SECTOR;
	command[2] = CFG_SECTOR;
	iap_entry(command, result);
	if(result[0]!=0) { VICIntEnable = vic_save; return result[0]; }

	/* 2. erase the sector */
	command[0] = IAP_ERASE;
	command[1] = CFG_SECTOR;
	command[2] = CFG_SECTOR;
	command[3] = CCLK_KHZ;
	iap_entry(command, result);
	if(result[0]!=0) { VICIntEnable = vic_save; return result[0]; }

	/* 3. prepare again for the write */
	command[0] = IAP_PREPARE;
	command[1] = CFG_SECTOR;
	command[2] = CFG_SECTOR;
	iap_entry(command, result);
	if(result[0]!=0) { VICIntEnable = vic_save; return result[0]; }

	/* 4. copy RAM buffer to flash */
	command[0] = IAP_COPY_RAM2FLASH;
	command[1] = CFG_ADDR;
	command[2] = (u32)iap_buf;
	command[3] = IAP_BLOCK;
	command[4] = CCLK_KHZ;
	iap_entry(command, result);

	VICIntEnable = vic_save;
	return result[0];
#endif
}

u32 IAP_LoadSchedule(SCHEDULE *s)
{
#if (USE_IAP == 0)
	(void)s;
	return 0;
#else
	SCHEDULE *f = (SCHEDULE*)CFG_ADDR;   /* flash is directly readable */
	u32 i;
	u32 *d = (u32*)s;
	u32 *p = (u32*)f;

	if(f->magic != CFG_MAGIC)
		return 0;

	for(i=0; i<sizeof(SCHEDULE)/4; i++)
		d[i] = p[i];

	if(s->csum != CheckSum(s))
		return 0;
	if(s->onHour>23 || s->offHour>23 || s->onMin>59 || s->offMin>59)
		return 0;
	if(s->onHour==s->offHour && s->onMin==s->offMin)
		return 0;

	return 1;
#endif
}
