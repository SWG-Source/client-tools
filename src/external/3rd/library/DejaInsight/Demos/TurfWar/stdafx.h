//==============================================================================
//
//  stdafx.h
//
//  Include file for standard system include files, or project specific include
//  files that are used frequently but change infrequently.
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef STDAFX_H
#define STDAFX_H

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
#ifdef WIN32
//------------------------------------------------------------------------------

// Modify the following defines if you have to target a platform prior to the 
// ones specified below.  Refer to MSDN for the latest info on corresponding 
// values for different platforms.

// Allow use of features specific to Windows XP or later.  Change this to the 
// appropriate value to target other versions of Windows.
#ifndef WINVER				
#define WINVER 0x0501		
#endif

// Allow use of features specific to Windows XP or later.  Change this to the 
// appropriate value to target other versions of Windows.
#ifndef _WIN32_WINNT		
#define _WIN32_WINNT 0x0501	
#endif						

// Allow use of features specific to Windows 98 or later.  Change this to the 
// appropriate value to target Windows Me or later.
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410 
#endif

// Allow use of features specific to IE 6.0 or later.  Change this to the 
// appropriate value to target other versions of IE.
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

//==============================================================================
//  INCLUDES - Standard
//==============================================================================

// Exclude rarely-used stuff from Windows headers.
#define WIN32_LEAN_AND_MEAN		

// And now...  Windows!
#include <windows.h>

// Standard C run-time header files.
#include <malloc.h>
#include <tchar.h>

//------------------------------------------------------------------------------
#endif // ifdef WIN32
//==============================================================================

#ifdef _XBOX
#include <xtl.h>
#include <xboxmath.h>
#endif // _XBOX

#include <stdlib.h>
#include <memory.h>

#define _USE_MATH_DEFINES
#include <math.h>

//==============================================================================
//  INCLUDES - Application specific
//==============================================================================

// Bring in the TinyEngine mem_mgr system.
#include "../TinyEngine/mem_mgr.h"

//==============================================================================
#endif // ifndef STDAFX_H
//==============================================================================
