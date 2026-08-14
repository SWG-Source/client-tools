/****************************************************************************
** $Id: qregion_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QRegion class for X11
**
** Created : 20030119
**
** Copyright (C) 2003 Holger Schroeder
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2003 Ivan de Jesus Deras Tabora
** Copyright (C) 2004 Tom and Timi Cecka
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

#include "qregion.h"

#include "qbitmap.h"
#include "qpointarray.h"
#include "qt_windows.h"

//#define DEBUG_QREGION

// qpixmap_win.cpp
bool qt_GetBitmapBits( const QPixmap *pm, uchar **bits, long &sbpr );

static QRegion *empty_region = 0;

static void cleanup_empty_region()
{
    delete empty_region;
    empty_region = 0;
}


/*!
  Constructs a null region.
  \sa isNull()
*/

QRegion::QRegion()
{
#ifdef DEBUG_QREGION
    qDebug( "QRegion::QRegion()" );
#endif

    if ( !empty_region ) {
        qAddPostRoutine( cleanup_empty_region );
        empty_region = new QRegion( TRUE );
    }
    data = empty_region->data;
    data->ref();
}

/*!
  Internal constructor that creates a null region.
*/

QRegion::QRegion( bool is_null )
{
#ifdef DEBUG_QREGION
    qDebug( "QRegion::QRegion( is_null: %d )", is_null );
#endif

    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = CreateRectRgn( 0, 0, 0, 0 );
    CHECK_PTR( data->rgn );
    data->is_null = is_null;
}

/*!
\overload

If the rectangle is invalid, the rectangle will be normalized() before the region is created.
This semantics will change in Qt-3
and an invalid rectangle will construct an empty region.

\sa QRect::isValid() QRect::normalize()
 */

QRegion::QRegion( const QRect &r, RegionType t )
{
#ifdef DEBUG_QREGION
    qDebug( "QRegion::QRegion( (%d, %d, %d, %d), %s )", r.x(), r.y(), r.width(), r.height(), t == Rectangle ? "Rectangle" : "Ellipse" );
#endif

    QRect rr = r.normalize();
    data = new QRegionData;
    CHECK_PTR( data );
    data->is_null = FALSE;
    if ( t == Rectangle ) {   // rectangular region
        //+1 because CreateRectRgn exclude right and bottom !?
        data->rgn = CreateRectRgn( rr.x(), rr.y(), rr.x() + rr.width(), rr.y() + rr.height() );
    } else if ( t == Ellipse ) {  // elliptic region
        data->rgn = CreateEllipticRgn( rr.rLeft(), rr.rTop(), rr.rRight(), rr.rBottom() );
    }
    CHECK_PTR( data->rgn );
}


/*!
  Constructs a polygon region from the point array \a a.

  If \a winding is TRUE, the polygon
  region is filled using the winding algorithm, otherwise the default
  even-odd fill algorithm is used.

  This constructor may create complex regions that will slow
  down painting when used.
*/

QRegion::QRegion( const QPointArray &a, bool winding )
{
#ifdef DEBUG_QREGION
    qDebug( "QRegion::QRegion( a: 0x%08p, winding: %d )", a, winding );
#endif

    data = new QRegionData;
    CHECK_PTR( data );
    data->is_null = FALSE;

    data->rgn = CreatePolygonRgn( ( POINT* ) a.data(), a.size(), winding ? WINDING : ALTERNATE );
    CHECK_PTR( data->rgn );
}


/*!
  Constructs a new region which is equal to \a r.
*/

QRegion::QRegion( const QRegion &r )
{
#ifdef DEBUG_QREGION
    qDebug( "QRegion::QRegion( r: 0x%08X )", r );
#endif

    data = r.data;
    data->ref();
}

