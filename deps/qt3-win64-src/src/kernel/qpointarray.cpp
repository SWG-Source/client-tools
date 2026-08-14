/****************************************************************************
** $Id: qpointarray.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QPointArray class
**
** Created : 940213
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

#include "qpointarray.h"
#include "qrect.h"
#include "qdatastream.h"
#include "qwmatrix.h"
#include <stdarg.h>

const double Q_PI = 3.14159265358979323846;   // pi // one more useful comment


/*!
    \class QPointArray qpointarray.h
    \brief The QPointArray class provides an array of points.

    \ingroup images
    \ingroup graphics
    \ingroup shared

    A QPointArray is an array of QPoint objects. In addition to the
    functions provided by QMemArray, QPointArray provides some
    point-specific functions.

    For convenient reading and writing of the point data use
    setPoints(), putPoints(), point(), and setPoint().

    For geometry operations use boundingRect() and translate(). There
    is also the QWMatrix::map() function for more general
    transformations of QPointArrays. You can also create arcs and
    ellipses with makeArc() and makeEllipse().

    Among others, QPointArray is used by QPainter::drawLineSegments(),
    QPainter::drawPolyline(), QPainter::drawPolygon() and
    QPainter::drawCubicBezier().

    Note that because this class is a QMemArray, copying an array and
    modifying the copy modifies the original as well, i.e. a shallow
    copy. If you need a deep copy use copy() or detach(), for example:

    \code
	void drawGiraffe( const QPointArray & r, QPainter * p )
	{
	    QPointArray tmp = r;
	    tmp.detach();
	    // some code that modifies tmp
	    p->drawPoints( tmp );
	}
    \endcode

    If you forget the tmp.detach(), the const array will be modified.

    \sa QPainter QWMatrix QMemArray
*/


/*****************************************************************************
  QPointArray member functions
 *****************************************************************************/

/*!
    \fn QPointArray::QPointArray()

    Constructs a null point array.

    \sa isNull()
*/

/*!
    \fn QPointArray::QPointArray( int size )

    Constructs a point array with room for \a size points. Makes a
    null array if \a size == 0.

    \sa resize(), isNull()
*/

/*!
    \fn QPointArray::QPointArray( const QPointArray &a )

    Constructs a shallow copy of the point array \a a.

    \sa copy() detach()
*/

/*!
    Constructs a point array from the rectangle \a r.

    If \a closed is FALSE, then the point array just contains the
    following four points in the listed order: r.topLeft(),
    r.topRight(), r.bottomRight() and r.bottomLeft().

    If \a closed is TRUE, then a fifth point is set to r.topLeft().
*/

QPointArray::QPointArray( const QRect &r, bool closed )
{
    setPoints( 4, r.left(),  r.top(),
		  r.right(), r.top(),
		  r.right(), r.bottom(),
		  r.left(),  r.bottom() );
    if ( closed ) {
	resize( 5 );
	setPoint( 4, r.left(), r.top() );
    }
}

/*!
  \internal
  Constructs a point array with \a nPoints points, taken from the
  \a points array.

  Equivalent to setPoints(nPoints, points).
*/

QPointArray::QPointArray( int nPoints, const QCOORD *points )
{
    setPoints( nPoints, points );
}


/*!
    \fn QPointArray::~QPointArray()

    Destroys the point array.
*/


/*!
    \fn QPointArray &QPointArray::operator=( const QPointArray &a )

    Assigns a shallow copy of \a a to this point array and returns a
    reference to this point array.

    Equivalent to assign(a).

    \sa copy() detach()
*/

/*!
    \fn QPointArray QPointArray::copy() const

    Creates a deep copy of the array.

    \sa detach()
*/



/*!
    Translates all points in the array by \a (dx, dy).
*/

void QPointArray::translate( int dx, int dy )
{
    register QPoint *p = data();
    register int i = size();
    QPoint pt( dx, dy );
    while ( i-- ) {
	*p += pt;
	p++;
    }
}


/*!
    Reads the coordinates of the point at position \a index within the
    array and writes them into \a *x and \a *y.
*/

