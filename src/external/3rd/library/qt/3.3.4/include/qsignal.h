/****************************************************************************
** $Id: qt/qsignal.h   3.3.4   edited May 27 2003 $
**
** Definition of QSignal class
**
** Created : 941201
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

#ifndef QSIGNAL_H
#define QSIGNAL_H

#ifndef QT_H
#include "qvariant.h"
#include "qobject.h"
#endif // QT_H


class Q_EXPORT QSignal : public QObject
{
    Q_OBJECT

public:
    QSignal( QObject *parent=0, const char *name=0 );
    ~QSignal();

    bool	connect( const QObject *receiver, const char *member );
    bool	disconnect( const QObject *receiver, const char *member=0 );

    void	activate();

#ifndef QT_NO_COMPAT
    bool	isBlocked()	 const		{ return QObject::signalsBlocked(); }
    void	block( bool b )		{ QObject::blockSignals( b ); }
#ifndef QT_NO_VARIANT
    void	setParameter( int value );
    int		parameter() const;
#endif
#endif

#ifndef QT_NO_VARIANT
    void	setValue( const QVariant &value );
    QVariant	value() const;
#endif
signals:
#ifndef QT_NO_VARIANT
    void signal( const QVariant& );
#endif
    void intSignal( int );

private:
#ifndef QT_NO_VARIANT
    QVariant val;
#endif
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSignal( const QSignal & );
    QSignal &operator=( const QSignal & );
#endif
};


#endif // QSIGNAL_H
