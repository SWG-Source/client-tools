/****************************************************************************
** $Id: qeventloop_win_utils.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QEventLoop class
**
** Created : 20040419
**
** Copyright (C) 2004 Tom and Timi Cecka
** Copyright (C) 2004 Ralf Habacker
** Copyright (C) 2004 Christian Ehrlicher
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
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact kde-cygwin@kde.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
// FIXME: cleanup necessary

#include "qeventloop_p.h" // includes qplatformdefs.h
#include "qeventloop.h"
#include "qapplication.h"
#include "qbitarray.h"
#include "qcolor_p.h"
#include "qt_windows.h"

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif

//#define DEBUG_QEVENTLOOP

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
     int interval timer interval in milliseconds
     QObject *obj where to send the timer event
 Returns:
     int   timer identifier, or zero if not successful

  qKillTimer( timerId )
 Stops a timer specified by a timer identifier.
 Arguments:
     int timerId  timer identifier
 Returns:
     bool  TRUE if successful

  qKillTimer( obj )
 Stops all timers that are sent to the specified object.
 Arguments:
     QObject *obj object receiving timer events
 Returns:
     bool  TRUE if successful
 *****************************************************************************/
//
// Internal data structure for timers
//

struct TimerInfo
{    // internal timer info
    UINT id;    // - timer identifier
    UINT interval;    // - timer interval
    UINT timeout;    // - when to sent event
    QObject *obj;    // - object to receive event
};

typedef QList<TimerInfo> TimerList; // list of TimerInfo structs

//static QBitArray *timerBitVec;   // timer bit vector
static TimerList *timerList = 0;  // timer list

static void initTimers();
//void cleanupTimers();
//static timeval watchtime;   // watch if time is turned back
FILETIME *qt_wait_timer();
FILETIME *qt_wait_timer_max = 0;
void CALLBACK qt_TimerProc( HWND /*hwnd*/, UINT /*uMsg*/, UINT idEvent, DWORD /*dwTime*/ );

//
// Internal functions for manipulating timer data structures.
// The timerBitVec array is used for keeping track of timer identifiers.
//

static int allocTimerId( int interval )       // find avail timer identifier
{
    int iTimerId;
    iTimerId = SetTimer( NULL, 0, ( UINT ) interval, ( TIMERPROC ) qt_TimerProc );
#ifdef DEBUG_QEVENTLOOP
    qDebug( "allocTimerId with id 0x%08x", iTimerId );
#endif
    return iTimerId;
}

static void insertTimer( const TimerInfo *ti )     // insert timer info into list
{
#ifdef DEBUG_QEVENTLOOP
    qDebug( "qeventloop_win.cpp: insertTimer" );
#endif

    TimerInfo *t = timerList->first();
    int index = 0;
#if defined(QT_DEBUG)

    int dangerCount = 0;
#endif

    while ( t && t->timeout < ti->timeout ) { // list is sorted by timeout
#if defined(QT_DEBUG)
        if ( t->obj == ti->obj )
            dangerCount++;
#endif

        t = timerList->next();
        index++;
    }
    timerList->insert( index, ti );  // inserts sorted
#if defined(QT_DEBUG)

    if ( dangerCount > 16 )
        qDebug( "QObject: %d timers now exist for object %s::%s",
                dangerCount, ti->obj->className(), ti->obj->name() );
#endif
}

// Timer initialization
static void initTimers()       // initialize timers
{
#ifdef DEBUG_QEVENTLOOP
    qDebug( "qeventloop_win.cpp: initTimers" );
#endif

    timerList = new TimerList;
    Q_CHECK_PTR( timerList );
    timerList->setAutoDelete( TRUE );
    //gettimeofday( &watchtime, 0 );
}

// Timer cleanup
void cleanupTimers()
{
    delete timerList;
    timerList = 0;
}

