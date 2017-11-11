// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#define LOCTOOL_UNREF(a) static_cast<void>(a)

#pragma warning(disable: 4702)

#if WIN32
#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define NOMINMAX

#include <windows.h>
#include "Unicode.h"

void * __cdecl operator new(size_t size);
void * __cdecl operator new[](size_t size);

void operator delete(void *pointer);
void operator delete[](void *pointer);

#include "fileInterface/FirstFileInterface.h"

#define LOC_UNREF(a) (static_cast<void>(a))

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)

#endif