void QPointArray::point( uint index, int *x, int *y ) const
{
    QPoint p = QMemArray<QPoint>::at( index );
    if ( x )
	*x = (int)p.x();
    if ( y )
	*y = (int)p.y();
}

/*!
    \overload

    Returns the point at position \a index within the array.
*/

QPoint QPointArray::point( uint index ) const
{ // #### index out of bounds
    return QMemArray<QPoint>::at( index );
}

/*!
    \fn void QPointArray::setPoint( uint i, const QPoint &p )

    \overload

    Sets the point at array index \a i to \a p.
*/

/*!
    Sets the point at position \a index in the array to \a (x, y).
*/

void QPointArray::setPoint( uint index, int x, int y )
{ // #### index out of bounds
    QMemArray<QPoint>::at( index ) = QPoint( x, y );
}

/*!
  \internal
  Resizes the array to \a nPoints and sets the points in the array to
  the values taken from \a points.

  Returns TRUE if successful, or FALSE if the array could not be
  resized (normally due to lack of memory).

  The example code creates an array with two points (1,2) and (3,4):
  \code
    static QCOORD points[] = { 1,2, 3,4 };
    QPointArray a;
    a.setPoints( 2, points );
  \endcode

  \sa resize(), putPoints()
*/

bool QPointArray::setPoints( int nPoints, const QCOORD *points )
{
    if ( !resize(nPoints) )
	return FALSE;
    int i = 0;
    while ( nPoints-- ) {			// make array of points
	setPoint( i++, *points, *(points+1) );
	points++;
	points++;
    }
    return TRUE;
}

/*!
    \overload

    Resizes the array to \a nPoints and sets the points in the array
    to the values taken from the variable argument list.

    Returns TRUE if successful, or FALSE if the array could not be
    resized (typically due to lack of memory).

    The example code creates an array with two points (1,2) and (3,4):

    \code
	QPointArray a;
	a.setPoints( 2, 1,2, 3,4 );
    \endcode

    The points are given as a sequence of integers, starting with \a
    firstx then \a firsty, and so on.

    \sa resize(), putPoints()
*/

bool QPointArray::setPoints( int nPoints, int firstx, int firsty, ... )
{
    va_list ap;
    if ( !resize(nPoints) )
	return FALSE;
    setPoint( 0, firstx, firsty );		// set first point
    int i = 1, x, y;
    nPoints--;
    va_start( ap, firsty );
    while ( nPoints-- ) {
	x = va_arg( ap, int );
	y = va_arg( ap, int );
	setPoint( i++, x, y );
    }
    va_end( ap );
    return TRUE;
}

/*! \overload
  \internal
  Copies \a nPoints points from the \a points coord array into
  this point array, and resizes the point array if
  \c{index+nPoints} exceeds the size of the array.

  Returns TRUE if successful, or FALSE if the array could not be
  resized (typically due to lack of memory).

*/

bool QPointArray::putPoints( int index, int nPoints, const QCOORD *points )
{
    if ( index + nPoints > (int)size() ) {	// extend array
	if ( !resize( index + nPoints ) )
	    return FALSE;
    }
    int i = index;
    while ( nPoints-- ) {			// make array of points
	setPoint( i++, *points, *(points+1) );
	points++;
	points++;
    }
    return TRUE;
}

/*!
    Copies \a nPoints points from the variable argument list into this
    point array from position \a index, and resizes the point array if
    \c{index+nPoints} exceeds the size of the array.

    Returns TRUE if successful, or FALSE if the array could not be
    resized (typically due to lack of memory).

    The example code creates an array with three points (4,5), (6,7)
    and (8,9), by expanding the array from 1 to 3 points:

    \code
	QPointArray a( 1 );
	a[0] = QPoint( 4, 5 );
	a.putPoints( 1, 2, 6,7, 8,9 ); // index == 1, points == 2
    \endcode

    This has the same result, but here putPoints overwrites rather
    than extends:
    \code
	QPointArray a( 3 );
	a.putPoints( 0, 3, 4,5, 0,0, 8,9 );
	a.putPoints( 1, 1, 6,7 );
    \endcode

    The points are given as a sequence of integers, starting with \a
    firstx then \a firsty, and so on.

    \sa resize()
*/

