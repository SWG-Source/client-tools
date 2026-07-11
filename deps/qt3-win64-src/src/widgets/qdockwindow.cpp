/****************************************************************************
** $Id: qdockwindow.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of the QDockWindow class
**
** Created : 001010
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

#include "qdockwindow.h"

#ifndef QT_NO_MAINWINDOW
#include "qdesktopwidget.h"
#include "qdockarea.h"
#include "qwidgetresizehandler_p.h"
#include "qtitlebar_p.h"
#include "qpainter.h"
#include "qapplication.h"
#include "qtoolbutton.h"
#include "qtoolbar.h"
#include "qlayout.h"
#include "qmainwindow.h"
#include "qtimer.h"
#include "qtooltip.h"
#include "qguardedptr.h"
#include "qcursor.h"
#include "qstyle.h"

#if defined(Q_WS_MAC9)
#define MAC_DRAG_HACK
#endif
#ifdef Q_WS_MACX
static bool default_opaque = TRUE;
#else
static bool default_opaque = FALSE;
#endif

class QDockWindowPrivate
{
};

class QDockWindowResizeHandle : public QWidget
{
    Q_OBJECT

public:
    QDockWindowResizeHandle( Qt::Orientation o, QWidget *parent, QDockWindow *w, const char* /*name*/=0 );
    void setOrientation( Qt::Orientation o );
    Qt::Orientation orientation() const { return orient; }

    QSize sizeHint() const;

protected:
    void paintEvent( QPaintEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );

private:
    void startLineDraw();
    void endLineDraw();
    void drawLine( const QPoint &globalPos );

private:
    Qt::Orientation orient;
    bool mousePressed;
    QPainter *unclippedPainter;
    QPoint lastPos, firstPos;
    QDockWindow *dockWindow;

};

QDockWindowResizeHandle::QDockWindowResizeHandle( Qt::Orientation o, QWidget *parent,
						  QDockWindow *w, const char * )
    : QWidget( parent, "qt_dockwidget_internal" ), mousePressed( FALSE ), unclippedPainter( 0 ), dockWindow( w )
{
    setOrientation( o );
}

QSize QDockWindowResizeHandle::sizeHint() const
{
    int sw = 2 * style().pixelMetric(QStyle::PM_SplitterWidth, this) / 3;
    return (style().sizeFromContents(QStyle::CT_DockWindow, this, QSize(sw, sw)).
	    expandedTo(QApplication::globalStrut()));
}

void QDockWindowResizeHandle::setOrientation( Qt::Orientation o )
{
    orient = o;
    if ( o == QDockArea::Horizontal ) {
#ifndef QT_NO_CURSOR
	setCursor( splitVCursor );
#endif
	setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    } else {
#ifndef QT_NO_CURSOR
	setCursor( splitHCursor );
#endif
	setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding ) );
    }
}

void QDockWindowResizeHandle::mousePressEvent( QMouseEvent *e )
{
    e->ignore();
    if ( e->button() != LeftButton )
	return;
    e->accept();
    mousePressed = TRUE;
    if ( !dockWindow->opaqueMoving() )
	startLineDraw();
    lastPos = firstPos = e->globalPos();
    if ( !dockWindow->opaqueMoving() )
	drawLine( e->globalPos() );
}

void QDockWindowResizeHandle::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed )
	return;
    if ( !dockWindow->opaqueMoving() ) {
	if ( orientation() != dockWindow->area()->orientation() ) {
	    if ( orientation() == Horizontal ) {
		int minpos = dockWindow->area()->mapToGlobal( QPoint( 0, 0 ) ).y();
		int maxpos = dockWindow->area()->mapToGlobal( QPoint( 0, 0 ) ).y() + dockWindow->area()->height();
		if ( e->globalPos().y() < minpos || e->globalPos().y() > maxpos )
		    return;
	    } else {
		int minpos = dockWindow->area()->mapToGlobal( QPoint( 0, 0 ) ).x();
		int maxpos = dockWindow->area()->mapToGlobal( QPoint( 0, 0 ) ).x() + dockWindow->area()->width();
		if ( e->globalPos().x() < minpos || e->globalPos().x() > maxpos )
		    return;
	    }
	} else {
	    QWidget *w = dockWindow->area()->topLevelWidget();
	    if ( w ) {
		if ( orientation() == Horizontal ) {
		    int minpos = w->mapToGlobal( QPoint( 0, 0 ) ).y();
		    int maxpos = w->mapToGlobal( QPoint( 0, 0 ) ).y() + w->height();
		    if ( e->globalPos().y() < minpos || e->globalPos().y() > maxpos )
			return;
		} else {
		    int minpos = w->mapToGlobal( QPoint( 0, 0 ) ).x();
		    int maxpos = w->mapToGlobal( QPoint( 0, 0 ) ).x() + w->width();
		    if ( e->globalPos().x() < minpos || e->globalPos().x() > maxpos )
			return;
		}
	    }
	}
    }

    if ( !dockWindow->opaqueMoving() )
	drawLine( lastPos );
    lastPos = e->globalPos();
    if ( dockWindow->opaqueMoving() ) {
	mouseReleaseEvent( e );
	mousePressed = TRUE;
	firstPos = e->globalPos();
    }
    if ( !dockWindow->opaqueMoving() )
	drawLine( e->globalPos() );
}

void QDockWindowResizeHandle::mouseReleaseEvent( QMouseEvent *e )
{
    if ( mousePressed ) {
	if ( !dockWindow->opaqueMoving() ) {
	    drawLine( lastPos );
	    endLineDraw();
	}
	if ( orientation() != dockWindow->area()->orientation() )
	    dockWindow->area()->invalidNextOffset( dockWindow );
	if ( orientation() == Horizontal ) {
	    int dy;
	    if ( dockWindow->area()->handlePosition() == QDockArea::Normal || orientation() != dockWindow->area()->orientation() )
		dy = e->globalPos().y() - firstPos.y();
	    else
		dy =  firstPos.y() - e->globalPos().y();
	    int d = dockWindow->height() + dy;
	    if ( orientation() != dockWindow->area()->orientation() ) {
		dockWindow->setFixedExtentHeight( -1 );
		d = QMAX( d, dockWindow->minimumHeight() );
		int ms = dockWindow->area()->maxSpace( d, dockWindow );
		d = QMIN( d, ms );
		dockWindow->setFixedExtentHeight( d );
	    } else {
		dockWindow->area()->setFixedExtent( d, dockWindow );
	    }
	} else {
	    int dx;
	    if ( dockWindow->area()->handlePosition() == QDockArea::Normal || orientation() != dockWindow->area()->orientation() )
		dx = e->globalPos().x() - firstPos.x();
	    else
		dx = firstPos.x() - e->globalPos().x();
	    int d = dockWindow->width() + dx;
	    if ( orientation() != dockWindow->area()->orientation() ) {
		dockWindow->setFixedExtentWidth( -1 );
		d = QMAX( d, dockWindow->minimumWidth() );
		int ms = dockWindow->area()->maxSpace( d, dockWindow );
		d = QMIN( d, ms );
		dockWindow->setFixedExtentWidth( d );
	    } else {
		dockWindow->area()->setFixedExtent( d, dockWindow );
	    }
	}
    }

    QApplication::postEvent( dockWindow->area(), new QEvent( QEvent::LayoutHint ) );
    mousePressed = FALSE;
}

void QDockWindowResizeHandle::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    style().drawPrimitive(QStyle::PE_DockWindowResizeHandle, &p, rect(), colorGroup(),
			  (isEnabled() ?
			   QStyle::Style_Enabled : QStyle::Style_Default) |
			  (orientation() == Qt::Horizontal ?
			   QStyle::Style_Horizontal : QStyle::Style_Default ));
}

void QDockWindowResizeHandle::startLineDraw()
{
    if ( unclippedPainter )
	endLineDraw();
#ifdef MAC_DRAG_HACK
    QWidget *paint_on = topLevelWidget();
#else
    int scr = QApplication::desktop()->screenNumber( this );
    QWidget *paint_on = QApplication::desktop()->screen( scr );
#endif
    unclippedPainter = new QPainter( paint_on, TRUE );
    unclippedPainter->setPen( QPen( gray, orientation() == Horizontal ? height() : width() ) );
    unclippedPainter->setRasterOp( XorROP );
}

void QDockWindowResizeHandle::endLineDraw()
{
    if ( !unclippedPainter )
	return;
    delete unclippedPainter;
    unclippedPainter = 0;
}

