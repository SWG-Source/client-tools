/****************************************************************************
** $Id: qcursor.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QCursor class
**
** Created : 940220
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

#include "qcursor.h"

#ifndef QT_NO_CURSOR

#include "qbitmap.h"
#include "qimage.h"
#include "qdatastream.h"


/*!
    \class QCursor qcursor.h

    \brief The QCursor class provides a mouse cursor with an arbitrary
    shape.

    \ingroup appearance
    \ingroup shared

    This class is mainly used to create mouse cursors that are
    associated with particular widgets and to get and set the position
    of the mouse cursor.

    Qt has a number of standard cursor shapes, but you can also make
    custom cursor shapes based on a QBitmap, a mask and a hotspot.

    To associate a cursor with a widget, use QWidget::setCursor(). To
    associate a cursor with all widgets (normally for a short period
    of time), use QApplication::setOverrideCursor().

    To set a cursor shape use QCursor::setShape() or use the QCursor
    constructor which takes the shape as argument, or you can use one
    of the predefined cursors defined in the \l CursorShape enum.

    If you want to create a cursor with your own bitmap, either use
    the QCursor constructor which takes a bitmap and a mask or the
    constructor which takes a pixmap as arguments.

    To set or get the position of the mouse cursor use the static
    methods QCursor::pos() and QCursor::setPos().

    \img cursors.png Cursor Shapes

    \sa QWidget \link guibooks.html#fowler GUI Design Handbook:
    Cursors\endlink

    On X11, Qt supports the \link
    http://www.xfree86.org/4.3.0/Xcursor.3.html Xcursor\endlink
    library, which allows for full color icon themes. The table below
    shows the cursor name used for each Qt::CursorShape value. If a
    cursor cannot be found using the name shown below, a standard X11
    cursor will be used instead. Note: X11 does not provide
    appropriate cursors for all possible Qt::CursorShape values. It
    is possible that some cursors will be taken from the Xcursor
    theme, while others will use an internal bitmap cursor.

    \table
    \header \i Qt::CursorShape Values   \i Cursor Names
    \row \i Qt::ArrowCursor             \i left_ptr
    \row \i Qt::UpArrowCursor           \i up_arrow
    \row \i Qt::CrossCursor             \i cross
    \row \i Qt::WaitCursor              \i wait
    \row \i Qt::BusyCursor		\i left_ptr_watch
    \row \i Qt::IbeamCursor             \i ibeam
    \row \i Qt::SizeVerCursor           \i size_ver
    \row \i Qt::SizeHorCursor           \i size_hor
    \row \i Qt::SizeBDiagCursor         \i size_bdiag
    \row \i Qt::SizeFDiagCursor         \i size_fdiag
    \row \i Qt::SizeAllCursor           \i size_all
    \row \i Qt::SplitVCursor            \i split_v
    \row \i Qt::SplitHCursor            \i split_h
    \row \i Qt::PointingHandCursor      \i pointing_hand
    \row \i Qt::ForbiddenCursor         \i forbidden
    \row \i Qt::WhatsThisCursor         \i whats_this
    \endtable
*/

/*!
    \enum Qt::CursorShape

    This enum type defines the various cursors that can be used.

    \value ArrowCursor  standard arrow cursor
    \value UpArrowCursor  upwards arrow
    \value CrossCursor  crosshair
    \value WaitCursor  hourglass/watch
    \value BusyCursor  standard arrow with hourglass/watch
    \value IbeamCursor  ibeam/text entry
    \value SizeVerCursor  vertical resize
    \value SizeHorCursor  horizontal resize
    \value SizeFDiagCursor  diagonal resize (\)
    \value SizeBDiagCursor  diagonal resize (/)
    \value SizeAllCursor  all directions resize
    \value BlankCursor  blank/invisible cursor
    \value SplitVCursor  vertical splitting
    \value SplitHCursor  horizontal splitting
    \value PointingHandCursor  a pointing hand
    \value ForbiddenCursor  a slashed circle
    \value WhatsThisCursor  an arrow with a question mark
    \value BitmapCursor

    ArrowCursor is the default for widgets in a normal state.

    \img cursors.png Cursor Shapes
*/

