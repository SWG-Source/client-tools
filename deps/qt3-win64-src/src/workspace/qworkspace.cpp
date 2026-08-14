/****************************************************************************
** $Id: qworkspace.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of the QWorkspace class
**
** Created : 931107
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the workspace module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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

#include "qworkspace.h"
#ifndef QT_NO_WORKSPACE
#include "qapplication.h"
#include "../widgets/qtitlebar_p.h"
#include "qobjectlist.h"
#include "qlayout.h"
#include "qtoolbutton.h"
#include "qlabel.h"
#include "qvbox.h"
#include "qaccel.h"
#include "qcursor.h"
#include "qpopupmenu.h"
#include "qmenubar.h"
#include "qguardedptr.h"
#include "qiconset.h"
#include "../widgets/qwidgetresizehandler_p.h"
#include "qfocusdata.h"
#include "qdatetime.h"
#include "qtooltip.h"
#include "qwmatrix.h"
#include "qimage.h"
#include "qscrollbar.h"
#include "qstyle.h"
#include "qbitmap.h"

// magic non-mdi things
#include "qtimer.h"
#include "qdockarea.h"
#include "qstatusbar.h"
#include "qmainwindow.h"
#include "qdockwindow.h"
#include "qtoolbar.h"

#define BUTTON_WIDTH	16
#define BUTTON_HEIGHT	14

/*!
    \class QWorkspace qworkspace.h
    \brief The QWorkspace widget provides a workspace window that can
    contain decorated windows, e.g. for MDI.

    \module workspace

    \ingroup application
    \ingroup organizers
    \mainclass

    MDI (multiple document interface) applications typically have one
    main window with a menu bar and toolbar, and a central widget that
    is a QWorkspace. The workspace itself contains zero, one or more
    document windows, each of which is a widget.

    The workspace itself is an ordinary Qt widget. It has a standard
    constructor that takes a parent widget and an object name. The
    parent window is usually a QMainWindow, but it need not be.

    Document windows (i.e. MDI windows) are also ordinary Qt widgets
    which have the workspace as their parent widget. When you call
    show(), hide(), showMaximized(), setCaption(), etc. on a document
    window, it is shown, hidden, etc. with a frame, caption, icon and
    icon text, just as you'd expect. You can provide widget flags
    which will be used for the layout of the decoration or the
    behaviour of the widget itself.

    To change or retrieve the geometry of MDI windows you must operate
    on the MDI widget's parentWidget(). (The parentWidget() provides
    access to the decorated window in which the MDI window's widget is
    shown.)

    A document window becomes active when it gets the keyboard focus.
    You can also activate a window in code using setFocus(). The user
    can activate a window by moving focus in the usual ways, for
    example by clicking a window or by pressing Tab. The workspace
    emits a signal windowActivated() when it detects the activation
    change, and the function activeWindow() always returns a pointer
    to the active document window.

    The convenience function windowList() returns a list of all
    document windows. This is useful to create a popup menu
    "<u>W</u>indows" on the fly, for example.

    QWorkspace provides two built-in layout strategies for child
    windows: cascade() and tile(). Both are slots so you can easily
    connect menu entries to them.

    If you want your users to be able to work with document windows
    larger than the actual workspace, set the scrollBarsEnabled
    property to TRUE.

    If the top-level window contains a menu bar and a document window
    is maximised, QWorkspace moves the document window's minimize,
    restore and close buttons from the document window's frame to the
    workspace window's menu bar. It then inserts a window operations
    menu at the far left of the menu bar.
*/

static bool inCaptionChange = FALSE;

class QWorkspaceChild : public QFrame
{
    Q_OBJECT

    friend class QWorkspace;
    friend class QTitleBar;

public:
    QWorkspaceChild( QWidget* window,
		     QWorkspace* parent=0, const char* name=0 );
    ~QWorkspaceChild();

    void setActive( bool );
    bool isActive() const;

    void adjustToFullscreen();

    void setStatusBar(QStatusBar *);
    QWidget* windowWidget() const;
    QWidget* iconWidget() const;

    void doResize();
    void doMove();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QSize baseSize() const;

signals:
    void showOperationMenu();
    void popupOperationMenu( const QPoint& );

public slots:
    void activate();
    void showMinimized();
    void showMaximized();
    void showNormal();
    void showShaded();
    void setCaption( const QString& );
    void internalRaise();
    void titleBarDoubleClicked();

    void move( int x, int y );

protected:
    bool event(QEvent * );
    void enterEvent( QEvent * );
    void leaveEvent( QEvent * );
    void childEvent( QChildEvent* );
    void resizeEvent( QResizeEvent * );
    void moveEvent( QMoveEvent * );
    bool eventFilter( QObject *, QEvent * );

    bool focusNextPrevChild( bool );

    void drawFrame( QPainter * );
    void styleChange( QStyle & );

private:
    QWidget* childWidget;
    QGuardedPtr<QWidget> lastfocusw;
    QWidgetResizeHandler *widgetResizeHandler;
    QTitleBar* titlebar;
    QGuardedPtr<QStatusBar> statusbar;
    QGuardedPtr<QTitleBar>  iconw;
    QSize windowSize;
    QSize shadeRestore;
    QSize shadeRestoreMin;
    bool act		    :1;
    bool shademode	    :1;
    bool snappedRight	    :1;
    bool snappedDown	    :1;
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QWorkspaceChild( const QWorkspaceChild & );
    QWorkspaceChild &operator=( const QWorkspaceChild & );
#endif
};

class QMainWindow;

class QWorkspacePrivate {
public:
    QWorkspaceChild* active;
    QPtrList<QWorkspaceChild> windows;
    QPtrList<QWorkspaceChild> focus;
    QPtrList<QWidget> icons;
    QWorkspaceChild* maxWindow;
    QRect maxRestore;
    QGuardedPtr<QFrame> maxcontrols;
    QGuardedPtr<QMenuBar> maxmenubar;

    int px;
    int py;
    QWidget *becomeActive;
    QGuardedPtr<QLabel> maxtools;
    QPopupMenu* popup;
    QPopupMenu* toolPopup;
    int menuId;
    int controlId;
    QString topCaption;

    QScrollBar *vbar, *hbar;
    QWidget *corner;
    int yoffset, xoffset;

    // toplevel mdi fu
    QWorkspace::WindowMode wmode;
    QGuardedPtr<QMainWindow> mainwindow;
    QPtrList<QDockWindow> dockwindows, newdocks;
};

static bool isChildOf( QWidget * child, QWidget * parent )
{
    if ( !parent || !child )
	return FALSE;
    QWidget * w = child;
    while( w && w != parent )
	w = w->parentWidget();
    return w != 0;
}

/*!
    Constructs a workspace with a \a parent and a \a name.
*/
QWorkspace::QWorkspace( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    init();
}

#ifdef QT_WORKSPACE_WINDOWMODE
/*!
    Constructs a workspace with a \a parent and a \a name. This
    constructor will also set the WindowMode to \a mode.

    \sa windowMode()
*/
QWorkspace::QWorkspace( QWorkspace::WindowMode mode, QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    init();
    d->wmode = mode;
}
#endif


/*!
    \internal
*/
void
QWorkspace::init()
{
    d = new QWorkspacePrivate;
    d->maxcontrols = 0;
    d->active = 0;
    d->maxWindow = 0;
    d->maxtools = 0;
    d->px = 0;
    d->py = 0;
    d->becomeActive = 0;
#if defined( QT_WORKSPACE_WINDOWMODE ) && defined( Q_WS_MAC )
    d->wmode = AutoDetect;
#else
    d->wmode = MDI;
#endif
    d->mainwindow = 0;
#if defined(Q_WS_WIN)
    d->popup = new QPopupMenu( this, "qt_internal_mdi_popup" );
    d->toolPopup = new QPopupMenu( this, "qt_internal_mdi_popup" );
#else
    d->popup = new QPopupMenu( parentWidget(), "qt_internal_mdi_popup" );
    d->toolPopup = new QPopupMenu( parentWidget(), "qt_internal_mdi_popup" );
#endif

    d->menuId = -1;
    d->controlId = -1;
    connect( d->popup, SIGNAL( aboutToShow() ), this, SLOT(operationMenuAboutToShow() ));
    connect( d->popup, SIGNAL( activated(int) ), this, SLOT( operationMenuActivated(int) ) );
    d->popup->insertItem(QIconSet(style().stylePixmap(QStyle::SP_TitleBarNormalButton)), tr("&Restore"), 1);
    d->popup->insertItem(tr("&Move"), 2);
    d->popup->insertItem(tr("&Size"), 3);
    d->popup->insertItem(QIconSet(style().stylePixmap(QStyle::SP_TitleBarMinButton)), tr("Mi&nimize"), 4);
    d->popup->insertItem(QIconSet(style().stylePixmap(QStyle::SP_TitleBarMaxButton)), tr("Ma&ximize"), 5);
    d->popup->insertSeparator();
    d->popup->insertItem(QIconSet(style().stylePixmap(QStyle::SP_TitleBarCloseButton)),
				  tr("&Close")
#ifndef QT_NO_ACCEL
					+"\t"+QAccel::keyToString(CTRL+Key_F4)
#endif
		    , this, SLOT( closeActiveWindow() ) );

    connect( d->toolPopup, SIGNAL( aboutToShow() ), this, SLOT(toolMenuAboutToShow() ));
    connect( d->toolPopup, SIGNAL( activated(int) ), this, SLOT( operationMenuActivated(int) ) );
    d->toolPopup->insertItem(tr("&Move"), 2);
    d->toolPopup->insertItem(tr("&Size"), 3);
    d->toolPopup->insertItem(tr("Stay on &Top"), 7);
    d->toolPopup->setItemChecked( 7, TRUE );
    d->toolPopup->setCheckable( TRUE );
    d->toolPopup->insertSeparator();
    d->toolPopup->insertItem(QIconSet(style().stylePixmap(QStyle::SP_TitleBarShadeButton)), tr("Sh&ade"), 6);
    d->toolPopup->insertItem(QIconSet(style().stylePixmap(QStyle::SP_TitleBarCloseButton)),
				      tr("&Close")
#ifndef QT_NO_ACCEL
					+"\t"+QAccel::keyToString( CTRL+Key_F4)
#endif
		, this, SLOT( closeActiveWindow() ) );

#ifndef QT_NO_ACCEL
    QAccel* a = new QAccel( this );
    a->connectItem( a->insertItem( ALT + Key_Minus),
		    this, SLOT( showOperationMenu() ) );

    a->connectItem( a->insertItem( CTRL + Key_F6),
		    this, SLOT( activateNextWindow() ) );
    a->connectItem( a->insertItem( CTRL + Key_Tab),
		    this, SLOT( activateNextWindow() ) );
    a->connectItem( a->insertItem( Key_Forward ),
		    this, SLOT( activateNextWindow() ) );

    a->connectItem( a->insertItem( CTRL + SHIFT + Key_F6),
		    this, SLOT( activatePreviousWindow() ) );
    a->connectItem( a->insertItem( CTRL + SHIFT + Key_Tab),
		    this, SLOT( activatePreviousWindow() ) );
    a->connectItem( a->insertItem( Key_Back ),
		    this, SLOT( activatePreviousWindow() ) );

    a->connectItem( a->insertItem( CTRL + Key_F4 ),
		    this, SLOT( closeActiveWindow() ) );
#endif

    setBackgroundMode( PaletteDark );
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

#ifndef QT_NO_WIDGET_TOPEXTRA
    d->topCaption = topLevelWidget()->caption();
#endif

    d->hbar = d->vbar = 0;
    d->corner = 0;
    d->xoffset = d->yoffset = 0;

    updateWorkspace();

    topLevelWidget()->installEventFilter( this );
}

/*!  Destroys the workspace and frees any allocated resources. */

QWorkspace::~QWorkspace()
{
    delete d;
    d = 0;
}

/*!\reimp */
QSize QWorkspace::sizeHint() const
{
    QSize s( QApplication::desktop()->size() );
    return QSize( s.width()*2/3, s.height()*2/3);
}

/*! \reimp */
void QWorkspace::setPaletteBackgroundColor( const QColor & c )
{
    setEraseColor( c );
}


/*! \reimp */
void QWorkspace::setPaletteBackgroundPixmap( const QPixmap & pm )
{
    setErasePixmap( pm );
}