void QDockWindowResizeHandle::drawLine( const QPoint &globalPos )
{
#ifdef MAC_DRAG_HACK
    QPoint start = mapTo(topLevelWidget(), QPoint(0, 0));
    QPoint starta = dockWindow->area()->mapTo(topLevelWidget(), QPoint(0, 0));
    QPoint end = globalPos - topLevelWidget()->pos();
#else
    QPoint start = mapToGlobal( QPoint( 0, 0 ) );
    QPoint starta = dockWindow->area()->mapToGlobal( QPoint( 0, 0 ) );
    QPoint end = globalPos;
#endif

    if ( orientation() == Horizontal ) {
	if ( orientation() == dockWindow->orientation() )
	    unclippedPainter->drawLine( starta.x() , end.y(), starta.x() + dockWindow->area()->width(), end.y() );
	else
	    unclippedPainter->drawLine( start.x(), end.y(), start.x() + width(), end.y() );
    } else {
	if ( orientation() == dockWindow->orientation() )
	    unclippedPainter->drawLine( end.x(), starta.y(), end.x(), starta.y() + dockWindow->area()->height() );
	else
	    unclippedPainter->drawLine( end.x(), start.y(), end.x(), start.y() + height() );
    }
}

static QPoint realWidgetPos( QDockWindow *w )
{
    if ( !w->parentWidget() || w->place() == QDockWindow::OutsideDock )
	return w->pos();
    return w->parentWidget()->mapToGlobal( w->geometry().topLeft() );
}

class QDockWindowHandle : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString caption READ caption )
    friend class QDockWindow;
    friend class QDockWindowTitleBar;

public:
    QDockWindowHandle( QDockWindow *dw );
    void updateGui();

    QSize minimumSizeHint() const;
    QSize minimumSize() const { return minimumSizeHint(); }
    QSize sizeHint() const { return minimumSize(); }
    QSizePolicy sizePolicy() const;
    void setOpaqueMoving( bool b ) { opaque = b; }

    QString caption() const { return dockWindow->caption(); }

signals:
    void doubleClicked();

protected:
    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
    void mouseDoubleClickEvent( QMouseEvent *e );
    void keyPressEvent( QKeyEvent *e );
    void keyReleaseEvent( QKeyEvent *e );
#ifndef QT_NO_STYLE
    void styleChange( QStyle& );
#endif

private slots:
    void minimize();

private:
    QDockWindow *dockWindow;
    QPoint offset;
    QToolButton *closeButton;
    QTimer *timer;
    uint opaque		: 1;
    uint mousePressed	: 1;
    uint hadDblClick	: 1;
    uint ctrlDown : 1;
    QGuardedPtr<QWidget> oldFocus;
};

class QDockWindowTitleBar : public QTitleBar
{
    Q_OBJECT
    friend class QDockWindow;
    friend class QDockWindowHandle;

public:
    QDockWindowTitleBar( QDockWindow *dw );
    void updateGui();
    void setOpaqueMoving( bool b ) { opaque = b; }

protected:
    void resizeEvent( QResizeEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
    void mouseDoubleClickEvent( QMouseEvent *e );
    void keyPressEvent( QKeyEvent *e );
    void keyReleaseEvent( QKeyEvent *e );

private:
    QDockWindow *dockWindow;
    QPoint offset;
    uint mousePressed : 1;
    uint hadDblClick : 1;
    uint opaque : 1;
    uint ctrlDown : 1;
    QGuardedPtr<QWidget> oldFocus;

};

QDockWindowHandle::QDockWindowHandle( QDockWindow *dw )
    : QWidget( dw, "qt_dockwidget_internal", WNoAutoErase ), dockWindow( dw ),
      closeButton( 0 ), opaque( default_opaque ), mousePressed( FALSE )
{
    ctrlDown = FALSE;
    timer = new QTimer( this );
    connect( timer, SIGNAL( timeout() ), this, SLOT( minimize() ) );
#ifdef Q_WS_WIN
    setCursor( SizeAllCursor );
#endif
}

void QDockWindowHandle::paintEvent( QPaintEvent *e )
{
    if ( (!dockWindow->dockArea || mousePressed) && !opaque )
	return;
    erase();
    QPainter p( this );
    QStyle::SFlags flags = QStyle::Style_Default;
    if ( isEnabled() )
	flags |= QStyle::Style_Enabled;
    if ( !dockWindow->area() || dockWindow->area()->orientation() == Horizontal )
	flags |= QStyle::Style_Horizontal;

    style().drawPrimitive( QStyle::PE_DockWindowHandle, &p,
			   QStyle::visualRect( style().subRect( QStyle::SR_DockWindowHandleRect,
								this ), this ),
			   colorGroup(), flags );
    QWidget::paintEvent( e );
}

void QDockWindowHandle::keyPressEvent( QKeyEvent *e )
{
    if ( !mousePressed )
	return;
    if ( e->key() == Key_Control ) {
	ctrlDown = TRUE;
	dockWindow->handleMove( mapFromGlobal(QCursor::pos()) - offset, QCursor::pos(), !opaque );
    }
}

void QDockWindowHandle::keyReleaseEvent( QKeyEvent *e )
{
    if ( !mousePressed )
	return;
    if ( e->key() == Key_Control ) {
	ctrlDown = FALSE;
	dockWindow->handleMove( mapFromGlobal(QCursor::pos()) - offset, QCursor::pos(), !opaque );
    }
}

void QDockWindowHandle::mousePressEvent( QMouseEvent *e )
{
    if ( !dockWindow->dockArea )
	return;
    ctrlDown = ( e->state() & ControlButton ) == ControlButton;
    oldFocus = qApp->focusWidget();
    setFocus();
    e->ignore();
    if ( e->button() != LeftButton )
	return;
    e->accept();
    hadDblClick = FALSE;
    mousePressed = TRUE;
    offset = e->pos();
    dockWindow->startRectDraw( mapToGlobal( e->pos() ), !opaque );
    if ( !opaque )
	qApp->installEventFilter( dockWindow );
}

void QDockWindowHandle::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed || e->pos() == offset )
	return;
    ctrlDown = ( e->state() & ControlButton ) == ControlButton;
    dockWindow->handleMove( e->pos() - offset, e->globalPos(), !opaque );
    if ( opaque )
	dockWindow->updatePosition( e->globalPos() );
}

void QDockWindowHandle::mouseReleaseEvent( QMouseEvent *e )
{
    ctrlDown = FALSE;
    qApp->removeEventFilter( dockWindow );
    if ( oldFocus )
	oldFocus->setFocus();
    if ( !mousePressed )
	return;
    dockWindow->endRectDraw( !opaque );
    mousePressed = FALSE;
#ifdef Q_WS_MAC
    releaseMouse();
#endif
    if ( !hadDblClick && offset == e->pos() ) {
	timer->start( QApplication::doubleClickInterval(), TRUE );
    } else if ( !hadDblClick ) {
	dockWindow->updatePosition( e->globalPos() );
    }
    if ( opaque )
	dockWindow->titleBar->mousePressed = FALSE;
}

void QDockWindowHandle::minimize()
{
    if ( !dockWindow->area() )
	return;

    QMainWindow *mw = ::qt_cast<QMainWindow*>(dockWindow->area()->parentWidget());
    if ( mw && mw->isDockEnabled( dockWindow, Qt::DockMinimized ) )
	mw->moveDockWindow( dockWindow, Qt::DockMinimized );
}

void QDockWindowHandle::resizeEvent( QResizeEvent * )
{
    updateGui();
}

void QDockWindowHandle::updateGui()
{
    if ( !closeButton ) {
	closeButton = new QToolButton( this, "qt_close_button1" );
#ifndef QT_NO_CURSOR
	closeButton->setCursor( arrowCursor );
#endif
	closeButton->setPixmap( style().stylePixmap( QStyle::SP_DockWindowCloseButton, closeButton ) );
	closeButton->setFixedSize( 12, 12 );
	connect( closeButton, SIGNAL( clicked() ),
		 dockWindow, SLOT( hide() ) );
    }

    if ( dockWindow->isCloseEnabled() && dockWindow->area() )
	closeButton->show();
    else
	closeButton->hide();

    if ( !dockWindow->area() )
	return;

    if ( dockWindow->area()->orientation() == Horizontal ) {
	int off = ( width() - closeButton->width() - 1 ) / 2;
	closeButton->move( off, 2 );
    } else {
	int off = ( height() - closeButton->height() - 1 ) / 2;
	int x = QApplication::reverseLayout() ? 2 : width() - closeButton->width() - 2;
	closeButton->move( x, off );
    }
}

