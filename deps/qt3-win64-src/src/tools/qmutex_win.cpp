/****************************************************************************
** $Id: qmutex_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** QMutex class for Windows
**
** Created : 20040628
**
** From Wolfpack 3.3.1 release
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

#include "qt_windows.h"
#include "qmutex.h"
//#define DEBUG_QMUTEX_WIN

#define Q_MUTEX_T void*

#include <private/qmutex_p.h>
#include <private/qcriticalsection_p.h>

// Private class declarations
class QRecursiveMutexPrivate : public QMutexPrivate
{
public:
    QRecursiveMutexPrivate();

    virtual void lock();
    virtual void unlock();
    virtual bool locked();
    virtual bool trylock();
    virtual int type() const
    {
        return Q_MUTEX_RECURSIVE;
    }
};

// base destructor
QMutexPrivate::~QMutexPrivate()
{
#ifdef DEBUG_QMUTEX_WIN
    qDebug( "QMutexPrivate::~QMutexPrivate - handle: 0x%08p", handle );
#endif

    if ( !::CloseHandle( handle ) )
#if defined( QT_CHECK_RANGE )

        qSystemWarning( "[QMutexPrivate::~QMutexPrivate] error calling ::CloseHandle()" );
#endif
}

// Recursive mutex class
/*****************************************************************************
  QRecursiveMutexPrivate member functions
 *****************************************************************************/

QRecursiveMutexPrivate::QRecursiveMutexPrivate()
{
    handle = ::CreateMutex( NULL, FALSE, NULL );
    if ( !handle )
        qSystemWarning( "QRecursiveMutexPrivate::QRecursiveMutexPrivate() - ::CreateMutex failed" );

#ifdef DEBUG_QMUTEX_WIN
    qDebug( "QRecursiveMutexPrivate::QRecursiveMutexPrivate - handle: 0x%08p", handle );
#endif
}

void QRecursiveMutexPrivate::lock()
{
    switch ( ::WaitForSingleObject( handle, INFINITE ) ) {
    case WAIT_FAILED:
    case WAIT_TIMEOUT:
        qSystemWarning( "QRecursiveMutexPrivate::lock(), error calling WaitForSingleObject" );
        break;
    case WAIT_ABANDONED:
        qWarning( "Thread terminated while in QRealMutexPrivate::lock()" );
        break;
    default:
        break;
    }
}

void QRecursiveMutexPrivate::unlock()
{
    if ( ::ReleaseMutex( handle ) == 0 ) {
#if defined(QT_CHECK_RANGE)
        qSystemWarning( "QRealMutexPrivate::unlock(), error calling ReleaseMutex" );
#endif

    }
}

bool QRecursiveMutexPrivate::locked()
{
    switch ( ::WaitForSingleObject( handle, 0 ) ) {
    case STATUS_WAIT_0:             // 0x00
        ::ReleaseMutex( handle );
        return false;
    case WAIT_ABANDONED:   // 0x80
        qSystemWarning( "QRecursiveMutexPrivate::locked() - ::WaitForSingleObject() failed" );
        ::ReleaseMutex( handle );
        return false;
    case WAIT_TIMEOUT:            // 0x102
    case WAIT_FAILED:               // ~0U
        return true;
    default:
        break;
    }
    return true;
}

bool QRecursiveMutexPrivate::trylock()
{
    switch ( ::WaitForSingleObject( handle, 0 ) ) {
    case WAIT_ABANDONED:   // 0x80
        qSystemWarning( "QRecursiveMutexPrivate::trylock() - ::WaitForSingleObject() failed" );
        return false;
    case WAIT_TIMEOUT:            // 0x102
        return false;
    case WAIT_FAILED:               // ~0U
        return false;
    default:
        break;
    }
    return true;
}

/*****************************************************************************
  QRealMutexPrivate member functions
 *****************************************************************************/

class QRealMutexPrivate : public QMutexPrivate
{
public:
    QRealMutexPrivate();

    virtual void lock();
    virtual void unlock();
    virtual bool locked();
    virtual bool trylock();
    virtual int type() const
    {
        return Q_MUTEX_NORMAL;
    }

    QCriticalSection criticalsection;
    unsigned int owner;
};

QRealMutexPrivate::QRealMutexPrivate()
{
    owner = 0;
    handle = ::CreateMutex( NULL, FALSE, NULL );
    if ( !handle )
        qSystemWarning( "QRecursiveMutexPrivate::QRecursiveMutexPrivate() - ::CreateMutex failed" );

#ifdef DEBUG_QMUTEX_WIN
    qDebug( "QRealMutexPrivate::QRealMutexPrivate - handle: 0x%08p", handle );
#endif
}

