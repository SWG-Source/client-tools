/****************************************************************************
** $Id: qthreadstorage_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** QThreadStorageData class for Windows
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

#ifdef QT_THREAD_SUPPORT

#include <windows.h>    // GetCurrentThreadId

#include "qthreadstorage.h"

#include "qthread.h"
#include "qthreadinstance_p.h"
#include "qmutex.h"
#include "qcriticalsection_p.h"

// #define QTHREADSTORAGE_DEBUG

// keep this in sync with the implementation in qthreadstorage.cpp
static const int MAX_THREAD_STORAGE = 257; // 256 maximum + 1 used in QRegExp

static bool thread_storage_init = false;
static struct
{
    bool used;
    void ( *func ) ( void * );
}
thread_storage_usage[ MAX_THREAD_STORAGE ];


QThreadStorageData::QThreadStorageData( void ( *func ) ( void * ) )
        : id( 0 )
{
    QCriticalSection cs;
    cs.enter();

    // make sure things are initialized
    if ( ! thread_storage_init )
        memset( thread_storage_usage, 0, sizeof( thread_storage_usage ) );
    thread_storage_init = TRUE;

    for ( ; id < MAX_THREAD_STORAGE; ++id ) {
        if ( !thread_storage_usage[ id ].used )
            break;
    }

    Q_ASSERT( id >= 0 && id < MAX_THREAD_STORAGE );
    thread_storage_usage[ id ].used = true;
    thread_storage_usage[ id ].func = func;

#ifdef QTHREADSTORAGE_DEBUG

    qDebug( "QThreadStorageData: allocated id %d", id );
#endif

    cs.leave();
}

QThreadStorageData::~QThreadStorageData()
{
    QCriticalSection cs;
    cs.enter();

    thread_storage_usage[ id ].used = false;
    thread_storage_usage[ id ].func = 0;

#ifdef QTHREADSTORAGE_DEBUG

    qDebug( "QThreadStorageData: released id %d", id );
#endif

    cs.leave();
}

void **QThreadStorageData::get() const
{
    QThreadInstance * d = QThreadInstance::current();
    if ( !d ) {
        qWarning( "QThreadStorage can only be used with threads started with QThread" );
        return 0;
    }
    return d->thread_storage && d->thread_storage[ id ] ? &d->thread_storage[ id ] : 0;
}

void **QThreadStorageData::set( void *p )
{
    QThreadInstance * d = QThreadInstance::current();
    if ( !d ) {
        qWarning( "QThreadStorage can only be used with threads started with QThread" );
        return 0;
    }
    if ( !d->thread_storage ) {
#ifdef QTHREADSTORAGE_DEBUG
        qDebug( "QThreadStorageData: allocating storage for thread %lx",
                ( unsigned long ) pthread_self() );
#endif // QTHREADSTORAGE_DEBUG

        d->thread_storage = new void * [ MAX_THREAD_STORAGE ];
        memset( d->thread_storage, 0, sizeof( void* ) * MAX_THREAD_STORAGE );
    }

    // delete any previous data
    if ( d->thread_storage[ id ] ) {
#ifdef QTHREADSTORAGE_DEBUG
        qDebug( "QThreadStorageData: deleting previous storage %d for thread %p",
                id, d );
#endif // QTHREADSTORAGE_DEBUG

        void *q = d->thread_storage[ id ];
        d->thread_storage[ id ] = 0;
        thread_storage_usage[ id ].func( q );
    }

    // store new data
    d->thread_storage[ id ] = p;
    return &d->thread_storage[ id ];
}

void QThreadStorageData::finish( void **thread_storage )
{
    if ( ! thread_storage )
        return ; // nothing to do

#ifdef QTHREADSTORAGE_DEBUG

    qDebug( "QThreadStorageData: destroying storage for thread %lx",
            ( unsigned long ) pthread_self() );
#endif // QTHREADSTORAGE_DEBUG

    for ( int i = 0; i < MAX_THREAD_STORAGE; ++i ) {
        if ( !thread_storage[ i ] )
            continue;
        if ( !thread_storage_usage[ i ].used ) {
#ifdef QT_CHECK_STATE
            qWarning( "QThreadStorage: thread %lx exited after QThreadStorage destroyed",
                      ( unsigned long ) GetCurrentThreadId() );
#endif // QT_CHECK_STATE

            continue;
        }

        void *q = thread_storage[ i ];
        thread_storage[ i ] = 0;
        thread_storage_usage[ i ].func( q );
    }

    delete [] thread_storage;
}


/*!
    \class QThreadStorage
    \brief The QThreadStorage class provides per-thread data storage.

    \threadsafe
    \ingroup thread
    \ingroup environment

    QThreadStorage is a template class that provides per-thread data
    storage.

    \e{Note that due to compiler limitations, QThreadStorage can only
    store pointers.}

    The setLocalData() function stores a single thread-specific value
    for the calling thread. The data can be accessed later using the
    localData() functions. QThreadStorage takes ownership of the
    data (which must be created on the heap with \e new) and deletes
    it when the thread exits (either normally or via termination).

    The hasLocalData() function allows the programmer to determine if
    data has previously been set using the setLocalData() function.
    This is useful for lazy initializiation.

    For example, the following code uses QThreadStorage to store a
    single cache for each thread that calls the \e cacheObject() and
    \e removeFromCache() functions. The cache is automatically
    deleted when the calling thread exits (either normally or via
    termination).

    \code
    QThreadStorage<QCache<SomeClass> *> caches;

    void cacheObject( const QString &key, SomeClass *object )
    {
        if ( ! caches.hasLocalData() )
            caches.setLocalData( new QCache<SomeClass> );

        caches.localData()->insert( key, object );
    }

    void removeFromCache( const QString &key )
    {
        if ( ! caches.hasLocalData() )
            return; // nothing to do

        caches.localData()->remove( key );
    }
    \endcode

    \section1 Caveats

    \list

    \i As noted above, QThreadStorage can only store pointers due to
    compiler limitations. Support for value-based objects will be
    added when the majority of compilers are able to support partial
    template specialization.

    \i The \link ~QThreadStorage() destructor\endlink does \e not
    delete per-thread data. QThreadStorage only deletes per-thread
    data when the thread exits or when setLocalData() is called
    multiple times.

    \i QThreadStorage can only be used with threads started with
    QThread. It \e cannot be used with threads started with
    platform-specific APIs.

    \i As a corollary to the above, platform-specific APIs cannot be
    used to exit or terminate a QThread using QThreadStorage. Doing so
    will cause all per-thread data to be leaked. See QThread::exit()
    and QThread::terminate().

    \i QThreadStorage \e can be used to store data for the \e main()
    thread \e after QApplication has been constructed. QThreadStorage
    deletes all data set for the \e main() thread when QApplication is
    destroyed, regardless of whether or not the \e main() thread has
    actually finished.

    \i The implementation of QThreadStorage limits the total number of
    QThreadStorage objects to 256. An unlimited number of threads
    can store per-thread data in each QThreadStorage object.

    \endlist
*/