// Main timer functions for starting and killing timers
int qStartTimer( int interval, QObject *obj )
{
#ifdef DEBUG_QEVENTLOOP
    qDebug( "qeventloop_win.cpp: qStartTimer" );
#endif

    if ( !timerList )        // initialize timer data
        initTimers();
    if ( !obj )
        return 0;
    int id = allocTimerId( interval );   // get free timer id
    if ( id == 0 )    // cannot create timer
        return 0;
    TimerInfo *t = new TimerInfo;  // create timer
    Q_CHECK_PTR( t );
    t->id = id;
    t->interval = interval;
    t->timeout = GetTickCount() + t->interval;
    t->obj = obj;
    insertTimer( t );    // put timer in list
    return id;
}

bool qKillTimer( int id )
{
#ifdef DEBUG_QEVENTLOOP
    qDebug( "qeventloop_win.cpp: qKillTimer" );
#endif

    register TimerInfo *t;
    if ( !timerList || id <= 0 )
        return FALSE;    // not init'd or invalid timer
    t = timerList->first();
    while ( t && ( int ) t->id != id )       // find timer info in list
        t = timerList->next();
    if ( t ) {     // id found
        KillTimer( NULL, ( UINT ) id );
        return timerList->remove
               ();
    } else     // id not found
        return FALSE;
}

bool qKillTimer( QObject *obj )
{
#ifdef DEBUG_QEVENTLOOP
    qDebug( "qeventloop_win.cpp: qKillTimer" );
#endif

    register TimerInfo *t;
    if ( !timerList )        // not initialized
        return FALSE;
    t = timerList->first();
    while ( t ) {    // check all timers
        if ( t->obj == obj ) {   // object found
            KillTimer( NULL, t->id );
            timerList->remove
            ();
            t = timerList->current();
        } else {
            t = timerList->next();
        }
    }
    return TRUE;
}

#ifdef Q_OS_CYGWIN
/*****************************************************************************
 Socket notifier type
 *****************************************************************************/
QSockNotType::QSockNotType()
        : list( 0 )
{}

QSockNotType::~QSockNotType()
{
    if ( list )
        delete list;
    list = 0;
}
#endif

/*****************************************************************************
QEventLoop implementations for Windows
*****************************************************************************/
void QEventLoop::registerSocketNotifier( QSocketNotifier *notifier )
{
#ifdef DEBUG_QEVENTLOOP
    qDebug( "QEventLoop::registerSocketNotifier(fd: %x type: %d, p: 0x%08p)",
            notifier->socket(), notifier->type(), notifier );
#endif

    int socket = notifier->socket();
    int type = notifier->type();
    if ( socket < 0 || type < 0 || type > 2 || notifier == 0 ) {
        qWarning( "QSocketNotifier: Internal error" );
        return;
    }

    QSNDict *sn_vec[3] = { &sn_read, &sn_write, &sn_except };
    QSNDict *dict = sn_vec[ type ];

    if (dict->contains( socket )) {
        const char *t[] = { "read", "write", "exception" };
        qWarning("QSocketNotifier: Multiple socket notifiers for "
                    "same socket %d and type %s", socket, t[type]);
    }

    QSockNot *sn = new QSockNot;
    sn->obj = notifier;
    sn->fd = socket;
    dict->insert( sn->fd, sn );

#ifndef Q_OS_TEMP
    int sn_event = 0;
    if (sn_read.contains(socket))
        sn_event |= FD_READ | FD_CLOSE | FD_ACCEPT;
    if (sn_write.contains(socket))
        sn_event |= FD_WRITE | FD_CONNECT;
    if (sn_except.contains(socket))
        sn_event |= FD_OOB;
    // BoundsChecker may emit a warning for WSAAsyncSelect when sn_event == 0
    // This is a BoundsChecker bug and not a Qt bug
    WSAAsyncSelect(socket, qt_SocketWindow, sn_event ? qt_SocketMessage : 0, sn_event);
#else
/*
    fd_set        rd,wt,ex;
    FD_ZERO(&rd);
    FD_ZERO(&wt);
    FD_ZERO(&ex);
    if (sn_read && sn_read->find(sockfd))
        FD_SET(sockfd, &rd);
    if (sn_write && sn_write->find(sockfd))
        FD_SET(sockfd, &wt);
    if (sn_except && sn_except->find(sockfd))
        FD_SET(sockfd, &ex);
    select(1, &rd, &wt, &ex, NULL);
*/
#endif
}