/*****************************************************************************
  QCursor stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM


/*!
    \relates QCursor
    Writes the cursor \a c to the stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<( QDataStream &s, const QCursor &c )
{
    s << (Q_INT16)c.shape();			// write shape id to stream
    if ( c.shape() == Qt::BitmapCursor ) {		// bitmap cursor
#if !defined(QT_NO_IMAGEIO)
	s << *c.bitmap() << *c.mask();
	s << c.hotSpot();
#else
	qWarning("No Image Cursor I/O");
#endif
    }
    return s;
}

/*!
    \relates QCursor
    Reads a cursor from the stream \a s and sets \a c to the read data.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QCursor &c )
{
    Q_INT16 shape;
    s >> shape;					// read shape id from stream
    if ( shape == Qt::BitmapCursor ) {		// read bitmap cursor
#if !defined(QT_NO_IMAGEIO)
	QBitmap bm, bmm;
	QPoint	hot;
	s >> bm >> bmm >> hot;
	c = QCursor( bm, bmm, hot.x(), hot.y() );
#else
	qWarning("No Image Cursor I/O");
#endif
    } else {
	c.setShape( (int)shape );		// create cursor with shape
    }
    return s;
}
#endif // QT_NO_DATASTREAM


/*!
    Constructs a custom pixmap cursor.

    \a pixmap is the image. It is usual to give it a mask (set using
    QPixmap::setMask()). \a hotX and \a hotY define the cursor's hot
    spot.

    If \a hotX is negative, it is set to the \c{pixmap().width()/2}.
    If \a hotY is negative, it is set to the \c{pixmap().height()/2}.

    Valid cursor sizes depend on the display hardware (or the
    underlying window system). We recommend using 32x32 cursors,
    because this size is supported on all platforms. Some platforms
    also support 16x16, 48x48 and 64x64 cursors.

    Currently, only black-and-white pixmaps can be used.

    \sa QPixmap::QPixmap(), QPixmap::setMask()
*/

QCursor::QCursor( const QPixmap &pixmap, int hotX, int hotY )
{
    QImage img = pixmap.convertToImage().
		    convertDepth( 8, Qt::ThresholdDither|Qt::AvoidDither );
    QBitmap bm;
    bm.convertFromImage( img, Qt::ThresholdDither|Qt::AvoidDither );
    QBitmap bmm;
    if ( bm.mask() ) {
	bmm = *bm.mask();
	QBitmap nullBm;
	bm.setMask( nullBm );
    }
    else if ( pixmap.mask() ) {
	QImage mimg = pixmap.mask()->convertToImage().
	              convertDepth( 8, Qt::ThresholdDither|Qt::AvoidDither );
	bmm.convertFromImage( mimg, Qt::ThresholdDither|Qt::AvoidDither );
    }
    else {
	bmm.resize( bm.size() );
	bmm.fill( Qt::color1 );
    }

    setBitmap(bm,bmm,hotX,hotY);
}



/*!
    Constructs a custom bitmap cursor.

    \a bitmap and
    \a mask make up the bitmap.
    \a hotX and
    \a hotY define the cursor's hot spot.

    If \a hotX is negative, it is set to the \c{bitmap().width()/2}.
    If \a hotY is negative, it is set to the \c{bitmap().height()/2}.

    The cursor \a bitmap (B) and \a mask (M) bits are combined like this:
    \list
    \i B=1 and M=1 gives black.
    \i B=0 and M=1 gives white.
    \i B=0 and M=0 gives transparent.
    \i B=1 and M=0 gives an undefined result.
    \endlist

    Use the global Qt color \c color0 to draw 0-pixels and \c color1 to
    draw 1-pixels in the bitmaps.

    Valid cursor sizes depend on the display hardware (or the
    underlying window system). We recommend using 32x32 cursors,
    because this size is supported on all platforms. Some platforms
    also support 16x16, 48x48 and 64x64 cursors.

    \sa QBitmap::QBitmap(), QBitmap::setMask()
*/

QCursor::QCursor( const QBitmap &bitmap, const QBitmap &mask,
		  int hotX, int hotY )
{
    setBitmap(bitmap,mask,hotX,hotY);
}

#endif // QT_NO_CURSOR


