/****************************************************************************
** $Id: qeventloop_unix.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QEventLoop class
**
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
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

#include "qeventloop_p.h" // includes qplatformdefs.h
#include "qeventloop.h"
#include "qapplication.h"
#include "qbitarray.h"
#include <stdlib.h>
#include <sys/types.h>


/*****************************************************************************
  Timer handling; UNIX has no application timer support so we'll have to
  make our own from scratch.

  NOTE: These functions are for internal use. QObject::startTimer() and
	QObject::killTimer() are for public use.
	The QTimer class provides a high-level interface which translates
	timer events into signals.

  qStartTimer( interval, obj )
	Starts a timer which will run until it is killed with qKillTimer()
	Arguments:
	    int interval	timer interval in milliseconds
	    QObject *obj	where to send the timer event
	Returns:
	    int			timer identifier, or zero if not successful

  qKillTimer( timerId )
	Stops a timer specified by a timer identifier.
	Arguments:
	    int timerId		timer identifier
	Returns:
	    bool		TRUE if successful

  qKillTimer( obj )
	Stops all timers that are sent to the specified object.
	Arguments:
	    QObject *obj	object receiving timer events
	Returns:
	    bool		TRUE if successful
 *****************************************************************************/

//
// Internal data structure for timers
//

struct TimerInfo {				// internal timer info
    int	     id;				// - timer identifier
    timeval  interval;				// - timer interval
    timeval  timeout;				// - when to sent event
    QObject *obj;				// - object to receive event
};

typedef QPtrList<TimerInfo> TimerList;	// list of TimerInfo structs

static QBitArray *timerBitVec;			// timer bit vector
static TimerList *timerList	= 0;		// timer list

static void	initTimers();
void cleanupTimers();
static timeval	watchtime;			// watch if time is turned back
timeval		*qt_wait_timer();
timeval	*qt_wait_timer_max = 0;

//
// Internal operator functions for timevals
//

static inline bool operator<( const timeval &t1, const timeval &t2 )
{
    return t1.tv_sec < t2.tv_sec ||
	  (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
}

static inline bool operator==( const timeval &t1, const timeval &t2 )
{
    return t1.tv_sec == t2.tv_sec && t1.tv_usec == t2.tv_usec;
}

static inline timeval &operator+=( timeval &t1, const timeval &t2 )
{
    t1.tv_sec += t2.tv_sec;
    if ( (t1.tv_usec += t2.tv_usec) >= 1000000 ) {
	t1.tv_sec++;
	t1.tv_usec -= 1000000;
    }
    return t1;
}

static inline timeval operator+( const timeval &t1, const timeval &t2 )
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec + t2.tv_sec;
    if ( (tmp.tv_usec = t1.tv_usec + t2.tv_usec) >= 1000000 ) {
	tmp.tv_sec++;
	tmp.tv_usec -= 1000000;
    }
    return tmp;
}

static inline timeval operator-( const timeval &t1, const timeval &t2 )
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec - t2.tv_sec;
    if ( (tmp.tv_usec = t1.tv_usec - t2.tv_usec) < 0 ) {
	tmp.tv_sec--;
	tmp.tv_usec += 1000000;
    }
    return tmp;
}


//
// Internal functions for manipulating timer data structures.
// The timerBitVec array is used for keeping track of timer identifiers.
//

static int allocTimerId()			// find avail timer identifier
{
    int i = timerBitVec->size()-1;
    while ( i >= 0 && (*timerBitVec)[i] )
	i--;
    if ( i < 0 ) {
	i = timerBitVec->size();
	timerBitVec->resize( 4 * i );
	for( int j=timerBitVec->size()-1; j > i; j-- )
	    timerBitVec->clearBit( j );
    }
    timerBitVec->setBit( i );
    return i+1;
}

static void insertTimer( const TimerInfo *ti )	// insert timer info into list
{
    TimerInfo *t = timerList->first();
    int index = 0;
#if defined(QT_DEBUG)
    int dangerCount = 0;
#endif
    while ( t && t->timeout < ti->timeout ) {	// list is sorted by timeout
#if defined(QT_DEBUG)
	if ( t->obj == ti->obj )
	    dangerCount++;
#endif
	t = timerList->next();
	index++;
    }
    timerList->insert( index, ti );		// inserts sorted
#if defined(QT_DEBUG)
    if ( dangerCount > 16 )
	qDebug( "QObject: %d timers now exist for object %s::%s",
	       dangerCount, ti->obj->className(), ti->obj->name() );
#endif
}

