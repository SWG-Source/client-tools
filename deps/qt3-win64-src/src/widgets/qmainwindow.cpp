/****************************************************************************
** $Id: qmainwindow.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QMainWindow class
**
** Created : 980312
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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

#include "qmainwindow.h"
#ifndef QT_NO_MAINWINDOW

#include "qtimer.h"
#include "qlayout.h"
#include "qobjectlist.h"
#include "qintdict.h"
#include "qapplication.h"
#include "qptrlist.h"
#include "qmap.h"
#include "qcursor.h"
#include "qpainter.h"
#include "qmenubar.h"
#include "qpopupmenu.h"
#include "qtoolbar.h"
#include "qstatusbar.h"
#include "qscrollview.h"
#include "qtooltip.h"
#include "qdatetime.h"
#include "qwhatsthis.h"
#include "qbitmap.h"
#include "qdockarea.h"
#include "qstringlist.h"
#include "qstyle.h"
#ifdef Q_WS_MACX
#  include "qt_mac.h"
#endif

class QHideDock;
class QMainWindowLayout;

class QMainWindowPrivate
{
public:
    QMainWindowPrivate()
	:  mb(0), sb(0), ttg(0), mc(0), tll(0), mwl(0), ubp( FALSE ), utl( FALSE ),
	   justify( FALSE ), movable( TRUE ), opaque( FALSE ), dockMenu( TRUE )
    {
	docks.insert( Qt::DockTop, TRUE );
	docks.insert( Qt::DockBottom, TRUE );
	docks.insert( Qt::DockLeft, TRUE );
	docks.insert( Qt::DockRight, TRUE );
	docks.insert( Qt::DockMinimized, FALSE );
	docks.insert( Qt::DockTornOff, TRUE );
    }

    ~QMainWindowPrivate()
    {
    }

#ifndef QT_NO_MENUBAR
    QMenuBar * mb;
#else
    QWidget * mb;
#endif
    QStatusBar * sb;
    QToolTipGroup * ttg;

    QWidget * mc;

    QBoxLayout * tll;
    QMainWindowLayout * mwl;

    uint ubp :1;
    uint utl :1;
    uint justify :1;
    uint movable :1;
    uint opaque :1;
    uint dockMenu :1;

    QDockArea *topDock, *bottomDock, *leftDock, *rightDock;

    QPtrList<QDockWindow> dockWindows;
    QMap<Qt::Dock, bool> docks;
    QStringList disabledDocks;
    QHideDock *hideDock;

    QGuardedPtr<QPopupMenu> rmbMenu, tbMenu, dwMenu;
    QMap<QDockWindow*, bool> appropriate;
    QMap<QPopupMenu*, QMainWindow::DockWindows> dockWindowModes;

};


/* QMainWindowLayout, respects widthForHeight layouts (like the left
  and right docks are)
*/

class QMainWindowLayout : public QLayout
{
    Q_OBJECT

public:
    QMainWindowLayout( QMainWindow *mw, QLayout* parent = 0 );
    ~QMainWindowLayout() {}

    void addItem( QLayoutItem * );
    void setLeftDock( QDockArea *l );
    void setRightDock( QDockArea *r );
    void setCentralWidget( QWidget *w );
    bool hasHeightForWidth() const { return FALSE; }
    QSize sizeHint() const;
    QSize minimumSize() const;
    QLayoutIterator iterator();
    QSizePolicy::ExpandData expanding() const { return QSizePolicy::BothDirections; }
    void invalidate() {}

protected:
    void setGeometry( const QRect &r ) {
	QLayout::setGeometry( r );
	layoutItems( r );
    }

private:
    int layoutItems( const QRect&, bool testonly = FALSE );
    int extraPixels() const;

    QDockArea *left, *right;
    QWidget *central;
    QMainWindow *mainWindow;

};

QSize QMainWindowLayout::sizeHint() const
{
    int w = 0;
    int h = 0;

    if ( left ) {
	w += left->sizeHint().width();
	h = QMAX( h, left->sizeHint().height() );
    }
    if ( right ) {
	w += right->sizeHint().width();
	h = QMAX( h, right->sizeHint().height() );
    }
    if ( central ) {
	w += central->sizeHint().width();
	int diff = extraPixels();
	h = QMAX( h, central->sizeHint().height() + diff );
    }
    return QSize( w, h );
}

QSize QMainWindowLayout::minimumSize() const
{
    int w = 0;
    int h = 0;

    if ( left ) {
	QSize ms = left->minimumSizeHint().expandedTo( left->minimumSize() );
	w += ms.width();
	h = QMAX( h, ms.height() );
    }
    if ( right ) {
	QSize ms = right->minimumSizeHint().expandedTo( right->minimumSize() );
	w += ms.width();
	h = QMAX( h, ms.height() );
    }
    if ( central ) {
	QSize min = central->minimumSize().isNull() ?
		    central->minimumSizeHint() : central->minimumSize();
	w += min.width();
	int diff = extraPixels();
	h = QMAX( h, min.height() + diff );
    }
    return QSize( w, h );
}

QMainWindowLayout::QMainWindowLayout( QMainWindow *mw, QLayout* parent )
    : QLayout( parent ), left( 0 ), right( 0 ), central( 0 )
{
    mainWindow = mw;
}

void QMainWindowLayout::setLeftDock( QDockArea *l )
{
    left = l;
}

void QMainWindowLayout::setRightDock( QDockArea *r )
{
    right = r;
}

void QMainWindowLayout::setCentralWidget( QWidget *w )
{
    central = w;
}

int QMainWindowLayout::layoutItems( const QRect &r, bool testonly )
{
    if ( !left && !central && !right )
	return 0;

    int wl = 0, wr = 0;
    if ( left )
	wl = ( (QDockAreaLayout*)left->QWidget::layout() )->widthForHeight( r.height() );
    if ( right )
	wr = ( (QDockAreaLayout*)right->QWidget::layout() )->widthForHeight( r.height() );
    int w = r.width() - wr - wl;
    if ( w < 0 )
	w = 0;

    int diff = extraPixels();
    if ( !testonly ) {
	QRect g( geometry() );
	if ( left )
	    left->setGeometry( QRect( g.x(), g.y() + diff, wl, r.height() - diff ) );
	if ( right )
	    right->setGeometry( QRect( g.x() + g.width() - wr, g.y() + diff, wr, r.height() - diff ) );
	if ( central )
	    central->setGeometry( g.x() + wl, g.y() + diff, w, r.height() - diff );
    }

    w = wl + wr;
    if ( central )
	w += central->minimumSize().width();
    return w;
}

int QMainWindowLayout::extraPixels() const
{
    if ( mainWindow->d->topDock->isEmpty() &&
	 !(mainWindow->d->leftDock->isEmpty() &&
	   mainWindow->d->rightDock->isEmpty()) ) {
	return 2;
    } else {
	return 0;
    }
}

void QMainWindowLayout::addItem( QLayoutItem * /* item */ )
{
}


QLayoutIterator QMainWindowLayout::iterator()
{
    return 0;
}


/*
  QHideToolTip and QHideDock - minimized dock
*/

#ifndef QT_NO_TOOLTIP
class QHideToolTip : public QToolTip
{
public:
    QHideToolTip( QWidget *parent ) : QToolTip( parent ) {}
    ~QHideToolTip() {}

    virtual void maybeTip( const QPoint &pos );
};
#endif


class QHideDock : public QWidget
{
    Q_OBJECT

public:
    QHideDock( QMainWindow *parent ) : QWidget( parent, "qt_hide_dock" ) {
	hide();
	setFixedHeight( style().pixelMetric( QStyle::PM_DockWindowHandleExtent,
					     this ) + 3 );
	pressedHandle = -1;
	pressed = FALSE;
	setMouseTracking( TRUE );
	win = parent;
#ifndef QT_NO_TOOLTIP
	tip = new QHideToolTip( this );
#endif
    }
    ~QHideDock()
    {
#ifndef QT_NO_TOOLTIP
	delete tip;
#endif
    }

protected:
    void paintEvent( QPaintEvent *e ) {
	if ( !children() || children()->isEmpty() )
	    return;
	QPainter p( this );
	p.setClipRegion( e->rect() );
	p.fillRect( e->rect(), colorGroup().brush( QColorGroup::Background ) );
	int x = 0;
	int i = -1;
	QObjectListIt it( *children() );
	QObject *o;
	while ( ( o = it.current() ) ) {
	    ++it;
	    ++i;
	    QDockWindow *dw = ::qt_cast<QDockWindow*>(o);
	    if ( !dw || !dw->isVisible() )
		continue;

	    QStyle::SFlags flags = QStyle::Style_Default;
	    if ( i == pressedHandle )
		flags |= QStyle::Style_On;

	    style().drawPrimitive( QStyle::PE_DockWindowHandle, &p,
				   QRect( x, 0, 30, 10 ), colorGroup(),
				   flags );
	    x += 30;
	}
    }

    void mousePressEvent( QMouseEvent *e ) {
	pressed = TRUE;
	if ( !children() || children()->isEmpty() )
	    return;
	mouseMoveEvent( e );
	pressedHandle = -1;

	if ( e->button() == RightButton && win->isDockMenuEnabled() ) {
	    // ### TODO: HideDock menu
	} else {
	    mouseMoveEvent( e );
	}
    }

