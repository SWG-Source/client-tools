/****************************************************************************
** $Id: qdesktopwidget_x11.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QDesktopWidget class.
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
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
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

#include "qdesktopwidget.h"
#include "qapplication.h"
#include "qobjectlist.h"
#include "qt_x11_p.h"

// defined in qwidget_x11.cpp
extern int qt_x11_create_desktop_on_screen;

// defined in qapplication_x11.cpp
extern Atom qt_net_workarea;
extern bool qt_net_supports(Atom atom);

// function to update the workarea of the screen
static bool qt_desktopwidget_workarea_dirty = TRUE;
void qt_desktopwidget_update_workarea()
{
    qt_desktopwidget_workarea_dirty = TRUE;
}


class QSingleDesktopWidget : public QWidget
{
public:
    QSingleDesktopWidget();
    ~QSingleDesktopWidget();
};

QSingleDesktopWidget::QSingleDesktopWidget()
    : QWidget( 0, "desktop", WType_Desktop )
{
}

QSingleDesktopWidget::~QSingleDesktopWidget()
{
    while ( children() )
        removeChild( children()->getFirst() );
}


class QDesktopWidgetPrivate
{
public:
    QDesktopWidgetPrivate();
    ~QDesktopWidgetPrivate();

    void init();

    bool use_xinerama;
    int defaultScreen;
    int screenCount;

    QWidget **screens;
    QRect *rects;
    QRect *workareas;
};

QDesktopWidgetPrivate::QDesktopWidgetPrivate()
    : use_xinerama(FALSE), defaultScreen(0), screenCount(1),
      screens( 0 ), rects( 0 ), workareas( 0 )
{
}

QDesktopWidgetPrivate::~QDesktopWidgetPrivate()
{
    if ( screens ) {
	for ( int i = 0; i < screenCount; ++i ) {
	    if (i == defaultScreen) continue;
	    delete screens[i];
	    screens[i] = 0;
	}

	delete [] screens;
    }

    if ( rects )     delete [] rects;
    if ( workareas ) delete [] workareas;
}

void QDesktopWidgetPrivate::init()
{
    // get the screen count
#ifndef QT_NO_XINERAMA
    XineramaScreenInfo *xinerama_screeninfo = 0;
    int unused;
    use_xinerama = (XineramaQueryExtension(QPaintDevice::x11AppDisplay(),
					   &unused, &unused) &&
		    XineramaIsActive(QPaintDevice::x11AppDisplay()));

    if (use_xinerama) {
	xinerama_screeninfo =
	    XineramaQueryScreens(QPaintDevice::x11AppDisplay(), &screenCount);
	defaultScreen = 0;
    } else
#endif // QT_NO_XINERAMA
    {
	defaultScreen = DefaultScreen(QPaintDevice::x11AppDisplay());
	screenCount = ScreenCount(QPaintDevice::x11AppDisplay());
    }

    delete [] rects;
    rects     = new QRect[ screenCount ];
    delete [] workareas;
    workareas = new QRect[ screenCount ];

    // get the geometry of each screen
    int i, x, y, w, h;
    for ( i = 0; i < screenCount; i++ ) {

#ifndef QT_NO_XINERAMA
	if (use_xinerama) {
	    x = xinerama_screeninfo[i].x_org;
	    y = xinerama_screeninfo[i].y_org;
	    w = xinerama_screeninfo[i].width;
	    h = xinerama_screeninfo[i].height;
	} else
#endif // QT_NO_XINERAMA
	    {
		x = 0;
		y = 0;
		w = WidthOfScreen(ScreenOfDisplay(QPaintDevice::x11AppDisplay(), i));
		h = HeightOfScreen(ScreenOfDisplay(QPaintDevice::x11AppDisplay(), i));
	    }

	rects[i].setRect(x, y, w, h);
	workareas[i] = QRect();
    }

#ifndef QT_NO_XINERAMA
    if (xinerama_screeninfo)
	XFree(xinerama_screeninfo);
#endif // QT_NO_XINERAMA

}

// the QDesktopWidget itself will be created on the default screen
// as qt_x11_create_desktop_on_screen defaults to -1
QDesktopWidget::QDesktopWidget()
    : QWidget( 0, "desktop", WType_Desktop )
{
    d = new QDesktopWidgetPrivate();

    /*
      we don't call d->init() here, since the initial resize event
      will end up calling init() a second time, which is inefficient.
      instead, for the sending of all posted event to the desktop
      widget (including the initial resize event, which calls
      d->init()).
    */
    QApplication::sendPostedEvents( this, 0 );
}

