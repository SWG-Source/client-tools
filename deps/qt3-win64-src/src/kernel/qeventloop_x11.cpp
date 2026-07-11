/****************************************************************************
** $Id: qeventloop_x11.cpp 2051 2007-02-21 10:04:20Z chehrlic $
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
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
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
#include "qcolor_p.h"
#include "qt_x11_p.h"

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif // QT_THREAD_SUPPORT

#include <errno.h>


// resolve the conflict between X11's FocusIn and QEvent::FocusIn
#undef FocusOut
#undef FocusIn

static const int XKeyPress = KeyPress;
static const int XKeyRelease = KeyRelease;
#undef KeyPress
#undef KeyRelease

// from qapplication.cpp
extern bool qt_is_gui_used;

// from qeventloop_unix.cpp
extern timeval *qt_wait_timer();
extern void cleanupTimers();

// ### this needs to go away at some point...
typedef void (*VFPTR)();
typedef QValueList<VFPTR> QVFuncList;
void qt_install_preselect_handler( VFPTR );
void qt_remove_preselect_handler( VFPTR );
static QVFuncList *qt_preselect_handler = 0;
void qt_install_postselect_handler( VFPTR );
void qt_remove_postselect_handler( VFPTR );
static QVFuncList *qt_postselect_handler = 0;

void qt_install_preselect_handler( VFPTR handler )
{
    if ( !qt_preselect_handler )
	qt_preselect_handler = new QVFuncList;
    qt_preselect_handler->append( handler );
}
void qt_remove_preselect_handler( VFPTR handler )
{
    if ( qt_preselect_handler ) {
	QVFuncList::Iterator it = qt_preselect_handler->find( handler );
	if ( it != qt_preselect_handler->end() )
		qt_preselect_handler->remove( it );
    }
}
void qt_install_postselect_handler( VFPTR handler )
{
    if ( !qt_postselect_handler )
	qt_postselect_handler = new QVFuncList;
    qt_postselect_handler->prepend( handler );
}
void qt_remove_postselect_handler( VFPTR handler )
{
    if ( qt_postselect_handler ) {
	QVFuncList::Iterator it = qt_postselect_handler->find( handler );
	if ( it != qt_postselect_handler->end() )
		qt_postselect_handler->remove( it );
    }
}


void QEventLoop::init()
{
    // initialize the common parts of the event loop
    pipe( d->thread_pipe );
    fcntl(d->thread_pipe[0], F_SETFD, FD_CLOEXEC);
    fcntl(d->thread_pipe[1], F_SETFD, FD_CLOEXEC);

    d->sn_highest = -1;

    // intitialize the X11 parts of the event loop
    d->xfd = -1;
    if ( qt_is_gui_used )
        d->xfd = XConnectionNumber( QPaintDevice::x11AppDisplay() );
}

void QEventLoop::cleanup()
{
    // cleanup the common parts of the event loop
    close( d->thread_pipe[0] );
    close( d->thread_pipe[1] );
    cleanupTimers();

    // cleanup the X11 parts of the event loop
    d->xfd = -1;
}

bool QEventLoop::processEvents( ProcessEventsFlags flags )
{
    // process events from the X server
    XEvent event;
    int	   nevents = 0;

#if defined(QT_THREAD_SUPPORT)
    QMutexLocker locker( QApplication::qt_mutex );
#endif

    // handle gui and posted events
    if ( qt_is_gui_used ) {
	QApplication::sendPostedEvents();

	// Two loops so that posted events accumulate
	while ( XPending( QPaintDevice::x11AppDisplay() ) ) {
	    // also flushes output buffer
	    while ( XPending( QPaintDevice::x11AppDisplay() ) ) {
		if ( d->shortcut ) {
		    return FALSE;
		}

		XNextEvent( QPaintDevice::x11AppDisplay(), &event );

		if ( flags & ExcludeUserInput ) {
		    switch ( event.type ) {
		    case ButtonPress:
		    case ButtonRelease:
		    case MotionNotify:
		    case XKeyPress:
		    case XKeyRelease:
		    case EnterNotify:
		    case LeaveNotify:
			continue;

		    case ClientMessage:
			{
			    // from qapplication_x11.cpp
			    extern Atom qt_wm_protocols;
			    extern Atom qt_wm_take_focus;
			    extern Atom qt_qt_scrolldone;

			    // only keep the wm_take_focus and
			    // qt_qt_scrolldone protocols, discard all
			    // other client messages
			    if ( event.xclient.format != 32 )
				continue;

			    if ( event.xclient.message_type == qt_wm_protocols ||
				 (Atom) event.xclient.data.l[0] == qt_wm_take_focus )
				break;
			    if ( event.xclient.message_type == qt_qt_scrolldone )
				break;
			}

		    default: break;
		    }
		}

		nevents++;
		if ( qApp->x11ProcessEvent( &event ) == 1 )
		    return TRUE;
	    }
	}
    }

    if ( d->shortcut ) {
	return FALSE;
    }

    QApplication::sendPostedEvents();

    const uint exclude_all = ExcludeSocketNotifiers | 0x08;
    // 0x08 == ExcludeTimers for X11 only
    if ( nevents > 0 && ( flags & exclude_all ) == exclude_all &&
	 ( flags & WaitForMore ) ) {
	return TRUE;
    }

    // don't block if exitLoop() or exit()/quit() has been called.
    bool canWait = d->exitloop || d->quitnow ? FALSE : (flags & WaitForMore);

    // Process timers and socket notifiers - the common UNIX stuff

    // return the maximum time we can wait for an event.
    static timeval zerotm;
    timeval *tm = 0;
    if ( ! ( flags & 0x08 ) ) {			// 0x08 == ExcludeTimers for X11 only
	tm = qt_wait_timer();			// wait for timer or X event
	if ( !canWait ) {
	    if ( !tm )
		tm = &zerotm;
	    tm->tv_sec  = 0;			// no time to wait
	    tm->tv_usec = 0;
	}
    }

    int highest = 0;
    if ( ! ( flags & ExcludeSocketNotifiers ) ) {
	// return the highest fd we can wait for input on
	if ( d->sn_highest >= 0 ) {			// has socket notifier(s)
	    if ( d->sn_vec[0].list && ! d->sn_vec[0].list->isEmpty() )
		d->sn_vec[0].select_fds = d->sn_vec[0].enabled_fds;
	    else
		FD_ZERO( &d->sn_vec[0].select_fds );

	    if ( d->sn_vec[1].list && ! d->sn_vec[1].list->isEmpty() )
		d->sn_vec[1].select_fds = d->sn_vec[1].enabled_fds;
	    else
		FD_ZERO( &d->sn_vec[1].select_fds );

	    if ( d->sn_vec[2].list && ! d->sn_vec[2].list->isEmpty() )
		d->sn_vec[2].select_fds = d->sn_vec[2].enabled_fds;
	    else
		FD_ZERO( &d->sn_vec[2].select_fds );
	} else {
	    FD_ZERO( &d->sn_vec[0].select_fds );

	    FD_ZERO( &d->sn_vec[1].select_fds );
	    FD_ZERO( &d->sn_vec[2].select_fds );
	}

	highest = d->sn_highest;
    } else {
        FD_ZERO( &d->sn_vec[0].select_fds );
	FD_ZERO( &d->sn_vec[1].select_fds );
	FD_ZERO( &d->sn_vec[2].select_fds );
    }

    if ( qt_is_gui_used ) {
	// select for events on the event socket - only on X11
	FD_SET( d->xfd, &d->sn_vec[0].select_fds );
	highest = QMAX( highest, d->xfd );
    }

    FD_SET( d->thread_pipe[0], &d->sn_vec[0].select_fds );
    highest = QMAX( highest, d->thread_pipe[0] );

    if ( canWait )
	emit aboutToBlock();

    if ( qt_preselect_handler ) {
	QVFuncList::Iterator it, end = qt_preselect_handler->end();
	for ( it = qt_preselect_handler->begin(); it != end; ++it )
	    (**it)();
    }

    // unlock the GUI mutex and select.  when we return from this function, there is
    // something for us to do
#if defined(QT_THREAD_SUPPORT)
    locker.mutex()->unlock();
#endif

    int nsel;
    do {
	nsel = select( highest + 1,
		       &d->sn_vec[0].select_fds,
		       &d->sn_vec[1].select_fds,
		       &d->sn_vec[2].select_fds,
		       tm );
    } while (nsel == -1 && (errno == EINTR || errno == EAGAIN));

    // relock the GUI mutex before processing any pending events
#if defined(QT_THREAD_SUPPORT)
    locker.mutex()->lock();
#endif

    // we are awake, broadcast it
    emit awake();
    emit qApp->guiThreadAwake();

    if (nsel == -1) {
	if (errno == EBADF) {
	    // it seems a socket notifier has a bad fd... find out
	    // which one it is and disable it
	    fd_set fdset;
	    zerotm.tv_sec = zerotm.tv_usec = 0l;

	    for (int type = 0; type < 3; ++type) {
		QPtrList<QSockNot> *list = d->sn_vec[type].list;
		if (!list) continue;

		QSockNot *sn = list->first();
		while (sn) {
		    FD_ZERO(&fdset);
		    FD_SET(sn->fd, &fdset);

		    int ret = -1;
		    do {
			switch (type) {
			case 0: // read
			    ret = select(sn->fd + 1, &fdset, 0, 0, &zerotm);
			    break;
			case 1: // write
			    ret = select(sn->fd + 1, 0, &fdset, 0, &zerotm);
			    break;
			case 2: // except
			    ret = select(sn->fd + 1, 0, 0, &fdset, &zerotm);
			    break;
			}
		    } while (ret == -1 && (errno == EINTR || errno == EAGAIN));

		    if (ret == -1 && errno == EBADF) {
			// disable the invalid socket notifier
			static const char *t[] = { "Read", "Write", "Exception" };
			qWarning("QSocketNotifier: invalid socket %d and type '%s', disabling...",
				 sn->fd, t[type]);
			sn->obj->setEnabled(FALSE);
		    }

		    sn = list->next();
		}
	    }
	} else {
	    // EINVAL... shouldn't happen, so let's complain to stderr
	    // and hope someone sends us a bug report
	    perror( "select" );
	}
    }

    // some other thread woke us up... consume the data on the thread pipe so that
    // select doesn't immediately return next time
    if ( nsel > 0 && FD_ISSET( d->thread_pipe[0], &d->sn_vec[0].select_fds ) ) {
	char c;
	::read( d->thread_pipe[0], &c, 1 );
    }

    if ( qt_postselect_handler ) {
	QVFuncList::Iterator it, end = qt_postselect_handler->end();
	for ( it = qt_postselect_handler->begin(); it != end; ++it )
	    (**it)();
    }

    // activate socket notifiers
    if ( ! ( flags & ExcludeSocketNotifiers ) && nsel > 0 && d->sn_highest >= 0 ) {
	// if select says data is ready on any socket, then set the socket notifier
	// to pending
	int i;
	for ( i=0; i<3; i++ ) {
	    if ( ! d->sn_vec[i].list )
		continue;

	    QPtrList<QSockNot> *list = d->sn_vec[i].list;
	    QSockNot *sn = list->first();
	    while ( sn ) {
		if ( FD_ISSET( sn->fd, &d->sn_vec[i].select_fds ) )
		    setSocketNotifierPending( sn->obj );
		sn = list->next();
	    }
	}

	nevents += activateSocketNotifiers();
    }

    // activate timers
    if ( ! ( flags & 0x08 ) ) {
	// 0x08 == ExcludeTimers for X11 only
	nevents += activateTimers();
    }

    // color approx. optimization - only on X11
    qt_reset_color_avail();

    // return true if we handled events, false otherwise
    return (nevents > 0);
}

bool QEventLoop::hasPendingEvents() const
{
    extern uint qGlobalPostedEventsCount(); // from qapplication.cpp
    return ( qGlobalPostedEventsCount() || XPending( QPaintDevice::x11AppDisplay() ) );
}

void QEventLoop::appStartingUp()
{
    if ( qt_is_gui_used )
        d->xfd = XConnectionNumber( QPaintDevice::x11AppDisplay() );
}

void QEventLoop::appClosingDown()
{
    d->xfd = -1;
}