#ifndef QT_NO_STYLE
void QDockWindowHandle::styleChange( QStyle& )
{
    if ( closeButton )
	closeButton->setPixmap( style().stylePixmap( QStyle::SP_DockWindowCloseButton, closeButton ) );
}
#endif

QSize QDockWindowHandle::minimumSizeHint() const
{
    if ( !dockWindow->dockArea )
	return QSize( 0, 0 );
    int wh = dockWindow->isCloseEnabled() ? 17 : style().pixelMetric( QStyle::PM_DockWindowHandleExtent, this );
    if ( dockWindow->orientation() == Horizontal )
	return QSize( wh, 0 );
    return QSize( 0, wh );
}

QSizePolicy QDockWindowHandle::sizePolicy() const
{
    if ( dockWindow->orientation() != Horizontal )
	return QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
    return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );
}

void QDockWindowHandle::mouseDoubleClickEvent( QMouseEvent *e )
{
    e->ignore();
    if ( e->button() != LeftButton )
	return;
    e->accept();
    timer->stop();
    emit doubleClicked();
    hadDblClick = TRUE;
}

QDockWindowTitleBar::QDockWindowTitleBar( QDockWindow *dw )
    : QTitleBar( 0, dw, "qt_dockwidget_internal" ), dockWindow( dw ),
      mousePressed( FALSE ), hadDblClick( FALSE ), opaque( default_opaque )
{
    setWFlags( getWFlags() | WStyle_Tool );
    ctrlDown = FALSE;
    setMouseTracking( TRUE );
    setFixedHeight( style().pixelMetric( QStyle::PM_TitleBarHeight, this ) );
    connect( this, SIGNAL(doClose()), dockWindow, SLOT(hide()) );
}

void QDockWindowTitleBar::keyPressEvent( QKeyEvent *e )
{
    if ( !mousePressed )
	return;
    if ( e->key() == Key_Control ) {
	ctrlDown = TRUE;
	dockWindow->handleMove( mapFromGlobal( QCursor::pos() ) - offset, QCursor::pos(), !opaque );
    }
}

void QDockWindowTitleBar::keyReleaseEvent( QKeyEvent *e )
{
    if ( !mousePressed )
	return;
    if ( e->key() == Key_Control ) {
	ctrlDown = FALSE;
	dockWindow->handleMove( mapFromGlobal( QCursor::pos() ) - offset, QCursor::pos(), !opaque );
    }
}

void QDockWindowTitleBar::mousePressEvent( QMouseEvent *e )
{
    QStyle::SubControl tbctrl = style().querySubControl( QStyle::CC_TitleBar, this, e->pos() );
    if ( tbctrl > QStyle::SC_TitleBarLabel ) {
	QTitleBar::mousePressEvent( e );
	return;
    }

    ctrlDown = ( e->state() & ControlButton ) == ControlButton;
    oldFocus = qApp->focusWidget();
// setFocus activates the window, which deactivates the main window
// not what we want, and not required anyway on Windows
#ifndef Q_WS_WIN
    setFocus();
#endif

    e->ignore();
    if ( e->button() != LeftButton )
	return;
    if ( e->y() < 3 && dockWindow->isResizeEnabled() )
	return;

    e->accept();
    bool oldPressed = mousePressed;
    mousePressed = TRUE;
    hadDblClick = FALSE;
    offset = e->pos();
    dockWindow->startRectDraw( mapToGlobal( e->pos() ), !opaque );
// grabMouse resets the Windows mouse press count, so we never receive a double click on Windows
// not required on Windows, and did work on X11, too, but no problem there in the first place
#ifndef Q_WS_WIN
    if(!oldPressed && dockWindow->opaqueMoving())
	grabMouse();
#else
    Q_UNUSED( oldPressed );
#endif
}

void QDockWindowTitleBar::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed ) {
	QTitleBar::mouseMoveEvent( e );
	return;
    }

    ctrlDown = ( e->state() & ControlButton ) == ControlButton;
    e->accept();
    dockWindow->handleMove( e->pos() - offset, e->globalPos(), !opaque );
}

void QDockWindowTitleBar::mouseReleaseEvent( QMouseEvent *e )
{
    if ( !mousePressed ) {
	QTitleBar::mouseReleaseEvent( e );
	return;
    }

    ctrlDown = FALSE;
    qApp->removeEventFilter( dockWindow );
    if ( oldFocus )
	oldFocus->setFocus();

    if ( dockWindow->place() == QDockWindow::OutsideDock )
	dockWindow->raise();

    if(dockWindow->opaqueMoving())
	releaseMouse();
    if ( !mousePressed )
	return;
    dockWindow->endRectDraw( !opaque );
    mousePressed = FALSE;
    if ( !hadDblClick )
	dockWindow->updatePosition( e->globalPos() );
    if ( opaque ) {
	dockWindow->horHandle->mousePressed = FALSE;
	dockWindow->verHandle->mousePressed = FALSE;
    }
}

void QDockWindowTitleBar::resizeEvent( QResizeEvent *e )
{
    updateGui();
    QTitleBar::resizeEvent( e );
}

void QDockWindowTitleBar::updateGui()
{
    if ( dockWindow->isCloseEnabled() ) {
	setWFlags( getWFlags() | WStyle_SysMenu );
    } else {
	setWFlags( getWFlags() & ~WStyle_SysMenu );
    }
}

void QDockWindowTitleBar::mouseDoubleClickEvent( QMouseEvent * )
{
    emit doubleClicked();
    hadDblClick = TRUE;
}

/*!
    \class QDockWindow qdockwindow.h
    \brief The QDockWindow class provides a widget which can be docked
    inside a QDockArea or floated as a top level window on the
    desktop.

    \ingroup application
    \mainclass

    This class handles moving, resizing, docking and undocking dock
    windows. QToolBar is a subclass of QDockWindow so the
    functionality provided for dock windows is available with the same
    API for toolbars.

    \img qmainwindow-qdockareas.png QDockWindows in a QDockArea
    \caption Two QDockWindows (\l{QToolBar}s) in a \l QDockArea

    \img qdockwindow.png A QDockWindow
    \caption A Floating QDockWindow

    If the user drags the dock window into the dock area the dock
    window will be docked. If the user drags the dock area outside any
    dock areas the dock window will be undocked (floated) and will
    become a top level window. Double clicking a floating dock
    window's titlebar will dock the dock window to the last dock area
    it was docked in. Double clicking a docked dock window's handle
    will undock (float) the dock window.
    \omit
    Single clicking a docked dock window's handle will minimize the
    dock window (only its handle will appear, below the menu bar).
    Single clicking the minimized handle will restore the dock window
    to the last dock area that it was docked in.
    \endomit
    If the user clicks the close button (which does not appear on
    dock windows by default - see \l closeMode) the dock window will
    disappear. You can control whether or not a dock window has a
    close button with setCloseMode().

    QMainWindow provides four dock areas (top, left, right and bottom)
    which can be used by dock windows. For many applications using the
    dock areas provided by QMainWindow is sufficient. (See the \l
    QDockArea documentation if you want to create your own dock
    areas.) In QMainWindow a right-click popup menu (the dock window
    menu) is available which lists dock windows and can be used to
    show or hide them. (The popup menu only lists dock windows that
    have a \link setCaption() caption\endlink.)

    When you construct a dock window you \e must pass it a QDockArea
    or a QMainWindow as its parent if you want it docked. Pass 0 for
    the parent if you want it floated.

    \code
    QToolBar *fileTools = new QToolBar( this, "File Actions" );
    moveDockWindow( fileTools, Left );
    \endcode

    In the example above we create a new QToolBar in the constructor
    of a QMainWindow subclass (so that the \e this pointer points to
    the QMainWindow). By default the toolbar will be added to the \c
    Top dock area, but we've moved it to the \c Left dock area.

    A dock window is often used to contain a single widget. In these
    cases the widget can be set by calling setWidget(). If you're
    constructing a dock window that contains multiple widgets, e.g. a
    toolbar, arrange the widgets within a box layout inside the dock
    window. To do this use the boxLayout() function to get a pointer
    to the dock window's box layout, then add widgets to the layout
    using the box layout's QBoxLayout::addWidget() function. The dock
    window will dynamically set the orientation of the layout to be
    vertical or horizontal as necessary, although you can control this
    yourself with setOrientation().

    Although a common use of dock windows is for toolbars, they can be
    used with any widgets. (See the \link designer-manual.book Qt
    Designer\endlink and \link linguist-manual.book Qt
    Linguist\endlink applications, for example.) When using larger
    widgets it may make sense for the dock window to be resizable by
    calling setResizeEnabled(). Resizable dock windows are given
    splitter-like handles to allow the user to resize them within
    their dock area. When resizable dock windows are undocked they
    become top level windows and can be resized like any other top
    level windows, e.g. by dragging a corner or edge.

    Dock windows can be docked and undocked using dock() and undock().
    A dock window's orientation can be set with setOrientation(). You
    can also use QDockArea::moveDockWindow(). If you're using a
    QMainWindow, QMainWindow::moveDockWindow() and
    QMainWindow::removeDockWindow() are available.

    A dock window can have some preferred settings, for example, you
    can set a preferred offset from the left edge (or top edge for
    vertical dock areas) of the dock area using setOffset(). If you'd
    prefer a dock window to start on a new \link qdockarea.html#lines
    line\endlink when it is docked use setNewLine(). The
    setFixedExtentWidth() and setFixedExtentHeight() functions can be
    used to define the dock window's preferred size, and the
    setHorizontallyStretchable() and setVerticallyStretchable()
    functions set whether the dock window can be stretched or not.
    Dock windows can be moved by default, but this can be changed with
    setMovingEnabled(). When a dock window is moved it is shown as a
    rectangular outline, but it can be shown normally using
    setOpaqueMoving().

    When a dock window's visibility changes, i.e. it is shown or
    hidden, the visibilityChanged() signal is emitted. When a dock
    window is docked, undocked or moved inside the dock area the
    placeChanged() signal is emitted.
*/

