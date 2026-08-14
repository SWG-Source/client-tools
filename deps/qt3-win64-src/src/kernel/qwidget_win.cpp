/****************************************************************************
** $Id: qwidget_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QWidget and QWindow classes for X11
**
** Created : 20030119
**
** Copyright (C) 2003 Holger Schroeder
** Copyright (C) 2003, 2004 Christopher January
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2003 Ivan de Jesus Deras Tabora
** Copyright (C) 2004 Ralf Habacker
** Copyright (C) 2004 Peter Kuemmel
** Copyright (C) 2004, 2005, 2006 Christian Ehrlicher
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

#include "qapplication.h"
#include "qbitmap.h"
#include "qcursor.h"
#include "qlibrary.h"           /* QLibrary::resolve() */
#include "qobjectlist.h"
#include "qpainter.h"
#include "qpaintdevicemetrics.h"
#include "qpixmap.h"
#include "qwidget.h"
#include "qt_windows.h"

//#define DEBUG_QWIDGET

// defined in qpainter_win.cpp
void qt_erase_region( QWidget *w, QPixmap *pm, QPoint ofs, const QRegion &reg );       // QWidget::erase()
// defined in qregion_win.cpp
HRGN QBitmapToHRGN( const QBitmap& bm );                // QWidget::setMask( QBitmap )
// defined in qdnd_win.cpp
void            qt_olednd_unregister(QWidget* widget, QOleDropTarget *dst);
QOleDropTarget* qt_olednd_register(QWidget* widget);
// defined in qapplication_x11.cpp
const QString qt_reg_winclass( Qt::WFlags flags );
void qt_leave_modal( QWidget * );                       // QWidget::destroy()

extern bool qt_nograb();

static QWidget *mouseGrb    = 0;
static QCursor *mouseGrbCur = 0;
static QWidget *keyboardGrb = 0;
static HHOOK   journalRec  = 0;

extern "C" LRESULT CALLBACK qt_window_procedure ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

//Used in  QWidget::internalSetGeometry, QETWidget::translateConfigEvent
void getWndBorderInfo( HWND hWnd, int& captionWidth, int& xBorderWidth, int& yBorderWidth )
{
    RECT wndRect, clientRc;

    /* The GetWindowRect function retrieves the dimensions of the bounding rectangle of the
       specified window. The dimensions are given in screen coordinates that are relative to
       the upper-left corner of the screen. */
    GetWindowRect( hWnd, &wndRect );
    /* The GetClientRect function retrieves the coordinates of a window's client area.
       The client coordinates specify the upper-left and lower-right corners of the client area.
       Because client coordinates are relative to the upper-left corner of a window's client
       area, the coordinates of the upper-left corner are (0,0). */
    GetClientRect( hWnd, &clientRc );
    MapWindowPoints( hWnd, HWND_DESKTOP, ( POINT* ) & clientRc, 2 );
    captionWidth = ( int ) ( clientRc.top - wndRect.top );
    xBorderWidth = ( int ) ( clientRc.left - wndRect.left );
    yBorderWidth = ( int ) ( wndRect.bottom - clientRc.bottom );
}

/*!
  Creates a new widget window if \a window is null, otherwise sets the
  widget's window to \a window.

  Initializes the window (sets the geometry etc.) if \a initializeWindow
  is TRUE.  If \a initializeWindow is FALSE, no initialization is
  performed.  This parameter makes only sense if \a window is a valid
  window.

  Destroys the old window if \a destroyOldWindow is TRUE.  If \a
  destroyOldWindow is FALSE, you are responsible for destroying
  the window yourself (using platform native code).

  The QWidget constructor calls create(0,TRUE,TRUE) to create a window for
  this widget.
*/

void QWidget::create( WId window, bool initializeWindow, bool destroyOldWindow )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::create(0x%08X, %d, %d), parent()=0x%08X, pWinId: 0x%08X, popup: %d",
            window, initializeWindow, destroyOldWindow, parent(),
            parent() ? parentWidget()->winId() : NULL, testWFlags( WType_Popup ) );