void QEventLoop::unregisterSocketNotifier( QSocketNotifier *notifier )
{
#ifdef DEBUG_QEVENTLOOP
    qDebug( "QEventLoop::unregisterSocketNotifier (fd: %x type: %d p: 0x%08p)",
            notifier->socket(), notifier->type(), notifier );
#endif

    int socket = notifier->socket();
    int type = notifier->type();
    if ( socket < 0 || type < 0 || type > 2 || notifier == 0 ) {
        qWarning( "QSocketNotifier: Internal error" );
        return;
    }

    QSNDict *sn_vec[3] = { &sn_read, &sn_write, &sn_except };
    QSNDict *dict = sn_vec[ type ];
    QSNDict::Iterator it = dict->find( socket );
    if( it == dict->end() )
        return;
    QSockNot *sn = it.data();
    if ( !sn )
        return;

    d->sn_pending_list.removeRef( sn );  // remove from activation list
    dict->remove( it );
    delete sn;

#ifndef Q_OS_TEMP // ### This probably needs fixing
    int sn_event = 0;
    if (sn_read.contains(socket))
        sn_event |= FD_READ | FD_CLOSE | FD_ACCEPT;
    if (sn_write.contains(socket))
        sn_event |= FD_WRITE | FD_CONNECT;
    if (sn_except.contains(socket))
        sn_event |= FD_OOB;
    // BoundsChecker may emit a warning for WSAAsyncSelect when sn_event == 0
    // This is a BoundsChecker bug and not a Qt bug
    WSAAsyncSelect(socket, qt_SocketWindow, sn_event ? qt_SocketMessage : 0, sn_event);
#else
    fd_set        rd,wt,ex;
    FD_ZERO(&rd);
    FD_ZERO(&wt);
    FD_ZERO(&ex);
    if (sn_read && sn_read->find(sockfd))
        FD_SET(sockfd, &rd);
    if (sn_write && sn_write->find(sockfd))
        FD_SET(sockfd, &wt);
    if (sn_except && sn_except->find(sockfd))
        FD_SET(sockfd, &ex);
    select(1, &rd, &wt, &ex, NULL);
#endif
#ifdef DEBUG_QEVENTLOOP

    qDebug( "QEventLoop::unregisterSocketNotifier( sn: 0x%08p, fd: %x ), mode: %d", sn, socket, type );
#endif
}

void QEventLoop::setSocketNotifierPending( QSocketNotifier *notifier )
{
#ifdef DEBUG_QEVENTLOOP
    qDebug( "QEventLoop::setSocketNotifierPending(fd: %x type: %d)",
            notifier->socket(), notifier->type() );
#endif

    int socket = notifier->socket();
    int type = notifier->type();
    if ( socket < 0 || type < 0 || type > 2 || notifier == 0 ) {
#if defined(QT_CHECK_RANGE)
        qWarning( "QSocketNotifier: Internal error" );
#endif

        return ;
    }

    QSNDict *sn_vec[3] = { &sn_read, &sn_write, &sn_except };
    QSNDict *dict = sn_vec[ type ];
 
    QSNDict::Iterator it = dict->find( socket );
    if( it == dict->end() )
        return;
    QSockNot *sn = it.data();
    if ( !sn )
        return;
    /* CE: Don't know if this is correct, but when we're already in this pending
           list, we shouldn't be added anymore...
           if we don't do so, we must change unregisterSocketNotifier() ! */
    if ( !d->sn_pending_list.contains( sn ) )
        d->sn_pending_list.append( sn );
#ifdef DEBUG_QEVENTLOOP
    else
        qDebug( "QEventLoop::setSocketNotifierPending(): sn already in list!" );

    qDebug( "QEventLoop::setSocketNotifierPending() added! count: %d", d->sn_pending_list.count() );
#endif
}

void QEventLoop::wakeUp()
{
    /* Idea from wxWidgets :) */
    QT_WA( PostThreadMessageW( qt_gui_thread_id, WM_NULL, 0, 0 );,
           PostThreadMessageA( qt_gui_thread_id, WM_NULL, 0, 0 ); )
}

/*
  Activates the timer events that have expired. Returns the number of timers
  (not 0-timer) that were activated.
*/