    void mouseMoveEvent( QMouseEvent *e ) {
	if ( !children() || children()->isEmpty() )
	    return;
	if ( !pressed )
	    return;
	int x = 0;
	int i = -1;
	if ( e->y() >= 0 && e->y() <= height() ) {
	    QObjectListIt it( *children() );
	    QObject *o;
	    while ( ( o = it.current() ) ) {
		++it;
		++i;
		QDockWindow *dw = ::qt_cast<QDockWindow*>(o);
		if ( !dw || !dw->isVisible() )
		    continue;

		if ( e->x() >= x && e->x() <= x + 30 ) {
		    int old = pressedHandle;
		    pressedHandle = i;
		    if ( pressedHandle != old )
			repaint( TRUE );
		    return;
		}
		x += 30;
	    }
	}
	int old = pressedHandle;
	pressedHandle = -1;
	if ( old != -1 )
	    repaint( TRUE );
    }

    void mouseReleaseEvent( QMouseEvent *e ) {
	pressed = FALSE;
	if ( pressedHandle == -1 )
	    return;
	if ( !children() || children()->isEmpty() )
	    return;
	if ( e->button() == LeftButton ) {
	    if ( e->y() >= 0 && e->y() <= height() ) {
		QObject *o = ( (QObjectList*)children() )->at( pressedHandle );
		QDockWindow *dw = ::qt_cast<QDockWindow*>(o);
		if ( dw ) {
		    dw->show();
		    dw->dock();
		}
	    }
	}
	pressedHandle = -1;
	repaint( FALSE );
    }

    bool eventFilter( QObject *o, QEvent *e ) {
	if ( o == this || !o->isWidgetType() )
	    return QWidget::eventFilter( o, e );
	if ( e->type() == QEvent::Hide ||
	     e->type() == QEvent::Show ||
	     e->type() == QEvent::ShowToParent )
	    updateState();
	return QWidget::eventFilter( o, e );
    }

    void updateState() {
	bool visible = TRUE;
	if ( !children() || children()->isEmpty() ) {
	    visible = FALSE;
	} else {
	    QObjectListIt it( *children() );
	    QObject *o;
	    while ( ( o = it.current() ) ) {
		++it;
		QDockWindow *dw = ::qt_cast<QDockWindow*>(o);
		if ( !dw )
		    continue;
		if ( dw->isHidden() ) {
		    visible = FALSE;
		    continue;
		}
		if ( !dw->isVisible() )
		    continue;
		visible = TRUE;
		break;
	    }
	}

	if ( visible )
	    show();
	else
	    hide();
	win->triggerLayout( FALSE );
	update();
    }

    void childEvent( QChildEvent *e ) {
	QWidget::childEvent( e );
	if ( e->type() == QEvent::ChildInserted )
	    e->child()->installEventFilter( this );
	else
	    e->child()->removeEventFilter( this );
	updateState();
    }

private:
    QMainWindow *win;
    int pressedHandle;
    bool pressed;
#ifndef QT_NO_TOOLTIP
    QHideToolTip *tip;
    friend class QHideToolTip;
#endif
};

#ifndef QT_NO_TOOLTIP
void QHideToolTip::maybeTip( const QPoint &pos )
{
    if ( !parentWidget() )
	return;
    QHideDock *dock = (QHideDock*)parentWidget();

    if ( !dock->children() || dock->children()->isEmpty() )
	return;
    QObjectListIt it( *dock->children() );
    QObject *o;
    int x = 0;
    while ( ( o = it.current() ) ) {
	++it;
	QDockWindow *dw = ::qt_cast<QDockWindow*>(o);
	if ( !dw || !dw->isVisible() )
	    continue;

	if ( pos.x() >= x && pos.x() <= x + 30 ) {
	    QDockWindow *dw = (QDockWindow*)o;
	    if ( !dw->caption().isEmpty() )
		tip( QRect( x, 0, 30, dock->height() ), dw->caption() );
	    return;
	}
	x += 30;
    }
}
#endif

/*!
    \class QMainWindow qmainwindow.h
    \brief The QMainWindow class provides a main application window,
    with a menu bar, dock windows (e.g. for toolbars), and a status
    bar.

    \ingroup application
    \mainclass

    Main windows are most often used to provide menus, toolbars and a
    status bar around a large central widget, such as a text edit,
    drawing canvas or QWorkspace (for MDI applications). QMainWindow
    is usually subclassed since this makes it easier to encapsulate
    the central widget, menus and toolbars as well as the window's
    state. Subclassing makes it possible to create the slots that are
    called when the user clicks menu items or toolbar buttons. You can
    also create main windows using \link designer-manual.book Qt
    Designer\endlink. We'll briefly review adding menu items and
    toolbar buttons then describe the facilities of QMainWindow
    itself.

    \code
    QMainWindow *mw = new QMainWindow;
    QTextEdit *edit = new QTextEdit( mw, "editor" );
    edit->setFocus();
    mw->setCaption( "Main Window" );
    mw->setCentralWidget( edit );
    mw->show();
    \endcode

    QMainWindows may be created in their own right as shown above.
    The central widget is set with setCentralWidget(). Popup menus can
    be added to the default menu bar, widgets can be added to the
    status bar, toolbars and dock windows can be added to any of the
    dock areas.

    \quotefile application/main.cpp
    \skipto ApplicationWindow
    \printuntil show

    In the extract above ApplicationWindow is a subclass of
    QMainWindow that we must write for ourselves; this is the usual
    approach to using QMainWindow. (The source for the extracts in
    this description are taken from \l application/main.cpp, \l
    application/application.cpp, \l action/main.cpp, and \l
    action/application.cpp )

    When subclassing we add the menu items and toolbars in the
    subclass's constructor. If we've created a QMainWindow instance
    directly we can add menu items and toolbars just as easily by
    passing the QMainWindow instance as the parent instead of the \e
    this pointer.

    \quotefile application/application.cpp
    \skipto help = new
    \printuntil about

    Here we've added a new menu with one menu item. The menu has been
    inserted into the menu bar that QMainWindow provides by default
    and which is accessible through the menuBar() function. The slot
    will be called when the menu item is clicked.

    \quotefile application/application.cpp
    \skipto fileTools
    \printuntil setLabel
    \skipto QToolButton
    \printuntil open file

    This extract shows the creation of a toolbar with one toolbar
    button. QMainWindow supplies four dock areas for toolbars. When a
    toolbar is created as a child of a QMainWindow (or derived class)
    instance it will be placed in a dock area (the \c Top dock area by
    default). The slot will be called when the toolbar button is
    clicked. Any dock window can be added to a dock area either using
    addDockWindow(), or by creating a dock window with the QMainWindow
    as the parent.

    \quotefile application/application.cpp
    \skipto editor
    \printuntil statusBar

    Having created the menus and toolbar we create an instance of the
    large central widget, give it the focus and set it as the main
    window's central widget. In the example we've also set the status
    bar, accessed via the statusBar() function, to an initial message
    which will be displayed for two seconds. Note that you can add
    additional widgets to the status bar, for example labels, to show
    further status information. See the QStatusBar documentation for
    details, particularly the addWidget() function.

    Often we want to synchronize a toolbar button with a menu item.
    For example, if the user clicks a 'bold' toolbar button we want
    the 'bold' menu item to be checked. This synchronization can be
    achieved automatically by creating actions and adding the actions
    to the toolbar and menu.

    \quotefile action/application.cpp
    \skipto QAction * fileOpen
    \printline
    \skipto fileOpenAction
    \printuntil choose

    Here we create an action with an icon which will be used in any
    menu and toolbar that the action is added to. We've also given the
    action a menu name, '\&Open', and a keyboard shortcut. The
    connection that we have made will be used when the user clicks
    either the menu item \e or the toolbar button.

    \quotefile action/application.cpp
    \skipto QPopupMenu * file
    \printuntil menuBar
    \skipto fileOpen
    \printline

    The extract above shows the creation of a popup menu. We add the
    menu to the QMainWindow's menu bar and add our action.

    \quotefile action/application.cpp
    \skipto QToolBar * fileTool
    \printuntil OpenAction

    Here we create a new toolbar as a child of the QMainWindow and add
    our action to the toolbar.

    We'll now explore the functionality offered by QMainWindow.

    The main window will take care of the dock areas, and the geometry
    of the central widget, but all other aspects of the central widget
    are left to you. QMainWindow automatically detects the creation of
    a menu bar or status bar if you specify the QMainWindow as parent,
    or you can use the provided menuBar() and statusBar() functions.
    The functions menuBar() and statusBar() create a suitable widget
    if one doesn't exist, and update the window's layout to make
    space.

    QMainWindow provides a QToolTipGroup connected to the status bar.
    The function toolTipGroup() provides access to the default
    QToolTipGroup. It isn't possible to set a different tool tip
    group.

    New dock windows and toolbars can be added to a QMainWindow using
    addDockWindow(). Dock windows can be moved using moveDockWindow()
    and removed with removeDockWindow(). QMainWindow allows default
    dock window (toolbar) docking in all its dock areas (\c Top, \c
    Left, \c Right, \c Bottom). You can use setDockEnabled() to
    enable and disable docking areas for dock windows. When adding or
    moving dock windows you can specify their 'edge' (dock area). The
    currently available edges are: \c Top, \c Left, \c Right, \c
    Bottom, \c Minimized (effectively a 'hidden' dock area) and \c
    TornOff (floating). See \l Qt::Dock for an explanation of these
    areas. Note that the *ToolBar functions are included for backward
    compatibility; all new code should use the *DockWindow functions.
    QToolbar is a subclass of QDockWindow so all functions that work
    with dock windows work on toolbars in the same way.

    \target dwm
    If the user clicks the close button, then the dock window is
    hidden. A dock window can be hidden or unhidden by the user by
    right clicking a dock area and clicking the name of the relevant
    dock window on the pop up dock window menu. This menu lists the
    names of every dock window; visible dock windows have a tick
    beside their names. The dock window menu is created automatically
    as required by createDockWindowMenu(). Since it may not always be
    appropriate for a dock window to appear on this menu the
    setAppropriate() function is used to inform the main window
    whether or not the dock window menu should include a particular
    dock window. Double clicking a dock window handle (usually on the
    left-hand side of the dock window) undocks (floats) the dock
    window. Double clicking a floating dock window's titlebar will
    dock the floating dock window. (See also
    \l{QMainWindow::DockWindows}.)

    Some functions change the appearance of a QMainWindow globally:
    \list
    \i QDockWindow::setHorizontalStretchable() and
    QDockWindow::setVerticalStretchable() are used to make specific dock
    windows or toolbars stretchable.
    \i setUsesBigPixmaps() is used to set whether tool buttons should
    draw small or large pixmaps (see QIconSet for more information).
    \i setUsesTextLabel() is used to set whether tool buttons
    should display a textual label in addition to pixmaps
    (see QToolButton for more information).
    \endlist

    The user can drag dock windows into any enabled docking area. Dock
    windows can also be dragged \e within a docking area, for example
    to rearrange the order of some toolbars. Dock windows can also be
    dragged outside any docking area (undocked or 'floated'). Being
    able to drag dock windows can be enabled (the default) and
    disabled using setDockWindowsMovable().

    The \c Minimized edge is a hidden dock area. If this dock area is
    enabled the user can hide (minimize) a dock window or show (restore)
    a minimized dock window by clicking the dock window handle. If the
    user hovers the mouse cursor over one of the handles, the caption of
    the dock window is displayed in a tool tip (see
    QDockWindow::caption() or QToolBar::label()), so if you enable the
    \c Minimized dock area, it is best to specify a meaningful caption
    or label for each dock window. To minimize a dock window
    programmatically use moveDockWindow() with an edge of \c Minimized.

    Dock windows are moved transparently by default, i.e. during the
    drag an outline rectangle is drawn on the screen representing the
    position of the dock window as it moves. If you want the dock
    window to be shown normally whilst it is moved use
    setOpaqueMoving().

    The location of a dock window, i.e. its dock area and position
    within the dock area, can be determined by calling getLocation().
    Movable dock windows can be lined up to minimize wasted space with
    lineUpDockWindows(). Pointers to the dock areas are available from
    topDock(), leftDock(), rightDock() and bottomDock(). A customize
    menu item is added to the pop up dock window menu if
    isCustomizable() returns TRUE; it returns FALSE by default.
    Reimplement isCustomizable() and customize() if you want to offer
    this extra menu item, for example, to allow the user to change
    settings relating to the main window and its toolbars and dock
    windows.

    The main window's menu bar is fixed (at the top) by default. If
    you want a movable menu bar, create a QMenuBar as a stretchable
    widget inside its own movable dock window and restrict this dock
    window to only live within the \c Top or \c Bottom dock:

    \code
    QToolBar *tb = new QToolBar( this );
    addDockWindow( tb, tr( "Menubar" ), Top, FALSE );
    QMenuBar *mb = new QMenuBar( tb );
    mb->setFrameStyle( QFrame::NoFrame );
    tb->setStretchableWidget( mb );
    setDockEnabled( tb, Left, FALSE );
    setDockEnabled( tb, Right, FALSE );
    \endcode

    An application with multiple dock windows can choose to save the
    current dock window layout in order to restore it later, e.g. in
    the next session. You can do this by using the streaming operators
    for QMainWindow.

    To save the layout and positions of all the dock windows do this:

    \code
    QFile file( filename );
    if ( file.open( IO_WriteOnly ) ) {
	QTextStream stream( &file );
	stream << *mainWindow;
	file.close();
    }
    \endcode

    To restore the dock window positions and sizes (normally when the
    application is next started), do following:

    \code
    QFile file( filename );
    if ( file.open( IO_ReadOnly ) ) {
	QTextStream stream( &file );
	stream >> *mainWindow;
	file.close();
    }
    \endcode

    The QSettings class can be used in conjunction with the streaming
    operators to store the application's settings.

    QMainWindow's management of dock windows and toolbars is done
    transparently behind-the-scenes by QDockArea.

    For multi-document interfaces (MDI), use a QWorkspace as the
    central widget.

    Adding dock windows, e.g. toolbars, to QMainWindow's dock areas is
    straightforward. If the supplied dock areas are not sufficient for
    your application we suggest that you create a QWidget subclass and
    add your own dock areas (see \l QDockArea) to the subclass since
    QMainWindow provides functionality specific to the standard dock
    areas it provides.

    <img src=qmainwindow-m.png> <img src=qmainwindow-w.png>

    \sa QToolBar QDockWindow QStatusBar QAction QMenuBar QPopupMenu QToolTipGroup QDialog
*/