#endif

   if ( testWState( WState_Created ) && window == 0 )
        return ;

    // set created flag
    setWState( WState_Created );

    bool popup = testWFlags( WType_Popup );
    bool dialog = testWFlags( WType_Dialog );
    bool desktop = testWFlags( WType_Desktop );

    // top-level widget
    if ( !parentWidget() || parentWidget() ->isDesktop() )
        setWFlags( WType_TopLevel );

    // these are top-level, too
    if ( dialog || popup || desktop )
        setWFlags( WType_TopLevel );

    // a popup stays on top
    /*Ivan: The problem with this is, that if we set parent to NULL,
    every time that a Popup Menu appear it is showed in the taskbar because it is a tlw and has no parent.
    Then to avoid it a popup need the style WS_EX_TOOLWINDOW. See below ... */
    if ( popup )
        setWFlags( WStyle_StaysOnTop );

    bool topLevel = testWFlags( WType_TopLevel );
    HWND parentw, destroyw = 0;
    WId id;

    // always initialize
    if ( !window )
        initializeWindow = TRUE;

    static int sw = -1, sh = -1;  // screen size

    if ( sw < 0 ) {    // get the screen size
        sw = GetSystemMetrics( SM_CXSCREEN );
        sh = GetSystemMetrics( SM_CYSCREEN );
    }

    if ( desktop ) {    // desktop widget
        popup = FALSE;   // force these flags off
#ifndef Q_OS_TEMP
        if ( QApplication::winVersion() != Qt::WV_NT && QApplication::winVersion() != Qt::WV_95)
            crect.setRect(GetSystemMetrics(76 /* SM_XVIRTUALSCREEN  */), GetSystemMetrics(77 /* SM_YVIRTUALSCREEN  */),
                          GetSystemMetrics(78 /* SM_CXVIRTUALSCREEN */), GetSystemMetrics(79 /* SM_CYVIRTUALSCREEN */));
        else
#endif
            crect.setRect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    } else if ( topLevel ) {   // calc pos/size from screen
        crect.setRect( sw / 4, 3 * sh / 10, sw / 2, 4 * sh / 10 );
    } else {     // child widget
        crect.setRect( 0, 0, 100, 30 );
    }
    
    parentw = parentWidget() ? parentWidget()->winId () : NULL;

    DWORD dwStyle = WS_CLIPCHILDREN;
    DWORD dwExtStyle = WS_EX_NOPARENTNOTIFY;

    if ( topLevel && parentWidget() ) {
        // if our parent has WStyle_StaysOnTop, so must we
        QWidget * ptl = parentWidget() ->topLevelWidget();
        if ( ptl && ptl->testWFlags( WStyle_StaysOnTop ) )
            setWFlags( WStyle_StaysOnTop );
    }
    if ( !testWFlags( WStyle_Customize ) && !( desktop || popup ) ) {
        if ( testWFlags( WStyle_Dialog ) )
            setWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu | WStyle_ContextHelp );
        else
            setWFlags( WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_MinMax | WStyle_SysMenu );
    }

    if ( topLevel ) {
#ifdef DEBUG_QWIDGET
        qDebug ( "qwidget_win.cpp: QWidget::create %s top level window!", className() );
#endif
        // these flags only make sense for top-level windows
        if ( popup ) {
            dwStyle |= WS_POPUP;
            // otherwise the popup is shown in the toolbar...
            dwExtStyle |= WS_EX_TOOLWINDOW;
        }
        if ( testWFlags( WStyle_Title ) )
            dwStyle |= WS_CAPTION;
        if ( testWFlags( WStyle_NormalBorder & WStyle_Dialog ) ) {
            dwStyle |= WS_THICKFRAME;
            dwExtStyle |= WS_EX_WINDOWEDGE;
        } else if ( testWFlags( WStyle_NormalBorder ) )
            dwStyle |= WS_THICKFRAME;
        else if ( testWFlags( WStyle_DialogBorder ) )
            dwExtStyle |= WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE;
        else if ( testWFlags( WStyle_NoBorder ) )
            dwStyle |= WS_POPUP;
        if ( testWFlags( WStyle_SysMenu ) )
            dwStyle |= WS_SYSMENU;
        if ( testWFlags( WStyle_ContextHelp ) )
            dwExtStyle |= WS_EX_CONTEXTHELP;
        if ( testWFlags( WStyle_Minimize ) )
            dwStyle |= WS_MINIMIZEBOX;
        if ( testWFlags ( WStyle_Maximize ) )
            dwStyle |= WS_MAXIMIZEBOX;
        if ( testWFlags ( WStyle_Tool ) )
            dwExtStyle |= WS_EX_TOOLWINDOW;
        if ( testWFlags ( WStyle_StaysOnTop ) )
            dwExtStyle |= WS_EX_TOPMOST;
        if ( testWFlags ( WStyle_Dialog ) )
            dwStyle |= WS_POPUP;
        if ( !( dwStyle & WS_POPUP ) )
            dwStyle |= WS_OVERLAPPED;
    } else {
        dwStyle |= WS_CHILD | WS_CLIPSIBLINGS;
#ifdef DEBUG_QWIDGET

        qDebug ( "qwidget_win.cpp: QWidget::create child window, %s!", className() );
#endif

    }
    QString windowClassName = qt_reg_winclass( getWFlags() );
    if ( window ) {             // override .. the old window
        if ( destroyOldWindow )
            destroyw = winId();
        id = window;
        SetParent( id, parentw );
        LONG res = SetWindowLongA(window, GWL_STYLE, dwStyle);
        if (!res)
            qWarning("QWidget::create: Failed to set window style");
#ifdef _WIN64
        res = SetWindowLongPtrA( window, GWLP_WNDPROC, (LONG_PTR)qt_window_procedure );
#else
        res = SetWindowLongA( window, GWL_WNDPROC, (LONG)qt_window_procedure );
#endif
        if (!res)
            qWarning("QWidget::create: Failed to set window procedure");
    } else if ( desktop ) {         // desktop widget
        id = ( WId ) GetDesktopWindow (); // this is the desktop window handle
    } else if (topLevel) {                       // create top-level widget
        if (popup)
            parentw = 0;

#ifdef Q_OS_TEMP

        const TCHAR *cname = windowClassName.utf16();

        id = CreateWindowEx(exsty, cname, ttitle, style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parentw, 0, appinst, 0);
#else

        QT_WA({
            id = CreateWindowEx(dwExtStyle,
                                ( LPCWSTR )windowClassName.ucs2(),
                                ( LPCWSTR )caption().ucs2(),
                                dwStyle,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                parentw,
                                NULL,
                                qWinAppInst(),
                                NULL);
        } , {
            id = CreateWindowExA(dwExtStyle,
                                 ( LPCSTR )windowClassName.latin1(),
                                 ( LPCSTR )caption().latin1(),
                                 dwStyle,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 parentw,
                                 NULL,
                                 qWinAppInst(),
                                 NULL);
        });

#endif

        if (!id)
            qWarning("QWidget::create: Failed to create window");
        if ( testWFlags( Qt::WStyle_StaysOnTop) )
            SetWindowPos(id, HWND_TOPMOST, 0, 0, 100, 100, SWP_NOACTIVATE);
    } else {                                        // create child widget
        QT_WA({
            id = CreateWindowEx(dwExtStyle,
                                ( LPCWSTR )windowClassName.ucs2(),
                                ( LPCWSTR )caption().ucs2(),
                                dwStyle,
                                0, 0,
                                100, 30,
                                parentw,
                                NULL,
                                qWinAppInst(),
                                NULL);
        } , {
            id = CreateWindowExA(dwExtStyle,
                                ( LPCSTR )windowClassName.latin1(),
                                ( LPCSTR )caption().latin1(),
                                dwStyle,
                                0, 0,
                                100, 30,
                                parentw,
                                NULL,
                                qWinAppInst(),
                                NULL);
        });
        if (!id)
            qWarning("QWidget::create: Failed to create window");
        SetWindowPos(id, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    setWinId ( id );
    hdc = 0;
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::create() hdc: 0x%08p, WinID: 0x%08X, pWinID: 0x%08X, flags: 0x%08X, flagsex: 0x%08X", hdc, id, parentw, dwStyle, dwExtStyle );
#endif

    if (desktop) {
        setWState( Qt::WState_Visible );
    } else {
        RECT  fr, cr;
        GetWindowRect(id, &fr);                // update rects
        GetClientRect(id, &cr);
        if (cr.top == cr.bottom && cr.left == cr.right) {
            if (initializeWindow) {
                int x, y, w, h;
                if (topLevel) {
                    x = sw/4;
                    y = 3*sh/10;
                    w = sw/2;
                    h = 4*sh/10;
                } else {
                    x = y = 0;
                    w = 100;
                    h = 30;
                }
                MoveWindow(winId(), x, y, w, h, true);
            }
            GetWindowRect(id, &fr);                // update rects
            GetClientRect(id, &cr);
        }
        if (topLevel){
            // one cannot trust cr.left and cr.top, use a correction POINT instead
            POINT pt;
            pt.x = 0;
            pt.y = 0;
            ClientToScreen(id, &pt);
            crect = QRect(QPoint(pt.x, pt.y),
                          QPoint(pt.x+cr.right, pt.y+cr.bottom));

            QTLWExtra *top = topData();
            top->ftop = crect.top() - fr.top;
            top->fleft = crect.left() - fr.left;
            top->fbottom = fr.bottom - crect.bottom();
            top->fright = fr.right - crect.right();
            fstrut_dirty = false;

            createTLExtra();
        } else {
            crect.setCoords(cr.left, cr.top, cr.right, cr.bottom);
            // in case extra data already exists (eg. reparent()).  Set it.
        }
    }

    if (window) {                                // got window from outside
        if (IsWindowVisible(window))
            setWState(Qt::WState_Visible);
        else
            setWState(Qt::WState_Visible);
    }

    if (destroyw) {
        DestroyWindow(destroyw);
    }

//    QWinInputContext::enable(this, testWFlags(Qt::InputMethodEnabled) & isEnabled());
//    if (q != qt_tablet_widget)
//        qt_tablet_init();
}

/*!
  Frees up window system resources.
  Destroys the widget window if \a destroyWindow is TRUE.

  destroy() calls itself recursively for all the child widgets,
  passing \a destroySubWindows for the \a destroyWindow parameter.
  To have more control over destruction of subwidgets,
  destroy subwidgets selectively first.

  This function is usually called from the QWidget destructor.
*/

void QWidget::destroy( bool destroyWindow, bool destroySubWindows )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::destroy( destroyWindow = %d, destroySubWindows = %d )",
            destroyWindow, destroySubWindows );
#endif

    deactivateWidgetCleanup();
    if ( testWState( WState_Created ) ) {
        clearWState( WState_Created );
        if ( children() ) {
            QObjectListIt it( *children() );
            register QObject *obj;
            while ( ( obj = it.current() ) ) { // destroy all widget children
                ++it;
                if ( obj->isWidgetType() )
                    ( ( QWidget* ) obj ) ->destroy( destroySubWindows,
                                                    destroySubWindows );
            }
        }
        if ( mouseGrb == this )
            releaseMouse();
        if ( keyboardGrb == this )
            releaseKeyboard();
        if ( acceptDrops() )
            setAcceptDrops( false );
        if ( testWFlags( WType_Modal ) )                 // just be sure we leave modal
            qt_leave_modal( this );
        else if ( testWFlags( WType_Popup ) )
            qApp->closePopup( this );

        if ( !testWFlags( WType_Desktop ) ) {
            if ( destroyWindow ) {
#ifdef DEBUG_QWIDGET
                qDebug( "QWidget::destroy() DestroyWindow ( hdc: 0x%08p, id: 0x%08X )", hdc, winId() );
#endif

                DestroyWindow( winId() );
            }
        }
        setWinId( 0 );
    }
}

void QWidget::reparentSys( QWidget *parent, WFlags f, const QPoint &p, bool showIt )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::reparentSys(NewParent: 0x%08p, OldParentObj: 0x%08p, showIt: %d)", parent, parentObj, showIt );
    qDebug( "this: 0x%08p, hdc: 0x%08p, id: 0x%08X, NewParent: hdc: 0x%08p, id: 0x%08X", this, hdc, winId(), parent ? parent->hdc : (HDC)-1, parent ? parent->winId() : (HWND)-1);
