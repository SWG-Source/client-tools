#ifndef INCLUDED_FirstUiBuilder_H
#define INCLUDED_FirstUiBuilder_H

#include "_precompile.h"

#include <afxres.h>
#include <assert.h>
#include <windows.h>

typedef signed char    int8;
typedef signed short   int16;
typedef signed int     int32;
typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

typedef uint8          byte;
typedef unsigned int   uint;
typedef float          real;

#define isizeof(a)        static_cast<int>(sizeof(a))
#define istrlen(a)        static_cast<int>(strlen(a))
#define UNREF(a)          static_cast<void>(a)
#define FATAL(a, b)       assert(!(a))
#define DEBUG_FATAL(a, b) FATAL(a, b)
#define NOT_NULL(a)       assert((a) != NULL)
#define MEM_OWN(a)        (a)
#define NOP               static_cast<void>(0)

inline char *DuplicateString(const char *string)
{
	if (!string)
		return NULL;

	const int length = strlen(string) + 1;
	char *result = new char[length];
	strcpy(result, string);
	return result;
}

//#pragma warning(disable: 4702)

#endif