/*! \reimp */
void QWorkspace::childEvent( QChildEvent * e)
{
    if (e->inserted() && e->child()->isWidgetType()) {
	QWidget* w = (QWidget*) e->child();
	if ( !w || !w->testWFlags( WStyle_Title | WStyle_NormalBorder | WStyle_DialogBorder) || w->testWFlags(WType_Dialog)
	     || d->icons.contains( w ) || w == d->vbar || w == d->hbar || w == d->corner )
	    return;	    // nothing to do

	bool wasMaximized = w->isMaximized();
	bool wasMinimized = w->isMinimized();
	bool hasBeenHidden = w->isHidden();
	bool hasSize = w->testWState( WState_Resized );
	int x = w->x();
	int y = w->y();
	bool hasPos = x != 0 || y != 0;
	QSize s = w->size().expandedTo( w->minimumSizeHint() );
	if ( !hasSize && w->sizeHint().isValid() )
	    w->adjustSize();

	QWorkspaceChild* child = new QWorkspaceChild( w, this, "qt_workspacechild" );
	child->installEventFilter( this );

	connect( child, SIGNAL( popupOperationMenu(const QPoint&) ),
		 this, SLOT( popupOperationMenu(const QPoint&) ) );
	connect( child, SIGNAL( showOperationMenu() ),
		 this, SLOT( showOperationMenu() ) );
	d->windows.append( child );
	if ( child->isVisibleTo( this ) )
	    d->focus.append( child );
	child->internalRaise();

	if ( !hasPos )
	    place( child );
	if ( hasSize )
	    child->resize( s + child->baseSize() );
	else
	    child->adjustSize();
	if ( hasPos )
	    child->move( x, y );

	if ( hasBeenHidden )
	    w->hide();
	else if ( !isVisible() ) 	// that's a case were we don't receive a showEvent in time. Tricky.
	    child->show();

	if ( wasMaximized )
	    w->showMaximized();
	else if ( wasMinimized )
	    w->showMinimized();
        else if (!hasBeenHidden)
	    activateWindow( w );

	updateWorkspace();
    } else if (e->removed() ) {
	if ( d->windows.contains( (QWorkspaceChild*)e->child() ) ) {
	    d->windows.removeRef( (QWorkspaceChild*)e->child() );
	    d->focus.removeRef( (QWorkspaceChild*)e->child() );
	    if (d->maxWindow == e->child())
		d->maxWindow = 0;
	    updateWorkspace();
	}
    }
}

/*! \reimp
*/
#ifndef QT_NO_WHEELEVENT
void QWorkspace::wheelEvent( QWheelEvent *e )
{
    if ( !scrollBarsEnabled() )
	return;
    if ( d->vbar && d->vbar->isVisible() && !( e->state() & AltButton ) )
	QApplication::sendEvent( d->vbar, e );
    else if ( d->hbar && d->hbar->isVisible() )
	QApplication::sendEvent( d->hbar, e );
}
#endif

void QWorkspace::activateWindow( QWidget* w, bool change_focus )
{
    if ( !w ) {
	d->active = 0;
	emit windowActivated( 0 );
	return;
    }
    if ( d->wmode == MDI && !isVisibleTo( 0 ) ) {
	d->becomeActive = w;
	return;
    }

    if ( d->active && d->active->windowWidget() == w ) {
	if ( !isChildOf( focusWidget(), w ) ) // child window does not have focus
	    d->active->setActive( TRUE );
	return;
    }

    d->active = 0;
    // First deactivate all other workspace clients
    QPtrListIterator<QWorkspaceChild> it( d->windows );
    while ( it.current () ) {
     	QWorkspaceChild* c = it.current();
	++it;
	if(windowMode() == QWorkspace::TopLevel && c->isTopLevel() &&
	    c->windowWidget() == w && !c->isActive())
	    c->setActiveWindow();
	if (c->windowWidget() == w)
	    d->active = c;
	else
	    c->setActive( FALSE );
    }

    if (!d->active)
	return;

    // Then activate the new one, so the focus is stored correctly
    d->active->setActive( TRUE );

    if (!d->active)
	return;

    if ( d->maxWindow && d->maxWindow != d->active && d->active->windowWidget() &&
	 d->active->windowWidget()->testWFlags( WStyle_MinMax ) &&
	 !d->active->windowWidget()->testWFlags( WStyle_Tool ) ) {
        d->active->showMaximized();
	if ( d->maxtools ) {
#ifndef QT_NO_WIDGET_TOPEXTRA
	    if ( w->icon() ) {
		QPixmap pm(*w->icon());
		int iconSize = d->maxtools->size().height();
		if(pm.width() > iconSize || pm.height() > iconSize) {
		    QImage im;
		    im = pm;
		    pm = im.smoothScale( QMIN(iconSize, pm.width()), QMIN(iconSize, pm.height()) );
		}
		d->maxtools->setPixmap( pm );
	    } else
#endif
	    {
		QPixmap pm(14,14);
		pm.fill( color1 );
		pm.setMask(pm.createHeuristicMask());
		d->maxtools->setPixmap( pm );
	    }
	}
    } else { // done implicitly in maximizeWindow otherwise        
        d->active->internalRaise();
    }

    if ( change_focus ) {
	if ( d->focus.find( d->active ) >=0 ) {
	    d->focus.removeRef( d->active );
	    d->focus.append( d->active );
	}
    }

    updateWorkspace();
    emit windowActivated( w );
}


/*!
    Returns the active window, or 0 if no window is active.
*/
QWidget* QWorkspace::activeWindow() const
{
    return d->active?d->active->windowWidget():0;
}


void QWorkspace::place( QWidget* w)
{
    QPtrList<QWidget> widgets;
    for(QPtrListIterator<QWorkspaceChild> it( d->windows ); it.current(); ++it)
	if ((*it) != w)
	    widgets.append((*it));

    if(d->wmode == TopLevel) {
	for(QPtrListIterator<QDockWindow> it( d->dockwindows ); it.current(); ++it)
	    if ((*it) != w)
		widgets.append((*it));
    }

    int overlap, minOverlap = 0;
    int possible;

    QRect r1(0, 0, 0, 0);
    QRect r2(0, 0, 0, 0);
    QRect maxRect = rect();
    if(d->wmode == TopLevel) {
	const QDesktopWidget *dw = qApp->desktop();
	maxRect = dw->availableGeometry(dw->screenNumber(topLevelWidget()));
    }
    int x = maxRect.left(), y = maxRect.top();
    QPoint wpos(maxRect.left(), maxRect.top());

    bool firstPass = TRUE;

    do {
	if ( y + w->height() > maxRect.bottom() ) {
	    overlap = -1;
	} else if( x + w->width() > maxRect.right() ) {
	    overlap = -2;
	} else {
	    overlap = 0;

	    r1.setRect(x, y, w->width(), w->height());

	    QWidget *l;
	    QPtrListIterator<QWidget> it( widgets );
	    while ( it.current () ) {
		l = it.current();
		++it;

		if ( d->maxWindow == l )
		    r2 = d->maxRestore;
		else
		    r2.setRect(l->x(), l->y(), l->width(), l->height());

		if (r2.intersects(r1)) {
		    r2.setCoords(QMAX(r1.left(), r2.left()),
				 QMAX(r1.top(), r2.top()),
				 QMIN(r1.right(), r2.right()),
				 QMIN(r1.bottom(), r2.bottom())
				 );

		    overlap += (r2.right() - r2.left()) *
			       (r2.bottom() - r2.top());
		}
	    }
	}

	if (overlap == 0) {
	    wpos = QPoint(x, y);
	    break;
	}

	if (firstPass) {
	    firstPass = FALSE;
	    minOverlap = overlap;
	} else if ( overlap >= 0 && overlap < minOverlap) {
	    minOverlap = overlap;
	    wpos = QPoint(x, y);
	}

	if ( overlap > 0 ) {
	    possible = maxRect.right();
	    if ( possible - w->width() > x) possible -= w->width();

	    QWidget *l;
	    QPtrListIterator<QWidget> it( widgets );
	    while ( it.current () ) {
		l = it.current();
		++it;
		if ( d->maxWindow == l )
		    r2 = d->maxRestore;
		else
		    r2.setRect(l->x(), l->y(), l->width(), l->height());

		if( ( y < r2.bottom() ) && ( r2.top() < w->height() + y ) ) {
		    if( r2.right() > x )
			possible = possible < r2.right() ?
				   possible : r2.right();

		    if( r2.left() - w->width() > x )
			possible = possible < r2.left() - w->width() ?
				   possible : r2.left() - w->width();
		}
	    }

	    x = possible;
	} else if ( overlap == -2 ) {
	    x = maxRect.left();
	    possible = maxRect.bottom();

	    if ( possible - w->height() > y ) possible -= w->height();

	    QWidget *l;
	    QPtrListIterator<QWidget> it( widgets );
	    while ( it.current () ) {
		l = it.current();
		++it;
		if ( d->maxWindow == l )
		    r2 = d->maxRestore;
		else
		    r2.setRect(l->x(), l->y(), l->width(), l->height());

		if( r2.bottom() > y)
		    possible = possible < r2.bottom() ?
			       possible : r2.bottom();

		if( r2.top() - w->height() > y )
		    possible = possible < r2.top() - w->height() ?
			       possible : r2.top() - w->height();
	    }

	    y = possible;
	}
    }
    while( overlap != 0 && overlap != -1 );

#if 0
    if(windowMode() == QWorkspace::TopLevel && wpos.y()) {
	QPoint fr = w->topLevelWidget()->frameGeometry().topLeft(),
		r = w->topLevelWidget()->geometry().topLeft();
	wpos += QPoint(r.x() - fr.x(), r.y() - fr.y());
    }
#endif
    w->move(wpos);
    updateWorkspace();
}


void QWorkspace::insertIcon( QWidget* w )
{
    if ( !w || d->icons.contains( w ) )
	return;
    d->icons.append( w );
    if (w->parentWidget() != this )
	w->reparent( this, 0, QPoint(0,0), FALSE);
    QRect cr = updateWorkspace();
    int x = 0;
    int y = cr.height() - w->height();

    QPtrListIterator<QWidget> it( d->icons );
    while ( it.current () ) {
	QWidget* i = it.current();
	++it;
	if ( x > 0 && x + i->width() > cr.width() ){
	    x = 0;
	    y -= i->height();
	}

	if ( i != w &&
	    i->geometry().intersects( QRect( x, y, w->width(), w->height() ) ) )
	    x += i->width();
    }
    w->move( x, y );

    if ( isVisibleTo( parentWidget() ) ) {
	w->show();
	w->lower();
    }
    updateWorkspace();
}


void QWorkspace::removeIcon( QWidget* w)
{
    if ( !d->icons.contains( w ) )
	return;
    d->icons.remove( w );
    w->hide();
}


/*! \reimp  */
void QWorkspace::resizeEvent( QResizeEvent * )
{
    if ( d->maxWindow ) {
	d->maxWindow->adjustToFullscreen();
	if (d->maxWindow->windowWidget())
	    ((QWorkspace*)d->maxWindow->windowWidget())->setWState( WState_Maximized );
    }

    QRect cr = updateWorkspace();

    QPtrListIterator<QWorkspaceChild> it( d->windows );
    while ( it.current () ) {
	QWorkspaceChild* c = it.current();
	++it;
	if ( c->windowWidget() && !c->windowWidget()->testWFlags( WStyle_Tool ) )
	    continue;

	int x = c->x();
	int y = c->y();
	if ( c->snappedDown )
	    y =  cr.height() - c->height();
	if ( c->snappedRight )
	    x =  cr.width() - c->width();

	if ( x != c->x() || y != c->y() )
	    c->move( x, y );
    }

}

