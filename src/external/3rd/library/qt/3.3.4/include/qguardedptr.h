/****************************************************************************
** $Id: qt/qguardedptr.h   3.3.4   edited Dec 28 2003 $
**
** Definition of QGuardedPtr class
**
** Created : 990929
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

#ifndef QGUARDEDPTR_H
#define QGUARDEDPTR_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

// ### 4.0: rename to something without Private in it. Not really internal.
class Q_EXPORT QGuardedPtrPrivate : public QObject, public QShared
{
    Q_OBJECT
public:
    QGuardedPtrPrivate( QObject* );
    ~QGuardedPtrPrivate();

    QObject* object() const;
    void reconnect( QObject* );

private slots:
    void objectDestroyed();

private:
    QObject* obj;
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QGuardedPtrPrivate( const QGuardedPtrPrivate & );
    QGuardedPtrPrivate &operator=( const QGuardedPtrPrivate & );
#endif
};

template <class T>
class QGuardedPtr
{
public:
    QGuardedPtr() : priv( new QGuardedPtrPrivate( 0 ) ) {}

    QGuardedPtr( T* o) {
	priv = new QGuardedPtrPrivate( (QObject*)o );
    }

    QGuardedPtr(const QGuardedPtr<T> &p) {
	priv = p.priv;
	ref();
    }

    ~QGuardedPtr() { deref(); }

    QGuardedPtr<T> &operator=(const QGuardedPtr<T> &p) {
	if ( priv != p.priv ) {
	    deref();
	    priv = p.priv;
	    ref();
	}
	return *this;
    }

    QGuardedPtr<T> &operator=(T* o) {
	if ( priv && priv->count == 1 ) {
	    priv->reconnect( (QObject*)o );
	} else {
	    deref();
	    priv = new QGuardedPtrPrivate( (QObject*)o );
	}
	return *this;
    }

    bool operator==( const QGuardedPtr<T> &p ) const {
	return (T*)(*this) == (T*) p;
    }

    bool operator!= ( const QGuardedPtr<T>& p ) const {
	return !( *this == p );
    }

    bool isNull() const { return !priv || !priv->object(); }

    T* operator->() const { return (T*)(priv?priv->object():0); }

    T& operator*() const { return *((T*)(priv?priv->object():0)); }

    operator T*() const { return (T*)(priv?priv->object():0); }

private:

    void ref() { if (priv) priv->ref(); }

    void deref() {
	if ( priv && priv->deref() )
	    delete priv;
    }

    QGuardedPtrPrivate* priv;
};




inline QObject* QGuardedPtrPrivate::object() const
{
    return obj;
}

#define Q_DEFINED_QGUARDEDPTR
#include "qwinexport.h"
#endif
