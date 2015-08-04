// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef INCLUDED_FirstUiBuilder_H
#define INCLUDED_FirstUiBuilder_H

#pragma warning (disable:4786)

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <mmsystem.h>
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <mmreg.h>
#include <assert.h>

#include "resource.h"

#include "_precompile.h"

typedef unsigned long ULONG_PTR;
typedef ULONG_PTR DWORD_PTR;

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

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