void QWorkspace::handleUndock(QDockWindow *w)
{
    const QDesktopWidget *dw = qApp->desktop();
    QRect maxRect = dw->availableGeometry(dw->screenNumber(d->mainwindow));
    QPoint wpos(maxRect.left(), maxRect.top());
    int possible = 0;
    if(!::qt_cast<QToolBar*>(w)) {
	struct place_score { int o, x, y; } score = {0, 0, 0};
	int left = 1, x = wpos.x(), y = wpos.y();
	QPtrListIterator<QDockWindow> it( d->dockwindows );
	while(1) {
	    if(y + w->height() > maxRect.bottom()) {
		if(left) {
		    x = maxRect.right() - w->width();
		    y = maxRect.top();
		    left = 0;
		} else {
		    break;
		}
	    }

	    QDockWindow *l, *nearest = NULL, *furthest;
	    for ( it.toFirst(); it.current(); ++it ) {
		l = it.current();
		if ( l != w && y == l->y() ) {
		    if(!nearest) {
			nearest = l;
		    } else if(l->x() == x) {
			nearest = l;
			break;
		    } else if(left && (l->x() - x) < (nearest->x() - x)) {
			nearest = l;
		    } else if(!left && (x - l->x()) < (x - nearest->x())) {
			nearest = l;
		    }
		}
	    }
	    QRect r2(x, y, w->width(), w->height());
	    if(!nearest || !nearest->geometry().intersects(r2)) {
		wpos = QPoint(x, y); //best possible outcome
		possible = 2;
		break;
	    }

	    QDockWindow *o = NULL;
	    int overlap = 0;
	    for( it.toFirst(); it.current(); ++it ) {
		l = it.current();
		if ( l != w && l->geometry().intersects(QRect(QPoint(x, y), w->size()))) {
		    overlap++;
		    o = l;
		}
	    }
	    if(o && overlap == 1 && w->isVisible() && !o->isVisible()) {
		wpos = QPoint(x, y);
		possible = 2;
		while(d->dockwindows.remove(o));
		d->newdocks.append(o);
		if(d->newdocks.count() == 1)
		    QTimer::singleShot(0, this, SLOT(dockWindowsShow()));
		break;
	    }

	    for ( furthest = nearest, it.toFirst(); it.current(); ++it ) {
		l = it.current();
		if ( l != w && l->y() == nearest->y() &&
		     ((left  && (l->x() == nearest->x() + nearest->width())) ||
		      (!left && (l->x() + l->width() == nearest->x()) )))
		    furthest = l;
	    }
	    if(left)
		x = furthest->x() + furthest->width();
	    else
		x = furthest->x() - w->width();

	    QPoint sc_pt(x, y);
	    place_score sc;
	    if(left)
		sc.x = (x + w->width()) * 2;
	    else
		sc.x = ((maxRect.width() - x) * 2) + 1;
	    sc.y = sc_pt.y();
	    for( sc.o = 0, it.toFirst(); it.current(); ++it ) {
		l = it.current();
		if ( l != w && l->geometry().intersects(QRect(sc_pt, w->size())))
		    sc.o++;
	    }
	    if(maxRect.contains(QRect(sc_pt, w->size())) &&
	       (!possible || (sc.o < score.o) ||
		((score.o || sc.o == score.o) && score.x < sc.x))) {
		wpos = sc_pt;
		score = sc;
		possible = 1;
	    }
	    y += nearest->height();
	    if(left)
		x = maxRect.x();
	    else
		x = maxRect.right() - w->width();
	}
	if(!possible || (possible == 1 && score.o)) { //fallback to less knowledgeable function
	    place(w);
	    wpos = w->pos();
	}
    }

    bool ishidden = w->isHidden();
    QSize olds(w->size());
    if(w->place() == QDockWindow::InDock)
	w->undock();
    w->move(wpos);
    w->resize(olds);
    if(!ishidden)
	w->show();
    else
	w->hide();
}

void QWorkspace::dockWindowsShow()
{
    QPtrList<QDockWindow> lst = d->newdocks;
    d->newdocks.clear();
    for(QPtrListIterator<QDockWindow> dw_it(lst); (*dw_it); ++dw_it) {
	if(d->dockwindows.find((*dw_it)) != -1)
	    continue;
	handleUndock((*dw_it));
	d->dockwindows.append((*dw_it));
    }
}

/*! \reimp */
void QWorkspace::showEvent( QShowEvent *e )
{
    /* This is all magic, be carefull when playing with this code - this tries to allow people to
       use QWorkspace as a high level abstraction for window management, but removes enforcement that
       QWorkspace be used as an MDI. */
    if(d->wmode == AutoDetect) {
	d->wmode = MDI;
	QWidget *o = topLevelWidget();
	if(::qt_cast<QMainWindow*>(o)) {
	    d->wmode = TopLevel;
	    const QObjectList *c = o->children();
	    for(QObjectListIt it(*c); it; ++it) {
		if((*it)->isWidgetType() && !((QWidget *)(*it))->isTopLevel() &&
		   !::qt_cast<QHBox*>(*it) && !::qt_cast<QVBox*>(*it) &&
		   !::qt_cast<QWorkspaceChild*>(*it) && !(*it)->inherits("QHideDock") &&
		   !::qt_cast<QDockArea*>(*it) && !::qt_cast<QWorkspace*>(*it) &&
		   !::qt_cast<QMenuBar*>(*it) && !::qt_cast<QStatusBar*>(*it)) {
		    d->wmode = MDI;
		    break;
		}
	    }
	}
    }
    if(d->wmode == TopLevel) {
	QWidget *o = topLevelWidget();
	d->mainwindow = ::qt_cast<QMainWindow*>(o);
	const QObjectList children = *o->children();
	for(QObjectListIt it(children); it; ++it) {
	    if(!(*it)->isWidgetType())
		continue;
	    QWidget *w = (QWidget *)(*it);
	    if(w->isTopLevel())
		continue;
	    if(::qt_cast<QDockArea*>(w)) {
		const QObjectList *dock_c = w->children();
		if(dock_c) {
		    QPtrList<QToolBar> tb_list;
		    for(QObjectListIt dock_it(*dock_c); dock_it; ++dock_it) {
			if(!(*dock_it)->isWidgetType())
			    continue;
			if(::qt_cast<QToolBar*>(*dock_it)) {
			    tb_list.append((QToolBar *)(*dock_it));
			} else if (::qt_cast<QDockWindow*>(*dock_it)) {
			    QDockWindow *dw = (QDockWindow*)(*dock_it);
			    dw->move(dw->mapToGlobal(QPoint(0, 0)));
			    d->newdocks.append(dw);
			} else {
			    qDebug("not sure what to do with %s %s", (*dock_it)->className(),
				   (*dock_it)->name());
			}
		    }
		    if(tb_list.count() == 1) {
			QToolBar *tb = tb_list.first();
			tb->move(0, 0);
			d->newdocks.prepend(tb);
		    } else if(tb_list.count()) {
			QDockWindow *dw = new QDockWindow(QDockWindow::OutsideDock,
							  w->parentWidget(),
							  QString("QMagicDock_") + w->name());
			dw->installEventFilter(this);
			dw->setResizeEnabled(TRUE);
			dw->setCloseMode( QDockWindow::Always );
			dw->setResizeEnabled(FALSE);
#ifndef QT_NO_WIDGET_TOPEXTRA
			dw->setCaption(o->caption());
#endif
			QSize os(w->size());
			if(w->layout() && w->layout()->hasHeightForWidth()) {
			    w->layout()->invalidate();
			    os.setHeight(w->layout()->heightForWidth(os.width()));
			}
			if(!w->isHidden())
			    dw->show();
			w->reparent(dw, QPoint(0, 0));
			dw->setWidget(w);
			dw->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
			dw->setGeometry(0, 0, os.width(), os.height() + dw->sizeHint().height());
			d->newdocks.prepend(dw);
			((QDockArea*)w)->setAcceptDockWindow(dw, FALSE);
			w->show();
		    }
		}
		w->installEventFilter(this);
	    } else if(::qt_cast<QStatusBar*>(w)) {
		if(activeWindow()) {
		    QWorkspaceChild *c;
		    if ( ( c = findChild(activeWindow()) ) )
			c->setStatusBar((QStatusBar*)w);
		}
	    } else if(::qt_cast<QWorkspaceChild*>(w)) {
		w->reparent(this, w->testWFlags(~(0)) | WType_TopLevel, w->pos());
	    }
	}
	dockWindowsShow(); //now place and undock windows discovered

	QWidget *w = new QWidget(NULL, "QDoesNotExist",
				 WType_Dialog | WStyle_Customize | WStyle_NoBorder);
//	if(qApp->mainWidget() == o)
//	    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
	QDesktopWidget *dw = QApplication::desktop();
	w->setGeometry(dw->availableGeometry(dw->screenNumber(o)));
	o->reparent(w, QPoint(0, 0), TRUE);
	{
	    QMenuBar *mb = 0;
	    if(::qt_cast<QMainWindow*>(o))
		mb = ((QMainWindow *)o)->menuBar();
	    if(!mb)
		mb = (QMenuBar*)o->child(NULL, "QMenuBar");
	    if(mb)
		mb->reparent(w, QPoint(0, 0));
	}
	reparent(w, QPoint(0,0));
	setGeometry(0, 0, w->width(), w->height());
#if 0
	/* Hide really isn't acceptable because I need to make the rest of Qt
	   think it is visible, so instead I set it to an empty mask. I'm not
	   sure what problems this is going to create, hopefully everything will
	   be happy (or not even notice since this is mostly intended for Qt/Mac) */
//	w->setMask(QRegion());
//	w->show();
#else
	w->hide();
#endif
    }

    //done with that nastiness, on with your regularly schedueled programming..
    if ( d->maxWindow && !style().styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, this))
	showMaximizeControls();
    QWidget::showEvent( e );
    if ( d->becomeActive ) {
	activateWindow( d->becomeActive );
	d->becomeActive = 0;
    } else if ( d->windows.count() > 0 && !d->active ) {
	activateWindow( d->windows.first()->windowWidget() );
    }

    // force a frame repaint - this is a workaround for what seems to be a bug
    // introduced when changing the QWidget::show() implementation. Might be
    // a windows bug as well though.
    for (QPtrListIterator<QWorkspaceChild> it( d->windows ); it.current(); ++it ) {
	QWorkspaceChild* c = it.current();
        QApplication::postEvent(c, new QPaintEvent(c->rect(), TRUE));
    }

    updateWorkspace();
}

/*! \reimp */
void QWorkspace::hideEvent( QHideEvent * )
{
    if ( !isVisibleTo(0) && !style().styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, this))
	hideMaximizeControls();
}

void QWorkspace::minimizeWindow( QWidget* w)
{
    QWorkspaceChild* c = findChild( w );

    if ( !w || w && (!w->testWFlags( WStyle_Minimize ) || w->testWFlags( WStyle_Tool) ) )
	return;

    if ( c ) {
	QWorkspace *fake = (QWorkspace*)w;

	setUpdatesEnabled( FALSE );
	bool wasMax = FALSE;
	if ( c == d->maxWindow ) {
	    wasMax = TRUE;
	    d->maxWindow = 0;
	    inCaptionChange = TRUE;
#ifndef QT_NO_WIDGET_TOPEXTRA
	    if ( !!d->topCaption )
		topLevelWidget()->setCaption( d->topCaption );
#endif
	    inCaptionChange = FALSE;
	    if ( !style().styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, this) )
		hideMaximizeControls();
	    for (QPtrListIterator<QWorkspaceChild> it( d->windows ); it.current(); ++it ) {
		QWorkspaceChild* c = it.current();
		if ( c->titlebar )
		    c->titlebar->setMovable( TRUE );
		c->widgetResizeHandler->setActive( TRUE );
	    }
	}

	insertIcon( c->iconWidget() );
	c->hide();
	if ( wasMax )
	    c->setGeometry( d->maxRestore );
	d->focus.append( c );

        activateWindow(w);

	setUpdatesEnabled( TRUE );
	updateWorkspace();

	fake->clearWState( WState_Maximized );
	fake->setWState( WState_Minimized );
	c->clearWState( WState_Maximized );
	c->setWState( WState_Minimized );
    }
}

void QWorkspace::normalizeWindow( QWidget* w)
{
    QWorkspaceChild* c = findChild( w );
    if ( !w )
	return;
    if ( c ) {
	QWorkspace *fake = (QWorkspace*)w;
	fake->clearWState( WState_Minimized | WState_Maximized );
	if ( !style().styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, this) && d->maxWindow ) {
	    hideMaximizeControls();
	} else {
	    if ( w->minimumSize() != w->maximumSize() )
		c->widgetResizeHandler->setActive( TRUE );
	    if ( c->titlebar )
		c->titlebar->setMovable(TRUE);
	}
	fake->clearWState( WState_Minimized | WState_Maximized );
	c->clearWState( WState_Minimized | WState_Maximized );

	if ( c == d->maxWindow ) {
	    c->setGeometry( d->maxRestore );
	    d->maxWindow = 0;
#ifndef QT_NO_WIDGET_TOPEXTRA
	    inCaptionChange = TRUE;
	    if ( !!d->topCaption )
		topLevelWidget()->setCaption( d->topCaption );
	    inCaptionChange = FALSE;
#endif
	} else {
	    if ( c->iconw )
		removeIcon( c->iconw->parentWidget() );
	    c->show();
	}

	if ( !style().styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, this))
	    hideMaximizeControls();
	for (QPtrListIterator<QWorkspaceChild> it( d->windows ); it.current(); ++it ) {
	    QWorkspaceChild* c = it.current();
	    if ( c->titlebar )
		c->titlebar->setMovable( TRUE );
	    if ( c->childWidget && c->childWidget->minimumSize() != c->childWidget->maximumSize() )
		c->widgetResizeHandler->setActive( TRUE );
	}
	activateWindow( w, TRUE );
	updateWorkspace();
    }
}

