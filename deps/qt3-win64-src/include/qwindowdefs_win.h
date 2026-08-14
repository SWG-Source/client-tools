/****************************************************************************
** $Id: qwindowdefs_win.h 1964 2006-12-15 12:14:12Z chehrlic $
**
** Definition of general window system dependent functions, types and
** constants
**
** Created : 20030126
**
** Copyright (C) 2003 Holger Schroeder
** Copyright (C) 2003 Christopher January
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2004 Ralf Habacker
** Copyright (C) 2004 Christian Ehrlicher
** Copyright (C) 2004 Andreas Hausladen
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact kde-cygwin@kde.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QWINDOWDEFS_WIN_H
#define QWINDOWDEFS_WIN_H


#if (defined(Q_CC_BOR) || defined(Q_CC_WAT)) && !defined(__CONSOLE__)
#define NEEDS_QMAIN
#endif

#define Q_DECLARE_HANDLE(name) struct name##__; typedef struct name##__ *name

Q_DECLARE_HANDLE( HINSTANCE );
Q_DECLARE_HANDLE( HDC );
Q_DECLARE_HANDLE( HWND );
Q_DECLARE_HANDLE( HFONT );
Q_DECLARE_HANDLE( HPEN );
Q_DECLARE_HANDLE( HBRUSH );
Q_DECLARE_HANDLE( HBITMAP );
Q_DECLARE_HANDLE( HICON );
typedef HICON HCURSOR;
Q_DECLARE_HANDLE( HPALETTE );
Q_DECLARE_HANDLE( HRGN );

typedef struct tagMSG MSG;
typedef HWND WId;

/* implemented in qapplication_win.cpp */
Q_EXPORT HINSTANCE qWinAppInst();       /* appInst */
Q_EXPORT HINSTANCE qWinAppPrevInst();   /* appPrevInst */
Q_EXPORT int qWinAppCmdShow();          /* nCmdShow */
Q_EXPORT HDC qt_display_dc();           /* use this instead of GetDC(0) to
                                           avoid GetDC/ReleaseDC functions */

#define QT_WIN_PAINTER_MEMBERS \
    HDC  hdc;               \
    HPEN hpen;              \
    HFONT hfont;            \
    HBRUSH hbrush;          \
    uint pixmapBrush :1;    \
    void nativeXForm(bool); \
    QPoint  curPt;


#if defined(__MINGW32__) // Postgresql-8.0.0 bug in postgresql-8.0.0/src/inlclude/c.h?
#define	WIN32_CLIENT_ONLY
#endif

#endif
