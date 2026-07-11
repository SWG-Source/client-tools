// ======================================================================
//
// PlatformGlue.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PlatformGlue_H
#define INCLUDED_PlatformGlue_H

// ======================================================================

#include <time.h>

// ======================================================================

char *      strsep(char **string, const char *delim);
#if defined(_MSC_VER) && _MSC_VER < 1900   // VS2015+
int snprintf(char* buffer, size_t count, const char* format, ...);
#endif
struct tm * gmtime_r(const time_t *timep, struct tm *result);
int         finite(double value);

//Format specifier for non-portable printf
#define UINT64_FORMAT_SPECIFIER "%I64u"
#define INT64_FORMAT_SPECIFIER "%lld"

//Constant definition macro for 64 bit values
#define UINT64_LITERAL(a) a ## ui64
#define INT64_LITERAL(a) a ## i64

// ======================================================================

#endif
