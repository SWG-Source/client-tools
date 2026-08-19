/****************************************************************************
** $Id: qglcolormap.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QGLColormap class
**
** Created : 20010326
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the opengl module of the Qt GUI Toolkit.
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

/*!
    \class QGLColormap qglcolormap.h
    \brief The QGLColormap class is used for installing custom colormaps into
    QGLWidgets.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module OpenGL
    \ingroup graphics
    \ingroup images

    QGLColormap provides a platform independent way of specifying and
    installing indexed colormaps into QGLWidgets. QGLColormap is
    especially useful when using the \link opengl.html OpenGL\endlink
    color-index mode.

    Under X11 you must use an X server that supports either a \c
    PseudoColor or \c DirectColor visual class. If your X server
    currently only provides a \c GrayScale, \c TrueColor, \c
    StaticColor or \c StaticGray visual, you will not be able to
    allocate colorcells for writing. If this is the case, try setting
    your X server to 8 bit mode. It should then provide you with at
    least a \c PseudoColor visual. Note that you may experience
    colormap flashing if your X server is running in 8 bit mode.

    Under Windows the size of the colormap is always set to 256
    colors. Note that under Windows you can also install colormaps
    in child widgets.

    This class uses explicit sharing (see \link shclass.html Shared
    Classes\endlink).

    Example of use:
    \code
    #include <qapplication.h>
    #include <qglcolormap.h>

    int main()
    {
	QApplication a( argc, argv );

	MySuperGLWidget widget( 0 ); // A QGLWidget in color-index mode
	QGLColormap colormap;

	// This will fill the colormap with colors ranging from
	// black to white.
	for ( int i = 0; i < colormap.size(); i++ )
	    colormap.setEntry( i, qRgb( i, i, i ) );

	widget.setColormap( colormap );
	widget.show();
	return a.exec();
    }
    \endcode

    \sa QGLWidget::setColormap(), QGLWidget::colormap()
*/

#include "qglcolormap.h"
#include "qmemarray.h"


/*!
    Construct a QGLColormap.
*/
QGLColormap::QGLColormap()
{
    d = 0;
}


/*!
    Construct a shallow copy of \a map.
*/
QGLColormap::QGLColormap( const QGLColormap & map )
{
    d = map.d;
    if ( d )
	d->ref();
}

/*!
    Dereferences the QGLColormap and deletes it if this was the last
    reference to it.
*/
QGLColormap::~QGLColormap()
{
    if ( d && d->deref() ) {
	delete d;
	d = 0;
    }
}

/*!
    Assign a shallow copy of \a map to this QGLColormap.
*/
QGLColormap & QGLColormap::operator=( const QGLColormap & map )
{
    if ( map.d != 0 )
	map.d->ref();

    if ( d && d->deref() )
	delete d;
    d = map.d;

    return *this;
}

/*!
    Detaches this QGLColormap from the shared block.
*/
void QGLColormap::detach()
{
    if ( d && d->count != 1 ) {
	// ### What about the actual colormap handle?
	Private * newd = new Private();
	newd->cells = d->cells;
	newd->cells.detach();
	if ( d->deref() )
	    delete d;
	d = newd;
    }
}

/*!
    Set cell at index \a idx in the colormap to color \a color.
*/
void QGLColormap::setEntry( int idx, QRgb color )
{
    if ( !d )
	d = new Private();

#if defined(QT_CHECK_RANGE)
    if ( idx < 0 || idx > (int) d->cells.size() ) {
	qWarning( "QGLColormap::setRgb: Index out of range." );
	return;
    }
#endif
    d->cells[ idx ] = color;
}

/*!
    Set an array of cells in this colormap. \a count is the number of
    colors that should be set, \a colors is the array of colors, and
    \a base is the starting index.
*/
void QGLColormap::setEntries( int count, const QRgb * colors, int base )
{
    if ( !d )
	d = new Private();

    if ( !colors || base < 0 || base >= (int) d->cells.size() )
	return;

    for( int i = base; i < base + count; i++ ) {
	if ( i < (int) d->cells.size() )
	    setEntry( i, colors[i] );
	else
	    break;
    }
}

/*!
    Returns the QRgb value in the colorcell with index \a idx.
*/
QRgb QGLColormap::entryRgb( int idx ) const
{
    if ( !d || idx < 0 || idx > (int) d->cells.size() )
	return 0;
    else
	return d->cells[ idx ];
}

/*!
    \overload

    Set the cell with index \a idx in the colormap to color \a color.
*/
void QGLColormap::setEntry( int idx, const QColor & color )
{
    setEntry( idx, color.rgb() );
}

/*!
    Returns the QRgb value in the colorcell with index \a idx.
*/
QColor QGLColormap::entryColor( int idx ) const
{
    if ( !d || idx < 0 || idx > (int) d->cells.size() )
	return QColor();
    else
	return QColor( d->cells[ idx ] );
}

/*!
    Returns TRUE if the colormap is empty; otherwise returns FALSE. A
    colormap with no color values set is considered to be empty.
*/
bool QGLColormap::isEmpty() const
{
    return (d == 0) || (d->cells.size() == 0)  || (d->cmapHandle == 0);
}


/*!
    Returns the number of colorcells in the colormap.
*/
int QGLColormap::size() const
{
    return d != 0 ? d->cells.size() : 0;
}

/*!
    Returns the index of the color \a color. If \a color is not in the
    map, -1 is returned.
*/
int QGLColormap::find( QRgb color ) const
{
    if ( d )
	return d->cells.find( color );
    return -1;
}

/*!
    Returns the index of the color that is the closest match to color
    \a color.
*/
int QGLColormap::findNearest( QRgb color ) const
{
    int idx = find( color );
    if ( idx >= 0 )
	return idx;
    int mapSize = size();
    int mindist = 200000;
    int r = qRed( color );
    int g = qGreen( color );
    int b = qBlue( color );
    int rx, gx, bx, dist;
    for ( int i=0; i < mapSize; i++ ) {
	QRgb ci = d->cells[i];
	rx = r - qRed( ci );
	gx = g - qGreen( ci );
	bx = b - qBlue( ci );
	dist = rx*rx + gx*gx + bx*bx;	// calculate distance
	if ( dist < mindist ) {		// minimal?
	    mindist = dist;
	    idx = i;
	}
    }
    return idx;
}
