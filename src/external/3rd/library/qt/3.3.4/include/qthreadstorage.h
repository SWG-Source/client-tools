/****************************************************************************
** $Id: qt/qthreadstorage.h   3.3.4   edited Oct 29 2003 $
**
** ...
**
** Copyright (C) 2003 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QTHREADSTORAGE_H
#define QTHREADSTORAGE_H

#ifdef QT_THREAD_SUPPORT

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H

class Q_EXPORT QThreadStorageData
{
public:
    QThreadStorageData( void (*func)(void *) );
    ~QThreadStorageData();

    void** get() const;
    void** set( void* p );

    static void finish( void** );
    int id;
};


template <class T>
class QThreadStorage
{
private:
    QThreadStorageData d;

#if defined(Q_DISABLE_COPY)
    // disable copy constructor and operator=
    QThreadStorage( const QThreadStorage & );
    QThreadStorage &operator=( const QThreadStorage & );
#endif // Q_DISABLE_COPY

    static void deleteData( void *x ) { delete (T)x; }

public:
    inline QThreadStorage() : d( deleteData ) {  }
    inline ~QThreadStorage() { }

    inline bool hasLocalData() const
    { return d.get() != 0; }

    inline T& localData()
    { void **v = d.get(); if ( !v ) v = d.set( 0 ); return *(T*)v; }

    inline T localData() const
    { void **v = d.get(); return ( v ? *(T*)v : 0 ); }

    inline void setLocalData( T t )
    { (void) d.set( t ); }
};

#endif // QT_THREAD_SUPPORT

#endif // QTHREADSTORAGE_H
