/****************************************************************************
** $Id: qeventloop.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QEventLoop class
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

#include "qeventloop_p.h" // includes qplatformdefs.h
#include "qeventloop.h"
#include "qapplication.h"
#include "qdatetime.h"

/*!
    \class QEventLoop
    \brief The QEventLoop class manages the event queue.

    \ingroup application
    \ingroup events

    It receives events from the window system and other sources.  It
    then sends them to QApplication for processing and delivery.

    QEventLoop allows the application programmer to have more control
    over event delivery.  Programs that perform long operations can
    call either processOneEvent() or processEvents() with various
    ProcessEvent values OR'ed together to control which events should
    be delivered.

    QEventLoop also allows the integration of an external event loop
    with the Qt event loop.  The Motif Extension included with Qt
    includes a reimplementation of QEventLoop for merging Qt and Motif
    events together.

    To use your own instance of QEventLoop or QEventLoop subclass create 
    it before you create the QApplication object.
*/

/*! \enum QEventLoop::ProcessEvents

    This enum controls the types of events processed by the
    processEvents() functions.

    \value AllEvents - All events are processed
    \value ExcludeUserInput - Do not process user input events.
           ( ButtonPress, KeyPress, etc. )
    \value ExcludeSocketNotifiers - Do not process socket notifier
           events.
    \value WaitForMore - Wait for events if no pending events
           are available.

    \sa processEvents()
*/

/*! \enum QEventLoop::ProcessEventsFlags
    A \c typedef to allow various ProcessEvents values to be OR'ed together.

    \sa ProcessEvents
 */

/*!
    Creates a QEventLoop object, this object becomes the global event loop object.
    There can only be one event loop object. The QEventLoop is usually constructed 
    by calling QApplication::eventLoop(). To create your own event loop object create 
    it before you instantiate the QApplication object.
    
    The \a parent and \a name arguments are passed on to the QObject constructor.
*/
QEventLoop::QEventLoop( QObject *parent, const char *name )
    : QObject( parent, name )
{
#if defined(QT_CHECK_STATE)
    if ( QApplication::eventloop )
	qFatal( "QEventLoop: there must be only one event loop object. \nConstruct it before QApplication." );
    // for now ;)
#endif // QT_CHECK_STATE

    d = new QEventLoopPrivate;

    init();
    QApplication::eventloop = this;
}

/*!
    Destructs the QEventLoop object.
*/
QEventLoop::~QEventLoop()
{
    cleanup();
    delete d;
    QApplication::eventloop = 0;
}

/*!
    Enters the main event loop and waits until exit() is called, and
    returns the value that was set to exit().

    It is necessary to call this function to start event handling. The
    main event loop receives events from the window system and
    dispatches these to the application widgets.

    Generally speaking, no user interaction can take place before
    calling exec(). As a special case, modal widgets like QMessageBox
    can be used before calling exec(), because modal widgets call
    exec() to start a local event loop.

    To make your application perform idle processing, i.e. executing a
    special function whenever there are no pending events, use a
    QTimer with 0 timeout. More advanced idle processing schemes can
    be achieved using processEvents().

    \sa QApplication::quit(), exit(), processEvents()
*/
int QEventLoop::exec()
{
    d->reset();

    enterLoop();

    // cleanup
    d->looplevel = 0;
    d->quitnow  = FALSE;
    d->exitloop = FALSE;
    d->shortcut = FALSE;
    // don't reset quitcode!

    return d->quitcode;
}

/*! \fn void QEventLoop::exit( int retcode = 0 )

    Tells the event loop to exit with a return code.

    After this function has been called, the event loop returns from
    the call to exec(). The exec() function returns \a retcode.

    By convention, a \a retcode of 0 means success, and any non-zero
    value indicates an error.

    Note that unlike the C library function of the same name, this
    function \e does return to the caller -- it is event processing that
    stops.

    \sa QApplication::quit(), exec()
*/
void QEventLoop::exit( int retcode )
{
    if ( d->quitnow ) // preserve existing quitcode
	return;
    d->quitcode = retcode;
    d->quitnow  = TRUE;
    d->exitloop = TRUE;
    d->shortcut = TRUE;
}


/*! \fn int QEventLoop::enterLoop()

    This function enters the main event loop (recursively). Do not call
    it unless you really know what you are doing.
 */
int QEventLoop::enterLoop()
{
    // save the current exitloop state
    bool old_exitloop = d->exitloop;
    d->exitloop = FALSE;
    d->shortcut = FALSE;

    d->looplevel++;
    while ( ! d->exitloop )
	processEvents( AllEvents | WaitForMore );
    d->looplevel--;

    // restore the exitloop state, but if quitnow is TRUE, we need to keep
    // exitloop set so that all other event loops drop out.
    d->exitloop = old_exitloop || d->quitnow;
    d->shortcut = d->quitnow;

    if ( d->looplevel < 1 ) {
	d->quitnow  = FALSE;
	d->exitloop = FALSE;
	d->shortcut = FALSE;
	emit qApp->aboutToQuit();

	// send deferred deletes
	QApplication::sendPostedEvents( 0, QEvent::DeferredDelete );
    }

    return d->looplevel;
}