HRGN QBitmapToHRGN( const QBitmap& bm )
{
    bool freeBits;
    uchar *ptBits, *sptr;
    long bpr;
    HRGN hRgn = 0;
    RGNDATA *pData;
    RECT *pRects, boundRect = {MAXLONG, MAXLONG, 0, 0};

    freeBits = qt_GetBitmapBits( &bm, &sptr, bpr );
    ptBits = sptr;  // btBits is beeing modified!

    pData = ( RGNDATA* ) new uchar[ sizeof( RGNDATAHEADER ) + ( sizeof( RECT ) * 1000 ) ];
    Q_CHECK_PTR( pData );
    pRects = ( RECT* ) & pData->Buffer;
    pData->rdh.dwSize = sizeof( RGNDATAHEADER );
    pData->rdh.iType = RDH_RECTANGLES;
    pData->rdh.nCount = pData->rdh.nRgnSize = 0;
    pData->rdh.rcBound = boundRect;

    //Top down Bitmap
    for ( int y = 0; y < bm.height(); y++ ) {

        uchar *pt = ptBits;
        for ( int x = 0; x < bm.width(); x++ ) {
            int x0 = x;
            while ( x < bm.width() ) {
                if ( ( *( pt + ( x >> 3 ) ) & ( 1 << ( 7 - ( x & 7 ) ) ) ) )                //Transparent pixel
                    break;
                x++;
            }
            if ( x > x0 ) {
                //(x0, y)-(x, y+1) make a new rectangle in the region
                SetRect( &pRects[ pData->rdh.nCount ], x0, y, x, y + 1 );
                if ( x0 < pData->rdh.rcBound.left )
                    pData->rdh.rcBound.left = x0;
                if ( y < pData->rdh.rcBound.top )
                    pData->rdh.rcBound.top = y;
                if ( x > pData->rdh.rcBound.right )
                    pData->rdh.rcBound.right = x;
                if ( y + 1 > pData->rdh.rcBound.bottom )
                    pData->rdh.rcBound.bottom = y + 1;
                pData->rdh.nCount++;

                if ( pData->rdh.nCount == 1000 ) {
                    HRGN h = ExtCreateRegion( NULL, sizeof( RGNDATAHEADER ) + ( sizeof( RECT ) * 1000 ), pData );
#ifdef DEBUG_QWIDGET

                    if ( h == NULL )
                        qDebug( "QBitmapToHRGN failed GetLastError()=%d", GetLastError() );
#endif

                    if ( hRgn ) {
                        CombineRgn( hRgn, hRgn, h, RGN_OR );
                        DeleteObject( h );
                    } else
                        hRgn = h;
                    pData->rdh.nCount = 0;
                    pData->rdh.rcBound = boundRect;
                }
            }
        }
        //Next scan line
        ptBits += bpr;
    }

#ifdef DEBUG_QWIDGET
    qDebug( "QBitmapToHRGN ( pData->rdh.nCount=%d, pData->rdh.rcBound=(%d, %d, %d, %d) )",
            pData->rdh.nCount, pData->rdh.rcBound.left, pData->rdh.rcBound.top,
            pData->rdh.rcBound.right, pData->rdh.rcBound.bottom );
#endif

    HRGN h = ExtCreateRegion( NULL, sizeof( RGNDATAHEADER ) + ( sizeof( RECT ) * 1000 ), pData );
#ifdef DEBUG_QWIDGET

    if ( h == NULL )
        qDebug( "QBitmapToHRGN failed GetLastError()=%d **", GetLastError() );
#endif

    if ( hRgn ) {
        CombineRgn( hRgn, hRgn, h, RGN_OR );
        DeleteObject( h );
    } else
        hRgn = h;

    delete [] pData;
    if ( freeBits )
        delete [] sptr;
    
    return hRgn;
}
/*!
  Constructs a region from the bitmap \a bm.

  The resulting region consists of the pixels in \a bm that are \c
  color1, as if each pixel was a 1 by 1 rectangle.

  This constructor may create complex regions that will slow
  down painting when used. Note that drawing masked pixmaps
  can be done much faster using QPixmap::setMask().

*/
QRegion::QRegion( const QBitmap &bm )
{
#ifdef DEBUG_QREGION
    qDebug( "QRegion::QRegion( bm: 0x%08p )", &bm );
#endif

    data = new QRegionData;
    CHECK_PTR( data );
    data->is_null = FALSE;
    data->rgn = QBitmapToHRGN( bm );
}

/*!
  Destructs the region.
*/

QRegion::~QRegion()
{
#ifdef DEBUG_QREGION
    qDebug( "QRegion::QRegion" );
#endif

    if ( data->deref() ) {
        DeleteObject( data->rgn );
        delete data;
    }
}


