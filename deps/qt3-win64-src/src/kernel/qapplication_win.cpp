/****************************************************************************
** $Id: qapplication_win.cpp 2053 2007-02-23 14:05:56Z chehrlic $
**
** Implementation of Windows startup routines and event handling
**
** Created : 20030119
**
** Copyright (C) 2003 Holger Schroeder
** Copyright (C) 2003,2004 Ralf Habacker
** Copyright (C) 2003,2004 Christopher January
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2003 Ivan de Jesus Deras Tabora
** Copyright (C) 2004 Andreas Hausladen
** Copyright (C) 2004 Peter Kuemmel
** Copyright (C) 2004 Dmitriy Kazimirow
** Copyright (C) 2004-2006 Christian Ehrlicher
** Copyright (C) 2005 Trolltech AS (parts of the source code are from qt4/gpl)
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

#include "qt_windows.h" // before all others!
#include "qplatformdefs.h"

#include <ole2.h>       // OleInitialize
#include <imm.h>        // for mingw
#include <locale.h>
#include <windowsx.h>

//#define DEBUG_QAPPLICATION
//#define QT_REAL_HARD_DEBUG
#include "qapplication.h"
#include "qapplication_p.h"
#include "qclipboard.h"
#include "qcursor.h"
#include "qguardedptr.h"
#include "qlibrary.h"
#include "qobjectlist.h"
#include "qpainter.h"
#include "qregexp.h"
#include "qsessionmanager.h"
#include "qsettings.h"
#include "qtextcodec.h"
#include "qwhatsthis.h"
#include "qwidget.h"
#include "qwidgetintdict.h"
#include "qwidgetlist.h"

#if defined(QT_THREAD_SUPPORT)
# include "qthread.h"
# include <private/qcriticalsection_p.h>
#endif

// msvc6 needs them
#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN 523
#endif
#ifndef WM_XBUTTONUP
#define WM_XBUTTONUP 524
#endif
#ifndef WM_XBUTTONDBLCLK
#define WM_XBUTTONDBLCLK 525
#endif

/* from qclipboard_win.cpp */
extern HWND hwndNextViewer;
extern bool clipboardHasChanged;

// defined in qpainter_win.cpp
void qt_erase_region( QWidget *w, QPixmap *pm, QPoint ofs, const QRegion &reg );     // for QETWidget::translatePaintEvent & translateEraseEvent
// defined in qeventloop_win.cpp
bool winPeekMessage(MSG* msg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
bool winPostMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// defined in qwidget_win.cpp
extern QCursor *qt_grab_cursor();
// forward declarations
static void set_winapp_name();
extern "C" LRESULT CALLBACK qt_window_procedure ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
static char appName[256];               // application name
static char appFileName[256];           // application file name
static HINSTANCE appInst = 0;           // qWinAppInst()
static HINSTANCE appPrevInst = 0;       // qWinAppPrevInst()
static int appCmdShow = 0;              // qWinAppCmdShow()

static HWND curWin = 0;                 // current window
static HDC appDisplay = 0;              // qt_display_dc()
#ifdef Q_OS_TEMP
static UINT appUniqueID = 0;            // application id
#endif

// Session management
static bool sm_blockUserInput = false;  // session management
static bool sm_smActive = true;
extern QSessionManager* qt_session_manager_self;
static bool sm_cancel = false;
// popup handling
static bool replayPopupMouseEvent = false; // replay handling when popups close
static QGuardedPtr<QWidget>* activeBeforePopup = 0; // focus handling with popups
static QWidget *popupButtonFocus = 0;
// ignore the next release event if return from a modal widget
bool qt_win_ignoreNextMouseReleaseEvent = false;

#if defined(QT_DEBUG)
static bool appNoGrab = false;          // mouse/keyboard grabbing
#endif 

// modal dialog handling
static bool app_do_modal = false;       // modal mode
extern QWidgetList *qt_modal_stack;     // stack of modal widgets
void qt_enter_modal( QWidget *widget );
void qt_leave_modal( QWidget *widget );
static bool qt_try_modal( QWidget *widget, MSG *msg, int& ret );

QWidget *qt_button_down = 0;  // widget got last button-down
extern QDesktopWidget *qt_desktopWidget;

static void unregWinClasses();

static HWND autoCaptureWnd = 0;
static void setAutoCapture(HWND);                // automatic capture
static void releaseAutoCapture();

static const char *appStyle = 0;    // application style
static const char *mwGeometry = 0;  // main widget geometry
static const char *mwTitle = 0;     // main widget title
static bool mwIconic = false;       // main widget iconified

typedef int (*QwinEventFilter) (MSG*);
QwinEventFilter qt_set_win_event_filter (QwinEventFilter filter);

static QwinEventFilter qt_win_event_filter = 0;
QwinEventFilter qt_set_win_event_filter (QwinEventFilter filter)
{
#ifdef DEBUG_QAPPLICATION
 qDebug("qapplication_win.cpp: qt_win_event_filter");
#endif
    QwinEventFilter old_filter = qt_win_event_filter;
    qt_win_event_filter = filter;
    return old_filter;
}

static bool qt_winEventFilter( MSG* msg )
{
    if ( qt_win_event_filter && qt_win_event_filter( msg )  )
        return TRUE;
    return qApp->winEventFilter( msg );
}

#ifdef DEBUG_QAPPLICATION
char* getMsgName( UINT msgID );
#endif

// mouse event handling
static UINT WM95_MOUSEWHEEL = 0;

#if(_WIN32_WINNT < 0x0400)
// This struct is defined in winuser.h if the _WIN32_WINNT >= 0x0400 -- in the
// other cases we have to define it on our own.
typedef struct tagTRACKMOUSEEVENT {
    DWORD cbSize;
    DWORD dwFlags;
    HWND  hwndTrack;
    DWORD dwHoverTime;
} TRACKMOUSEEVENT, *LPTRACKMOUSEEVENT;
#endif
#ifndef WM_MOUSELEAVE
#define WM_MOUSELEAVE                   0x02A3
#endif

extern bool qt_is_gui_used; // qapplication.cpp
extern void qt_dispatchEnterLeave( QWidget*, QWidget* ); // qapplication.cpp

// Keyboard handling
static bool bInTranslateMessage = false;
static bool bChar = true;
static unsigned short uAscii = 0;

inline QString getVersionString()
{
    return QString::number( ( QT_VERSION >> 16 ) & 0xff )
           + "." + QString::number( ( QT_VERSION >> 8 ) & 0xff );
}

class QETWidget : public QWidget  // event translator widget
{
public:
    void setWState( WFlags f ) { QWidget::setWState( f ); }
    void clearWState( WFlags f ) { QWidget::clearWState( f ); }
    void setWFlags( WFlags f ) { QWidget::setWFlags( f ); }
    void clearWFlags( WFlags f ) { QWidget::clearWFlags( f ); }
    void markFrameStrutDirty() { fstrut_dirty = 1; }

    bool translateMouseEvent( const MSG &msg );
    bool translateKeyEvent( MSG msg, bool grab );
    bool translateEraseEvent( const MSG &msg );
    bool translatePaintEvent( const MSG &msg );
    bool translateConfigEvent( const MSG &msg );
    bool translateCloseEvent( WPARAM wParam, LPARAM lParam );
    bool translateWheelEvent( const MSG &msg );

    bool invokeWinEvent( MSG* msg ) { return winEvent( msg ); }

    bool sendKeyEvent(MSG& msg, QEvent::Type type, int code, int ascii,
                      int state, bool grab, const QString& text,
                      bool autor=false);
    bool sendSpontaneousEvent( QObject *receiver, QEvent *event )
    {
        return QApplication::sendSpontaneousEvent( receiver, event );   
    }
    void showChildren( bool spontaneous ) { QWidget::showChildren( spontaneous ); }
    void hideChildren( bool spontaneous ) { QWidget::hideChildren( spontaneous ); }
    static bool inPopupMode(const QApplication *app) { return app->inPopupMode(); }
    void setActiveWindow( QWidget *w ) { qApp->setActiveWindow( w ); }
    void setNormalGeometry( const QRect &geo ) { topData()->normalGeometry = geo; }
    void setWindowStateET( unsigned int newState )
    {
        if( newState & WindowMinimized )
            setWState( WState_Minimized );
        else
            clearWState( WState_Minimized );

        if( newState & WindowMaximized )
            setWState( WState_Maximized );
        else
            clearWState( WState_Maximized );

        if( newState & WindowFullScreen )
            setWState( WState_FullScreen );
        else
            clearWState( WState_FullScreen );

        if( newState & WindowActive )
            setActiveWindow( this );
    }
};

static void qt_show_system_menu(QWidget* tlw)
{
    HMENU menu = GetSystemMenu(tlw->winId(), FALSE);
    if (!menu)
        return; // no menu for this window

#define enabled (MF_BYCOMMAND | MF_ENABLED)
#define disabled (MF_BYCOMMAND | MF_GRAYED)

#ifndef Q_OS_TEMP
//    EnableMenuItem(menu, SC_MINIMIZE, (tlw->windowFlags() & Qt::WindowMinimizeButtonHint)?enabled:disabled);
    EnableMenuItem(menu, SC_MINIMIZE, enabled);
    bool maximized = IsZoomed(tlw->winId());

//    EnableMenuItem(menu, SC_MAXIMIZE, ! (tlw->windowFlags() & Qt::WindowMaximizeButtonHint) || maximized?disabled:enabled);
    EnableMenuItem(menu, SC_MAXIMIZE, maximized?disabled:enabled);
    EnableMenuItem(menu, SC_RESTORE, maximized?enabled:disabled);

    EnableMenuItem(menu, SC_SIZE, maximized?disabled:enabled);
    EnableMenuItem(menu, SC_MOVE, maximized?disabled:enabled);
    EnableMenuItem(menu, SC_CLOSE, enabled);
#endif

#undef enabled
#undef disabled

    int ret = TrackPopupMenuEx(menu,
                                TPM_LEFTALIGN  | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
                                tlw->geometry().x(), tlw->geometry().y(),
                                tlw->winId(),
                                0);
    if (ret)
#ifdef Q_OS_TEMP
        DefWindowProc(tlw->winId(), WM_SYSCOMMAND, ret, 0);
#else
        qt_window_procedure(tlw->winId(), WM_SYSCOMMAND, ret, 0);
#endif
}

static const QColor getQColorFromSys( int syscolor )
{
    COLORREF c = GetSysColor( syscolor );
    return QColor( GetRValue( c ), GetGValue( c ), GetBValue( c ) );
}

extern QFont qt_LOGFONTtoQFont(LOGFONT& lf,bool scale);

static void qt_set_windows_resources()
{
    if ( !qApp )
        return ;

    //setup the global palette
    /* Available color roles:
        * QColorGroup::Background - general background color.
        * QColorGroup::Foreground - general foreground color.
        * QColorGroup::Base - used as background color for text entry widgets, for example;
                              usually white or another light color.
        * QColorGroup::Text - the foreground color used with Base. Usually this is the same
                              as the Foreground, in which case it must provide good contrast
                              with Background and Base.
        * QColorGroup::Button - general button background color in which buttons need a
                                background different from Background, as in the Macintosh style.
        * QColorGroup::ButtonText - a foreground color used with the Button color.
        * QColorGroup::Light - lighter than Button color.
        * QColorGroup::Midlight - between Button and Light.
        * QColorGroup::Dark - darker than Button.
        * QColorGroup::Mid - between Button and Dark.
        * QColorGroup::Shadow - a very dark color. By default, the shadow color is Qt::black.
        * QColorGroup::Highlight - a color to indicate a selected item or the current item.
                                   By default, the highlight color is Qt::darkBlue.
        * QColorGroup::HighlightedText - a text color that contrasts with Highlight. By default,
                                         the highlighted text color is Qt::white.
        * QColorGroup::BrightText - a text color that is very different from Foreground and
                                    contrasts well with e.g. Dark.
        * QColorGroup::Link - a text color used for unvisited hyperlinks. By default, the link
                              color is Qt::blue.
        * QColorGroup::LinkVisited - a text color used for already visited hyperlinks. By
                                     default, the linkvisited color is Qt::magenta.
    */

    QColor qc;
    QColor qc_gray = getQColorFromSys( COLOR_GRAYTEXT );
    QPalette pal = QApplication::palette();

    /* Background is everytime the same
       it is *not* COLOR_BACKGROUND */
    qc = getQColorFromSys( COLOR_BTNFACE );
    pal.setColor( QPalette::Active, QColorGroup::Background, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::Background, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::Background, qc );

    /* Foreground is the same as Windowtext when I see it at
       http://doc.trolltech.com/3.3/palette.png correct */
    qc = getQColorFromSys( COLOR_WINDOWTEXT );
    pal.setColor( QPalette::Active, QColorGroup::Foreground, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::Foreground, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::Foreground, qc_gray );

    /* don't know if this is correct ... */
    qc = getQColorFromSys( COLOR_WINDOW );
    pal.setColor( QPalette::Active, QColorGroup::Base, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::Base, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::Base, qc );

    /* Text ... is disabled text gray? have to look at qt321nc ! */
    qc = getQColorFromSys( COLOR_WINDOWTEXT );
    pal.setColor( QPalette::Active, QColorGroup::Text, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::Text, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::Text, qc_gray );

    /* but a Button is everytime the same */
    qc = getQColorFromSys( COLOR_BTNFACE );
    pal.setColor( QPalette::Active, QColorGroup::Button, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::Button, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::Button, qc );

    /* disabled ButtonText is gray */
    qc = getQColorFromSys( COLOR_BTNTEXT );
    pal.setColor( QPalette::Active, QColorGroup::ButtonText, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::ButtonText, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::ButtonText, qc_gray );

    /* Light... mhhh don't know - I took COLOR_BTNHIGHLIGHT */
    qc = getQColorFromSys( COLOR_BTNHIGHLIGHT );
    pal.setColor( QPalette::Active, QColorGroup::Light, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::Light, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::Light, qc );

    /* Midlight... what the hell is this? */
    /*  cg.background(): 212/208/200
        cg.midlight(): 233/229/220 -> approx background.light( 110 )
    */
    qc = getQColorFromSys( COLOR_BTNFACE ).light( 110 );
    pal.setColor( QPalette::Active, QColorGroup::Midlight, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::Midlight, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::Midlight, qc );

    /* Dark -> Shadow (COLOR_BTNSHADOW or COLOR_3DSHADOW ? )*/
    qc = getQColorFromSys( COLOR_BTNSHADOW );
    pal.setColor( QPalette::Active, QColorGroup::Dark, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::Dark, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::Dark, qc );

    /* Mid ... same procedure as midlight :) */
    /*  cg.background(): 212/208/200
        cg.mid(): 141/138/133 -> approx background.dark( 150 )
    */
    qc = getQColorFromSys( COLOR_BTNFACE ).dark( 150 );
    pal.setColor( QPalette::Active, QColorGroup::Mid, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::Mid, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::Mid, qc );

    /* Shadow */
    qc = getQColorFromSys( COLOR_3DDKSHADOW );
    pal.setColor( QPalette::Active, QColorGroup::Shadow, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::Shadow, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::Shadow, qc );

    /* Highlight */
    qc = getQColorFromSys( COLOR_HIGHLIGHT );
    pal.setColor( QPalette::Active, QColorGroup::Highlight, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::Highlight, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::Highlight, qc );

    /* HighlightedText */
    qc = getQColorFromSys( COLOR_HIGHLIGHTTEXT );
    pal.setColor( QPalette::Active, QColorGroup::HighlightedText, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::HighlightedText, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::HighlightedText, qc_gray );

    /* BrightText - really don't know - correct me! */
    qc = getQColorFromSys( COLOR_HIGHLIGHTTEXT );
    pal.setColor( QPalette::Active, QColorGroup::BrightText, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::BrightText, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::BrightText, qc );

    /* Link - there is no GetSysColor const for this :(
       But I've found this:
       http://msdn.microsoft.com/workshop/browser/overview/hypertextlinks.asp
       for now I use blue like explained in the docs
    */
    qc = Qt::blue;
    pal.setColor( QPalette::Active, QColorGroup::Link, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::Link, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::Link, qc );

    /* LinkVisited: see QColorGroup::Link
       purple -> magenta*/
    qc = Qt::magenta;
    pal.setColor( QPalette::Active, QColorGroup::LinkVisited, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::LinkVisited, qc );
    pal.setColor( QPalette::Disabled, QColorGroup::LinkVisited, qc );

    if ( !( pal == QApplication::palette() ) )
        QApplication::setPalette( pal );

    // now setup the palette for special widgets
    // only set the values which are different from standard values...

    //Popup Menu
    pal = QApplication::palette();
    qc = getQColorFromSys( COLOR_MENU );
    pal.setBrush( QPalette::Active, QColorGroup::Background, qc );
    pal.setBrush( QPalette::Inactive, QColorGroup::Background, qc );
    pal.setBrush( QPalette::Disabled, QColorGroup::Background, qc );

    pal.setBrush( QPalette::Active, QColorGroup::Button, qc );
    pal.setBrush( QPalette::Inactive, QColorGroup::Button, qc );
    pal.setBrush( QPalette::Disabled, QColorGroup::Button, qc );

    qc = getQColorFromSys( COLOR_MENUTEXT );
    pal.setColor( QPalette::Active, QColorGroup::ButtonText, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::ButtonText, qc );

    if ( pal != QApplication::palette() )
        QApplication::setPalette( pal, TRUE, "QPopupMenu" );

    //Button
    pal = QApplication::palette();
    qc = getQColorFromSys( COLOR_BTNFACE );
    pal.setBrush( QPalette::Active, QColorGroup::Background, qc );
    pal.setBrush( QPalette::Inactive, QColorGroup::Background, qc );
    pal.setBrush( QPalette::Disabled, QColorGroup::Background, qc );

    pal.setBrush( QPalette::Active, QColorGroup::Foreground, qc );
    pal.setBrush( QPalette::Inactive, QColorGroup::Foreground, qc );
    pal.setBrush( QPalette::Disabled, QColorGroup::Foreground, qc );

    qc = getQColorFromSys( COLOR_BTNTEXT );
    pal.setColor( QPalette::Active, QColorGroup::HighlightedText, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::HighlightedText, qc );

    pal.setColor( QPalette::Active, QColorGroup::Text, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::Text, qc );

    if ( pal != QApplication::palette() )
        QApplication::setPalette( pal, TRUE, "QPushButton" );

    //Tool Tip
    pal = QApplication::palette();
    qc = getQColorFromSys( COLOR_INFOBK );
    pal.setColor( QPalette::Active, QColorGroup::Background, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::Background, qc );

    qc = getQColorFromSys ( COLOR_INFOTEXT );
    pal.setColor( QPalette::Active, QColorGroup::Text, qc );
    pal.setColor( QPalette::Active, QColorGroup::HighlightedText, qc );
    pal.setColor( QPalette::Active, QColorGroup::Foreground, qc );

    pal.setColor( QPalette::Inactive, QColorGroup::Text, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::HighlightedText, qc );
    pal.setColor( QPalette::Inactive, QColorGroup::Foreground, qc );

    if ( pal != QApplication::palette() )
        QApplication::setPalette( pal, TRUE, "QTipLabel" );

#ifndef Q_OS_TEMP
    QFont menuFont;
    QFont messageFont;
    QFont statusFont;
    QFont titleFont;
    QFont smallTitleFont;

    QT_WA({
        NONCLIENTMETRICS ncm;
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        menuFont = qt_LOGFONTtoQFont(ncm.lfMenuFont,true);
        messageFont = qt_LOGFONTtoQFont(ncm.lfMessageFont,true);
        statusFont = qt_LOGFONTtoQFont(ncm.lfStatusFont,true);
        titleFont = qt_LOGFONTtoQFont(ncm.lfCaptionFont,true);
        smallTitleFont = qt_LOGFONTtoQFont(ncm.lfSmCaptionFont,true);
    } , {
        // A version
        NONCLIENTMETRICSA ncm;
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        menuFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfMenuFont,true);
        messageFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfMessageFont,true);
        statusFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfStatusFont,true);
        titleFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfCaptionFont,true);
        smallTitleFont = qt_LOGFONTtoQFont((LOGFONT&)ncm.lfSmCaptionFont,true);
    });

    QApplication::setFont(menuFont, true, "QMenuBar");          // ok
    QApplication::setFont(menuFont, true, "QMenuData");         // ok (win2k only?)
    QApplication::setFont(menuFont, true, "QPopupMenu");        // ok (winxp)
    QApplication::setFont(messageFont, true, "QMessageBox");    // ok
    QApplication::setFont(statusFont, true, "QTipLabel");       // ok
    QApplication::setFont(statusFont, true, "QStatusBar");      // ??
    QApplication::setFont(titleFont, true, "QTitleBar");        // ok
    QApplication::setFont(smallTitleFont, true, "QDockWindowTitleBar"); // I think ok
