/****************************************************************************
** $Id: qwidgetresizehandler.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of the QWidgetResizeHandler class
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

#include "qwidgetresizehandler_p.h"

#ifndef QT_NO_RESIZEHANDLER
#include "qframe.h"
#include "qapplication.h"
#include "qcursor.h"
#include "qsizegrip.h"
#if defined(Q_WS_WIN)
#include "qt_windows.h"
#endif

#define RANGE 4

static bool resizeHorizontalDirectionFixed = FALSE;
static bool resizeVerticalDirectionFixed = FALSE;

QWidgetResizeHandler::QWidgetResizeHandler( QWidget *parent, QWidget *cw, const char *name )
    : QObject( parent, name ), widget( parent ), childWidget( cw ? cw : parent ),
      extrahei( 0 ), buttonDown( FALSE ), moveResizeMode( FALSE ), sizeprotect( TRUE ), moving( TRUE )
{
    mode = Nowhere;
    widget->setMouseTracking( TRUE );
    QFrame *frame = ::qt_cast<QFrame*>(widget);
    range = frame ? frame->frameWidth() : RANGE;
    range = QMAX( RANGE, range );
    activeForMove = activeForResize = TRUE;
    qApp->installEventFilter( this );
}

void QWidgetResizeHandler::setActive( Action ac, bool b )
{
    if ( ac & Move )
	activeForMove = b;
    if ( ac & Resize )
	activeForResize = b; 

    if ( !isActive() )
	setMouseCursor( Nowhere );
}

bool QWidgetResizeHandler::isActive( Action ac ) const
{
    bool b = FALSE;
    if ( ac & Move ) b = activeForMove;
    if ( ac & Resize ) b |= activeForResize;

    return b;
}

static QWidget *childOf( QWidget *w, QWidget *child )
{
    while ( child ) {
	if ( child == w )
	    return child;
	if ( child->isTopLevel() )
	    break;
	child = child->parentWidget();
    }
    return 0;
}

bool QWidgetResizeHandler::eventFilter( QObject *o, QEvent *ee )
{
    if ( !isActive() || !o->isWidgetType() || !ee->spontaneous())
	return FALSE;

    if ( ee->type() != QEvent::MouseButtonPress &&
	 ee->type() != QEvent::MouseButtonRelease &&
	 ee->type() != QEvent::MouseMove &&
	 ee->type() != QEvent::KeyPress &&
	 ee->type() != QEvent::AccelOverride )
	return FALSE;

    QWidget *w = childOf( widget, (QWidget*)o );
    if ( !w
#ifndef QT_NO_SIZEGRIP
	 || ::qt_cast<QSizeGrip*>(o)
#endif
	 || qApp->activePopupWidget() ) {
	if ( buttonDown && ee->type() == QEvent::MouseButtonRelease )
	    buttonDown = FALSE;
	return FALSE;
    }

    QMouseEvent *e = (QMouseEvent*)ee;
    switch ( e->type() ) {
    case QEvent::MouseButtonPress: {
	if ( w->isMaximized() )
	    break;
	if ( !widget->rect().contains( widget->mapFromGlobal( e->globalPos() ) ) )
	    return FALSE;
	if ( e->button() == LeftButton ) {
	    emit activate();
	    bool me = isMovingEnabled();
	    setMovingEnabled( me && o == widget );
	    mouseMoveEvent( e );
	    setMovingEnabled( me );
	    buttonDown = TRUE;
	    moveOffset = widget->mapFromGlobal( e->globalPos() );
	    invertedMoveOffset = widget->rect().bottomRight() - moveOffset;
	}
    } break;
    case QEvent::MouseButtonRelease:
	if ( w->isMaximized() )
	    break;
	if ( e->button() == LeftButton ) {
	    moveResizeMode = FALSE;
	    buttonDown = FALSE;
	    widget->releaseMouse();
	    widget->releaseKeyboard();
	}
	break;
    case QEvent::MouseMove: {
	if ( w->isMaximized() )
	    break;
	bool me = isMovingEnabled();
	setMovingEnabled( me && o == widget );
	mouseMoveEvent( e );
	setMovingEnabled( me );
	if ( buttonDown && mode != Center )
	    return TRUE;
    } break;
    case QEvent::KeyPress:
	keyPressEvent( (QKeyEvent*)e );
	break;
    case QEvent::AccelOverride:
	if ( buttonDown ) {
	    ((QKeyEvent*)ee)->accept();
	    return TRUE;
	}
	break;
    default:
	break;
    }
    return FALSE;
}

void QWidgetResizeHandler::mouseMoveEvent( QMouseEvent *e )
{
    QPoint pos = widget->mapFromGlobal( e->globalPos() );
    if ( !moveResizeMode && ( !buttonDown || ( e->state() & LeftButton ) == 0 ) ) {
	if ( pos.y() <= range && pos.x() <= range)
	    mode = TopLeft;
	else if ( pos.y() >= widget->height()-range && pos.x() >= widget->width()-range)
	    mode = BottomRight;
	else if ( pos.y() >= widget->height()-range && pos.x() <= range)
	    mode = BottomLeft;
	else if ( pos.y() <= range && pos.x() >= widget->width()-range)
	    mode = TopRight;
	else if ( pos.y() <= range )
	    mode = Top;
	else if ( pos.y() >= widget->height()-range )
	    mode = Bottom;
	else if ( pos.x() <= range )
	    mode = Left;
	else if (  pos.x() >= widget->width()-range )
	    mode = Right;
	else
	    mode = Center;

	if ( widget->isMinimized() || !isActive(Resize) )
	    mode = Center;
#ifndef QT_NO_CURSOR
	setMouseCursor( mode );
#endif
	return;
    }

    if ( buttonDown && !isMovingEnabled() && mode == Center && !moveResizeMode )
	return;

    if ( widget->testWState( WState_ConfigPending ) )
 	return;

    QPoint globalPos = widget->parentWidget( TRUE ) ?
		       widget->parentWidget( TRUE )->mapFromGlobal( e->globalPos() ) : e->globalPos();
    if ( widget->parentWidget( TRUE ) && !widget->parentWidget( TRUE )->rect().contains( globalPos ) ) {
	if ( globalPos.x() < 0 )
	    globalPos.rx() = 0;
	if ( globalPos.y() < 0 )
	    globalPos.ry() = 0;
	if ( sizeprotect && globalPos.x() > widget->parentWidget()->width() )
	    globalPos.rx() = widget->parentWidget()->width();
	if ( sizeprotect && globalPos.y() > widget->parentWidget()->height() )
	    globalPos.ry() = widget->parentWidget()->height();
    }

    QPoint p = globalPos + invertedMoveOffset;
    QPoint pp = globalPos - moveOffset;

    int fw = 0;
    int mw = QMAX( childWidget->minimumSizeHint().width(),
		   childWidget->minimumWidth() );
    int mh = QMAX( childWidget->minimumSizeHint().height(),
		   childWidget->minimumHeight() );
    if ( childWidget != widget ) {
	QFrame *frame = ::qt_cast<QFrame*>(widget);
	if ( frame )
	    fw = frame->frameWidth();
	mw += 2 * fw;
	mh += 2 * fw + extrahei;
    }

    QSize mpsize( widget->geometry().right() - pp.x() + 1,
		  widget->geometry().bottom() - pp.y() + 1 );
    mpsize = mpsize.expandedTo( widget->minimumSize() ).expandedTo( QSize(mw, mh) );
    QPoint mp( widget->geometry().right() - mpsize.width() + 1,
	       widget->geometry().bottom() - mpsize.height() + 1 );

    QRect geom = widget->geometry();

    switch ( mode ) {
    case TopLeft:
	geom = QRect( mp, widget->geometry().bottomRight() ) ;
	break;
    case BottomRight:
	geom = QRect( widget->geometry().topLeft(), p ) ;
	break;
    case BottomLeft:
	geom = QRect( QPoint(mp.x(), widget->geometry().y() ), QPoint( widget->geometry().right(), p.y()) ) ;
	break;
    case TopRight:
	geom = QRect( QPoint( widget->geometry().x(), mp.y() ), QPoint( p.x(), widget->geometry().bottom()) ) ;
	break;
    case Top:
	geom = QRect( QPoint( widget->geometry().left(), mp.y() ), widget->geometry().bottomRight() ) ;
	break;
    case Bottom:
	geom = QRect( widget->geometry().topLeft(), QPoint( widget->geometry().right(), p.y() ) ) ;
	break;
    case Left:
	geom = QRect( QPoint( mp.x(), widget->geometry().top() ), widget->geometry().bottomRight() ) ;
	break;
    case Right:
	geom = QRect( widget->geometry().topLeft(), QPoint( p.x(), widget->geometry().bottom() ) ) ;
	break;
    case Center:
	if ( isMovingEnabled() || moveResizeMode )
	    geom.moveTopLeft( pp );
	break;
    default:
	break;
    }

    QSize maxsize( childWidget->maximumSize() );
    if ( childWidget != widget )
	maxsize += QSize( 2 * fw, 2 * fw + extrahei );

    geom = QRect( geom.topLeft(),
		  geom.size().expandedTo( widget->minimumSize() )
			     .expandedTo( QSize(mw, mh) )
			     .boundedTo( maxsize ) );

    if ( geom != widget->geometry() &&
	( widget->isTopLevel() || widget->parentWidget()->rect().intersects( geom ) ) ) {
	if ( widget->isMinimized() )
	    widget->move( geom.topLeft() );
	else
	    widget->setGeometry( geom );
    }

#if defined(Q_WS_WIN)
    MSG msg;
    QT_WA( {
	while(PeekMessageW( &msg, widget->winId(), WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE ))
	    ;
    } , {
	while(PeekMessageA( &msg, widget->winId(), WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE ))
	    ;
    } );
#endif

    QApplication::syncX();
}

void QWidgetResizeHandler::setMouseCursor( MousePosition m )
{
#ifndef QT_NO_CURSOR
    switch ( m ) {
    case TopLeft:
    case BottomRight:
	widget->setCursor( sizeFDiagCursor );
	break;
    case BottomLeft:
    case TopRight:
	widget->setCursor( sizeBDiagCursor );
	break;
    case Top:
    case Bottom:
	widget->setCursor( sizeVerCursor );
	break;
    case Left:
    case Right:
	widget->setCursor( sizeHorCursor );
	break;
    default:
	widget->setCursor( arrowCursor );
	break;
    }
#endif
}

void QWidgetResizeHandler::keyPressEvent( QKeyEvent * e )
{
    if ( !isMove() && !isResize() )
	return;
    bool is_control = e->state() & ControlButton;
    int delta = is_control?1:8;
    QPoint pos = QCursor::pos();
    switch ( e->key() ) {
    case Key_Left:
	pos.rx() -= delta;
	if ( pos.x() <= QApplication::desktop()->geometry().left() ) {
	    if ( mode == TopLeft || mode == BottomLeft ) {
		moveOffset.rx() += delta;
		invertedMoveOffset.rx() += delta;
	    } else {
		moveOffset.rx() -= delta;
		invertedMoveOffset.rx() -= delta;
	    }
	}
	if ( isResize() && !resizeHorizontalDirectionFixed ) {
	    resizeHorizontalDirectionFixed = TRUE;
	    if ( mode == BottomRight )
		mode = BottomLeft;
	    else if ( mode == TopRight )
		mode = TopLeft;
#ifndef QT_NO_CURSOR
	    setMouseCursor( mode );
	    widget->grabMouse( widget->cursor() );
#else
	    widget->grabMouse();
#endif
	}
	break;
    case Key_Right:
	pos.rx() += delta;
	if ( pos.x() >= QApplication::desktop()->geometry().right() ) {
	    if ( mode == TopRight || mode == BottomRight ) {
		moveOffset.rx() += delta;
		invertedMoveOffset.rx() += delta;
	    } else {
		moveOffset.rx() -= delta;
		invertedMoveOffset.rx() -= delta;
	    }
	}
	if ( isResize() && !resizeHorizontalDirectionFixed ) {
	    resizeHorizontalDirectionFixed = TRUE;
	    if ( mode == BottomLeft )
		mode = BottomRight;
	    else if ( mode == TopLeft )
		mode = TopRight;
#ifndef QT_NO_CURSOR
	    setMouseCursor( mode );
	    widget->grabMouse( widget->cursor() );
#else
	    widget->grabMouse();
#endif
	}
	break;
    case Key_Up:
	pos.ry() -= delta;
	if ( pos.y() <= QApplication::desktop()->geometry().top() ) {
	    if ( mode == TopLeft || mode == TopRight ) {
		moveOffset.ry() += delta;
		invertedMoveOffset.ry() += delta;
	    } else {
		moveOffset.ry() -= delta;
		invertedMoveOffset.ry() -= delta;
	    }
	}
	if ( isResize() && !resizeVerticalDirectionFixed ) {
	    resizeVerticalDirectionFixed = TRUE;
	    if ( mode == BottomLeft )
		mode = TopLeft;
	    else if ( mode == BottomRight )
		mode = TopRight;
#ifndef QT_NO_CURSOR
	    setMouseCursor( mode );
	    widget->grabMouse( widget->cursor() );
#else
	    widget->grabMouse();
#endif
	}
	break;
    case Key_Down:
	pos.ry() += delta;
	if ( pos.y() >= QApplication::desktop()->geometry().bottom() ) {
	    if ( mode == BottomLeft || mode == BottomRight ) {
		moveOffset.ry() += delta;
		invertedMoveOffset.ry() += delta;
	    } else {
		moveOffset.ry() -= delta;
		invertedMoveOffset.ry() -= delta;
	    }
	}
	if ( isResize() && !resizeVerticalDirectionFixed ) {
	    resizeVerticalDirectionFixed = TRUE;
	    if ( mode == TopLeft )
		mode = BottomLeft;
	    else if ( mode == TopRight )
		mode = BottomRight;
#ifndef QT_NO_CURSOR
	    setMouseCursor( mode );
	    widget->grabMouse( widget->cursor() );
#else
	    widget->grabMouse();
#endif
	}
	break;
    case Key_Space:
    case Key_Return:
    case Key_Enter:
    case Key_Escape:
	moveResizeMode = FALSE;
	widget->releaseMouse();
	widget->releaseKeyboard();
	buttonDown = FALSE;
	break;
    default:
	return;
    }
    QCursor::setPos( pos );
}


void QWidgetResizeHandler::doResize()
{
    if ( !activeForResize )
	return;

    moveResizeMode = TRUE;
    buttonDown = TRUE;
    moveOffset = widget->mapFromGlobal( QCursor::pos() );
    if ( moveOffset.x() < widget->width()/2) {
	if ( moveOffset.y() < widget->height()/2)
	    mode = TopLeft;
	else
	    mode = BottomLeft;
    } else {
	if ( moveOffset.y() < widget->height()/2)
	    mode = TopRight;
	else
	    mode = BottomRight;
    }
    invertedMoveOffset = widget->rect().bottomRight() - moveOffset;
#ifndef QT_NO_CURSOR
    setMouseCursor( mode );
    widget->grabMouse( widget->cursor()  );
#else
    widget->grabMouse();
#endif
    widget->grabKeyboard();
    resizeHorizontalDirectionFixed = FALSE;
    resizeVerticalDirectionFixed = FALSE;
}

void QWidgetResizeHandler::doMove()
{
    if ( !activeForMove )
	return;

    mode = Center;
    moveResizeMode = TRUE;
    buttonDown = TRUE;
    moveOffset = widget->mapFromGlobal( QCursor::pos() );
    invertedMoveOffset = widget->rect().bottomRight() - moveOffset;
#ifndef QT_NO_CURSOR
    widget->grabMouse( SizeAllCursor );
#else
    widget->grabMouse();
#endif
    widget->grabKeyboard();
}

#endif //QT_NO_RESIZEHANDLER
