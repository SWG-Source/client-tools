/****************************************************************************
** $Id: qt_windows.h 1964 2006-12-15 12:14:12Z chehrlic $
**
** Windows Specific headers
**
** Copyright (C) 2002 Wolfpack Emu.  All rights reserved.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
**********************************************************************/

#if !defined ( __QTWINDOWS_H__ )
#define __QTWINDOWS_H__

// Some ..A/..W functions are available
// only in win2k and above.
// include qt_windows.h first!
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef Q_CYGWIN_WIN
#define WSAGETSELECTEVENT(l)	LOWORD(l)
#define FD_READ_BIT      0
#define FD_READ          (1 << FD_READ_BIT)
#define FD_WRITE_BIT     1
#define FD_WRITE         (1 << FD_WRITE_BIT)
#define FD_OOB_BIT       2
#define FD_OOB           (1 << FD_OOB_BIT)
#define FD_ACCEPT_BIT    3
#define FD_ACCEPT        (1 << FD_ACCEPT_BIT)
#define FD_CONNECT_BIT   4
#define FD_CONNECT       (1 << FD_CONNECT_BIT)
#define FD_CLOSE_BIT     5
#define FD_CLOSE         (1 << FD_CLOSE_BIT)
typedef unsigned int	SOCKET;
extern "C" int PASCAL WSAAsyncSelect(SOCKET,HWND,unsigned int,long);
#endif 

#include "qnamespace.h"

#ifndef QT_SOCKLEN_T
#define QT_SOCKLEN_T int FAR
#endif

void qlasterror( CHAR *msg, DWORD dwLastError, bool showWindow = false );

#if !defined(SPI_GETKEYBOARDCUES)
#define SPI_GETKEYBOARDCUES 0x100A
#endif

// msvc 6.0 lacks some symbole
#if defined(Q_CC_MSVC) || defined(Q_CC_BOR)
#include <winuser.h>
#endif

#ifndef WM_MOUSEWHEEL
# define WM_MOUSEWHEEL 522
#endif

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED           0x00080000
#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002
#endif

#ifndef AC_SRC_OVER
#define AC_SRC_OVER                 0x00
#endif
#ifndef AC_SRC_ALPHA
#define AC_SRC_ALPHA                0x01
#endif

// already defined when compiled with WINVER >= 0x0500
// and we only use them in Qt::WV_2000 and Qt::WV_98
#ifndef SPI_SETMENUANIMATION
#define SPI_SETMENUANIMATION 0x1003
#endif
#ifndef SPI_SETMENUFADE
#define SPI_SETMENUFADE 0x1013
#endif
#ifndef SPI_SETCOMBOBOXANIMATION
#define SPI_SETCOMBOBOXANIMATION 0x1005
#endif
#ifndef SPI_SETTOOLTIPANIMATION
#define SPI_SETTOOLTIPANIMATION 0x1017
#endif
#ifndef SPI_SETTOOLTIPFADE
#define SPI_SETTOOLTIPFADE 0x1019
#endif
#ifndef SPI_SETUIEFFECTS
#define SPI_SETUIEFFECTS 0x103F
#endif
#ifndef SPI_GETMENUANIMATION
#define SPI_GETMENUANIMATION 0x1002
#endif
#ifndef SPI_GETMENUFADE
#define SPI_GETMENUFADE 0x1012
#endif
#ifndef SPI_GETCOMBOBOXANIMATION
#define SPI_GETCOMBOBOXANIMATION 0x1004
#endif
#ifndef SPI_GETTOOLTIPANIMATION
#define SPI_GETTOOLTIPANIMATION 0x1016
#endif
#ifndef SPI_GETTOOLTIPFADE
#define SPI_GETTOOLTIPFADE 0x1018
#endif
#ifndef SPI_GETUIEFFECTS
#define SPI_GETUIEFFECTS 0x103E
#endif
#ifndef SPI_GETKEYBOARDCUES
#define SPI_GETKEYBOARDCUES 0x100A
#endif
#ifndef IDC_HAND
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL        0x020A
#endif

#ifndef WM_MOUSEHOVER
# define WM_MOUSEHOVER     0x2A1
#endif
#ifndef WM_MOUSELEAVE
# define WM_MOUSELEAVE     0x2A3
#endif

#ifndef VK_OEM_PLUS
#define VK_OEM_PLUS	0xBB
#endif
#ifndef VK_OEM_COMMA
#define VK_OEM_COMMA	0xBC
#endif
#ifndef VK_OEM_MINUS
#define VK_OEM_MINUS	0xBD
#endif
#ifndef VK_OEM_PERIOD
#define VK_OEM_PERIOD	0xBE
#endif