void QRealMutexPrivate::lock()
{
    criticalsection.enter();

    if ( GetCurrentThreadId() == owner ) {
        criticalsection.leave();
        qWarning( "Non-recursive mutex already locked by this thread" );
        return;
    }

    criticalsection.leave();

    switch ( ::WaitForSingleObject( handle, INFINITE ) ) {
    case WAIT_TIMEOUT:      // 0x102
        qSystemWarning( "QRealMutexPrivate::lock() - ::WaitForSingleObject() returned WAIT_TIMEOUT" );
        break;
    case WAIT_FAILED:       // ~0U
        qSystemWarning( "QRealMutexPrivate::lock() - ::WaitForSingleObject() returned WAIT_FAILED" );
        break;
    case WAIT_ABANDONED:    // 0x80
        qSystemWarning( "QRealMutexPrivate::lock() - ::WaitForSingleObject() returned WAIT_ABANDONED" );
    case WAIT_OBJECT_0:
    default:
        criticalsection.enter();
        owner = GetCurrentThreadId();
        criticalsection.leave();
        break;
    }
}

void QRealMutexPrivate::unlock()
{
    criticalsection.enter();
    if(owner == 0) {
        criticalsection.leave();
    	return;
    }
    if ( ::ReleaseMutex( handle ) == 0 )
        qSystemWarning( "QRealMutexPrivate::unlock(), error calling ReleaseMutex" );
    owner = 0;
    criticalsection.leave();
}

bool QRealMutexPrivate::locked()
{
    criticalsection.enter();
    if ( GetCurrentThreadId() == owner ) {
        criticalsection.leave();
        return true;
    }

    switch ( ::WaitForSingleObject( handle, 0 ) ) {
    case WAIT_ABANDONED:    // 0x80
        qSystemWarning( "QRealMutexPrivate::locked() - ::WaitForSingleObject() returned WAIT_ABANDONED" );
        return false;
    case WAIT_TIMEOUT:      // 0x102
        return false;
    case WAIT_FAILED:       // ~0U
        return false;
    default:
        criticalsection.enter();
        owner = GetCurrentThreadId();
        criticalsection.leave();
        break;
    }
    return true;
}

bool QRealMutexPrivate::trylock()
{
    criticalsection.enter();
    if ( GetCurrentThreadId() == owner ) {
        criticalsection.leave();
        return false;
    }
    criticalsection.leave();

    switch ( ::WaitForSingleObject( handle, 0 ) ) {
    case WAIT_FAILED:
    case WAIT_TIMEOUT:
        return false;
    case WAIT_ABANDONED_0:
        qSystemWarning( "QRealMutexPrivate::trylock() - ::WaitForSingleObject() failed" );
        return false;
    default:
        criticalsection.enter();
        owner = GetCurrentThreadId();
        criticalsection.leave();
        break;
    }
    return true;
}

/*!
    \class QMutex qmutex.h
    \threadsafe
    \brief The QMutex class provides access serialization between threads.

    \ingroup thread
    \ingroup environment

    The purpose of a QMutex is to protect an object, data structure or
    section of code so that only one thread can access it at a time
    (This is similar to the Java \c synchronized keyword). For
    example, say there is a method which prints a message to the user
    on two lines:

    \code
    int number = 6;

    void method1()
    {
        number *= 5;
	number /= 4;
    }

    void method1()
    {
        number *= 3;
	number /= 2;
    }
    \endcode

    If these two methods are called in succession, the following happens:

    \code
    // method1()
    number *= 5;	// number is now 30
    number /= 4;	// number is now 7

    // method2()
    number *= 3;	// nubmer is now 21
    number /= 2;	// number is now 10
    \endcode

    If these two methods are called simultaneously from two threads then the
    following sequence could result:

    \code
    // Thread 1 calls method1()
    number *= 5;	// number is now 30

    // Thread 2 calls method2().
    //
    // Most likely Thread 1 has been put to sleep by the operating
    // system to allow Thread 2 to run.
    number *= 3;	// number is now 90
    number /= 2;	// number is now 45

    // Thread 1 finishes executing.
    number /= 4;	// number is now 11, instead of 10
    \endcode

    If we add a mutex, we should get the result we want:

    \code
    QMutex mutex;
    int number = 6;

    void method1()
    {
	mutex.lock();
        number *= 5;
	number /= 4;
	mutex.unlock();
    }

    void method2()
    {
	mutex.lock();
        number *= 3;
	number /= 2;
	mutex.unlock();
    }
    \endcode

    Then only one thread can modify \c number at any given time and
    the result is correct. This is a trivial example, of course, but
    applies to any other case where things need to happen in a
    particular sequence.

    When you call lock() in a thread, other threads that try to call
    lock() in the same place will block until the thread that got the
    lock calls unlock(). A non-blocking alternative to lock() is
    tryLock().
*/

/*!
    Constructs a new mutex. The mutex is created in an unlocked state.
    A recursive mutex is created if \a recursive is TRUE; a normal
    mutex is created if \a recursive is FALSE (the default). With a
    recursive mutex, a thread can lock the same mutex multiple times
    and it will not be unlocked until a corresponding number of
    unlock() calls have been made.
*/
QMutex::QMutex( bool recursive )
{
    if ( recursive )
        d = new QRecursiveMutexPrivate();
    else
        d = new QRealMutexPrivate();
}

/*!
    Destroys the mutex.

    \warning If you destroy a mutex that still holds a lock the
    resultant behavior is undefined.
*/
QMutex::~QMutex()
{
    delete d;
}