#endif

    QCursor oldcurs;
    bool setcurs = testWState( WState_OwnCursor );
    if ( setcurs ) {
        oldcurs = cursor();
        unsetCursor();
    }

    // dnd unregister (we will register again below)
    bool accept_drops = acceptDrops();
    setAcceptDrops( FALSE );

    // clear mouse tracking, re-enabled below
    bool mouse_tracking = hasMouseTracking();
    clearWState( WState_MouseTracking );

    QWidget* oldtlw = topLevelWidget();
    //QWidget *oldparent = parentWidget();
    WId old_winid = winId();
    if ( testWFlags( WType_Desktop ) )
        old_winid = 0;
    setWinId( 0 );

    //    if ( isTopLevel() || !parent ) // we are toplevel, or reparenting to toplevel
    //        topData()->parentWinId = 0;

    if ( parent != parentObj ) {
        if ( parentObj )          // remove from parent
            parentObj->removeChild( this );
        if ( parent )             // insert into new parent
            parent->insertChild( this );
    }
    bool enable = isEnabled();  // remember status
    FocusPolicy fp = focusPolicy();
    QSize s = size();
    QString capt = caption();
    widget_flags = f;
    clearWState( WState_Created | WState_Visible | WState_ForceHide );
    create();
    if ( isTopLevel() || ( !parent || parent->isVisible() ) )
        setWState( WState_ForceHide ); // new widgets do not show up in already visible parents

    const QObjectList *chlist = children();
    if ( chlist ) {    // reparent children
        QObjectListIt it( *chlist );
        QObject *obj;
        while ( ( obj = it.current() ) ) {
            if ( obj->isWidgetType() ) {
                QWidget * w = ( QWidget * ) obj;
                if ( !w->isTopLevel() ) {
                    SetParent( w->winId(), winId() );
                } else if ( w->isPopup()
                            || w->testWFlags( WStyle_DialogBorder )
                            || w->testWFlags( WType_Dialog )
                            || w->testWFlags( WStyle_Tool ) ) {
                    QApplication::postEvent( w, new QEvent( QEvent::ShowWindowRequest ) );
               }
            }
            ++it;
        }
    }

    if ( isTopLevel() ) {
        // preserve maximized/fullscreen flags and the normal geometry
        uint save_state = widget_state & ( WState_Maximized | WState_FullScreen );
        const QRect r = topData() ->normalGeometry;
        setGeometry( p.x(), p.y(), s.width(), s.height() );
        widget_state |= save_state;
        topData() ->normalGeometry = r;
    } else {
        setGeometry( p.x(), p.y(), s.width(), s.height() );
    }

    setEnabled( enable );
    setFocusPolicy( fp );
    if ( !capt.isNull() ) {
        extra->topextra->caption = QString::null;
        setCaption( capt );
    }
    if ( showIt )
        show();
    if ( old_winid ) {
        DestroyWindow( old_winid );
    }
    if ( setcurs )
        setCursor( oldcurs );

    reparentFocusWidgets( oldtlw );

    if ( accept_drops )
        setAcceptDrops( TRUE );

    // re-enable mouse tracking
    if ( mouse_tracking )
        setMouseTracking( mouse_tracking );
}


/*!
  Translates the widget coordinate \e pos to global screen coordinates.
  For example, \code mapToGlobal(QPoint(0,0))\endcode would give the
  global coordinates of the top-left pixel of the widget.
  \sa mapFromGlobal() mapTo() mapToParent()
*/

QPoint QWidget::mapToGlobal( const QPoint &pos ) const
{
    QWidget *window = topLevelWidget();
    if (!isVisible() || isMinimized())
        return mapTo(window, pos) + window->pos() +
        (window->geometry().topLeft() - window->frameGeometry().topLeft());

    POINT p;
    p.x = pos.x();
    p.y = pos.y();

    ClientToScreen( winId(), &p );
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::mapToGlobal( %d,%d ) = (%d, %d)", pos.x(), pos.y(), p.x, p.y );
#endif

    return QPoint( p.x, p.y );
}

/*!
  Translates the global screen coordinate \e pos to widget coordinates.
  \sa mapToGlobal() mapFrom() mapFromParent()
*/

QPoint QWidget::mapFromGlobal( const QPoint &pos ) const
{
    QWidget *window = topLevelWidget();
    if (!isVisible() || isMinimized())
        return mapFrom( window, pos - window->pos());

    POINT p;
    p.x = pos.x();
    p.y = pos.y();

    ScreenToClient( winId(), &p );
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::mapFromGlobal( %d,%d ) = (%d, %d)", pos.x(), pos.y(), p.x, p.y );
#endif

    return QPoint( p.x, p.y );
}

/*!
  When a widget gets focus, it should call setMicroFocusHint for some
  appropriate position and size - \a x, \a y and \a w by \a h.  This
  has no \e visual effect, it just provides hints to any
  system-specific input handling tools.

  The \a text argument should be TRUE if this is a position for text
  input.

  In the Windows version of Qt, this method sets the system caret, which is
  used for user Accessibility focus handling.  If \a text is TRUE, it also
  sets the IME composition window in Far East Asian language input systems.

  In the X11 version of Qt, if \a text is TRUE, this method sets the
  XIM "spot" point for complex language input handling.

  \sa microFocusHint()
*/
void QWidget::setMicroFocusHint( int x, int y, int width, int height, bool /*text*/, QFont * )
{
#ifdef DEBUG_QWIDGET
    qDebug( "qwidget_win.cpp: setMicroFocusHint" );
#endif

    if ( QRect( x, y, width, height ) != microFocusHint() )
        extraData() ->micro_focus_hint.setRect( x, y, width, height );
}

void QWidget::setFontSys( QFont* fnt )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::setFontSys( 0x%08p )", fnt );
#endif
    // Do nothing, that's what the x11 implementation does...
}


void QWidget::setBackgroundColorDirect( const QColor &color )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::setBackgroundColorDirect( rgb: %x/%x/%x )", color.red(), color.green(), color.blue() );
#endif

    bg_col = color;
    if ( extra && extra->bg_pix ) {  // kill the background pixmap
        delete extra->bg_pix;
        extra->bg_pix = 0;
    }
}

static int allow_null_pixmaps = 0;
void QWidget::setBackgroundPixmapDirect( const QPixmap &pixmap )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::setBackgroundPixmapDirect(), %s", className() );
#endif

    QPixmap old;
    if ( extra && extra->bg_pix )
        old = *extra->bg_pix;
    if ( !allow_null_pixmaps && pixmap.isNull() ) {
        if ( extra && extra->bg_pix ) {
            delete extra->bg_pix;
            extra->bg_pix = 0;
        }
    } else {
        QPixmap pm = pixmap;
        if ( !pm.isNull() ) {
            if ( pm.depth() == 1 && QPixmap::defaultDepth() > 1 ) {
                pm = QPixmap( pixmap.size() );
                bitBlt( &pm, 0, 0, &pixmap, 0, 0, pm.width(), pm.height() );
            }
        }
        if ( extra && extra->bg_pix )
            delete extra->bg_pix;
        else
            createExtra();
        extra->bg_pix = new QPixmap( pm );
    }
}


/*!
  Sets the window-system background of the widget to nothing.

  Note that `nothing' is actually a pixmap that isNull(), thus you
  can check for an empty background by checking backgroundPixmap().

  \sa setBackgroundPixmap(), setBackgroundColor()

  This class should \e NOT be made virtual - it is an alternate usage
  of setBackgroundPixmap().
*/
void QWidget::setBackgroundEmpty()
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::setBackgroundEmpty()" );
#endif

    allow_null_pixmaps++;
    setBackgroundPixmap( QPixmap() );
    allow_null_pixmaps--;
}

/*!
  Sets the widget cursor shape to \e cursor.

  The mouse cursor will assume this shape when it's over this widget.
  See a list of predefined cursor objects with a range of useful
  shapes in the QCursor documentation.

  An editor widget would for example use an I-beam cursor:
  \code
    setCursor( ibeamCursor );
  \endcode

  \sa cursor(), unsetCursor(), QApplication::setOverrideCursor()
*/

void QWidget::setCursor( const QCursor &cursor )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::setCursor()" );
#endif

    if ( cursor.handle() != arrowCursor.handle()
            || ( extra && extra->curs ) ) {
        createExtra();
        delete extra->curs;
        extra->curs = new QCursor( cursor );
    }
    setWState( WState_OwnCursor );
}


/*!
  Unset the cursor for this widget. The widget will use the cursor of
  its parent from now on.

  This functions does nothing for top-level windows.

  \sa cursor(), setCursor(), QApplication::setOverrideCursor()
 */

void QWidget::unsetCursor()
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::unsetCursor()" );
#endif

    if ( !isTopLevel() ) {
        if ( extra ) {
            delete extra->curs;
            extra->curs = 0;
        }
        clearWState( WState_OwnCursor );
    }
}

/*!
  Sets the window caption (title) to \a caption.
  \sa caption(), setIcon(), setIconText()
*/

void QWidget::setCaption( const QString &caption )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::setCaption( %s )", caption.latin1() );
#endif

    if ( QWidget::caption() == caption )
        return ; // for less flicker
    topData() ->caption = caption;
    QT_WA(
        SetWindowTextW( winId(), ( LPCWSTR ) caption.ucs2() );,
        SetWindowTextA( winId(), ( LPCSTR ) caption.local8Bit() );
    )
    QCustomEvent e( QEvent::CaptionChange, 0 );
    QApplication::sendEvent( this, &e );
}

/*!
  Sets the window icon to \a pixmap.
  \sa icon(), setIconText(), setCaption(),
      \link appicon.html Setting the Application Icon\endlink
*/

