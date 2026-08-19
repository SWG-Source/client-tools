/****************************************************************************
** $Id: qeventloop_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QEventLoop class
**
** Created : 20030714
**
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2003 Ivan de Jesus Deras Tabora
** Copyright (C) 2004 Ralf Habacker
** Copyright (C) 2004 Christian Ehrlicher
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

#include "qt_windows.h"
#include <winsock.h>

#include "qeventloop_p.h" // includes qplatformdefs.h
#include "qeventloop.h"
#include "qapplication.h"

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#endif // QT_THREAD_SUPPORT*/

//#define DEBUG_QEVENTLOOP

static HWND qt_SocketWindow = NULL;
static UINT qt_SocketMessage = NULL;
typedef QMap<int, QSockNot *> QSNDict;
// socket notifiers
QSNDict sn_read;
QSNDict sn_write;
QSNDict sn_except;

extern bool qt_is_gui_used;
void qWinProcessConfigRequests();

static DWORD qt_gui_thread_id;

// from qeventloop_win_utils.cpp
FILETIME *qt_wait_timer();
void cleanupTimers();
static bool idleTimerWasActive;
static UINT_PTR idleTimerId;

bool winPeekMessage(MSG* msg, HWND hWnd, UINT wMsgFilterMin,
                     UINT wMsgFilterMax, UINT wRemoveMsg)
{
    QT_WA({ return PeekMessage(msg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg); } ,
          { return PeekMessageA(msg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg); });
}

bool winPostMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    QT_WA({ return PostMessage(hWnd, msg, wParam, lParam); } ,
          { return PostMessageA(hWnd, msg, wParam, lParam); });
}

bool winGetMessage(MSG* msg, HWND hWnd, UINT wMsgFilterMin,
                     UINT wMsgFilterMax)
{
    QT_WA({ return GetMessage(msg, hWnd, wMsgFilterMin, wMsgFilterMax); } ,
          { return GetMessageA(msg, hWnd, wMsgFilterMin, wMsgFilterMax); });
}

LRESULT CALLBACK qt_SocketWindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    if ( uMsg == WM_NCCREATE )
            return TRUE;
    if ( uMsg == qt_SocketMessage ) {

        // socket notifier message
        MSG msg;
        msg.hwnd = hWnd;
        msg.message = uMsg;
        msg.wParam = wParam;
        msg.lParam = lParam;
        
        if ( qApp && qApp->winEventFilter( &msg ) )
            return TRUE;

        int sockfd = wParam;
        int idx = -1;
#ifndef Q_OS_TEMP
        switch ( WSAGETSELECTEVENT( lParam ) ) {
        case FD_READ:
        case FD_ACCEPT:
        case FD_CLOSE:
            idx = 0;
            break;
        case FD_WRITE:
        case FD_CONNECT:
            idx = 1;
            break;
        case FD_OOB:
            idx = 2;
            break;
        }
#endif
        if ( idx >= 0 ) {
            QSNDict *sn_vec[3] = { &sn_read, &sn_write, &sn_except };
            QSNDict *dict = sn_vec[ idx ];
            QSNDict::iterator it;
            it = dict->find( wParam ) ;
            if ( it != dict->end() )
                QApplication::eventLoop() ->setSocketNotifierPending( it.data()->obj );
            return 0;
        }
    } else if ( uMsg == WM_TIMER ) {

        MSG msg;
        msg.hwnd = hWnd;
        msg.message = uMsg;
        msg.wParam = wParam;
        msg.lParam = lParam;

        if ( !qApp ) {
            KillTimer( hWnd, wParam );
            return 0;
        }

        if ( qApp && qApp->winEventFilter( &msg ) )
            return TRUE;
            
        // FIXME !!!
        
        return 0;
    }
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

static HWND qt_create_internal_window()
{
    extern HINSTANCE qWinAppInst();
    HINSTANCE hi = qWinAppInst();
    WNDCLASSA wc;
    wc.style = 0;
    wc.lpfnWndProc = qt_SocketWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hi;
    wc.hIcon = 0;
    wc.hCursor = 0;
    wc.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "QEventDispatcherWin32_Internal_Widget";
    RegisterClassA(&wc);

    HWND wnd = CreateWindowA(wc.lpszClassName,  // classname
                             wc.lpszClassName,  // window name
                             0,                 // style
                             0, 0, 0, 0,        // geometry
                             0,                 // parent
                             0,                 // menu handle
                             hi,                // application
                             0);                // windows creation data.

    qt_SocketMessage = RegisterWindowMessageA ( "qt_SocketMessage" );
    if (!wnd) {
        qWarning("Failed to create QEventDispatcherWin32 internal window: %d\n", (int)GetLastError());
    }
    return wnd;
}