/*!
    \fn QThreadStorage::QThreadStorage()

    Constructs a new per-thread data storage object.
*/

/*!
    \fn QThreadStorage::~QThreadStorage()

    Destroys the per-thread data storage object.

    Note: The per-thread data stored is \e not deleted. Any data left
    in QThreadStorage is leaked. Make sure that all threads using
    QThreadStorage have exited before deleting the QThreadStorage.

    \sa hasLocalData()
*/

/*!
    \fn bool QThreadStorage::hasLocalData() const

    Returns TRUE if the calling thread has non-zero data available;
    otherwise returns FALSE.

    \sa localData()
*/

/*!
    \fn T& QThreadStorage::localData()

    Returns a reference to the data that was set by the calling
    thread.

    Note: QThreadStorage can only store pointers. This function
    returns a \e reference to the pointer that was set by the calling
    thread. The value of this reference is 0 if no data was set by
    the calling thread,

    \sa hasLocalData()
*/
/*
  ### addition to the above documentation when we start supporting
  ### partial template specialization, and QThreadStorage can store
  ### values *and* pointers

  When using QThreadStorage to store values (not pointers), this
  function stores an object of type \e T (created with its default
  constructor) and returns a reference to that object.
*/

/*!
    \fn const T QThreadStorage::localData() const
    \overload

    Returns a copy of the data that was set by the calling thread.

    Note: QThreadStorage can only store pointers. This function
    returns a pointer to the data that was set by the calling thread.
    If no data was set by the calling thread, this function returns 0.

    \sa hasLocalData()
*/
/*
  ### addition to the above documentation when we start supporting
  ### partial template specialization, and QThreadStorage can store
  ### values *and* pointers

  When using QThreadStorage to store values (not pointers), this
  function returns an object of type \e T (created with its default
  constructor). Unlike the above function, this object is \e not
  stored automatically. You will need to call setLocalData() to store
  the object.
*/

/*!
    \fn void QThreadStorage::setLocalData( T data )

    Sets the local data for the calling thread to \a data. It can be
    accessed later using the localData() functions.

    If \a data is 0, this function deletes the previous data (if
    any) and returns immediately.

    If \a data is non-zero, QThreadStorage takes ownership of the \a
    data and deletes it automatically either when the thread exits
    (either normally or via termination) or when setLocalData() is
    called again.

    Note: QThreadStorage can only store pointers. The \a data
    argument must be either a pointer to an object created on the heap
    (i.e. using \e new) or 0. You should not delete \a data
    yourself; QThreadStorage takes ownership and will delete the \a
    data itself.

    \sa localData() hasLocalData()
*/

#endif // QT_THREAD_SUPPORT