/*! \enum Qt::ToolBarDock
    \internal
*/

/*!
    \enum Qt::Dock

    Each dock window can be in one of the following positions:

    \value DockTop  above the central widget, below the menu bar.

    \value DockBottom  below the central widget, above the status bar.

    \value DockLeft  to the left of the central widget.

    \value DockRight to the right of the central widget.

    \value DockMinimized the dock window is not shown (this is
    effectively a 'hidden' dock area); the handles of all minimized
    dock windows are drawn in one row below the menu bar.

    \value DockTornOff the dock window floats as its own top level
    window which always stays on top of the main window.

    \value DockUnmanaged not managed by a QMainWindow.
*/

/*!
    \enum QMainWindow::DockWindows

    Right-clicking a dock area will pop-up the dock window menu
    (createDockWindowMenu() is called automatically). When called in
    code you can specify what items should appear on the menu with
    this enum.

    \value OnlyToolBars The menu will list all the toolbars, but not
    any other dock windows.

    \value NoToolBars The menu will list dock windows but not
    toolbars.

    \value AllDockWindows The menu will list all toolbars and other
    dock windows. (This is the default.)
*/

/*!
    \obsolete
    \fn void QMainWindow::addToolBar( QDockWindow *, Dock = Top, bool newLine = FALSE );
*/

/*!
    \obsolete
    \overload void QMainWindow::addToolBar( QDockWindow *, const QString &label, Dock = Top, bool newLine = FALSE );
*/

/*!
    \obsolete
    \fn void QMainWindow::moveToolBar( QDockWindow *, Dock = Top );
*/

/*!
    \obsolete
    \overload void QMainWindow::moveToolBar( QDockWindow *, Dock, bool nl, int index, int extraOffset = -1 );
*/

/*!
    \obsolete
    \fn void QMainWindow::removeToolBar( QDockWindow * );
*/

/*!
    \obsolete
    \fn void QMainWindow::lineUpToolBars( bool keepNewLines = FALSE );
*/

/*!
    \obsolete
    \fn void QMainWindow::toolBarPositionChanged( QToolBar * );
*/

/*!
    \obsolete
    \fn bool QMainWindow::toolBarsMovable() const
*/

/*!
    \obsolete
    \fn void QMainWindow::setToolBarsMovable( bool )
*/

/*!
    Constructs an empty main window. The \a parent, \a name and widget
    flags \a f, are passed on to the QWidget constructor.

    By default, the widget flags are set to \c WType_TopLevel rather
    than 0 as they are with QWidget. If you don't want your
    QMainWindow to be a top level widget then you will need to set \a
    f to 0.
*/

QMainWindow::QMainWindow( QWidget * parent, const char * name, WFlags f )
    : QWidget( parent, name, f )
{
    d = new QMainWindowPrivate;
#ifdef Q_WS_MACX
    d->opaque = TRUE;
#else
    d->opaque = FALSE;
#endif
    installEventFilter( this );
    d->topDock = new QDockArea( Horizontal, QDockArea::Normal, this, "qt_top_dock" );
    d->topDock->installEventFilter( this );
    d->bottomDock = new QDockArea( Horizontal, QDockArea::Reverse, this, "qt_bottom_dock" );
    d->bottomDock->installEventFilter( this );
    d->leftDock = new QDockArea( Vertical, QDockArea::Normal, this, "qt_left_dock" );
    d->leftDock->installEventFilter( this );
    d->rightDock = new QDockArea( Vertical, QDockArea::Reverse, this, "qt_right_dock" );
    d->rightDock->installEventFilter( this );
    d->hideDock = new QHideDock( this );
}


/*!
    Destroys the object and frees any allocated resources.
*/

QMainWindow::~QMainWindow()
{
    delete layout();
    delete d;
}

#ifndef QT_NO_MENUBAR
/*!
    Sets this main window to use the menu bar \a newMenuBar.

    The existing menu bar (if any) is deleted along with its contents.

    \sa menuBar()
*/

void QMainWindow::setMenuBar( QMenuBar * newMenuBar )
{
    if ( !newMenuBar )
	return;
    if ( d->mb )
	delete d->mb;
    d->mb = newMenuBar;
    d->mb->installEventFilter( this );
    triggerLayout();
}


/*!
    Returns the menu bar for this window.

    If there isn't one, then menuBar() creates an empty menu bar.

    \sa statusBar()
*/

QMenuBar * QMainWindow::menuBar() const
{
    if ( d->mb )
	return d->mb;

    QObjectList * l
	= ((QObject*)this)->queryList( "QMenuBar", 0, FALSE, FALSE );
    QMenuBar * b;
    if ( l && l->count() ) {
	b = (QMenuBar *)l->first();
    } else {
	b = new QMenuBar( (QMainWindow *)this, "automatic menu bar" );
	b->show();
    }
    delete l;
    d->mb = b;
    d->mb->installEventFilter( this );
    ((QMainWindow *)this)->triggerLayout();
    return b;
}
#endif // QT_NO_MENUBAR

/*!
    Sets this main window to use the status bar \a newStatusBar.

    The existing status bar (if any) is deleted along with its
    contents.

    Note that \a newStatusBar \e must be a child of this main window,
    and that it is not automatically displayed. If you call this
    function after show(), you will probably also need to call
    newStatusBar->show().

    \sa setMenuBar() statusBar()
*/