#else
    LOGFONT lf;
    HGDIOBJ stockFont = GetStockObject(SYSTEM_FONT);
    GetObject(stockFont, sizeof(lf), &lf);
    QApplication::setFont(qt_LOGFONTtoQFont(lf, true));
#endif// Q_OS_TEMP
}

// need to get default font?
extern bool qt_app_has_font;

static void qt_init_gui()
{
    // Get the application name/instance if qWinMain() was not invoked
#ifndef Q_OS_TEMP
    // No message boxes but important ones
    SetErrorMode(SetErrorMode(0) | SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
#endif

    if (appInst == 0) {
        QT_WA({
            appInst = GetModuleHandleW(0);
        }, {
            appInst = GetModuleHandleA(0);
        });
    }

#ifndef Q_OS_TEMP
    // Initialize OLE/COM
    //         S_OK means success and S_FALSE means that it has already
    //         been initialized
    HRESULT r;
    r = OleInitialize(0);
    if (r != S_OK && r != S_FALSE) {
        qWarning("Qt: Could not initialize OLE (error %x)", (unsigned int)r);
    }
#endif

    // Misc. initialization
#if defined(QT_DEBUG)
    GdiSetBatchLimit(1);
#endif

    QColor::initialize();
    QFont::initialize();
    QCursor::initialize();
    QPainter::initialize();
    QWindowsMime::initialize();

    qApp->setName( qAppName() );

    // default font
    if (!qt_app_has_font) {
        HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        QFont f("MS Sans Serif", 8);
        QT_WA({
            LOGFONT lf;
            if (GetObject(hfont, sizeof(lf), &lf))
                f = qt_LOGFONTtoQFont((LOGFONT&)lf,true);
        } , {
            LOGFONTA lf;
            if (GetObjectA(hfont, sizeof(lf), &lf))
                f = qt_LOGFONTtoQFont((LOGFONT&)lf,true);
        });
        QApplication::setFont(f);
    }
    qt_set_windows_resources();
    QT_WA({
        WM95_MOUSEWHEEL = RegisterWindowMessage(L"MSWHEEL_ROLLMSG");
    } , {
        WM95_MOUSEWHEEL = RegisterWindowMessageA("MSWHEEL_ROLLMSG");
    });
}

void qt_init( int *argcptr, char **argv, QApplication::Type )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "qapplication_win.cpp: qt_init" );
#endif

    int argc = *argcptr;

    set_winapp_name();
     // Get command line params
    int j = 0;
    for ( int i = j; i < argc; i++ ) {
        if ( argv[ i ] && *argv[ i ] != '-' ) {
            argv[ j++ ] = argv[ i ];
            continue;
        }
        QString arg = QString( argv[ i ] ).lower();
        if ( arg == "-name" ) {
            if ( ++i < argc )
                strncpy( appName, argv[ i ], sizeof( appName )-1 );
                appName[ sizeof( appName )-1 ] = '\0';
        } else if ( arg == "-style" ) {
            if ( ++i < argc )
                appStyle = argv[ i ];
        } else if ( arg == "-title" ) {
            if ( ++i < argc )
                mwTitle = argv[ i ];
        } else if ( arg == "-geometry" ) {
            if ( ++i < argc )
                mwGeometry = argv[ i ];
        } else if ( arg == "-iconic" ) {
            mwIconic = !mwIconic;
        }
#if defined(DEBUG)
        else if ( arg == "-nograb" )
            appNoGrab = !appNoGrab;
#endif

        else
            argv[ j++ ] = argv[ i ];
    }

    *argcptr = j;

    if ( qt_is_gui_used )
        qt_init_gui();

    setlocale( LC_ALL, "" );  // use correct char set mapping

    //DKZM:setup codecForCStrings() - it has Latin1 by default
    //setlocale is not enough for it
    //checked under Visual C++ .NET 2003
    QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

    setlocale( LC_NUMERIC, "C" ); // make sprintf()/scanf() work

    /* we can't call something like qt_init_dir() in qdir_win.cpp since link_includes and
       configure.exe will fail linking because of missing qAddPostRoutine */
#if !defined(Q_CYGWIN_WIN)
    extern void qt_dir_cleanup();
    qAddPostRoutine( qt_dir_cleanup );
#endif
    //
    // application style settings
    //
    extern bool qt_explicit_app_style; // defined in qapplication.cpp

    // set style from command line
    if ( appStyle ) {
        QApplication::setStyle( appStyle );
        // took the style from the user settings, so mark the explicit flag FALSE
        qt_explicit_app_style = FALSE;
    }
    // set style from default settings
    else {
        QSettings settings;
        settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
        const QString key = "/" + getVersionString() + "/";

        QString stylename = settings.readEntry( key + "style" );

        if ( !stylename.isEmpty() || appStyle /*&& !qt_explicit_app_style*/ ) {
            QApplication::setStyle( stylename );
            // took the style from the user settings, so mark the explicit flag FALSE
            qt_explicit_app_style = FALSE;
        }
    }
}



/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "qt_cleanup()" );
#endif
    unregWinClasses();
    /* Clean Up DC if used */
    if ( appDisplay )
        ReleaseDC( NULL, appDisplay );

    extern void cleanupTimers();
    cleanupTimers();

    if ( activeBeforePopup ) {
        delete activeBeforePopup;
        activeBeforePopup = 0;
    }
#ifndef Q_OS_TEMP
  // Deinitialize OLE/COM
    OleUninitialize();
#endif
}


/*****************************************************************************
  Safe configuration (move,resize,setGeometry) mechanism to avoid
  recursion when processing messages.
 *****************************************************************************/

struct QWinConfigRequest {
    WId         id;             // widget to be configured
    int         req;            // 0=move, 1=resize, 2=setGeo
    int         x, y, w, h;     // request parameters
};

QList<QWinConfigRequest> *configRequests = 0;

void qWinRequestConfig(WId id, int req, int x, int y, int w, int h)
{
    if ( !configRequests )  // create queue
        configRequests = new QList<QWinConfigRequest>;
    QWinConfigRequest *r = new QWinConfigRequest;
    r->id = id;                     // create new request
    r->req = req;
    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
    configRequests->append( r );    // store request in queue
}

void qWinProcessConfigRequests()    // perform requests in queue
{
    if ( !configRequests )
        return;
    QWinConfigRequest *r;
    for ( ;; ) {
        if ( configRequests->isEmpty() )
            break;
        r = configRequests->last();
        configRequests->removeLast();
        QWidget *w = QWidget::find( r->id );
        QRect rect( r->x, r->y, r->w, r->h );
        int req = r->req;
        delete r;

        if ( w ) {              // widget exists
            if ( w->testWState( Qt::WState_ConfigPending ) )
                return;             // biting our tail
            if ( req == 0 )
                w->move( rect.topLeft() );
            else if ( req == 1 )
                w->resize( rect.size() );
            else
                w->setGeometry( rect );
        }
    }
    delete configRequests;
    configRequests = 0;
}

/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

int qWinAppCmdShow()
{
    return appCmdShow;
}

HDC qt_display_dc()
{
    if ( !appDisplay )
        appDisplay = GetDC( 0 );
    return appDisplay;
}

bool qt_nograb()               // application no-grab option
{
#if defined(DEBUG)
    return appNoGrab;
#else
    return false;
#endif
}

typedef QMap<QString, int> WinClassNameMap;
static WinClassNameMap winclassNames;