/*!
    \enum QDockWindow::Place

    This enum specifies the possible locations for a QDockWindow:

    \value InDock  Inside a QDockArea.
    \value OutsideDock  Floating as a top level window on the desktop.
*/

/*!
    \enum QDockWindow::CloseMode

    This enum type specifies when (if ever) a dock window has a close
    button.

    \value Never  The dock window never has a close button and cannot
    be closed by the user.
    \value Docked  The dock window has a close button only when
    docked.
    \value Undocked  The dock window has a close button only when
    floating.
    \value Always The dock window always has a close button.
    \omit
    Note that dock windows can always be minimized if the user clicks
    their dock window handle when they are docked.
    \endomit
*/

/*!
    \fn void QDockWindow::setHorizontalStretchable( bool b )
    \obsolete
*/
/*!
    \fn void QDockWindow::setVerticalStretchable( bool b )
    \obsolete
*/
/*!
    \fn bool QDockWindow::isHorizontalStretchable() const
    \obsolete
*/
/*!
    \fn bool QDockWindow::isVerticalStretchable() const
    \obsolete
*/
/*!
    \fn void QDockWindow::orientationChanged( Orientation o )

    This signal is emitted when the orientation of the dock window is
    changed. The new orientation is \a o.
*/

/*!
    \fn void QDockWindow::placeChanged( QDockWindow::Place p )

    This signal is emitted when the dock window is docked (\a p is \c
    InDock), undocked (\a p is \c OutsideDock) or moved inside the
    the dock area.

    \sa QDockArea::moveDockWindow(), QDockArea::removeDockWindow(),
    QMainWindow::moveDockWindow(), QMainWindow::removeDockWindow()
*/

/*!
    \fn void QDockWindow::visibilityChanged( bool visible )

    This signal is emitted when the visibility of the dock window
    relatively to its dock area is changed. If \a visible is TRUE, the
    QDockWindow is now visible to the dock area, otherwise it has been
    hidden.

    A dock window can be hidden if it has a close button which the
    user has clicked. In the case of a QMainWindow a dock window can
    have its visibility changed (hidden or shown) by clicking its name
    in the dock window menu that lists the QMainWindow's dock windows.
*/

/*!
    \fn QDockArea *QDockWindow::area() const

    Returns the dock area in which this dock window is docked, or 0 if
    the dock window is floating.
*/

// DOC: Can't use \property 'cos it thinks the thing returns a bool.
/*!
    \fn Place QDockWindow::place() const

    This function returns where the dock window is placed. This is
    either \c InDock or \c OutsideDock.

    \sa QDockArea::moveDockWindow(), QDockArea::removeDockWindow(),
    QMainWindow::moveDockWindow(), QMainWindow::removeDockWindow()
*/


/*!
    Constructs a QDockWindow with parent \a parent, called \a name and
    with widget flags \a f.
*/

QDockWindow::QDockWindow( QWidget* parent, const char* name, WFlags f )
    : QFrame( parent, name, f | WType_Dialog | WStyle_Customize | WStyle_NoBorder )
{
    curPlace = InDock;
    isToolbar = FALSE;
    init();
}

/*!
    Constructs a QDockWindow with parent \a parent, called \a name and
    with widget flags \a f.

    If \a p is \c InDock, the dock window is docked into a dock area
    and \a parent \e must be a QDockArea or a QMainWindow. If the \a
    parent is a QMainWindow the dock window will be docked in the main
    window's \c Top dock area.

    If \a p is \c OutsideDock, the dock window is created as a floating
    window.

    We recommend creating the dock area \c InDock with a QMainWindow
    as parent then calling QMainWindow::moveDockWindow() to move the
    dock window where you want it.
*/

QDockWindow::QDockWindow( Place p, QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f | WType_Dialog | WStyle_Customize | WStyle_NoBorder )
{
    curPlace = p;
    isToolbar = FALSE;
    init();
}

/*! \internal
*/

QDockWindow::QDockWindow( Place p, QWidget *parent, const char *name, WFlags f, bool toolbar )
    : QFrame( parent, name, f | WType_Dialog | WStyle_Customize | WStyle_NoBorder )
{
    curPlace = p;
    isToolbar = toolbar;
    init();
}

class QDockWindowGridLayout : public QGridLayout
{
public:
    QDockWindowGridLayout( QWidget *parent, int nRows, int nCols )
	: QGridLayout( parent, nRows, nCols ) {};

    QSizePolicy::ExpandData expanding() const
    {
	return QSizePolicy::NoDirection;
    }
};

void QDockWindow::init()
{
    wid = 0;
    unclippedPainter = 0;
    dockArea = 0;
    tmpDockArea = 0;
    resizeEnabled = FALSE;
    moveEnabled = TRUE;
    nl = FALSE;
    opaque = default_opaque;
    cMode = Never;
    offs = 0;
    fExtent = QSize( -1, -1 );
    dockWindowData = 0;
    lastPos = QPoint( -1, -1 );
    lastSize = QSize( -1, -1 );

    widgetResizeHandler = new QWidgetResizeHandler( this );
    widgetResizeHandler->setMovingEnabled( FALSE );

    titleBar      = new QDockWindowTitleBar( this );
    verHandle     = new QDockWindowHandle( this );
    horHandle     = new QDockWindowHandle( this );

    vHandleLeft   = new QDockWindowResizeHandle( Qt::Vertical, this, this, "vert. handle" );
    vHandleRight  = new QDockWindowResizeHandle( Qt::Vertical, this, this, "vert. handle" );
    hHandleTop    = new QDockWindowResizeHandle( Qt::Horizontal, this, this, "horz. handle" );
    hHandleBottom = new QDockWindowResizeHandle( Qt::Horizontal, this, this, "horz. handle" );

    // Creating inner layout
    hbox	  = new QVBoxLayout();
    vbox	  = new QHBoxLayout();
    childBox	  = new QBoxLayout(QBoxLayout::LeftToRight);
    vbox->addWidget( verHandle );
    vbox->addLayout( childBox );

    hbox->setResizeMode( QLayout::FreeResize );
    hbox->setMargin( isResizeEnabled() || curPlace == OutsideDock ? 2 : 0 );
    hbox->setSpacing( 1 );
    hbox->addWidget( titleBar );
    hbox->addWidget( horHandle );
    hbox->addLayout( vbox );

    // Set up the initial handle layout for Vertical
    // Handle layout will change on calls to setOrienation()
    QGridLayout *glayout = new QDockWindowGridLayout( this, 3, 3 );
    glayout->setResizeMode( QLayout::Minimum );
    glayout->addMultiCellWidget( hHandleTop,    0, 0, 1, 1 );
    glayout->addMultiCellWidget( hHandleBottom, 2, 2, 1, 1 );
    glayout->addMultiCellWidget( vHandleLeft,   0, 2, 0, 0 );
    glayout->addMultiCellWidget( vHandleRight,  0, 2, 2, 2 );
    glayout->addLayout( hbox, 1, 1 );
    glayout->setRowStretch( 1, 1 );
    glayout->setColStretch( 1, 1 );

    hHandleBottom->hide();
    vHandleRight->hide();
    hHandleTop->hide();
    vHandleLeft->hide();
    setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    setLineWidth( 2 );

    if ( parentWidget() )
	parentWidget()->installEventFilter( this );
    QWidget *mw = parentWidget();
    QDockArea *da = ::qt_cast<QDockArea*>(parentWidget());
    if ( da ) {
	if ( curPlace == InDock )
	    da->moveDockWindow( this );
	mw = da->parentWidget();
    }
    if ( ::qt_cast<QMainWindow*>(mw) ) {
	if ( place() == InDock ) {
	    Dock myDock = Qt::DockTop;
	    // make sure we put the window in the correct dock.
	    if ( dockArea ) {
		QMainWindow *mainw = (QMainWindow*)mw;
		// I'm not checking if it matches the top because I've
		// done the assignment to it above.
		if ( dockArea == mainw->leftDock() )
		    myDock = Qt::DockLeft;
		else if ( dockArea == mainw->rightDock() )
		    myDock = Qt::DockRight;
		else if ( dockArea == mainw->bottomDock() )
		    myDock = Qt::DockBottom;
	    }
	    ( (QMainWindow*)mw )->addDockWindow( this, myDock );
	}
	moveEnabled = ((QMainWindow*)mw)->dockWindowsMovable();
	opaque = ((QMainWindow*)mw)->opaqueMoving();
    }

    updateGui();
    stretchable[ Horizontal ] = FALSE;
    stretchable[ Vertical ] = FALSE;

    connect( titleBar, SIGNAL( doubleClicked() ), this, SLOT( dock() ) );
    connect( verHandle, SIGNAL( doubleClicked() ), this, SLOT( undock() ) );
    connect( horHandle, SIGNAL( doubleClicked() ), this, SLOT( undock() ) );
    connect( this, SIGNAL( orientationChanged(Orientation) ),
	     this, SLOT( setOrientation(Orientation) ) );
}