int QEventLoop::activateTimers()
{
#ifdef DEBUG_QEVENTLOOP
    qDebug( "TODO: qeventloop_win.cpp: activateTimers" );
#endif

    return 0;
}


int QEventLoop::activateSocketNotifiers()
{
    int n_act = 0;

    if ( d->sn_pending_list.isEmpty() )
        return 0;

    // activate entries
    QEvent event( QEvent::SockAct );
    QPtrListIterator<QSockNot> it( d->sn_pending_list );
    QSockNot *sn;
    while ( ( sn = it.current() ) ) {
#ifdef DEBUG_QEVENTLOOP
        qDebug( "QEventLoop::activateSocketNotifiers(fd: %x count: %d)",
                sn->fd, d->sn_pending_list.count() );
        qDebug( "QApplication::sendEvent( sn->obj: 0x%08p ) called!", sn->obj );
#endif

        ++it;
        d->sn_pending_list.removeRef( sn );
        QApplication::sendEvent( sn->obj, &event );
        n_act++;
    }
    return n_act;
}

/*! Returns the number of milliseconds that Qt needs to handle its timers or -1
    if there are no timers running.

    QEventLoop subclasses that do their own timer handling need to use this to
    make sure that Qt's timers continue to work.

    Note: This function is only useful on systems where select() is used to
    block the eventloop. On Windows, this function always returns -1. On
    MacOS X, this function always returns -1 when the GUI is enabled.
    On MacOS X, this function returns the documented value when the GUI is
    disabled.
*/
int QEventLoop::timeToWait() const
{
    return -1;
}


//Callback Function for Timers Events
void CALLBACK qt_TimerProc( HWND /*hwnd*/, UINT /*uMsg*/, UINT idEvent, DWORD /*dwTime*/ )
{
#ifdef DEBUG_QEVENTLOOP
    qDebug( "qeventloop_win.cpp: qt_TimerProc, %d", ( int ) idEvent );
#endif

    register TimerInfo *t;

    t = timerList->first();
    while ( t && t->id != idEvent )       // find timer info in list
        t = timerList->next();
    if ( t ) {
        QTimerEvent e( t->id );
        QApplication::sendEvent( t->obj, &e ); // send event
        QApplication::flush();
    }
#ifdef DEBUG_QEVENTLOOP
    qDebug( "qeventloop_win.cpp: qt_TimerProc t=%X", ( int ) t );
#endif
}