const QString qt_reg_winclass( Qt::WFlags flags )         // register window class
{
    int type = flags & Qt::WType_Mask;

    uint style;
    bool icon;
    QString cname;
    if ( type == Qt::WType_Popup ) {
        cname = "QPopup";
        style = CS_DBLCLKS;
#ifndef Q_OS_TEMP
        style |= CS_SAVEBITS;
#endif

        if ( ( qt_winver >= Qt::WV_XP && qt_winver < Qt::WV_NT_based ) )
            style |= 0x00020000;                // CS_DROPSHADOW
        icon = false;
    } else {
        cname = "QWidget";
        style = CS_DBLCLKS;
#ifndef Q_OS_TEMP
        style |= CS_SAVEBITS;
#endif
        icon = true;
    }

#ifdef Q_OS_TEMP
    // We need to register the classes with the
    // unique ID on WinCE to make sure we can
    // move the windows to the front when starting
    // a second instance.
    cname = QString::number( appUniqueID );
#endif

    // since multiple Qt versions can be used in one process
    // each one has to have window class names with a unique name
    // The first instance gets the unmodified name; if the class
    // has already been registered by another instance of Qt then
    // add an instance-specific ID, the address of the window proc.
    static int classExists = -1;

    if ( classExists == -1 ) {
        QT_WA( {
                   WNDCLASS wcinfo;
                   classExists = GetClassInfo( ( HINSTANCE ) qWinAppInst(), ( TCHAR* ) cname.ucs2(), &wcinfo );
                   classExists = classExists && wcinfo.lpfnWndProc != qt_window_procedure;
               }, {
                   WNDCLASSA wcinfo;
                   classExists = GetClassInfoA( ( HINSTANCE ) qWinAppInst(), cname.latin1(), &wcinfo );
                   classExists = classExists && wcinfo.lpfnWndProc != qt_window_procedure;
               } );
    }

    if ( classExists )
        cname += QString::number( ( uint ) qt_window_procedure );

    if ( winclassNames.contains( cname ) )         // already registered in our list
        return cname;

    ATOM atom;
#ifndef Q_OS_TEMP

    QT_WA( {
               WNDCLASS wc;
               wc.style = style;
               wc.lpfnWndProc = ( WNDPROC ) qt_window_procedure;
               wc.cbClsExtra = 0;
               wc.cbWndExtra = 0;
               wc.hInstance = ( HINSTANCE ) qWinAppInst();
               if ( icon ) {
                    wc.hIcon = LoadIcon( appInst, L"IDI_ICON1" );
                    if ( !wc.hIcon )
                        wc.hIcon = LoadIcon( 0, IDI_APPLICATION );
               } else {
                   wc.hIcon = 0;
               }
               wc.hCursor = 0;
               wc.hbrBackground = 0;
               wc.lpszMenuName = 0;
               wc.lpszClassName = ( TCHAR* ) cname.ucs2();
               atom = RegisterClass( &wc );
           } , {
               WNDCLASSA wc;
               wc.style = style;
               wc.lpfnWndProc = ( WNDPROC ) qt_window_procedure;
               wc.cbClsExtra = 0;
               wc.cbWndExtra = 0;
               wc.hInstance = ( HINSTANCE ) qWinAppInst();
               if ( icon ) {
                    wc.hIcon = LoadIconA( appInst, ( char* ) "IDI_ICON1" );
                    if ( !wc.hIcon )
                        wc.hIcon = LoadIconA( 0, ( char* ) IDI_APPLICATION );
               } else {
                   wc.hIcon = 0;
               }
               wc.hCursor = 0;
               wc.hbrBackground = 0;
               wc.lpszMenuName = 0;
               QCString tempArray = cname.latin1();
               wc.lpszClassName = tempArray.data();
               atom = RegisterClassA( &wc );
           } );
#else

    WNDCLASS wc;
    wc.style = style;
    wc.lpfnWndProc = ( WNDPROC ) qt_window_procedure;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = ( HINSTANCE ) qWinAppInst();
    if ( icon ) {
        wc.hIcon = LoadIcon( appInst, L"IDI_ICON1" );
        //            if (!wc.hIcon)
        //                wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    } else {
        wc.hIcon = 0;
    }
    wc.hCursor = 0;
    wc.hbrBackground = 0;
    wc.lpszMenuName = 0;
    wc.lpszClassName = ( TCHAR* ) cname.ucs2();
    atom = RegisterClass( &wc );
#endif

#ifndef QT_NO_DEBUG

    if ( !atom )
        qWarning( "QApplication::regClass: Registering window class failed." );
#endif

    winclassNames.insert( cname, 1 );
    return cname;
}

static void unregWinClasses()
{
    QMap<QString, int>::ConstIterator it = winclassNames.constBegin();
    while ( it != winclassNames.constEnd() ) {
        QT_WA( {
                   UnregisterClass( ( TCHAR* ) it.key().ucs2(), ( HINSTANCE ) qWinAppInst() );
               } , {
                   UnregisterClassA( it.key().latin1(), ( HINSTANCE ) qWinAppInst() );
               } );
        ++it;
    }
    winclassNames.clear();
}

/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

void QApplication::setMainWidget( QWidget *mainWidget )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "QApplication::setMainWidget( QWidget: 0x%08p )", mainWidget );
#endif

    main_widget = mainWidget;
    if ( main_widget ) {   // give WM command line

        if ( mwTitle )
            main_widget->setCaption( mwTitle );

        if ( mwGeometry ) {   // parse geometry
            int x, y;
            int w, h;
            QRegExp re( "[=]*(?:([0-9]+)[xX]([0-9]+)){0,1}[ ]*(?:([+-][0-9]*)([+-][0-9]*)){0,1}" );
            re.search( mwGeometry );
            w = re.cap( 1 ).toInt();
            h = re.cap( 2 ).toInt();
            x = re.cap( 3 ).toInt();
            y = re.cap( 4 ).toInt();
            QSize minSize = main_widget->minimumSize();
            QSize maxSize = main_widget->maximumSize();
            w = QMIN( w, maxSize.width() );
            h = QMIN( h, maxSize.height() );
            w = QMAX( w, minSize.width() );
            h = QMAX( h, minSize.height() );
            main_widget->setGeometry( x, y, w, h );
        }
        if ( mwIconic ) {
            main_widget->showMinimized();
        }
    }
}
#ifndef QT_NO_CURSOR

/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

typedef QList<QCursor> QCursorList;
static QCursorList *cursorStack = 0;

void QApplication::setOverrideCursor( const QCursor &cursor, bool replace )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "QApplication::setOverrideCursor( QCursor 0x%08p, replace: %d )", cursor.handle(), replace );
#endif

    if ( !cursorStack ) {
        cursorStack = new QCursorList;
        CHECK_PTR( cursorStack );
        cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor( cursor );
    CHECK_PTR( app_cursor );
    if ( replace )
        cursorStack->removeLast();
    cursorStack->append( app_cursor );
}

void QApplication::restoreOverrideCursor()
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "qapplication_win.cpp: restoreOverrideCursor" );
#endif

    if ( !cursorStack )               // no cursor stack
        return ;
    cursorStack->removeLast();
    app_cursor = cursorStack->last();
    if ( !app_cursor ) {
        delete cursorStack;
        cursorStack = 0;
    }
}

#endif

/*!
  \fn bool QApplication::hasGlobalMouseTracking()
 
  Returns TRUE if global mouse tracking is enabled, otherwise false.
 
  \sa setGlobalMouseTracking()
*/

/*!
  Enables global mouse tracking if \a enable is TRUE or disables it
  if \a enable is false.
 
  Enabling global mouse tracking makes it possible for widget event
  filters or application event filters to get all mouse move events, even
  when no button is depressed.  This is useful for special GUI elements,
  e.g. tool tips.
 
  Global mouse tracking does not affect widgets and their
  mouseMoveEvent().  For a widget to get mouse move events when no button
  is depressed, it must do QWidget::setMouseTracking(TRUE).
 
  This function uses an internal counter.  Each
  setGlobalMouseTracking(TRUE) must have a corresponding
  setGlobalMouseTracking(false):
  \code
    // at this point global mouse tracking is off
    QApplication::setGlobalMouseTracking( TRUE );
    QApplication::setGlobalMouseTracking( TRUE );
    QApplication::setGlobalMouseTracking( false );
    // at this point it's still on
    QApplication::setGlobalMouseTracking( false );
    // but now it's off
  \endcode
 
  \sa hasGlobalMouseTracking(), QWidget::hasMouseTracking()
*/

void QApplication::setGlobalMouseTracking( bool enable )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "qapplication_win.cpp: setGlobalMouseTracking" );
#endif

    bool tellAllWidgets;
    if ( enable ) {
        tellAllWidgets = ( ++app_tracking == 1 );
    } else {
        tellAllWidgets = ( --app_tracking == 0 );
    }
    if ( tellAllWidgets ) {
        QWidgetIntDictIt it( *( ( QWidgetIntDict* ) QWidget::mapper ) );
        register QWidget *w;
        while ( ( w = it.current() ) ) {
            if ( app_tracking > 0 ) {  // switch on
                if ( !w->testWState( WState_MouseTracking ) ) {
                    w->setMouseTracking( TRUE );
                    w->clearWState( WState_MouseTracking );
                }
            } else {    // switch off
                if ( !w->testWState( WState_MouseTracking ) ) {
                    w->setWState( WState_MouseTracking );
                    w->setMouseTracking( false );
                }
            }
            ++it;
        }
    }
}


/*****************************************************************************
  Routines to find a Qt widget from a screen position
 *****************************************************************************/
static QWidget *findChildWidget( const QWidget *p, const QPoint &pos )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "findChildWidget( p: 0x%08p, pos: %d/%d", p, pos.x(), pos.y() );
#endif

    if ( p->children() ) {
        QWidget * w;
        QObjectListIt it( *p->children() );
        it.toLast();
        while ( it.current() ) {
            if ( it.current() ->isWidgetType() ) {
                w = ( QWidget* ) it.current();
                if ( w->isVisible() && w->geometry().contains( pos ) ) {
                    QWidget * c = findChildWidget( w, w->mapFromParent( pos ) );
                    return c ? c : w;
                }
            }
            --it;
        }
    }
    return 0;
}

/*!
  Returns a pointer to the widget at global screen position \a (x,y), or a
  null pointer if there is no Qt widget there.
 
  If \a child is false and there is a child widget at position \a
  (x,y), the top-level widget containing it is returned. If \a child
  is TRUE the child widget at position \a (x,y) is returned.
 
  This function is normally rather slow.
 
  \sa QCursor::pos(), QWidget::grabMouse(), QWidget::grabKeyboard()
*/

QWidget *QApplication::widgetAt( int x, int y, bool bChild )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "QApplication::widgetAt( x: %d, y: %d bChild: %d )", x, y, bChild );
#endif
    POINT p;
    p.x = x;
    p.y = y;

    //  Window target
    HWND target = WindowFromPoint( p );

    if ( !target )
        return 0;

    QWidget *w;
    // go up until we've found a 'valid' widget
    while ( ( w = QWidget::find( ( WId ) target ) ) == NULL ) {
        if ( !bChild )
            return NULL;
        target = GetParent( target );
        if ( !target ) {
            // only with this hack I can get the slider-widgets in examples...
            if ( qApp->mainWidget() ) {
                target = ChildWindowFromPoint( qApp->mainWidget()->winId(), p );
                w = QWidget::find( ( WId ) target );
            }
            return w;
        }
    }
    return w; 
}

/*!
  \overload QWidget *QApplication::widgetAt( const QPoint &pos, bool child )
*/

/*!
    Flushes the window system specific event queues.
 
    If you are doing graphical changes inside a loop that does not
    return to the event loop on asynchronous window systems like X11
    or double buffered window systems like MacOS X, and you want to
    visualize these changes immediately (e.g. Splash Screens), call
    this function.
 
    \sa flushX() sendPostedEvents() QPainter::flush()
*/

void QApplication::flush()
{
    GdiFlush();
}
/*!
  Sounds the bell, using the default volume and sound.
*/

void QApplication::beep()
{
    MessageBeep( MB_OK );
}

bool QApplication::winEventFilter( MSG* )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "QApplication::winEventFilter( MSG* )" );
#endif

    return false;
}

/*****************************************************************************
  Modal widgets; Since Xlib has little support for this we roll our own
  modal widget mechanism.
  A modal widget without a parent becomes application-modal.
  A modal widget with a parent becomes modal to its parent and grandparents..
 
  qt_enter_modal()
 Enters modal state
 Arguments:
     QWidget *widget A modal widget
 
  qt_leave_modal()
 Leaves modal state for a widget
 Arguments:
     QWidget *widget A modal widget
 *****************************************************************************/
bool qt_modal_state()
{
    return app_do_modal;
}

void qt_enter_modal( QWidget *widget )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "qt_enter_modal( 0x%08p ), winid: %d", widget, widget ? widget->winId() : 0 );
#endif

    if ( !qt_modal_stack ) {                        // create modal stack
        qt_modal_stack = new QWidgetList;
    }

    releaseAutoCapture();
    qt_dispatchEnterLeave( 0, QWidget::find( ( WId ) curWin ) );
    qt_modal_stack->insert( 0, widget );
    app_do_modal = true;
    curWin = 0;
    qt_button_down = 0;
    qt_win_ignoreNextMouseReleaseEvent = false;

    if ( widget->parentWidget() ) {
        QEvent e( QEvent::WindowBlocked );
        QApplication::sendEvent( widget->parentWidget(), &e );
    }
}

void qt_leave_modal( QWidget *widget )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "qt_leave_modal( 0x%08p ), winid: %d", widget, widget ? widget->winId() : 0 );
#endif

    if ( qt_modal_stack && qt_modal_stack->removeRef( widget ) ) {
        if ( qt_modal_stack->isEmpty() ) {
            delete qt_modal_stack;
            qt_modal_stack = 0;
            QPoint p( QCursor::pos() );
            app_do_modal = false; // necessary, we may get recursively into qt_try_modal below
            QWidget* w = QApplication::widgetAt( p.x(), p.y() );
            qt_dispatchEnterLeave( w, QWidget::find( curWin ) ); // send synthetic enter event
            curWin = w ? w->winId() : 0;
        }
        qt_win_ignoreNextMouseReleaseEvent = true;
    }
    app_do_modal = qt_modal_stack != 0;

    if ( widget->parentWidget() ) {
        QEvent e( QEvent::WindowUnblocked );
        QApplication::sendEvent( widget->parentWidget(), &e );
    }
}

static bool qt_blocked_modal( QWidget *widget )
{
    if ( !app_do_modal )
        return false;
    if ( qApp->activePopupWidget() )
        return false;
    if ( ( widget->testWFlags( Qt::WStyle_Tool ) ) )    // allow tool windows
        return false;

    QWidget *modal = 0, *top = qt_modal_stack->first();

    widget = widget->topLevelWidget();
    if ( widget->testWFlags( Qt::WShowModal ) )         // widget is modal
        modal = widget;
    if ( !top || modal == top )                         // don't block event
        return false;
    return true;
}


static bool qt_try_modal( QWidget *widget, MSG *msg, int& ret )
{
    QWidget * top = 0;

    if ( qt_tryModalHelper( widget, &top ) )
        return TRUE;

    int type = msg->message;

    bool block_event = false;
#ifndef Q_OS_TEMP

    if ( type == WM_NCHITTEST ) {
        block_event = true;
    } else
#endif
        if ( ( type >= WM_MOUSEFIRST && type <= WM_MOUSELAST ) ||
                type == WM_MOUSEWHEEL || type == ( int ) WM95_MOUSEWHEEL ||
                type == WM_MOUSELEAVE ||
                ( type >= WM_KEYFIRST && type <= WM_KEYLAST )
#ifndef Q_OS_TEMP
                || type == WM_NCMOUSEMOVE
#endif
           ) {
            if ( type == WM_MOUSEMOVE
#ifndef Q_OS_TEMP
                    || type == WM_NCMOUSEMOVE
#endif
               ) {
                QCursor * c = qt_grab_cursor();
                if ( !c )
                    c = QApplication::overrideCursor();
                if ( c )                                 // application cursor defined
                    SetCursor( c->handle() );
                else
                    SetCursor( QCursor( Qt::ArrowCursor ).handle() );
            }
            block_event = true;
        } else if ( type == WM_CLOSE ) {
            block_event = true;
        }
#ifndef Q_OS_TEMP
        else if ( type == WM_MOUSEACTIVATE || type == WM_NCLBUTTONDOWN ) {
            if ( !top->isActiveWindow() ) {
                top->setActiveWindow();
            } else {
                QApplication::beep();
            }
            block_event = true;
            ret = MA_NOACTIVATEANDEAT;
        } else if ( type == WM_SYSCOMMAND ) {
            if ( !( msg->wParam == SC_RESTORE && widget->isMinimized() ) )
                block_event = true;
        }
#endif

    return !block_event;
}

