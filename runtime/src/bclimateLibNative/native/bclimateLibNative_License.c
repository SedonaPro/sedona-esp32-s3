#line 1 "../src/bclimateLibNative/native/bclimateLibNative_License.c"
//
// Copyright 2016 Andrey Skvortsov <andrej.skvortzov@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "sedona.h"
#include "bclimateLibNative_License_common.h"
#include <stdbool.h>

uint32_t licensed_features;

Cell bclimateLibNative_BclimateLicense_GetRequestString(SedonaVM* vm, Cell* params)
{
	Cell ret_cell = trueCell;

	uint32_t feature = params[0].ival;

	// sizeof(int) for array length of unsized array
	char *buf     = params[1].aval + sizeof(int);
	int   buf_len = params[2].ival;

	if (GetLicenseRequestString(buf, buf_len, feature, NULL))
		ret_cell = falseCell;
	return ret_cell;
}

Cell bclimateLibNative_BclimateLicense_IsLicenseAvailable(SedonaVM* vm, Cell* params)
{
	uint32_t feature = params[0].ival;
	bool licensed;
	licensed = (licensed_features & feature) == feature;
	return licensed ? trueCell : falseCell;
}

Cell bclimateLibNative_BclimateLicense_SubmitLicenseKey(SedonaVM* vm, Cell* params)
{
	int ret;
	Cell ret_cell;

	// sizeof(int) for array length of unsized array
	char *req = params[0].aval + sizeof(int);
	char *lic = params[1].aval + sizeof(int);
	uint32_t feature = params[2].ival;

	char req_str[REQ_CODE_STRLEN];
	if (GetLicenseRequestString(req_str, sizeof(req_str), feature, NULL))
		return falseCell;
	ret = CheckLicenseString(req_str, lic, NULL);

	licensed_features &= feature;
	if (!ret)
		licensed_features |= feature;
	return ret ? falseCell : trueCell;
}
