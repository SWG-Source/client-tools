/****************************************************************************
** $Id: qsizegrip.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QSizeGrip class
**
** Created : 980119
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

#include "qsizegrip.h"

#ifndef QT_NO_SIZEGRIP

#include "qpainter.h"
#include "qapplication.h"
#include "qstyle.h"

#if defined(Q_WS_X11)
#include "qt_x11_p.h"
extern Atom qt_sizegrip;			// defined in qapplication_x11.cpp
#elif defined (Q_WS_WIN )
#include "qobjectlist.h"
#include "qt_windows.h"
#elif defined(Q_WS_MAC)
bool qt_mac_update_sizer(QWidget *, int); //qwidget_mac.cpp
#endif


static QWidget *qt_sizegrip_topLevelWidget( QWidget* w)
{
    QWidget *p = w->parentWidget();
    while ( !w->isTopLevel() && p && !p->inherits("QWorkspace") ) {
	w = p;
	p = p->parentWidget();
    }
    return w;
}

static QWidget* qt_sizegrip_workspace( QWidget* w )
{
    while ( w && !w->inherits("QWorkspace" ) ) {
	if ( w->isTopLevel() )
	    return 0;
	w = w->parentWidget();
    }
    return w;
}


/*!
    \class QSizeGrip qsizegrip.h

    \brief The QSizeGrip class provides a corner-grip for resizing a top-level window.

    \ingroup application
    \ingroup basic
    \ingroup appearance

    This widget works like the standard Windows resize handle. In the
    X11 version this resize handle generally works differently from
    the one provided by the system; we hope to reduce this difference
    in the future.

    Put this widget anywhere in a widget tree and the user can use it
    to resize the top-level window. Generally, this should be in the
    lower right-hand corner. Note that QStatusBar already uses this
    widget, so if you have a status bar (e.g. you are using
    QMainWindow), then you don't need to use this widget explicitly.

    <img src=qsizegrip-m.png> <img src=qsizegrip-w.png>

    \sa QStatusBar
*/


/*!
    Constructs a resize corner called \a name, as a child widget of \a
    parent.
*/
QSizeGrip::QSizeGrip( QWidget * parent, const char* name )
    : QWidget( parent, name )
{
#ifndef QT_NO_CURSOR
#ifndef Q_WS_MAC
    if ( QApplication::reverseLayout() )
	setCursor( sizeBDiagCursor );
    else
	setCursor( sizeFDiagCursor );
#endif
#endif
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
#if defined(Q_WS_X11)
    if ( !qt_sizegrip_workspace( this ) ) {
	WId id = winId();
	XChangeProperty(qt_xdisplay(), topLevelWidget()->winId(),
			qt_sizegrip, XA_WINDOW, 32, PropModeReplace,
			(unsigned char *)&id, 1);
    }
#endif
    tlw = qt_sizegrip_topLevelWidget( this );
    if ( tlw )
	tlw->installEventFilter( this );
    installEventFilter( this ); //for binary compatibility fix in 4.0 with an event() ###
}


/*!
    Destroys the size grip.
*/
QSizeGrip::~QSizeGrip()
{
#if defined(Q_WS_X11)
    if ( !QApplication::closingDown() && parentWidget() ) {
	WId id = None;
 	XChangeProperty(qt_xdisplay(), topLevelWidget()->winId(),
 			qt_sizegrip, XA_WINDOW, 32, PropModeReplace,
 			(unsigned char *)&id, 1);
    }
#endif
}

/*!
    Returns the size grip's size hint; this is a small size.
*/
QSize QSizeGrip::sizeHint() const
{
    return (style().sizeFromContents(QStyle::CT_SizeGrip, this, QSize(13, 13)).
	    expandedTo(QApplication::globalStrut()));
}

/*!
    Paints the resize grip. Resize grips are usually rendered as small
    diagonal textured lines in the lower-right corner. The event is in
    \a e.
*/
void QSizeGrip::paintEvent( QPaintEvent *e )
{
    QPainter painter( this );
    painter.setClipRegion(e->region());
    style().drawPrimitive(QStyle::PE_SizeGrip, &painter, rect(), colorGroup());
}

/*!
    Primes the resize operation. The event is in \a e.
*/
void QSizeGrip::mousePressEvent( QMouseEvent * e )
{
    p = e->globalPos();
    s = qt_sizegrip_topLevelWidget(this)->size();
}


/*!
    Resizes the top-level widget containing this widget. The event is
    in \a e.
*/
void QSizeGrip::mouseMoveEvent( QMouseEvent * e )
{
    if ( e->state() != LeftButton )
	return;

    QWidget* tlw = qt_sizegrip_topLevelWidget(this);
    if ( tlw->testWState(WState_ConfigPending) )
	return;

    QPoint np( e->globalPos() );

    QWidget* ws = qt_sizegrip_workspace( this );
    if ( ws ) {
	QPoint tmp( ws->mapFromGlobal( np ) );
	if ( tmp.x() > ws->width() )
	    tmp.setX( ws->width() );
	if ( tmp.y() > ws->height() )
	    tmp.setY( ws->height() );
	np = ws->mapToGlobal( tmp );
    }

    int w;
    int h = np.y() - p.y() + s.height();

    if ( QApplication::reverseLayout() )
	w = s.width() - ( np.x() - p.x() );
    else
	w = np.x() - p.x() + s.width();

    if ( w < 1 )
	w = 1;
    if ( h < 1 )
	h = 1;
    QSize ms( tlw->minimumSizeHint() );
    ms = ms.expandedTo( minimumSize() );
    if ( w < ms.width() )
	w = ms.width();
    if ( h < ms.height() )
	h = ms.height();

    if (QApplication::reverseLayout()) {
	tlw->resize( w, h );
	if (tlw->size() == QSize(w,h))
	    tlw->move( tlw->x() + ( np.x()-p.x() ), tlw->y() );
    } else {
	tlw->resize( w, h );
    }
#ifdef Q_WS_WIN
    MSG msg;
    while( PeekMessage( &msg, winId(), WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE ) )
      ;
#endif
    QApplication::syncX();

    if ( QApplication::reverseLayout() && tlw->size() == QSize(w,h) ) {
	s.rwidth() = tlw->size().width();
	p.rx() = np.x();
    }
}

/*! \reimp */
bool QSizeGrip::eventFilter( QObject *o, QEvent *e )
{
    if ( o == tlw ) {
	switch ( e->type() ) {
#ifndef Q_WS_MAC
	/* The size grip goes no where on Mac OS X when you maximize!  --Sam */
	case QEvent::ShowMaximized:
#endif
	case QEvent::ShowFullScreen:
	    hide();
	    break;
	case QEvent::ShowNormal:
	    show();
	    break;
	default:
	    break;
	}
    } else if(o == this) {
#if defined(Q_WS_MAC)
	switch(e->type()) {
	case QEvent::Hide:
	case QEvent::Show:
	    if(!QApplication::closingDown() && parentWidget() && !qt_sizegrip_workspace(this)) {
		if(QWidget *w = qt_sizegrip_topLevelWidget(this)) {
		    if(w->isTopLevel()) 
			qt_mac_update_sizer(w, e->type() == QEvent::Hide ? -1 : 1);
		}
	    }
	    break;
	default:
	    break;
	}
 #endif	    
    }
    return FALSE;
}

#endif //QT_NO_SIZEGRIP