bool QPointArray::putPoints( int index, int nPoints, int firstx, int firsty,
			     ... )
{
    va_list ap;
    if ( index + nPoints > (int)size() ) {	// extend array
	if ( !resize(index + nPoints) )
	    return FALSE;
    }
    if ( nPoints <= 0 )
	return TRUE;
    setPoint( index, firstx, firsty );		// set first point
    int i = index + 1, x, y;
    nPoints--;
    va_start( ap, firsty );
    while ( nPoints-- ) {
	x = va_arg( ap, int );
	y = va_arg( ap, int );
	setPoint( i++, x, y );
    }
    va_end( ap );
    return TRUE;
}


/*!
    \overload

    This version of the function copies \a nPoints from \a from into
    this array, starting at \a index in this array and \a fromIndex in
    \a from. \a fromIndex is 0 by default.

    \code
	QPointArray a;
	a.putPoints( 0, 3, 1,2, 0,0, 5,6 );
	// a is now the three-point array ( 1,2, 0,0, 5,6 );
	QPointArray b;
	b.putPoints( 0, 3, 4,4, 5,5, 6,6 );
	// b is now ( 4,4, 5,5, 6,6 );
	a.putPoints( 2, 3, b );
	// a is now ( 1,2, 0,0, 4,4, 5,5, 6,6 );
    \endcode
*/

bool QPointArray::putPoints( int index, int nPoints,
			     const QPointArray & from, int fromIndex )
{
    if ( index + nPoints > (int)size() ) {	// extend array
	if ( !resize(index + nPoints) )
	    return FALSE;
    }
    if ( nPoints <= 0 )
	return TRUE;
    int n = 0;
    while( n < nPoints ) {
	setPoint( index+n, from[fromIndex+n] );
	n++;
    }
    return TRUE;
}


/*!
    Returns the bounding rectangle of the points in the array, or
    QRect(0,0,0,0) if the array is empty.
*/

QRect QPointArray::boundingRect() const
{
    if ( isEmpty() )
	return QRect( 0, 0, 0, 0 );		// null rectangle
    register QPoint *pd = data();
    int minx, maxx, miny, maxy;
    minx = maxx = pd->x();
    miny = maxy = pd->y();
    pd++;
    for ( int i=1; i<(int)size(); i++ ) {	// find min+max x and y
	if ( pd->x() < minx )
	    minx = pd->x();
	else if ( pd->x() > maxx )
	    maxx = pd->x();
	if ( pd->y() < miny )
	    miny = pd->y();
	else if ( pd->y() > maxy )
	    maxy = pd->y();
	pd++;
    }
    return QRect( QPoint(minx,miny), QPoint(maxx,maxy) );
}


static inline int fix_angle( int a )
{
    if ( a > 16*360 )
	a %= 16*360;
    else if ( a < -16*360 ) {
	a = -( (-a) % (16*360) );
    }
    return a;
}

/*!
    Sets the points of the array to those describing an arc of an
    ellipse with size, width \a w by height \a h, and position (\a x,
    \a y), starting from angle \a a1 and spanning by angle \a a2. The
    resulting array has sufficient resolution for pixel accuracy (see
    the overloaded function which takes an additional QWMatrix
    parameter).

    Angles are specified in 16ths of a degree, i.e. a full circle
    equals 5760 (16*360). Positive values mean counter-clockwise,
    whereas negative values mean the clockwise direction. Zero degrees
    is at the 3 o'clock position.

    See the \link qcanvasellipse.html#anglediagram angle diagram\endlink.
*/

