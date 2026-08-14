/****************************************************************************
** $Id: qpipe_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of (internal used) QPipe class
**
** Created : 20040420
**
** Copyright (C) 2004-2005 Ralf Habacker
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


#include <qglobal.h>
#include <windows.h>
#include <stdio.h>

// maybe also needed for Q_CC_MSVC_NET
#if !defined(Q_CC_GNU)
#define __FUNCTION__ ""
#endif

#include "qpipe_p.h"

QPipe::QPipe( Type _type, StdType stdtype )
{
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof( SECURITY_ATTRIBUTES );
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    bool fSuccess;
    type = _type;
    m_stdtype = stdtype;
    hRead = 0;
    hWrite = 0;
    hDup = 0;

    // The steps for redirecting child process's STDxxx:
    //     1. Save current STDxxx, to be restored later.
    //     2. Create anonymous pipe to be STDxxx for child process.
    //     3. Set STDxxx of the parent process to be write handle to
    //        the pipe, so it is inherited by the child process.
    //     4. Create a noninheritable duplicate of the read handle and
    //        close the inheritable read handle.

    // Save the handle to the current STDxxx
    if ( stdtype != none ) {
        std_handle = get_std_handle( stdtype );
        hSave = GetStdHandle( std_handle );
    }

    // Create a pipe for the child process's stdxx.

    if ( !CreatePipe( &hRead, &hWrite, &saAttr, 0 ) ) {
        error = "Stderr pipe creation failed\n";
        printf( "%s: %s\n", __FUNCTION__, error );
        return ;
    }

    // Set a write handle to the pipe to be STDxxx.
    if ( ( ( type == toClient ) && ! SetStdHandle( std_handle, hRead ) ) ||
            ( ( type == fromClient ) && ! SetStdHandle( std_handle, hWrite ) ) ) {
        error = "Redirecting stdxxx failed";
        printf( "%s: %s\n", __FUNCTION__, error );
        return ;
    }

    // Create noninheritable handle and close the inheritable handle.
    HANDLE hOrigin = type == toClient ? hWrite : hRead;
    fSuccess = DuplicateHandle( GetCurrentProcess(), hOrigin,
                                GetCurrentProcess(), &hDup , 0,
                                FALSE,
                                DUPLICATE_SAME_ACCESS );

    if ( !fSuccess ) {
        error = "DuplicateHandle failed";
        printf( "%s: %s\n", __FUNCTION__, error );
        return ;
    }
    CloseHandle( hOrigin );
    hOrigin = 0;
    if ( type == toClient )
        hWrite = 0;
    else
        hRead = 0;
}


QPipe::~QPipe()
{
    if ( hRead )
        CloseHandle( hRead );
    if ( hWrite )
        CloseHandle( hWrite );
    if ( hDup )
        CloseHandle( hDup );
}

DWORD QPipe::get_std_handle( StdType stdtype )
{
    // get handle to the current STDERR.
    switch ( stdtype ) {
    case none:
        return 0;
        break;
    case Stdin:
        return STD_INPUT_HANDLE;
        break;
    case Stdout:
        return STD_OUTPUT_HANDLE;
        break;
    case Stderr:
        return STD_ERROR_HANDLE;
        break;
    }
    return 0;
}

int QPipe::write( char *buf, int size )
{
    // FIXME set error code
    if ( type == fromClient )
        return 0;

    DWORD dwWritten;
    int ret;
    ret = WriteFile( hDup, buf, size, &dwWritten, NULL );
    if ( ret == 0 ) {
        printf( "%s:%d\n", __FUNCTION__, GetLastError() );
        return 0;
    } else
        return dwWritten > 0 ? dwWritten : -1;
}

int QPipe::read( char *buf, int size )
{
    // FIXME set error code
    if ( type == toClient )
        return 0;

    DWORD dwAvail;
    if ( ! PeekNamedPipe ( hDup, NULL, 0, NULL, &dwAvail, NULL))
      return 0;

    if ( dwAvail > size )
        dwAvail = size;

    DWORD dwRead;
    // using the number of bytes got from PeekNamedPipe(), otherwise
    // this is a blocking call
    if ( ! ReadFile( hDup, buf, dwAvail, &dwRead, NULL ) )
        return 0;
    else
        return dwRead > 0 ? dwRead : -1;
}

int QPipe::isReadable()
{
    // FIXME set error code
    if ( type == toClient )
        return -1;

    DWORD dwAvail;
    if ( ! PeekNamedPipe ( hDup, NULL, 0, NULL, &dwAvail, NULL))
        return -1;
    return dwAvail;
}

HANDLE QPipe::getClientHandle()
{
    if ( type == toClient )
        return hRead;
    else
        return hWrite;
}

void QPipe::restore()
{
    if ( m_stdtype != none )
        SetStdHandle( std_handle, hSave );
}

