/****************************************************************************
** $Id: qt/qsplashscreen.h   3.3.4   edited Dec 23 2003 $
**
** Definition of QSplashScreen class
**
** Copyright (C) 2003 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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

#ifndef QSPLASHSCREEN_H
#define QSPLASHSCREEN_H

#ifndef QT_H
#include "qpixmap.h"
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_SPLASHSCREEN
class QSplashScreenPrivate;

class Q_EXPORT QSplashScreen : public QWidget
{
    Q_OBJECT
public:
    QSplashScreen( const QPixmap &pixmap = QPixmap(), WFlags f = 0 );
    virtual ~QSplashScreen();

    void setPixmap( const QPixmap &pixmap );
    QPixmap* pixmap() const;
    void finish( QWidget *w );
    void repaint();

public slots:
    void message( const QString &str, int flags = AlignLeft,
		  const QColor &color = black );
    void clear();

signals:
    void messageChanged( const QString &str );

protected:
    virtual void drawContents( QPainter *painter );
    void mousePressEvent( QMouseEvent * );

private:
    void drawContents();

    QSplashScreenPrivate *d;
};
#endif //QT_NO_SPLASHSCREEN
#endif