static inline void getTime( timeval &t )	// get time of day
{
    gettimeofday( &t, 0 );
    while ( t.tv_usec >= 1000000 ) {		// NTP-related fix
	t.tv_usec -= 1000000;
	t.tv_sec++;
    }
    while ( t.tv_usec < 0 ) {
	if ( t.tv_sec > 0 ) {
	    t.tv_usec += 1000000;
	    t.tv_sec--;
	} else {
	    t.tv_usec = 0;
	    break;
	}
    }
}

static void repairTimer( const timeval &time )	// repair broken timer
{
    timeval diff = watchtime - time;
    register TimerInfo *t = timerList->first();
    while ( t ) {				// repair all timers
	t->timeout = t->timeout - diff;
	t = timerList->next();
    }
}

//
// Timer activation functions (called from the event loop)
//

/*
  Returns the time to wait for the next timer, or null if no timers are
  waiting.

  The result is bounded to qt_wait_timer_max if this exists.
*/

timeval *qt_wait_timer()
{
    static timeval tm;
    bool first = TRUE;
    timeval currentTime;
    if ( timerList && timerList->count() ) {	// there are waiting timers
	getTime( currentTime );
	if ( first ) {
	    if ( currentTime < watchtime )	// clock was turned back
		repairTimer( currentTime );
	    first = FALSE;
	    watchtime = currentTime;
	}
	TimerInfo *t = timerList->first();	// first waiting timer
	if ( currentTime < t->timeout ) {	// time to wait
	    tm = t->timeout - currentTime;
	} else {
	    tm.tv_sec  = 0;			// no time to wait
	    tm.tv_usec = 0;
	}
	if ( qt_wait_timer_max && *qt_wait_timer_max < tm )
	    tm = *qt_wait_timer_max;
	return &tm;
    }
    if ( qt_wait_timer_max ) {
	tm = *qt_wait_timer_max;
	return &tm;
    }
    return 0;					// no timers
}

// Timer initialization
static void initTimers()			// initialize timers
{
    timerBitVec = new QBitArray( 128 );
    Q_CHECK_PTR( timerBitVec );
    int i = timerBitVec->size();
    while( i-- > 0 )
	timerBitVec->clearBit( i );
    timerList = new TimerList;
    Q_CHECK_PTR( timerList );
    timerList->setAutoDelete( TRUE );
    gettimeofday( &watchtime, 0 );
}

// Timer cleanup
void cleanupTimers()
{
    delete timerList;
    timerList = 0;
    delete timerBitVec;
    timerBitVec = 0;
}

// Main timer functions for starting and killing timers
int qStartTimer( int interval, QObject *obj )
{
    if ( !timerList )				// initialize timer data
	initTimers();
    int id = allocTimerId();			// get free timer id
    if ( id <= 0 ||
	 id > (int)timerBitVec->size() || !obj )// cannot create timer
	return 0;
    timerBitVec->setBit( id-1 );		// set timer active
    TimerInfo *t = new TimerInfo;		// create timer
    Q_CHECK_PTR( t );
    t->id = id;
    t->interval.tv_sec  = interval/1000;
    t->interval.tv_usec = (interval%1000)*1000;
    timeval currentTime;
    getTime( currentTime );
    t->timeout = currentTime + t->interval;
    t->obj = obj;
    insertTimer( t );				// put timer in list
    return id;
}

bool qKillTimer( int id )
{
    register TimerInfo *t;
    if ( !timerList || id <= 0 ||
	 id > (int)timerBitVec->size() || !timerBitVec->testBit( id-1 ) )
	return FALSE;				// not init'd or invalid timer
    t = timerList->first();
    while ( t && t->id != id )			// find timer info in list
	t = timerList->next();
    if ( t ) {					// id found
	timerBitVec->clearBit( id-1 );		// set timer inactive
	return timerList->remove();
    }
    else					// id not found
	return FALSE;
}