void QEventLoop::init()
{
    qt_gui_thread_id = GetCurrentThreadId();

    qt_SocketWindow = qt_create_internal_window();

    sn_read.clear();
    sn_write.clear();
    sn_except.clear();

    if ( !qt_SocketWindow ) {
        qWarning( "QEventLoop:init failed - network support is not available" );
    }
}

void QEventLoop::cleanup()
{
    /* Clean up pending socket notifiers */
    // clean up any socketnotifiers
    while (!sn_read.isEmpty())
        unregisterSocketNotifier((*(sn_read.begin()))->obj);
    while (!sn_write.isEmpty())
        unregisterSocketNotifier((*(sn_write.begin()))->obj);
    while (!sn_except.isEmpty())
        unregisterSocketNotifier((*(sn_except.begin()))->obj);

    DestroyWindow( qt_SocketWindow );
}

void CALLBACK idleTimerProc( HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR idEvent, DWORD /*dwTime*/ )
{
#ifdef DEBUG_QEVENTLOOP
    if ( idleTimerId == idEvent ) {
        KillTimer( 0, idEvent );
    } else {
        qDebug( "idleTimerProc with unknown idEvent: %p", (void *)idEvent );
        KillTimer( 0, idEvent );
    }
#else
    KillTimer( 0, idEvent );
#endif
    idleTimerId = 0;
    idleTimerWasActive = TRUE;
    PostMessageA( 0, WM_NULL, 0, 0 );
}

bool qt_app_idle() {
    return  true;   
}

bool QEventLoop::processEvents( ProcessEventsFlags flags )
{
#ifdef DEBUG_QEVENTLOOP
    qDebug( "QEventLoop::processEvents (flags: %x)", flags );
#endif

    int nevents = 0;

#if defined(QT_THREAD_SUPPORT)

    QMutexLocker locker( QApplication::qt_mutex );
#endif

    // handle gui and posted events
    if ( qt_is_gui_used ) {
        qWinProcessConfigRequests();
        QApplication::sendPostedEvents();

        MSG msg;
        while ( winPeekMessage( &msg, 0, 0, 0, PM_REMOVE ) ) {
#ifdef DEBUG_QEVENTLOOP
            const char * getMsgName( UINT msgID );
            qDebug( "peekmessage (%s)", getMsgName( msg.message ) );
#endif

/*
            if (msg.message == WM_QUIT) {
                qApp->quit();
                return false;
            }*/
            if ( msg.message == WM_NULL )
                continue;
            if ( flags & ExcludeUserInput ) {
                /* Someone else some more user messages? */
                switch ( msg.message ) {
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_MOUSEMOVE:                 // == WM_MOUSEFIRST
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_LBUTTONDBLCLK:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_RBUTTONDBLCLK:
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                case WM_MBUTTONDBLCLK:
                case WM_MOUSEWHEEL:                // == WM_MOUSELAST
                    continue;
                }
            }
            nevents++;
#if defined(QT_THREAD_SUPPORT)

            locker.mutex() ->unlock();
#endif

            TranslateMessage( &msg );
            QT_WA( DispatchMessageW( &msg );,
                   DispatchMessageA( &msg ); )
#if defined(QT_THREAD_SUPPORT)
            locker.mutex()->lock();
#endif
        }
    }

    if ( d->shortcut ) {          // break immediately
        return FALSE;
    }

    QApplication::sendPostedEvents();

#if defined(QT_THREAD_SUPPORT)

    locker.mutex() ->unlock();
#endif

    if ( !( flags & ExcludeSocketNotifiers ) )
        nevents += activateSocketNotifiers();
#if defined(QT_THREAD_SUPPORT)
    locker.mutex()->lock();
#endif

    // don't block if exitLoop() or exit()/quit() has been called.
    bool canWait = d->exitloop || d->quitnow ? FALSE : ( flags & WaitForMore );

    
    if ( canWait ) {
        idleTimerId = 0;
        do {
            idleTimerWasActive = FALSE;
            if ( !qt_app_idle() && ( idleTimerId != 0 ) ) {
                idleTimerId = SetTimer( 0, 0, 100, idleTimerProc );
                qDebug( "QEventLoop::processEvents - SetTimer with id %p", (void *)idleTimerId );
            }
            WaitMessage();
        } while ( idleTimerWasActive );
    }

    // we are awake, broadcast it
    emit awake();
    emit qApp->guiThreadAwake();

    // give up current time slice to avoid 100 % cpu usage
    Sleep( 0 );

    // return true if we handled events, false otherwise
    return ( nevents > 0 );
}

bool QEventLoop::hasPendingEvents() const
{
    MSG msg;
    extern uint qGlobalPostedEventsCount();
    return qGlobalPostedEventsCount() || winPeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
}

#include "qeventloop_win_utils.cpp"