#if _MSC_VER < 1300 || defined(Q_CC_BOR)
// missing function
# define __FUNCTION__     ""
# define SetWindowLongPtrA SetWindowLongA
# define SetWindowLongPtrW SetWindowLongW

#endif


#if defined(UNICODE)
#define qt_strncpy wcsncpy
#else
#define qt_strncpy strncpy
#endif

// CCJ - remove for beta release!
// CCJ - uncomment this define to enable backtrace generation on MSVC++.
//#define BACKTRACES

#ifdef BACKTRACES
/*
 * Backtrace Generator
 *
 * Copyright 2004 Eric Poech
 * Copyright 2004 Robert Shearman
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <winver.h>
#include <dbghelp.h>
#include <stdio.h>

void DPRINTF( const char *msg, ... )
{
    char buf[256];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
#if defined(QT_VSNPRINTF)
    QT_VSNPRINTF( buf, 256, msg, ap );
#else
    vsprintf( buf, msg, ap );
#endif
    va_end( ap );
//    QString fstr( buf );
//    OutputDebugString( fstr.ucs2() );
    OutputDebugStringA (buf);
}


//#define MAKE_FUNCPTR(f) static typeof(f) * p##f

//MAKE_FUNCPTR(StackWalk);
//MAKE_FUNCPTR(SymGetModuleBase);
//MAKE_FUNCPTR(SymFunctionTableAccess);
//MAKE_FUNCPTR(SymInitialize);
//MAKE_FUNCPTR(SymGetSymFromAddr);
//MAKE_FUNCPTR(SymGetModuleInfo);
static BOOL WINAPI (*pStackWalk)(
  DWORD MachineType,
  HANDLE hProcess,
  HANDLE hThread,
  LPSTACKFRAME StackFrame,
  PVOID ContextRecord,
  PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
  PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
  PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
  PTRANSLATE_ADDRESS_ROUTINE TranslateAddress
);
static DWORD WINAPI (*pSymGetModuleBase)(
  HANDLE hProcess,
  DWORD dwAddr
);
static PVOID WINAPI (*pSymFunctionTableAccess)(
  HANDLE hProcess,
  DWORD AddrBase
);
static BOOL WINAPI (*pSymInitialize)(
  HANDLE hProcess,
  PSTR UserSearchPath,
  BOOL fInvadeProcess
);
static BOOL WINAPI (*pSymGetSymFromAddr)(
  HANDLE hProcess,
  DWORD Address,
  PDWORD Displacement,
  PIMAGEHLP_SYMBOL Symbol
);
static BOOL WINAPI (*pSymGetModuleInfo)(
  HANDLE hProcess,
  DWORD dwAddr,
  PIMAGEHLP_MODULE ModuleInfo
);
static DWORD WINAPI (*pSymSetOptions)(
  DWORD SymOptions
);


static BOOL init_backtrace()
{
    HMODULE hmodDbgHelp = LoadLibraryA("dbghelp");
/*
    #define GETFUNC(x) \
    p##x = (typeof(x)*)GetProcAddress(hmodDbgHelp, #x); \
    if (!p##x) \
    { \
        return FALSE; \
    }
    */


//    GETFUNC(StackWalk);
//    GETFUNC(SymGetModuleBase);
//    GETFUNC(SymFunctionTableAccess);
//    GETFUNC(SymInitialize);
//    GETFUNC(SymGetSymFromAddr);
//    GETFUNC(SymGetModuleInfo);