bool qKillTimer( QObject *obj )
{
    register TimerInfo *t;
    if ( !timerList )				// not initialized
	return FALSE;
    t = timerList->first();
    while ( t ) {				// check all timers
	if ( t->obj == obj ) {			// object found
	    timerBitVec->clearBit( t->id-1 );
	    timerList->remove();
	    t = timerList->current();
	} else {
	    t = timerList->next();
	}
    }
    return TRUE;
}

/*****************************************************************************
 Socket notifier type
 *****************************************************************************/
QSockNotType::QSockNotType()
    : list( 0 )
{
    FD_ZERO( &select_fds );
    FD_ZERO( &enabled_fds );
    FD_ZERO( &pending_fds );
}

QSockNotType::~QSockNotType()
{
    if ( list )
	delete list;
    list = 0;
}

/*****************************************************************************
 QEventLoop implementations for UNIX
 *****************************************************************************/
void QEventLoop::registerSocketNotifier( QSocketNotifier *notifier )
{
    int sockfd = notifier->socket();
    int type = notifier->type();
    if ( sockfd < 0 || sockfd >= FD_SETSIZE || type < 0 || type > 2 || notifier == 0 ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QSocketNotifier: Internal error" );
#endif
	return;
    }

    QPtrList<QSockNot>  *list = d->sn_vec[type].list;
    fd_set *fds  = &d->sn_vec[type].enabled_fds;
    QSockNot *sn;

    if ( ! list ) {
	// create new list, the QSockNotType destructor will delete it for us
	list = new QPtrList<QSockNot>;
	Q_CHECK_PTR( list );
	list->setAutoDelete( TRUE );
	d->sn_vec[type].list = list;
    }

    sn = new QSockNot;
    Q_CHECK_PTR( sn );
    sn->obj = notifier;
    sn->fd = sockfd;
    sn->queue = &d->sn_vec[type].pending_fds;

    if ( list->isEmpty() ) {
	list->insert( 0, sn );
    } else {				// sort list by fd, decreasing
	QSockNot *p = list->first();
	while ( p && p->fd > sockfd )
	    p = list->next();
#if defined(QT_CHECK_STATE)
	if ( p && p->fd == sockfd ) {
	    static const char *t[] = { "read", "write", "exception" };
	    qWarning( "QSocketNotifier: Multiple socket notifiers for "
		      "same socket %d and type %s", sockfd, t[type] );
	}
#endif
	if ( p )
	    list->insert( list->at(), sn );
	else
	    list->append( sn );
    }

    FD_SET( sockfd, fds );
    d->sn_highest = QMAX( d->sn_highest, sockfd );
}

void QEventLoop::unregisterSocketNotifier( QSocketNotifier *notifier )
{
    int sockfd = notifier->socket();
    int type = notifier->type();
    if ( sockfd < 0 || type < 0 || type > 2 || notifier == 0 ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QSocketNotifier: Internal error" );
#endif
	return;
    }

    QPtrList<QSockNot> *list = d->sn_vec[type].list;
    fd_set *fds  =  &d->sn_vec[type].enabled_fds;
    QSockNot *sn;
    if ( ! list )
	return;
    sn = list->first();
    while ( sn && !(sn->obj == notifier && sn->fd == sockfd) )
	sn = list->next();
    if ( !sn ) // not found
	return;

    FD_CLR( sockfd, fds );			// clear fd bit
    FD_CLR( sockfd, sn->queue );
    d->sn_pending_list.removeRef( sn );		// remove from activation list
    list->remove();				// remove notifier found above

    if ( d->sn_highest == sockfd ) {		// find highest fd
	d->sn_highest = -1;
	for ( int i=0; i<3; i++ ) {
	    if ( d->sn_vec[i].list && ! d->sn_vec[i].list->isEmpty() )
		d->sn_highest = QMAX( d->sn_highest,  // list is fd-sorted
				      d->sn_vec[i].list->getFirst()->fd );
	}
    }
}