void QWidget::setIcon( const QPixmap &pixmap )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::setIcon()" );
#endif

    if ( extra && extra->topextra ) {
        delete extra->topextra->icon;
        extra->topextra->icon = 0;
        if ( extra->topextra->winIcon )
            DestroyIcon( extra->topextra->winIcon );
        extra->topextra->winIcon = 0;
    } else {
        createTLExtra();
    }
    if ( !pixmap.isNull() ) {
        int w = pixmap.width();
        int h = pixmap.height();
        QPixmap *pm = new QPixmap( pixmap );
        extra->topextra->icon = pm;

        if ( !pm->mask() )
            pm->setMask( pm->createHeuristicMask() );

        /* Same procedure like in qcursor_win.cpp / qt_createPixmapCursor */
        /* Create Bitmaps  */
        HDC myHDC = hdc ? hdc : qt_display_dc();
        HDC hAndMask = CreateCompatibleDC( myHDC );
        HDC hXorMask = CreateCompatibleDC( myHDC );
        HBITMAP hbAndMask = CreateCompatibleBitmap( myHDC, w, h );
        HBITMAP hbXorMask = CreateCompatibleBitmap( myHDC, w, h );
        /* Select the bitmaps to DC  */
        HBITMAP hbOldAndMask = ( HBITMAP ) SelectObject( hAndMask, hbAndMask );
        HBITMAP hbOldXorMask = ( HBITMAP ) SelectObject( hXorMask, hbXorMask );

        /* some BitBlt'ing :) */
        BitBlt( hAndMask, 0, 0, w, h, NULL, 0, 0, WHITENESS );
        BitBlt( hAndMask, 0, 0, w, h, pm->mask() ->handle(), 0, 0, SRCAND );

        BitBlt( hXorMask, 0, 0, w, h, pm->mask() ->handle(), 0, 0, NOTSRCCOPY );
        if ( pm->isQBitmap() ) {
            QPixmap xorMask( w, h );
            xorMask.fill ( Qt::color1 );
            QPainter paint2( &xorMask );

            QPixmap tmp = *pm;
            tmp.setMask( *pm->mask() );
            paint2.drawPixmap ( 0, 0, tmp );
            paint2.end();
            BitBlt( hXorMask, 0, 0, w, h, xorMask.handle(), 0, 0, SRCAND );
        } else
            BitBlt( hXorMask, 0, 0, w, h, pm->handle(), 0, 0, SRCAND );

        /* Restore & free */
        SelectObject( hAndMask, hbOldAndMask );
        SelectObject( hXorMask, hbOldXorMask );
        DeleteDC( hAndMask );
        DeleteDC( hXorMask );

        ICONINFO ii;
        ii.fIcon = true;
        ii.xHotspot = 0;
        ii.yHotspot = 0;
        ii.hbmMask = hbAndMask;
        ii.hbmColor = hbXorMask;

        if ( extra->topextra->winIcon )
            DestroyIcon( extra->topextra->winIcon );
        extra->topextra->winIcon = CreateIconIndirect ( &ii );
        SendMessage( winId(), WM_SETICON, ICON_BIG, ( LPARAM ) extra->topextra->winIcon );

        QCustomEvent e( QEvent::IconChange, 0 );
        QApplication::sendEvent( this, &e );
        
        DeleteObject( hbAndMask );
        DeleteObject( hbXorMask );        
    }
}


/*!
  Sets the text of the window's icon to \e iconText.
  \sa iconText(), setIcon(), setCaption()
*/
void QWidget::setIconText( const QString & iconText )
{
#ifdef DEBUG_QWIDGET
    qDebug( "TODO? qwidget_win.cpp: setIconText" );
#endif

    createTLExtra();
    extra->topextra->iconText = iconText;
}

QCursor *qt_grab_cursor()
{
    return mouseGrbCur;
}

// The procedure does nothing, but is required for mousegrabbing to work
LRESULT CALLBACK qJournalRecordProc( int nCode, WPARAM wParam, LPARAM lParam )
{
#ifndef Q_OS_TEMP
    return CallNextHookEx( journalRec, nCode, wParam, lParam );
#else
    return 0;
#endif
}

/*!
  Grabs the mouse input.

  This widget will be the only one to receive mouse events until
  releaseMouse() is called.

  \warning Grabbing the mouse might lock the terminal.

  It is almost never necessary to grab the mouse when using Qt since
  Qt grabs and releases it sensibly.  In particular, Qt grabs the
  mouse when a button is pressed and keeps it until the last button is
  released.

  Beware that only widgets actually shown on the screen may grab the
  mouse input.

  \sa releaseMouse(), grabKeyboard(), releaseKeyboard()
*/
void QWidget::grabMouse()
{
    if ( !qt_nograb() ) {
        if ( mouseGrb )
            mouseGrb->releaseMouse();
#ifndef Q_OS_TEMP
        journalRec = SetWindowsHookExA( WH_JOURNALRECORD, ( HOOKPROC ) qJournalRecordProc, GetModuleHandleA( 0 ), 0 );
#endif
        SetCapture( winId() );
        mouseGrb = this;
    }
}

/*!
  Grabs the mouse input and changes the cursor shape.

  The cursor will assume shape \e cursor (for as long as the mouse focus is
  grabbed) and this widget will be the only one to receive mouse events
  until releaseMouse() is called().

  \warning Grabbing the mouse might lock the terminal.

  \sa releaseMouse(), grabKeyboard(), releaseKeyboard(), setCursor()
*/
void QWidget::grabMouse( const QCursor & cursor )
{
    if ( !qt_nograb() ) {
        if ( mouseGrb )
            mouseGrb->releaseMouse();
#ifndef Q_OS_TEMP
        journalRec = SetWindowsHookExA( WH_JOURNALRECORD, ( HOOKPROC ) qJournalRecordProc, GetModuleHandleA( 0 ), 0 );
#endif
        SetCapture( winId() );
        mouseGrbCur = new QCursor( cursor );
        SetCursor( mouseGrbCur->handle() );
        mouseGrb = this;
    }
}

/*!
  Releases the mouse grab.

  \sa grabMouse(), grabKeyboard(), releaseKeyboard()
*/
void QWidget::releaseMouse()
{
    if ( !qt_nograb() && mouseGrb == this ) {
        ReleaseCapture();
        if ( journalRec ) {
#ifndef Q_OS_TEMP
            UnhookWindowsHookEx( journalRec );
#endif
            journalRec = 0;
        }
        if ( mouseGrbCur ) {
            delete mouseGrbCur;
            mouseGrbCur = 0;
        }
        mouseGrb = 0;
    }
}

/*!
  Grabs all keyboard input.

  This widget will receive all keyboard events, independent of the active
  window.

  \warning Grabbing the keyboard might lock the terminal.

  \sa releaseKeyboard(), grabMouse(), releaseMouse()
*/
void QWidget::grabKeyboard()
{
    if ( !qt_nograb() ) {
        if ( keyboardGrb )
            keyboardGrb->releaseKeyboard();
        keyboardGrb = this;
    }
}

/*!
  Releases the keyboard grab.

  \sa grabKeyboard(), grabMouse(), releaseMouse()
*/
void QWidget::releaseKeyboard()
{
    if ( !qt_nograb() && keyboardGrb == this ) {
        keyboardGrb = 0;
    }
}

/*!
  Returns a pointer to the widget that is currently grabbing the
  mouse input.

  If no widget in this application is currently grabbing the mouse, 0 is
  returned.

  \sa grabMouse(), keyboardGrabber()
*/
QWidget *QWidget::mouseGrabber()
{
    return mouseGrb;
}

/*!
  Returns a pointer to the widget that is currently grabbing the
  keyboard input.

  If no widget in this application is currently grabbing the keyboard, 0
  is returned.

  \sa grabMouse(), mouseGrabber()
*/
QWidget *QWidget::keyboardGrabber()
{
    return keyboardGrb;
}

/*!
  Sets the top-level widget containing this widget to be the active
  window.

  An active window is a visible top-level window that has the keyboard input
  focus.

  This function performs the same operation as clicking the mouse on
  the title bar of a top-level window. On X11, the result depends on
  the Window Manager. If you want to ensure that the window is stacked
  on top as well, call raise() in addition. Note that the window has be
  to visible, otherwise setActiveWindow() has no effect.

  On Windows, if you are calling this when the application is not
  currently the active one then it will not make it the active window.  It
  will flash the task bar entry blue to indicate that the window has done
  something.  This is due to Microsoft not allowing an application to
  interrupt what the user is currently doing in another application.

  \sa isActiveWindow(), topLevelWidget(), show()
*/
void QWidget::setActiveWindow()
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::setActiveWindow(), %s", className() );
#endif
    QWidget *tlw = topLevelWidget();
    if ( tlw->isVisible() ) {
        SetForegroundWindow( tlw->winId() );
    }
}