void QPointArray::makeArc( int x, int y, int w, int h, int a1, int a2 )
{
#if !defined(QT_OLD_MAKEELLIPSE) && !defined(QT_NO_TRANSFORMATIONS)
    QWMatrix unit;
    makeArc(x,y,w,h,a1,a2,unit);
#else
    a1 = fix_angle( a1 );
    if ( a1 < 0 )
	a1 += 16*360;
    a2 = fix_angle( a2 );
    int a3 = a2 > 0 ? a2 : -a2;			// abs angle
    makeEllipse( x, y, w, h );
    int npts = a3*size()/(16*360);		// # points in arc array
    QPointArray a(npts);
    int i = a1*size()/(16*360);
    int j = 0;
    if ( a2 > 0 ) {
	while ( npts-- ) {
	    if ( i >= (int)size() )			// wrap index
		i = 0;
	    a.QMemArray<QPoint>::at( j++ ) = QMemArray<QPoint>::at( i++ );
	}
    } else {
	while ( npts-- ) {
	    if ( i < 0 )				// wrap index
		i = (int)size()-1;
	    a.QMemArray<QPoint>::at( j++ ) = QMemArray<QPoint>::at( i-- );
	}
    }
    *this = a;
    return;
#endif
}

#ifndef QT_NO_TRANSFORMATIONS
// Based upon:
//   parelarc.c from Graphics Gems III
//   VanAken / Simar, "A Parametric Elliptical Arc Algorithm"
//
static void
qtr_elips(QPointArray& a, int off, double dxP, double dyP, double dxQ, double dyQ, double dxK, double dyK, int m)
{
#define PIV2  102944     /* fixed point PI/2 */
#define TWOPI 411775     /* fixed point 2*PI */
#define HALF  32768      /* fixed point 1/2 */

    int xP, yP, xQ, yQ, xK, yK;
    xP = int(dxP * 65536.0); yP = int(dyP * 65536.0);
    xQ = int(dxQ * 65536.0); yQ = int(dyQ * 65536.0);
    xK = int(dxK * 65536.0); yK = int(dyK * 65536.0);

    int i;
    int vx, ux, vy, uy, xJ, yJ;

    vx = xK - xQ;                 /* displacements from center */
    ux = xK - xP;
    vy = yK - yQ;
    uy = yK - yP;
    xJ = xP - vx + HALF;          /* center of ellipse J */
    yJ = yP - vy + HALF;

    int r;
    ux -= (r = ux >> (2*m + 3));  /* cancel 2nd-order error */
    ux -= (r >>= (2*m + 4));      /* cancel 4th-order error */
    ux -= r >> (2*m + 3);         /* cancel 6th-order error */
    ux += vx >> (m + 1);          /* cancel 1st-order error */
    uy -= (r = uy >> (2*m + 3));  /* cancel 2nd-order error */
    uy -= (r >>= (2*m + 4));      /* cancel 4th-order error */
    uy -= r >> (2*m + 3);         /* cancel 6th-order error */
    uy += vy >> (m + 1);          /* cancel 1st-order error */

    const int qn = a.size()/4;
    for (i = 0; i < qn; i++) {
        a[off+i] = QPoint((xJ + vx) >> 16, (yJ + vy) >> 16);
	ux -= vx >> m;
	vx += ux >> m;
	uy -= vy >> m;
	vy += uy >> m;
    }

#undef PIV2
#undef TWOPI
#undef HALF
}