/*!
    Sets the orientation of the dock window to \a o. The orientation
    is propagated to the layout boxLayout().

    \warning All undocked QToolBars will always have a horizontal orientation.
*/

void QDockWindow::setOrientation( Orientation o )
{
    QGridLayout *glayout = (QGridLayout*)layout();
    glayout->remove( hHandleTop );
    glayout->remove( hHandleBottom );
    glayout->remove( vHandleLeft );
    glayout->remove( vHandleRight );

    if ( o == Horizontal ) {
	// Set up the new layout as
	//   3 3 3      1 = vHandleLeft   4 = hHandleBottom
	//   1 X 2      2 = vHandleRight  X = Inner Layout
	//   4 4 4      3 = hHandleTop
	glayout->addMultiCellWidget( hHandleTop,    0, 0, 0, 2 );
	glayout->addMultiCellWidget( hHandleBottom, 2, 2, 0, 2 );
	glayout->addMultiCellWidget( vHandleLeft,   1, 1, 0, 0 );
	glayout->addMultiCellWidget( vHandleRight,  1, 1, 2, 2 );
    } else {
	// Set up the new layout as
	//   1 3 2      1 = vHandleLeft   4 = hHandleBottom
	//   1 X 2      2 = vHandleRight  X = Inner Layout
	//   1 4 2      3 = hHandleTop
	glayout->addMultiCellWidget( hHandleTop,    0, 0, 1, 1 );
	glayout->addMultiCellWidget( hHandleBottom, 2, 2, 1, 1 );
	glayout->addMultiCellWidget( vHandleLeft,   0, 2, 0, 0 );
	glayout->addMultiCellWidget( vHandleRight,  0, 2, 2, 2 );
    }
    boxLayout()->setDirection( o == Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom );
    QApplication::sendPostedEvents( this, QEvent::LayoutHint );
    QEvent *e = new QEvent( QEvent::LayoutHint );
    QApplication::postEvent( this, e );
}

/*!
    \reimp

    Destroys the dock window and its child widgets.
*/

QDockWindow::~QDockWindow()
{
    qApp->removeEventFilter( this );
    if ( area() )
	area()->removeDockWindow( this, FALSE, FALSE );
    QDockArea *a = area();
    if ( !a && dockWindowData )
	a = ( (QDockArea::DockWindowData*)dockWindowData )->area;
    QMainWindow *mw = a ? ::qt_cast<QMainWindow*>(a->parentWidget()) : 0;
    if ( mw )
	mw->removeDockWindow( this );

    delete (QDockArea::DockWindowData*)dockWindowData;
}

/*!  \reimp
*/

void QDockWindow::resizeEvent( QResizeEvent *e )
{
    QFrame::resizeEvent( e );
    updateGui();
}


void QDockWindow::swapRect( QRect &r, Qt::Orientation o, const QPoint &offset, QDockArea * )
{
    QBoxLayout *bl = boxLayout()->createTmpCopy();
    bl->setDirection( o == Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom );
    bl->activate();
    r.setSize( bl->sizeHint() );
    bl->data = 0;
    delete bl;
    bool reverse = QApplication::reverseLayout();
    if ( o == Qt::Horizontal )
	r.moveBy( -r.width()/2, 0 );
    else
	r.moveBy( reverse ? - r.width() : 0, -r.height() / 2  );
    r.moveBy( offset.x(), offset.y() );
}

QWidget *QDockWindow::areaAt( const QPoint &gp )
{
    QWidget *w = qApp->widgetAt( gp, TRUE );

    if ( w && ( w == this || w == titleBar ) && parentWidget() )
	w = parentWidget()->childAt( parentWidget()->mapFromGlobal( gp ) );

    while ( w ) {
	if ( ::qt_cast<QDockArea*>(w) ) {
	    QDockArea *a = (QDockArea*)w;
	    if ( a->isDockWindowAccepted( this ) )
		return w;
	}
	if ( ::qt_cast<QMainWindow*>(w) ) {
	    QMainWindow *mw = (QMainWindow*)w;
	    QDockArea *a = mw->dockingArea( mw->mapFromGlobal( gp ) );
	    if ( a && a->isDockWindowAccepted( this ) )
		return a;
	}
	w = w->parentWidget( TRUE );
    }
    return 0;
}

void QDockWindow::handleMove( const QPoint &pos, const QPoint &gp, bool drawRect )
{
    if ( !unclippedPainter )
	return;

    if ( drawRect ) {
	QRect dr(currRect);
#ifdef MAC_DRAG_HACK
	dr.moveBy(-topLevelWidget()->geometry().x(), -topLevelWidget()->geometry().y());
#endif
	unclippedPainter->drawRect( dr );
    }
    currRect = QRect( realWidgetPos( this ), size() );
    QWidget *w = areaAt( gp );
    if ( titleBar->ctrlDown || horHandle->ctrlDown || verHandle->ctrlDown )
	w = 0;
    currRect.moveBy( pos.x(), pos.y() );
    if ( !::qt_cast<QDockArea*>(w) ) {
        if ( startOrientation != Horizontal && ::qt_cast<QToolBar*>(this) )
	    swapRect( currRect, Horizontal, startOffset, (QDockArea*)w );
	if ( drawRect ) {
	    unclippedPainter->setPen( QPen( gray, 3 ) );
	    QRect dr(currRect);
#ifdef MAC_DRAG_HACK
	    dr.moveBy(-topLevelWidget()->geometry().x(), -topLevelWidget()->geometry().y());
#endif
	    unclippedPainter->drawRect( dr );
	} else {
	    QPoint mp( mapToGlobal( pos ));
	    if(place() == InDock) {
		undock();
		if(titleBar) {
		    mp = QPoint(titleBar->width() / 2, titleBar->height() / 2);
		    QMouseEvent me(QEvent::MouseButtonPress, mp, LeftButton, 0);
		    QApplication::sendEvent(titleBar, &me);
		    mp = titleBar->mapToGlobal( mp );
		}
	    }
	    move( mp );
	}
	state = OutsideDock;
        return;
    }

    QDockArea *area = (QDockArea*)w;
    if( area->isVisible() ) {
        state = InDock;
	Orientation o = ( area ? area->orientation() :
			  ( boxLayout()->direction() == QBoxLayout::LeftToRight ||
			    boxLayout()->direction() == QBoxLayout::RightToLeft ?
			    Horizontal : Vertical ) );
	if ( startOrientation != o )
	    swapRect( currRect, o, startOffset, area );
	if ( drawRect ) {
	    unclippedPainter->setPen( QPen( gray, 1 ) );
	    QRect dr(currRect);
#ifdef MAC_DRAG_HACK
	    dr.moveBy(-topLevelWidget()->geometry().x(), -topLevelWidget()->geometry().y());
#endif
	    unclippedPainter->drawRect( dr );
	}
	tmpDockArea = area;
    }
}

