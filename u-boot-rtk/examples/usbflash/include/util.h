#ifndef UTIL_H
#define UTIL_H

#include "sysdefs.h"

#define REG8( addr )		  (*(volatile UINT8 *) (addr))
#define REG16( addr )		  (*(volatile UINT16 *)(addr))
#define REG32( addr )		  (*(volatile UINT32 *)(addr))
#define REG64( addr )		  (*(volatile UINT64 *)(addr))

#define FOR_ICE_LOAD                        //for ice load
#define UBOOT_DDR_OFFSET		0xa0000000	//for RTD299X

#ifdef FOR_ICE_LOAD
#define rtprintf(format,args...)
#endif

#define PHOENIX_REV_B   0x0001
#define IS_CHIP_REV_B() ((REG32(SB2_CHIP_INFO)>>16) == PHOENIX_REV_B)
#define GET_CHIP_REV() (REG32(SB2_CHIP_INFO)>>16)

#define REF_MALLOC_BASE						(0xA9000000 - UBOOT_DDR_OFFSET)

UINT32 min(UINT32 a, UINT32 b);
UINT32 max(UINT32 a, UINT32 b);

int	timer_init(void);
ulong	get_timer	   (ulong base);

#endif