/*****************************************************************************
  Popup widget mechanism
 
  openPopup()
 Adds a widget to the list of popup widgets
 Arguments:
     QWidget *widget The popup widget to be added
 
  closePopup()
 Removes a widget from the list of popup widgets
 Arguments:
     QWidget *widget The popup widget to be removed
 *****************************************************************************/
void QApplication::openPopup( QWidget *popup )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "QApplication::openPopup( QWidget 0x%08p, name: %s )", popup, popup->name() );
#endif

    if ( !popupWidgets ) {   // create list
        popupWidgets = new QWidgetList;
        Q_CHECK_PTR( popupWidgets );
        if ( !activeBeforePopup )
            activeBeforePopup = new QGuardedPtr<QWidget>;
        ( *activeBeforePopup ) = focus_widget ? focus_widget : active_window;
    }
    popupWidgets->append( popup );  // add to end of list
    if ( popupWidgets->count() == 1 && !qt_nograb() )
        setAutoCapture( popup->winId() );        // grab mouse/keyboard
    // Popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    if ( popup->focusWidget() ) {
        popup->focusWidget() ->setFocus( );
    } else if ( popupWidgets->count() == 1 ) { // this was the first popup
        if ( QWidget * fw = focusWidget() ) {
            QFocusEvent e( QEvent::FocusOut );
            e.setReason( QFocusEvent::Popup );
            sendEvent( fw, &e );
        }
    }
}

void QApplication::closePopup( QWidget *popup )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "QApplication::closePopup( QWidget 0x%08p, name: %s )", popup, popup->name() );
#endif

    if ( !popupWidgets )
        return ;
    popupWidgets->removeRef( popup );

    POINT curPos;
    GetCursorPos( &curPos );
    replayPopupMouseEvent = ( !popup->geometry().contains( QPoint( curPos.x, curPos.y ) ) );

    if ( popupWidgets->isEmpty() ) { // this was the last popup
        delete popupWidgets;
        popupWidgets = 0;
        if ( !popup->isEnabled() )
            return ;
        if ( !qt_nograb() )                         // grabbing not disabled
            releaseAutoCapture();
        if ( active_window ) {
            if ( QWidget * fw = active_window->focusWidget() ) {
                if ( fw != focusWidget() ) {
                    fw->setFocus();
                } else {
                    QFocusEvent e( QEvent::FocusIn );
                    e.setReason( QFocusEvent::Popup );
                    sendEvent( fw, &e );
                }
            }
        }
    } else {
        // Popups are not focus-handled by the window system (the
        // first popup grabbed the keyboard), so we have to do that
        // manually: A popup was closed, so the previous popup gets
        // the focus.
        QWidget* aw = popupWidgets->last();
        if ( popupWidgets->count() == 1 )
            setAutoCapture( aw->winId() );
        if ( QWidget * fw = aw->focusWidget() )
            fw->setFocus();
    }
}

/*****************************************************************************
  Event translation; translates Windows events to Qt events
 *****************************************************************************/

//
// Auto-capturing for mouse press and mouse release
//
static void setAutoCapture( HWND h )
{
    if ( autoCaptureWnd )
        releaseAutoCapture();
    autoCaptureWnd = h;
    SetCapture( h );
}

static void releaseAutoCapture()
{
    if ( autoCaptureWnd ) {
        ReleaseCapture();
        autoCaptureWnd = 0;
    }
}

//
// Mouse event translation
//
// Non-client mouse messages are not translated
//
static ushort mouseTbl[] = {
    WM_MOUSEMOVE,       QEvent::MouseMove,          0,
    WM_LBUTTONDOWN,     QEvent::MouseButtonPress,   Qt::LeftButton,
    WM_LBUTTONUP,       QEvent::MouseButtonRelease, Qt::LeftButton,
    WM_LBUTTONDBLCLK,   QEvent::MouseButtonDblClick,Qt::LeftButton,
    WM_RBUTTONDOWN,     QEvent::MouseButtonPress,   Qt::RightButton,
    WM_RBUTTONUP,       QEvent::MouseButtonRelease, Qt::RightButton,
    WM_RBUTTONDBLCLK,   QEvent::MouseButtonDblClick,Qt::RightButton,
    WM_MBUTTONDOWN,     QEvent::MouseButtonPress,   Qt::MidButton,
    WM_MBUTTONUP,       QEvent::MouseButtonRelease, Qt::MidButton,
    WM_MBUTTONDBLCLK,   QEvent::MouseButtonDblClick,Qt::MidButton,
    0, 0, 0
};

static int translateButtonState( int s, int type, int button )
{
    Q_UNUSED( type );
    Q_UNUSED( button );
    int bst = 0;
    if ( s & MK_LBUTTON )
        bst |= Qt::LeftButton;
    if ( s & MK_MBUTTON )
        bst |= Qt::MidButton;
    if ( s & MK_RBUTTON )
        bst |= Qt::RightButton;
    if ( s & MK_SHIFT )
        bst |= Qt::ShiftButton;
    if ( s & MK_CONTROL )
        bst |= Qt::ControlButton;
    if ( GetKeyState( VK_MENU ) < 0 )
        bst |= Qt::AltButton;
    if ( ( GetKeyState( VK_LWIN ) < 0 ) || ( GetKeyState( VK_RWIN ) < 0 ) )
        bst |= Qt::MetaButton;

    return bst;
}

void qt_win_eatMouseMove()
{
    // after closing a windows dialog with a double click (i.e. open a file)
    // the message queue still contains a dubious WM_MOUSEMOVE message where
    // the left button is reported to be down (wParam != 0).
    // remove all those messages (usually 1) and post the last one with a
    // reset button state

    MSG msg = {0, 0, 0, 0, 0, 0, 0};
    QT_WA( {
               while ( PeekMessage( &msg, 0, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE ) )
               ;
               if ( msg.message == WM_MOUSEMOVE )
                   PostMessage( msg.hwnd, msg.message, 0, msg.lParam );
               },
            {
               while ( PeekMessageA( &msg, 0, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE ) )
                   ;
               if ( msg.message == WM_MOUSEMOVE )
                   PostMessageA( msg.hwnd, msg.message, 0, msg.lParam );
            } );
}

// In DnD, the mouse release event never appears, so the
// mouse button state machine must be manually reset
/*! \internal */
void QApplication::winMouseButtonUp()
{
    qt_button_down = 0;
    releaseAutoCapture();
}

bool QETWidget::translateMouseEvent( const MSG &msg )
{
    static QPoint pos;
    static POINT gpos = { -1, -1};
    QEvent::Type type;                                // event parameters
    int button;
    int state, realstate;
    int i;

    if ( sm_blockUserInput )  //block user interaction during session management
        return true;

    // Compress mouse move events
    if ( msg.message == WM_MOUSEMOVE ) {
        MSG mouseMsg;
        while ( winPeekMessage( &mouseMsg, msg.hwnd, WM_MOUSEFIRST,
                                WM_MOUSELAST, PM_NOREMOVE ) ) {
            if ( mouseMsg.message == WM_MOUSEMOVE ) {
#define PEEKMESSAGE_IS_BROKEN 1
#ifdef PEEKMESSAGE_IS_BROKEN
                // Since the Windows PeekMessage() function doesn't
                // correctly return the wParam for WM_MOUSEMOVE events
                // if there is a key release event in the queue
                // _before_ the mouse event, we have to also consider
                // key release events (kls 2003-05-13):
                MSG keyMsg;
                bool done = false;
                while ( winPeekMessage( &keyMsg, 0, WM_KEYFIRST, WM_KEYLAST,
                                        PM_NOREMOVE ) ) {
                    if ( keyMsg.time < mouseMsg.time ) {
                        if ( ( keyMsg.lParam & 0xC0000000 ) == 0x40000000 ) {
                            winPeekMessage( &keyMsg, 0, keyMsg.message,
                                            keyMsg.message, PM_REMOVE );
                        } else {
                            done = true;
                            break;
                        }
                    } else {
                        break; // no key event before the WM_MOUSEMOVE event
                    }
                }
                if ( done )
                    break;
#else
                // Actually the following 'if' should work instead of
                // the above key event checking, but apparently
                // PeekMessage() is broken :-(
                if ( mouseMsg.wParam != msg.wParam )
                    break; // leave the message in the queue because
                // the key state has changed
#endif

                MSG *msgPtr = ( MSG * ) ( &msg );
                // Update the passed in MSG structure with the
                // most recent one.
                msgPtr->lParam = mouseMsg.lParam;
                msgPtr->wParam = mouseMsg.wParam;
                msgPtr->pt = mouseMsg.pt;
                // Remove the mouse move message
                winPeekMessage( &mouseMsg, msg.hwnd, WM_MOUSEMOVE,
                                WM_MOUSEMOVE, PM_REMOVE );
            } else {
                break; // there was no more WM_MOUSEMOVE event
            }
        }
    }


    for ( i = 0; ( UINT ) mouseTbl[ i ] != msg.message && mouseTbl[ i ]; i += 3 )
        ;
    if ( !mouseTbl[ i ] )
        return false;
    type = ( QEvent::Type ) mouseTbl[ ++i ];        // event type
    button = mouseTbl[ ++i ];                       // which button
    state = translateButtonState( msg.wParam, type, button ); // button state
    realstate = state;
    if ( type == QEvent::MouseMove ) {
        if ( !( state & Qt::MouseButtonMask ) )
            qt_button_down = 0;
        QCursor *c = qt_grab_cursor();
        if ( !c )
            c = QApplication::overrideCursor();
        if ( c )                                 // application cursor defined
            SetCursor( c->handle() );
        else {
            QWidget *w = this; // use  widget cursor if widget is enabled
            while ( !w->isTopLevel() && !w->isEnabled() )
                w = w->parentWidget();
            SetCursor( w->cursor().handle() );
        }
        if ( curWin != winId() ) {                // new current window
            qt_dispatchEnterLeave( this, QWidget::find( curWin ) );
            curWin = winId();
#ifndef Q_OS_TEMP

            static bool trackMouseEventLookup = false;
            typedef BOOL ( WINAPI * PtrTrackMouseEvent ) ( LPTRACKMOUSEEVENT );
            static PtrTrackMouseEvent ptrTrackMouseEvent = 0;
            if ( !trackMouseEventLookup ) {
                trackMouseEventLookup = true;
                ptrTrackMouseEvent = ( PtrTrackMouseEvent ) QLibrary::resolve( "comctl32", "_TrackMouseEvent" );
            }
            if ( ptrTrackMouseEvent && !qApp->inPopupMode() ) {
                // We always have to set the tracking, since
                // Windows detects more leaves than we do..
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof( TRACKMOUSEEVENT );
                tme.dwFlags = 0x00000002;    // TME_LEAVE
                tme.hwndTrack = curWin;      // Track on window receiving msgs
                tme.dwHoverTime = ( DWORD ) - 1; // HOVER_DEFAULT
                ptrTrackMouseEvent( &tme );
            }
#endif // Q_OS_TEMP

        }

        POINT curPos = msg.pt;
        if ( curPos.x == gpos.x && curPos.y == gpos.y )
            return true;                        // same global position
        gpos = curPos;

        ScreenToClient( winId(), &curPos );

        pos.rx() = curPos.x;
        pos.ry() = curPos.y;
        //        pos = mapFromWS(pos); FIXME!
    } else {
// CE: This is important and maybe a little bit wrong - but works for now
// state: button & keyboard states *bfore* the button was pressed
// button: button that caused the event
        if ( type == QEvent::MouseButtonRelease ) {
            state |= button;
        } else {
            state &= ~button;
        }
        gpos = msg.pt;
        pos = mapFromGlobal( QPoint( gpos.x, gpos.y ) );

        if ( type == QEvent::MouseButtonPress || type == QEvent::MouseButtonDblClick ) {        // mouse button pressed
            // Magic for masked widgets
            qt_button_down = findChildWidget( this, pos );
            if ( !qt_button_down || !qt_button_down->testWFlags( Qt::WMouseNoMask ) )
                qt_button_down = this;
        }
    }

    bool res = false;

    if ( qApp->inPopupMode() ) {                        // in popup mode
        replayPopupMouseEvent = false;
        QWidget* activePopupWidget = qApp->activePopupWidget();
        QWidget *popup = activePopupWidget;

        if ( popup != this ) {
            if ( ( testWFlags( WType_Popup ) ) && rect().contains( pos ) )
                popup = this;
            else                                // send to last popup
                pos = popup->mapFromGlobal( QPoint( gpos.x, gpos.y ) );
        }
        QWidget *popupChild = findChildWidget( popup, pos );
        bool releaseAfter = false;
        switch ( type ) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonDblClick:
            popupButtonFocus = popupChild;
            break;
        case QEvent::MouseButtonRelease:
            releaseAfter = true;
            break;
        default:
            break;                                // nothing for mouse move
        }

        if ( popupButtonFocus )
            popup = popupButtonFocus;
        else if ( popupChild )
            popup = popupChild;

        QPoint globalPos( gpos.x, gpos.y );
        pos = popup->mapFromGlobal( globalPos );
        QMouseEvent e( type, pos, globalPos, button, state );
        res = QApplication::sendSpontaneousEvent( popup, &e );
        res = res && e.isAccepted();

        if ( releaseAfter ) {
            popupButtonFocus = 0;
            qt_button_down = 0;
        }

        if ( type == QEvent::MouseButtonPress
                && qApp->activePopupWidget() != activePopupWidget
                && replayPopupMouseEvent ) {
            // the popup dissappeared. Replay the event
            QWidget * w = QApplication::widgetAt( gpos.x, gpos.y );
            if ( w && !qt_blocked_modal( w ) ) {
// FIXME: this seems not work with qt3...
//                if ( QWidget::mouseGrabber() == 0 )
//                    setAutoCapture( w->winId() );
                POINT widgetpt = gpos;
                ScreenToClient( w->winId(), &widgetpt );
                LPARAM lParam = MAKELPARAM( widgetpt.x, widgetpt.y );
                winPostMessage( w->winId(), msg.message, msg.wParam, lParam );
            }
        } else if ( type == QEvent::MouseButtonRelease && button == Qt::RightButton
                    && qApp->activePopupWidget() == activePopupWidget ) {
            // popup still alive and received right-button-release
            QContextMenuEvent e2( QContextMenuEvent::Mouse, pos, globalPos, state );
            bool res2 = QApplication::sendSpontaneousEvent( popup, &e2 );
            if ( !res )  // RMB not accepted
                res = res2 && e2.isAccepted();
        }
    } else {                                        // not popup mode
        int bs = realstate & Qt::MouseButtonMask;
        if ( ( type == QEvent::MouseButtonPress ||
                type == QEvent::MouseButtonDblClick ) && bs == button ) {
            if ( QWidget::mouseGrabber() == 0 )
                setAutoCapture( winId() );
        } else if ( type == QEvent::MouseButtonRelease && bs == 0 ) {
            if ( QWidget::mouseGrabber() == 0 )
                releaseAutoCapture();
        }

        QWidget *widget = this;
        QWidget *w = QWidget::mouseGrabber();
        if ( !w )
            w = qt_button_down;
        if ( w && w != this ) {
            widget = w;
            pos = w->mapFromGlobal( QPoint( gpos.x, gpos.y ) );
        }

        if ( type == QEvent::MouseButtonRelease &&
                ( realstate & Qt::MouseButtonMask ) == 0 ) {
            qt_button_down = 0;
        }

        QMouseEvent e( type, pos, QPoint( gpos.x, gpos.y ), button, state );
        res = QApplication::sendSpontaneousEvent( widget, &e );
        res = res && e.isAccepted();
        if ( type == QEvent::MouseButtonRelease && button == Qt::RightButton ) {
            QContextMenuEvent e2( QContextMenuEvent::Mouse, pos, QPoint( gpos.x, gpos.y ), state );
            bool res2 = QApplication::sendSpontaneousEvent( widget, &e2 );
            if ( !res )
                res = res2 && e2.isAccepted();
        }

        if ( type != QEvent::MouseMove )
            pos.rx() = pos.ry() = -9999;        // init for move compression
    }
    return res;
}