void QMainWindow::setStatusBar( QStatusBar * newStatusBar )
{
    if ( !newStatusBar || newStatusBar == d->sb )
	return;
    if ( d->sb )
	delete d->sb;
    d->sb = newStatusBar;
#ifndef QT_NO_TOOLTIP
    // ### this code can cause unnecessary creation of a tool tip group
    connect( toolTipGroup(), SIGNAL(showTip(const QString&)),
	     d->sb, SLOT(message(const QString&)) );
    connect( toolTipGroup(), SIGNAL(removeTip()),
	     d->sb, SLOT(clear()) );
#endif
    d->sb->installEventFilter( this );
    triggerLayout();
}


/*!
    Returns this main window's status bar. If there isn't one,
    statusBar() creates an empty status bar, and if necessary a tool
    tip group too.

    \sa  menuBar() toolTipGroup()
*/

QStatusBar * QMainWindow::statusBar() const
{
    if ( d->sb )
	return d->sb;

    QObjectList * l
	= ((QObject*)this)->queryList( "QStatusBar", 0, FALSE, FALSE );
    QStatusBar * s;
    if ( l && l->count() ) {
	s = (QStatusBar *)l->first();
    } else {
	s = new QStatusBar( (QMainWindow *)this, "automatic status bar" );
	s->show();
    }
    delete l;
    ((QMainWindow *)this)->setStatusBar( s );
    ((QMainWindow *)this)->triggerLayout( TRUE );
    return s;
}


#ifndef QT_NO_TOOLTIP
/*!
    Sets this main window to use the tool tip group \a
    newToolTipGroup.

    The existing tool tip group (if any) is deleted along with its
    contents. All the tool tips connected to it lose the ability to
    display the group texts.

    \sa menuBar() toolTipGroup()
*/

void QMainWindow::setToolTipGroup( QToolTipGroup * newToolTipGroup )
{
    if ( !newToolTipGroup || newToolTipGroup == d->ttg )
	return;
    if ( d->ttg )
	delete d->ttg;
    d->ttg = newToolTipGroup;

    connect( toolTipGroup(), SIGNAL(showTip(const QString&)),
	     statusBar(), SLOT(message(const QString&)) );
    connect( toolTipGroup(), SIGNAL(removeTip()),
	     statusBar(), SLOT(clear()) );
}


/*!
    Returns this main window's tool tip group. If there isn't one,
    toolTipGroup() creates an empty tool tip group.

    \sa menuBar() statusBar()
*/

QToolTipGroup * QMainWindow::toolTipGroup() const
{
    if ( d->ttg )
	return d->ttg;

    QToolTipGroup * t = new QToolTipGroup( (QMainWindow*)this,
					   "automatic tool tip group" );
    ((QMainWindowPrivate*)d)->ttg = t;
    return t;
}
#endif


/*!
    If \a enable is TRUE then users can dock windows in the \a dock
    area. If \a enable is FALSE users cannot dock windows in the \a
    dock dock area.

    Users can dock (drag) dock windows into any enabled dock area.
*/

void QMainWindow::setDockEnabled( Dock dock, bool enable )
{
    d->docks.replace( dock, enable );
}


/*!
    Returns TRUE if the \a dock dock area is enabled, i.e. it can
    accept user dragged dock windows; otherwise returns FALSE.

    \sa setDockEnabled()
*/

bool QMainWindow::isDockEnabled( Dock dock ) const
{
    return d->docks[ dock ];
}

/*!
    \overload

    Returns TRUE if dock area \a area is enabled, i.e. it can accept
    user dragged dock windows; otherwise returns FALSE.

    \sa setDockEnabled()
*/

bool QMainWindow::isDockEnabled( QDockArea *area ) const
{

    if ( area == d->leftDock )
	return d->docks[ DockLeft ];
    if ( area == d->rightDock )
	return d->docks[ DockRight ];
    if ( area == d->topDock )
	return d->docks[ DockTop ];
    if ( area == d->bottomDock )
	return d->docks[ DockBottom ];
    return FALSE;
}

/*!
    \overload

    If \a enable is TRUE then users can dock the \a dw dock window in
    the \a dock area. If \a enable is FALSE users cannot dock the \a
    dw dock window in the \a dock area.

    In general users can dock (drag) dock windows into any enabled
    dock area. Using this function particular dock areas can be
    enabled (or disabled) as docking points for particular dock
    windows.
*/


void QMainWindow::setDockEnabled( QDockWindow *dw, Dock dock, bool enable )
{
    if ( d->dockWindows.find( dw ) == -1 ) {
	d->dockWindows.append( dw );
	connect( dw, SIGNAL( placeChanged(QDockWindow::Place) ),
		 this, SLOT( slotPlaceChanged() ) );
    }
    QString s;
    s.sprintf( "%p_%d", (void*)dw, (int)dock );
    if ( enable )
	d->disabledDocks.remove( s );
    else if ( d->disabledDocks.find( s ) == d->disabledDocks.end() )
	d->disabledDocks << s;
    switch ( dock ) {
	case DockTop:
	    topDock()->setAcceptDockWindow( dw, enable );
	    break;
	case DockLeft:
	    leftDock()->setAcceptDockWindow( dw, enable );
	    break;
	case DockRight:
	    rightDock()->setAcceptDockWindow( dw, enable );
	    break;
	case DockBottom:
	    bottomDock()->setAcceptDockWindow( dw, enable );
	    break;
	default:
	    break;
    }
}

/*!
    \overload

    Returns TRUE if dock area \a area is enabled for the dock window
    \a dw; otherwise returns FALSE.

    \sa setDockEnabled()
*/

bool QMainWindow::isDockEnabled( QDockWindow *dw, QDockArea *area ) const
{
    if ( !isDockEnabled( area ) )
	return FALSE;
    Dock dock;
    if ( area == d->leftDock )
	dock = DockLeft;
    else if ( area == d->rightDock )
	dock = DockRight;
    else if ( area == d->topDock )
	dock = DockTop;
    else if ( area == d->bottomDock )
	dock = DockBottom;
    else
	return FALSE;
    return isDockEnabled( dw, dock );
}

/*!
    \overload

    Returns TRUE if dock area \a dock is enabled for the dock window
    \a tb; otherwise returns FALSE.

    \sa setDockEnabled()
*/

bool QMainWindow::isDockEnabled( QDockWindow *tb, Dock dock ) const
{
    if ( !isDockEnabled( dock ) )
	return FALSE;
    QString s;
    s.sprintf( "%p_%d", (void*)tb, (int)dock );
    return d->disabledDocks.find( s ) == d->disabledDocks.end();
}



/*!
    Adds \a dockWindow to the \a edge dock area.

    If \a newLine is FALSE (the default) then the \a dockWindow is
    added at the end of the \a edge. For vertical edges the end is at
    the bottom, for horizontal edges (including \c Minimized) the end
    is at the right. If \a newLine is TRUE a new line of dock windows
    is started with \a dockWindow as the first (left-most and
    top-most) dock window.

    If \a dockWindow is managed by another main window, it is first
    removed from that window.
*/

void QMainWindow::addDockWindow( QDockWindow *dockWindow,
			      Dock edge, bool newLine )
{
#ifdef Q_WS_MAC
    if(isTopLevel() && edge == DockTop)
	ChangeWindowAttributes((WindowPtr)handle(), kWindowToolbarButtonAttribute, 0);
#endif
    moveDockWindow( dockWindow, edge );
    dockWindow->setNewLine( newLine );
    if ( d->dockWindows.find( dockWindow ) == -1 ) {
	d->dockWindows.append( dockWindow );
	connect( dockWindow, SIGNAL( placeChanged(QDockWindow::Place) ),
		 this, SLOT( slotPlaceChanged() ) );
	dockWindow->installEventFilter( this );
    }
    dockWindow->setOpaqueMoving( d->opaque );
}


/*!
    \overload

    Adds \a dockWindow to the dock area with label \a label.

    If \a newLine is FALSE (the default) the \a dockWindow is added at
    the end of the \a edge. For vertical edges the end is at the
    bottom, for horizontal edges (including \c Minimized) the end is
    at the right. If \a newLine is TRUE a new line of dock windows is
    started with \a dockWindow as the first (left-most and top-most)
    dock window.

    If \a dockWindow is managed by another main window, it is first
    removed from that window.
*/

void QMainWindow::addDockWindow( QDockWindow * dockWindow, const QString &label,
			      Dock edge, bool newLine )
{
    addDockWindow( dockWindow, edge, newLine );
#ifndef QT_NO_TOOLBAR
    QToolBar *tb = ::qt_cast<QToolBar*>(dockWindow);
    if ( tb )
	tb->setLabel( label );
#endif
}

/*!
    Moves \a dockWindow to the end of the \a edge.

    For vertical edges the end is at the bottom, for horizontal edges
    (including \c Minimized) the end is at the right.

    If \a dockWindow is managed by another main window, it is first
    removed from that window.
*/