void QWorkspace::maximizeWindow( QWidget* w)
{
    QWorkspaceChild* c = findChild( w );

    if ( !w || w && (!w->testWFlags( WStyle_Maximize ) || w->testWFlags( WStyle_Tool) ) )
	return;

    if ( c ) {
	setUpdatesEnabled( FALSE );
	if (c->iconw && d->icons.contains( c->iconw->parentWidget() ) )
	    normalizeWindow( w );
	QWorkspace *fake = (QWorkspace*)w;

	QRect r( c->geometry() );
	c->adjustToFullscreen();
	c->show();
	c->internalRaise();
	qApp->sendPostedEvents( c, QEvent::Resize );
	qApp->sendPostedEvents( c, QEvent::Move );
	qApp->sendPostedEvents( c, QEvent::ShowWindowRequest );
	if ( d->maxWindow != c ) {
	    if ( d->maxWindow )
		d->maxWindow->setGeometry( d->maxRestore );
	    d->maxWindow = c;
	    d->maxRestore = r;
	}

	activateWindow( w );
	if(!style().styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, this)) {
	    showMaximizeControls();
	} else {
	    c->widgetResizeHandler->setActive( FALSE );
	    if ( c->titlebar )
		c->titlebar->setMovable( FALSE );
	}
#ifndef QT_NO_WIDGET_TOPEXTRA
	inCaptionChange = TRUE;
	if ( !!d->topCaption )
	    topLevelWidget()->setCaption( tr("%1 - [%2]")
		.arg(d->topCaption).arg(c->caption()) );
	inCaptionChange = FALSE;
#endif
	setUpdatesEnabled( TRUE );

	updateWorkspace();

	fake->clearWState( WState_Minimized );
	fake->setWState( WState_Maximized );
	c->clearWState( WState_Minimized );
	c->setWState( WState_Maximized );
    }
}

void QWorkspace::showWindow( QWidget* w)
{
    if ( d->maxWindow && w->testWFlags( WStyle_Maximize ) && !w->testWFlags( WStyle_Tool) )
	maximizeWindow(w);
    else if (w->isMinimized() && !w->testWFlags(WStyle_Tool))
	minimizeWindow(w);
    else if ( !w->testWFlags( WStyle_Tool ) )
	normalizeWindow(w);
    else
	w->parentWidget()->show();
    if ( d->maxWindow )
	d->maxWindow->internalRaise();
    updateWorkspace();
}


QWorkspaceChild* QWorkspace::findChild( QWidget* w)
{
    QPtrListIterator<QWorkspaceChild> it( d->windows );
    while ( it.current () ) {
	QWorkspaceChild* c = it.current();
	++it;
	if (c->windowWidget() == w)
	    return c;
    }
    return 0;
}

/*!
  \obsolete
  \overload
 */
QWidgetList QWorkspace::windowList() const
{
    return windowList( CreationOrder );
}

/*!
    Returns a list of all windows. If \a order is CreationOrder
    (the default) the windows are listed in the order in which they
    had been inserted into the workspace. If \a order is StackingOrder
    the windows are listed in their stacking order, with the topmost window
    being the last window in the list.

    QWidgetList is the same as QPtrList<QWidget>.

    \sa QPtrList
*/
QWidgetList QWorkspace::windowList( WindowOrder order ) const
{
    QWidgetList windows;
    if ( order == StackingOrder ) {
	const QObjectList *cl = children();
	if ( cl ) {
	    QObjectListIt it( *cl );
	    while (it.current()) {
		QObject *o = it.current();
		++it;
		QWorkspaceChild *c = ::qt_cast<QWorkspaceChild*>(o);
		if (c && c->windowWidget())
		    windows.append(c->windowWidget());
	    }
	}
    } else {
	QPtrListIterator<QWorkspaceChild> it( d->windows );
	while (it.current()) {
	    QWorkspaceChild* c = it.current();
	    ++it;
	    if ( c->windowWidget() )
		windows.append( c->windowWidget() );
	}
    }
    return windows;
}

/*!\reimp*/
bool QWorkspace::eventFilter( QObject *o, QEvent * e)
{
    if(d->wmode == TopLevel && d->mainwindow && o->parent() == d->mainwindow) {
	if((e->type() == QEvent::ChildInserted || e->type() == QEvent::Reparent) &&
	    ::qt_cast<QDockArea*>(o) && !((QWidget*)o)->isVisible()) {
	    QChildEvent *ce = (QChildEvent*)e;
	    if(!::qt_cast<QDockWindow*>(ce->child())) {
		qDebug("No idea what to do..");
		return FALSE;
	    }
	    QDockWindow *w = (QDockWindow*)ce->child();
	    if(d->newdocks.find(w) == -1 && d->dockwindows.find(w) == -1) {
		if(::qt_cast<QToolBar*>(w))
		    d->newdocks.prepend(w);
		else
		    d->newdocks.append(w);
		if(d->newdocks.count() == 1)
		    QTimer::singleShot(0, this, SLOT(dockWindowsShow()));
	    }
	} else if(e->type() == QEvent::Hide && !e->spontaneous() && !qstrncmp(o->name(), "QMagicDock_", 11)) {
//	    d->mainwindow->close();
	}
	return QWidget::eventFilter(o, e);
    }

    static QTime* t = 0;
    static QWorkspace* tc = 0;
#ifndef QT_NO_MENUBAR
    if ( o == d->maxtools && d->menuId != -1 ) {
	switch ( e->type() ) {
	case QEvent::MouseButtonPress:
	    {
		QMenuBar* b = (QMenuBar*)o->parent();
		if ( !t )
		    t = new QTime;
		if ( tc != this || t->elapsed() > QApplication::doubleClickInterval() ) {
		    if ( QApplication::reverseLayout() ) {
			QPoint p = b->mapToGlobal( QPoint( b->x() + b->width(), b->y() + b->height() ) );
			p.rx() -= d->popup->sizeHint().width();
			popupOperationMenu( p );
		    } else {
			popupOperationMenu( b->mapToGlobal( QPoint( b->x(), b->y() + b->height() ) ) );
		    }
		    t->start();
		    tc = this;
		} else {
		    tc = 0;
		    closeActiveWindow();
		}
		return TRUE;
	    }
	default:
	    break;
	}
	return QWidget::eventFilter( o, e );
    }
#endif
    switch ( e->type() ) {
    case QEvent::Hide:
    case QEvent::HideToParent:
	if ( !o->isA( "QWorkspaceChild" ) || !isVisible() || e->spontaneous() )
	    break;
	if ( d->active == o ) {
	    int a = d->focus.find( d->active );
	    for ( ;; ) {
		if ( --a < 0 )
		    a = d->focus.count()-1;
		QWorkspaceChild* c = d->focus.at( a );
		if ( !c || c == o ) {
		    if ( c && c->iconw && d->icons.contains( c->iconw->parentWidget() ) )
			break;
		    activateWindow( 0 );
		    break;
		}
		if ( c->isShown() ) {
		    activateWindow( c->windowWidget(), FALSE );
		    break;
		}
	    }
	}
	d->focus.removeRef( (QWorkspaceChild*)o );
	if ( d->maxWindow == o && d->maxWindow->isHidden() ) {
	    d->maxWindow->setGeometry( d->maxRestore );
	    d->maxWindow = 0;
	    if ( d->active )
		maximizeWindow( d->active );

	    if ( !d->maxWindow ) {

		if ( style().styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, this)) {
		    QWorkspaceChild *wc = (QWorkspaceChild *)o;
		    wc->widgetResizeHandler->setActive( TRUE );
		    if ( wc->titlebar )
			wc->titlebar->setMovable( TRUE );
		} else {
		    hideMaximizeControls();
		}
#ifndef QT_NO_WIDGET_TOPEXTRA
		inCaptionChange = TRUE;
		if ( !!d->topCaption )
		    topLevelWidget()->setCaption( d->topCaption );
		inCaptionChange = FALSE;
#endif
	    }
	}
	updateWorkspace();
	break;
    case QEvent::Show:
	if ( o->isA("QWorkspaceChild") && !d->focus.containsRef( (QWorkspaceChild*)o ) )
	    d->focus.append( (QWorkspaceChild*)o );
	updateWorkspace();
	break;
    case QEvent::CaptionChange:
	if ( inCaptionChange )
	    break;

#ifndef QT_NO_WIDGET_TOPEXTRA
	inCaptionChange = TRUE;
	if ( o == topLevelWidget() ) {
	    QWidget *tlw = (QWidget*)o;
	    if ( !d->maxWindow
		|| tlw->caption() != tr("%1 - [%2]").arg(d->topCaption).arg(d->maxWindow->caption()) )
		d->topCaption = tlw->caption();
	}

	if ( d->maxWindow && !!d->topCaption )
	    topLevelWidget()->setCaption( tr("%1 - [%2]")
		.arg(d->topCaption).arg(d->maxWindow->caption()));
	inCaptionChange = FALSE;
#endif

	break;
    case QEvent::Close:
	if ( o == topLevelWidget() )
	{
	    QPtrListIterator<QWorkspaceChild> it( d->windows );
	    while ( it.current () ) {
		QWorkspaceChild* c = it.current();
		++it;
		if ( c->shademode )
		    c->showShaded();
	    }
	} else if ( ::qt_cast<QWorkspaceChild*>(o) ) {
	    d->popup->hide();
	}
	updateWorkspace();
	break;
    default:
	break;
    }
    return QWidget::eventFilter( o, e);
}

void QWorkspace::showMaximizeControls()
{
#ifndef QT_NO_MENUBAR
    Q_ASSERT(d->maxWindow);
    QMenuBar* b = 0;

    // Do a breadth-first search first on every parent,
    QWidget* w = parentWidget();
    QObjectList * l = 0;
    while ( !l && w ) {
	l = w->queryList( "QMenuBar", 0, FALSE, FALSE );
	w = w->parentWidget();
	if ( l && !l->count() ) {
	    delete l;
	    l = 0;
	}
    }

    // and query recursively if nothing is found.
    if ( !l || !l->count() ) {
	if ( l )
	    delete l;
	l = topLevelWidget()->queryList( "QMenuBar", 0, 0, TRUE );
    }
    if ( l && l->count() )
	b = (QMenuBar *)l->first();
    delete l;

    if ( !b )
	return;

    if ( !d->maxcontrols ) {
	d->maxmenubar = b;
	d->maxcontrols = new QFrame( topLevelWidget(), "qt_maxcontrols" );
	QHBoxLayout* l = new QHBoxLayout( d->maxcontrols,
					  d->maxcontrols->frameWidth(), 0 );
	if ( d->maxWindow->windowWidget() &&
	     d->maxWindow->windowWidget()->testWFlags(WStyle_Minimize) ) {
	    QToolButton* iconB = new QToolButton( d->maxcontrols, "iconify" );
#ifndef QT_NO_TOOLTIP
	    QToolTip::add( iconB, tr( "Minimize" ) );
#endif
	    l->addWidget( iconB );
	    iconB->setFocusPolicy( NoFocus );
	    iconB->setIconSet(style().stylePixmap(QStyle::SP_TitleBarMinButton));
	    iconB->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	    connect( iconB, SIGNAL( clicked() ),
		     this, SLOT( minimizeActiveWindow() ) );
	}

	QToolButton* restoreB = new QToolButton( d->maxcontrols, "restore" );
#ifndef QT_NO_TOOLTIP
	QToolTip::add( restoreB, tr( "Restore Down" ) );
#endif
	l->addWidget( restoreB );
	restoreB->setFocusPolicy( NoFocus );
	restoreB->setIconSet( style().stylePixmap(QStyle::SP_TitleBarNormalButton));
	restoreB->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	connect( restoreB, SIGNAL( clicked() ),
		 this, SLOT( normalizeActiveWindow() ) );

	l->addSpacing( 2 );
	QToolButton* closeB = new QToolButton( d->maxcontrols, "close" );
#ifndef QT_NO_TOOLTIP
	QToolTip::add( closeB, tr( "Close" ) );
#endif
	l->addWidget( closeB );
	closeB->setFocusPolicy( NoFocus );
	closeB->setIconSet( style().stylePixmap(QStyle::SP_TitleBarCloseButton) );
	closeB->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	connect( closeB, SIGNAL( clicked() ),
		 this, SLOT( closeActiveWindow() ) );

	d->maxcontrols->setFixedSize( d->maxcontrols->minimumSizeHint() );
    }

    if ( d->controlId == -1 || b->indexOf( d->controlId ) == -1 ) {
	QFrame* dmaxcontrols = d->maxcontrols;
	d->controlId = b->insertItem( dmaxcontrols, -1, b->count() );
    }
    if ( !d->active && d->becomeActive ) {
	d->active = (QWorkspaceChild*)d->becomeActive->parentWidget();
	d->active->setActive( TRUE );
	d->becomeActive = 0;
	emit windowActivated( d->active->windowWidget() );
    }
    if ( d->active && ( d->menuId == -1 || b->indexOf( d->menuId ) == -1 ) ) {
	if ( !d->maxtools ) {
	    d->maxtools = new QLabel( topLevelWidget(), "qt_maxtools" );
	    d->maxtools->installEventFilter( this );
	}
#ifndef QT_NO_WIDGET_TOPEXTRA
	if ( d->active->windowWidget() && d->active->windowWidget()->icon() ) {
	    QPixmap pm(*d->active->windowWidget()->icon());
	    int iconSize = d->maxcontrols->size().height();
	    if(pm.width() > iconSize || pm.height() > iconSize) {
		QImage im;
		im = pm;
		pm = im.smoothScale( QMIN(iconSize, pm.width()), QMIN(iconSize, pm.height()) );
	    }
	    d->maxtools->setPixmap( pm );
	} else
#endif
	{
	    QPixmap pm(14,14);
	    pm.fill( color1 );
	    pm.setMask(pm.createHeuristicMask());
	    d->maxtools->setPixmap( pm );
	}
	d->menuId = b->insertItem( d->maxtools, -1, 0 );
    }
#endif
}