//
// Wheel event translation
//
bool QETWidget::translateWheelEvent( const MSG &msg )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "qapplication_win.cpp: translateWheelEvent: %s, delta=%d", className(), delta );
#endif

    int state = 0;

    if ( sm_blockUserInput )  // block user interaction during session management
        return true;

    if ( GetKeyState( VK_SHIFT ) < 0 )
        state |= Qt::ShiftButton;
    if ( GetKeyState( VK_CONTROL ) < 0 )
        state |= Qt::ControlButton;
    if ( GetKeyState( VK_MENU ) < 0 )
        state |= Qt::AltButton;
    if ( ( GetKeyState( VK_LWIN ) < 0 ) || ( GetKeyState( VK_RWIN ) < 0 ) )
        state |= Qt::MetaButton;

    int delta;
    if ( msg.message == WM_MOUSEWHEEL )
        delta = ( short ) HIWORD ( msg.wParam );
    else
        delta = ( int ) msg.wParam;

    Qt::Orientation orient = ( state & Qt::AltButton
#if 0 // disabled for now - Trenton's one-wheel mouse makes trouble...
                               // "delta" for usual wheels is +-120. +-240 seems to indicate the second wheel
                               // see more recent MSDN for WM_MOUSEWHEEL

                               || delta == 240 || delta == -240 ) ? Qt::Horizontal : Vertical;
    if ( delta == 240 || delta == -240 )
        delta /= 2;
#endif

    ) ? Qt::Horizontal : Qt::Vertical;

    QPoint globalPos;


    globalPos.rx() = ( short ) LOWORD ( msg.lParam );
    globalPos.ry() = ( short ) HIWORD ( msg.lParam );


    // if there is a widget under the mouse and it is not shadowed
    // by modality, we send the event to it first
    int ret = 0;
    QWidget* w = QApplication::widgetAt( globalPos );
    if ( !w || !qt_try_modal( w, ( MSG* ) & msg, ret )
           )
        w = this;

        // send the event to the widget or its ancestors
    {
        QWidget* popup = qApp->activePopupWidget();
        if ( popup && w->topLevelWidget()
                != popup )
            popup->close();
        QWheelEvent e( w->mapFromGlobal( globalPos ), globalPos, delta, state, orient );
        if ( QApplication::sendSpontaneousEvent( w, &e ) )
            return true;
    }

    // send the event to the widget that has the focus or its ancestors, if different
    if ( ( w != qApp->focusWidget() ) &&
         ( w = qApp->focusWidget() ) ) {
        QWidget * popup = qApp->activePopupWidget();
        if ( popup && w->topLevelWidget() != popup )
            popup->close();
        QWheelEvent e( w->mapFromGlobal( globalPos ), globalPos, delta, state, orient );
        if ( QApplication::sendSpontaneousEvent( w, &e ) )
            return true;
    }
    return false;
}

//
// Keyboard event translation
//
static int inputcharset = CP_ACP;

struct KeyRec {
    KeyRec(int c, int a, int s, const QString& t) : code(c), ascii(a), state(s), text(t) { }
    KeyRec() { }
    int code, ascii, state;
    QString text;
};

static const int maxrecs=64; // User has LOTS of fingers...
static KeyRec key_rec[maxrecs];
static int nrecs=0;

static KeyRec* find_key_rec(int code, bool remove)
{
    KeyRec *result = 0;
    for (int i=0; i<nrecs; i++) {
        if (key_rec[i].code == code) {
            if (remove) {
                static KeyRec tmp;
                tmp = key_rec[i];
                while (i+1 < nrecs) {
                    key_rec[i] = key_rec[i+1];
                    i++;
                }
                nrecs--;
                result = &tmp;
            } else {
                result = &key_rec[i];
            }
            break;
        }
    }
    return result;
}

static void store_key_rec(int code, int ascii, int state, const QString& text)
{
    if (nrecs == maxrecs) {
        qWarning("Qt: Internal keyboard buffer overflow");
        return;
    }

    key_rec[nrecs++] = KeyRec(code,ascii,state,text);
}

static void clear_key_rec()
{
    nrecs = 0;
}

static bool isModifierKey(int code)
{
    return code >= Qt::Key_Shift && code <= Qt::Key_ScrollLock;
}

bool QETWidget::sendKeyEvent(MSG& msg, QEvent::Type type, int code, int ascii,
                              int state, bool grab, const QString& text,
                              bool autor)
{
    bool isAccel = false;
    if ( !grab && ( HIWORD( msg.lParam ) & KF_ALTDOWN ) ) { // test for accel if the keyboard is not grabbed
        QKeyEvent a( QEvent::AccelAvailable, code, ascii, state, text, false,
                     QMAX( 1, int( text.length() ) ) );
        a.ignore();
        QApplication::sendEvent( topLevelWidget(), &a );
        isAccel = a.isAccepted();
    }

    // process acceleraters before popups
    QKeyEvent e( type, code, ascii, state, text, autor,
                 QMAX( 1, int( text.length() ) ) );
    if ( type == QEvent::KeyPress && !grab ) {
        // send accel events if the keyboard is not grabbed
        QKeyEvent aa( QEvent::AccelOverride, code, ascii, state, text, autor,
                      QMAX( 1, int( text.length() ) ) );
        aa.ignore();
        QApplication::sendEvent( this, &aa );
        if ( !aa.isAccepted() ) {
            QKeyEvent a( QEvent::Accel, code, ascii, state, text, autor,
                         QMAX( 1, int( text.length() ) ) );
            a.ignore();
            QApplication::sendEvent( topLevelWidget(), &a );
            if ( a.isAccepted() )
                return FALSE;
        }
    }
    QApplication::sendSpontaneousEvent( this, &e );
    return e.isAccepted();
}

static const uint KeyTbl[] = {          // keyboard mapping table
    VK_ESCAPE,      Qt::Key_Escape,     // misc keys
    VK_TAB,         Qt::Key_Tab,
    VK_BACK,        Qt::Key_Backspace,
    VK_RETURN,      Qt::Key_Return,
    VK_INSERT,      Qt::Key_Insert,
    VK_DELETE,      Qt::Key_Delete,
    VK_CLEAR,       Qt::Key_Clear,
    VK_PAUSE,       Qt::Key_Pause,
    VK_SNAPSHOT,    Qt::Key_Print,
    VK_HOME,        Qt::Key_Home,       // cursor movement
    VK_END,         Qt::Key_End,
    VK_LEFT,        Qt::Key_Left,
    VK_UP,          Qt::Key_Up,
    VK_RIGHT,       Qt::Key_Right,
    VK_DOWN,        Qt::Key_Down,
    VK_PRIOR,       Qt::Key_PageUp,
    VK_NEXT,        Qt::Key_PageDown,
    VK_SHIFT,       Qt::Key_Shift,      // modifiers
    VK_CONTROL,     Qt::Key_Control,
    VK_LWIN,        Qt::Key_Meta,
    VK_RWIN,        Qt::Key_Meta,
    VK_MENU,        Qt::Key_Alt,
    VK_CAPITAL,     Qt::Key_CapsLock,
    VK_NUMLOCK,     Qt::Key_NumLock,
    VK_SCROLL,      Qt::Key_ScrollLock,
    VK_NUMPAD0,     Qt::Key_0,          // numeric Keypad
    VK_NUMPAD1,     Qt::Key_1,
    VK_NUMPAD2,     Qt::Key_2,
    VK_NUMPAD3,     Qt::Key_3,
    VK_NUMPAD4,     Qt::Key_4,
    VK_NUMPAD5,     Qt::Key_5,
    VK_NUMPAD6,     Qt::Key_6,
    VK_NUMPAD7,     Qt::Key_7,
    VK_NUMPAD8,     Qt::Key_8,
    VK_NUMPAD9,     Qt::Key_9,
    VK_MULTIPLY,    Qt::Key_Asterisk,
    VK_ADD,         Qt::Key_Plus,
    VK_SEPARATOR,   Qt::Key_Comma,
    VK_SUBTRACT,    Qt::Key_Minus,
    VK_DECIMAL,     Qt::Key_Period,
    VK_DIVIDE,      Qt::Key_Slash,
    VK_APPS,        Qt::Key_Menu,
    0,              0
};

static int translateKeyCode(int key)                // get Qt::Key_... code
{
    int code;
    if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) {
        code = 0;
    } else if (key >= VK_F1 && key <= VK_F24) {
        code = Qt::Key_F1 + (key - VK_F1);                // function keys
    } else {
        int i = 0;                                // any other keys
        code = 0;
        while (KeyTbl[i]) {
            if (key == (int)KeyTbl[i]) {
                code = KeyTbl[i+1];
                break;
            }
            i += 2;
        }
    }
    return code;
}

int qt_translateKeyCode(int key)
{
    return translateKeyCode(key);
}

static int asciiToKeycode(char a, int state)
{
    if (a >= 'a' && a <= 'z')
        a = toupper(a);
    if ((state & Qt::ControlButton) != 0) {
        if ( a >= 0 && a <= 31 )      // Ctrl+@..Ctrl+A..CTRL+Z..Ctrl+_
            a += '@';                 // to @..A..Z.._
    }

    return a & 0xff;
}

static
QChar wmchar_to_unicode(DWORD c)
{
    // qt_winMB2QString is the generalization of this function.
    QT_WA({
        return QChar((ushort)c);
    } , {
        char mb[2];
        mb[0] = c&0xff;
        mb[1] = 0;
        WCHAR wc[1];
        MultiByteToWideChar(inputcharset, MB_PRECOMPOSED, mb, -1, wc, 1);
        return QChar(wc[0]);
    });
}

static
QChar imechar_to_unicode(DWORD c)
{
    // qt_winMB2QString is the generalization of this function.
    QT_WA({
        return QChar((ushort)c);
    } , {
        char mb[3];
        mb[0] = (c>>8)&0xff;
        mb[1] = c&0xff;
        mb[2] = 0;
        WCHAR wc[1];
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
            mb, -1, wc, 1);
        return QChar(wc[0]);
    });
}