/*! \fn void QEventLoop::exitLoop()

    This function exits from a recursive call to the main event loop.
    Do not call it unless you really know what you are doing.
*/
void QEventLoop::exitLoop()
{
    d->exitloop = TRUE;
    d->shortcut = TRUE;
}

/*! \fn void QEventLoop::loopLevel() const

    Returns the current loop level.
*/
int QEventLoop::loopLevel() const
{
    return d->looplevel;
}

/*!
    Process pending events that match \a flags for a maximum of \a
    maxTime milliseconds, or until there are no more events to
    process, which ever is shorter.

    This function is especially useful if you have a long running
    operation and want to show its progress without allowing user
    input, i.e. by using the \c ExcludeUserInput flag.

    NOTE: This function will not process events continuously; it
    returns after all available events are processed.

    NOTE: Specifying the \c WaitForMore flag makes no sense and will
    be ignored.
*/
void QEventLoop::processEvents( ProcessEventsFlags flags, int maxTime )
{
    QTime start = QTime::currentTime();
    QTime now;
    while ( ! d->quitnow && processEvents( flags & ~WaitForMore ) ) {
	now = QTime::currentTime();
	if ( start.msecsTo( now ) > maxTime )
	    break;
    }
}

/*!
    \fn bool QEventLoop::processEvents( ProcessEventsFlags flags )
    \overload

    Processes pending events that match \a flags until there are no
    more events to process.

    This function is especially useful if you have a long running
    operation and want to show its progress without allowing user
    input, i.e. by using the \c ExcludeUserInput flag.

    If the \c WaitForMore flag is set in \a flags, the behavior of
    this function is as follows:

    \list

    \i If events are available, this function returns after processing
    them.

    \i If no events are available, this function will wait until more
    are available and return after processing newly available events.

    \endlist

    If the \c WaitForMore flag is \e not set in \a flags, and no
    events are available, this function will return immediately.

    NOTE: This function will not process events continuously; it
    returns after all available events are processed.

    This function returns TRUE if an event was processed; otherwise it
    returns FALSE.

    \sa ProcessEvents hasPendingEvents()
*/

/*! \fn bool QEventLoop::hasPendingEvents() const

    Returns TRUE if there is an event waiting, otherwise it returns FALSE.
*/

/*! \fn void QEventLoop::registerSocketNotifier( QSocketNotifier *notifier )

    Registers \a notifier with the event loop.  Subclasses need to
    reimplement this method to tie a socket notifier into another
    event loop.  Reimplementations \e MUST call the base
    implementation.
*/

/*! \fn void QEventLoop::unregisterSocketNotifier( QSocketNotifier *notifier )

    Unregisters \a notifier from the event loop.  Subclasses need to
    reimplement this method to tie a socket notifier into another
    event loop.  Reimplementations \e MUST call the base
    implementation.
*/

/*! \fn void QEventLoop::setSocketNotifierPending( QSocketNotifier *notifier )

    Marks \a notifier as pending.  The socket notifier will be
    activated the next time activateSocketNotifiers() is called.
*/

/*! \fn int QEventLoop::activateSocketNotifiers()

    Activates all pending socket notifiers and returns the number of
    socket notifiers that were activated.
*/

/*! \fn int QEventLoop::activateTimers()

    Activates all Qt timers and returns the number of timers that were
    activated.

    QEventLoop subclasses that do their own timer handling need to
    call this after the time returned by timeToWait() has elapsed.

    Note: This function is only useful on systems where \c select() is
    used to block the eventloop.  On Windows, this function always
    returns 0.  On MacOS X, this function always returns 0 when the
    GUI is enabled.  On MacOS X, this function returns the documented
    value when the GUI is disabled.
*/

/*! \fn int QEventLoop::timeToWait() const

    Returns the number of milliseconds that Qt needs to handle its
    timers or -1 if there are no timers running.

    QEventLoop subclasses that do their own timer handling need to use
    this to make sure that Qt's timers continue to work.

    Note: This function is only useful on systems where \c select() is
    used to block the eventloop.  On Windows, this function always
    returns -1.  On MacOS X, this function always returns -1 when the
    GUI is enabled.  On MacOS X, this function returns the documented
    value when the GUI is disabled.
*/

/*! \fn void QEventLoop::wakeUp()
    \threadsafe

    Wakes up the event loop.

    \sa awake()
*/

/*! \fn void QEventLoop::awake()

    This signal is emitted after the event loop returns from a
    function that could block.

    \sa wakeUp() aboutToBlock()
*/

/*! \fn void QEventLoop::aboutToBlock()

    This signal is emitted before the event loop calls a function that
    could block.

    \sa awake()
*/

#if !defined(Q_WS_X11)
void QEventLoop::appStartingUp(){}
void QEventLoop::appClosingDown(){}
#endif // Q_WS_X11
