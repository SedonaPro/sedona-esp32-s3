#ifndef _BCLIMATELIBNATIVE_LICENSE_COMMON_H_
#define _BCLIMATELIBNATIVE_LICENSE_COMMON_H_

#include <stdint.h>
#include <time.h>

// 160 bits in SHA1 hash
#define HOST_ID_SIZE (160/32)
#define LICENSE_VERSION 0


#define FEATURE_ALL 0xFFFFFFFF


struct LicenseRequestCodeData {
	uint32_t host_id[HOST_ID_SIZE];
	uint32_t version;
	uint32_t features;
};

struct LicenseRequestCodeRaw{
	struct LicenseRequestCodeData data;
	uint32_t checksum;
};

struct LicenseCodeData {
	uint32_t license[sizeof(struct LicenseRequestCodeRaw)/sizeof(uint32_t)];
	uint32_t version;
	struct {
		uint16_t  year;
		uint8_t  month;
		uint8_t  day;
	} valid_until;	// size of structure is uint32_t
	uint32_t unused;
	uint32_t checksum;
};

struct License {
	union {
		uint32_t data[sizeof(struct LicenseCodeData)/sizeof(uint32_t)];
		struct LicenseCodeData val;
	};
};

#define LICENSE_CODE_STRLEN ((sizeof(struct License)/sizeof(uint32_t))*9)
#define REQ_CODE_STRLEN ((sizeof(struct LicenseRequestCodeRaw)/sizeof(uint32_t))*9)
#define MERGED_DATA_SIZE (sizeof(struct LicenseRequestCodeRaw)/sizeof(uint32_t) - HOST_ID_SIZE)

struct LicenseRequestCode {
	uint32_t host_id[HOST_ID_SIZE];
	uint32_t merged_data[MERGED_DATA_SIZE];
};


int  RequestCodeToString(struct LicenseRequestCode *request, char *str, int len);
int  GetLicenseRequestString(char *str, int len, uint32_t feature, struct LicenseRequestCode *request);
int  LicenseRequestCodeRawFromStr(char *str, struct LicenseRequestCodeRaw *request);
int  PrintLicenseRequestCodeRaw(struct LicenseRequestCodeRaw *request);
int  GetLicenseCode(struct LicenseRequestCodeRaw *request, struct License *lic, struct tm *valid_until);

int  LicenseCodeToStr(struct License *lic, char *str, int len);
int  LicenseCodeFromStr(char *str, struct License *lic);
int  CheckLicenseCode(struct LicenseRequestCodeRaw *request, struct License *lic_in, struct tm *valid_until);
int  CheckLicenseString(char* req_str, char* lic_str, struct tm *valid_until);
void PrintValidDate(struct tm* date);

#endif

