#ifndef _STRPTIME_WIN32_H_
#define _STRPTIME_WIN32_H_

#include <time.h>

#ifdef _WIN32
char *strptime(const char *s, const char *format, struct tm *tm);
#endif

#endif
