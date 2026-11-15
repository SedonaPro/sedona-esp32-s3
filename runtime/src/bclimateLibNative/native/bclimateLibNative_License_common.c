#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <assert.h>

#include "inet_sha1.h"
#include "bclimateLibNativeCrc32.h"
#include "bclimateLibNativeRtea.h"
#include "bclimateLibNative_License_common.h"


#define HOST_ID_BYTE_SIZE (HOST_ID_SIZE*4)
#define MAC_LEN 6
#define SYSFS_BASE "/sys/class/net"
#define SYSFS_MAC "address"
#define HOST_ID_VERSION 1
#define MERGE_MASK  0x0F0FF00F;

static const struct License valid_license = {
	.val = {
		.license = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7}
	}
};


static int GetMacAddr(char *name, uint8_t *mac)
{
	int ret = 0;
	memset(mac, 0, MAC_LEN);
	const int size = strlen(name) + strlen(SYSFS_BASE) + 3 + strlen(SYSFS_MAC);
	char fname[size];
	snprintf(fname, size, "%s/%s/%s", SYSFS_BASE, name, SYSFS_MAC);
	FILE *f;
	f = fopen(fname, "r");
	if (!f)
		return -1;

	uint8_t mac_addr[(MAC_LEN)*3];
	size_t size_read;
	size_read = fread(&mac_addr, 1, sizeof(mac_addr) - 1, f);
	if (size_read == (sizeof(mac_addr) -1)) {
		mac_addr[sizeof(mac_addr) -1] = 0;
		int nread;
		nread = sscanf(mac_addr, "%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX",
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
		if (nread != MAC_LEN)
			ret = -1;
	}
	if (ret)
		memset(mac, 0, MAC_LEN);
	fclose(f);
	return ret;
}

static int GetHostIdData(uint8_t **merged_macs, int* len)
{
	int i;
	DIR *sysfs;
	sysfs = opendir("/sys/class/net");
	if (!sysfs) {
		return -1;
	}
	*merged_macs = malloc(MAC_LEN);
	if (!*merged_macs)
		return -1;

	memset(*merged_macs, 0, MAC_LEN);
	for(;;) {
		struct dirent* iface;
		iface = readdir(sysfs);
		if (!iface)
			break;
		if (!strcmp(iface->d_name,".") || !strcmp(iface->d_name,".."))
			continue;

		uint8_t mac[MAC_LEN];
		if (!GetMacAddr(iface->d_name, mac)) {
			// merge mac address
			for(i = 0; i < MAC_LEN; i++)
				(*merged_macs)[i] ^=mac[i];
		}
	}
	closedir(sysfs);
	*len = MAC_LEN;
	return 0;
}

static int GetHostId(uint32_t *host_id)
{
	uint8_t *data;
	int len;
	memset(host_id, 0, HOST_ID_SIZE);
	if (GetHostIdData(&data, &len))
		return -1;

	// hash information with sha1
	SHA1Context cx;

	SHA1Reset(&cx);
	SHA1Input(&cx, data, len);
	SHA1Result(&cx, (uint8_t*)host_id);

	free(data);
	return 0;
}

static void PrintBinaryData(uint32_t *pdata, const int len)
{
	int i;
	const int max_i  = len/sizeof(uint32_t);

	assert(len%sizeof(uint32_t) == 0);

	for(i = 0; i < max_i;) {
		printf("%08X", pdata[i]);
		if (++i != max_i)
			printf("-");
	}
}

static int BinaryDataToStr(uint32_t *pdata, const int len, char *str, const int slen)
{
	int left = slen;
	int max_i = len/sizeof(uint32_t);

	assert(len%sizeof(uint32_t) == 0);

	if (slen < (max_i*9)) {
		printf("string buffer is too small to store the code\n");
		return -1;
	}

	memset(str, 0, slen);
	int i;
	for(i = 0; i < max_i; ++i) {
		int ret = snprintf(str, left, "%08X", pdata[i]);
		left -= ret;
		str += ret;
		if (i < (max_i-1)) {
			ret = snprintf(str, left, "-");
			left -= ret;
			str += ret;
		}
	}
	return 0;
}

static int BinaryDataFromStr(char *str, uint32_t *pdata, const int len) {
	const int num_ret = len/sizeof(uint32_t);
	int offset, i;
	int ret;
	for(i=0; i<num_ret; i++) {
		ret = sscanf(str, "%08X-%n", &pdata[i], &offset);
		if (ret != 1)
			break;
		str += offset;
	}
	if (i != num_ret) {
		memset(pdata, 0, len);
		return -1;
	}
	return 0;
}

int RequestCodeToString(struct LicenseRequestCode *request, char *str, int len)
{
	return BinaryDataToStr((uint32_t*)request, sizeof(*request), str, len);
}

int GetLicenseRequestString(char *str, int len, uint32_t feature, struct LicenseRequestCode *request)
{
	struct LicenseRequestCodeRaw r;
	struct LicenseRequestCode local_req;
	struct LicenseRequestCode *ret_req;
	ret_req = request;
	if (!ret_req)
		ret_req = &local_req;

	memset(ret_req, 0, sizeof(*ret_req));
	if (GetHostId(&r.data.host_id[0]))
		return -1;

	r.data.features = feature;
	r.data.version = HOST_ID_VERSION;
	r.checksum = crc32_block(&r.data, sizeof(r.data));

	// mix some fields
	memcpy(&ret_req->host_id, &r.data.host_id, sizeof(ret_req->host_id));
	const uint32_t mask = MERGE_MASK;
	ret_req->merged_data[0] = (r.checksum & mask) | (r.data.version & ~mask);
	ret_req->merged_data[1] = (r.checksum & ~mask) | (r.data.features & mask);
	ret_req->merged_data[2] = ((r.data.features & ~mask) | (r.data.version & mask)) ^ r.checksum;

	return RequestCodeToString(ret_req, str, len);
}

int LicenseRequestCodeRawFromStr(char *str, struct LicenseRequestCodeRaw *request)
{
	struct LicenseRequestCode req;
	memset(request, 0, sizeof(*request));
	if (BinaryDataFromStr(str, (uint32_t*)&req, sizeof(req)))
		return -1;

	const uint32_t mask = MERGE_MASK;
	memcpy(request->data.host_id, req.host_id, sizeof(request->data.host_id));
	request->checksum      = (req.merged_data[1] & ~mask) | (req.merged_data[0] & mask);
	request->data.version  = (req.merged_data[0] & ~mask) | ((req.merged_data[2] ^ request->checksum) & mask);
	request->data.features = ((req.merged_data[2] ^ request->checksum) & ~mask) | (req.merged_data[1] &  mask);

	// check checksum
	uint32_t crc = crc32_block(&request->data, sizeof(request->data));
	if (crc != request->checksum) {
		memset(request, 0, sizeof(*request));
		return -1;
	}
	return 0;
}

int PrintLicenseRequestCodeRaw(struct LicenseRequestCodeRaw *request)
{
	int i;
	printf("HOST_ID:  ");
	PrintBinaryData(request->data.host_id, sizeof(request->data.host_id));
	printf("\n");
	printf("VERSION:  %d\n", request->data.version);
	printf("FEATURES: 0x%08X\n", request->data.features);
	printf("CHECKSUM: 0x%08X\n", request->checksum);
}

static int ApplyLicenseRequestCode(struct LicenseRequestCodeRaw *request, struct License *lic)
{
	int i, j;
	uint32_t *data = (uint32_t*)request;
	const int max_i = sizeof(*lic)/sizeof(uint32_t);
	const int max_j = sizeof(*request)/sizeof(uint32_t);

	for(i = 0, j = 0; i < max_i; i++) {
		lic->data[i] ^= data[j];
		if (++j >= max_j)
			j=0;
	}
	return 0;
}

int GetLicenseCode(struct LicenseRequestCodeRaw *request, struct License *lic, struct tm *valid_until)
{
	memset(lic, 0, sizeof(*lic));
	uint32_t crc = crc32_block(&request->data, sizeof(request->data));
	if (crc != request->checksum)
		return -1;

	memset(lic, 0, sizeof(*lic));
	memcpy(lic, &valid_license, sizeof(valid_license));

	lic->val.version = LICENSE_VERSION;

	if (valid_until &&
            ((valid_until->tm_year != 0) || (valid_until->tm_mon != 0) || (valid_until->tm_mday != 0))) {
		lic->val.valid_until.year  = valid_until->tm_year + 1900;
		lic->val.valid_until.month = valid_until->tm_mon + 1;
		lic->val.valid_until.day   = valid_until->tm_mday;
	}

	// some random value
	static bool inited_rseed;
	if (!inited_rseed)
		srand(time(NULL));
	lic->val.unused = (rand() & 0xFFFF) << 16 | (rand() & 0xFFFF);

	// add checksum
	lic->val.checksum = crc32_block(lic, sizeof(*lic) - sizeof(lic->val.checksum));

	// merge with license request code
	ApplyLicenseRequestCode(request, lic);

	// encrypt license
	assert(sizeof(*lic)%sizeof(uint64_t) == 0);
	int num_blocks = sizeof(*lic)/sizeof(uint64_t);
	uint64_t *data = (uint64_t*)lic;
	int i;
	for(i = 0; i < num_blocks; i++)
		rtea_crypt(&data[i]);
	return 0;
}

int LicenseCodeToStr(struct License *lic, char *str, int len)
{
	return BinaryDataToStr(lic->data, sizeof(*lic), str, len);
}

int LicenseCodeFromStr(char *str, struct License *lic)
{
	return BinaryDataFromStr(str, lic->data, sizeof(*lic));
}


int CheckLicenseCode(struct LicenseRequestCodeRaw *request, struct License *lic_in, struct tm *valid_until)
{
	bool valid = true;
	struct License lic = *lic_in;
	// decrypt license
	assert(sizeof(lic)%sizeof(uint64_t) == 0);
	int num_blocks = sizeof(lic)/sizeof(uint64_t);
	uint64_t *data = (uint64_t*)&lic;
	int i;
	for(i = 0; i < num_blocks; i++)
		rtea_decrypt(&data[i]);

	// merge with license request code
	ApplyLicenseRequestCode(request, &lic);

	// check whether this license version is supported
	if (lic.val.version > LICENSE_VERSION)
		valid = false;

	// check valid license code
	if (memcmp(&lic, &valid_license.val.license, sizeof(valid_license.val.license)))
		valid = false;

	// check license checksum
	uint32_t crc = crc32_block(&lic, sizeof(lic) - sizeof(lic.val.checksum));
	if (crc != lic.val.checksum)
		valid = false;

	// save valid date
	struct tm valid_date;

	memset(&valid_date, 0, sizeof(valid_date));
	if ((lic.val.valid_until.year != 0) || (lic.val.valid_until.month != 0) || (lic.val.valid_until.day != 0)) {
		valid_date.tm_year = lic.val.valid_until.year - 1900;
		valid_date.tm_mon  = lic.val.valid_until.month  - 1;
		valid_date.tm_mday = lic.val.valid_until.day;

		time_t valid_time_t;
		time_t today = time(NULL);
		valid_time_t = mktime(&valid_date);
		if (valid_time_t < today) {
			valid = false;
		}
	}
	if (valid_until)
		*valid_until = valid_date;

	return valid ? 0 : 1;
}

int CheckLicenseString(char* req_str, char* lic_str, struct tm *valid_until)
{
	struct License lic;
	struct LicenseRequestCodeRaw req;
	if (LicenseCodeFromStr(lic_str, &lic))
		return -1;
	if (LicenseRequestCodeRawFromStr(req_str, &req))
		return -1;
	return CheckLicenseCode(&req, &lic, valid_until);
}


void PrintValidDate(struct tm* date)
{
	if ((date->tm_year == 0) && (date->tm_mon == 0) && (date->tm_mday == 0))
		printf("forever");
	else
		printf("until %d-%02d-%02d", 1900 + date->tm_year, 1 + date->tm_mon, date->tm_mday);
}
