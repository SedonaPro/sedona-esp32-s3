#include <stdint.h>
#include "bclimateLibNativeRtea.h"


//=============================================================================
static inline void _rtea_key_decrypt(void* block64bits, const uint8_t key[8])
{
	uint32_t* p = (uint32_t*)block64bits;
	uint32_t a = p[0];
	uint32_t b = p[1];
	uint_fast8_t r;

	for (r=0; r<64; r++)
	{
		b += a + ((a << 6) ^ (a >> 8)) + (key[r % 8] + r);
		r++;
		a += b + ((b << 6) ^ (b >> 8)) + (key[r % 8] + r);
	}

	p[0] = a;
	p[1] = b;
}


static inline void _rtea_key_crypt(void* block64bits, const uint8_t key[8])
{
	uint32_t*    p = (uint32_t*)block64bits;
	uint32_t     a = p[0];
	uint32_t     b = p[1];
	uint_fast8_t r = 64;

	do
	{
		r--;
		a -= b + ((b << 6) ^ (b >> 8)) + (key[r % 8] + r);
		r--;
		b -= a + ((a << 6) ^ (a >> 8)) + (key[r % 8] + r);
	}
	while (r);

	p[0] = a;
	p[1] = b;
}



void rtea_key_crypt(void* block64bits, const uint8_t key[8])
{
	_rtea_key_crypt(block64bits, key);
}

void rtea_key_decrypt(void* block64bits, const uint8_t key[8])
{
	_rtea_key_decrypt(block64bits, key);
}




#ifdef RTEA_KEY

static const uint8_t rtea_key[8] = {RTEA_KEY};

void rtea_crypt(void* block64bits)
{
	_rtea_key_crypt(block64bits, rtea_key);
}

void rtea_decrypt(void* block64bits)
{
	_rtea_key_decrypt(block64bits, rtea_key);
}

#endif
