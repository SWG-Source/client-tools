/****************************************************************************
** $Id: qdesktopwidget_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QDesktopWidget class.
**
** Created : 20030714
**
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2003 Ivan de Jesus Deras Tabora
** Copyright (C) 2004 Ralf Habacker
** Copyright (C) 2005,2006 Christian Ehrlicher
** Copyright (C) 2006 Trolltech AS. All rights reserved. (parts of the source is from qt4/gpl)
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
#ifdef Q_CC_MSVC
#include <multimon.h>
#endif

#include "qdesktopwidget.h"
#include "qlibrary.h"
#include "qobjectlist.h"
#include "qpoint.h"
#include "qptrlist.h"
#include "qrect.h"
#include "qwidget.h"


class QDesktopWidgetPrivate
{
public:
    QDesktopWidgetPrivate();
    ~QDesktopWidgetPrivate();

    void clear();
    void init();
    void addScreen( HMONITOR hMonitor );

    QValueList<QRect> rcMon;
    QValueList<QRect> rcWork;

    int defaultScreen;
    int screenCount;
private:
    // GetMonitorInfo
    BOOL ( WINAPI * qt_GMI ) ( HMONITOR, LPMONITORINFO );
};

QDesktopWidgetPrivate::QDesktopWidgetPrivate()
        : defaultScreen( 0 ), screenCount( 1 ), qt_GMI ( NULL )
{
    init();
}

QDesktopWidgetPrivate::~QDesktopWidgetPrivate()
{
    clear();
}

void QDesktopWidgetPrivate::addScreen( HMONITOR hMonitor )
{
    MONITORINFO mi;
    RECT *lpRC;
	QRect r;

    memset( &mi, 0, sizeof ( MONITORINFO ) );
    mi.cbSize = sizeof( MONITORINFO );
    
	if ( !qt_GMI( hMonitor, &mi ) ) {
		rcMon.append( r );
		rcWork.append( r );
		return;
	}
	lpRC = &mi.rcMonitor;
	r = QRect ( QPoint ( lpRC->left, lpRC->top ), QPoint( lpRC->right - 1, lpRC->bottom - 1 ) );
	rcMon.append( r );
	lpRC = &mi.rcWork;
	r = QRect ( QPoint ( lpRC->left, lpRC->top ), QPoint( lpRC->right - 1, lpRC->bottom - 1 ) );
	rcWork.append( r );

    if ( mi.dwFlags & 0x00000001 ) //MONITORINFOF_PRIMARY
        defaultScreen = screenCount;

    screenCount++;
}

BOOL CALLBACK MonitorEnumProc( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprc, LPARAM dwData )
{
    QDesktopWidgetPrivate * lpDW = ( QDesktopWidgetPrivate* ) dwData;

    lpDW->addScreen( hMonitor );
    return TRUE;
}

void QDesktopWidgetPrivate::clear()
{
    uint i;
	rcMon.clear();
	rcWork.clear();
}

void QDesktopWidgetPrivate::init()
{
    screenCount = 0;
    // EnumDisplayMonitors
    BOOL ( WINAPI * qt_EDM ) ( HDC, LPCRECT, MONITORENUMPROC, LPARAM );

    clear();

    /* Only available in Win98 and up */
    QLibrary lib( "user32.dll" );

    qt_EDM = reinterpret_cast<decltype(qt_EDM)>(lib.resolve( "EnumDisplayMonitors" ));
	if ( !qt_GMI ) {
		QT_WA (
	        qt_GMI = reinterpret_cast<decltype(qt_GMI)>(lib.resolve( "GetMonitorInfoA" ));
			,
	        qt_GMI = reinterpret_cast<decltype(qt_GMI)>(lib.resolve( "GetMonitorInfoW" ));
		)
	}

    if ( qt_EDM && qt_GMI ) {
        qt_EDM( NULL, NULL, MonitorEnumProc, ( LPARAM ) this );
    } else {
        // Win95
        RECT rc;
        GetWindowRect( GetDesktopWindow(), &rc );
        QRect r = QRect ( QPoint ( rc.left, rc.top ), QPoint( rc.right, rc.bottom ) );
        rcMon.append( r );
        rcWork.append( r );
    }
}

// the QDesktopWidget itself will be created on the default screen
// as qt_x11_create_desktop_on_screen defaults to -1
QDesktopWidget::QDesktopWidget()
        : QWidget( 0, "desktop", WType_Desktop )
{
    d = new QDesktopWidgetPrivate;
}

QDesktopWidget::~QDesktopWidget()
{
    delete d;
}

bool QDesktopWidget::isVirtualDesktop() const
{
    return true;
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
    return this;
}

const QRect& QDesktopWidget::availableGeometry( int screen ) const
{
    if ( ( screen < 0 ) || ( screen >= d->screenCount ) )
        screen = d->defaultScreen;
    return d->rcWork[ screen ];
}

const QRect& QDesktopWidget::screenGeometry( int screen ) const
{
    if ( ( screen < 0 ) || ( screen >= d->screenCount ) )
        screen = d->defaultScreen;
	return d->rcMon[ screen ];
}

int QDesktopWidget::screenNumber( QWidget *widget ) const
{
    int maxSize = -1;
    int maxScreen = -1;

    if ( !widget )
        return d->defaultScreen;

    QRect frame = widget->frameGeometry();
    if ( !widget->isTopLevel() )
        frame.moveTopLeft( widget->mapToGlobal( QPoint( 0, 0 ) ) );

    for ( int i = 0; i < d->screenCount; ++i ) {
        QRect sect = d->rcMon[ i ].intersect( frame );
        int size = sect.width() * sect.height();
        if ( size > maxSize && sect.width() > 0 && sect.height() > 0 ) {
            maxSize = size;
            maxScreen = i;
        }
    }
    return maxScreen;
}

int QDesktopWidget::screenNumber( const QPoint &point ) const
{
    for ( int i = 0; i < d->screenCount; ++i ) {
        if ( d->rcMon[ i ].contains( point ) )
            return i;
    }
    return -1;
}

void QDesktopWidget::resizeEvent( QResizeEvent *ev )
{
	int oldscreencount = d->screenCount;
    QValueList<QRect> oldrects = d->rcMon;
    QValueList<QRect> oldworkrects = d->rcWork;

	d->init();
    QWidget::resizeEvent( ev );

	for ( int i = 0; i < QMIN(oldscreencount, d->screenCount); ++i ) {
        QRect oldrect = oldrects[ i ];
        QRect newrect = d->rcMon[ i ];
        if (oldrect != newrect)
            emit resized( i );
    }

#ifndef Q_OS_TEMP
    for ( int j = 0; j < QMIN(oldscreencount, d->screenCount); ++j ) {
        QRect oldrect = oldworkrects[ j ];
        QRect newrect = d->rcWork[ j ];
        if ( oldrect != newrect )
            emit workAreaResized( j );
    }
#endif
}
