/****************************************************************************
** $Id: qtmain_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of Windows Exe entry point
**
** Created : 20040730
**
** Copyright (C) 2004 Ralf Habacker
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact kde-cygwin@kde.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qapplication.h"
#include "qt_windows.h"
#include "qglobal.h"

#if defined( Q_OS_TEMP ) // same as qapplication_win.cpp
extern void __cdecl qWinMain( HINSTANCE, HINSTANCE, LPSTR, int, int &, QMemArray<pchar> & );
#else
extern void qWinMain( HINSTANCE, HINSTANCE, LPSTR, int, int &, QMemArray<pchar> & );
#endif

/* for borland and watchcom regular applications defines an entry point named
 qMain() instead of main() because of internal limitations in gui applications we
 can't call main() inside our WinMain.
 But we only need this crap for GUI-apps
 -> see qwindowdefs_win.h
 */
#ifdef NEEDS_QMAIN
/* Don't know why, but defining this 'extern "C"' prevents ilink32 from
   finding qMain in main.cpp ... courious
   because of this our old version:
   undef main
   int main (int argc,char** argv) {
        qMain(argc,argv     
   }
   won't work when it is defined at the end of this file since then qMain
   is already defined as extern "C" ...
   So I think this is the best solution atm...
   */
int qMain( int , char ** );
#else
#ifdef Q_OS_TEMP
extern "C" int __cdecl main( int, char ** ); // hmmmm
#else
extern "C" int main( int, char ** );
#endif
#endif

/*
 \internal
 WinMain() - Under Windows, this is the entry point, instead of main, let's "fix" it :)
 \sa http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wceui40/htm/cerefWinMain.asp
*/

#ifdef Q_OS_TEMP
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdParam, int nShowCmd )
#else
extern "C"
    int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                          LPSTR lpCmdParam, int nShowCmd )
#endif
{
    int argc = 0;
    QMemArray<pchar> argv( 8 ); // to avoid crash...

    qWinMain( hInstance, hPrevInstance, lpCmdParam, nShowCmd, argc, argv );
    int result = main( argc, argv.data() );
    return result;
}

/* This is imho the only way how to include qtcrtentrypoint without modifying
   official qtmain.pro from TT */
#if defined (Q_OS_WIN) && defined (Q_CC_GNU)
#include "../../mkspecs/win32-g++/qtcrtentrypoint.cpp"
#endif