bool QETWidget::translateKeyEvent( MSG msg, bool grab )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "QETWidget::translateKeyEvent( %d, 0x%08x, 0x%08x, %d )", msg.message, msg.wParam, msg.lParam, grab );
#endif

    bool k0=false, k1=false;
    int  state = 0;

    if (sm_blockUserInput) // block user interaction during session management
        return true;

    if (GetKeyState(VK_SHIFT) < 0)
        state |= Qt::ShiftButton;
    if (GetKeyState(VK_CONTROL) < 0)
        state |= Qt::ControlButton;
    if (GetKeyState(VK_MENU) < 0)
        state |= Qt::AltButton;
    if ((GetKeyState(VK_LWIN) < 0) ||
         (GetKeyState(VK_RWIN) < 0))
        state |= Qt::MetaButton;

    if (msg.message == WM_CHAR) {
        // a multi-character key not found by our look-ahead
        QString s;
        QChar ch = wmchar_to_unicode(msg.wParam);
        if (!ch.isNull())
            s += ch;
        char a = ch.row() ? 0 : ch.cell();
        k0 = sendKeyEvent(msg, QEvent::KeyPress, 0, a, state, grab, s);
        k1 = sendKeyEvent(msg, QEvent::KeyRelease, 0, a, state, grab, s);
    }
    else if (msg.message == WM_IME_CHAR) {
        // input method characters not found by our look-ahead
        QString s;
        QChar ch = imechar_to_unicode(msg.wParam);
        if (!ch.isNull())
            s += ch;
        char a = ch.row() ? 0 : ch.cell();
        k0 = sendKeyEvent(msg, QEvent::KeyPress, 0, a, state, grab, s);
        k1 = sendKeyEvent(msg, QEvent::KeyRelease, 0, a, state, grab, s);
    } else {
        bool qt_use_rtl_extensions = false; // for now
//        extern bool qt_use_rtl_extensions;
        if (qt_use_rtl_extensions) {
            // for Directionality changes (BiDi)
            static int dirStatus = 0;
            if (!dirStatus && state == Qt::ControlButton && msg.wParam == VK_CONTROL && msg.message == WM_KEYDOWN) {
                if (GetKeyState(VK_LCONTROL) < 0) {
                    dirStatus = VK_LCONTROL;
                } else if (GetKeyState(VK_RCONTROL) < 0) {
                    dirStatus = VK_RCONTROL;
                }
            } else if (dirStatus) {
                if (msg.message == WM_KEYDOWN) {
                    if (msg.wParam == VK_SHIFT) {
                        if (dirStatus == VK_LCONTROL && GetKeyState(VK_LSHIFT) < 0) {
                                dirStatus = VK_LSHIFT;
                        } else if (dirStatus == VK_RCONTROL && GetKeyState(VK_RSHIFT) < 0) {
                            dirStatus = VK_RSHIFT;
                        }
                    } else {
                        dirStatus = 0;
                    }
                } else if (msg.message == WM_KEYUP) {
                    if (dirStatus == VK_LSHIFT &&
                        (msg.wParam == VK_SHIFT && GetKeyState(VK_LCONTROL)  ||
                          msg.wParam == VK_CONTROL && GetKeyState(VK_LSHIFT))) {
                        k0 = sendKeyEvent(msg, QEvent::KeyPress, Qt::Key_Direction_L, 0, 0, grab, QString());
                        k1 = sendKeyEvent(msg, QEvent::KeyRelease, Qt::Key_Direction_L, 0, 0, grab, QString());
                        dirStatus = 0;
                    } else if (dirStatus == VK_RSHIFT &&
                        (msg.wParam == VK_SHIFT && GetKeyState(VK_RCONTROL) ||
                          msg.wParam == VK_CONTROL && GetKeyState(VK_RSHIFT))) {
                        k0 = sendKeyEvent(msg, QEvent::KeyPress, Qt::Key_Direction_R, 0, 0, grab, QString());
                        k1 = sendKeyEvent(msg, QEvent::KeyRelease, Qt::Key_Direction_R, 0, 0, grab, QString());
                        dirStatus = 0;
                    } else {
                        dirStatus = 0;
                    }
                } else {
                    dirStatus = 0;
                }
            }
        }

        if(msg.wParam == VK_PROCESSKEY)
            // the IME will process these
            return true;

        int code = translateKeyCode(msg.wParam);
        // Invert state logic
        if (code == Qt::Key_Alt) {
            state = state^Qt::AltButton;
        } else if (code == Qt::Key_Control) {
            state = state^Qt::ControlButton;
        } else if (code == Qt::Key_Shift) {
            state = state^Qt::ShiftButton;
        }

        // If the bit 24 of lParm is set you received a enter,
        // otherwise a Return. (This is the extended key bit)
        if ((code == Qt::Key_Return) && (msg.lParam & 0x1000000)) {
            code = Qt::Key_Enter;
        }

        if (!(msg.lParam & 0x1000000)) {        // All cursor keys without extended bit
            switch (code) {
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
            case Qt::Key_Home:
            case Qt::Key_End:
            case Qt::Key_Insert:
            case Qt::Key_Delete:
            case Qt::Key_Asterisk:
            case Qt::Key_Plus:
            case Qt::Key_Minus:
            case Qt::Key_Period:
            case Qt::Key_0:
            case Qt::Key_1:
            case Qt::Key_2:
            case Qt::Key_3:
            case Qt::Key_4:
            case Qt::Key_5:
            case Qt::Key_6:
            case Qt::Key_7:
            case Qt::Key_8:
            case Qt::Key_9:
                state |= Qt::Keypad;
            default:
                if ((uint)msg.lParam == 0x004c0001 ||
                     (uint)msg.lParam == 0xc04c0001)
                    state |= Qt::Keypad;
                break;
            }
        } else {                                // And some with extended bit
            switch (code) {
            case Qt::Key_Enter:
            case Qt::Key_Slash:
            case Qt::Key_NumLock:
                state |= Qt::Keypad;
            default:
                break;
            }
        }

        int t = msg.message;
        if (t == WM_KEYDOWN || t == WM_IME_KEYDOWN || t == WM_SYSKEYDOWN) {
            // KEYDOWN
            KeyRec* rec = find_key_rec(msg.wParam, false);
            // If rec's state doesn't match the current state, something
            // has changed without us knowning about it. (Consumed by
            // modal widget is one posibility) So, remove rec from list.
            if ( rec && rec->state != state ) {
                find_key_rec( msg.wParam, TRUE );
                rec = 0;
            }
            // Find uch
            QChar uch;
            MSG wm_char;
            UINT charType = (t == WM_KEYDOWN ? WM_CHAR :
                              t == WM_IME_KEYDOWN ? WM_IME_CHAR : WM_SYSCHAR);
            if (winPeekMessage(&wm_char, 0, charType, charType, PM_REMOVE)) {
                // Found a XXX_CHAR
                uch = charType == WM_IME_CHAR
                        ? imechar_to_unicode(wm_char.wParam)
                        : wmchar_to_unicode(wm_char.wParam);
                if (t == WM_SYSKEYDOWN &&
                     uch.isLetter() && (msg.lParam & KF_ALTDOWN)) {
                    // (See doc of WM_SYSCHAR)
                    uch = uch.lower(); //Alt-letter
                }
                if (!code && !uch.row())
                    code = asciiToKeycode(uch.cell(), state);
            }
            if ( uch.isNull() && !( ( code == 0x40 ) && ( ( state & Qt::ControlButton ) != 0 ) ) ) {
                // No XXX_CHAR; deduce uch from XXX_KEYDOWN params
                if (msg.wParam == VK_DELETE)
                    uch = QChar((char)0x7f); // Windows doesn't know this one.
                else {
                    if (t != WM_SYSKEYDOWN || !code) {
                        UINT map;
                        QT_WA({
                            map = MapVirtualKey(msg.wParam, 2);
                        } , {
                            map = MapVirtualKeyA(msg.wParam, 2);
                            // High-order bit is 0x8000 on '95
                            if (map & 0x8000)
                                map = (map^0x8000)|0x80000000;
                        });
                        // If the high bit of the return value of
                        // MapVirtualKey is set, the key is a deadkey.
                        if (!(map & 0x80000000)) {
                            uch = wmchar_to_unicode((DWORD)map);
                        }
                    }
                }
                if (!code && !uch.row())
                    code = asciiToKeycode(uch.cell(), state);
            }

            if (state == Qt::AltButton) {
                // Special handling of global Windows hotkeys
                switch (code) {
                case Qt::Key_Escape:
                case Qt::Key_Tab:
                case Qt::Key_Enter:
                case Qt::Key_F4:
                    return false;                // Send the event on to Windows
                case Qt::Key_Space:
                    // do not pass this key to windows, we will process it ourselves
                    qt_show_system_menu(topLevelWidget());
                    return false;
                default:
                    break;
                }
            }

            // map shift+tab to shift+backtab, QShortcutMap knows about it
            // and will handle it
            if (code == Qt::Key_Tab && (state & Qt::ShiftButton) == Qt::ShiftButton)
                code = Qt::Key_Backtab;

            if (rec) {
                char a = uch.row() ? 0 : uch.cell();
                // it is already down (so it is auto-repeating)
                if (code < Qt::Key_Shift || code > Qt::Key_ScrollLock) {
                    k0 = sendKeyEvent(msg, QEvent::KeyRelease, code, a, state, grab, rec->text, true);
                    k1 = sendKeyEvent(msg, QEvent::KeyPress, code, a, state, grab, rec->text, true);
                }
            } else {
                QString text;
                if (!uch.isNull())
                    text += uch;
                char a = uch.row() ? 0 : uch.cell();
                k0 = sendKeyEvent(msg, QEvent::KeyPress, code, a, state, grab, text);

                bool store = true;
                // Alt+<alphanumerical> go to the Win32 menu system if unhandled by Qt
                if (msg.message == WM_SYSKEYDOWN && !k0 && a) {
                    HWND parent = GetParent(winId());
                    while (parent) {
                        if (GetMenu(parent)) {
                            SendMessage(parent, WM_SYSCOMMAND, SC_KEYMENU, a);
                            store = false;
                            k0 = true;
                            break;
                        }
                        parent = GetParent(parent);
                    }
                }
                if (store)
                    store_key_rec( msg.wParam, a, state, text );
            }
        } else {
            // Must be KEYUP
            KeyRec* rec = find_key_rec(msg.wParam, true);
            if (!rec) {
                // Someone ate the key down event
            } else {
                if (!code)
                    code = asciiToKeycode(rec->ascii ? rec->ascii : msg.wParam, state);
                // see comment above
                if (code == Qt::Key_Tab && (state & Qt::ShiftButton) == Qt::ShiftButton)
                    code = Qt::Key_Backtab;

                k0 = sendKeyEvent(msg, QEvent::KeyRelease, code, rec->ascii, state, grab, rec->text);

                // don't pass Alt to Windows unless we are embedded in a non-Qt window
                if ( code == Qt::Key_Alt ) {
                    k0 = true;
                    HWND parent = GetParent(winId());
                    while (parent) {
                        if (!QWidget::find(parent) && GetMenu(parent)) {
                            k0 = false;
                            break;
                        }
                        parent = GetParent(parent);
                    }
                }
            }
        }
    }

    return k0 || k1;
}

bool QETWidget::translateEraseEvent( const MSG &msg )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "QETWidget::translateEraseEvent: %s, %s", className(), name() );
#endif
    QRegion rgn;    // here we need empty_region - qt_erase_region needs it & doesn't touch it
    QPixmap *pm = ( extra ) ? extra->bg_pix : NULL;
    hdc = ( HDC ) msg.wParam;
    setWState( WState_InPaintEvent );
    qt_erase_region( this, pm, backgroundOffset(), rgn );
    clearWState ( WState_InPaintEvent );
    hdc = 0;
    /* An application should return nonzero in response to WM_ERASEBKGND
       if it processes the message and erases the background */
    return true;
}

bool QETWidget::translatePaintEvent( const MSG &msg )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "QETWidget::translatePaintEvent %s, %s", className(), name() );
#endif

    PAINTSTRUCT ps;
    QRegion paintRegion( 0, 0, 1, 1 );  // otherwise we would pollute empty_region
    int res = GetUpdateRgn ( winid, paintRegion.handle (), FALSE );

    if ( !GetUpdateRect( winId(), 0, false )  // The update bounding rect is invalid
         || ( res == ERROR )
         || ( res == NULLREGION ) ) {
        return false;
    }

    hdc = BeginPaint( winid, &ps );
    setWState( WState_InPaintEvent );
#ifdef DEBUG_QAPPLICATION

    qDebug( "painting region %d,%d-%d,%d", ps.rcPaint.top, ps.rcPaint.left, ps.rcPaint.bottom, ps.rcPaint.right );
    qDebug( "QETWidget::translatePaintEvent crect %d,%d-%d,%d", crect.left(), crect.top(), crect.right(), crect.bottom() );
#endif
    repaint( paintRegion, !testWFlags( WRepaintNoErase ) );

    clearWState( WState_InPaintEvent );
    EndPaint( winid, &ps );
    hdc = 0;
    return true;
}

//
// ConfigureNotify (window move and resize) event translation
//
bool QETWidget::translateConfigEvent( const MSG &msg )
{
    if ( !testWState( Qt::WState_Created ) )                 // in QWidget::create()
        return true;
    if ( testWState( Qt::WState_ConfigPending ) )
        return true;
    if ( !isTopLevel() )
        return true;
    setWState( Qt::WState_ConfigPending );                // set config flag
    QRect cr = geometry();
    if ( msg.message == WM_SIZE ) {                // resize event
        WORD a = LOWORD( msg.lParam );
        WORD b = HIWORD( msg.lParam );
        QSize oldSize = size();
        QSize newSize( a, b );
        cr.setSize( newSize );
        if ( msg.wParam != SIZE_MINIMIZED )
            crect = cr;
        if ( isTopLevel() ) {                        // update title/icon text
            createTLExtra();
            // Capture SIZE_MINIMIZED without preceding WM_SYSCOMMAND
            // (like Windows+M)
            if ( msg.wParam == SIZE_MINIMIZED && !isMinimized() ) {
                setWindowState( windowState() | Qt::WindowMinimized );
                if ( isVisible() ) {
                    QHideEvent e;
                    QApplication::sendSpontaneousEvent( this, &e );
                    hideChildren( true );
                }
            } else if ( msg.wParam != SIZE_MINIMIZED && isMinimized() ) {
                setWindowState( windowState() & ( ~Qt::WindowMinimized ) );
                showChildren( true );
                QShowEvent e;
                QApplication::sendSpontaneousEvent( this, &e );
            }
            QString txt;
#ifndef Q_OS_TEMP

            if ( IsIconic( winId() ) && iconText().length() )
                txt = iconText();
            else
#endif
                txt = caption();
            if ( !txt.isEmpty() )
                setCaption( txt );
        }
        if ( msg.wParam != SIZE_MINIMIZED && oldSize != newSize ) {
            if ( isVisible() ) {
                QResizeEvent e( newSize, oldSize );
                QApplication::sendSpontaneousEvent( this, &e );
                if ( !testWFlags( Qt::WStaticContents ) )
                    testWState( Qt::WState_InPaintEvent ) ? update() : repaint();
            } else {
                QResizeEvent *e = new QResizeEvent( newSize, oldSize );
                QApplication::postEvent( this, e );
            }
        }
    } else if ( msg.message == WM_MOVE ) {        // move event
        int a = ( int ) ( short ) LOWORD( msg.lParam );
        int b = ( int ) ( short ) HIWORD( msg.lParam );
        QPoint oldPos = geometry().topLeft();
        QPoint newCPos( a, b );
        // Ignore silly Windows move event to wild pos after iconify.
        if ( !IsIconic( winId() ) && newCPos != oldPos ) {
            cr.moveTopLeft( newCPos );
            crect = cr;
            if ( isVisible() ) {
                QMoveEvent e( newCPos, oldPos );  // cpos (client position)
                QApplication::sendSpontaneousEvent( this, &e );
            } else {
                QMoveEvent * e = new QMoveEvent( newCPos, oldPos );
                QApplication::postEvent( this, e );
            }
        }
    }
    clearWState( Qt::WState_ConfigPending );                // clear config flag
    return true;
}


//
// Close window event translation.
//
bool QETWidget::translateCloseEvent( WPARAM /*wParam*/, LPARAM /*lParam*/ )
{
#ifdef DEBUG_QAPPLICATION
    qDebug( "QETWidget::translateCloseEvent ( hdc: 0x%08p, id: 0x%08X )", hdc, winId() );
#endif

    return close( false );
}


/*!
  Sets the text cursor's flash time to \a msecs milliseconds.  The
  flash time is the time required to display, invert and restore the
  caret display: A full flash cycle.  Usually, the text cursor is
  displayed for \a msecs/2 milliseconds, then hidden for \a msecs/2
  milliseconds, but this may vary.
 
  Note that on Microsoft Windows, calling this function sets the
  cursor flash time for all windows.
 
  \sa cursorFlashTime()
 */
void QApplication::setCursorFlashTime( int msecs )
{
    SetCaretBlinkTime(msecs / 2);
    cursor_flash_time = msecs;
}


/*!
  Returns the text cursor's flash time in milliseconds. The flash time
  is the time required to display, invert and restore the caret
  display.
 
  The default value on X11 is 1000 milliseconds. On Windows, the
  control panel value is used.
 
  Widgets should not cache this value since it may vary any time the
  user changes the global desktop settings.
 
  \sa setCursorFlashTime()
 */
int QApplication::cursorFlashTime()
{
    int blink = (int)GetCaretBlinkTime();
    if (!blink)
        return cursor_flash_time;
    if (blink > 0)
        return 2*blink;
    return 0;
}

/*!
  Sets the time limit that distinguishes a double click from two
  consecutive mouse clicks to \a ms milliseconds.
 
  Note that on Microsoft Windows, calling this function sets the
  double click interval for all windows.
 
  \sa doubleClickInterval()
*/

void QApplication::setDoubleClickInterval( int ms )
{
#ifndef Q_OS_TEMP
    SetDoubleClickTime(ms);
#endif
    mouse_double_click_time = ms;
}


/*!
  Returns the maximum duration for a double click.
 
  The default value on X11 is 400 milliseconds. On Windows, the control
  panel value is used.
 
  \sa setDoubleClickInterval()
*/

int QApplication::doubleClickInterval()
{
    int ms = GetDoubleClickTime();
    if (ms != 0)
        return ms;
    return mouse_double_click_time;
}