/*!
  Updates the widget unless updates are disabled or the widget is hidden.

  Updating the widget will erase the widget contents and generate an
  appropriate paint event for the invalidated region. The paint event
  is processed after the program has returned to the main event loop.
  Calling update() many times in a row will generate a single paint
  event.

  If the widgets sets the WRepaintNoErase flag, update() will not erase
  its contents.

  \sa repaint(), paintEvent(), setUpdatesEnabled(), erase(), setWFlags()
*/

void QWidget::update()
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::update () %s, %s", className(), name() );
#endif
    if ( isVisible() && isUpdatesEnabled() ) {
        InvalidateRect( winId(), 0, false );
    }
}

/*!
  Updates a rectangle (\e x, \e y, \e w, \e h) inside the widget
  unless updates are disabled or the widget is hidden.

  Updating the widget erases the widget area \e (x,y,w,h) and generate
  an appropriate paint event for the invalidated region. The paint
  event is processed after the program has returned to the main event
  loop.  Calling update() many times in a row will generate a single
  paint event.

  If \e w is negative, it is replaced with <code>width() - x</code>.
  If \e h is negative, it is replaced width <code>height() - y</code>.


  If the widgets sets the WRepaintNoErase flag, update() will not erase
  its contents.

  \sa repaint(), paintEvent(), setUpdatesEnabled(), erase()
*/

void QWidget::update( int x, int y, int w, int h )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::update (%d, %d, %d, %d ) %s, %s", x, y, w, h, className(), name() );
#endif

    if ( !isVisible() || !isUpdatesEnabled() )
        return;

    if ( w && h ) {
        RECT r;
        r.left = x;
        r.top  = y;
        if ( w < 0 )
            r.right = crect.width();
        else
            r.right = x + w;
        if ( h < 0 )
            r.bottom = crect.height();
        else
            r.bottom = y + h;
        InvalidateRect( winId(), &r, false );
    }
}

/*!
  \overload void QWidget::update( const QRect &r )
*/

/*!
  \overload void QWidget::repaint( bool erase )

  This version repaints the entire widget.
*/

/*!
  \overload void QWidget::repaint()

  This version erases and repaints the entire widget.
*/

/*!
  Repaints the widget directly by calling paintEvent() directly,
  unless updates are disabled or the widget is hidden.

  Erases the widget area  \e (x,y,w,h) if \e erase is TRUE.

  If \e w is negative, it is replaced with <code>width() - x</code>.
  If \e h is negative, it is replaced width <code>height() - y</code>.

  Use repaint if your widget needs to be repainted immediately, for
  example when doing some animation. In all other cases, update() is
  to be preferred. Calling update() many times in a row will generate
  a single paint event.

  \warning If you call repaint() in a function which may itself be called
  from paintEvent(), you may see infinite recursion. The update() function
  never generates recursion.

  \sa update(), paintEvent(), setUpdatesEnabled(), erase()
*/

void QWidget::repaint( int x, int y, int w, int h, bool erase )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::repaint( %d, %d, %d, %d ) %s, %s", x, y, w, h, className(), name() );
#endif

    if ( !isVisible() || !isUpdatesEnabled() )
        return;

    if ( x > crect.width() || y > crect.height() )
        return ;
    if ( w < 0 )
        w = crect.width() - x;
    if ( h < 0 )
        h = crect.height() - y;
    QRegion r( x, y, w, h );
    repaint( r, erase );
}

/*!
  Repaints the widget directly by calling paintEvent() directly,
  unless updates are disabled or the widget is hidden.

  Erases the widget region  \a reg if \a erase is TRUE.

  Use repaint if your widget needs to be repainted immediately, for
  example when doing some animation. In all other cases, update() is
  to be preferred. Calling update() many times in a row will generate
  a single paint event.

  \warning If you call repaint() in a function which may itself be called
  from paintEvent(), you may see infinite recursion. The update() function
  never generates recursion.

  \sa update(), paintEvent(), setUpdatesEnabled(), erase()
*/

void QWidget::repaint( const QRegion& rgn, bool erase )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::repaint( QRegion& reg, bool erase=%d)", erase );
    qDebug( "  this is: %s %s", className(), name() );
    QRect rect = rgn.boundingRect();
    qDebug( "rect: %d,%d - %d,%d", rect.left(), rect.top(), rect.right(), rect.bottom() );
#endif

    if ( !isVisible() || !isUpdatesEnabled() || rgn.isEmpty() )
        return;

    bool bTempDC = false;
    if( !testWState(WState_InPaintEvent)) {
        hdc = GetDC( isDesktop() ? 0 : winId() );
        bTempDC = true;
    }
    QPaintEvent e( rgn );
    if ( erase )
        this->erase( rgn );
    QApplication::sendSpontaneousEvent( this, &e );
    ValidateRgn( winId(), rgn.handle() );
    if( bTempDC ) {
        ReleaseDC( isDesktop() ? 0 : winId(), hdc );
        hdc = 0;
    }
}

/*!
  \overload void QWidget::repaint( const QRect &r, bool erase )
*/


/*!
  \internal
  Platform-specific part of QWidget::show().
*/

void QWidget::showWindow()
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::showWindow() %s", className() );
#endif

    if ( isDesktop() )                //desktop is always visible
        return;
    fstrut_dirty = TRUE;

    int sm = SW_SHOWNORMAL;
    if ( isTopLevel() ) {
        if ( isMinimized() )
            sm = SW_SHOWMINIMIZED;
        else if ( isMaximized() )
            sm = SW_SHOWMAXIMIZED;
    }
    if ( testWFlags( WType_Popup | WStyle_Tool ) )
        sm = SW_SHOWNOACTIVATE;

    ShowWindow( winId(), sm );
    if ( IsIconic( winId() ) )
        setWindowState( windowState() | Qt::WindowMinimized );
    if ( IsZoomed( winId() ) )
        setWindowState( windowState() | Qt::WindowMaximized );

    UpdateWindow( winId() );
}


/*!
  \internal
  Platform-specific part of QWidget::hide().
*/

void QWidget::hideWindow()
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::hideWindow() %s, hdc: %p, winid: 0x%08X", className(), hdc, winId() );
#endif

    clearWState( WState_Exposed );
    deactivateWidgetCleanup();
    if ( isTopLevel() ) {
        QTLWExtra *top = topData();

        // zero the frame strut and mark it dirty
        top->fleft = top->fright = top->ftop = top->fbottom = 0;
        fstrut_dirty = TRUE;

    }
    ShowWindow( winId(), SW_HIDE );
}

/*!
  Restores the widget after it has been maximized or minimized.

  Calling this function has no effect for other than \link isTopLevel()
  top-level widgets\endlink.

  \sa showMinimized(), showMaximized(), show(), hide(), isVisible()
*/

/*!
  Raises this widget to the top of the parent widget's stack.

  If there are any siblings of this widget that overlap it on the screen,
  this widget will be visually in front of its siblings afterwards.

  \sa lower(), stackUnder()
*/

void QWidget::raise()
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::raise() %s", className() );
#endif

    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef( this ) >= 0 )
        p->childObjects->append( p->childObjects->take() );

    // if ( testWFlags( WStyle_Splash ) ) isn't the same like
    // if ( testWFlags( WStyle_Splash ) == WStyle_Splash ) since WStyle_Splash is only or'ed together in win32
    HWND hWndInsertAfter = ( testWFlags( WStyle_Splash ) == WStyle_Splash ) ? HWND_TOPMOST : HWND_TOP;

#ifdef DEBUG_QWIDGET
    qDebug( "SetWindowPos( %08x, %08x )", winId(), hWndInsertAfter );
#endif
    SetWindowPos( winId(), hWndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
}

/*!
  Lowers the widget to the bottom of the parent widget's stack.

  If there are siblings of this widget that overlap it on the screen, this
  widget will be obscured by its siblings afterwards.

  \sa raise(), stackUnder()
*/

