/*
  Copyright (C) 2017 Andrey Skvortsov

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3, or (at your  option)
  any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "bclimateLibNative_License_common.h"

int TestLicenseCodeFunctions(char *req_str, struct tm *valid_until)
{
	printf("\n");
	
	struct LicenseRequestCodeRaw req;
	if (LicenseRequestCodeRawFromStr(req_str, &req)) {
		printf("LicenseRequestCodeRawFromStr() failed\n");
		return -1;
	}
	
	struct License lic;
	if (GetLicenseCode(&req, &lic, valid_until)) {
		printf("GetLicenseCode() failed\n");
		return -1;
	}

	char lic_str[LICENSE_CODE_STRLEN];
	if (LicenseCodeToStr(&lic, lic_str, sizeof(lic_str))) {
		printf("LicenseCodeToStr() failed\n");
		return -1;
	}
	printf("License: %s\n", lic_str);

	struct tm lic_date;
	int ret = CheckLicenseCode(&req, &lic, &lic_date);
	printf("Valid: "); PrintValidDate(&lic_date); printf("\n");
	if (ret) {
		printf("CheckLicenseCode() failed\n");
		return -1;
	}

	ret = CheckLicenseString(req_str, lic_str, &lic_date);
	printf("Valid: "); PrintValidDate(&lic_date); printf("\n");			
	if (ret) {
		printf("CheckLicenseString() failed\n");
		return -1;
	}
	printf("License is valid\n");
	return 0;
}

int TestRequestCodeFunctions(char *req_str, int len) {
	if (GetLicenseRequestString(req_str, len, FEATURE_ALL, NULL)) {
		printf("GetLicenseRequestString() failed\n");
		return -1;
	}
	printf("Request: %s\n", req_str);

	struct LicenseRequestCodeRaw req;
	if (LicenseRequestCodeRawFromStr(req_str, &req)) {
		printf("LicenseRequestCodeRawFromStr() failed\n");
		return -1;
	}
	PrintLicenseRequestCodeRaw(&req);
	return 0;
}

int main(void)
{
	char req_str[REQ_CODE_STRLEN];
	if (TestRequestCodeFunctions(req_str, sizeof(req_str)))
		return -1;

	printf("\n");
	/* check no valid date */	
	if (TestLicenseCodeFunctions(req_str, NULL))
		return -1;

	/* check empty valid date */
	struct tm valid_until;
	memset(&valid_until, 0, sizeof(valid_until));
	if (TestLicenseCodeFunctions(req_str, &valid_until))
		return -1;

	/* check valid date */
	valid_until.tm_year = 2027 - 1900;
	valid_until.tm_mon = 1 - 1;
	valid_until.tm_mday = 1;
	if (TestLicenseCodeFunctions(req_str, &valid_until))
		return -1;
	
	/* check valid date */
	valid_until.tm_year = 2017 - 1900;
	valid_until.tm_mon = 11 - 1;
	valid_until.tm_mday = 2;
	if (!TestLicenseCodeFunctions(req_str, &valid_until))
		return -1;

	printf("\n");
	printf("All checks are good\n");
	return 0;
}
