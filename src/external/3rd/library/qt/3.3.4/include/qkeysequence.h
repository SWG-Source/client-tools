/****************************************************************************
** $Id: qt/qkeysequence.h   3.3.4   edited May 27 2003 $
**
** Definition of QKeySequence class
**
** Created : 0108007
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#ifndef QKEYSEQUENCE_H
#define QKEYSEQUENCE_H

#ifndef QT_H
#ifndef QT_H
#include "qnamespace.h"
#include "qstring.h"
#endif // QT_H
#endif

#ifndef QT_NO_ACCEL

/*****************************************************************************
  QKeySequence stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
class QKeySequence;
Q_EXPORT QDataStream &operator<<( QDataStream &, const QKeySequence & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QKeySequence & );
#endif

class QKeySequencePrivate;

class Q_EXPORT QKeySequence : public Qt
{
public:
    QKeySequence();
    QKeySequence( const QString& key );
    QKeySequence( int key );
    QKeySequence( int k1, int k2, int k3 = 0, int k4 = 0 );
    QKeySequence( const QKeySequence & );
    ~QKeySequence();

    uint count() const;
    bool isEmpty() const;
    Qt::SequenceMatch matches( const QKeySequence & ) const;

    operator QString() const;
    operator int () const;
    int operator[]( uint ) const;
    QKeySequence &operator=( const QKeySequence & );
    bool operator==( const QKeySequence& ) const;
    bool operator!= ( const QKeySequence& ) const;

private:
    static int decodeString( const QString & );
    static QString encodeString( int );
    int assign( QString );
    void setKey( int key, int index );

    QKeySequencePrivate* d;

    friend Q_EXPORT QDataStream &operator<<( QDataStream &, const QKeySequence & );
    friend Q_EXPORT QDataStream &operator>>( QDataStream &, QKeySequence & );
    friend class QAccelManager;
};

#else

class Q_EXPORT QKeySequence : public Qt
{
public:
    QKeySequence() {}
    QKeySequence( int ) {}
};

#endif //QT_NO_ACCEL

#endif