/*!
  This function is available only on Windows.
 
  If gotFocus is TRUE, widget will become the active window. Otherwise the
  active window is reset to NULL.
*/
void QApplication::winFocus( QWidget *widget, bool gotFocus )
{
    if ( qApp->inPopupMode() ) // some delayed focus event to ignore
        return;       
    if (gotFocus) {
        setActiveWindow(widget);
        QWidget *mw = QApplication::active_window;
        QWidget *mwOld = NULL;
        if (mw && mw->isDialog()) {
            // raise the entire application, not just the dialog
            while(mw->parentWidget() && mw->isDialog() && mw != mwOld ) {
                mw = mw->topLevelWidget();
                mwOld = mw;
            }
            if (mw != QApplication::active_window)
                SetWindowPos(mw->winId(), HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
        }
    } else {
        setActiveWindow(0);
    }
}

// Windows specific code
extern "C"
LRESULT CALLBACK qt_window_procedure ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    bool result = true;
    QETWidget *widget = 0;

#ifdef QT_REAL_HARD_DEBUG
    qDebug( "qt_window_procedure( hWnd: 0x%08p, uMsg: %d, wParam: %d, lParam: %d )", hWnd, uMsg, wParam, lParam );
    HDC mhdc = widget ? widget->handle() : 0;
    HWND mwid = widget ? widget->parentWidget() ? widget->parentWidget()->winId() : 0 : 0;
    qDebug( "qt_window_procedure: hdc:  0x%08p, ParentWinID: 0x%08X, GetParent( 0x%08X )", mhdc, mwid, GetParent( hWnd ) );
#endif

#ifdef DEBUG_QAPPLICATION
    qDebug( "qapplication_win.cpp: qt_window_procedure(widget=%X)", ( int ) widget );
#endif

    // invoke winEventFilter
    MSG msg;
    msg.hwnd = hWnd;
    msg.message = uMsg;
    msg.wParam = wParam;
    msg.lParam = lParam;
    msg.time = 0; // not used
    msg.pt.x = GET_X_LPARAM(lParam);
    msg.pt.y = GET_Y_LPARAM(lParam);
    ClientToScreen(msg.hwnd, &msg.pt);         // the coords we get are client coords

    // app Filter
    if ( qt_winEventFilter( &msg ) )
        return 1;

    switch ( uMsg ) {
        case WM_QUERYENDSESSION: {
            if ( sm_smActive ) // bogus message from windows
                return TRUE;
    
            sm_smActive = true;
            sm_blockUserInput = true; // prevent user-interaction outside interaction windows
            sm_cancel = false;
            if ( qt_session_manager_self )
                qApp->commitData( *qt_session_manager_self );
            if ( lParam == (LPARAM)ENDSESSION_LOGOFF ) {
                _flushall();
            }
            return ( !sm_cancel );
        }
        case WM_ENDSESSION: {
            sm_smActive = false;
            sm_blockUserInput = false;
            bool endsession = (bool) wParam;
    
            if ( endsession ) {
    /*
                // since the process will be killed immediately quit() has no real effect
                int index = QApplication::staticMetaObject.indexOfSignal( "aboutToQuit()" );
                qApp->qt_metacall( QMetaObject::InvokeMetaMethod, index,0 );
    */
                qApp->quit();
            }
            return FALSE;
        }
        case WM_DISPLAYCHANGE:
            if ( qt_desktopWidget ) {
                qt_desktopWidget->move( GetSystemMetrics(76), GetSystemMetrics(77) );
                QSize sz( GetSystemMetrics(78), GetSystemMetrics(79) );
                if ( sz == qt_desktopWidget->size() ) {
                     // a screen resized without changing size of the virtual desktop
                    QResizeEvent rs( sz, qt_desktopWidget->size() );
                    QApplication::sendEvent( qt_desktopWidget, &rs );
                } else {
                    qt_desktopWidget->resize( sz );
                }
            }
            break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_XBUTTONDOWN:
            if (qt_win_ignoreNextMouseReleaseEvent)
                qt_win_ignoreNextMouseReleaseEvent = false;
            break;

        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_XBUTTONUP:
            if (qt_win_ignoreNextMouseReleaseEvent) {
                qt_win_ignoreNextMouseReleaseEvent = false;
                if (qt_button_down && qt_button_down->winId() == autoCaptureWnd) {
                    releaseAutoCapture();
                    qt_button_down = 0;
                }
                return FALSE;
            }
            break;
        default:
            break;
    }

/*
    QETWidget *keywidget = 0;
    bool grabbed = false;
    if ( uMsg == WM_KEYDOWN    || uMsg == WM_KEYUP ||
         uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP ) {
        keywidget = ( QETWidget* ) QWidget::keyboardGrabber();
        if ( keywidget ) {
            grabbed = TRUE;
        } else {
            if ( qApp->focusWidget () )
                keywidget = ( QETWidget* ) qApp->focusWidget ();
            else if ( QETWidget::inPopupMode(qApp) )
                widget = (QETWidget*) qApp->activePopupWidget();
            if ( !keywidget && widget )
                keywidget = widget->focusWidget() ? ( QETWidget* ) widget->focusWidget() : widget;
        }
    } */

    if ( !widget )
        widget = (QETWidget*)QWidget::find( hWnd );
    if ( !widget )      // don't know this widget
        goto do_default;

    if ( app_do_modal ) {  // modal event handling
        int ret = 0;
        if ( !qt_try_modal( widget, &msg, ret ) )
            return ret;
    }

    if (widget->invokeWinEvent(&msg))                // send through widget filter
        return FALSE;

    // WM95_MOUSEWHEEL is not const

    if ((uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST ||
         uMsg >= WM_XBUTTONDOWN && uMsg <= WM_XBUTTONDBLCLK) &&
         uMsg != WM_MOUSEWHEEL) {
        if ( qApp->activePopupWidget() != 0 ) { // in popup mode
            POINT curPos = msg.pt;
            QWidget* w = QApplication::widgetAt( curPos.x, curPos.y );
            if ( w )
                widget = ( QETWidget* ) w;
        }
    
        result =  widget->translateMouseEvent( msg );
    } else
    if ( uMsg == WM95_MOUSEWHEEL ) {
        result = widget->translateWheelEvent( msg );
    } else
    switch ( uMsg ) {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_IME_CHAR:
        case WM_IME_KEYDOWN:
        case WM_CHAR: {
            MSG msg1;
            bool anyMsg = winPeekMessage( &msg1, msg.hwnd, 0, 0, PM_NOREMOVE );
            if ( anyMsg && msg1.message == WM_DEADCHAR ) {
                result = true; // consume event since there is a dead char next
                break;
            }
            QWidget *g = QWidget::keyboardGrabber();
            if ( g )
                widget = ( QETWidget* ) g;
            else if ( QApplication::activePopupWidget() )
                widget = ( QETWidget* ) QApplication::activePopupWidget();
            else if ( qApp->focusWidget() )
                widget = ( QETWidget* ) qApp->focusWidget();
            else if ( !widget || widget->winId() == GetFocus() )  // We faked the message to go to exactly that widget.
                widget = ( QETWidget* ) widget->topLevelWidget();
            if ( widget->isEnabled() )
                result =  widget->translateKeyEvent( msg, g != 0 );
            break;
        }
        case WM_SYSCHAR:
            result = true;                        // consume event
            break;
        case WM_MOUSEWHEEL:
            result = widget->translateWheelEvent( msg );
            break;
#ifndef Q_OS_TEMP
        case WM_NCHITTEST:
            if ( widget->isTopLevel() ) {
                QPoint pos = widget->mapFromGlobal(QPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
                // don't show resize-cursors for fixed-size widgets
                int fleft = widget->frameGeometry().left();
                int ftop = widget->frameGeometry().top();

                if (widget->minimumWidth() == widget->maximumWidth() && (pos.x() < 0 || pos.x() >= widget->width()))
                    break;
                if (widget->minimumHeight() == widget->maximumHeight() && (pos.y() < -(ftop - fleft) || pos.y() >= widget->height()))
                    break;
            }

            result = false;
            break;
        case WM_NCMOUSEMOVE: {
                // span the application wide cursor over the
                // non-client area.
                QCursor *c = qt_grab_cursor();
                if (!c)
                    c = QApplication::overrideCursor();
                if (c)        // application cursor defined
                    SetCursor( c->handle() );
                else
                    result = false;
                // generate leave event also when the caret enters
                // the non-client area.
                qt_dispatchEnterLeave(0, QWidget::find(curWin));
                curWin = 0;
            }
            break;
#endif
        case WM_SYSCOMMAND: {
#ifndef Q_OS_TEMP
            bool window_state_change = false;
            unsigned int oldstate = widget->windowState();
            switch(0xfff0 & wParam) {
            case SC_CONTEXTHELP:
#ifndef QT_NO_WHATSTHIS
                QWhatsThis::enterWhatsThisMode();
#endif
                QT_WA({
                    DefWindowProcW( hWnd, WM_NCPAINT, 1, 0 );
                } , {
                    DefWindowProcA( hWnd, WM_NCPAINT, 1, 0 );
                });
                break;
            case SC_MINIMIZE:
                window_state_change = true;
                widget->setWindowStateET( oldstate | Qt::WindowMinimized );
                if ( widget->isVisible() ) {
                    QHideEvent e;
                    widget->sendSpontaneousEvent( widget, &e );
                    widget->hideChildren(true);
                }
                result = false;
                break;
            case SC_MAXIMIZE:
                if( widget->isTopLevel() )
                    widget->setNormalGeometry( widget->geometry() );
            case SC_RESTORE:
                window_state_change = true;
                if ( ( 0xfff0 & wParam ) == SC_MAXIMIZE )
                    widget->setWindowStateET( oldstate | Qt::WindowMaximized );
                else if (!widget->isMinimized())
                    widget->setWindowStateET( oldstate & ~Qt::WindowMaximized );

                if ( widget->isMinimized() ) {
                    widget->setWindowStateET( oldstate & ~Qt::WindowMinimized );
                    widget->showChildren( true );
                    QShowEvent e;
                    widget->sendSpontaneousEvent( widget, &e );
                }
                result = false;
                break;
            default:
                result = false;
                break;
            }

            if ( window_state_change ) {
                QEvent e( QEvent::WindowStateChange );
                widget->sendSpontaneousEvent( widget, &e );
            }
#endif
            break;
        }
        case WM_SETTINGCHANGE:
            if ( !msg.wParam ) {
                QString area = QT_WA_INLINE(QString::fromUcs2( ( unsigned short * ) msg.lParam ),
                                             QString::fromLocal8Bit( ( char* ) msg.lParam ) );
                if ( area == "intl" )
                    QApplication::postEvent( widget, new QEvent( QEvent::LocaleChange ) );
            }
            if (QApplication::desktopSettingsAware() && wParam != SPI_SETWORKAREA) {
                widget->markFrameStrutDirty();
                if (!widget->parentWidget())
                    qt_set_windows_resources();
            }
            break;
        case WM_SYSCOLORCHANGE:
            if (QApplication::desktopSettingsAware()) {
                if (!widget->parentWidget())
                    qt_set_windows_resources();
            }
            break;

        case WM_PAINT:
            result = widget->translatePaintEvent( msg );
            break;
        case WM_ERASEBKGND:
            result = widget->translateEraseEvent( msg );
            break;
        case WM_MOVE:                                // move window
        case WM_SIZE:                                // resize window
             result =  widget->translateConfigEvent( msg );
             break;
        case WM_ACTIVATEAPP:
            if (wParam == FALSE)
                widget->setActiveWindow( 0 );
            break;
        case WM_ACTIVATE:
            if ( QApplication::activePopupWidget() && LOWORD( wParam ) == WA_INACTIVE &&
                    QWidget::find( ( HWND ) lParam ) == 0 ) {
                // Another application was activated while our popups are open,
                // then close all popups.  In case some popup refuses to close,
                // we give up after 1024 attempts (to avoid an infinite loop).
                int maxiter = 1024;
                QWidget *popup;
                while ( ( popup = QApplication::activePopupWidget() ) && maxiter-- )
                    popup->close();
            }
    /*
            // If we are a tool with no child or us that accepts focus then reject
            // the activation
            if (LOWORD(wParam) == WA_ACTIVE && widget->testWFlags( Qt::WStyle_Tool )) {
                QETWidget *fw = widget;
                while ((fw = fw->nextInFocusChain()) != widget && fw->focusPolicy() == QWidget::NoFocus)
                    ;
                if (fw == widget && widget->focusPolicy() == QWidget::NoFocus) {
                    result = true;
                    break;
                }
            } */
            if (LOWORD(wParam) != WA_INACTIVE)
                qApp->winFocus( widget, true );
            // Windows tries to activate a modally blocked window.
            // This happens when restoring an application after "Show Desktop"
            if ( app_do_modal && LOWORD( wParam ) == WA_ACTIVE ) {
                QWidget * top = 0;
                if ( !qt_tryModalHelper( widget, &top ) && top && widget != top )
                    top->setActiveWindow();
            }
            if ( LOWORD( wParam ) == WA_INACTIVE )
                clear_key_rec(); // Ensure nothing gets consider an auto-repeat press later
            break;

#ifndef Q_OS_TEMP
        case WM_MOUSEACTIVATE: {
            const QWidget * tlw = widget->topLevelWidget();
            // Do not change activation if the clicked widget is inside a floating dock window
            if ( tlw->inherits( "QDockWidget" ) && qApp->activeWindow()
                    && !qApp->activeWindow() ->inherits( "QDockWidget" ) )
                return MA_NOACTIVATE;
            return MA_ACTIVATE;
        }
#endif

        case WM_MOUSELEAVE:
            // We receive a mouse leave for curWin, meaning
            // the mouse was moved outside our widgets
            if ( widget->winId() == curWin ) {
                bool dispatch = !widget->hasMouse();
                // hasMouse is updated when dispatching enter/leave,
                // so test if it is actually up-to-date
                if ( !dispatch ) {
                    QRect geom = widget->geometry();
                    if ( widget->parentWidget() && !widget->isTopLevel() ) {
                        QPoint gp = widget->parentWidget() ->mapToGlobal( widget->pos() );
                        geom.setX( gp.x() );
                        geom.setY( gp.y() );
                    }
                    QPoint cpos = QCursor::pos();
                    dispatch = !geom.contains( cpos );
                    if ( !dispatch ) {
                        QWidget * hittest = QApplication::widgetAt( cpos );
                        dispatch = !hittest || hittest->winId() != curWin;
                    }
                    if ( !dispatch ) {
                        HRGN hrgn = CreateRectRgn( 0, 0, 0, 0 );
                        if ( GetWindowRgn( curWin, hrgn ) != ERROR ) {
                            QPoint lcpos = widget->mapFromGlobal( cpos );
                            dispatch = !PtInRegion( hrgn, lcpos.x(), lcpos.y() );
                        }
                        DeleteObject( hrgn );
                    }
                }
                if ( dispatch ) {
                    qt_dispatchEnterLeave( 0, QWidget::find( ( WId ) curWin ) );
                    curWin = 0;
                }
            }
            break;
        case WM_CLOSE:
            widget->translateCloseEvent( wParam, lParam );
            return false;
        case WM_DESTROY:
            if ( hWnd == curWin ) {
                QEvent leave( QEvent::Leave );
                QApplication::sendEvent( widget, &leave );
                curWin = 0;
            }
            if (widget == popupButtonFocus)
                popupButtonFocus = 0;
            result = false;
            break;
        case WM_GETMINMAXINFO: {
                MINMAXINFO *minMaxInfo = ( MINMAXINFO * ) lParam;
                if ( widget ) {
                    QSize minSize = widget->minimumSize ();
                    QSize maxSize = widget->maximumSize ();
                    DWORD dwStyle = QT_WA_INLINE(
                        GetWindowLongW( widget->winId(), GWL_STYLE ),
                        GetWindowLongA( widget->winId(), GWL_STYLE ) );
                    DWORD dwExtStyle = QT_WA_INLINE(
                        GetWindowLongW( widget->winId(), GWL_EXSTYLE ),
                        GetWindowLongA( widget->winId(), GWL_EXSTYLE ) );
                    RECT rect;
                    rect.left = 0;
                    rect.top = 0;
                    rect.right = minSize.width();
                    rect.bottom = minSize.height();
                    AdjustWindowRectEx( &rect, dwStyle, FALSE, dwExtStyle );
                    minMaxInfo->ptMinTrackSize.x = rect.right - rect.left;
                    minMaxInfo->ptMinTrackSize.y = rect.bottom - rect.top;
                    rect.left = 0;
                    rect.top = 0;
                    rect.right = maxSize.width();
                    rect.bottom = maxSize.height();
                    AdjustWindowRectEx( &rect, dwStyle, FALSE, dwExtStyle );
                    minMaxInfo->ptMaxTrackSize.x = rect.right - rect.left;
                    minMaxInfo->ptMaxTrackSize.y = rect.bottom - rect.top;
                }
                break;
            }
        case WM_INPUTLANGCHANGE: {
                char info[ 7 ];
                if ( !GetLocaleInfoA( MAKELCID( lParam, SORT_DEFAULT ), LOCALE_IDEFAULTANSICODEPAGE, info, 6 ) ) {
                    inputcharset = CP_ACP;
                } else {
                    inputcharset = QString( info ).toInt();
                }
                break;
            }
        case WM_KILLFOCUS:
            if (!QWidget::find((HWND)wParam)) { // we don't get focus, so unset it now
                if (!widget->hasFocus()) // work around Windows bug after minimizing/restoring
                    widget = (QETWidget*)qApp->focusWidget();
                HWND focus = ::GetFocus();
                if (!widget || (focus && ::IsChild(widget->winId(), focus))) {
                    result = false;
                } else {
                    widget->clearFocus();
                    result = true;
                }
            } else {
                result = false;
            }
            break;
        /* for qclipboard_win.cpp */
        case WM_DRAWCLIPBOARD:
            clipboardHasChanged = true;
            if ( qt_clipboard ) {
                QEvent ev( QEvent::Clipboard );
                QApplication::sendEvent( qt_clipboard, &ev );
            }
            // pass the message to the next link.
            QT_WA (
                SendMessageW( hwndNextViewer, uMsg, wParam, lParam );,
                SendMessageA( hwndNextViewer, uMsg, wParam, lParam );
            )
            result = true;
            break;
        case WM_CHANGECBCHAIN:
            /* from msdn: "Processing the WM_CHANGECBCHAIN Message" */
            // If the next window is closing, repair the chain.

            if ( ( HWND ) wParam == hwndNextViewer ) {
                hwndNextViewer = ( HWND ) lParam;
            // Otherwise, pass the message to the next link.
            } else {
                QT_WA (
                    SendMessageW( hwndNextViewer, uMsg, wParam, lParam );,
                    SendMessageA( hwndNextViewer, uMsg, wParam, lParam );
                )
            }
            result = true;
            break;
        default:
            result = false;
            break;
    }
#ifdef DEBUG_QAPPLICATION

    qDebug( "qapplication_win.cpp: processNextEvent: DefWindowProc: %s", getMsgName( uMsg ) );
#endif

    if (result)
        return FALSE;
do_default:
    return QT_WA_INLINE ( DefWindowProcW( hWnd, uMsg, wParam, lParam ),
                          DefWindowProcA( hWnd, uMsg, wParam, lParam ) );
}

/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/
#if defined (Q_OS_CYGWIN)
size_t wcslen ( const TCHAR *s )
{
    const TCHAR * p;
    for ( p = s; *p; ++p )
        ;
    return p -s;
}
#endif

HINSTANCE qWinAppInst()
{
    return appInst;
}

HINSTANCE qWinAppPrevInst()
{
    return appPrevInst;
}

Qt::WindowsVersion QApplication::winVersion()
{
    return qt_winver;
}

static void set_winapp_name()
{
    static bool already_set = false;
    if ( !already_set ) {
        already_set = true;
#ifndef Q_OS_TEMP
        GetModuleFileNameA( appInst, appFileName, sizeof( appFileName ) );
#else
        QString afm;
        afm.setLength( 256 );
        afm.setLength( GetModuleFileName( appInst, ( unsigned short* ) afm.unicode(), 255 ) );
        strncpy( appFileName, afm.latin1(), afm.length() );
#endif

        const char *p = strrchr( appFileName, '\\' );        // skip path
        if ( p )
            memcpy( appName, p + 1, qstrlen( p ) );
        int l = qstrlen( appName );
        if ( ( l > 4 ) && !qstricmp( appName + l - 4, ".exe" ) )
            appName[ l - 4 ] = '\0';                // drop .exe extension
    }
    mwTitle = appName;
}

Q_EXPORT char const *qAppFileName()
{
    if ( !appFileName[0] )
        set_winapp_name();
    return appFileName;
}

const char *qAppName()
{
    if ( !appName[0] )
        set_winapp_name();
    return appName;
}

static void qWinMsgHandler( QtMsgType t, const char* msg )
{
#if defined(QT_THREAD_SUPPORT)
    static QCriticalSection staticSection;
#endif

    if ( !msg )
        msg = "(null)";
    QCString s = msg;
    s += "\n";
#if defined(QT_THREAD_SUPPORT)
    staticSection.enter();
#endif

    /* We only output ascii here since uincode conversion doesn't work on shutdown... */
    OutputDebugStringA( s.data() );

#if defined(QT_THREAD_SUPPORT)
    staticSection.leave();
#endif

    if ( t == QtFatalMsg )
        exit( 1 );
}

/*****************************************************************************
  qWinMain() - Initializes Windows. Called from WinMain() in qtmain_win.cpp
 *****************************************************************************/
// from qt4
// template implementation of the parsing algorithm
template <typename Char>
static QMemArray<Char*> qWinCmdLine( Char *cmdParam, int length, int &argc )
{
    QMemArray<Char*> argv( 8 );
    Char *p = cmdParam;
    Char *p_end = p + length;

    //the first one is the appfilename
    argv[ 0 ] = ( Char* ) qAppFileName();

    argc = 1;

    while ( *p && p < p_end ) {                              // parse cmd line arguments
        while ( QChar( *p ).isSpace() )                      // skip white space
            p++;
        if ( *p && p < p_end ) {                             // arg starts
            int quote;
            Char *start, *r;
            if ( *p == Char( '\"' ) || *p == Char( '\'' ) ) {    // " or ' quote
                quote = *p;
                start = ++p;
            } else {
                quote = 0;
                start = p;
            }
            r = start;
            while ( *p && p < p_end ) {
                if ( quote ) {
                    if ( *p == quote ) {
                        p++;
                        if ( QChar( *p ).isSpace() )
                            break;
                        quote = 0;
                    }
                } else if ( *p == '\\' ) {                // escape char?
                    p++;
                    if ( *p == Char( '\"' ) || *p == Char( '\'' ) )
                        ;                        // yes
                    else
                        p--;                     // treat \ literally
                } else {
                    if ( *p == Char( '\"' ) || *p == Char( '\'' ) ) {        // " or ' quote
                        quote = *p++;
                        continue;
                    } else if ( QChar( *p ).isSpace() )
                        break;
                }
                if ( *p )
                    * r++ = *p++;
            }
            if ( *p && p < p_end )
                p++;
            *r = Char( '\0' );

            if ( argc >= ( int ) argv.size() - 1 )         // expand array
                argv.resize( argv.size() * 2 );
            argv[ argc++ ] = start;
        }
    }
    argv[ argc ] = 0;

    return argv;
}

#if defined( Q_OS_TEMP )
void Q_EXPORT __cdecl qWinMain( HINSTANCE hinstance, HINSTANCE prevInstance, LPSTR lpCmdParam, int show, int &argc, QMemArray<pchar> &argv )
#else
void Q_EXPORT qWinMain( HINSTANCE hinstance, HINSTANCE prevInstance, LPSTR lpCmdParam, int show, int &argc, QMemArray<pchar> &argv )
#endif
{
    static bool already_called = false;

    if ( already_called ) {
        qWarning( "Qt internal error: qWinMain should be called only once" );
        return;
    }
    already_called = true;

    // Install default debug handler
    qInstallMsgHandler( qWinMsgHandler );

    // create command line (from Qt4)
    argv = qWinCmdLine<char>( lpCmdParam, strlen( lpCmdParam ), argc );

    // Get Windows parameters
    appInst = hinstance;
    appPrevInst = prevInstance;
    appCmdShow = show;
}

//#define STOP_EXECUTION_IN_QLASTERROR_BY_MESSAGE_WINDOW

void qlasterror( CHAR *msg, DWORD dwLastError, bool showWindow )
{

    DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_IGNORE_INSERTS |
                          FORMAT_MESSAGE_FROM_SYSTEM ;

    LPSTR lpMsgBuf;
    if ( !FormatMessageA( dwFormatFlags, NULL, dwLastError,
                          MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),      // Default language
                          ( LPSTR ) & lpMsgBuf, 0, NULL ) )
        return ;

    QString qs( lpMsgBuf );
    qs = qs.left( qs.length() - 1 ); // no new line (removing \n)
    qDebug( "%s  %s", msg, qs.latin1() );

#ifdef STOP_EXECUTION_IN_QLASTERROR_BY_MESSAGE_WINDOW

    static QString lastmsg;
    if ( lastmsg.compare( qs ) != 0 )
        MessageBoxA( NULL, ( LPCSTR ) lpMsgBuf, msg, MB_OK | MB_ICONINFORMATION );
    lastmsg = qs;
#endif

    LocalFree( lpMsgBuf );
}