void QWorkspace::hideMaximizeControls()
{
#ifndef QT_NO_MENUBAR
    if ( d->maxmenubar ) {
	int mi = d->menuId;
	if ( mi != -1 ) {
	    if ( d->maxmenubar->indexOf( mi ) != -1 )
		d->maxmenubar->removeItem( mi );
	    d->maxtools = 0;
	}
	int ci = d->controlId;
	if ( ci != -1 && d->maxmenubar->indexOf( ci ) != -1 )
	    d->maxmenubar->removeItem( ci );
    }
    d->maxcontrols = 0;
    d->menuId = -1;
    d->controlId = -1;
#endif
}

/*!
    Closes the child window that is currently active.

    \sa closeAllWindows()
*/
void QWorkspace::closeActiveWindow()
{
    setUpdatesEnabled( FALSE );
    if ( d->maxWindow && d->maxWindow->windowWidget() )
	d->maxWindow->windowWidget()->close();
    else if ( d->active && d->active->windowWidget() )
	d->active->windowWidget()->close();
    setUpdatesEnabled( TRUE );
    updateWorkspace();
}

/*!
    Closes all child windows.

    The windows are closed in random order. The operation stops if a
    window does not accept the close event.

    \sa closeActiveWindow()
*/
void QWorkspace::closeAllWindows()
{
    bool did_close = TRUE;
    QPtrListIterator<QWorkspaceChild> it( d->windows );
    QWorkspaceChild *c = 0;
    while ( ( c = it.current() ) && did_close ) {
	++it;
	if ( c->windowWidget() )
	    did_close = c->windowWidget()->close();
    }
}

void QWorkspace::normalizeActiveWindow()
{
    if  ( d->maxWindow )
	d->maxWindow->showNormal();
    else if ( d->active )
	d->active->showNormal();
}

void QWorkspace::minimizeActiveWindow()
{
    if ( d->maxWindow )
	d->maxWindow->showMinimized();
    else if ( d->active )
	d->active->showMinimized();
}

void QWorkspace::showOperationMenu()
{
    if  ( !d->active || !d->active->windowWidget() )
	return;
    Q_ASSERT( d->active->windowWidget()->testWFlags( WStyle_SysMenu ) );
    QPoint p;
    QPopupMenu *popup = d->active->windowWidget()->testWFlags( WStyle_Tool ) ? d->toolPopup : d->popup;
    if ( QApplication::reverseLayout() ) {
	p = QPoint( d->active->windowWidget()->mapToGlobal( QPoint(d->active->windowWidget()->width(),0) ) );
	p.rx() -= popup->sizeHint().width();
    } else {
	p = QPoint( d->active->windowWidget()->mapToGlobal( QPoint(0,0) ) );
    }
    if ( !d->active->isVisible() ) {
	p = d->active->iconWidget()->mapToGlobal( QPoint(0,0) );
	p.ry() -= popup->sizeHint().height();
    }
    popupOperationMenu( p );
}

void QWorkspace::popupOperationMenu( const QPoint&  p)
{
    if ( !d->active || !d->active->windowWidget() || !d->active->windowWidget()->testWFlags( WStyle_SysMenu ) )
	return;
    if ( d->active->windowWidget()->testWFlags( WStyle_Tool ))
	d->toolPopup->popup( p );
    else
	d->popup->popup( p );
}

void QWorkspace::operationMenuAboutToShow()
{
    for ( int i = 1; i < 6; i++ ) {
	bool enable = d->active != 0;
	d->popup->setItemEnabled( i, enable );
    }

    if ( !d->active || !d->active->windowWidget() )
	return;

    QWidget *windowWidget = d->active->windowWidget();
    bool canResize = windowWidget->maximumSize() != windowWidget->minimumSize();
    d->popup->setItemEnabled( 3, canResize );
    d->popup->setItemEnabled( 4, windowWidget->testWFlags( WStyle_Minimize ) );
    d->popup->setItemEnabled( 5, windowWidget->testWFlags( WStyle_Maximize ) && canResize );

    if ( d->active == d->maxWindow ) {
	d->popup->setItemEnabled( 2, FALSE );
	d->popup->setItemEnabled( 3, FALSE );
	d->popup->setItemEnabled( 5, FALSE );
    } else if ( d->active->isVisible() ){
	d->popup->setItemEnabled( 1, FALSE );
    } else {
	d->popup->setItemEnabled( 2, FALSE );
	d->popup->setItemEnabled( 3, FALSE );
	d->popup->setItemEnabled( 4, FALSE );
    }
}

void QWorkspace::toolMenuAboutToShow()
{
    if ( !d->active || !d->active->windowWidget() )
	return;

    QWidget *windowWidget = d->active->windowWidget();
    bool canResize = windowWidget->maximumSize() != windowWidget->minimumSize();

    d->toolPopup->setItemEnabled( 3, !d->active->shademode && canResize );
    if ( d->active->shademode )
	d->toolPopup->changeItem( 6,
				  QIconSet(style().stylePixmap(QStyle::SP_TitleBarUnshadeButton)), tr("&Unshade") );
    else
	d->toolPopup->changeItem( 6, QIconSet(style().stylePixmap(QStyle::SP_TitleBarShadeButton)), tr("Sh&ade") );
    d->toolPopup->setItemEnabled( 6, d->active->windowWidget()->testWFlags( WStyle_MinMax ) );
    d->toolPopup->setItemChecked( 7, d->active->windowWidget()->testWFlags( WStyle_StaysOnTop ) );
}

void QWorkspace::operationMenuActivated( int a )
{
    if ( !d->active )
	return;
    switch ( a ) {
    case 1:
	d->active->showNormal();
	break;
    case 2:
	d->active->doMove();
	break;
    case 3:
	if ( d->active->shademode )
	    d->active->showShaded();
	d->active->doResize();
	break;
    case 4:
	d->active->showMinimized();
	break;
    case 5:
	d->active->showMaximized();
	break;
    case 6:
	d->active->showShaded();
	break;
    case 7:
	{
	    QWorkspace* w = (QWorkspace*)d->active->windowWidget();
	    if ( !w )
		break;
	    if ( w->testWFlags( WStyle_StaysOnTop ) ) {
		w->clearWFlags( WStyle_StaysOnTop );
	    } else {
		w->setWFlags( WStyle_StaysOnTop );
		w->parentWidget()->raise();
	    }
	}
	break;
    default:
	break;
    }
}

/*!
    Activates the next window in the child window chain.

    \sa activatePrevWindow()
*/
void QWorkspace::activateNextWindow()
{
    if ( d->focus.isEmpty() )
	return;
    if ( !d->active ) {
	if ( d->focus.first() )
	    activateWindow( d->focus.first()->windowWidget(), FALSE );
	return;
    }

    int a = d->focus.find( d->active ) + 1;

    a = a % d->focus.count();

    if ( d->focus.at( a ) )
	activateWindow( d->focus.at( a )->windowWidget(), FALSE );
    else
	activateWindow(0);
}

void QWorkspace::activatePreviousWindow()
{
    activatePrevWindow();
}

/*!
    Activates the previous window in the child window chain.

    \sa activateNextWindow()
*/
void QWorkspace::activatePrevWindow()
{
    if ( d->focus.isEmpty() )
	return;
    if ( !d->active ) {
	if ( d->focus.last() )
	    activateWindow( d->focus.first()->windowWidget(), FALSE );
	else
	    activateWindow( 0 );

	return;
    }

    int a = d->focus.find( d->active ) - 1;
    if ( a < 0 )
	a = d->focus.count()-1;

    if ( d->focus.at( a ) )
	activateWindow( d->focus.at( a )->windowWidget(), FALSE );
    else
	activateWindow( 0 );
}


/*!
  \fn void QWorkspace::windowActivated( QWidget* w )

  This signal is emitted when the window widget \a w becomes active.
  Note that \a w can be null, and that more than one signal may be
  emitted for a single activation event.

  \sa activeWindow(), windowList()
*/



/*!
    Arranges all the child windows in a cascade pattern.

    \sa tile()
*/
void QWorkspace::cascade()
{
    blockSignals(TRUE);
    if  ( d->maxWindow )
	d->maxWindow->showNormal();

    if ( d->vbar ) {
	d->vbar->blockSignals( TRUE );
	d->vbar->setValue( 0 );
	d->vbar->blockSignals( FALSE );
	d->hbar->blockSignals( TRUE );
	d->hbar->setValue( 0 );
	d->hbar->blockSignals( FALSE );
	scrollBarChanged();
    }

    const int xoffset = 13;
    const int yoffset = 20;

    // make a list of all relevant mdi clients
    QPtrList<QWorkspaceChild> widgets;
    QWorkspaceChild* wc = 0;
    for ( wc = d->focus.first(); wc; wc = d->focus.next() )
	if ( wc->windowWidget()->isVisibleTo( this ) && !wc->windowWidget()->testWFlags( WStyle_Tool ) && !wc->iconw )
	    widgets.append( wc );

    int x = 0;
    int y = 0;

    setUpdatesEnabled( FALSE );
    QPtrListIterator<QWorkspaceChild> it( widgets );
    while ( it.current () ) {
	QWorkspaceChild *child = it.current();
	++it;
	child->setUpdatesEnabled( FALSE );
	QSize prefSize = child->windowWidget()->sizeHint().expandedTo( child->windowWidget()->minimumSizeHint() );

	if ( !prefSize.isValid() )
	    prefSize = child->windowWidget()->size();
	prefSize = prefSize.expandedTo( child->windowWidget()->minimumSize() ).boundedTo( child->windowWidget()->maximumSize() );
	if (prefSize.isValid())
	    prefSize += QSize( child->baseSize().width(), child->baseSize().height() );

	int w = prefSize.width();
	int h = prefSize.height();

	child->showNormal();
	qApp->sendPostedEvents( 0, QEvent::ShowNormal );
	if ( y + h > height() )
	    y = 0;
	if ( x + w > width() )
	    x = 0;
	child->setGeometry( x, y, w, h );
	x += xoffset;
	y += yoffset;
	child->internalRaise();
	child->setUpdatesEnabled( TRUE );
    }
    setUpdatesEnabled( TRUE );
    updateWorkspace();
    blockSignals(FALSE);
}