/*!
    Attempt to lock the mutex. If another thread has locked the mutex
    then this call will \e block until that thread has unlocked it.

    \sa unlock(), locked()
*/
void QMutex::lock()
{
    d->lock();
}

/*!
    Unlocks the mutex. Attempting to unlock a mutex in a different
    thread to the one that locked it results in an error. Unlocking a
    mutex that is not locked results in undefined behaviour (varies
    between different Operating Systems' thread implementations).

    \sa lock(), locked()
*/
void QMutex::unlock()
{
    d->unlock();
}

/*!
    Returns TRUE if the mutex is locked by another thread; otherwise
    returns FALSE.

    \warning Due to differing implementations of recursive mutexes on
    various platforms, calling this function from the same thread that
    previously locked the mutex will return undefined results.

    \sa lock(), unlock()
*/
bool QMutex::locked()
{
    return d->locked();
}

/*!
    Attempt to lock the mutex. If the lock was obtained, this function
    returns TRUE. If another thread has locked the mutex, this
    function returns FALSE, instead of waiting for the mutex to become
    available, i.e. it does not block.

    If the lock was obtained, the mutex must be unlocked with unlock()
    before another thread can successfully lock it.

    \sa lock(), unlock(), locked()
*/
bool QMutex::tryLock()
{
    return d->trylock();
}

/*!
    \class QMutexLocker qmutex.h
    \brief The QMutexLocker class simplifies locking and unlocking QMutexes.

    \threadsafe

    \ingroup thread
    \ingroup environment

    The purpose of QMutexLocker is to simplify QMutex locking and
    unlocking. Locking and unlocking a QMutex in complex functions and
    statements or in exception handling code is error prone and
    difficult to debug. QMutexLocker should be used in such situations
    to ensure that the state of the mutex is well defined and always
    locked and unlocked properly.

    QMutexLocker should be created within a function where a QMutex
    needs to be locked. The mutex is locked when QMutexLocker is
    created, and unlocked when QMutexLocker is destroyed.

    For example, this complex function locks a QMutex upon entering
    the function and unlocks the mutex at all the exit points:

    \code
    int complexFunction( int flag )
    {
	mutex.lock();

	int return_value = 0;

	switch ( flag ) {
	case 0:
	case 1:
	    {
		mutex.unlock();
		return moreComplexFunction( flag );
	    }

	case 2:
	    {
		int status = anotherFunction();
		if ( status < 0 ) {
		    mutex.unlock();
		    return -2;
		}
		return_value = status + flag;
		break;
	    }

	default:
	    {
		if ( flag > 10 ) {
		    mutex.unlock();
		    return -1;
		}
		break;
	    }
	}

	mutex.unlock();
	return return_value;
    }
    \endcode

    This example function will get more complicated as it is
    developed, which increases the likelihood that errors will occur.

    Using QMutexLocker greatly simplifies the code, and makes it more
    readable:

    \code
    int complexFunction( int flag )
    {
	QMutexLocker locker( &mutex );

	int return_value = 0;

	switch ( flag ) {
	case 0:
	case 1:
	    {
		return moreComplexFunction( flag );
	    }

	case 2:
	    {
		int status = anotherFunction();
		if ( status < 0 )
		    return -2;
		return_value = status + flag;
		break;
	    }

	default:
	    {
		if ( flag > 10 )
		    return -1;
		break;
	    }
	}

	return return_value;
    }
    \endcode

    Now, the mutex will always be unlocked when the QMutexLocker
    object is destroyed (when the function returns since \c locker is
    an auto variable).

    The same principle applies to code that throws and catches
    exceptions. An exception that is not caught in the function that
    has locked the mutex has no way of unlocking the mutex before the
    exception is passed up the stack to the calling function.

    QMutexLocker also provides a mutex() member function that returns
    the mutex on which the QMutexLocker is operating. This is useful
    for code that needs access to the mutex, such as
    QWaitCondition::wait(). For example:

    \code
    class SignalWaiter
    {
    private:
	QMutexLocker locker;

    public:
	SignalWaiter( QMutex *mutex )
	    : locker( mutex )
	{
	}

	void waitForSignal()
	{
	    ...
	    ...
	    ...

	    while ( ! signalled )
		waitcondition.wait( locker.mutex() );

	    ...
	    ...
	    ...
	}
    };
    \endcode

    \sa QMutex, QWaitCondition
*/

/*!
    \fn QMutexLocker::QMutexLocker( QMutex *mutex )

    Constructs a QMutexLocker and locks \a mutex. The mutex will be
    unlocked when the QMutexLocker is destroyed.

    \sa QMutex::lock()
*/

/*!
    \fn QMutexLocker::~QMutexLocker()

    Destroys the QMutexLocker and unlocks the mutex which was locked
    in the constructor.

    \sa QMutexLocker::QMutexLocker(), QMutex::unlock()
*/

/*!
    \fn QMutex *QMutexLocker::mutex() const

    Returns a pointer to the mutex which was locked in the
    constructor.

    \sa QMutexLocker::QMutexLocker()
*/

#endif // QT_THREAD_SUPPORT