/*!
    \overload

    Sets the points of the array to those describing an arc of an
    ellipse with width \a w and height \a h and position (\a x, \a y),
    starting from angle \a a1, and spanning angle by \a a2, and
    transformed by the matrix \a xf. The resulting array has
    sufficient resolution for pixel accuracy.

    Angles are specified in 16ths of a degree, i.e. a full circle
    equals 5760 (16*360). Positive values mean counter-clockwise,
    whereas negative values mean the clockwise direction. Zero degrees
    is at the 3 o'clock position.

    See the \link qcanvasellipse.html#anglediagram angle diagram\endlink.
*/
void QPointArray::makeArc( int x, int y, int w, int h,
			       int a1, int a2,
			       const QWMatrix& xf )
{
#define PIV2  102944     /* fixed point PI/2 */
    if ( --w < 0 || --h < 0 || !a2 ) {
	resize( 0 );
	return;
    }

    bool rev = a2 < 0;
    if ( rev ) {
	a1 += a2;
	a2 = -a2;
    }
    a1 = fix_angle( a1 );
    if ( a1 < 0 )
	a1 += 16*360;
    a2 = fix_angle( a2 );

    bool arc = a1 != 0 || a2 != 360*16 || rev;

    double xP, yP, xQ, yQ, xK, yK;

    xf.map(x+w, y+h/2.0, &xP, &yP);
    xf.map(x+w/2.0, y, &xQ, &yQ);
    xf.map(x+w, y, &xK, &yK);

    int m = 3;
    int max;
    int q = int(QMAX(QABS(xP-xQ),QABS(yP-yQ)));
    if ( arc )
	q *= 2;
    do {
	m++;
	max = 4*(1 + (PIV2 >> (16 - m)) );
    } while (max < q && m < 16); // 16 limits memory usage on HUGE arcs

    double inc = 1.0/(1<<m);

    const int qn = (PIV2 >> (16 - m));
    resize(qn*4);

    qtr_elips(*this, 0, xP, yP, xQ, yQ, xK, yK, m);
    xP = xQ; yP = yQ;
    xf.map(x, y+h/2.0, &xQ, &yQ);
    xf.map(x, y, &xK, &yK);
    qtr_elips(*this, qn, xP, yP, xQ, yQ, xK, yK, m);
    xP = xQ; yP = yQ;
    xf.map(x+w/2.0, y+h, &xQ, &yQ);
    xf.map(x, y+h, &xK, &yK);
    qtr_elips(*this, qn*2, xP, yP, xQ, yQ, xK, yK, m);
    xP = xQ; yP = yQ;
    xf.map(x+w, y+h/2.0, &xQ, &yQ);
    xf.map(x+w, y+h, &xK, &yK);
    qtr_elips(*this, qn*3, xP, yP, xQ, yQ, xK, yK, m);

    int n = size();

    if ( arc ) {
	double da1 = double(a1)*Q_PI / (360*8);
	double da3 = double(a2+a1)*Q_PI / (360*8);
	int i = int(da1/inc+0.5);
	int l = int(da3/inc+0.5);
	int k = (l-i)+1;
	QPointArray r(k);
	int j = 0;

	if ( rev ) {
	    while ( k-- )
		r[j++] = at((i+k)%n);
	} else {
	    while ( j < k ) {
		r[j] = at((i+j)%n);
		j++;
	    }
	}
	*this = r;
    }
#undef PIV2
}

#endif // QT_NO_TRANSFORMATIONS

/*!
    Sets the points of the array to those describing an ellipse with
    size, width \a w by height \a h, and position (\a x, \a y).

    The returned array has sufficient resolution for use as pixels.
*/
void QPointArray::makeEllipse( int x, int y, int w, int h )
{						// midpoint, 1/4 ellipse
#if !defined(QT_OLD_MAKEELLIPSE) && !defined(QT_NO_TRANSFORMATIONS)
    QWMatrix unit;
    makeArc(x,y,w,h,0,360*16,unit);
    return;
#else
    if ( w <= 0 || h <= 0 ) {
	if ( w == 0 || h == 0 ) {
	    resize( 0 );
	    return;
	}
	if ( w < 0 ) {				// negative width
	    w = -w;
	    x -= w;
	}
	if ( h < 0 ) {				// negative height
	    h = -h;
	    y -= h;
	}
    }
    int s = (w+h+2)/2;				// max size of xx,yy array
    int *px = new int[s];			// 1/4th of ellipse
    int *py = new int[s];
    int xx, yy, i=0;
    double d1, d2;
    double a2=(w/2)*(w/2),  b2=(h/2)*(h/2);
    xx = 0;
    yy = int(h/2);
    d1 = b2 - a2*(h/2) + 0.25*a2;
    px[i] = xx;
    py[i] = yy;
    i++;
    while ( a2*(yy-0.5) > b2*(xx+0.5) ) {		// region 1
	if ( d1 < 0 ) {
	    d1 = d1 + b2*(3.0+2*xx);
	    xx++;
	} else {
	    d1 = d1 + b2*(3.0+2*xx) + 2.0*a2*(1-yy);
	    xx++;
	    yy--;
	}
	px[i] = xx;
	py[i] = yy;
	i++;
    }
    d2 = b2*(xx+0.5)*(xx+0.5) + a2*(yy-1)*(yy-1) - a2*b2;
    while ( yy > 0 ) {				// region 2
	if ( d2 < 0 ) {
	    d2 = d2 + 2.0*b2*(xx+1) + a2*(3-2*yy);
	    xx++;
	    yy--;
	} else {
	    d2 = d2 + a2*(3-2*yy);
	    yy--;
	}
	px[i] = xx;
	py[i] = yy;
	i++;
    }
    s = i;
    resize( 4*s );				// make full point array
    x += w/2;
    y += h/2;
    for ( i=0; i<s; i++ ) {			// mirror
	xx = px[i];
	yy = py[i];
	setPoint( s-i-1, x+xx, y-yy );
	setPoint( s+i, x-xx, y-yy );
	setPoint( 3*s-i-1, x-xx, y+yy );
	setPoint( 3*s+i, x+xx, y+yy );
    }
    delete[] px;
    delete[] py;
#endif
}