struct
{
    UINT msgID;
    const char* msgName;
}
msg[] = { { WM_APP, "WM_APP" },
          { WM_ACTIVATE, "WM_ACTIVATE" },
          { WM_ACTIVATEAPP, "WM_ACTIVATEAPP" },
          { WM_AFXFIRST, "WM_AFXFIRST" },
          { WM_AFXLAST, "WM_AFXLAST" },
          { WM_ASKCBFORMATNAME, "WM_ASKCBFORMATNAME" },
          { WM_CANCELJOURNAL, "WM_CANCELJOURNAL" },
          { WM_CANCELMODE, "WM_CANCELMODE" },
          { WM_CAPTURECHANGED, "WM_CAPTURECHANGED" },
          { WM_CHANGECBCHAIN, "WM_CHANGECBCHAIN" },
          { WM_CHAR, "WM_CHAR" },
          { WM_CHARTOITEM, "WM_CHARTOITEM" },
          { WM_CHILDACTIVATE, "WM_CHILDACTIVATE" },
          { WM_CLEAR, "WM_CLEAR" },
          { WM_CLOSE, "WM_CLOSE" },
          { WM_COMMAND, "WM_COMMAND" },
          { WM_COMMNOTIFY, "WM_COMMNOTIFY" },
          { WM_COMPACTING, "WM_COMPACTING" },
          { WM_COMPAREITEM, "WM_COMPAREITEM" },
          { WM_CONTEXTMENU, "WM_CONTEXTMENU" },
          { WM_COPY, "WM_COPY" },
          { WM_COPYDATA, "WM_COPYDATA" },
          { WM_CREATE, "WM_CREATE" },
          { WM_CTLCOLORBTN, "WM_CTLCOLORBTN" },
          { WM_CTLCOLORDLG, "WM_CTLCOLORDLG" },
          { WM_CTLCOLOREDIT, "WM_CTLCOLOREDIT" },
          { WM_CTLCOLORLISTBOX, "WM_CTLCOLORLISTBOX" },
          { WM_CTLCOLORMSGBOX, "WM_CTLCOLORMSGBOX" },
          { WM_CTLCOLORSCROLLBAR, "WM_CTLCOLORSCROLLBAR" },
          { WM_CTLCOLORSTATIC, "WM_CTLCOLORSTATIC" },
          { WM_CUT, "WM_CUT" },
          { WM_DEADCHAR, "WM_DEADCHAR" },
          { WM_DELETEITEM, "WM_DELETEITEM" },
          { WM_DESTROY, "WM_DESTROY" },
          { WM_DESTROYCLIPBOARD, "WM_DESTROYCLIPBOARD" },
          { WM_DEVICECHANGE, "WM_DEVICECHANGE" },
          { WM_DEVMODECHANGE, "WM_DEVMODECHANGE" },
          { WM_DISPLAYCHANGE, "WM_DISPLAYCHANGE" },
          { WM_DRAWCLIPBOARD, "WM_DRAWCLIPBOARD" },
          { WM_DRAWITEM, "WM_DRAWITEM" },
          { WM_DROPFILES, "WM_DROPFILES" },
          { WM_ENABLE, "WM_ENABLE" },
          { WM_ENDSESSION, "WM_ENDSESSION" },
          { WM_ENTERIDLE, "WM_ENTERIDLE" },
          { WM_ENTERMENULOOP, "WM_ENTERMENULOOP" },
          { WM_ENTERSIZEMOVE, "WM_ENTERSIZEMOVE" },
          { WM_ERASEBKGND, "WM_ERASEBKGND" },
          { WM_EXITMENULOOP, "WM_EXITMENULOOP" },
          { WM_EXITSIZEMOVE, "WM_EXITSIZEMOVE" },
          { WM_FONTCHANGE, "WM_FONTCHANGE" },
          { WM_GETDLGCODE, "WM_GETDLGCODE" },
          { WM_GETFONT, "WM_GETFONT" },
          { WM_GETHOTKEY, "WM_GETHOTKEY" },
          { WM_GETICON, "WM_GETICON" },
          { WM_GETMINMAXINFO, "WM_GETMINMAXINFO" },
          { WM_GETTEXT, "WM_GETTEXT" },
          { WM_GETTEXTLENGTH, "WM_GETTEXTLENGTH" },
          { WM_HANDHELDFIRST, "WM_HANDHELDFIRST" },
          { WM_HANDHELDLAST, "WM_HANDHELDLAST" },
          { WM_HELP, "WM_HELP" },
          { WM_HOTKEY, "WM_HOTKEY" },
          { WM_HSCROLL, "WM_HSCROLL" },
          { WM_HSCROLLCLIPBOARD, "WM_HSCROLLCLIPBOARD" },
          { WM_ICONERASEBKGND, "WM_ICONERASEBKGND" },
          { WM_INITDIALOG, "WM_INITDIALOG" },
          { WM_INITMENU, "WM_INITMENU" },
          { WM_INITMENUPOPUP, "WM_INITMENUPOPUP" },
          { WM_INPUTLANGCHANGE, "WM_INPUTLANGCHANGE" },
          { WM_INPUTLANGCHANGEREQUEST, "WM_INPUTLANGCHANGEREQUEST" },
          { WM_KEYDOWN, "WM_KEYDOWN" },
          { WM_KEYUP, "WM_KEYUP" },
          { WM_KILLFOCUS, "WM_KILLFOCUS" },
          { WM_MDIACTIVATE, "WM_MDIACTIVATE" },
          { WM_MDICASCADE, "WM_MDICASCADE" },
          { WM_MDICREATE, "WM_MDICREATE" },
          { WM_MDIDESTROY, "WM_MDIDESTROY" },
          { WM_MDIGETACTIVE, "WM_MDIGETACTIVE" },
          { WM_MDIICONARRANGE, "WM_MDIICONARRANGE" },
          { WM_MDIMAXIMIZE, "WM_MDIMAXIMIZE" },
          { WM_MDINEXT, "WM_MDINEXT" },
          { WM_MDIREFRESHMENU, "WM_MDIREFRESHMENU" },
          { WM_MDIRESTORE, "WM_MDIRESTORE" },
          { WM_MDISETMENU, "WM_MDISETMENU" },
          { WM_MDITILE, "WM_MDITILE" },
          { WM_MEASUREITEM, "WM_MEASUREITEM" },
          //{ WM_MENURBUTTONUP, "WM_MENURBUTTONUP" },
          { WM_MENUCHAR, "WM_MENUCHAR" },
          { WM_MENUSELECT, "WM_MENUSELECT" },
          { WM_NEXTMENU, "WM_NEXTMENU" },
          { WM_MOVE, "WM_MOVE" },
          { WM_MOVING, "WM_MOVING" },
          { WM_NCACTIVATE, "WM_NCACTIVATE" },
          { WM_NCCALCSIZE, "WM_NCCALCSIZE" },
          { WM_NCCREATE, "WM_NCCREATE" },
          { WM_NCDESTROY, "WM_NCDESTROY" },
          { WM_NCHITTEST, "WM_NCHITTEST" },
          { WM_NCLBUTTONDBLCLK, "WM_NCLBUTTONDBLCLK" },
          { WM_NCLBUTTONDOWN, "WM_NCLBUTTONDOWN" },
          { WM_NCLBUTTONUP, "WM_NCLBUTTONUP" },
          { WM_NCMBUTTONDBLCLK, "WM_NCMBUTTONDBLCLK" },
          { WM_NCMBUTTONDOWN, "WM_NCMBUTTONDOWN" },
          { WM_NCMBUTTONUP, "WM_NCMBUTTONUP" },
          { WM_NCMOUSEMOVE, "WM_NCMOUSEMOVE" },
          { WM_NCPAINT, "WM_NCPAINT" },
          { WM_NCRBUTTONDBLCLK, "WM_NCRBUTTONDBLCLK" },
          { WM_NCRBUTTONDOWN, "WM_NCRBUTTONDOWN" },
          { WM_NCRBUTTONUP, "WM_NCRBUTTONUP" },
          { WM_NEXTDLGCTL, "WM_NEXTDLGCTL" },
          { WM_NEXTMENU, "WM_NEXTMENU" },
          { WM_NOTIFY, "WM_NOTIFY" },
          { WM_NOTIFYFORMAT, "WM_NOTIFYFORMAT" },
          { WM_NULL, "WM_NULL" },
          { WM_PAINT, "WM_PAINT" },
          { WM_PAINTCLIPBOARD, "WM_PAINTCLIPBOARD" },
          { WM_PAINTICON, "WM_PAINTICON" },
          { WM_PALETTECHANGED, "WM_PALETTECHANGED" },
          { WM_PALETTEISCHANGING, "WM_PALETTEISCHANGING" },
          { WM_PARENTNOTIFY, "WM_PARENTNOTIFY" },
          { WM_PASTE, "WM_PASTE" },
          { WM_PENWINFIRST, "WM_PENWINFIRST" },
          { WM_PENWINLAST, "WM_PENWINLAST" },
          { WM_POWER, "WM_POWER" },
          { WM_POWERBROADCAST, "WM_POWERBROADCAST" },
          { WM_PRINT, "WM_PRINT" },
          { WM_PRINTCLIENT, "WM_PRINTCLIENT" },
          { WM_QUERYDRAGICON, "WM_QUERYDRAGICON" },
          { WM_QUERYENDSESSION, "WM_QUERYENDSESSION" },
          { WM_QUERYNEWPALETTE, "WM_QUERYNEWPALETTE" },
          { WM_QUERYOPEN, "WM_QUERYOPEN" },
          { WM_QUEUESYNC, "WM_QUEUESYNC" },
          { WM_QUIT, "WM_QUIT" },
          { WM_RENDERALLFORMATS, "WM_RENDERALLFORMATS" },
          { WM_RENDERFORMAT, "WM_RENDERFORMAT" },
          { WM_SETCURSOR, "WM_SETCURSOR" },
          { WM_SETFOCUS, "WM_SETFOCUS" },
          { WM_SETFONT, "WM_SETFONT" },
          { WM_SETHOTKEY, "WM_SETHOTKEY" },
          { WM_SETICON, "WM_SETICON" },
          { WM_SETREDRAW, "WM_SETREDRAW" },
          { WM_SETTEXT, "WM_SETTEXT" },
          { WM_SETTINGCHANGE, "WM_SETTINGCHANGE" },
          { WM_SHOWWINDOW, "WM_SHOWWINDOW" },
          { WM_SIZE, "WM_SIZE" },
          { WM_SIZECLIPBOARD, "WM_SIZECLIPBOARD" },
          { WM_SIZING, "WM_SIZING" },
          { WM_SPOOLERSTATUS, "WM_SPOOLERSTATUS" },
          { WM_STYLECHANGED, "WM_STYLECHANGED" },
          { WM_STYLECHANGING, "WM_STYLECHANGING" },
          { WM_SYSCHAR, "WM_SYSCHAR" },
          { WM_SYSCOLORCHANGE, "WM_SYSCOLORCHANGE" },
          { WM_SYSCOMMAND, "WM_SYSCOMMAND" },
          { WM_SYSDEADCHAR, "WM_SYSDEADCHAR" },
          { WM_SYSKEYDOWN, "WM_SYSKEYDOWN" },
          { WM_SYSKEYUP, "WM_SYSKEYUP" },
          { WM_TCARD, "WM_TCARD" },
          { WM_TIMECHANGE, "WM_TIMECHANGE" },
          { WM_TIMER, "WM_TIMER" },
          { WM_UNDO, "WM_UNDO" },
          { WM_USER, "WM_USER" },
          { WM_USERCHANGED, "WM_USERCHANGED" },
          { WM_VKEYTOITEM, "WM_VKEYTOITEM" },
          { WM_VSCROLL, "WM_VSCROLL" },
          { WM_VSCROLLCLIPBOARD, "WM_VSCROLLCLIPBOARD" },
          { WM_WINDOWPOSCHANGED, "WM_WINDOWPOSCHANGED" },
          { WM_WINDOWPOSCHANGING, "WM_WINDOWPOSCHANGING" },
          { WM_WININICHANGE, "WM_WININICHANGE" },
          { WM_KEYFIRST, "WM_KEYFIRST" },
          { WM_KEYLAST, "WM_KEYLAST" },
          { WM_SYNCPAINT, "WM_SYNCPAINT" },
          { WM_MOUSEACTIVATE, "WM_MOUSEACTIVATE" },
          { WM_MOUSEMOVE, "WM_MOUSEMOVE" },
          { WM_LBUTTONDOWN, "WM_LBUTTONDOWN" },
          { WM_LBUTTONUP, "WM_LBUTTONUP" },
          { WM_LBUTTONDBLCLK, "WM_LBUTTONDBLCLK" },
          { WM_RBUTTONDOWN, "WM_RBUTTONDOWN" },
          { WM_RBUTTONUP, "WM_RBUTTONUP" },
          { WM_RBUTTONDBLCLK, "WM_RBUTTONDBLCLK" },
          { WM_MBUTTONDOWN, "WM_MBUTTONDOWN" },
          { WM_MBUTTONUP, "WM_MBUTTONUP" },
          { WM_MBUTTONDBLCLK, "WM_MBUTTONDBLCLK" },
          { WM_MOUSEWHEEL, "WM_MOUSEWHEEL" },
          { WM_MOUSEFIRST, "WM_MOUSEFIRST" },
          { WM_MOUSELAST, "WM_MOUSELAST" },
          { WM_MOUSEHOVER, "WM_MOUSEHOVER" },
          { WM_MOUSELEAVE, "WM_MOUSELEAVE" }
        };

const char* getMsgName( UINT msgID )
{
    for ( int i = 0; msg[ i ].msgName != 0; i++ )
        if ( msg[ i ].msgID == msgID )
            return msg[ i ].msgName;
    return "";
}