void QMainWindow::moveDockWindow( QDockWindow * dockWindow, Dock edge )
{
    Orientation oo = dockWindow->orientation();
    switch ( edge ) {
    case DockTop:
	if ( dockWindow->area() != d->topDock )
	    dockWindow->removeFromDock( FALSE );
	d->topDock->moveDockWindow( dockWindow );
	emit dockWindowPositionChanged( dockWindow );
	break;
    case DockBottom:
	if ( dockWindow->area() != d->bottomDock )
	    dockWindow->removeFromDock( FALSE );
	d->bottomDock->moveDockWindow( dockWindow );
	emit dockWindowPositionChanged( dockWindow );
	break;
    case DockRight:
	if ( dockWindow->area() != d->rightDock )
	    dockWindow->removeFromDock( FALSE );
	d->rightDock->moveDockWindow( dockWindow );
	emit dockWindowPositionChanged( dockWindow );
	break;
    case DockLeft:
	if ( dockWindow->area() != d->leftDock )
	    dockWindow->removeFromDock( FALSE );
	d->leftDock->moveDockWindow( dockWindow );
	emit dockWindowPositionChanged( dockWindow );
	break;
    case DockTornOff:
	dockWindow->undock();
	break;
    case DockMinimized:
	dockWindow->undock( d->hideDock );
	break;
    case DockUnmanaged:
	break;
    }

    if ( oo != dockWindow->orientation() )
	dockWindow->setOrientation( dockWindow->orientation() );
}

/*!
    \overload

    Moves \a dockWindow to position \a index within the \a edge dock
    area.

    Any dock windows with positions \a index or higher have their
    position number incremented and any of these on the same line are
    moved right (down for vertical dock areas) to make room.

    If \a nl is TRUE, a new dock window line is created below the line
    in which the moved dock window appears and the moved dock window,
    with any others with higher positions on the same line, is moved
    to this new line.

    The \a extraOffset is the space to put between the left side of
    the dock area (top side for vertical dock areas) and the dock
    window. (This is mostly used for restoring dock windows to the
    positions the user has dragged them to.)

    If \a dockWindow is managed by another main window, it is first
    removed from that window.
*/

void QMainWindow::moveDockWindow( QDockWindow * dockWindow, Dock edge, bool nl, int index, int extraOffset )
{
    Orientation oo = dockWindow->orientation();

    dockWindow->setNewLine( nl );
    dockWindow->setOffset( extraOffset );
    switch ( edge ) {
    case DockTop:
	if ( dockWindow->area() != d->topDock )
	    dockWindow->removeFromDock( FALSE );
	d->topDock->moveDockWindow( dockWindow, index );
	break;
    case DockBottom:
	if ( dockWindow->area() != d->bottomDock )
	    dockWindow->removeFromDock( FALSE );
	d->bottomDock->moveDockWindow( dockWindow, index );
	break;
    case DockRight:
	if ( dockWindow->area() != d->rightDock )
	    dockWindow->removeFromDock( FALSE );
	d->rightDock->moveDockWindow( dockWindow, index );
	break;
    case DockLeft:
	if ( dockWindow->area() != d->leftDock )
	    dockWindow->removeFromDock( FALSE );
	d->leftDock->moveDockWindow( dockWindow, index );
	break;
    case DockTornOff:
	dockWindow->undock();
	break;
    case DockMinimized:
	dockWindow->undock( d->hideDock );
	break;
    case DockUnmanaged:
	break;
    }

    if ( oo != dockWindow->orientation() )
	dockWindow->setOrientation( dockWindow->orientation() );
}

/*!
    Removes \a dockWindow from the main window's docking area,
    provided \a dockWindow is non-null and managed by this main
    window.
*/

void QMainWindow::removeDockWindow( QDockWindow * dockWindow )
{
#ifdef Q_WS_MAC
    if(isTopLevel() && dockWindow->area() == topDock() && !dockWindows( DockTop ).count())
	ChangeWindowAttributes((WindowPtr)handle(), 0, kWindowToolbarButtonAttribute);
#endif

    dockWindow->hide();
    d->dockWindows.removeRef( dockWindow );
    disconnect( dockWindow, SIGNAL( placeChanged(QDockWindow::Place) ),
		this, SLOT( slotPlaceChanged() ) );
    dockWindow->removeEventFilter( this );
}

/*!
    Sets up the geometry management of the window. It is called
    automatically when needed, so you shouldn't need to call it.
*/

void QMainWindow::setUpLayout()
{
#ifndef QT_NO_MENUBAR
    if ( !d->mb ) {
	// slightly evil hack here.  reconsider this
	QObjectList * l
	    = ((QObject*)this)->queryList( "QMenuBar", 0, FALSE, FALSE );
	if ( l && l->count() )
	    d->mb = menuBar();
	delete l;
    }
#endif
    if ( !d->sb ) {
	// as above.
	QObjectList * l
	    = ((QObject*)this)->queryList( "QStatusBar", 0, FALSE, FALSE );
	if ( l && l->count() )
	    d->sb = statusBar();
	delete l;
    }

    if (!d->tll) {
        d->tll = new QBoxLayout( this, QBoxLayout::Down );
        d->tll->setResizeMode( minimumSize().isNull() ? QLayout::Minimum : QLayout::FreeResize );
    } else {
        d->tll->setMenuBar( 0 );
        QLayoutIterator it = d->tll->iterator();
        QLayoutItem *item;
        while ( (item = it.takeCurrent()) )
	    delete item;
    }

#ifndef QT_NO_MENUBAR
    if ( d->mb && d->mb->isVisibleTo( this ) ) {
	d->tll->setMenuBar( d->mb );
	if (style().styleHint(QStyle::SH_MainWindow_SpaceBelowMenuBar, this))
	    d->tll->addSpacing( d->movable ? 1 : 2 );
    }
#endif

    d->tll->addWidget( d->hideDock );
    if(d->topDock->parentWidget() == this)
	d->tll->addWidget( d->topDock );

    QMainWindowLayout *mwl = new QMainWindowLayout( this, d->tll );
    d->tll->setStretchFactor( mwl, 1 );

    if(d->leftDock->parentWidget() == this)
	mwl->setLeftDock( d->leftDock );
    if ( centralWidget() )
	mwl->setCentralWidget( centralWidget() );
    if(d->rightDock->parentWidget() == this)
	mwl->setRightDock( d->rightDock );
    d->mwl = mwl;

    if(d->bottomDock->parentWidget() == this)
	d->tll->addWidget( d->bottomDock );

    if ( d->sb && d->sb->parentWidget() == this) {
	d->tll->addWidget( d->sb, 0 );
	// make the sb stay on top of tool bars if there isn't enough space
	d->sb->raise();
    }
}

/*!  \reimp */
void QMainWindow::show()
{
    if ( !d->tll )
	setUpLayout();

    // show all floating dock windows not explicitly hidden
    if (!isVisible()) {
	QPtrListIterator<QDockWindow> it(d->dockWindows);
	while ( it.current() ) {
	    QDockWindow *dw = it.current();
	    ++it;
	    if ( dw->isTopLevel() && !dw->isVisible() && !dw->testWState(WState_ForceHide) )
		dw->show();
	}
    }

    // show us last so we get focus
    QWidget::show();
}


/*! \reimp
*/
void QMainWindow::hide()
{
    if ( isVisible() ) {
	QPtrListIterator<QDockWindow> it(d->dockWindows);
	while ( it.current() ) {
	    QDockWindow *dw = it.current();
	    ++it;
	    if ( dw->isTopLevel() && dw->isVisible() ) {
		dw->hide(); // implicit hide, so clear forcehide
		((QMainWindow*)dw)->clearWState(WState_ForceHide);
	    }
	}
    }

    QWidget::hide();
}


/*!  \reimp */
QSize QMainWindow::sizeHint() const
{
    QMainWindow* that = (QMainWindow*) this;
    // Workaround: because d->tll get's deleted in
    // totalSizeHint->polish->sendPostedEvents->childEvent->triggerLayout
    // [eg. canvas example on Qt/Embedded]
    QApplication::sendPostedEvents( that, QEvent::ChildInserted );
    if ( !that->d->tll )
	that->setUpLayout();
    return that->d->tll->totalSizeHint();
}

/*!  \reimp */
QSize QMainWindow::minimumSizeHint() const
{
    if ( !d->tll ) {
	QMainWindow* that = (QMainWindow*) this;
	that->setUpLayout();
    }
    return d->tll->totalMinimumSize();
}

/*!
    Sets the central widget for this main window to \a w.

    The central widget is surrounded by the left, top, right and
    bottom dock areas. The menu bar is above the top dock area.

    \sa centralWidget()
*/

void QMainWindow::setCentralWidget( QWidget * w )
{
    if ( d->mc )
	d->mc->removeEventFilter( this );
    d->mc = w;
    if ( d->mc )
	d->mc->installEventFilter( this );
    triggerLayout();
}


/*!
    Returns a pointer to the main window's central widget.

    The central widget is surrounded by the left, top, right and
    bottom dock areas. The menu bar is above the top dock area.

    \sa setCentralWidget()
*/

QWidget * QMainWindow::centralWidget() const
{
    return d->mc;
}


/*! \reimp */

void QMainWindow::paintEvent( QPaintEvent * )
{
    if (d->mb &&
	style().styleHint(QStyle::SH_MainWindow_SpaceBelowMenuBar, this)) {
	QPainter p( this );
	int y = d->mb->height() + 1;
	style().drawPrimitive(QStyle::PE_Separator, &p, QRect(0, y, width(), 1),
			      colorGroup(), QStyle::Style_Sunken);
    }
}


bool QMainWindow::dockMainWindow( QObject *dock )
{
    while ( dock ) {
	if ( dock->parent() && dock->parent() == this )
	    return TRUE;
	if ( ::qt_cast<QMainWindow*>(dock->parent()) )
	    return FALSE;
	dock = dock->parent();
    }
    return FALSE;
}

/*!
    \reimp
*/

bool QMainWindow::eventFilter( QObject* o, QEvent *e )
{
    if ( e->type() == QEvent::Show && o == this ) {
	if ( !d->tll )
	    setUpLayout();
	d->tll->activate();
    } else if ( e->type() == QEvent::ContextMenu && d->dockMenu &&
	( ::qt_cast<QDockArea*>(o) && dockMainWindow( o ) || o == d->hideDock || o == d->mb ) ) {
	if ( showDockMenu( ( (QMouseEvent*)e )->globalPos() ) ) {
	    ( (QContextMenuEvent*)e )->accept();
	    return TRUE;
	}
    }

    return QWidget::eventFilter( o, e );
}