void QDockWindow::updateGui()
{
    if ( curPlace == OutsideDock ) {
	hbox->setMargin( 2 );
	horHandle->hide();
	verHandle->hide();
	if ( moveEnabled )
	    titleBar->show();
	else
	    titleBar->hide();
	titleBar->updateGui();
	hHandleTop->hide();
	vHandleLeft->hide();
	hHandleBottom->hide();
	vHandleRight->hide();
	setLineWidth( 2 );
	widgetResizeHandler->setActive( isResizeEnabled() );
    } else {
	hbox->setMargin( isResizeEnabled() ? 0 : 2 );
	titleBar->hide();
	if ( orientation() == Horizontal ) {
	    horHandle->hide();
	    if ( moveEnabled )
		verHandle->show();
	    else
		verHandle->hide();
#ifdef Q_WS_MAC
	    if(horHandle->mousePressed) {
		horHandle->mousePressed = FALSE;
		verHandle->mousePressed = TRUE;
		verHandle->grabMouse();
	    }
#endif
	    verHandle->updateGui();
	} else {
	    if ( moveEnabled )
		horHandle->show();
	    else
		horHandle->hide();
	    horHandle->updateGui();
#ifdef Q_WS_MAC
	    if(verHandle->mousePressed) {
		verHandle->mousePressed = FALSE;
		horHandle->mousePressed = TRUE;
		horHandle->grabMouse();
	    }
#endif
	    verHandle->hide();
	}
	if ( isResizeEnabled() ) {
	    if ( orientation() == Horizontal ) {
		hHandleBottom->raise();
		hHandleTop->raise();
	    } else {
		vHandleRight->raise();
		vHandleLeft->raise();
	    }

	    if ( area() ) {
		if ( orientation() == Horizontal ) {
		    if ( area()->handlePosition() == QDockArea::Normal ) {
			hHandleBottom->show();
			hHandleTop->hide();
		    } else {
			hHandleTop->show();
			hHandleBottom->hide();
		    }
		    if ( !area()->isLastDockWindow( this ) )
			vHandleRight->show();
		    else
			vHandleRight->hide();
		    vHandleLeft->hide();
		} else {
		    if ( (area()->handlePosition() == QDockArea::Normal) != QApplication::reverseLayout() ) {
			vHandleRight->show();
			vHandleLeft->hide();
		    } else {
			vHandleLeft->show();
			vHandleRight->hide();
		    }
		    if ( !area()->isLastDockWindow( this ) )
			hHandleBottom->show();
		    else
			hHandleBottom->hide();
		    hHandleTop->hide();
		}
	    }
	} else if ( area() ) { // hide resize handles if resizing is disabled
            if ( orientation() == Horizontal ) {
                hHandleTop->hide();
                hHandleBottom->hide();
            } else {
                vHandleLeft->hide();
                vHandleRight->hide();
            }
        }
#ifndef Q_OS_TEMP
	if ( moveEnabled )
	    setLineWidth( 1 );
	else
	    setLineWidth( 0 );
	hbox->setMargin( lineWidth() );
#else
	hbox->setMargin( 2 );
#endif
	widgetResizeHandler->setActive( FALSE );
    }
}

void QDockWindow::updatePosition( const QPoint &globalPos )
{
    if ( curPlace == OutsideDock && state == InDock )
	lastSize = size();

    bool doAdjustSize = curPlace != state && state == OutsideDock;
    bool doUpdate = TRUE;
    bool doOrientationChange = TRUE;
    if ( state != curPlace && state == InDock ) {
        doUpdate = FALSE;
        curPlace = state;
	updateGui();
	QApplication::sendPostedEvents();
    }
    Orientation oo = orientation();

    if ( state == InDock ) {
	if ( tmpDockArea ) {
	    bool differentDocks = FALSE;
	    if ( dockArea && dockArea != tmpDockArea ) {
		differentDocks = TRUE;
		delete (QDockArea::DockWindowData*)dockWindowData;
		dockWindowData = dockArea->dockWindowData( this );
		dockArea->removeDockWindow( this, FALSE, FALSE );
	    }
	    dockArea = tmpDockArea;
	    if ( differentDocks ) {
		if ( doUpdate ) {
		    doUpdate = FALSE;
                    curPlace = state;
		    updateGui();
		}
		emit orientationChanged( tmpDockArea->orientation() );
		doOrientationChange = FALSE;
	    } else {
		updateGui();
	    }
	    dockArea->moveDockWindow( this, globalPos, currRect, startOrientation != oo );
	}
    } else {
	if ( dockArea ) {
	    QMainWindow *mw = (QMainWindow*)dockArea->parentWidget();
	    if ( ::qt_cast<QMainWindow*>(mw) &&
		 ( !mw->isDockEnabled( QMainWindow::DockTornOff ) ||
		   !mw->isDockEnabled( this, QMainWindow::DockTornOff ) ) )
		return;
	    delete (QDockArea::DockWindowData*)dockWindowData;
	    dockWindowData = dockArea->dockWindowData( this );
	    dockArea->removeDockWindow( this, TRUE,
                                        startOrientation != Horizontal && ::qt_cast<QToolBar*>(this) );
	}
	dockArea = 0;
	QPoint topLeft = currRect.topLeft();
	QRect screen = qApp->desktop()->availableGeometry( topLeft );
	if ( !screen.contains( topLeft ) ) {
	    topLeft.setY(QMAX(topLeft.y(), screen.top()));
	    topLeft.setY(QMIN(topLeft.y(), screen.bottom()-height()));
	    topLeft.setX(QMAX(topLeft.x(), screen.left()));
	    topLeft.setX(QMIN(topLeft.x(), screen.right()-width()));
	}
	move( topLeft );
    }

    if ( curPlace == InDock && state == OutsideDock && !::qt_cast<QToolBar*>(this) ) {
	if ( lastSize != QSize( -1, -1 ) )
	    resize( lastSize );
    }

    if ( doUpdate ) {
        curPlace = state;
	updateGui();
    }
    if ( doOrientationChange )
	emit orientationChanged( orientation() );
    tmpDockArea = 0;
    if ( doAdjustSize ) {
	QApplication::sendPostedEvents( this, QEvent::LayoutHint );
	if ( ::qt_cast<QToolBar*>(this) )
	    adjustSize();
        if (lastSize == QSize(-1, -1))
            clearWState(WState_Resized); // Ensures size is recalculated (non-opaque).
	show();
	if ( parentWidget() && isTopLevel() )
	    parentWidget()->setActiveWindow();

    }

    emit placeChanged( curPlace );
}

/*!
    Sets the dock window's main widget to \a w.

    \sa boxLayout()
*/

void QDockWindow::setWidget( QWidget *w )
{
    wid = w;
    boxLayout()->addWidget( w );
    updateGui();
}

/*!
    Returns the dock window's main widget.

    \sa setWidget()
*/

QWidget *QDockWindow::widget() const
{
    return wid;
}

void QDockWindow::startRectDraw( const QPoint &so, bool drawRect )
{
    state = place();
    if ( unclippedPainter )
	endRectDraw( !opaque );
#ifdef MAC_DRAG_HACK
    QWidget *paint_on = topLevelWidget();
#else
    int scr = QApplication::desktop()->screenNumber( this );
    QWidget *paint_on = QApplication::desktop()->screen( scr );
#endif
    unclippedPainter = new QPainter( paint_on, TRUE );
    unclippedPainter->setPen( QPen( gray, curPlace == OutsideDock ? 3 : 1 ) );
    unclippedPainter->setRasterOp( XorROP );
    currRect = QRect( realWidgetPos( this ), size() );
    if ( drawRect ) {
	QRect dr(currRect);
#ifdef MAC_DRAG_HACK
	dr.moveBy(-topLevelWidget()->geometry().x(), -topLevelWidget()->geometry().y());
#endif
	unclippedPainter->drawRect( dr );
    }
    startOrientation = orientation();
    startOffset = mapFromGlobal( so );
}

void QDockWindow::endRectDraw( bool drawRect )
{
    if ( !unclippedPainter )
	return;
    if ( drawRect ) {
	QRect dr(currRect);
#ifdef MAC_DRAG_HACK
	dr.moveBy(-topLevelWidget()->geometry().x(), -topLevelWidget()->geometry().y());
#endif
	unclippedPainter->drawRect( dr );
    }
    delete unclippedPainter;
    unclippedPainter = 0;
}

/*!
  \reimp
*/
void QDockWindow::drawFrame( QPainter *p )
{
    if ( place() == InDock ) {
	QFrame::drawFrame( p );
	return;
    }

    QStyle::SFlags flags = QStyle::Style_Default;
    QStyleOption opt(lineWidth(),midLineWidth());

    if ( titleBar->isActive() )
	flags |= QStyle::Style_Active;

    style().drawPrimitive( QStyle::PE_WindowFrame, p, rect(), colorGroup(), flags, opt );
}