#define FUNC(x) #x

      pStackWalk = (BOOL WINAPI (*)(
DWORD MachineType,
HANDLE hProcess,
HANDLE hThread,
LPSTACKFRAME StackFrame,
PVOID ContextRecord,
PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
PTRANSLATE_ADDRESS_ROUTINE TranslateAddress
))GetProcAddress (hmodDbgHelp, FUNC(StackWalk));
    pSymGetModuleBase=(DWORD WINAPI (*)(
  HANDLE hProcess,
  DWORD dwAddr
))GetProcAddress (hmodDbgHelp, FUNC(SymGetModuleBase));
    pSymFunctionTableAccess=(PVOID WINAPI (*)(
  HANDLE hProcess,
  DWORD AddrBase
))GetProcAddress (hmodDbgHelp, FUNC(SymFunctionTableAccess));
    pSymInitialize = (BOOL WINAPI (*)(
  HANDLE hProcess,
  PSTR UserSearchPath,
  BOOL fInvadeProcess
))GetProcAddress (hmodDbgHelp, FUNC(SymInitialize));
    pSymGetSymFromAddr = (BOOL WINAPI (*)(
  HANDLE hProcess,
  DWORD Address,
  PDWORD Displacement,
  PIMAGEHLP_SYMBOL Symbol
))GetProcAddress (hmodDbgHelp, FUNC(SymGetSymFromAddr));
    pSymGetModuleInfo = (BOOL WINAPI (*)(
  HANDLE hProcess,
  DWORD dwAddr,
  PIMAGEHLP_MODULE ModuleInfo
))GetProcAddress (hmodDbgHelp, FUNC(SymGetModuleInfo));
pSymSetOptions = (DWORD WINAPI (*)(
DWORD SymOptions
))GetProcAddress (hmodDbgHelp, FUNC(SymSetOptions));


    pSymSetOptions(SYMOPT_UNDNAME);

    pSymInitialize(GetCurrentProcess(), NULL, TRUE);

    return TRUE;
}

static void dump_backtrace_for_thread(HANDLE hThread)
{
    STACKFRAME sf;
    CONTEXT context;
    DWORD dwImageType;

    if (!pStackWalk)
        if (!init_backtrace())
            return;

    /* can't use this function for current thread as GetThreadContext
     * doesn't support getting context from current thread */
    if (hThread == GetCurrentThread())
        return;

    DPRINTF("Backtrace:\n");

    memset(&context, 0, sizeof(context));
    context.ContextFlags = CONTEXT_FULL;

    SuspendThread(hThread);

    if (!GetThreadContext(hThread, &context))
    {
        DPRINTF("Couldn't get thread context (error %ld)\n", GetLastError());
        ResumeThread(hThread);
        return;
    }

    memset(&sf, 0, sizeof(sf));

#ifdef __i386__
    sf.AddrFrame.Offset = context.Ebp;
    sf.AddrFrame.Mode = AddrModeFlat;
    sf.AddrPC.Offset = context.Eip;
    sf.AddrPC.Mode = AddrModeFlat;
    dwImageType = IMAGE_FILE_MACHINE_I386;
#else
# error You need to fill in the STACKFRAME structure for your architecture
#endif

    while (pStackWalk(dwImageType, GetCurrentProcess(),
                     hThread, &sf, &context, NULL, pSymFunctionTableAccess,
                     pSymGetModuleBase, NULL))
    {
        BYTE buffer[256];
        IMAGEHLP_SYMBOL * pSymbol = (IMAGEHLP_SYMBOL *)buffer;
        DWORD dwDisplacement;

        pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        pSymbol->MaxNameLength = sizeof(buffer) - sizeof(IMAGEHLP_SYMBOL) + 1;

        if (!pSymGetSymFromAddr(GetCurrentProcess(), sf.AddrPC.Offset,
                                &dwDisplacement, pSymbol))
        {
            IMAGEHLP_MODULE ModuleInfo;
            ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);

            if (!pSymGetModuleInfo(GetCurrentProcess(), sf.AddrPC.Offset,
                                   &ModuleInfo))
                DPRINTF("1\t%p\n", (void*)sf.AddrPC.Offset);
            else
                DPRINTF("2\t%s+0x%lx\n", ModuleInfo.ImageName,
                    sf.AddrPC.Offset - ModuleInfo.BaseOfImage);
        }
        else if (dwDisplacement)
            DPRINTF("3\t%s+0x%lx\n", pSymbol->Name, dwDisplacement);
        else
            DPRINTF("4\t%s\n", pSymbol->Name);
    }

    ResumeThread(hThread);
}

static DWORD WINAPI dump_thread_proc(LPVOID lpParameter)
{
    dump_backtrace_for_thread((HANDLE)lpParameter);
    return 0;
}

/* cannot get valid context from current thread, so we have to execute
 * backtrace from another thread */
static void dump_backtrace()
{
    HANDLE hCurrentThread;
    HANDLE hThread;
    DWORD dwThreadId;
    DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
        GetCurrentProcess(), &hCurrentThread, 0, FALSE, DUPLICATE_SAME_ACCESS);
    hThread = CreateThread(NULL, 0, dump_thread_proc, (LPVOID)hCurrentThread,
        0, &dwThreadId);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(hCurrentThread);
}
#endif

#endif
