/****************************************************************************
** $Id: qthread_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** QThread class for Windows
**
** Created : 20040630
**
** Copyright (C) 2004 Ralf Habacker
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

#if defined(QT_THREAD_SUPPORT)

#include "qplatformdefs.h"
#include "qt_windows.h"
#include "qthread.h"

#include "qmutex.h"
#include "qwaitcondition.h"
#include <private/qmutexpool_p.h>
#include <private/qthreadinstance_p.h>
#include <private/qcriticalsection_p.h>
#include "qthreadstorage.h"

#include <process.h>

static QThreadInstance main_instance = {
                                           0, { 0, &main_instance }, 0, 0, 1, 0,    /*PTHREAD_COND_INITIALIZER*/0, 0
                                       };


static QMutexPool *qt_thread_mutexpool = 0;

static DWORD storage_key = TLS_OUT_OF_INDEXES;

static void create_storage_key()
{
    if ( storage_key != TLS_OUT_OF_INDEXES )
        return;

    static QMutex mutex;
    QMutexLocker locker(&mutex);
    storage_key = TlsAlloc();
}

QThreadInstance *QThreadInstance::current()
{
    create_storage_key();
    QThreadInstance *ret = ( QThreadInstance * ) TlsGetValue( storage_key );
    if ( ! ret )
        return & main_instance;
    return ret;
}

void QThreadInstance::init( unsigned int stackSize )
{
    stacksize = stackSize;
    args[ 0 ] = args[ 1 ] = 0;
    thread_storage = 0;
    finished = FALSE;
    running = FALSE;
    orphan = FALSE;

    handle = 0;
    thread_id = 0;
    waiters = 0;

    // threads have not been initialized yet, do it now
    if ( ! qt_thread_mutexpool )
        QThread::initialize();
}

void QThreadInstance::deinit()
{
// this produces a crash...    
//    TlsFree( storage_key );
}

unsigned int __stdcall QThreadInstance::start( void *_arg )
{
    void **arg = ( void ** ) _arg;

    create_storage_key();
    TlsSetValue( storage_key, arg[ 1 ] );

    ( ( QThread * ) arg[ 0 ] ) ->run();

    finish( ( QThreadInstance * ) arg[ 1 ] );
    return 0;
}

void QThreadInstance::finish( QThreadInstance *d )
{
    if ( ! d ) {
#ifdef QT_CHECK_STATE
        qWarning( "QThread: internal error: zero data for running thread." );
#endif // QT_CHECK_STATE

        return ;
    }

    QMutexLocker locker( d->mutex() );
    d->running = FALSE;
    d->finished = TRUE;
    d->args[ 0 ] = d->args[ 1 ] = 0;


    QThreadStorageData::finish( d->thread_storage );
    d->thread_storage = 0;


    if (!d->waiters) {
        CloseHandle(d->handle);
        d->handle = 0;
    }
    d->thread_id = 0;

    if ( d->orphan ) {
        d->deinit();
        delete d;
    }
}

QMutex *QThreadInstance::mutex() const
{
    return qt_thread_mutexpool ? qt_thread_mutexpool->get
           ( ( void * ) this ) : 0;
}

void QThreadInstance::terminate()
{
    if ( !running )
        return;
    finish( this );
    ::TerminateThread( handle, 0 );
}

/**************************************************************************
 ** QThread
 *************************************************************************/

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
    This returns the thread handle of the currently executing thread.

    \warning The handle returned by this function is used for internal
    purposes and should \e not be used in any application code. On
    Windows, the returned value is a pseudo handle for the current
    thread, and it cannot be used for numerical comparison.
*/
Qt::HANDLE QThread::currentThread()
{
    return ( Qt::HANDLE ) GetCurrentThreadId() ; // didn't like static_cast here :(
}


/*! \internal
  Initializes the QThread system.
*/
void QThread::initialize()
{
    if ( ! qt_global_mutexpool )
        qt_global_mutexpool = new QMutexPool( TRUE, 73 );
    if ( ! qt_thread_mutexpool )
        qt_thread_mutexpool = new QMutexPool( FALSE, 127 );
}


/*! \internal
  Cleans up the QThread system.
*/
void QThread::cleanup()
{
    delete qt_global_mutexpool;
    delete qt_thread_mutexpool;
    qt_global_mutexpool = 0;
    qt_thread_mutexpool = 0;
    QThreadInstance::finish( &main_instance ); // ???
}



/*!
    System independent sleep. This causes the current thread to sleep
    for \a secs seconds.
*/
void QThread::sleep( unsigned long secs )
{
    Sleep( secs * 1000 );
}

/*!
    System independent sleep. This causes the current thread to sleep
    for \a msecs milliseconds
*/
void QThread::msleep( unsigned long msecs )
{
    Sleep( msecs );
}

