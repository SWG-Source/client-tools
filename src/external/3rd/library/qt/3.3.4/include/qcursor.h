/****************************************************************************
** $Id: qt/qcursor.h   3.3.4   edited Dec 18 2003 $
**
** Definition of QCursor class
**
** Created : 940219
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

#ifndef QCURSOR_H
#define QCURSOR_H

#ifndef QT_H
#include "qpoint.h"
#include "qshared.h"
#endif // QT_H

/*
  ### The fake cursor has to go first with old qdoc.
*/
#ifdef QT_NO_CURSOR

class Q_EXPORT QCursor : public Qt
{
public:
    static QPoint pos();
    static void	  setPos( int x, int y );
    static void	  setPos( const QPoint & );
private:
    QCursor();
};

#endif // QT_NO_CURSOR

#ifndef QT_NO_CURSOR

struct QCursorData;


class Q_EXPORT QCursor : public Qt
{
public:
    QCursor();				// create default arrow cursor
    QCursor( int shape );
    QCursor( const QBitmap &bitmap, const QBitmap &mask,
	     int hotX=-1, int hotY=-1 );
    QCursor( const QPixmap &pixmap,
	     int hotX=-1, int hotY=-1 );
    QCursor( const QCursor & );
   ~QCursor();
    QCursor &operator=( const QCursor & );

    int		  shape()   const;
    void	  setShape( int );

    const QBitmap *bitmap() const;
    const QBitmap *mask()   const;
    QPoint	  hotSpot() const;

#if defined(Q_WS_WIN)
    HCURSOR	  handle()  const;
    QCursor( HCURSOR );
#elif defined(Q_WS_X11)
    HANDLE	  handle()  const;
    QCursor( HANDLE );
#elif defined(Q_WS_MAC)
    HANDLE handle() const;
#elif defined(Q_WS_QWS)
    HANDLE	  handle()  const;
#endif

    static QPoint pos();
    static void	  setPos( int x, int y );
    static void	  setPos( const QPoint & );

    static void	  initialize();
    static void	  cleanup();

#if defined(Q_WS_X11)
    static int 	  x11Screen();
#endif
private:
    void	  setBitmap( const QBitmap &bitmap, const QBitmap &mask,
				 int hotX, int hotY );
    void	  update() const;
    QCursorData	 *data;
    QCursor	 *find_cur(int);
#if defined(Q_WS_MAC)
    friend void qt_mac_set_cursor(const QCursor *c, const Point *p);
#endif
};


#if !defined(QT_CLEAN_NAMESPACE)
// CursorShape is defined in X11/X.h
#ifdef CursorShape
#define X_CursorShape CursorShape
#undef CursorShape
#endif
typedef Qt::CursorShape QCursorShape;
#ifdef X_CursorShape
#define CursorShape X_CursorShape
#endif
#endif


/*****************************************************************************
  QCursor stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QCursor & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QCursor & );
#endif
#endif // QT_NO_CURSOR


inline void QCursor::setPos( const QPoint &p )
{
    setPos( p.x(), p.y() );
}

#endif // QCURSOR_H