/*!
    Monitors events, recieved in \a e, to ensure the layout is updated.
*/
void QMainWindow::childEvent( QChildEvent* e)
{
    if ( e->type() == QEvent::ChildRemoved ) {
	if ( e->child() == 0 ||
	     !e->child()->isWidgetType() ||
	     ((QWidget*)e->child())->testWFlags( WType_TopLevel ) ) {
	    // nothing
	} else if ( e->child() == d->sb ) {
	    d->sb = 0;
	    triggerLayout();
	} else if ( e->child() == d->mb ) {
	    d->mb = 0;
	    triggerLayout();
	} else if ( e->child() == d->mc ) {
	    d->mc = 0;
	    d->mwl->setCentralWidget( 0 );
	    triggerLayout();
	} else if ( ::qt_cast<QDockWindow*>(e->child()) ) {
	    removeDockWindow( (QDockWindow *)(e->child()) );
	    d->appropriate.remove( (QDockWindow*)e->child() );
	    triggerLayout();
	}
    } else if ( e->type() == QEvent::ChildInserted && !d->sb ) {
	d->sb = ::qt_cast<QStatusBar*>(e->child());
	if ( d->sb ) {
	    if ( d->tll ) {
		if ( !d->tll->findWidget( d->sb ) )
		    d->tll->addWidget( d->sb );
	    } else {
		triggerLayout();
	    }
	}
    }
}

/*!
    \reimp
*/

bool QMainWindow::event( QEvent * e )
{
    if ( e->type() == QEvent::ChildRemoved && ( (QChildEvent*)e )->child() == d->mc ) {
	d->mc->removeEventFilter( this );
	d->mc = 0;
	d->mwl->setCentralWidget( 0 );
    }

    return QWidget::event( e );
}


/*!
    \property QMainWindow::usesBigPixmaps
    \brief whether big pixmaps are enabled

    If FALSE (the default), the tool buttons will use small pixmaps;
    otherwise big pixmaps will be used.

    Tool buttons and other widgets that wish to respond to this
    setting are responsible for reading the correct state on startup,
    and for connecting to the main window's widget's
    pixmapSizeChanged() signal.
*/

bool QMainWindow::usesBigPixmaps() const
{
    return d->ubp;
}

void QMainWindow::setUsesBigPixmaps( bool enable )
{
    if ( enable == (bool)d->ubp )
	return;

    d->ubp = enable;
    emit pixmapSizeChanged( enable );

    QObjectList *l = queryList( "QLayout" );
    if ( !l || !l->first() ) {
	delete l;
	return;
    }
    for ( QLayout *lay = (QLayout*)l->first(); lay; lay = (QLayout*)l->next() )
	    lay->activate();
    delete l;
}

/*!
    \property QMainWindow::usesTextLabel
    \brief whether text labels for toolbar buttons are enabled

    If disabled (the default), the tool buttons will not use text
    labels. If enabled, text labels will be used.

    Tool buttons and other widgets that wish to respond to this
    setting are responsible for reading the correct state on startup,
    and for connecting to the main window's widget's
    usesTextLabelChanged() signal.

    \sa QToolButton::setUsesTextLabel()
*/

bool QMainWindow::usesTextLabel() const
{
    return d->utl;
}


void QMainWindow::setUsesTextLabel( bool enable )
{
    if ( enable == (bool)d->utl )
	return;

    d->utl = enable;
    emit usesTextLabelChanged( enable );

    QObjectList *l = queryList( "QLayout" );
    if ( !l || !l->first() ) {
	delete l;
	return;
    }
    for ( QLayout *lay = (QLayout*)l->first(); lay; lay = (QLayout*)l->next() )
	    lay->activate();
    delete l;
}


/*!
    \fn void QMainWindow::pixmapSizeChanged( bool )

    This signal is emitted whenever the setUsesBigPixmaps() is called
    with a value different to the current setting. All widgets that
    should respond to such changes, e.g. toolbar buttons, must connect
    to this signal.
*/

/*!
    \fn void QMainWindow::usesTextLabelChanged( bool )

    This signal is emitted whenever the setUsesTextLabel() is called
    with a value different to the current setting. All widgets that
    should respond to such changes, e.g. toolbar buttons, must connect
    to this signal.
*/

/*!
    \fn void QMainWindow::dockWindowPositionChanged( QDockWindow *dockWindow )

    This signal is emitted when the \a dockWindow has changed its
    position. A change in position occurs when a dock window is moved
    within its dock area or moved to another dock area (including the
    \c Minimized and \c TearOff dock areas).

    \sa getLocation()
*/

void QMainWindow::setRightJustification( bool enable )
{
    if ( enable == (bool)d->justify )
	return;
    d->justify = enable;
    triggerLayout( TRUE );
}


/*!
    \obsolete
    \property QMainWindow::rightJustification
    \brief whether the main window right-justifies its dock windows

    If disabled (the default), stretchable dock windows are expanded,
    and non-stretchable dock windows are given the minimum space they
    need. Since most dock windows are not stretchable, this usually
    results in an unjustified right edge (or unjustified bottom edge
    for a vertical dock area). If enabled, the main window will
    right-justify its dock windows.

    \sa QDockWindow::setVerticalStretchable(), QDockWindow::setHorizontalStretchable()
*/

bool QMainWindow::rightJustification() const
{
    return d->justify;
}

/*! \internal
 */

void QMainWindow::triggerLayout( bool deleteLayout )
{
    if ( deleteLayout || !d->tll )
	setUpLayout();
    QApplication::postEvent( this, new QEvent( QEvent::LayoutHint ) );
}

/*!
    Enters 'What's This?' mode and returns immediately.

    This is the same as QWhatsThis::enterWhatsThisMode(), but
    implemented as a main window object's slot. This way it can easily
    be used for popup menus, for example:

    \code
    QPopupMenu * help = new QPopupMenu( this );
    help->insertItem( "What's &This", this , SLOT(whatsThis()), SHIFT+Key_F1);
    \endcode

    \sa QWhatsThis::enterWhatsThisMode()
*/
void QMainWindow::whatsThis()
{
#ifndef QT_NO_WHATSTHIS
    QWhatsThis::enterWhatsThisMode();
#endif
}


/*!
    \reimp
*/

void QMainWindow::styleChange( QStyle& old )
{
    QWidget::styleChange( old );
}

/*!
    Finds the location of the dock window \a dw.

    If the \a dw dock window is found in the main window the function
    returns TRUE and populates the \a dock variable with the dw's dock
    area and the \a index with the dw's position within the dock area.
    It also sets \a nl to TRUE if the \a dw begins a new line
    (otherwise FALSE), and \a extraOffset with the dock window's offset.

    If the \a dw dock window is not found then the function returns
    FALSE and the state of \a dock, \a index, \a nl and \a extraOffset
    is undefined.

    If you want to save and restore dock window positions then use
    operator>>() and operator<<().

    \sa operator>>() operator<<()
*/

bool QMainWindow::getLocation( QDockWindow *dw, Dock &dock, int &index, bool &nl, int &extraOffset ) const
{
    dock = DockTornOff;
    if ( d->topDock->hasDockWindow( dw, &index ) )
	dock = DockTop;
    else if ( d->bottomDock->hasDockWindow( dw, &index ) )
	dock = DockBottom;
    else if ( d->leftDock->hasDockWindow( dw, &index ) )
	dock = DockLeft;
    else if ( d->rightDock->hasDockWindow( dw, &index ) )
	dock = DockRight;
    else if ( dw->parentWidget() == d->hideDock ) {
	index = 0;
	dock = DockMinimized;
    } else {
	index = 0;
    }
    nl = dw->newLine();
    extraOffset = dw->offset();
    return TRUE;
}

#ifndef QT_NO_TOOLBAR
/*!
    Returns a list of all the toolbars which are in the \a dock dock
    area, regardless of their state.

    For example, the \c TornOff dock area may contain closed toolbars
    but these are returned along with the visible toolbars.

    \sa dockWindows()
*/

QPtrList<QToolBar> QMainWindow::toolBars( Dock dock ) const
{
    QPtrList<QDockWindow> lst = dockWindows( dock );
    QPtrList<QToolBar> tbl;
    for ( QDockWindow *w = lst.first(); w; w = lst.next() ) {
	QToolBar *tb = ::qt_cast<QToolBar*>(w);
	if ( tb )
	    tbl.append( tb );
    }
    return tbl;
}
#endif

/*!
    Returns a list of all the dock windows which are in the \a dock
    dock area, regardless of their state.

    For example, the \c DockTornOff dock area may contain closed dock
    windows but these are returned along with the visible dock
    windows.
*/

QPtrList<QDockWindow> QMainWindow::dockWindows( Dock dock ) const
{
    QPtrList<QDockWindow> lst;
    switch ( dock ) {
    case DockTop:
	return d->topDock->dockWindowList();
    case DockBottom:
	return d->bottomDock->dockWindowList();
    case DockLeft:
	return d->leftDock->dockWindowList();
    case DockRight:
	return d->rightDock->dockWindowList();
    case DockTornOff: {
	for ( QDockWindow *w = d->dockWindows.first(); w; w = d->dockWindows.next() ) {
	    if ( !w->area() && w->place() == QDockWindow::OutsideDock )
		lst.append( w );
	}
    }
    return lst;
    case DockMinimized: {
	if ( d->hideDock->children() ) {
	    QObjectListIt it( *d->hideDock->children() );
	    QObject *o;
	    while ( ( o = it.current() ) ) {
		++it;
		QDockWindow *dw = ::qt_cast<QDockWindow*>(o);
		if ( !dw )
		    continue;
		lst.append( dw );
	    }
	}
    }
    return lst;
    default:
	break;
    }
    return lst;
}