/*!
    Arranges all child windows in a tile pattern.

    \sa cascade()
*/
void QWorkspace::tile()
{
    blockSignals(TRUE);
    QWidget *oldActive = d->active ? d->active->windowWidget() : 0;
    if  ( d->maxWindow )
	d->maxWindow->showNormal();

    if ( d->vbar ) {
	d->vbar->blockSignals( TRUE );
	d->vbar->setValue( 0 );
	d->vbar->blockSignals( FALSE );
	d->hbar->blockSignals( TRUE );
	d->hbar->setValue( 0 );
	d->hbar->blockSignals( FALSE );
	scrollBarChanged();
    }

    int rows = 1;
    int cols = 1;
    int n = 0;
    QWorkspaceChild* c;

    QPtrListIterator<QWorkspaceChild> it( d->windows );
    while ( it.current () ) {
	c = it.current();
	++it;
	if ( !c->windowWidget()->isHidden() &&
	     !c->windowWidget()->testWFlags( WStyle_StaysOnTop ) &&
	     !c->windowWidget()->testWFlags( WStyle_Tool ) &&
             !c->iconw )
	    n++;
    }

    while ( rows * cols < n ) {
	if ( cols <= rows )
	    cols++;
	else
	    rows++;
    }
    int add = cols * rows - n;
    bool* used = new bool[ cols*rows ];
    for ( int i = 0; i < rows*cols; i++ )
	used[i] = FALSE;

    int row = 0;
    int col = 0;
    int w = width() / cols;
    int h = height() / rows;

    it.toFirst();
    while ( it.current () ) {
	c = it.current();
	++it;
	if ( c->iconw || c->windowWidget()->isHidden() || c->windowWidget()->testWFlags( WStyle_Tool ) )
	    continue;
        if (!row && !col) {
            w -= c->baseSize().width();
            h -= c->baseSize().height();
        }
	if ( c->windowWidget()->testWFlags( WStyle_StaysOnTop ) ) {
	    QPoint p = c->pos();
	    if ( p.x()+c->width() < 0 )
		p.setX( 0 );
	    if ( p.x() > width() )
		p.setX( width() - c->width() );
	    if ( p.y() + 10 < 0 )
		p.setY( 0 );
	    if ( p.y() > height() )
		p.setY( height() - c->height() );

	    if ( p != c->pos() )
		c->QFrame::move( p );
	} else {
	    c->showNormal();
	    qApp->sendPostedEvents( 0, QEvent::ShowNormal );
	    used[row*cols+col] = TRUE;
            QSize sz(w, h);
            QSize bsize(c->baseSize());
            sz = sz.expandedTo(c->windowWidget()->minimumSize()).boundedTo(c->windowWidget()->maximumSize());
            sz += bsize;

	    if ( add ) {
                if (sz.height() == h + bsize.height()) // no relevant constrains
                    sz.rheight() *= 2;
		c->setGeometry(col*w + col*bsize.width(), row*h + row*bsize.height(), sz.width(), sz.height());
		used[(row+1)*cols+col] = TRUE;
		add--;
	    } else {
		c->setGeometry(col*w + col*bsize.width(), row*h + row*bsize.height(), sz.width(), sz.height());
	    }
	    while( row < rows && col < cols && used[row*cols+col] ) {
		col++;
		if ( col == cols ) {
		    col = 0;
		    row++;
		}
	    }
        }
    }
    delete [] used;

    activateWindow( oldActive );
    updateWorkspace();
    blockSignals(FALSE);
}

QWorkspaceChild::QWorkspaceChild( QWidget* window, QWorkspace *parent,
				  const char *name )
    : QFrame( parent, name,
	      (parent->windowMode() == QWorkspace::TopLevel ? (WStyle_MinMax | WStyle_SysMenu | WType_TopLevel) :
	       WStyle_NoBorder ) | WStyle_Customize | WDestructiveClose | WNoMousePropagation | WSubWindow )
{
    statusbar = 0;
    setMouseTracking( TRUE );
    act = FALSE;
    iconw = 0;
    lastfocusw = 0;
    shademode = FALSE;
    titlebar = 0;
    snappedRight = FALSE;
    snappedDown = FALSE;

    if (window) {
	switch (window->focusPolicy()) {
	case QWidget::NoFocus:
	    window->setFocusPolicy(QWidget::ClickFocus);
	    break;
	case QWidget::TabFocus:
	    window->setFocusPolicy(QWidget::StrongFocus);
	    break;
	default:
	    break;
	}
    }

    if ( window && window->testWFlags( WStyle_Title ) && parent->windowMode() != QWorkspace::TopLevel ) {
	titlebar = new QTitleBar( window, this, "qt_ws_titlebar" );
	connect( titlebar, SIGNAL( doActivate() ),
		 this, SLOT( activate() ) );
	connect( titlebar, SIGNAL( doClose() ),
		 window, SLOT( close() ) );
	connect( titlebar, SIGNAL( doMinimize() ),
		 this, SLOT( showMinimized() ) );
	connect( titlebar, SIGNAL( doNormal() ),
		 this, SLOT( showNormal() ) );
	connect( titlebar, SIGNAL( doMaximize() ),
		 this, SLOT( showMaximized() ) );
	connect( titlebar, SIGNAL( popupOperationMenu(const QPoint&) ),
		 this, SIGNAL( popupOperationMenu(const QPoint&) ) );
	connect( titlebar, SIGNAL( showOperationMenu() ),
		 this, SIGNAL( showOperationMenu() ) );
	connect( titlebar, SIGNAL( doShade() ),
		 this, SLOT( showShaded() ) );
	connect( titlebar, SIGNAL( doubleClicked() ),
		 this, SLOT( titleBarDoubleClicked() ) );
    }

    setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    setLineWidth( style().pixelMetric(QStyle::PM_MDIFrameWidth, this) );
    setMinimumSize( 128, 0 );

    childWidget = window;
    if (!childWidget)
	return;

#ifndef QT_NO_WIDGET_TOPEXTRA
    setCaption( childWidget->caption() );
#endif

    QPoint p;
    QSize s;
    QSize cs;

    bool hasBeenResized = childWidget->testWState( WState_Resized );

    if ( !hasBeenResized )
	cs = childWidget->sizeHint().expandedTo( childWidget->minimumSizeHint() );
    else
	cs = childWidget->size();

    int th = titlebar ? titlebar->sizeHint().height() : 0;
    if ( titlebar ) {
#ifndef QT_NO_WIDGET_TOPEXTRA
	int iconSize = th;
	if( childWidget->icon() ) {
	    QPixmap pm(*childWidget->icon());
	    if(pm.width() > iconSize || pm.height() > iconSize) {
		QImage im;
		im = pm;
		pm = im.smoothScale( QMIN(iconSize, pm.width()), QMIN(iconSize, pm.height()) );
	    }
	    titlebar->setIcon( pm );
	}
#endif
	if ( !style().styleHint( QStyle::SH_TitleBar_NoBorder, titlebar ) )
	    th += frameWidth();
	else
	    th -= contentsRect().y();

	p = QPoint( contentsRect().x(),
		    th + contentsRect().y() );
	s = QSize( cs.width() + 2*frameWidth(),
		   cs.height() + 2*frameWidth() + th );
    } else {
	p = QPoint( contentsRect().x(), contentsRect().y() );
	s = QSize( cs.width() + 2*frameWidth(),
		   cs.height() + 2*frameWidth() );
    }

    childWidget->reparent( this, p);
    resize( s );

    childWidget->installEventFilter( this );

    widgetResizeHandler = new QWidgetResizeHandler( this, window );
    widgetResizeHandler->setSizeProtection( !parent->scrollBarsEnabled() );
    connect( widgetResizeHandler, SIGNAL( activate() ),
	     this, SLOT( activate() ) );
    if ( !style().styleHint( QStyle::SH_TitleBar_NoBorder, titlebar ) )
	widgetResizeHandler->setExtraHeight( th + contentsRect().y() - 2*frameWidth() );
    else
	widgetResizeHandler->setExtraHeight( th + contentsRect().y() - frameWidth() );
    if(parent->windowMode() == QWorkspace::TopLevel && isTopLevel()) {
	move(0, 0);
	widgetResizeHandler->setActive( FALSE );
    }
    if ( childWidget->minimumSize() == childWidget->maximumSize() )
	widgetResizeHandler->setActive( QWidgetResizeHandler::Resize, FALSE );
    setBaseSize( baseSize() );
}

QWorkspaceChild::~QWorkspaceChild()
{
    if ( iconw )
	delete iconw->parentWidget();
    QWorkspace *workspace = ::qt_cast<QWorkspace*>(parentWidget());
    if ( workspace ) {
        workspace->d->focus.removeRef(this);
	if ( workspace->d->active == this ) {
	    workspace->activatePrevWindow();
            if (workspace->d->active == this) {
                workspace->activateWindow(0);
            }
        }
	if ( workspace->d->maxWindow == this ) {
	    workspace->hideMaximizeControls();
	    workspace->d->maxWindow = 0;
	}
    }
}

bool QWorkspaceChild::event( QEvent *e )
{
    if(((QWorkspace*)parentWidget())->windowMode() == QWorkspace::TopLevel) {
	switch(e->type()) {
	case QEvent::Close:
	    if(windowWidget()) {
		if(!windowWidget()->close()) {
		    if(((QWorkspace*) parentWidget() )->d->active == this)
			((QWorkspace*) parentWidget() )->activatePrevWindow();
		    return TRUE;
		}
	    }
	    break;
#if 0
	case QEvent::WindowDeactivate:
	    if(statusbar) {
		QSize newsize(width(), height() - statusbar->height());
		if(statusbar->parentWidget() == this)
		    statusbar->hide();
		statusbar = 0;
		resize(newsize);
	    }
	    break;
#endif
	case QEvent::WindowActivate:
	    if(((QWorkspace*)parentWidget())->activeWindow() == windowWidget())
		activate();
	    if(statusbar)
		statusbar->show();
	    else if(((QWorkspace*) parentWidget() )->d->mainwindow)
		setStatusBar(((QWorkspace*) parentWidget() )->d->mainwindow->statusBar());
	    break;
	default:
	    break;
	}
    }
    return QWidget::event(e);
}

void QWorkspaceChild::setStatusBar( QStatusBar *sb )
{
    if(((QWorkspace*) parentWidget() )->windowMode() == QWorkspace::TopLevel) {
	QSize newsize;
	if(sb) {
	    sb->show();
	    if(sb != statusbar) {
		sb->reparent(this, QPoint(0, height()), TRUE);
		newsize = QSize(width(), height() + sb->height());
	    }
	}
	statusbar = sb;
	if(!newsize.isNull())
	    resize(newsize);
    }
}

void QWorkspaceChild::moveEvent( QMoveEvent *e )
{
    if(((QWorkspace*) parentWidget() )->windowMode() == QWorkspace::TopLevel && !e->spontaneous()) {
	QPoint p = parentWidget()->topLevelWidget()->pos();
	if(x() < p.x() || y() < p.y())
	    move(QMAX(x(), p.x()), QMAX(y(), p.y()));
    }
    ((QWorkspace*) parentWidget() )->updateWorkspace();
}

void QWorkspaceChild::resizeEvent( QResizeEvent * )
{
    QRect r = contentsRect();
    QRect cr;

    if ( titlebar ) {
	int th = titlebar->sizeHint().height();
	QRect tbrect( 0, 0, width(), th );
	if ( !style().styleHint( QStyle::SH_TitleBar_NoBorder ) )
	    tbrect = QRect( r.x(), r.y(), r.width(), th );
	titlebar->setGeometry( tbrect );

	if ( style().styleHint( QStyle::SH_TitleBar_NoBorder, titlebar ) )
	    th -= frameWidth();
	cr = QRect( r.x(), r.y() + th + (shademode ? (frameWidth() * 3) : 0),
		    r.width(), r.height() - th );
    } else {
	cr = r;
    }

    if(statusbar && statusbar->isVisible()) {
	int sh = statusbar->height();
	statusbar->setGeometry(r.x(), r.bottom() - sh, r.width(), sh);
	cr.setBottom(cr.bottom() - sh);
    }

    if (!childWidget)
	return;

    windowSize = cr.size();
    childWidget->setGeometry( cr );
    ((QWorkspace*) parentWidget() )->updateWorkspace();
}

QSize QWorkspaceChild::baseSize() const
{
    int th = titlebar ? titlebar->sizeHint().height() : 0;
    if ( style().styleHint( QStyle::SH_TitleBar_NoBorder, titlebar ) )
	th -= frameWidth();
    return QSize( 2*frameWidth(), 2*frameWidth() + th );
}