QDesktopWidget::~QDesktopWidget()
{
    delete d;
}

bool QDesktopWidget::isVirtualDesktop() const
{
    return d->use_xinerama;
}

int QDesktopWidget::primaryScreen() const
{
    return d->defaultScreen;
}

int QDesktopWidget::numScreens() const
{
    return d->screenCount;
}

QWidget *QDesktopWidget::screen( int screen )
{
    if (d->use_xinerama)
	return this;

    if ( screen < 0 || screen >= d->screenCount )
	screen = d->defaultScreen;

    if ( ! d->screens ) {
	d->screens = new QWidget*[ d->screenCount ];
	memset( d->screens, 0, d->screenCount * sizeof( QWidget * ) );
	d->screens[ d->defaultScreen ] = this;
    }

    if ( ! d->screens[screen] ||               // not created yet
	 ! d->screens[screen]->isDesktop() ) { // reparented away
	qt_x11_create_desktop_on_screen = screen;
	d->screens[screen] = new QSingleDesktopWidget;
	qt_x11_create_desktop_on_screen = -1;
    }

    return d->screens[screen];
}

const QRect& QDesktopWidget::availableGeometry( int screen ) const
{
    if ( qt_desktopwidget_workarea_dirty ) {
	// the workareas are dirty, invalidate them
	for ( int i = 0; i < d->screenCount; ++i )
	    d->workareas[i] = QRect();
	qt_desktopwidget_workarea_dirty = FALSE;
    }

    if ( screen < 0 || screen >= d->screenCount )
	screen = d->defaultScreen;

    if ( d->workareas[screen].isValid() )
	return d->workareas[screen];

    if ( ! isVirtualDesktop() && qt_net_supports( qt_net_workarea ) ) {
	Atom ret;
	int format, e;
	unsigned char *data = 0;
	unsigned long nitems, after;

	e = XGetWindowProperty( QPaintDevice::x11AppDisplay(),
				QPaintDevice::x11AppRootWindow( screen ),
				qt_net_workarea, 0, 4, False, XA_CARDINAL,
				&ret, &format, &nitems, &after, &data );

	if (e == Success && ret == XA_CARDINAL &&
	    format == 32 && nitems == 4) {
	    long *workarea = (long *) data;
	    d->workareas[screen].setRect( workarea[0], workarea[1],
					  workarea[2], workarea[3] );
	} else {
	    d->workareas[screen] = screenGeometry(screen);
	}
	if ( data )
	    XFree( data );
    } else {
	d->workareas[screen] = screenGeometry(screen);
    }

    return d->workareas[screen];
}

const QRect& QDesktopWidget::screenGeometry( int screen ) const
{
    if ( screen < 0 || screen >= d->screenCount )
	screen = d->defaultScreen;

    return d->rects[ screen ];
}

int QDesktopWidget::screenNumber( QWidget *widget ) const
{
    if ( !widget )
	return d->defaultScreen;

#ifndef QT_NO_XINERAMA
    if (d->use_xinerama) {
	// this is how we do it for xinerama
	QRect frame = widget->frameGeometry();
	if ( !widget->isTopLevel() )
	    frame.moveTopLeft( widget->mapToGlobal( QPoint( 0, 0 ) ) );

	int maxSize = -1;
	int maxScreen = -1;

	for ( int i = 0; i < d->screenCount; ++i ) {
	    QRect sect = d->rects[i].intersect( frame );
	    int size = sect.width() * sect.height();
	    if ( size > maxSize && sect.width() > 0 && sect.height() > 0 ) {
		maxSize = size;
		maxScreen = i;
	    }
	}
	return maxScreen;
    }
#endif // QT_NO_XINERAMA

    return widget->x11Screen();
}

int QDesktopWidget::screenNumber( const QPoint &point ) const
{
    for ( int i = 0; i < d->screenCount; ++i ) {
	if ( d->rects[i].contains( point ) )
	    return i;
    }
    return -1;
}

void QDesktopWidget::resizeEvent( QResizeEvent *event )
{
    d->init();
    qt_desktopwidget_workarea_dirty = TRUE;
    QWidget::resizeEvent( event );
}
