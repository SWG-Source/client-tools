/****************************************************************************
** $Id: qt/qdesktopwidget.h   3.3.4   edited Dec 2 2003 $
**
** Definition of QDesktopWidget class.
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
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

#ifndef QDESKTOPWIDGET_H
#define QDESKTOPWIDGET_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

class QApplication;
class QDesktopWidgetPrivate; /* Don't touch! */

class Q_EXPORT QDesktopWidget : public QWidget
{
    Q_OBJECT
public:
    QDesktopWidget();
    ~QDesktopWidget();

    bool isVirtualDesktop() const;

    int numScreens() const;
    int primaryScreen() const;

    int screenNumber( QWidget *widget = 0 ) const; // ### 4.0: const QWidget*
    int screenNumber( const QPoint & ) const;

    QWidget *screen( int screen = -1 );

    const QRect& screenGeometry( int screen = -1 ) const;
    const QRect& screenGeometry( QWidget *widget ) const
    { return screenGeometry( screenNumber( widget ) ); }
    const QRect& screenGeometry( const QPoint &point ) const
    { return screenGeometry( screenNumber( point ) ); }

    const QRect& availableGeometry( int screen = -1 ) const;
    const QRect& availableGeometry( QWidget *widget ) const
    { return availableGeometry( screenNumber( widget ) ); }
    const QRect& availableGeometry( const QPoint &point ) const
    { return availableGeometry( screenNumber( point ) ); }

    void insertChild( QObject * );

signals:
    void resized( int );
    void workAreaResized( int );

protected:
    void resizeEvent( QResizeEvent *e );

private:
    QDesktopWidgetPrivate *d;

#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QDesktopWidget( const QDesktopWidget & );
    QDesktopWidget &operator=( const QDesktopWidget & );
#endif

    friend class QApplication;
#ifdef Q_WS_QWS
    friend class QWSDisplay;
#endif
};

#endif //QDESKTOPWIDGET_H