void QWidget::lower()
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::lower() %s", className() );
#endif

    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef( this ) >= 0 )
        p->childObjects->insert( 0, p->childObjects->take() );
    SetWindowPos( winId(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
}


/*!
  Places the widget under \a w in the parent widget's stack.

  To make this work, the widget itself and \a w have to be siblings.

  \sa raise(), lower()
*/
void QWidget::stackUnder( QWidget* w )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::stackUnder( QWidget* %p ) %s", w, className() );
#endif

    QWidget *p = parentWidget();
    if ( !w || isTopLevel() || p != w->parentWidget() || this == w )
        return ;
    if ( p && p->childObjects && p->childObjects->findRef( w ) >= 0 && p->childObjects->findRef( this ) >= 0 ) {
        p->childObjects->take();
        p->childObjects->insert( p->childObjects->findRef( w ), this );
    }
    SetWindowPos( winId(), w->winId(), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
}

/* coordinates are excluding the window frame ! */
void qWinRequestConfig(WId, int, int, int, int, int);
void QWidget::internalSetGeometry( int x, int y, int w, int h, bool isMove )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::internalSetGeometry( x=%d, y=%d, w=%d, h=%d, isMove=%d )", x, y, w, h, isMove );
#endif

	if ( isTopLevel() && isMove )
        createTLExtra();

	clearWState( WState_Maximized );
    if ( extra ) {    // any size restrictions?
        w = QMIN( w, extra->maxw );
        h = QMIN( h, extra->maxh );
        w = QMAX( w, extra->minw );
        h = QMAX( h, extra->minh );

        // Deal with size increment
        if ( extra->topextra ) {
            if ( extra->topextra->incw ) {
                w = w / extra->topextra->incw;
                w *= extra->topextra->incw;
            }
            if ( extra->topextra->inch ) {
                h = h / extra->topextra->inch;
                h *= extra->topextra->inch;
            }
        }
    }
    if ( w < 1 )          // invalid size
        w = 1;
    if ( h < 1 )
        h = 1;
    QPoint oldPos( pos() );
    QSize oldSize( size() );

#ifdef DEBUG_QWIDGET
    qDebug( "internalSetGeometry crect value: l: %d, t: %d, r: %d, b: %d", crect.left(), crect.top(), crect.right(), crect.bottom() );
#endif

    if ( !isTopLevel() )
        isMove = ( crect.topLeft() != QPoint( x, y ) );
    bool isResize = ( w != oldSize.width() || h != oldSize.height() );

	if ( !isMove && !isResize )
        return;

	if ( isResize && !testWFlags( WStaticContents ) )
        ValidateRgn( winId(), 0 );

    if ( testWState( Qt::WState_ConfigPending ) ) {        // processing config event
        qWinRequestConfig( winId(), isMove ? 2 : 1, x, y, w, h );
    } else {
    	setWState( WState_ConfigPending );
    	if ( testWFlags( WType_Desktop ) ) {
			crect.setRect( x, y, w, h );
    	} else
        if ( isTopLevel() ) {
            QRect fr( frameGeometry() );
            if ( extra ) {
                fr.setLeft(fr.left() + x - crect.left());
                fr.setTop(fr.top() + y - crect.top());
                fr.setRight(fr.right() + (x + w - 1) - crect.right());
                fr.setBottom(fr.bottom() + (y + h - 1) - crect.bottom());
            }
			MoveWindow( winId(), fr.x(), fr.y(), fr.width(), fr.height(), true );
            if( !isVisible() )
                InvalidateRect( winId(), 0, false );
            RECT rect;
            GetClientRect( winId(), &rect );
            crect.setRect( x, y, rect.right - rect.left, rect.bottom - rect.top );
        } else { // not toplevel window
            if ( isMove ) {
                SetWindowPos( winId(), 0, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING );
            } else if ( isResize ) {
                SetWindowPos( winId(), 0, 0, 0, w, h, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSENDCHANGING );
            }
            RECT rect;
            GetClientRect( winId(), &rect );
            crect.setRect( x, y, rect.right - rect.left, rect.bottom - rect.top );
        }
    	clearWState( WState_ConfigPending );
    }
#ifdef DEBUG_QWIDGET
    qDebug( "internalSetGeometry new crect value: l: %d, t: %d, r: %d, b: %d", crect.left(), crect.top(), crect.right(), crect.bottom() );
#endif

    if ( isVisible() ) {
        if ( isMove && pos() != oldPos ) {
            QMoveEvent e( pos(), oldPos );
            QApplication::sendEvent( this, &e );
        }
        if ( isResize ) {
            QResizeEvent e( size(), oldSize );
            QApplication::sendEvent( this, &e );
            if( !testWFlags( WStaticContents ) )
                repaint( !testWFlags( WNoAutoErase ) );
        }
    } else {
        if ( isMove && pos() != oldPos )
            QApplication::postEvent( this,
                                     new QMoveEvent( pos(), oldPos ) );
        if ( isResize )
            QApplication::postEvent( this,
                                     new QResizeEvent( size(), oldSize ) );
    }
}

/*!
  \overload void QWidget::setMinimumSize( const QSize &size )
*/

/*!
  Sets the minimum size of the widget to \e w by \e h pixels.

  The widget cannot be resized to a smaller size than the minimum widget
  size. The widget's size is forced to the minimum size if the current
  size is smaller.

  If you use a layout inside the widget, the minimum size will be set by the layout and
  not by setMinimumSize, unless you set the layouts resize mode to QLayout::FreeResize.

  \sa minimumSize(), setMaximumSize(), setSizeIncrement(), resize(), size(), QLayout::setResizeMode()
*/

void QWidget::setMinimumSize( int minw, int minh )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::setMinimumSize( %d, %d )", minw, minh );
#endif
#if defined(CHECK_RANGE)

    if ( minw < 0 || minh < 0 )
        qWarning( "QWidget::setMinimumSize: The smallest allowed size is (0,0)" );
#endif

    createExtra();
    if ( extra->minw == minw && extra->minh == minh )
        return ;
    extra->minw = minw;
    extra->minh = minh;
    if ( minw > width() || minh > height() ) {
        bool resized = testWState( WState_Resized );
        resize( QMAX( minw, width() ), QMAX( minh, height() ) );
        if ( !resized )
            clearWState( WState_Resized ); //not a user resize
    }
    //if ( testWFlags(WType_TopLevel) )
    //do_size_hints( this, extra );
    updateGeometry();
}

/*!
  \overload void QWidget::setMaximumSize( const QSize &size )
*/

/*!
  Sets the maximum size of the widget to \e w by \e h pixels.

  The widget cannot be resized to a larger size than the maximum widget
  size. The widget's size is forced to the maximum size if the current
  size is greater.

  \sa maximumSize(), setMinimumSize(), setSizeIncrement(), resize(), size()
*/

void QWidget::setMaximumSize( int maxw, int maxh )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::setMaximumSize( %d, %d )", maxw, maxh );
#endif
#if defined(CHECK_RANGE)

    if ( maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX ) {
        qWarning( "QWidget::setMaximumSize: (%s/%s) "
                  "The largest allowed size is (%d,%d)",
                  name( "unnamed" ), className(), QWIDGETSIZE_MAX,
                  QWIDGETSIZE_MAX );
        maxw = QMIN( maxw, QWIDGETSIZE_MAX );
        maxh = QMIN( maxh, QWIDGETSIZE_MAX );
    }
    if ( maxw < 0 || maxh < 0 ) {
        qWarning( "QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
                  "are not possible",
                  name( "unnamed" ), className(), maxw, maxh );
        maxw = QMAX( maxw, 0 );
        maxh = QMAX( maxh, 0 );
    }
#endif
    createExtra();
    if ( extra->maxw == maxw && extra->maxh == maxh )
        return ;
    extra->maxw = maxw;
    extra->maxh = maxh;
    if ( maxw < width() || maxh < height() ) {
        bool resized = testWState( WState_Resized );
        resize( QMIN( maxw, width() ), QMIN( maxh, height() ) );
        if ( !resized )
            clearWState( WState_Resized ); //not a user resize
    }
    /*if ( testWFlags(WType_TopLevel) )
    do_size_hints( this, extra );*/
    updateGeometry();
}

/*!
  Sets the size increment of the widget.  When the user resizes the
  window, the size will move in steps of \e w pixels horizontally and
  \e h pixels vertically, with baseSize() as basis. Preferred widget sizes are therefore for
  non-negative integers \e i and \e j:
  \code
  width = baseSize().width() + i * sizeIncrement().width();
  height = baseSize().height() + j * sizeIncrement().height();
  \endcode

  Note that while you can set the size increment for all widgets, it
  has no effect except for top-level widgets.

  \warning The size increment has no effect under Windows, and may be
  disregarded by the window manager on X.

  \sa sizeIncrement(), setMinimumSize(), setMaximumSize(), resize(), size()
*/

void QWidget::setSizeIncrement( int w, int h )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::setSizeIncrement( %d, %d )", w, h );
#endif

    QTLWExtra* x = topData();
    if ( x->incw == w && x->inch == h )
        return ;
    x->incw = w;
    x->inch = h;
    /*if ( testWFlags(WType_TopLevel) )
    do_size_hints( this, extra );*/
}
/*!
  \overload void QWidget::setSizeIncrement( const QSize& )
*/


/*!
  Sets the base size of the widget.  The base size is important only
  in combination with size increments. See setSizeIncrement() for details.

  \sa baseSize()
*/

void QWidget::setBaseSize( int basew, int baseh )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::setBaseSize( %d, %d)", basew, baseh );
#endif

    createTLExtra();
    QTLWExtra* x = topData();
    if ( x->basew == basew && x->baseh == baseh )
        return ;
    x->basew = basew;
    x->baseh = baseh;
    /*if ( testWFlags(WType_TopLevel) )
    do_size_hints( this, extra );*/
}



