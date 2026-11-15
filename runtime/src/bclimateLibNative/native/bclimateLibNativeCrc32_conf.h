#ifndef _CRC32_CONF_H_
#define _CRC32_CONF_H_ 1

#ifndef CRC32_POLY
	/* default crc32 poly */
	#define CRC32_POLY  (0x04C11DB7UL)
#endif

#define CRC32_INIT  0xffffffffUL

#endif