/*!
    \overload

    Returns the list of dock windows which belong to this main window,
    regardless of which dock area they are in or what their state is,
    (e.g. irrespective of whether they are visible or not).
*/

QPtrList<QDockWindow> QMainWindow::dockWindows() const
{
    return d->dockWindows;
}

void QMainWindow::setDockWindowsMovable( bool enable )
{
    d->movable = enable;
    QObjectList *l = queryList( "QDockWindow" );
    if ( l ) {
	for ( QObject *o = l->first(); o; o = l->next() )
	    ( (QDockWindow*)o )->setMovingEnabled( enable );
    }
    delete l;
}

/*!
    \property QMainWindow::dockWindowsMovable
    \brief whether the dock windows are movable

    If TRUE (the default), the user will be able to move movable dock
    windows from one QMainWindow dock area to another, including the
    \c TearOff area (i.e. where the dock window floats freely as a
    window in its own right), and the \c Minimized area (where only
    the dock window's handle is shown below the menu bar). Moveable
    dock windows can also be moved within QMainWindow dock areas, i.e.
    to rearrange them within a dock area.

    If FALSE the user will not be able to move any dock windows.

    By default dock windows are moved transparently (i.e. only an
    outline rectangle is shown during the drag), but this setting can
    be changed with setOpaqueMoving().

    \sa setDockEnabled(), setOpaqueMoving()
*/

bool QMainWindow::dockWindowsMovable() const
{
    return d->movable;
}

void QMainWindow::setOpaqueMoving( bool b )
{
    d->opaque = b;
    QObjectList *l = queryList( "QDockWindow" );
    if ( l ) {
	for ( QObject *o = l->first(); o; o = l->next() )
	    ( (QDockWindow*)o )->setOpaqueMoving( b );
    }
    delete l;
}

/*!
    \property QMainWindow::opaqueMoving
    \brief whether dock windows are moved opaquely

    If TRUE the dock windows of the main window are shown opaquely
    (i.e. it shows the toolbar as it looks when docked) whilst it is
    being moved. If FALSE (the default) they are shown transparently,
    (i.e. as an outline rectangle).

    \warning Opaque moving of toolbars and dockwindows is known to
    have several problems. We recommend avoiding the use of this
    feature for the time being. We intend fixing the problems in a
    future release.
*/

bool QMainWindow::opaqueMoving() const
{
    return d->opaque;
}

/*!
    This function will line up dock windows within the visible dock
    areas (\c Top, \c Left, \c Right and \c Bottom) as compactly as
    possible.

    If \a keepNewLines is TRUE, all dock windows stay on their
    original lines. If \a keepNewLines is FALSE then newlines may be
    removed to achieve the most compact layout possible.

    The method only works if dockWindowsMovable() returns TRUE.
*/

void QMainWindow::lineUpDockWindows( bool keepNewLines )
{
    if ( !dockWindowsMovable() )
	return;
    d->topDock->lineUp( keepNewLines );
    d->leftDock->lineUp( keepNewLines );
    d->rightDock->lineUp( keepNewLines );
    d->bottomDock->lineUp( keepNewLines );
}

/*!
    Returns TRUE, if the dock window menu is enabled; otherwise
    returns FALSE.

    The menu lists the (appropriate()) dock windows (which may be
    shown or hidden), and has a "Line Up Dock Windows" menu item. It
    will also have a "Customize" menu item if isCustomizable() returns
    TRUE.

    \sa setDockEnabled(), lineUpDockWindows() appropriate()
    setAppropriate()
*/

bool QMainWindow::isDockMenuEnabled() const
{
    return d->dockMenu;
}

/*!
    If \a b is TRUE, then right clicking on a dock window or dock area
    will pop up the dock window menu. If \a b is FALSE, right clicking
    a dock window or dock area will not pop up the menu.

    The menu lists the (appropriate()) dock windows (which may be
    shown or hidden), and has a "Line Up Dock Windows" item. It will
    also have a "Customize" menu item if isCustomizable() returns
    TRUE.

    \sa lineUpDockWindows(), isDockMenuEnabled()
*/

void QMainWindow::setDockMenuEnabled( bool b )
{
    d->dockMenu = b;
}

/*!
    Creates the dock window menu which contains all toolbars (if \a
    dockWindows is \c OnlyToolBars ), all dock windows (if \a
    dockWindows is \c NoToolBars) or all toolbars and dock windows (if
    \a dockWindows is \c AllDockWindows - the default).

    This function is called internally when necessary, e.g. when the
    user right clicks a dock area (providing isDockMenuEnabled()
    returns TRUE).
\omit
### Qt 4.0
    You can reimplement this function if you wish to customize the
    behaviour.
\endomit

    The menu items representing the toolbars and dock windows are
    checkable. The visible dock windows are checked and the hidden
    dock windows are unchecked. The user can click a menu item to
    change its state (show or hide the dock window).

    The list and the state are always kept up-to-date.

    Toolbars and dock windows which are not appropriate in the current
    context (see setAppropriate()) are not listed in the menu.

    The menu also has a menu item for lining up the dock windows.

    If isCustomizable() returns TRUE, a Customize menu item is added
    to the menu, which if clicked will call customize(). The
    isCustomizable() function we provide returns FALSE and customize()
    does nothing, so they must be reimplemented in a subclass to be
    useful.
*/

QPopupMenu *QMainWindow::createDockWindowMenu( DockWindows dockWindows ) const
{
    QObjectList *l = queryList( "QDockWindow" );

    if ( !l || l->isEmpty() )
	return 0;

    delete l;

    QPopupMenu *menu = new QPopupMenu( (QMainWindow*)this, "qt_customize_menu" );
    menu->setCheckable( TRUE );
    d->dockWindowModes.replace( menu, dockWindows );
    connect( menu, SIGNAL( aboutToShow() ), this, SLOT( menuAboutToShow() ) );
    return menu;
}

/*!
    This slot is called from the aboutToShow() signal of the default
    dock menu of the mainwindow. The default implementation
    initializes the menu with all dock windows and toolbars in this
    slot.
\omit
### Qt 4.0
    If you want to do small adjustments to the menu, you can do it in
    this slot; or you can reimplement createDockWindowMenu().
\endomit
*/

void QMainWindow::menuAboutToShow()
{
    QPopupMenu *menu = (QPopupMenu*)sender();
    QMap<QPopupMenu*, DockWindows>::Iterator it = d->dockWindowModes.find( menu );
    if ( it == d->dockWindowModes.end() )
	return;
    menu->clear();

    DockWindows dockWindows = *it;

    QObjectList *l = queryList( "QDockWindow" );

    bool empty = TRUE;
    if ( l && !l->isEmpty() ) {

	QObject *o = 0;
	if ( dockWindows == AllDockWindows || dockWindows == NoToolBars ) {
	    for ( o = l->first(); o; o = l->next() ) {
		QDockWindow *dw = (QDockWindow*)o;
		if ( !appropriate( dw ) || ::qt_cast<QToolBar*>(dw) || !dockMainWindow( dw ) )
		    continue;
		QString label = dw->caption();
		if ( !label.isEmpty() ) {
		    int id = menu->insertItem( label, dw, SLOT( toggleVisible() ) );
		    menu->setItemChecked( id, dw->isVisible() );
		    empty = FALSE;
		}
	    }
	    if ( !empty )
		menu->insertSeparator();
	}

	empty = TRUE;

#ifndef QT_NO_TOOLBAR
	if ( dockWindows == AllDockWindows || dockWindows == OnlyToolBars ) {
	    for ( o = l->first(); o; o = l->next() ) {
		QToolBar *tb = ::qt_cast<QToolBar*>(o);
		if ( !tb || !appropriate(tb) || !dockMainWindow(tb) )
		    continue;
		QString label = tb->label();
		if ( !label.isEmpty() ) {
		    int id = menu->insertItem( label, tb, SLOT( toggleVisible() ) );
		    menu->setItemChecked( id, tb->isVisible() );
		    empty = FALSE;
		}
	    }
	}
#endif

    }

    delete l;

    if ( !empty )
	menu->insertSeparator();

    if ( dockWindowsMovable() )
	menu->insertItem( tr( "Line up" ), this, SLOT( doLineUp() ) );
    if ( isCustomizable() )
	menu->insertItem( tr( "Customize..." ), this, SLOT( customize() ) );
}

/*!
    Shows the dock menu at the position \a globalPos. The menu lists
    the dock windows so that they can be shown (or hidden), lined up,
    and possibly customized. Returns TRUE if the menu is shown;
    otherwise returns FALSE.

    If you want a custom menu, reimplement this function. You can
    create the menu from scratch or call createDockWindowMenu() and
    modify the result.
\omit
### Qt 4.0
    The default implementation uses the dock window menu which gets
    created by createDockWindowMenu(). You can reimplement
    createDockWindowMenu() if you want to use your own specialized
    popup menu.
\endomit
*/

bool QMainWindow::showDockMenu( const QPoint &globalPos )
{
    if ( !d->dockMenu )
	return FALSE;
    if ( !d->rmbMenu )
	d->rmbMenu = createDockWindowMenu();
    if ( !d->rmbMenu )
	return FALSE;

    d->rmbMenu->exec( globalPos );
    return TRUE;
}

void QMainWindow::slotPlaceChanged()
{
    QObject* obj = (QObject*)sender();
    QDockWindow *dw = ::qt_cast<QDockWindow*>(obj);
    if ( dw )
	emit dockWindowPositionChanged( dw );
#ifndef QT_NO_TOOLBAR
    QToolBar *tb = ::qt_cast<QToolBar*>(obj);
    if ( tb )
	emit toolBarPositionChanged( tb );
#endif
}