/*!
  \overload void QWidget::setBaseSize( const QSize& )
*/

/*!
  \overload void QWidget::erase()
  This version erases the entire widget.
*/

/*!
  \overload void QWidget::erase( const QRect &r )
*/

/*!
  Erases the specified area \e (x,y,w,h) in the widget without generating
  a \link paintEvent() paint event\endlink.

  If \e w is negative, it is replaced with <code>width() - x</code>.
  If \e h is negative, it is replaced width <code>height() - y</code>.

  Child widgets are not affected.

  \sa repaint()
*/
void QWidget::erase( int x, int y, int w, int h )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::erase( %d, %d, %d, %d ) %s, %s", x, y, w, h, className(), name() );
#endif

    if ( w < 0 )
        w = crect.width() - x;
    if ( h < 0 )
        h = crect.height() - y;
    if ( w != 0 && h != 0 )
        erase( QRegion( x, y, w, h ) );
}

/*!
  Erases the area defined by \a reg, without generating a
  \link paintEvent() paint event\endlink.

  Child widgets are not affected.
*/

void QWidget::erase( const QRegion& reg )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::erase( const QRegion& reg ) %s, %s", className(), name() );
#endif

    if ( backgroundMode() == NoBackground || isDesktop() || !isVisible() )
        return ;
    QRect rr( reg.boundingRect() );
#ifdef DEBUG_QWIDGET

    qDebug( "QWidget::erase:reg.boundingRect()=(%d, %d, %d, %d)", rr.x(), rr.y(), rr.width(), rr.height() );
#endif

    QPixmap *pm = ( extra ) ? extra->bg_pix : NULL;
    qt_erase_region( this, pm, backgroundOffset(), reg );
}

/*! \overload

  This version of the function scrolls the entire widget and moves the
  widget's children along with the scroll.

  \sa bitBlt() QScrollView
*/
void QWidget::scroll( int dx, int dy )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::scroll( %d, %d )", dx, dy );
#endif
    if ( testWState( WState_BlockUpdates ) )
        return;

    UINT flags = SW_INVALIDATE | SW_SCROLLCHILDREN;
    if ( backgroundMode() != NoBackground )
        flags |= SW_ERASE;

    ScrollWindowEx( winId(), dx, dy, 0, 0, 0, 0, flags );
    UpdateWindow( winId() );
}

/*! Scrolls \a r \a dx pixels to the right and \a dy downwards.  Both
  \a dx and \a dy may be negative.

  If \a r is empty or invalid, the result is undefined.

  After scrolling, scroll() sends a paint event for the the part of \a r
  that is read but not written.  For example, when scrolling 10 pixels
  rightwards, the leftmost ten pixels of \a r need repainting. The paint
  event may be delivered immediately or later, depending on some heuristics.

  This version of scroll() does not move the children of this widget.

  \sa QScrollView erase() bitBlt()
*/
void QWidget::scroll( int dx, int dy, const QRect& r )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::scroll( %d, %d, %d/%d - %d/%d )", dx, dy, r.left(), r.top(), r.bottom(), r.right() );
#endif

    if ( testWState( WState_BlockUpdates ) )
        return;

    UINT flags = SW_INVALIDATE;
    if ( backgroundMode() != NoBackground )
        flags |= SW_ERASE;

    RECT wr;
    wr.top = r.top();
    wr.left = r.left();
    wr.bottom = r.bottom() + 1;
    wr.right = r.right() + 1;
    ScrollWindowEx( winId(), dx, dy, &wr, &wr, 0, 0, flags );
    UpdateWindow( winId() );
}

/*!
  \overload void QWidget::drawText( const QPoint &pos, const QString& str )
*/

/*!
  Writes \e str at position \e x,y.

  The \e y position is the base line position of the text.  The text is
  drawn using the default font and the default foreground color.

  This function is provided for convenience.  You will generally get
  more flexible results and often higher speed by using a a \link
  QPainter painter\endlink instead.

  \sa setFont(), foregroundColor(), QPainter::drawText()
*/

void QWidget::drawText( int x, int y, const QString &str )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::drawText( %d, %d, %s )", x, y, str.latin1() );
#endif

    if ( testWState( WState_Visible ) ) {
        QPainter paint;
        paint.begin( this );
        paint.drawText( x, y, str );
        paint.end();
    }
}


/*!
  Internal implementation of the virtual QPaintDevice::metric() function.

  Use the QPaintDeviceMetrics class instead.
*/

int QWidget::metric( int m ) const
{
    int val;
    if ( m == QPaintDeviceMetrics::PdmWidth ) {
        val = crect.width();
    } else if ( m == QPaintDeviceMetrics::PdmHeight ) {
        val = crect.height();
    } else {
        HDC ScreenDC = qt_display_dc();
        switch ( m ) {
        case QPaintDeviceMetrics::PdmDpiX:
        case QPaintDeviceMetrics::PdmPhysicalDpiX:
            val = GetDeviceCaps( ScreenDC, LOGPIXELSX );
            break;
        case QPaintDeviceMetrics::PdmDpiY:
        case QPaintDeviceMetrics::PdmPhysicalDpiY:
            val = GetDeviceCaps( ScreenDC, LOGPIXELSY );
            break;
        case QPaintDeviceMetrics::PdmWidthMM:
            val = crect.width()
                    * GetDeviceCaps( ScreenDC, HORZSIZE )
                    / GetDeviceCaps( ScreenDC, HORZRES );
            break;
        case QPaintDeviceMetrics::PdmHeightMM:
            val = crect.height()
                    * GetDeviceCaps( ScreenDC, VERTSIZE )
                    / GetDeviceCaps( ScreenDC, VERTRES );
            break;
        case QPaintDeviceMetrics::PdmNumColors:
            if (GetDeviceCaps( ScreenDC, RASTERCAPS ) & RC_PALETTE )
                val = GetDeviceCaps( ScreenDC, SIZEPALETTE );
            else {
                int bpp = GetDeviceCaps( ScreenDC, BITSPIXEL );
                if( bpp == 32 )
                    val = INT_MAX;
                else if( bpp <= 8 )
                    val = GetDeviceCaps( ScreenDC, NUMCOLORS );
                else
                    val = 1 << ( bpp * GetDeviceCaps( ScreenDC, PLANES ) );
            }
            break;
        case QPaintDeviceMetrics::PdmDepth:
            val = GetDeviceCaps( ScreenDC, BITSPIXEL );
            break;
        default:
            val = 0;
            qWarning( "QWidget::metric: Invalid metric command" );
        }
    }
    return val;
}

void QWidget::createSysExtra()
{
    extra->dropTarget = 0;
}

void QWidget::deleteSysExtra()
{
    setAcceptDrops( false );
}

void QWidget::createTLSysExtra()
{
    extra->topextra->winIcon = 0;
}

void QWidget::deleteTLSysExtra()
{
    if ( extra->topextra->winIcon )
        DestroyIcon( extra->topextra->winIcon );
}


/*!
  Returns TRUE if drop events are enabled for this widget.

  \sa setAcceptDrops()
*/

bool QWidget::acceptDrops() const
{
    return testWState( WState_DND );
}

/*!
  Announces to the system that this widget \e may be able to
  accept drop events.

  If the widgets is \link QWidget::isDesktop() the desktop\endlink,
  this may fail if another application is using the desktop - you
  can call acceptDrops() to test if this occurs.

  \sa acceptDrops()
*/

void QWidget::setAcceptDrops( bool on )
{
    if ( testWState( WState_DND ) == ( Qt::WState )on )
        return;

    // Enablement is defined by d->extra->dropTarget != 0.
    if ( on ) {
        // Turn on.
        createExtra();
        QWExtra *extra = extraData();
        if ( !extra->dropTarget ) {
            extra->dropTarget = qt_olednd_register( this );
            setWState( Qt::WState_DND );
        }
    } else {
        // Turn off.
        QWExtra *extra = extraData();
        if ( extra && extra->dropTarget ) {
            qt_olednd_unregister( this, extra->dropTarget );
            extra->dropTarget = 0;
            clearWState( Qt::WState_DND );
        }
    }
/*
    if ( children() ) {
        QObjectListIt it( *children() );
        register QObject *obj;
        while ( ( obj = it.current() ) ) {
            ++it;
            if ( obj->isWidgetType() )
                ( ( QWidget* ) obj ) ->setAcceptDrops( on );
        }
    }
*/
}

/*!
  Causes only the parts of the widget which overlap \a region
  to be visible.  If the region includes pixels outside the
  rect() of the widget, window system controls in that area
  may or may not be visible, depending on the platform.

  Note that this effect can be slow if the region is particularly
  complex.

  \sa setMask(QBitmap), clearMask()
*/

