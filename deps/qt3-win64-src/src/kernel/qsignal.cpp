/****************************************************************************
** $Id: qsignal.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QSignal class
**
** Created : 941201
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech ASA of Norway and appearing in the file
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

#include "qsignal.h"
#include "qmetaobject.h"
#include "qguardedptr.h"

/*!
    \class QSignal qsignal.h
    \brief The QSignal class can be used to send signals for classes
    that don't inherit QObject.

    \ingroup io
    \ingroup misc

    If you want to send signals from a class that does not inherit
    QObject, you can create an internal QSignal object to emit the
    signal. You must also provide a function that connects the signal
    to an outside object slot.  This is how we have implemented
    signals in the QMenuData class, which is not a QObject.

    In general, we recommend inheriting QObject instead. QObject
    provides much more functionality.

    You can set a single QVariant parameter for the signal with
    setValue().

    Note that QObject is a \e private base class of QSignal, i.e. you
    cannot call any QObject member functions from a QSignal object.

    Example:
    \code
	#include <qsignal.h>

	class MyClass
	{
	public:
	    MyClass();
	    ~MyClass();

	    void doSomething();

	    void connect( QObject *receiver, const char *member );

	private:
	    QSignal *sig;
	};

	MyClass::MyClass()
	{
	    sig = new QSignal;
	}

	MyClass::~MyClass()
	{
	    delete sig;
	}

	void MyClass::doSomething()
	{
	    // ... does something
	    sig->activate(); // emits the signal
	}

	void MyClass::connect( QObject *receiver, const char *member )
	{
	    sig->connect( receiver, member );
	}
    \endcode
*/

/*!
    Constructs a signal object called \a name, with the parent object
    \a parent. These arguments are passed directly to QObject.
*/

QSignal::QSignal( QObject *parent, const char *name )
    : QObject( parent, name )
{
    isSignal = TRUE;
#ifndef QT_NO_VARIANT
    val = 0;
#endif
}

/*!
    Destroys the signal. All connections are removed, as is the case
    with all QObjects.
*/
QSignal::~QSignal()
{
}
#ifndef QT_NO_VARIANT
// Returns TRUE if it matches ".+(.*int.*"
static inline bool intSignature( const char *member )
{
    QCString s( member );
    int p = s.find( '(' );
    return p > 0 && p < s.findRev( "int" );
}
#endif
/*!
    Connects the signal to \a member in object \a receiver.

    \sa disconnect(), QObject::connect()
*/

bool QSignal::connect( const QObject *receiver, const char *member )
{
#ifndef QT_NO_VARIANT
    if ( intSignature( member ) )
#endif
	return QObject::connect( (QObject *)this, SIGNAL(intSignal(int)), receiver, member );
#ifndef QT_NO_VARIANT
    return QObject::connect( (QObject *)this, SIGNAL(signal(const QVariant&)),
			     receiver, member );
#endif
}

/*!
    Disonnects the signal from \a member in object \a receiver.

    \sa connect(), QObject::disconnect()
*/

bool QSignal::disconnect( const QObject *receiver, const char *member )
{
    if (!member)
	return QObject::disconnect( (QObject *)this, 0, receiver, member);
#ifndef QT_NO_VARIANT
    if ( intSignature( member ) )
#endif
	return QObject::disconnect( (QObject *)this, SIGNAL(intSignal(int)), receiver, member );
#ifndef QT_NO_VARIANT
    return QObject::disconnect( (QObject *)this, SIGNAL(signal(const QVariant&)),
				receiver, member );
#endif
}


/*!
  \fn bool QSignal::isBlocked() const
  \obsolete
  Returns TRUE if the signal is blocked, or FALSE if it is not blocked.

  The signal is not blocked by default.

  \sa block(), QObject::signalsBlocked()
*/

/*!
  \fn void QSignal::block( bool b )
  \obsolete
  Blocks the signal if \a b is TRUE, or unblocks the signal if \a b is FALSE.

  An activated signal disappears into hyperspace if it is blocked.

  \sa isBlocked(), activate(), QObject::blockSignals()
*/


/*!
    \fn void QSignal::activate()

    Emits the signal. If the platform supports QVariant and a
    parameter has been set with setValue(), this value is passed in
    the signal.
*/
void  QSignal::activate()
{
#ifndef QT_NO_VARIANT
    /* Create this QGuardedPtr on this, if we get destroyed after the intSignal (but before the variant signal)
       we cannot just emit the signal (because val has been destroyed already) */
    QGuardedPtr<QSignal> me = this;
    if( me ) 
	emit intSignal( val.toInt() );
    if( me ) 
	emit signal( val );
#else
    emit intSignal(0);
#endif
}

#ifndef QT_NO_VARIANT
/*!
    Sets the signal's parameter to \a value
*/
void QSignal::setValue( const QVariant &value )
{
    val = value;
}

/*!
    Returns the signal's parameter
*/
QVariant QSignal::value() const
{
    return val;
}
/*! \fn void QSignal::signal( const QVariant & )
    \internal
*/
/*! \fn void QSignal::intSignal( int )
    \internal
*/

#ifndef QT_NO_COMPAT
/*! \obsolete */
void QSignal::setParameter( int value )
{
    val = value;
}

/*! \obsolete */
int QSignal::parameter() const
{
    return val.toInt();
}
#endif
#endif //QT_NO_VARIANT