/*!
    \internal
    For internal use of QDockWindow only.
 */

QDockArea *QMainWindow::dockingArea( const QPoint &p )
{
    int mh = d->mb ? d->mb->height() : 0;
    int sh = d->sb ? d->sb->height() : 0;
    if ( p.x() >= -5 && p.x() <= 100 && p.y() > mh && p.y() - height() - sh )
	return d->leftDock;
    if ( p.x() >= width() - 100 && p.x() <= width() + 5 && p.y() > mh && p.y() - height() - sh )
	return d->rightDock;
    if ( p.y() >= -5 && p.y() < mh + 100 && p.x() >= 0 && p.x() <= width() )
	return d->topDock;
    if ( p.y() >= height() - sh - 100 && p.y() <= height() + 5 && p.x() >= 0 && p.x() <= width() )
	return d->bottomDock;
    return 0;
}

/*!
    Returns TRUE if \a dw is a dock window known to the main window;
    otherwise returns FALSE.
*/

bool QMainWindow::hasDockWindow( QDockWindow *dw )
{
    return d->dockWindows.findRef( dw ) != -1;
}

/*!
    Returns the \c Left dock area

    \sa rightDock() topDock() bottomDock()
*/

QDockArea *QMainWindow::leftDock() const
{
    return d->leftDock;
}

/*!
    Returns the \c Right dock area

    \sa leftDock() topDock() bottomDock()
*/

QDockArea *QMainWindow::rightDock() const
{
    return d->rightDock;
}

/*!
    Returns the \c Top dock area

    \sa bottomDock() leftDock() rightDock()
*/

QDockArea *QMainWindow::topDock() const
{
    return d->topDock;
}

/*!
    Returns a pointer the \c Bottom dock area

    \sa topDock() leftDock() rightDock()
*/

QDockArea *QMainWindow::bottomDock() const
{
    return d->bottomDock;
}

/*!
    This function is called when the user clicks the Customize menu
    item on the dock window menu.

    The customize menu item will only appear if isCustomizable()
    returns TRUE (it returns FALSE by default).

    The function is intended, for example, to provide the user with a
    means of telling the application that they wish to customize the
    main window, dock windows or dock areas.

    The default implementation does nothing and the Customize menu
    item is not shown on the right-click menu by default. If you want
    the item to appear then reimplement isCustomizable() to return
    TRUE, and reimplement this function to do whatever you want.

    \sa isCustomizable()
*/

void QMainWindow::customize()
{
}

/*!
    Returns TRUE if the dock area dock window menu includes the
    Customize menu item (which calls customize() when clicked).
    Returns FALSE by default, i.e. the popup menu will not contain a
    Customize menu item. You will need to reimplement this function
    and set it to return TRUE if you wish the user to be able to see
    the dock window menu.

    \sa customize()
*/

bool QMainWindow::isCustomizable() const
{
    return FALSE;
}

/*!
    Returns TRUE if it is appropriate to include a menu item for the
    \a dw dock window in the dock window menu; otherwise returns
    FALSE.

    The user is able to change the state (show or hide) a dock window
    that has a menu item by clicking the item.

    Call setAppropriate() to indicate whether or not a particular dock
    window should appear on the popup menu.

    \sa setAppropriate()
*/

bool QMainWindow::appropriate( QDockWindow *dw ) const
{
    QMap<QDockWindow*, bool>::ConstIterator it = d->appropriate.find( dw );
    if ( it == d->appropriate.end() )
	return TRUE;
    return *it;
}

/*!
    Use this function to control whether or not the \a dw dock
    window's caption should appear as a menu item on the dock window
    menu that lists the dock windows.

    If \a a is TRUE then the \a dw will appear as a menu item on the
    dock window menu. The user is able to change the state (show or
    hide) a dock window that has a menu item by clicking the item;
    depending on the state of your application, this may or may not be
    appropriate. If \a a is FALSE the \a dw will not appear on the
    popup menu.

    \sa showDockMenu() isCustomizable() customize()
*/

void QMainWindow::setAppropriate( QDockWindow *dw, bool a )
{
    d->appropriate.replace( dw, a );
}

#ifndef QT_NO_TEXTSTREAM
static void saveDockArea( QTextStream &ts, QDockArea *a )
{
    QPtrList<QDockWindow> l = a->dockWindowList();
    for ( QDockWindow *dw = l.first(); dw; dw = l.next() ) {
	ts << QString( dw->caption() );
	ts << ",";
    }
    ts << endl;
    ts << *a;
}

/*!
    \relates QMainWindow

    Writes the layout (sizes and positions) of the dock windows in the
    dock areas of the QMainWindow \a mainWindow, including \c
    Minimized and \c TornOff dock windows, to the text stream \a ts.

    This can be used, for example, in conjunction with QSettings to
    save the user's layout when the \mainWindow receives a closeEvent.

    \sa operator>>() closeEvent()
*/

QTextStream &operator<<( QTextStream &ts, const QMainWindow &mainWindow )
{
    QPtrList<QDockWindow> l = mainWindow.dockWindows( Qt::DockMinimized );
    QDockWindow *dw = 0;
    for ( dw = l.first(); dw; dw = l.next() ) {
	ts << dw->caption();
	ts << ",";
    }
    ts << endl;

    l = mainWindow.dockWindows( Qt::DockTornOff );
    for ( dw = l.first(); dw; dw = l.next() ) {
	ts << dw->caption();
	ts << ",";
    }
    ts << endl;
    for ( dw = l.first(); dw; dw = l.next() ) {
	ts << "[" << dw->caption() << ","
	   << (int)dw->geometry().x() << ","
	   << (int)dw->geometry().y() << ","
	   << (int)dw->geometry().width() << ","
	   << (int)dw->geometry().height() << ","
	   << (int)dw->isVisible() << "]";
    }
    ts << endl;

    saveDockArea( ts, mainWindow.topDock() );
    saveDockArea( ts, mainWindow.bottomDock() );
    saveDockArea( ts, mainWindow.rightDock() );
    saveDockArea( ts, mainWindow.leftDock() );
    return ts;
}

static void loadDockArea( const QStringList &names, QDockArea *a, Qt::Dock d, QPtrList<QDockWindow> &l, QMainWindow *mw, QTextStream &ts )
{
    for ( QStringList::ConstIterator it = names.begin(); it != names.end(); ++it ) {
	for ( QDockWindow *dw = l.first(); dw; dw = l.next() ) {
	    if ( dw->caption() == *it ) {
		mw->addDockWindow( dw, d );
		break;
	    }
	}
    }
    if ( a ) {
	ts >> *a;
    } else if ( d == Qt::DockTornOff ) {
	QString s = ts.readLine();
	enum State { Pre, Name, X, Y, Width, Height, Visible, Post };
	int state = Pre;
	QString name, x, y, w, h, visible;
	QChar c;
	for ( int i = 0; i < (int)s.length(); ++i ) {
	    c = s[ i ];
	    if ( state == Pre && c == '[' ) {
		state++;
		continue;
	    }
	    if ( c == ',' &&
		 ( state == Name || state == X || state == Y || state == Width || state == Height ) ) {
		state++;
		continue;
	    }
	    if ( state == Visible && c == ']' ) {
		for ( QDockWindow *dw = l.first(); dw; dw = l.next() ) {
		    if ( QString( dw->caption() ) == name ) {
			if ( !::qt_cast<QToolBar*>(dw) )
			    dw->setGeometry( x.toInt(), y.toInt(), w.toInt(), h.toInt() );
			else
			    dw->setGeometry( x.toInt(), y.toInt(), dw->width(), dw->height() );
			if ( !(bool)visible.toInt() )
			    dw->hide();
			else
			    dw->show();
			break;
		    }
		}

		name = x = y = w = h = visible = "";

		state = Pre;
		continue;
	    }
	    if ( state == Name )
		name += c;
	    else if ( state == X )
		x += c;
	    else if ( state == Y )
		y += c;
	    else if ( state == Width )
		w += c;
	    else if ( state == Height )
		h += c;
	    else if ( state == Visible )
		visible += c;
	}
    }
}

/*!
    \relates QMainWindow

    Reads the layout (sizes and positions) of the dock windows in the
    dock areas of the QMainWindow \a mainWindow from the text stream,
    \a ts, including \c Minimized and \c TornOff dock windows.
    Restores the dock windows and dock areas to these sizes and
    positions. The layout information must be in the format produced
    by operator<<().

    This can be used, for example, in conjunction with QSettings to
    restore the user's layout.

    \sa operator<<()
*/

QTextStream &operator>>( QTextStream &ts, QMainWindow &mainWindow )
{
    QPtrList<QDockWindow> l = mainWindow.dockWindows();

    QString s = ts.readLine();
    QStringList names = QStringList::split( ',', s );
    loadDockArea( names, 0, Qt::DockMinimized, l, &mainWindow, ts );

    s = ts.readLine();
    names = QStringList::split( ',', s );
    loadDockArea( names, 0, Qt::DockTornOff, l, &mainWindow, ts );

    int i = 0;
    QDockArea *areas[] = { mainWindow.topDock(), mainWindow.bottomDock(), mainWindow.rightDock(), mainWindow.leftDock() };
    for ( int d = (int)Qt::DockTop; d != (int)Qt::DockMinimized; ++d, ++i ) {
	s = ts.readLine();
	names = QStringList::split( ',', s );
	loadDockArea( names, areas[ i ], (Qt::Dock)d, l, &mainWindow, ts );
    }
    return ts;
}
#endif

#include "qmainwindow.moc"

#endif
