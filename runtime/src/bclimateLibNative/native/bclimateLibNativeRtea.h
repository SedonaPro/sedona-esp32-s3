#ifndef _RTEA_H_
#define _RTEA_H_ 1

#ifdef __cplusplus
extern "C"{
#endif

#include "bclimateLibNativeRtea_conf.h"


	void rtea_key_decrypt(void* block64bits, const uint8_t key[8]);
	void rtea_key_crypt(void* block64bits, const uint8_t key[8]);

#ifdef RTEA_KEY
	void rtea_crypt  (void* block64bits);
	void rtea_decrypt(void* block64bits);
#endif


#ifdef __cplusplus
}
#endif

#endif
