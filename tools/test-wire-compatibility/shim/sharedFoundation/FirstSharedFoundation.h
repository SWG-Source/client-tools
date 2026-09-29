// Test shim: reproduces MSVC FoundationTypesWin32.h exactly (uint32 = unsigned long,
// int64 = __int64) without the VC2013-era STL/FloatMath headers. No serializer is replaced.
#ifndef INCLUDED_FirstSharedFoundation_H
#define INCLUDED_FirstSharedFoundation_H
#define INCLUDED_FoundationTypesWin32_H
#define INCLUDED_FirstSharedFoundation_H_SHIM
#define DEBUG_LEVEL_RELEASE 0
#define DEBUG_LEVEL_OPTIMIZED 1
#define DEBUG_LEVEL_DEBUG 2
#define PLATFORM_WIN32
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <algorithm>
typedef unsigned char uint8; typedef unsigned short uint16; typedef unsigned long uint32; typedef unsigned __int64 uint64;
typedef signed char int8; typedef signed short int16; typedef signed long int32; typedef signed __int64 int64;
typedef float real;
#define CONST_REAL(a) static_cast<float>(a)
#define RECIP(a) (1.0f/(a))
#include <cfloat>
const float REAL_MIN = FLT_MIN; const float REAL_MAX = FLT_MAX;
#define PI 3.14159265358979323846f
#define PI_TIMES_2 (PI*2.0f)
#define PI_OVER_2 (PI*0.5f)
#define INLINE inline
#define DLLEXPORT
void Fatal(char const *format, ...);
#define FATAL(c,a) do{ if(c) Fatal a; }while(0)
#define DEBUG_FATAL(c,a) ((void)0)
#define WARNING(c,a) ((void)0)
#define DEBUG_WARNING(c,a) ((void)0)
#define WARNING_STRICT_FATAL(c,a) ((void)0)
#define DEBUG_REPORT_LOG(c,a) ((void)0)
#define REPORT_LOG(c,a) ((void)0)
#define NOT_NULL(p) ((void)0)
#define IGNORE_RETURN(a) ((void)(a))
#define UNREF(a) ((void)(a))
#define VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(a,b,c) ((void)0)
#define DEBUG_OP_ONLY(x)
#define NON_NULL(p) (p)
#define safe_cast static_cast
#define isdigit_unsafe isdigit
#include "sharedFoundation/StlForwardDeclaration.h"
#define UINT64_FORMAT_SPECIFIER "%I64u"
#define INT64_FORMAT_SPECIFIER "%I64i"
#endif