/*!
  \reimp
*/
void QDockWindow::drawContents( QPainter *p )
{
    QStyle::SFlags flags = QStyle::Style_Default;
    if ( titleBar->isActive() )
	flags |= QStyle::Style_Active;
    style().drawControl( QStyle::CE_DockWindowEmptyArea, p, this,
			 rect(), colorGroup(), flags );
}

/*!
    \property QDockWindow::resizeEnabled
    \brief whether the dock window is resizeable

    A resizeable dock window can be resized using splitter-like
    handles inside a dock area and like every other top level window
    when floating.

    A dock window is both horizontally and vertically stretchable if
    you call setResizeEnabled(TRUE).

    This property is FALSE by default.

    \sa setVerticallyStretchable() setHorizontallyStretchable()
*/

void QDockWindow::setResizeEnabled( bool b )
{
    resizeEnabled = b;
    hbox->setMargin( b ? 0 : 2 );
    updateGui();
}

/*!
    \property QDockWindow::movingEnabled
    \brief whether the user can move the dock window within the dock
    area, move the dock window to another dock area, or float the dock
    window.

    This property is TRUE by default.
*/

void QDockWindow::setMovingEnabled( bool b )
{
    moveEnabled = b;
    updateGui();
}

bool QDockWindow::isResizeEnabled() const
{
    return resizeEnabled;
}

bool QDockWindow::isMovingEnabled() const
{
    return moveEnabled;
}

/*!
    \property QDockWindow::closeMode
    \brief the close mode of a dock window

    Defines when (if ever) the dock window has a close button. The
    choices are \c Never, \c Docked (i.e. only when docked), \c
    Undocked (only when undocked, i.e. floated) or \c Always.

    The default is \c Never.
*/

void QDockWindow::setCloseMode( int m )
{
    cMode = m;
    if ( place() == InDock ) {
	horHandle->updateGui();
	verHandle->updateGui();
    } else {
	titleBar->updateGui();
    }
}

/*!
    Returns TRUE if the dock window has a close button; otherwise
    returns FALSE. The result depends on the dock window's \l Place
    and its \l CloseMode.

    \sa setCloseMode()
*/

bool QDockWindow::isCloseEnabled() const
{
    return  ( ( cMode & Docked ) == Docked && place() == InDock ||
	      ( cMode & Undocked ) == Undocked && place() == OutsideDock );
}

int QDockWindow::closeMode() const
{
    return cMode;
}

/*!
    \property QDockWindow::horizontallyStretchable
    \brief whether the dock window is horizontally stretchable.

    A dock window is horizontally stretchable if you call
    setHorizontallyStretchable(TRUE) or setResizeEnabled(TRUE).

    \sa setResizeEnabled()

    \bug Strecthability is broken. You must call setResizeEnabled(TRUE) to get
    proper behavior and even then QDockWindow does not limit stretchablilty.
*/

void QDockWindow::setHorizontallyStretchable( bool b )
{
    stretchable[ Horizontal ] = b;
}

/*!
    \property QDockWindow::verticallyStretchable
    \brief whether the dock window is vertically stretchable.

    A dock window is vertically stretchable if you call
    setVerticallyStretchable(TRUE) or setResizeEnabled(TRUE).

    \sa setResizeEnabled()

    \bug Strecthability is broken. You must call setResizeEnabled(TRUE) to get
    proper behavior and even then QDockWindow does not limit stretchablilty.
*/

void QDockWindow::setVerticallyStretchable( bool b )
{
    stretchable[ Vertical ] = b;
}

bool QDockWindow::isHorizontallyStretchable() const
{
    return isResizeEnabled() || stretchable[ Horizontal ];
}

bool QDockWindow::isVerticallyStretchable() const
{
    return isResizeEnabled() || stretchable[ Vertical ];
}

/*!
    \property QDockWindow::stretchable
    \brief whether the dock window is stretchable in the current
    orientation()

    This property can be set using setHorizontallyStretchable() and
    setVerticallyStretchable(), or with setResizeEnabled().

    \sa setResizeEnabled()

    \bug Strecthability is broken. You must call setResizeEnabled(TRUE) to get
    proper behavior and even then QDockWindow does not limit stretchablilty.
*/

bool QDockWindow::isStretchable() const
{
    if ( orientation() == Horizontal )
	return isHorizontallyStretchable();
    return isVerticallyStretchable();
}

/*!
    Returns the orientation of the dock window.

    \sa orientationChanged()
*/

Qt::Orientation QDockWindow::orientation() const
{
    if ( dockArea )
	return dockArea->orientation();
    if ( ::qt_cast<QToolBar*>(this) )
	return Horizontal;
    return ( ((QDockWindow*)this)->boxLayout()->direction() == QBoxLayout::LeftToRight ||
	     ((QDockWindow*)this)->boxLayout()->direction() == QBoxLayout::RightToLeft ?
	     Horizontal : Vertical );
}

int QDockWindow::offset() const
{
    return offs;
}

/*!
    \property QDockWindow::offset
    \brief the dock window's preferred offset from the dock area's
    left edge (top edge for vertical dock areas)

    The default is 0.
*/

void QDockWindow::setOffset( int o )
{
    offs = o;
}

/*!
    Returns the dock window's preferred size (fixed extent).

    \sa setFixedExtentWidth() setFixedExtentHeight()
*/

QSize QDockWindow::fixedExtent() const
{
    return fExtent;
}

/*!
    Sets the dock window's preferred width for its fixed extent (size)
    to \a w.

    \sa setFixedExtentHeight()
*/

void QDockWindow::setFixedExtentWidth( int w )
{
    fExtent.setWidth( w );
}

/*!
    Sets the dock window's preferred height for its fixed extent
    (size) to \a h.

    \sa setFixedExtentWidth()
*/

void QDockWindow::setFixedExtentHeight( int h )
{
    fExtent.setHeight( h );
}

/*!
    \property QDockWindow::newLine
    \brief whether the dock window prefers to start a new line in the
    dock area.

    The default is FALSE, i.e. the dock window doesn't require a new
    line in the dock area.
*/

void QDockWindow::setNewLine( bool b )
{
    nl = b;
}

bool QDockWindow::newLine() const
{
    return nl;
}

/*!
    Returns the layout which is used for adding widgets to the dock
    window. The layout's orientation is set automatically to match the
    orientation of the dock window. You can add widgets to the layout
    using the box layout's QBoxLayout::addWidget() function.

    If the dock window only needs to contain a single widget use
    setWidget() instead.

    \sa setWidget() setOrientation()
*/

QBoxLayout *QDockWindow::boxLayout()
{
    return childBox;
}

/*! \reimp
 */

QSize QDockWindow::sizeHint() const
{
    QSize sh( QFrame::sizeHint() );
    if ( place() == InDock )
	sh = sh.expandedTo( fixedExtent() );
    sh = sh.expandedTo( QSize( 16, 16 ) );
    if ( area() ) {
	if ( area()->orientation() == Horizontal && !vHandleRight->isVisible() )
	    sh.setWidth( sh.width() + 2 * style().pixelMetric(QStyle::PM_SplitterWidth, this) / 3 );
	else if ( area()->orientation() == Vertical && !hHandleBottom->isVisible() )
	    sh.setHeight( sh.height() + 2 * style().pixelMetric(QStyle::PM_SplitterWidth, this) / 3 );
    }
    return sh;
}

/*! \reimp
 */

QSize QDockWindow::minimumSize() const
{
    QSize ms( QFrame::minimumSize() );
    if ( place() == InDock )
	ms = ms.expandedTo( fixedExtent() );
    ms = ms.expandedTo( QSize( 16, 16 ) );
    if ( area() ) {
	if ( area()->orientation() == Horizontal && !vHandleRight->isVisible() )
	    ms.setWidth( ms.width() + 2 * style().pixelMetric(QStyle::PM_SplitterWidth, this) / 3 );
	else if ( area()->orientation() == Vertical && !hHandleBottom->isVisible() )
	    ms.setHeight( ms.height() + 2 * style().pixelMetric(QStyle::PM_SplitterWidth, this) / 3 );
    }
    return ms;
}

/*! \reimp
 */

