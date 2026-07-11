/****************************************************************************
** $Id: qthread.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Cross-platform QThread implementation.
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
** licenses for Unix/X11 or for Qt/Embedded may use this file in accordance
** with the Qt Commercial License Agreement provided with the Software.
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

#ifdef QT_THREAD_SUPPORT

#include "qplatformdefs.h"

#include "qthread.h"
#include <private/qthreadinstance_p.h>

#ifndef QT_H
#  include "qapplication.h"
#endif // QT_H

#if QT_VERSION >= 0x040000
#  error "Remove QThread::QThread() and QThread::start()."
#endif


/*!
    \class QThread qthread.h
    \threadsafe
    \brief The QThread class provides platform-independent threads.

    \ingroup thread
    \ingroup environment

    A QThread represents a separate thread of control within the
    program; it shares data with all the other threads within the
    process but executes independently in the way that a separate
    program does on a multitasking operating system. Instead of
    starting in main(), QThreads begin executing in run(). You inherit
    run() to include your code. For example:

    \code
    class MyThread : public QThread {

    public:

	virtual void run();

    };

    void MyThread::run()
    {
	for( int count = 0; count < 20; count++ ) {
	    sleep( 1 );
	    qDebug( "Ping!" );
	}
    }

    int main()
    {
	MyThread a;
	MyThread b;
	a.start();
	b.start();
	a.wait();
	b.wait();
    }
    \endcode

    This will start two threads, each of which writes Ping! 20 times
    to the screen and exits. The wait() calls at the end of main() are
    necessary because exiting main() ends the program, unceremoniously
    killing all other threads. Each MyThread stops executing when it
    reaches the end of MyThread::run(), just as an application does
    when it leaves main().

    \sa \link threads.html Thread Support in Qt\endlink.
*/

/*!
    \enum QThread::Priority

    This enum type indicates how the operating system should schedule
    newly created threads.

    \value IdlePriority scheduled only when no other threads are
           running.

    \value LowestPriority scheduled less often than LowPriority.
    \value LowPriority scheduled less often than NormalPriority.

    \value NormalPriority the default priority of the operating
           system.

    \value HighPriority scheduled more often than NormalPriority.
    \value HighestPriority scheduled more often then HighPriority.

    \value TimeCriticalPriority scheduled as often as possible.

    \value InheritPriority use the same priority as the creating
           thread.  This is the default.
*/

QThread::QThread()
{
    d = new QThreadInstance;
    d->init(0);
}

/*!
    Constructs a new thread. The thread does not begin executing until
    start() is called.

    If \a stackSize is greater than zero, the maximum stack size is
    set to \a stackSize bytes, otherwise the maximum stack size is
    automatically determined by the operating system.

    \warning Most operating systems place minimum and maximum limits
    on thread stack sizes. The thread will fail to start if the stack
    size is outside these limits.
*/
QThread::QThread( unsigned int stackSize )
{
    d = new QThreadInstance;
    d->init(stackSize);
}

/*!
    QThread destructor.

    Note that deleting a QThread object will not stop the execution of
    the thread it represents. Deleting a running QThread (i.e.
    finished() returns FALSE) will probably result in a program crash.
    You can wait() on a thread to make sure that it has finished.
*/
QThread::~QThread()
{
    QMutexLocker locker( d->mutex() );
    if ( d->running && !d->finished ) {
#ifdef QT_CHECK_STATE
	qWarning("QThread object destroyed while thread is still running.");
#endif

	d->orphan = TRUE;
	return;
    }

    d->deinit();
    delete d;
}

/*!
    This function terminates the execution of the thread. The thread
    may or may not be terminated immediately, depending on the
    operating system's scheduling policies. Use QThread::wait()
    after terminate() for synchronous termination.

    When the thread is terminated, all threads waiting for the
    the thread to finish will be woken up.

    \warning This function is dangerous, and its use is discouraged.
    The thread can be terminated at any point in its code path.  Threads
    can be terminated while modifying data.  There is no chance for
    the thread to cleanup after itself, unlock any held mutexes, etc.
    In short, use this function only if \e absolutely necessary.
*/
void QThread::terminate()
{
    QMutexLocker locker( d->mutex() );
    if ( d->finished || !d->running )
	return;
    d->terminate();
}

/*!
    Returns TRUE if the thread is finished; otherwise returns FALSE.
*/
bool QThread::finished() const
{
    QMutexLocker locker( d->mutex() );
    return d->finished;
}

/*!
    Returns TRUE if the thread is running; otherwise returns FALSE.
*/
bool QThread::running() const
{
    QMutexLocker locker( d->mutex() );
    return d->running;
}

/*!
    \fn void QThread::run()

    This method is pure virtual, and must be implemented in derived
    classes in order to do useful work. Returning from this method
    will end the execution of the thread.

    \sa wait()
*/

#ifndef QT_NO_COMPAT
/*! \obsolete
    Use QApplication::postEvent() instead.
*/
void QThread::postEvent( QObject * receiver, QEvent * event )
{
    QApplication::postEvent( receiver, event );
}
#endif

#endif // QT_THREAD_SUPPORT
