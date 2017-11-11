// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers


#include <windows.h>

#ifndef LCDUITRACE
    // .NET compiler uses __noop intrinsic
    #if _MSC_VER > 1300
        #define LCDUITRACE __noop
    #else
        #define LCDUITRACE (void)0
    #endif
#endif

#ifndef LCDUIASSERT
    // .NET compiler uses __noop intrinsic
    #if _MSC_VER > 1300
        #define LCDUIASSERT __noop
    #else
        #define LCDUIASSERT (void)0
    #endif
#endif



// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
