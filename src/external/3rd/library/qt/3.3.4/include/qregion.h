/****************************************************************************
** $Id: qt/qregion.h   3.3.4   edited Oct 28 2003 $
**
** Definition of QRegion class
**
** Created : 940514
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

#ifndef QREGION_H
#define QREGION_H

#ifndef QT_H
#include "qshared.h"
#include "qrect.h"
#endif // QT_H

#ifdef Q_WS_X11
struct QRegionPrivate;
#endif

class Q_EXPORT QRegion
{
public:
    enum RegionType { Rectangle, Ellipse };

    QRegion();
    QRegion( int x, int y, int w, int h, RegionType = Rectangle );
    QRegion( const QRect &, RegionType = Rectangle );
    QRegion( const QPointArray &, bool winding=FALSE );
    QRegion( const QRegion & );
    QRegion( const QBitmap & );
   ~QRegion();
    QRegion &operator=( const QRegion & );

    bool    isNull()   const;
    bool    isEmpty()  const;

    bool    contains( const QPoint &p ) const;
    bool    contains( const QRect &r )	const;

    void    translate( int dx, int dy );

    QRegion unite( const QRegion & )	const;
    QRegion intersect( const QRegion &) const;
    QRegion subtract( const QRegion & ) const;
    QRegion eor( const QRegion & )	const;

    QRect   boundingRect() const;
    QMemArray<QRect> rects() const;
    void setRects( const QRect *, int );

    const QRegion operator|( const QRegion & ) const;
    const QRegion operator+( const QRegion & ) const;
    const QRegion operator&( const QRegion & ) const;
    const QRegion operator-( const QRegion & ) const;
    const QRegion operator^( const QRegion & ) const;
    QRegion& operator|=( const QRegion & );
    QRegion& operator+=( const QRegion & );
    QRegion& operator&=( const QRegion & );
    QRegion& operator-=( const QRegion & );
    QRegion& operator^=( const QRegion & );

    bool    operator==( const QRegion & )  const;
    bool    operator!=( const QRegion &r ) const
			{ return !(operator==(r)); }

#if defined(Q_WS_WIN)
    HRGN    handle() const { return data->rgn; }
#elif defined(Q_WS_X11)
    Region handle() const { if(!data->rgn) updateX11Region(); return data->rgn; }
#elif defined(Q_WS_MAC)
    RgnHandle handle(bool require_rgn=FALSE) const;
#elif defined(Q_WS_QWS)
    // QGfx_QWS needs this for region drawing
    void * handle() const { return data->rgn; }
#endif

#ifndef QT_NO_DATASTREAM
    friend Q_EXPORT QDataStream &operator<<( QDataStream &, const QRegion & );
    friend Q_EXPORT QDataStream &operator>>( QDataStream &, QRegion & );
#endif
private:
    QRegion( bool );
    QRegion copy() const;
    void    detach();
#if defined(Q_WS_WIN)
    QRegion winCombine( const QRegion &, int ) const;
#endif
#if defined(Q_WS_X11)
    void updateX11Region() const;
    void *clipRectangles( int &num ) const;
    friend void *qt_getClipRects( const QRegion &, int & );
#endif
    void    exec( const QByteArray &, int ver = 0 );
    struct QRegionData : public QShared {
#if defined(Q_WS_WIN)
	HRGN   rgn;
#elif defined(Q_WS_X11)
	Region rgn;
	void *xrectangles;
	QRegionPrivate *region;
#elif defined(Q_WS_MAC)
	uint is_rect:1;
	QRect rect;
	RgnHandle rgn;
#elif defined(Q_WS_QWS)
	void * rgn;
#endif
	bool   is_null;
    } *data;
#if defined(Q_WS_MAC)
    friend struct qt_mac_rgn_data_cache;
    friend QRegionData *qt_mac_get_rgn_data();
    friend void qt_mac_free_rgn_data(QRegionData *);
    void rectifyRegion();
#elif defined(Q_WS_WIN)
    friend class QETWidget;
#endif

};


#define QRGN_SETRECT		1		// region stream commands
#define QRGN_SETELLIPSE		2		//  (these are internal)
#define QRGN_SETPTARRAY_ALT	3
#define QRGN_SETPTARRAY_WIND	4
#define QRGN_TRANSLATE		5
#define QRGN_OR			6
#define QRGN_AND		7
#define QRGN_SUB		8
#define QRGN_XOR		9
#define QRGN_RECTS	       10


/*****************************************************************************
  QRegion stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QRegion & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QRegion & );
#endif


#endif // QREGION_H
