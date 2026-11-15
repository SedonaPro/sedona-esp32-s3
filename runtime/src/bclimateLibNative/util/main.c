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

#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "strptime.h"
#include "bclimateLibNative_License_common.h"


void help(void)
{
	printf("Usage: \n");
	printf(" license_generator LICENSE_REQUEST_CODE [date]\n");
	printf("\n");
	printf("date should be in format YYYY-MM-DD\n");
	exit(-1);
}

int main(int argc, char **argv)
{
	if (argc<2)
		help();

	char *req_str = argv[1];

	// check date
	struct tm date;
	struct tm *pdate = NULL;
	if (argc > 2) {
		char *date_str = argv[2];
		if (!strptime(date_str, "%Y-%m-%d", &date)) {
			printf("failed to parse date\n");
			printf("");
			help();
		}
		pdate = &date;
	}
	struct LicenseRequestCodeRaw req;
	if (LicenseRequestCodeRawFromStr(req_str, &req))
		return -1;
	printf("Request information:\n");
	PrintLicenseRequestCodeRaw(&req);

	struct License lic;
	if (GetLicenseCode(&req, &lic, pdate))
		return -1;

	char lic_str[LICENSE_CODE_STRLEN];
	if (LicenseCodeToStr(&lic, lic_str, sizeof(lic_str)))
		return -1;
	printf("\n");
	printf("License: %s\n", lic_str);

	struct tm valid_date;
	int ret = CheckLicenseString(req_str, lic_str, &valid_date);
	printf("valid: ");
	PrintValidDate(&valid_date);
	printf("\n");
	if (ret)
		printf("License isn't valid now\n");
	return ret;
}
