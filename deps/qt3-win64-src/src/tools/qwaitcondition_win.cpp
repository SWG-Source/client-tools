/****************************************************************************
** $Id: qwaitcondition_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** QWaitCondition class for Windows
**
** From Wolfpack 3.3.1 release + sync with Qt4.0.0 by Christian Ehrlicher
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

#if defined(QT_THREAD_SUPPORT)

#include "qwaitcondition.h"
#include "qnamespace.h"
#include "qmutex.h"
#include "qptrlist.h"
#include "qt_windows.h"
#include <private/qcriticalsection_p.h>

#define Q_MUTEX_T void*
#include <private/qmutex_p.h>

class QWaitConditionEvent
{
public:
    inline QWaitConditionEvent() : priority( 0 )
    {
        QT_WA ( {
                    event = CreateEvent( NULL, true, false, NULL );
                }, {
                    event = CreateEventA( NULL, true, false, NULL );
                } );
    }
    inline ~QWaitConditionEvent()
    {
        CloseHandle( event );
    }
    int priority;
    HANDLE event;
};

typedef QPtrList<QWaitConditionEvent> EventQueue;

class QWaitConditionPrivate
{
public:
    QCriticalSection cs;
    EventQueue queue;
    EventQueue freeQueue;

    bool wait( QMutex *mutex, unsigned long time );
};

bool QWaitConditionPrivate::wait( QMutex *mutex, unsigned long time )
{
    bool ret = false;

    cs.enter();
    QWaitConditionEvent *wce =
        freeQueue.isEmpty() ? new QWaitConditionEvent : freeQueue.getFirst();
    wce->priority = GetThreadPriority( GetCurrentThread() );

    // insert 'wce' into the queue (sorted by priority)
    int index = 0;
    for ( ; index < queue.count(); ++index ) {
        QWaitConditionEvent *current = queue.at( index );
        if ( current->priority < wce->priority )
            break;
    }
    queue.insert( index, wce );
    cs.leave();

    if(mutex)
        mutex->unlock();

    // wait for the event
    switch ( WaitForSingleObject( wce->event, time ) ) {
    default:
        break;

    case WAIT_OBJECT_0:
        ret = true;
        break;
    }

    if(mutex)
        mutex->lock ();

    cs.enter();
    // remove 'wce' from the queue
    while ( queue.remove( wce ) ) {};
    ResetEvent( wce->event );
    freeQueue.append( wce );
    cs.leave();

    return ret;
}

//***********************************************************************
// QWaitCondition implementation
//***********************************************************************

QWaitCondition::QWaitCondition()
{
    d = new QWaitConditionPrivate;
}

QWaitCondition::~QWaitCondition()
{
    if ( !d->queue.isEmpty() ) {
        qWarning( "QWaitCondition: destroyed while threads are still waiting" );
        while ( d->queue.remove() ) {};
    }

    while ( d->freeQueue.remove() ) {};
    delete d;
}

void QWaitCondition::wakeOne()
{
    // wake up the first thread in the queue
    d->cs.enter();
    if ( !d->queue.isEmpty() ) {
        QWaitConditionEvent * first = d->queue.first();
        SetEvent( first->event );
    }
    d->cs.leave();
}

void QWaitCondition::wakeAll()
{
    // wake up the all threads in the queue
    d->cs.enter();
    for ( int i = 0; i < d->queue.count(); ++i ) {
        QWaitConditionEvent *current = d->queue.at( i );
        SetEvent( current->event );
    }
    d->cs.leave();
}

bool QWaitCondition::wait( unsigned long time )
{
    return wait( 0, time );
}

bool QWaitCondition::wait( QMutex *mutex, unsigned long time )
{
    if ( !mutex )
        return false;

    if ( mutex->d->type() == Q_MUTEX_RECURSIVE ) {
        qWarning( "QWaitCondition::wait: Cannot wait on recursive mutexes." );
        return false;
    }
    return d->wait( mutex, time );
}

#endif // QT_THREAD_SUPPORT