QSize QWorkspaceChild::sizeHint() const
{
    if ( !childWidget )
	return QFrame::sizeHint() + baseSize();

    QSize prefSize = windowWidget()->sizeHint().expandedTo( windowWidget()->minimumSizeHint() );
    prefSize = prefSize.expandedTo( windowWidget()->minimumSize() ).boundedTo( windowWidget()->maximumSize() );
    prefSize += baseSize();

    return prefSize;
}

QSize QWorkspaceChild::minimumSizeHint() const
{
    if ( !childWidget )
	return QFrame::minimumSizeHint() + baseSize();
    QSize s = childWidget->minimumSize();
    if ( s.isEmpty() )
	s = childWidget->minimumSizeHint();
    return s + baseSize();
}

void QWorkspaceChild::activate()
{
    ((QWorkspace*)parentWidget())->activateWindow( windowWidget() );
}

bool QWorkspaceChild::eventFilter( QObject * o, QEvent * e)
{
    if ( !isActive() && ( e->type() == QEvent::MouseButtonPress ||
	e->type() == QEvent::FocusIn ) ) {
	if ( iconw ) {
	    ((QWorkspace*)parentWidget())->normalizeWindow( windowWidget() );
	    if ( iconw ) {
		((QWorkspace*)parentWidget())->removeIcon( iconw->parentWidget() );
		delete iconw->parentWidget();
		iconw = 0;
	    }
	}
	activate();
    }

    // for all widgets except the window, that's the only thing we
    // process, and if we have no childWidget we skip totally
    if ( o != childWidget || childWidget == 0 )
	return FALSE;

    switch ( e->type() ) {
    case QEvent::Show:
	if ( ((QWorkspace*)parentWidget())->d->focus.find( this ) < 0 )
	    ((QWorkspace*)parentWidget())->d->focus.append( this );
	if ( isVisibleTo( parentWidget() ) )
	    break;
	if (( (QShowEvent*)e)->spontaneous() )
	    break;
	// fall through
    case QEvent::ShowToParent:
	if ( windowWidget() && windowWidget()->testWFlags( WStyle_StaysOnTop ) ) {
	    internalRaise();
	    show();
	}
	((QWorkspace*)parentWidget())->showWindow( windowWidget() );
	break;
    case QEvent::ShowMaximized:
	if ( windowWidget()->maximumSize().isValid() &&
	     ( windowWidget()->maximumWidth() < parentWidget()->width() ||
	       windowWidget()->maximumHeight() < parentWidget()->height() ) ) {
	    windowWidget()->resize( windowWidget()->maximumSize() );
            ((QWorkspace*)windowWidget())->clearWState(WState_Maximized);
            if (titlebar)
                titlebar->repaint(FALSE);
	    break;
	}
	if ( windowWidget()->testWFlags( WStyle_Maximize ) && !windowWidget()->testWFlags( WStyle_Tool ) )
	    ((QWorkspace*)parentWidget())->maximizeWindow( windowWidget() );
	else
	    ((QWorkspace*)parentWidget())->normalizeWindow( windowWidget() );
	break;
    case QEvent::ShowMinimized:
	((QWorkspace*)parentWidget())->minimizeWindow( windowWidget() );
	break;
    case QEvent::ShowNormal:
	((QWorkspace*)parentWidget())->normalizeWindow( windowWidget() );
	if (iconw) {
	    ((QWorkspace*)parentWidget())->removeIcon( iconw->parentWidget() );
	    delete iconw->parentWidget();
	}
	break;
    case QEvent::Hide:
    case QEvent::HideToParent:
	if ( !childWidget->isVisibleTo( this ) ) {
	    QWidget * w = iconw;
	    if ( w && ( w = w->parentWidget() ) ) {
		((QWorkspace*)parentWidget())->removeIcon( w );
		delete w;
	    }
	    hide();
	}
	break;
    case QEvent::CaptionChange:
#ifndef QT_NO_WIDGET_TOPEXTRA
	setCaption( childWidget->caption() );
	if ( iconw )
	    iconw->setCaption( childWidget->caption() );
#endif
	break;
    case QEvent::IconChange:
	{
	    QWorkspace* ws = (QWorkspace*)parentWidget();
	    if ( !titlebar )
		break;

	    QPixmap pm;
	    int iconSize = titlebar->size().height();
#ifndef QT_NO_WIDGET_TOPEXTRA
	    if ( childWidget->icon() ) {
		pm = *childWidget->icon();
		if(pm.width() > iconSize || pm.height() > iconSize) {
		    QImage im;
		    im = pm;
		    pm = im.smoothScale( QMIN(iconSize, pm.width()), QMIN(iconSize, pm.height()) );
		}
	    } else
#endif
	    {
		pm.resize( iconSize, iconSize );
		pm.fill( color1 );
		pm.setMask(pm.createHeuristicMask());
	    }
	    titlebar->setIcon( pm );
	    if ( iconw )
		iconw->setIcon( pm );

	    if ( ws->d->maxWindow != this )
		break;

	    if ( ws->d->maxtools )
		ws->d->maxtools->setPixmap( pm );
	}
	break;
    case QEvent::Resize:
	{
	    QResizeEvent* re = (QResizeEvent*)e;
	    if ( re->size() != windowSize && !shademode )
		resize( re->size() + baseSize() );
	}
	break;

    case QEvent::WindowDeactivate:
	if ( titlebar )
	    titlebar->setActive( FALSE );
	repaint( FALSE );
	break;

    case QEvent::WindowActivate:
	if ( titlebar )
	    titlebar->setActive( act );
	repaint( FALSE );
	break;

    default:
	break;
    }

    return QFrame::eventFilter(o, e);
}

bool QWorkspaceChild::focusNextPrevChild( bool next )
{
    QFocusData *f = focusData();

    QWidget *startingPoint = f->home();
    QWidget *candidate = 0;
    QWidget *w = next ? f->next() : f->prev();
    while( !candidate && w != startingPoint ) {
	if ( w != startingPoint &&
	     (w->focusPolicy() & TabFocus) == TabFocus
	     && w->isEnabled() &&!w->focusProxy() && w->isVisible() )
	    candidate = w;
	w = next ? f->next() : f->prev();
    }

    if ( candidate ) {
	QObjectList *ol = queryList();
	bool ischild = ol->findRef( candidate ) != -1;
	delete ol;
	if ( !ischild ) {
	    startingPoint = f->home();
	    QWidget *nw = next ? f->prev() : f->next();
	    QObjectList *ol2 = queryList();
	    QWidget *lastValid = 0;
	    candidate = startingPoint;
	    while ( nw != startingPoint ) {
		if ( ( candidate->focusPolicy() & TabFocus ) == TabFocus
		    && candidate->isEnabled() &&!candidate->focusProxy() && candidate->isVisible() )
		    lastValid = candidate;
		if ( ol2->findRef( nw ) == -1 ) {
		    candidate = lastValid;
		    break;
		}
		candidate = nw;
		nw = next ? f->prev() : f->next();
	    }
	    delete ol2;
	}
    }

    if ( !candidate )
	return FALSE;

    candidate->setFocus();
    return TRUE;
}

void QWorkspaceChild::childEvent( QChildEvent*  e)
{
    if ( e->type() == QEvent::ChildRemoved && e->child() == childWidget ) {
	childWidget = 0;
	if ( iconw ) {
	    ((QWorkspace*)parentWidget())->removeIcon( iconw->parentWidget() );
	    delete iconw->parentWidget();
	}
	close();
    }
}


void QWorkspaceChild::doResize()
{
    widgetResizeHandler->doResize();
}

void QWorkspaceChild::doMove()
{
    widgetResizeHandler->doMove();
}

void QWorkspaceChild::enterEvent( QEvent * )
{
}

void QWorkspaceChild::leaveEvent( QEvent * )
{
#ifndef QT_NO_CURSOR
    if ( !widgetResizeHandler->isButtonDown() )
	setCursor( arrowCursor );
#endif
}

void QWorkspaceChild::drawFrame( QPainter *p )
{
    QStyle::SFlags flags = QStyle::Style_Default;
    QStyleOption opt(lineWidth(),midLineWidth());

    if ( titlebar && titlebar->isActive() )
	flags |= QStyle::Style_Active;

    style().drawPrimitive( QStyle::PE_WindowFrame, p, rect(), colorGroup(), flags, opt );
}

void QWorkspaceChild::styleChange( QStyle & )
{
    resizeEvent( 0 );
    if ( iconw ) {
	QVBox *vbox = (QVBox*)iconw->parentWidget()->qt_cast( "QVBox" );
	Q_ASSERT(vbox);
	if ( !style().styleHint( QStyle::SH_TitleBar_NoBorder ) ) {
	    vbox->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
	    vbox->resize( 196+2*vbox->frameWidth(), 20 + 2*vbox->frameWidth() );
	} else {
	    vbox->resize( 196, 20 );
	}
    }
}

void QWorkspaceChild::setActive( bool b )
{
    if ( !childWidget )
	return;

    bool hasFocus = isChildOf( focusWidget(), childWidget );
    if ( act == b && hasFocus )
	return;

    act = b;

    if ( titlebar )
	titlebar->setActive( act );
    if ( iconw )
	iconw->setActive( act );
    repaint( FALSE );

    QObjectList* ol = childWidget->queryList( "QWidget" );
    if ( act ) {
	QObject *o;
	for ( o = ol->first(); o; o = ol->next() )
	    o->removeEventFilter( this );
	if ( !hasFocus ) {
	    if ( lastfocusw && ol->contains( lastfocusw ) &&
		 lastfocusw->focusPolicy() != NoFocus ) {
		// this is a bug if lastfocusw has been deleted, a new
		// widget has been created, and the new one is a child
		// of the same window as the old one. but even though
		// it's a bug the behaviour is reasonable
		lastfocusw->setFocus();
	    } else if ( childWidget->focusPolicy() != NoFocus ) {
		childWidget->setFocus();
	    } else {
		// find something, anything, that accepts focus, and use that.
		o = ol->first();
		while( o && ((QWidget*)o)->focusPolicy() == NoFocus )
		    o = ol->next();
		if ( o )
		    ((QWidget*)o)->setFocus();
	    }
	}
    } else {
	if ( isChildOf( focusWidget(), childWidget ) )
	    lastfocusw = focusWidget();
	QObject * o;
	for ( o = ol->first(); o; o = ol->next() ) {
	    o->removeEventFilter( this );
	    o->installEventFilter( this );
	}
    }
    delete ol;
}

bool QWorkspaceChild::isActive() const
{
    return act;
}

QWidget* QWorkspaceChild::windowWidget() const
{
    return childWidget;
}


QWidget* QWorkspaceChild::iconWidget() const
{
    if ( !iconw ) {
	QWorkspaceChild* that = (QWorkspaceChild*) this;

	// ### why do we even need the vbox? -Brad
	QVBox* vbox = new QVBox(that, "qt_vbox", WType_TopLevel );
	QTitleBar *tb = new QTitleBar( windowWidget(), vbox, "_workspacechild_icon_");
	int th = style().pixelMetric( QStyle::PM_TitleBarHeight, tb );
	int iconSize = style().pixelMetric( QStyle::PM_MDIMinimizedWidth, this );
	if ( !style().styleHint( QStyle::SH_TitleBar_NoBorder ) ) {
	    vbox->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
	    vbox->resize( iconSize+2*vbox->frameWidth(), th+2*vbox->frameWidth() );
	} else {
	    vbox->resize( iconSize, th );
	}
	that->iconw = tb;
	iconw->setActive( isActive() );

	connect( iconw, SIGNAL( doActivate() ),
		 this, SLOT( activate() ) );
	connect( iconw, SIGNAL( doClose() ),
		 windowWidget(), SLOT( close() ) );
	connect( iconw, SIGNAL( doNormal() ),
		 this, SLOT( showNormal() ) );
	connect( iconw, SIGNAL( doMaximize() ),
		 this, SLOT( showMaximized() ) );
	connect( iconw, SIGNAL( popupOperationMenu(const QPoint&) ),
		 this, SIGNAL( popupOperationMenu(const QPoint&) ) );
	connect( iconw, SIGNAL( showOperationMenu() ),
		 this, SIGNAL( showOperationMenu() ) );
	connect( iconw, SIGNAL( doubleClicked() ),
		 this, SLOT( titleBarDoubleClicked() ) );
    }
#ifndef QT_NO_WIDGET_TOPEXTRA
    if ( windowWidget() ) {
	iconw->setCaption( windowWidget()->caption() );
	if ( windowWidget()->icon() ) {
	    int iconSize = iconw->sizeHint().height();

	    QPixmap pm(*childWidget->icon());
	    if(pm.width() > iconSize || pm.height() > iconSize) {
		QImage im;
		im = pm;
		pm = im.smoothScale( QMIN(iconSize, pm.width()), QMIN(iconSize, pm.height()) );
	    }
	    iconw->setIcon( pm );
	}
    }
#endif
    return iconw->parentWidget();
}