#ifndef QT_NO_BEZIER
// Work functions for QPointArray::cubicBezier()
static
void split(const double *p, double *l, double *r)
{
    double tmpx;
    double tmpy;

    l[0] =  p[0];
    l[1] =  p[1];
    r[6] =  p[6];
    r[7] =  p[7];

    l[2] = (p[0]+ p[2])/2;
    l[3] = (p[1]+ p[3])/2;
    tmpx = (p[2]+ p[4])/2;
    tmpy = (p[3]+ p[5])/2;
    r[4] = (p[4]+ p[6])/2;
    r[5] = (p[5]+ p[7])/2;

    l[4] = (l[2]+ tmpx)/2;
    l[5] = (l[3]+ tmpy)/2;
    r[2] = (tmpx + r[4])/2;
    r[3] = (tmpy + r[5])/2;

    l[6] = (l[4]+ r[2])/2;
    l[7] = (l[5]+ r[3])/2;
    r[0] = l[6];
    r[1] = l[7];
}

// Based on:
//
//   A Fast 2D Point-On-Line Test
//   by Alan Paeth
//   from "Graphics Gems", Academic Press, 1990
static
int pnt_on_line( const int* p, const int* q, const int* t )
{
/*
 * given a line through P:(px,py) Q:(qx,qy) and T:(tx,ty)
 * return 0 if T is not on the line through      <--P--Q-->
 *        1 if T is on the open ray ending at P: <--P
 *        2 if T is on the closed interior along:   P--Q
 *        3 if T is on the open ray beginning at Q:    Q-->
 *
 * Example: consider the line P = (3,2), Q = (17,7). A plot
 * of the test points T(x,y) (with 0 mapped onto '.') yields:
 *
 *     8| . . . . . . . . . . . . . . . . . 3 3
 *  Y  7| . . . . . . . . . . . . . . 2 2 Q 3 3    Q = 2
 *     6| . . . . . . . . . . . 2 2 2 2 2 . . .
 *  a  5| . . . . . . . . 2 2 2 2 2 2 . . . . .
 *  x  4| . . . . . 2 2 2 2 2 2 . . . . . . . .
 *  i  3| . . . 2 2 2 2 2 . . . . . . . . . . .
 *  s  2| 1 1 P 2 2 . . . . . . . . . . . . . .    P = 2
 *     1| 1 1 . . . . . . . . . . . . . . . . .
 *      +--------------------------------------
 *        1 2 3 4 5 X-axis 10        15      19
 *
 * Point-Line distance is normalized with the Infinity Norm
 * avoiding square-root code and tightening the test vs the
 * Manhattan Norm. All math is done on the field of integers.
 * The latter replaces the initial ">= MAX(...)" test with
 * "> (ABS(qx-px) + ABS(qy-py))" loosening both inequality
 * and norm, yielding a broader target line for selection.
 * The tightest test is employed here for best discrimination
 * in merging collinear (to grid coordinates) vertex chains
 * into a larger, spanning vectors within the Lemming editor.
 */

	// if all points are coincident, return condition 2 (on line)
	if(q[0]==p[0] && q[1]==p[1] && q[0]==t[0] && q[1]==t[1]) {
		return 2;
	}

    if ( QABS((q[1]-p[1])*(t[0]-p[0])-(t[1]-p[1])*(q[0]-p[0])) >=
	(QMAX(QABS(q[0]-p[0]), QABS(q[1]-p[1])))) return 0;

    if (((q[0]<p[0])&&(p[0]<t[0])) || ((q[1]<p[1])&&(p[1]<t[1])))
	return 1 ;
    if (((t[0]<p[0])&&(p[0]<q[0])) || ((t[1]<p[1])&&(p[1]<q[1])))
	return 1 ;
    if (((p[0]<q[0])&&(q[0]<t[0])) || ((p[1]<q[1])&&(q[1]<t[1])))
	return 3 ;
    if (((t[0]<q[0])&&(q[0]<p[0])) || ((t[1]<q[1])&&(q[1]<p[1])))
	return 3 ;

    return 2 ;
}
static
void polygonizeQBezier( double* acc, int& accsize, const double ctrl[],
			int maxsize )
{
    if ( accsize > maxsize / 2 )
    {
	// This never happens in practice.

	if ( accsize >= maxsize-4 )
	    return;
	// Running out of space - approximate by a line.
	acc[accsize++] = ctrl[0];
	acc[accsize++] = ctrl[1];
	acc[accsize++] = ctrl[6];
	acc[accsize++] = ctrl[7];
	return;
    }

    //intersects:
    double l[8];
    double r[8];
    split( ctrl, l, r);

    // convert to integers for line condition check
    int c0[2]; c0[0] = int(ctrl[0]); c0[1] = int(ctrl[1]);
    int c1[2]; c1[0] = int(ctrl[2]); c1[1] = int(ctrl[3]);
    int c2[2]; c2[0] = int(ctrl[4]); c2[1] = int(ctrl[5]);
    int c3[2]; c3[0] = int(ctrl[6]); c3[1] = int(ctrl[7]);

    // #### Duplication needed?
    if ( QABS(c1[0]-c0[0]) <= 1 && QABS(c1[1]-c0[1]) <= 1
      && QABS(c2[0]-c0[0]) <= 1 && QABS(c2[1]-c0[1]) <= 1
      && QABS(c3[0]-c1[0]) <= 1 && QABS(c3[1]-c0[1]) <= 1 )
    {
	// Approximate by one line.
	// Dont need to write last pt as it is the same as first pt
	// on the next segment
	acc[accsize++] = l[0];
	acc[accsize++] = l[1];
	return;
    }

    if ( ( pnt_on_line( c0, c3, c1 ) == 2 && pnt_on_line( c0, c3, c2 ) == 2 )
      || ( QABS(c1[0]-c0[0]) <= 1 && QABS(c1[1]-c0[1]) <= 1
	&& QABS(c2[0]-c0[0]) <= 1 && QABS(c2[1]-c0[1]) <= 1
	&& QABS(c3[0]-c1[0]) <= 1 && QABS(c3[1]-c0[1]) <= 1 ) )
    {
	// Approximate by one line.
	// Dont need to write last pt as it is the same as first pt
	// on the next segment
	acc[accsize++] = l[0];
	acc[accsize++] = l[1];
	return;
    }

    // Too big and too curved - recusively subdivide.
    polygonizeQBezier( acc, accsize, l, maxsize );
    polygonizeQBezier( acc, accsize, r, maxsize );
}