QSize QDockWindow::minimumSizeHint() const
{
    QSize msh( QFrame::minimumSize() );
    if ( place() == InDock )
	msh = msh.expandedTo( fixedExtent() );
    msh = msh.expandedTo( QSize( 16, 16 ) );
    if ( area() ) {
	if ( area()->orientation() == Horizontal && !vHandleRight->isVisible() )
	    msh.setWidth( msh.width() + 2 * style().pixelMetric(QStyle::PM_SplitterWidth, this) / 3 );
	else if ( area()->orientation() == Vertical && !hHandleBottom->isVisible() )
	    msh.setHeight( msh.height() + 2 * style().pixelMetric(QStyle::PM_SplitterWidth, this) / 3 );
    }
    return msh;
}

/*! \internal */
void QDockWindow::undock( QWidget *w )
{
    QMainWindow *mw = 0;
    if ( area() )
	mw = ::qt_cast<QMainWindow*>(area()->parentWidget());
    if ( mw && !mw->isDockEnabled( this, DockTornOff ) )
	return;
    if ( (place() == OutsideDock && !w) )
	return;

    QPoint p( 50, 50 );
    if ( topLevelWidget() )
	p = topLevelWidget()->pos() + QPoint( 20, 20 );
    if ( dockArea ) {
	delete (QDockArea::DockWindowData*)dockWindowData;
	dockWindowData = dockArea->dockWindowData( this );
	dockArea->removeDockWindow( this, TRUE, orientation() != Horizontal && ::qt_cast<QToolBar*>(this) );
    }
    dockArea = 0;
    if ( lastPos != QPoint( -1, -1 ) && lastPos.x() > 0 && lastPos.y() > 0 )
	move( lastPos );
    else
	move( p );
    if ( lastSize != QSize( -1, -1 ) )
	resize( lastSize );
    curPlace = OutsideDock;
    updateGui();
    emit orientationChanged( orientation() );
    QApplication::sendPostedEvents( this, QEvent::LayoutHint );
    if ( ::qt_cast<QToolBar*>(this) )
	adjustSize();
    if ( !w ) {
	if ( !parentWidget() || parentWidget()->isVisible() ) {
            if (lastSize == QSize(-1, -1))
                clearWState(WState_Resized); // Ensures size is recalculated (opaque).
	    show();
	}
    } else {
	reparent( w, 0, QPoint( 0, 0 ), FALSE );
	move( -width() - 5, -height() - 5 );
	resize( 1, 1 );
	show();
    }
    if ( parentWidget() && isTopLevel() )
	parentWidget()->setActiveWindow();
    emit placeChanged( place() );
}

/*!
    \fn void QDockWindow::undock()

    Undocks the QDockWindow from its current dock area if it is
    docked; otherwise does nothing.

    \sa dock() QDockArea::moveDockWindow(),
    QDockArea::removeDockWindow(), QMainWindow::moveDockWindow(),
    QMainWindow::removeDockWindow()
*/

void QDockWindow::removeFromDock( bool fixNewLines )
{
    if ( dockArea )
	dockArea->removeDockWindow( this, FALSE, FALSE, fixNewLines );
}

/*!
    Docks the dock window into the last dock area in which it was
    docked.

    If the dock window has no last dock area (e.g. it was created as a
    floating window and has never been docked), or if the last dock
    area it was docked in does not exist (e.g. the dock area has been
    deleted), nothing happens.

    The dock window will dock with the dock area regardless of the return value
    of QDockArea::isDockWindowAccepted().

    \sa undock() QDockArea::moveDockWindow(),
    QDockArea::removeDockWindow(), QMainWindow::moveDockWindow(),
    QMainWindow::removeDockWindow(), QDockArea::isDockWindowAccepted()

*/

void QDockWindow::dock()
{
    if ( !(QDockArea::DockWindowData*)dockWindowData ||
	 !( (QDockArea::DockWindowData*)dockWindowData )->area )
	return;
    curPlace = InDock;
    lastPos = pos();
    lastSize = size();
    ( (QDockArea::DockWindowData*)dockWindowData )->
	area->dockWindow( this, (QDockArea::DockWindowData*)dockWindowData );
    emit orientationChanged( orientation() );
    emit placeChanged( place() );
}

/*! \reimp
 */

void QDockWindow::hideEvent( QHideEvent *e )
{
    QFrame::hideEvent( e );
}

/*! \reimp
 */

void QDockWindow::showEvent( QShowEvent *e )
{
    if (curPlace == OutsideDock && (parent() && strcmp(parent()->name(), "qt_hide_dock") != 0)) {
	QRect sr = qApp->desktop()->availableGeometry( this );
	if ( !sr.contains( pos() ) ) {
	    int nx = QMIN( QMAX( x(), sr.x() ), sr.right()-width() );
	    int ny = QMIN( QMAX( y(), sr.y() ), sr.bottom()-height() );
	    move( nx, ny );
	}
    }

    QFrame::showEvent( e );
}

/*!
    \property QDockWindow::opaqueMoving
    \brief whether the dock window will be shown normally whilst it is
    being moved.

    If this property is FALSE, (the default), the dock window will be
    represented by an outline rectangle whilst it is being moved.

    \warning Currently opaque moving has some problems and we do not
    recommend using it at this time. We expect to fix these problems
    in a future release.
*/

void QDockWindow::setOpaqueMoving( bool b )
{
    opaque = b;
    horHandle->setOpaqueMoving( b );
    verHandle->setOpaqueMoving( b );
    titleBar->setOpaqueMoving( b );
}

bool QDockWindow::opaqueMoving() const
{
    return opaque;
}

/*! \reimp */

void QDockWindow::setCaption( const QString &s )
{
    titleBar->setCaption( s );
    verHandle->update();
    horHandle->update();
#ifndef QT_NO_WIDGET_TOPEXTRA
    QFrame::setCaption( s );
#endif
#ifndef QT_NO_TOOLTIP
    QToolTip::remove( horHandle );
    QToolTip::remove( verHandle );
    if ( !s.isEmpty() ) {
	QToolTip::add( horHandle, s );
	QToolTip::add( verHandle, s );
    }
#endif
}

void QDockWindow::updateSplitterVisibility( bool visible )
{
    if ( area() && isResizeEnabled() ) {
	if ( orientation() == Horizontal ) {
	    if ( visible )
		vHandleRight->show();
	    else
		vHandleRight->hide();
	    vHandleLeft->hide();
	} else {
	    if ( visible )
		hHandleBottom->show();
	    else
		hHandleBottom->hide();
	    hHandleTop->hide();
	}
    }
}

/*! \reimp */
bool QDockWindow::eventFilter( QObject * o, QEvent *e )
{
    if ( !o->isWidgetType() )
	return FALSE;

    if ( e->type() == QEvent::KeyPress &&
	( horHandle->mousePressed ||
	  verHandle->mousePressed ||
	  titleBar->mousePressed ) ) {
	QKeyEvent *ke = (QKeyEvent*)e;
	if ( ke->key() == Key_Escape ) {
	    horHandle->mousePressed =
		verHandle->mousePressed =
		    titleBar->mousePressed = FALSE;
	    endRectDraw( !opaque );
	    qApp->removeEventFilter( this );
	    return TRUE;
	}
    } else if ( ((QWidget*)o)->topLevelWidget() != this && place() == OutsideDock && isTopLevel() ) {
	if ( (e->type() == QEvent::WindowDeactivate ||
	    e->type() == QEvent::WindowActivate ) )
	    event( e );
    }
    return FALSE;
}

/*! \reimp */
bool QDockWindow::event( QEvent *e )
{
    switch ( e->type() ) {
    case QEvent::WindowDeactivate:
	if ( place() == OutsideDock && isTopLevel() && parentWidget()
	     && parentWidget()->isActiveWindow() )
	    return TRUE;
    case QEvent::Hide:
	if ( !isHidden() )
	    break;
	// fall through
    case QEvent::HideToParent:
	emit visibilityChanged( FALSE );
	break;
    case QEvent::Show:
	if ( e->spontaneous() )
	    break;
    case QEvent::ShowToParent:
	emit visibilityChanged( TRUE );
	break;
    default:
	break;
    }
    return QFrame::event( e );
}

#ifdef QT_NO_WIDGET_TOPEXTRA
QString QDockWindow::caption() const
{
    return titleBar->caption();
}
#endif

/*! \reimp */
void QDockWindow::contextMenuEvent( QContextMenuEvent *e )
{
    QObject *o = this;
    while ( o ) {
	if ( ::qt_cast<QMainWindow*>(o) )
	    break;
	o = o->parent();
    }
    if ( !o || ! ( (QMainWindow*)o )->showDockMenu( e->globalPos() ) )
	e->ignore();
}

#include "qdockwindow.moc"

#endif //QT_NO_MAINWINDOW