void QApplication::setWheelScrollLines( int n )
{
#ifdef SPI_SETWHEELSCROLLLINES
    if ( n < 0 )
        n = 0;
    QT_WA( {
               SystemParametersInfo( SPI_SETWHEELSCROLLLINES, ( uint ) n, 0, 0 );
           } , {
               SystemParametersInfoA( SPI_SETWHEELSCROLLLINES, ( uint ) n, 0, 0 );
           } );
#else
    wheel_scroll_lines = n;
#endif
}

int QApplication::wheelScrollLines()
{
#ifdef SPI_GETWHEELSCROLLLINES
    uint i = 3;
    QT_WA( {
               SystemParametersInfo( SPI_GETWHEELSCROLLLINES, sizeof( uint ), &i, 0 );
           } , {
               SystemParametersInfoA( SPI_GETWHEELSCROLLLINES, sizeof( uint ), &i, 0 );
           } );
    if ( i > INT_MAX )
        i = INT_MAX;
    return i;
#else
    return wheel_scroll_lines;
#endif
}

static bool effect_override = false;

void QApplication::setEffectEnabled( Qt::UIEffect effect, bool enable )
{
    effect_override = true;
    switch ( effect ) {
    case Qt::UI_AnimateMenu:
        animate_menu = enable;
        break;
    case Qt::UI_FadeMenu:
        fade_menu = enable;
        break;
    case Qt::UI_AnimateCombo:
        animate_combo = enable;
        break;
    case Qt::UI_AnimateTooltip:
        animate_tooltip = enable;
        break;
    case Qt::UI_FadeTooltip:
        fade_tooltip = enable;
        break;
    case Qt::UI_AnimateToolBox:
        animate_toolbox = enable;
        break;
    default:
        animate_ui = enable;
        break;
    }
}

bool QApplication::isEffectEnabled( Qt::UIEffect effect )
{
    /*
        if (QColormap::instance().depth() < 16)
            return false;
    */
    if ( !effect_override && desktopSettingsAware()
            && !( qt_winver == Qt::WV_95 || qt_winver == Qt::WV_NT ) ) {
        // we know that they can be used when we are here
        BOOL enabled = false;
        UINT api;
        switch ( effect ) {
        case Qt::UI_AnimateMenu:
            api = SPI_GETMENUANIMATION;
            break;
        case Qt::UI_FadeMenu:
            if ( qt_winver & Qt::WV_DOS_based )
                return false;
            api = SPI_GETMENUFADE;
            break;
        case Qt::UI_AnimateCombo:
            api = SPI_GETCOMBOBOXANIMATION;
            break;
        case Qt::UI_AnimateTooltip:
            if ( qt_winver & Qt::WV_DOS_based )
                api = SPI_GETMENUANIMATION;
            else
                api = SPI_GETTOOLTIPANIMATION;
            break;
        case Qt::UI_FadeTooltip:
            if ( qt_winver & Qt::WV_DOS_based )
                return false;
            api = SPI_GETTOOLTIPFADE;
            break;
        default:
            api = SPI_GETUIEFFECTS;
            break;
        }
        QT_WA( {
                   SystemParametersInfo( api, 0, &enabled, 0 );
               } , {
                   SystemParametersInfoA( api, 0, &enabled, 0 );
               } );
        return enabled;
    }

    switch ( effect ) {
    case Qt::UI_AnimateMenu:
        return animate_menu;
    case Qt::UI_FadeMenu:
        return fade_menu;
    case Qt::UI_AnimateCombo:
        return animate_combo;
    case Qt::UI_AnimateTooltip:
        return animate_tooltip;
    case Qt::UI_FadeTooltip:
        return fade_tooltip;
    case Qt::UI_AnimateToolBox:
        return animate_toolbox;
    default:
        return animate_ui;
    }
}

/**************************************************************
 * QSessionManager                                            *
 **************************************************************/
#ifndef QT_NO_SESSIONMANAGER
bool QSessionManager::allowsInteraction()
{
    sm_blockUserInput = false;
    return true;
}

bool QSessionManager::allowsErrorInteraction()
{
    sm_blockUserInput = false;
    return true;
}

void QSessionManager::release()
{
    if ( sm_smActive )
        sm_blockUserInput = true;
}

void QSessionManager::cancel()
{
    sm_cancel = true;
}
#endif // QT_NO_SESSIONMANAGER