/*!
    System independent sleep. This causes the current thread to sleep
    for \a usecs microseconds
*/
void QThread::usleep( unsigned long usecs )
{
    Sleep ( usecs / 1000 + 1 ); // can't be 0 (?) don't know.
}

/*!
    Ends the execution of the calling thread and wakes up any threads
    waiting for its termination.
*/
void QThread::exit()
{
    QThreadInstance::finish( QThreadInstance::current() );
    _endthreadex( 0 );
}

/*!
    Begins execution of the thread by calling run(), which should be
    reimplemented in a QThread subclass to contain your code.  The
    operating system will schedule the thread according to the \a
    priority argument.

    If you try to start a thread that is already running, this
    function will wait until the the thread has finished and then
    restart the thread.

    \sa Priority
*/
void QThread::start( Priority priority )
{
    QMutexLocker locker( d->mutex() );

    if ( d->running ) {
#ifdef QT_CHECK_STATE
        qWarning( "Attempt to start a thread already running" );
#endif // QT_CHECK_STATE

        return;
    }

    d->running = true;
    d->finished = false;
    // imho not needed - we can access d every time ...
    d->args[ 0 ] = this;
    d->args[ 1 ] = d;

    d->handle = ( Qt::HANDLE ) ::_beginthreadex( 0, d->stacksize, QThreadInstance::start, d->args, CREATE_SUSPENDED, &( d->thread_id ) );

    if ( !d->handle ) {
#ifdef QT_CHECK_STATE
        qWarning( "QThread::start: thread creation error: %i", GetLastError() );
#endif // QT_CHECK_STATE

        d->running = false;
        d->finished = false;
        d->args[ 0 ] = d->args[ 1 ] = 0;
        return;
    }

    int prio; // = THREAD_PRIORITY_NORMAL;
    switch ( priority ) {
    case IdlePriority:
        prio = THREAD_PRIORITY_IDLE;
        break;
    case LowestPriority:
        prio = THREAD_PRIORITY_LOWEST;
        break;
    case LowPriority:
        prio = THREAD_PRIORITY_BELOW_NORMAL;
        break;
    case NormalPriority:
        prio = THREAD_PRIORITY_NORMAL;
        break;
    case HighPriority:
        prio = THREAD_PRIORITY_ABOVE_NORMAL;
        break;
    case HighestPriority:
        prio = THREAD_PRIORITY_HIGHEST;
        break;
    case TimeCriticalPriority:
        prio =  THREAD_PRIORITY_TIME_CRITICAL;
        break;

    //case InheritPriority:
    default:
        prio = GetThreadPriority( GetCurrentThread() );
        break;
    }

    if ( !SetThreadPriority( d->handle, prio ) ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QThread::start: Failed to set thread priority" );
#endif

    }
    // Now we let the thread run
    if ( ResumeThread( d->handle ) == ( DWORD )-1 ) {
#if defined(QT_CHECK_STATE)
        qWarning( "QThread::start: Failed to resume new thread" );
#endif

    }

}

void QThread::start()
{
    start( InheritPriority );
}

/*!
    This provides similar functionality to POSIX pthread_join. A thread
    calling this will block until either of these conditions is met:
    \list
    \i The thread associated with this QThread object has finished
 execution (i.e. when it returns from \l{run()}). This function
 will return TRUE if the thread has finished. It also returns
 TRUE if the thread has not been started yet.
    \i \a time milliseconds has elapsed. If \a time is ULONG_MAX (the
 default), then the wait will never timeout (the thread must
 return from \l{run()}). This function will return FALSE if the
 wait timed out.
    \endlist
*/
bool QThread::wait( unsigned long time )
{
    QMutexLocker locker( d->mutex() );

    if ( d->thread_id == GetCurrentThreadId() ) {
        qWarning("Thread tried to wait on itself");
        return false;
    }

    if ( d->finished || ! d->running )
        return true;

    ++d->waiters;
    locker.mutex()->unlock();

    bool ret = false;
    switch ( WaitForSingleObject( d->handle, time ) ) {
    case WAIT_OBJECT_0:
        ret = true;
        break;
    case WAIT_FAILED:
        qWarning("QThread::wait: Thread wait failure");
        break;
    case WAIT_ABANDONED:
    case WAIT_TIMEOUT:
    default:
        break;
    }

    locker.mutex()->lock();
    --d->waiters;

    if ( ret && !d->finished ) {
        // thread was terminated by someone else
        QThreadInstance::finish( d );
    }

    if ( d->finished && !d->waiters ) {
        CloseHandle( d->handle );
        d->handle = 0;
    }

    return ret;
}

/*!
    \fn void QThread::run()

    This method is pure virtual, and must be implemented in derived
    classes in order to do useful work. Returning from this method
    will end the execution of the thread.

    \sa wait()
*/

#endif // QT_THREAD_SUPPORT
