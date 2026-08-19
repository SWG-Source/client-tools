/****************************************************************************
** $Id: qpipe_p.h 1964 2006-12-15 12:14:12Z chehrlic $
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


#ifndef QPIPE_H
#define QPIPE_H

#include "qobject.h"

class QPipe
{
public:
    typedef enum { toClient, fromClient } Type;
    typedef enum { none, Stdin, Stdout, Stderr} StdType;
    QPipe( Type _type, StdType stdtype = none );
    ~QPipe();
    const char *getError()
    {
        return error;
    }
    int write( char *buf, int size );
    int read( char *buf, int size );
    int isReadable();
    HANDLE getClientHandle();

    void restore();

private:
    DWORD get_std_handle( StdType stdtype );
    HANDLE hRead, hWrite, hDup, hSave;
    char *error;
    Type type;
    StdType m_stdtype;
    DWORD std_handle;
};

#endif