/*!
  Assigns \a r to this region and returns a reference to the
  region.

*/

QRegion &QRegion::operator=( const QRegion &r )
{
#ifdef DEBUG_QREGION
    qDebug( "QRegion::operator=( r: 0x%08p )", &r );
#endif

    r.data->ref();    // beware of r = r
    if ( data->deref() ) {
        DeleteObject( data->rgn );
        delete data;
    }
    data = r.data;
    return *this;
}


/*!
  Returns a \link shclass.html deep copy\endlink of the region.

  \sa detach()
*/

QRegion QRegion::copy() const
{
    QRegion r( data->is_null );
    if ( CombineRgn( r.data->rgn, data->rgn, 0, RGN_COPY ) == ERROR ) {
#ifdef DEBUG_QREGION
        qDebug( "QRegion::copy() failed, GetLastError()=%s", GetLastError() );
#endif

    }
    return r;
}

bool QRegion::isNull() const
{
    return data->is_null;
}

bool QRegion::isEmpty() const
{
    if (!empty_region) {
        QRegion rgn;
    }
  
    return data == empty_region->data || data->rgn == 0;
}


bool QRegion::contains( const QPoint &p ) const
{
    return data->rgn ? PtInRegion(data->rgn, p.x(), p.y()) : false;
}

bool QRegion::contains( const QRect &r ) const
{
    if (!data->rgn)
        return false;
    RECT rect;
    SetRect(&rect, r.left(), r.top(), r.right(), r.bottom());
    return RectInRegion(data->rgn, &rect);
}

void QRegion::translate( int dx, int dy )
{
    if (!data->rgn)
        return;
    detach();
    OffsetRgn( data->rgn, dx, dy );
}

QRegion QRegion::unite( const QRegion &r ) const
{
    QRegion result( FALSE );
    CombineRgn( result.data->rgn, data->rgn, r.data->rgn, RGN_OR );
    return result;
}

QRegion QRegion::intersect( const QRegion &r ) const
{
    QRegion result( FALSE );
    CombineRgn( result.data->rgn, data->rgn, r.data->rgn, RGN_AND );
    return result;
}

QRegion QRegion::subtract( const QRegion &r ) const
{
    QRegion result( FALSE );
    CombineRgn( result.data->rgn, data->rgn, r.data->rgn, RGN_DIFF );
    return result;
}

QRegion QRegion::eor( const QRegion &r ) const
{
    QRegion result( FALSE );
    CombineRgn( result.data->rgn, data->rgn, r.data->rgn, RGN_XOR );
    return result;
}

QRect QRegion::boundingRect() const
{
    RECT r;
    int result = data->rgn ? GetRgnBox(data->rgn, &r) : 0;
    if (result == 0 || result == NULLREGION)
        return QRect(0, 0, 0, 0);
    else
        return QRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
}

QMemArray<QRect> QRegion::rects() const
{
    QMemArray<QRect> a;
    if (data->rgn == 0)
        return a;

    int numBytes = data->rgn ? GetRegionData(data->rgn, 0, 0) : 0;
    if (numBytes == 0)
        return a;

    char *buf = new char[numBytes];
    if (buf == 0)
        return a;

    RGNDATA *rd = reinterpret_cast<RGNDATA*>(buf);
    if (GetRegionData(data->rgn, numBytes, rd) == 0) {
        delete [] buf;
        return a;
    }

    a = QMemArray<QRect>(rd->rdh.nCount);
    RECT *r = reinterpret_cast<RECT*>(rd->Buffer);
    for (unsigned int i = 0; i < a.size(); ++i) {
        a[(int)i].setCoords(r->left, r->top, r->right - 1, r->bottom - 1);
        ++r;
    }

    delete [] buf;
    return a;
}


void QRegion::setRects(const QRect *rects, int num)
{
    *this = QRegion();
    for (int i = 0; i < num; ++i)
        *this |= rects[i];
}

bool QRegion::operator==(const QRegion &r) const
{
    if (data == r.data)
        return true;
    if ((data->rgn == 0) ^ (r.data->rgn == 0)) // one is empty, not both
        return false;
    return data->rgn == 0 ? true // both empty
                       : EqualRgn(data->rgn, r.data->rgn); // both non-empty
}