void QWorkspaceChild::showMinimized()
{
    windowWidget()->setWindowState(WindowMinimized | windowWidget()->windowState());
}

void QWorkspaceChild::showMaximized()
{
    windowWidget()->setWindowState(WindowMaximized | (windowWidget()->windowState() & ~WindowMinimized));
}

void QWorkspaceChild::showNormal()
{
    windowWidget()->setWindowState(windowWidget()->windowState() & ~(WindowMinimized|WindowMaximized));
}

void QWorkspaceChild::showShaded()
{
    if ( !titlebar)
	return;
    Q_ASSERT( windowWidget()->testWFlags( WStyle_MinMax ) && windowWidget()->testWFlags( WStyle_Tool ) );
    ((QWorkspace*)parentWidget())->activateWindow( windowWidget() );
    if ( shademode ) {
	QWorkspaceChild* fake = (QWorkspaceChild*)windowWidget();
	fake->clearWState( WState_Minimized );
	clearWState( WState_Minimized );

	shademode = FALSE;
	resize( shadeRestore );
	setMinimumSize( shadeRestoreMin );
	style().polish(this);
    } else {
	shadeRestore = size();
	shadeRestoreMin = minimumSize();
	setMinimumHeight(0);
	shademode = TRUE;
	QWorkspaceChild* fake = (QWorkspaceChild*)windowWidget();
	fake->setWState( WState_Minimized );
	setWState( WState_Minimized );

	if ( style().styleHint( QStyle::SH_TitleBar_NoBorder ) )
	    resize( width(), titlebar->height() );
	else
	    resize( width(), titlebar->height() + 2*lineWidth() + 1 );
	style().polish(this);
    }
    titlebar->update();
}

void QWorkspaceChild::titleBarDoubleClicked()
{
    if ( !windowWidget() )
	return;
    if ( windowWidget()->testWFlags( WStyle_MinMax ) ) {
	if ( windowWidget()->testWFlags( WStyle_Tool ) )
	    showShaded();
	else if ( iconw )
	    showNormal();
	else if ( windowWidget()->testWFlags( WStyle_Maximize ) )
	    showMaximized();
    }
}

void QWorkspaceChild::adjustToFullscreen()
{
    if ( !childWidget )
	return;

    qApp->sendPostedEvents( this, QEvent::Resize );
    qApp->sendPostedEvents( childWidget, QEvent::Resize );
    qApp->sendPostedEvents( childWidget, QEvent::Move );
    if( style().styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, this) ) {
	setGeometry( 0, 0, parentWidget()->width(), parentWidget()->height());
    } else {
	int w = parentWidget()->width() + width() - childWidget->width();
	int h = parentWidget()->height() + height() - childWidget->height();
	w = QMAX( w, childWidget->minimumWidth() );
	h = QMAX( h, childWidget->minimumHeight() );
	setGeometry( -childWidget->x(), -childWidget->y(), w, h );
    }
    setWState( WState_Maximized );
    ((QWorkspaceChild*)childWidget)->setWState( WState_Maximized );
}

void QWorkspaceChild::setCaption( const QString& cap )
{
    if ( titlebar )
	titlebar->setCaption( cap );
#ifndef QT_NO_WIDGET_TOPEXTRA
    QWidget::setCaption( cap );
#endif
}

void QWorkspaceChild::internalRaise()
{
    setUpdatesEnabled( FALSE );
    if ( iconw )
	iconw->parentWidget()->raise();
    raise();

    if ( !windowWidget() || windowWidget()->testWFlags( WStyle_StaysOnTop ) ) {
	setUpdatesEnabled( TRUE );
	return;
    }

    QPtrListIterator<QWorkspaceChild> it( ((QWorkspace*)parent())->d->windows );
    while ( it.current () ) {
     	QWorkspaceChild* c = it.current();
	++it;
	if ( c->windowWidget() &&
	    !c->windowWidget()->isHidden() &&
	     c->windowWidget()->testWFlags( WStyle_StaysOnTop ) )
	     c->raise();
    }
    setUpdatesEnabled( TRUE );
}

void QWorkspaceChild::move( int x, int y )
{
    int nx = x;
    int ny = y;

    if ( windowWidget() && windowWidget()->testWFlags( WStyle_Tool ) ) {
	int dx = 10;
	int dy = 10;

	if ( QABS( x ) < dx )
	    nx = 0;
	if ( QABS( y ) < dy )
	    ny = 0;
	if ( QABS( x + width() - parentWidget()->width() ) < dx ) {
	    nx = parentWidget()->width() - width();
	    snappedRight = TRUE;
	} else
	    snappedRight = FALSE;

	if ( QABS( y + height() - parentWidget()->height() ) < dy ) {
	    ny = parentWidget()->height() - height();
	    snappedDown = TRUE;
	} else
	    snappedDown = FALSE;
    }
    QFrame::move( nx, ny );
}

bool QWorkspace::scrollBarsEnabled() const
{
    return d->vbar != 0;
}

/*!
    \property QWorkspace::scrollBarsEnabled
    \brief whether the workspace provides scrollbars

    If this property is set to TRUE, it is possible to resize child
    windows over the right or the bottom edge out of the visible area
    of the workspace. The workspace shows scrollbars to make it
    possible for the user to access those windows. If this property is
    set to FALSE (the default), resizing windows out of the visible
    area of the workspace is not permitted.
*/
void QWorkspace::setScrollBarsEnabled( bool enable )
{
    if ( (d->vbar != 0) == enable )
	return;

    d->xoffset = d->yoffset = 0;
    if ( enable ) {
	d->vbar = new QScrollBar( Vertical, this, "vertical scrollbar" );
	connect( d->vbar, SIGNAL( valueChanged(int) ), this, SLOT( scrollBarChanged() ) );
	d->hbar = new QScrollBar( Horizontal, this, "horizontal scrollbar" );
	connect( d->hbar, SIGNAL( valueChanged(int) ), this, SLOT( scrollBarChanged() ) );
	d->corner = new QWidget( this, "qt_corner" );
	updateWorkspace();
    } else {
	delete d->vbar;
	delete d->hbar;
	delete d->corner;
	d->vbar = d->hbar = 0;
	d->corner = 0;
    }

    QPtrListIterator<QWorkspaceChild> it( d->windows );
    while ( it.current () ) {
	QWorkspaceChild *child = it.current();
	++it;
	child->widgetResizeHandler->setSizeProtection( !enable );
    }
}

QRect QWorkspace::updateWorkspace()
{
    if ( !isUpdatesEnabled() )
	return rect();

    QRect cr( rect() );

    if ( scrollBarsEnabled() && !d->maxWindow ) {
	d->corner->raise();
	d->vbar->raise();
	d->hbar->raise();
	if ( d->maxWindow )
	    d->maxWindow->internalRaise();

	QRect r( 0, 0, 0, 0 );
	QPtrListIterator<QWorkspaceChild> it( d->windows );
	while ( it.current () ) {
	    QWorkspaceChild *child = it.current();
	    ++it;
	    if ( !child->isHidden() )
		r = r.unite( child->geometry() );
	}
	d->vbar->blockSignals( TRUE );
	d->hbar->blockSignals( TRUE );

	int hsbExt = d->hbar->sizeHint().height();
	int vsbExt = d->vbar->sizeHint().width();


	bool showv = d->yoffset || d->yoffset + r.bottom() - height() + 1 > 0 || d->yoffset + r.top() < 0;
	bool showh = d->xoffset || d->xoffset + r.right() - width() + 1 > 0 || d->xoffset + r.left() < 0;

	if ( showh && !showv)
	    showv = d->yoffset + r.bottom() - height() + hsbExt + 1 > 0;
	if ( showv && !showh )
	    showh = d->xoffset + r.right() - width() + vsbExt  + 1 > 0;

	if ( !showh )
	    hsbExt = 0;
	if ( !showv )
	    vsbExt = 0;

	if ( showv ) {
	    d->vbar->setSteps( QMAX( height() / 12, 30 ), height()  - hsbExt );
	    d->vbar->setRange( QMIN( 0, d->yoffset + QMIN( 0, r.top() ) ), QMAX( 0, d->yoffset + QMAX( 0, r.bottom() - height() + hsbExt + 1) ) );
	    d->vbar->setGeometry( width() - vsbExt, 0, vsbExt, height() - hsbExt );
	    d->vbar->setValue( d->yoffset );
	    d->vbar->show();
	} else {
	    d->vbar->hide();
	}

	if ( showh ) {
	    d->hbar->setSteps( QMAX( width() / 12, 30 ), width() - vsbExt );
	    d->hbar->setRange( QMIN( 0, d->xoffset + QMIN( 0, r.left() ) ), QMAX( 0, d->xoffset + QMAX( 0, r.right() - width() + vsbExt  + 1) ) );
	    d->hbar->setGeometry( 0, height() - hsbExt, width() - vsbExt, hsbExt );
	    d->hbar->setValue( d->xoffset );
	    d->hbar->show();
	} else {
	    d->hbar->hide();
	}

	if ( showh && showv ) {
	    d->corner->setGeometry( width() - vsbExt, height() - hsbExt, vsbExt, hsbExt );
	    d->corner->show();
	} else {
	    d->corner->hide();
	}

	d->vbar->blockSignals( FALSE );
	d->hbar->blockSignals( FALSE );

	cr.setRect( 0, 0, width() - vsbExt, height() - hsbExt );
    }

    QPtrListIterator<QWidget> ii( d->icons );
    while ( ii.current() ) {
	QWorkspaceChild* w = (QWorkspaceChild*)ii.current();
	++ii;
	int x = w->x();
	int y = w->y();
	bool m = FALSE;
	if ( x+w->width() > cr.width() ) {
	    m = TRUE;
	    x =  cr.width() - w->width();
	}
	if ( y+w->height() >  cr.height() ) {
	    y =  cr.height() - w->height();
	    m = TRUE;
	}
	if ( m )
	    w->move( x, y );
    }

    return cr;

}

void QWorkspace::scrollBarChanged()
{
    int ver = d->yoffset - d->vbar->value();
    int hor = d->xoffset - d->hbar->value();
    d->yoffset = d->vbar->value();
    d->xoffset = d->hbar->value();

    QPtrListIterator<QWorkspaceChild> it( d->windows );
    while ( it.current () ) {
	QWorkspaceChild *child = it.current();
	++it;
	// we do not use move() due to the reimplementation in QWorkspaceChild
	child->setGeometry( child->x() + hor, child->y() + ver, child->width(), child->height() );
    }
    updateWorkspace();
}

/*!
    \enum QWorkspace::WindowOrder

    Specifies the order in which windows are returned from windowList().

    \value CreationOrder The windows are returned in the order of their creation
    \value StackingOrder The windows are returned in the order of their stacking
*/

#ifdef QT_WORKSPACE_WINDOWMODE
/*!
    \enum QWorkspace::WindowMode

    Determines the Windowing Model QWorkspace will use for sub-windows.

    \value TopLevel Subwindows are treated as toplevel windows
    \value MDI Subwindows are organized in an MDI interface
    \value AutoDetect QWorkspace will detect whether TopLevel or MDI
    is appropriate
*/

/*!
    The windowing model influences how the subwindows are actually
    created. For most platforms the default behavior of a workspace is
    to operate in MDI mode, with Qt/Mac the default mode is
    AutoDetect.
*/
#else
/*! \internal */
#endif
QWorkspace::WindowMode QWorkspace::windowMode() const
{
    return d->wmode;
}

#ifndef QT_NO_STYLE
/*!\reimp */
void QWorkspace::styleChange( QStyle &olds )
{
    int fs = style().styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, this);
    if ( isVisibleTo(0) && d->maxWindow &&
	 fs != olds.styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, this)) {
	if( fs )
	    hideMaximizeControls();
	else
	    showMaximizeControls();
    }
    QWidget::styleChange(olds);
}
#endif




#include "qworkspace.moc"
#endif // QT_NO_WORKSPACE