/*!
    Returns the Bezier points for the four control points in this
    array.
*/

QPointArray QPointArray::cubicBezier() const
{
#ifdef USE_SIMPLE_QBEZIER_CODE
    if ( size() != 4 ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QPointArray::bezier: The array must have 4 control points" );
#endif
	QPointArray p;
	return p;
    }

    int v;
    float xvec[4];
    float yvec[4];
    for ( v=0; v<4; v++ ) {			// store all x,y in xvec,yvec
	int x, y;
	point( v, &x, &y );
	xvec[v] = (float)x;
	yvec[v] = (float)y;
    }

    QRect r = boundingRect();
    int m = QMAX(r.width(),r.height())/2;
    m = QMIN(m,30);				// m = number of result points
    if ( m < 2 )				// at least two points
	m = 2;
    QPointArray p( m );				// p = Bezier point array
    register QPointData *pd = p.data();

    float x0 = xvec[0],	 y0 = yvec[0];
    float dt = 1.0F/m;
    float cx = 3.0F * (xvec[1] - x0);
    float bx = 3.0F * (xvec[2] - xvec[1]) - cx;
    float ax = xvec[3] - (x0 + cx + bx);
    float cy = 3.0F * (yvec[1] - y0);
    float by = 3.0F * (yvec[2] - yvec[1]) - cy;
    float ay = yvec[3] - (y0 + cy + by);
    float t = dt;

    pd->rx() = (QCOORD)xvec[0];
    pd->ry() = (QCOORD)yvec[0];
    pd++;
    m -= 2;

    while ( m-- ) {
	pd->rx() = (QCOORD)qRound( ((ax * t + bx) * t + cx) * t + x0 );
	pd->ry() = (QCOORD)qRound( ((ay * t + by) * t + cy) * t + y0 );
	pd++;
	t += dt;
    }

    pd->rx() = (QCOORD)xvec[3];
    pd->ry() = (QCOORD)yvec[3];

    return p;
#else

    if ( size() != 4 ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QPointArray::bezier: The array must have 4 control points" );
#endif
	QPointArray pa;
	return pa;
    } else {
	QRect r = boundingRect();
	int m = 4+2*QMAX(r.width(),r.height());
	double *p = new double[m];
	double ctrl[8];
	int i;
	for (i=0; i<4; i++) {
	    ctrl[i*2] = at(i).x();
	    ctrl[i*2+1] = at(i).y();
	}
	int len=0;
	polygonizeQBezier( p, len, ctrl, m );
	QPointArray pa((len/2)+1); // one extra point for last point on line
	int j=0;
	for (i=0; j<len; i++) {
	    int x = qRound(p[j++]);
	    int y = qRound(p[j++]);
	    pa[i] = QPoint(x,y);
	}
	// add last pt on the line, which will be at the last control pt
	pa[(int)pa.size()-1] = at(3);
	delete[] p;

	return pa;
    }

#endif
}
#endif //QT_NO_BEZIER