void QWidget::setMask( const QRegion& region )
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::setMask( const QRegion& region ) %s", className() );
#endif

    // Since SetWindowRegion takes ownership, and we need to translate,
    // we take a copy.
    HRGN wr = CreateRectRgn( 0, 0, 0, 0 );
    CombineRgn( wr, region.handle(), 0, RGN_COPY );

    SetWindowRgn( winId(), wr, true );
}

/*!
  Causes only the pixels of the widget for which \a bitmap
  has a corresponding 1 bit
  to be visible.  If the region includes pixels outside the
  rect() of the widget, window system controls in that area
  may or may not be visible, depending on the platform.

  Note that this effect can be slow if the region is particularly
  complex.

  \sa setMask(const QRegion&), clearMask()
*/

void QWidget::setMask( const QBitmap &bitmap )
{
    QRegion region( bitmap );
    setMask( region );
}

/*!
  Removes any mask set by setMask().

  \sa setMask()
*/

void QWidget::clearMask()
{
    SetWindowRgn( winId(), 0, true );
}

/*!\reimp
 */
void QWidget::setName( const char *name )
{
    QObject::setName( name );
}

/*!
  \internal

  Computes the frame rectangle when needed.  This is an internal function, you
  should never call this.
*/

void QWidget::updateFrameStrut() const
{
#ifdef DEBUG_QWIDGET
    qDebug( "QWidget::updateFrameStrut()" );
#endif

    QWidget *q = ( QWidget * ) this;

    if ( !isVisible() || isDesktop() ) {
        q->fstrut_dirty = isVisible();
        return;
    }

    RECT  fr, cr;
    GetWindowRect( q->winId(), &fr );
    GetClientRect( q->winId(), &cr );

    POINT pt;
    pt.x = 0;
    pt.y = 0;

    ClientToScreen( q->winId(), &pt );
    q->crect = QRect( QPoint( pt.x, pt.y ),
					  QPoint( pt.x + cr.right - 1, pt.y + cr.bottom - 1 ) );

    QTLWExtra *top = q->topData();
    top->ftop = crect.top() - fr.top;
    top->fleft = crect.left() - fr.left;
    top->fbottom = fr.bottom - crect.bottom() - 1;
    top->fright = fr.right - crect.right() - 1;

    q->fstrut_dirty = false;
}



/*!
    This function is called when the user finishes input composition,
    e.g. changes focus to another widget, moves the cursor, etc.
*/
void QWidget::resetInputContext()
{
#ifdef DEBUG_QWIDGET
    qDebug( "TODO qwidget_win.cpp resetInputContext()" );
#endif
}
void QWidget::setMouseTracking( bool enable )
{
    if ( enable )
        setWState( WState_MouseTracking );
    else
        clearWState( WState_MouseTracking );
}

/* I don't understand this function completly, but I hope that I've
   ported it correctly from qwidget_x11.cpp
   changed the last (WindowMinimized) since only this uses api-functions
   */
void QWidget::setWindowState( unsigned int newstate )
{
    QWidget *d = this;
    unsigned int oldstate = windowState();
    if (oldstate == newstate)
        return;

    int max = SW_MAXIMIZE;
    int min = SW_MINIMIZE;
    int normal = SW_SHOWNOACTIVATE;
    if (newstate & Qt::WindowActive) {
        max = SW_SHOWMAXIMIZED;
        min = SW_SHOWMINIMIZED;
        normal = SW_SHOWNORMAL;
    }

    if (isTopLevel()) {

        // Ensure the initial size is valid, since we store it as normalGeometry below.
        if (!isVisible())
            adjustSize();

        if ((oldstate & Qt::WindowMaximized) != (newstate & Qt::WindowMaximized)) {
            if (newstate & Qt::WindowMaximized && !(oldstate & Qt::WindowFullScreen))
                d->topData()->normalGeometry = geometry();
            if (isVisible() && !(newstate & Qt::WindowMinimized)) {
                ShowWindow(winId(), (newstate & Qt::WindowMaximized) ? max : normal);
                if (!(newstate & Qt::WindowFullScreen)) {
                    QRect r = d->topData()->normalGeometry;
                    if (!(newstate & Qt::WindowMaximized) && r.width() >= 0) {
                        if (pos() != r.topLeft() || size() !=r.size()) {
                            d->topData()->normalGeometry = QRect(0,0,-1,-1);
                            setGeometry(r);
                        }
                    }
                } else {
                    d->updateFrameStrut();
                }
            }
        }

        if ((oldstate & Qt::WindowFullScreen) != (newstate & Qt::WindowFullScreen)) {
            if (newstate & Qt::WindowFullScreen) {
                if (d->topData()->normalGeometry.width() < 0 && !(oldstate & Qt::WindowMaximized))
                    d->topData()->normalGeometry = geometry();
                d->topData()->savedFlags = GetWindowLongA(winId(), GWL_STYLE);
                UINT style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP;
                if (isVisible())
                    style |= WS_VISIBLE;
                SetWindowLongA(winId(), GWL_STYLE, style);
                QRect r = qApp->desktop()->screenGeometry(this);
                UINT swpf = SWP_FRAMECHANGED;
                if (newstate & Qt::WindowActive)
                    swpf |= SWP_NOACTIVATE;

                SetWindowPos(winId(), HWND_TOP, r.left(), r.top(), r.width(), r.height(), swpf);
                d->updateFrameStrut();
            } else {
                UINT style = d->topData()->savedFlags;
                if (isVisible())
                    style |= WS_VISIBLE;
                SetWindowLongA(winId(), GWL_STYLE, style);

                UINT swpf = SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE;
                if (newstate & Qt::WindowActive)
                    swpf |= SWP_NOACTIVATE;
                SetWindowPos(winId(), 0, 0, 0, 0, 0, swpf);
                d->updateFrameStrut();

                // preserve maximized state
                if (isVisible())
                    ShowWindow(winId(), (newstate & Qt::WindowMaximized) ? max : normal);

                if (!(newstate & Qt::WindowMaximized)) {
                    QRect r = d->topData()->normalGeometry;
                    d->topData()->normalGeometry = QRect(0,0,-1,-1);
                    if (r.isValid())
                        setGeometry(r);
                }
            }
        }

        if ((oldstate & Qt::WindowMinimized) != (newstate & Qt::WindowMinimized)) {
            if (isVisible())
                ShowWindow(winId(), (newstate & Qt::WindowMinimized) ? min :
                                    (newstate & Qt::WindowMaximized) ? max : normal);
        }
    }


    widget_state &= ~( WState_Minimized | WState_Maximized | WState_FullScreen );
    if ( newstate & WindowMinimized )
        widget_state |= WState_Minimized;
    if ( newstate & WindowMaximized )
        widget_state |= WState_Maximized;
    if ( newstate & WindowFullScreen )
        widget_state |= WState_FullScreen;

    QEvent e( QEvent::WindowStateChange );
    QApplication::sendEvent( this, &e );
}

/* needed for dynamic loading ... */
static BOOL ( WINAPI * qtSLWA ) (
    HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags ) = NULL;

/* see SIM-Icq -> plugins -> tansparent for original source */
void QWidget::setWindowOpacity( double opacity )
{
    /* imho this only works for toplevel / getMainWindow() */
    if ( !isTopLevel() )
        return ;
    /* This is only supported on w2k and higher - so load it on runtime */
    if ( !qtSLWA )
        qtSLWA = reinterpret_cast<decltype(qtSLWA)>(QLibrary::resolve( "user32.dll", "SetLayeredWindowAttributes" ));

    /* not found -> win9x ? */
    if ( !qtSLWA )
        return ;
    QT_WA( SetWindowLongW( winId(), GWL_EXSTYLE,
                           GetWindowLongW( winId(), GWL_EXSTYLE ) | WS_EX_LAYERED );,
           SetWindowLongA( winId(), GWL_EXSTYLE,
                           GetWindowLongA( winId(), GWL_EXSTYLE ) | WS_EX_LAYERED ); )

    int wl = GetWindowLongA(winId(), GWL_EXSTYLE);
    if (opacity != 1.0) {
        if ((wl&WS_EX_LAYERED) == 0)
            SetWindowLongA(winId(), GWL_EXSTYLE, wl|WS_EX_LAYERED);
    } else if (wl&WS_EX_LAYERED) {
        SetWindowLongA(winId(), GWL_EXSTYLE, wl & ~WS_EX_LAYERED);
    }

    (*qtSLWA)(winId(), 0, (int)(opacity * 255), LWA_ALPHA);
    topData()->opacity = (uchar)(opacity * 255);
}

double QWidget::windowOpacity() const
{
    /* is this needed ? */
    if ( !isTopLevel() )
        return 0.0;
    /* 'this' is 'const class QWidget' ... need to cast */
    return ( ( QWidget* ) this )->topData()->opacity / 255.0;
}