void QEventLoop::setSocketNotifierPending( QSocketNotifier *notifier )
{
    int sockfd = notifier->socket();
    int type = notifier->type();
    if ( sockfd < 0 || type < 0 || type > 2 || notifier == 0 ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QSocketNotifier: Internal error" );
#endif
	return;
    }

    QPtrList<QSockNot> *list = d->sn_vec[type].list;
    QSockNot *sn;
    if ( ! list )
	return;
    sn = list->first();
    while ( sn && !(sn->obj == notifier && sn->fd == sockfd) )
	sn = list->next();
    if ( ! sn ) { // not found
	return;
    }

    // We choose a random activation order to be more fair under high load.
    // If a constant order is used and a peer early in the list can
    // saturate the IO, it might grab our attention completely.
    // Also, if we're using a straight list, the callback routines may
    // delete other entries from the list before those other entries are
    // processed.
    if ( ! FD_ISSET( sn->fd, sn->queue ) ) {
	d->sn_pending_list.insert( (rand() & 0xff) %
				   (d->sn_pending_list.count()+1), sn );
	FD_SET( sn->fd, sn->queue );
    }
}

void QEventLoop::wakeUp()
{
    /*
      Apparently, there is not consistency among different operating
      systems on how to use FIONREAD.

      FreeBSD, Linux and Solaris all expect the 3rd argument to
      ioctl() to be an int, which is normally 32-bit even on 64-bit
      machines.

      IRIX, on the other hand, expects a size_t, which is 64-bit on
      64-bit machines.

      So, the solution is to use size_t initialized to zero to make
      sure all bits are set to zero, preventing underflow with the
      FreeBSD/Linux/Solaris ioctls.
    */
    size_t nbytes = 0;
    char c = 0;
    if ( ::ioctl( d->thread_pipe[0], FIONREAD, (char*)&nbytes ) >= 0 && nbytes == 0 ) {
	::write(  d->thread_pipe[1], &c, 1  );
    }
}

int QEventLoop::timeToWait() const
{
    timeval *tm = qt_wait_timer();
    if ( ! tm )	// no active timers
	return -1;
    return (tm->tv_sec*1000) + (tm->tv_usec/1000);
}

int QEventLoop::activateTimers()
{
    if ( !timerList || !timerList->count() )	// no timers
	return 0;
    bool first = TRUE;
    timeval currentTime;
    int n_act = 0, maxCount = timerList->count();
    TimerInfo *begin = 0;
    register TimerInfo *t;

    for ( ;; ) {
	if ( ! maxCount-- )
	    break;
	getTime( currentTime );			// get current time
	if ( first ) {
	    if ( currentTime < watchtime )	// clock was turned back
		repairTimer( currentTime );
	    first = FALSE;
	    watchtime = currentTime;
	}
	t = timerList->first();
	if ( !t || currentTime < t->timeout )	// no timer has expired
	    break;
	if ( ! begin ) {
	    begin = t;
	} else if ( begin == t ) {
	    // avoid sending the same timer multiple times
	    break;
	} else if ( t->interval <  begin->interval || t->interval == begin->interval ) {
	    begin = t;
	}
	timerList->take();			// unlink from list
	t->timeout += t->interval;
	if ( t->timeout < currentTime )
	    t->timeout = currentTime + t->interval;
	insertTimer( t );			// relink timer
	if ( t->interval.tv_usec > 0 || t->interval.tv_sec > 0 )
	    n_act++;
	QTimerEvent e( t->id );
	QApplication::sendEvent( t->obj, &e );	// send event
	if ( timerList->findRef( begin ) == -1 )
	    begin = 0;
    }
    return n_act;
}

int QEventLoop::activateSocketNotifiers()
{
    if ( d->sn_pending_list.isEmpty() )
	return 0;

    // activate entries
    int n_act = 0;
    QEvent event( QEvent::SockAct );
    QPtrListIterator<QSockNot> it( d->sn_pending_list );
    QSockNot *sn;
    while ( (sn=it.current()) ) {
	++it;
	d->sn_pending_list.removeRef( sn );
	if ( FD_ISSET(sn->fd, sn->queue) ) {
	    FD_CLR( sn->fd, sn->queue );
	    QApplication::sendEvent( sn->obj, &event );
	    n_act++;
	}
    }

    return n_act;
}