/*****************************************************************************
  QPointArray stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \relates QPointArray

    Writes the point array, \a a to the stream \a s and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<( QDataStream &s, const QPointArray &a )
{
    register uint i;
    uint len = a.size();
    s << len;					// write size of array
    for ( i=0; i<len; i++ )			// write each point
	s << a.point( i );
    return s;
}

/*!
    \relates QPointArray

    Reads a point array, \a a from the stream \a s and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QPointArray &a )
{
    register uint i;
    uint len;
    s >> len;					// read size of array
    if ( !a.resize( len ) )			// no memory
	return s;
    QPoint p;
    for ( i=0; i<len; i++ ) {			// read each point
	s >> p;
	a.setPoint( i, p );
    }
    return s;
}
#endif //QT_NO_DATASTREAM



struct QShortPoint {			// Binary compatible with XPoint
    short x, y;
};

uint QPointArray::splen = 0;
void* QPointArray::sp = 0;		// Really a QShortPoint*

/*!
  \internal

  Converts the point coords to short (16bit) size, compatible with
  X11's XPoint structure. The pointer returned points to a static
  array, so its contents will be overwritten the next time this
  function is called.
*/

void* QPointArray::shortPoints( int index, int nPoints ) const
{

    if ( isNull() || !nPoints )
	return 0;
    QPoint* p = data();
    p += index;
    uint i = nPoints < 0 ? size() : nPoints;
    if ( splen < i ) {
	if ( sp )
	    delete[] ((QShortPoint*)sp);
	sp = new QShortPoint[i];
	splen = i;
    }
    QShortPoint* ps = (QShortPoint*)sp;
    while ( i-- ) {
	ps->x = (short)p->x();
	ps->y = (short)p->y();
	p++;
	ps++;
    }
    return sp;
}


/*!
  \internal

  Deallocates the internal buffer used by shortPoints().
*/

void QPointArray::cleanBuffers()
{
    if ( sp )
	delete[] ((QShortPoint*)sp);
    sp = 0;
    splen = 0;
}
