#include <stdint.h>
#include <stdio.h>
#include "bclimateLibNativeCrc32.h"

//=============================================================================
/// don't modify: used in avr bootloader!
//=============================================================================
static uint32_t _crc32_update(uint32_t crc, uint8_t byte, uint32_t poly)
{
	uint_fast8_t i;
	crc ^= byte;

	for (i=0; i<8; i++)
	{
		if ((uint8_t)crc & (1U<<0))  crc = (crc>>1) ^ poly; // (uint32_t)(CRC32_POLY);
		else                         crc >>= 1;
	}

	return crc;
}

uint32_t crc = CRC32_INIT;

static uint32_t _crc32_block_append(const void* src, size_t len, uint32_t crc, uint32_t poly)
{
	const uint8_t* p = (const uint8_t*)src;

	while (len--)
	{
		crc = _crc32_update(crc, *p++, poly);
	}

	return (crc);
}

static	uint32_t _crc32_block(const void* src, size_t len, uint32_t poly)
{
	return _crc32_block_append(src, len, CRC32_INIT, poly);
}

#ifdef CRC32_POLY
uint32_t crc32_update	   (uint32_t crc, uint8_t byte)		       {return _crc32_update	   (crc, byte,	   CRC32_POLY);}
uint32_t crc32_block	   (const void* src, size_t len)	       {return _crc32_block	   (src, len,	   CRC32_POLY);}
uint32_t crc32_block_append(const void* src, size_t len, uint32_t crc) {return _crc32_block_append (src, len, crc, CRC32_POLY);}
#endif
uint32_t crc32_poly_update	(uint32_t crc, uint8_t byte, uint32_t poly)		   {return _crc32_update      (crc, byte,     poly);}
uint32_t crc32_poly_block	(const void* src, size_t len, uint32_t poly)		   {return _crc32_block	      (src, len,      poly);}
uint32_t crc32_poly_block_append(const void* src, size_t len, uint32_t crc, uint32_t poly) {return _crc32_block_append(src, len, crc, poly);}
