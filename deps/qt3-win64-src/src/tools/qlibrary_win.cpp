/****************************************************************************
** $Id: qlibrary_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QLibraryPrivate class
**
** Created : 20040628
**
** Copyright (C) 2004 Ralf Habacker
** Copyright (C) 2004 Christian Ehrlicher
**
** This file is part of the tools module of the Qt GUI Toolkit.
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

#include "qplatformdefs.h"
#include "private/qlibrary_p.h"

/*
  The platform dependent implementations of
  - loadLibrary
  - freeLibrary
  - resolveSymbol
  It's not too hard to guess what the functions do.
*/

bool QLibraryPrivate::loadLibrary()
{
    if ( pHnd )
        return TRUE;

    QString filename = library->library();

    pHnd = QT_WA_INLINE( LoadLibraryExW( ( LPCWSTR ) filename.ucs2(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH ),
                         LoadLibraryExA( ( LPCSTR )filename.latin1(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH ) );

#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)

    if ( !pHnd )
        qWarning( "LoadLibrary(%s) failed with error %i",
                  library->library().latin1(), GetLastError() );
#endif

    return pHnd != 0;
}

bool QLibraryPrivate::freeLibrary()
{
    if ( !pHnd )
        return TRUE;

    if ( !FreeLibrary( pHnd ) ) {
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
        qWarning( "FreeLibrary() failed with error %i", GetLastError() );
#endif

        return FALSE;
    }

    pHnd = 0;
    return TRUE;
}

void* QLibraryPrivate::resolveSymbol( const char* symbol )
{
    if ( !pHnd )
        return 0;

    void* address = ( void* ) GetProcAddress( pHnd, symbol );
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)

    if ( !address )
        qWarning( "GetProcAddress(%s) failed with error code: %i",
                  symbol, GetLastError() );
#endif

    return address;
}

