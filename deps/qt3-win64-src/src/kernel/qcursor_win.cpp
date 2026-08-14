/****************************************************************************
** $Id: qcursor_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QCursor class for Windows
**
** Created : 20030119
**
** Copyright (C) 2003 Holger Schroeder
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2004 Christian Ehrlicher
** Copyright (C) 2004 Andreas Hausladen
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

//#define QCURSOR_DEBUG

#include "qcursor.h"
#include "qbitmap.h"
#include "qbuffer.h"
#include "qimage.h"
#include "qapplication.h"
#include "qdatastream.h"
#include "qnamespace.h"
#include "qpainter.h"
#include "qt_windows.h"

#ifndef IDC_HAND
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif

/*****************************************************************************
  Internal QCursorData class
 *****************************************************************************/

struct QCursorData : public QShared
{
    QCursorData( int s = 0 );
    ~QCursorData();
    int cshape;
    QBitmap *bm, *bmm;
    short hx, hy;
    HCURSOR hcurs;
};

QCursorData::QCursorData( int s )
{
#ifdef QCURSOR_DEBUG
    qDebug( "QCursorData::QCursorData (s = %d)", s );
#endif

    cshape = s;
    hcurs = 0;
    bm = bmm = 0;
    hx = hy = 0;
}

QCursorData::~QCursorData()
{
    if ( hcurs && ( cshape == Qt::BitmapCursor ) ) {
        // We could use CreateIcon() or CreateCursor for this...
        if ( !DestroyIcon( hcurs ) )
            DestroyCursor( hcurs );
    }
    delete bm;
    delete bmm;
}


/*****************************************************************************
  Global cursors
 *****************************************************************************/

static QCursor cursorTable[ Qt::LastCursor + 1 ];
static const int arrowCursorIdx = 0;
static bool initialized = FALSE;

QT_STATIC_CONST_IMPL QCursor & Qt::arrowCursor = cursorTable[ 0 ];
QT_STATIC_CONST_IMPL QCursor & Qt::upArrowCursor = cursorTable[ 1 ];
QT_STATIC_CONST_IMPL QCursor & Qt::crossCursor = cursorTable[ 2 ];
QT_STATIC_CONST_IMPL QCursor & Qt::waitCursor = cursorTable[ 3 ];
QT_STATIC_CONST_IMPL QCursor & Qt::ibeamCursor = cursorTable[ 4 ];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeVerCursor = cursorTable[ 5 ];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeHorCursor = cursorTable[ 6 ];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeBDiagCursor = cursorTable[ 7 ];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeFDiagCursor = cursorTable[ 8 ];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeAllCursor = cursorTable[ 9 ];
QT_STATIC_CONST_IMPL QCursor & Qt::blankCursor = cursorTable[ 10 ];
QT_STATIC_CONST_IMPL QCursor & Qt::splitHCursor = cursorTable[ 11 ];
QT_STATIC_CONST_IMPL QCursor & Qt::splitVCursor = cursorTable[ 12 ];
QT_STATIC_CONST_IMPL QCursor & Qt::pointingHandCursor = cursorTable[ 13 ];
QT_STATIC_CONST_IMPL QCursor & Qt::forbiddenCursor = cursorTable[ 14 ];
QT_STATIC_CONST_IMPL QCursor & Qt::whatsThisCursor = cursorTable[ 15 ];
QT_STATIC_CONST_IMPL QCursor & Qt::busyCursor = cursorTable[ 16 ];

/*!
  Creates a cursor with the specified window system handle.

  Warning: Portable in principle, but if you use it you are probably about to
           do something non-portable. Be careful.
*/

QCursor::QCursor( HCURSOR handle )
{
    if ( !initialized )
        initialize();

    data = new QCursorData;
    CHECK_PTR( data );
    if ( handle != 0 ) {
        ICONINFO iconInfo;
        GetIconInfo( handle, &iconInfo );
        data->hx = iconInfo.xHotspot;
        data->hy = iconInfo.yHotspot;
        DeleteObject( iconInfo.hbmMask );
        DeleteObject( iconInfo.hbmColor );
    } else {
        data->hx = 0;
        data->hy = 0;
    }
    data->hcurs = handle;
    data->bm = 0;
    data->bmm = 0;

    /* a switch statement isn't allowed here (see C++ standard 6.4.2 or search for
       "a casts to a type other than an integral or enumeration type cannot appear
        in a constant-expression") Seems like all compilers ignore this. Only
        gcc >=3.4 complains about this problem...
    */
    if ( handle == ( HICON ) IDC_ARROW )
        data->cshape = Qt::ArrowCursor;
    else if ( handle == ( HICON ) IDC_UPARROW )
        data->cshape = Qt::UpArrowCursor;
    else if ( handle == ( HICON ) IDC_CROSS )
        data->cshape = Qt::CrossCursor;
    else if ( handle == ( HICON ) IDC_WAIT )
        data->cshape = Qt::WaitCursor;
    else if ( handle == ( HICON ) IDC_IBEAM )
        data->cshape = Qt::IbeamCursor;
    else if ( handle == ( HICON ) IDC_SIZEALL )
        data->cshape = Qt::SizeAllCursor;
    else if ( handle == ( HICON ) IDC_SIZENS )
        data->cshape = Qt::SizeVerCursor;
    else if ( handle == ( HICON ) IDC_SIZEWE )
        data->cshape = Qt::SizeHorCursor;
    else if ( handle == ( HICON ) IDC_SIZENESW )
        data->cshape = Qt::SizeBDiagCursor;
    else if ( handle == ( HICON ) IDC_SIZENWSE )
        data->cshape = Qt::SizeFDiagCursor;
    else if ( handle == ( HICON ) 0 )
        data->cshape = Qt::BlankCursor;
    else if ( handle == ( HICON ) IDC_HAND )
        data->cshape = Qt::PointingHandCursor;
    else if ( handle == ( HICON ) IDC_NO )
        data->cshape = Qt::ForbiddenCursor;
    else if ( handle == ( HICON ) IDC_HELP )
        data->cshape = Qt::WhatsThisCursor;
    else if ( handle == ( HICON ) IDC_APPSTARTING )
        data->cshape = Qt::BusyCursor;
    else
        data->cshape = Qt::BitmapCursor;
}


QCursor *QCursor::find_cur( int shape )       // find predefined cursor
{
    return ( uint ) shape <= LastCursor ? &cursorTable[ shape ] : 0;
}

/*!
  Internal function that deinitializes the predefined cursors.
  This function is called from the QApplication destructor.
  \sa initialize()
*/
void QCursor::cleanup()
{
    if ( !initialized )
        return ;

    int shape;
    for ( shape = 0; shape <= LastCursor; shape++ ) {
        if ( cursorTable[ shape ].data && cursorTable[ shape ].data->deref() )
            delete cursorTable[ shape ].data;
        cursorTable[ shape ].data = 0;
    }
    initialized = FALSE;
}

/*!
  Internal function that initializes the predefined cursors.
  This function is called from the QApplication constructor.
  \sa cleanup()
*/

void QCursor::initialize()
{
#ifdef QCURSOR_DEBUG
    qDebug( "QCursor::initialize()" );
#endif

    int shape;
    for ( shape = 0; shape <= LastCursor; shape++ )
        cursorTable[ shape ].data = new QCursorData( shape );
    initialized = TRUE;
    qAddPostRoutine( cleanup );
}


/*!
  Constructs a cursor with the default arrow shape.
*/
QCursor::QCursor()
{
    if ( !initialized ) {
        if ( qApp->startingUp() ) {
            data = 0;
            return ;
        }
        initialize();
    }
    QCursor* c = &cursorTable[ arrowCursorIdx ];
    c->data->ref();
    data = c->data;
}



/*!
  Constructs a cursor with the specified \a shape.

  \a shape can be one of
  <ul>
  <li> \c ArrowCursor - standard arrow cursor
  <li> \c UpArrowCursor - upwards arrow
  <li> \c CrossCursor - crosshair
  <li> \c WaitCursor - hourglass/watch
  <li> \c IbeamCursor - ibeam/text entry
  <li> \c SizeVerCursor - vertical resize
  <li> \c SizeHorCursor - horizontal resize
  <li> \c SizeBDiagCursor - diagonal resize (/)
  <li> \c SizeFDiagCursor - diagonal resize (\)
  <li> \c SizeAllCursor - all directions resize
  <li> \c BlankCursor - blank/invisible cursor
  <li> \c SplitVCursor - vertical splitting
  <li> \c SplitHCursor - horziontal splitting
  <li> \c PointingHandCursor - a pointing hand
  <li> \c BitmapCursor - userdefined bitmap cursor
  </ul>

  These correspond to the <a href="#cursors">predefined</a>
  global QCursor objects.

  \sa setShape()
*/

QCursor::QCursor( int shape )
{
#ifdef QCURSOR_DEBUG
    qDebug( "QCursor::QCursor( shape = %d )", shape );
#endif

    if ( !initialized )
        initialize();
    QCursor *c = find_cur( shape );
    if ( !c )           // not found
        c = &cursorTable[ arrowCursorIdx ]; //   then use arrowCursor
    c->data->ref();
    data = c->data;
}


void QCursor::setBitmap( const QBitmap &bitmap, const QBitmap &mask,
                         int hotX, int hotY )
{
#ifdef QCURSOR_DEBUG
    qDebug( "QCursor::setBitmap()" );
#endif

    if ( !initialized )
        initialize();
    if ( bitmap.depth() != 1 || mask.depth() != 1 ||
            bitmap.size() != mask.size() ) {
#if defined(CHECK_NULL)
        qWarning( "QCursor: Cannot create bitmap cursor; invalid bitmap(s)" );
#endif

        QCursor *c = &cursorTable[ arrowCursorIdx ];
        c->data->ref();
        data = c->data;
        return ;
    }
    data = new QCursorData;
    CHECK_PTR( data );
    data->bm = new QBitmap( bitmap );
    data->bmm = new QBitmap( mask );
    data->hcurs = 0;
    data->cshape = BitmapCursor;
    data->hx = hotX >= 0 ? hotX : bitmap.width() / 2;
    data->hy = hotY >= 0 ? hotY : bitmap.height() / 2;
}


/*!
  Constructs a copy of the cursor \a c.
*/

QCursor::QCursor( const QCursor &c )
{
#ifdef QCURSOR_DEBUG
    qDebug( "QCursor::QCursor( const QCursor &c )" );
#endif

    if ( !initialized )
        initialize();
    data = c.data;    // shallow copy
    data->ref();
}

/*!
  Destructs the cursor.
*/

QCursor::~QCursor()
{
#ifdef QCURSOR_DEBUG
    qDebug( "QCursor::~QCursor()" );
#endif

    if ( data && data->deref() )
        delete data;
}


/*!
  Assigns \a c to this cursor and returns a reference to this cursor.
*/

QCursor &QCursor::operator=( const QCursor &c )
{
#ifdef QCURSOR_DEBUG
    qDebug( "QCursor &QCursor::operator=( const QCursor &c )" );
#endif

    if ( !initialized )
        initialize();
    c.data->ref();    // avoid c = c
    if ( data->deref() )
        delete data;
    data = c.data;
    return *this;
}


/*!
  Returns the cursor shape identifer. The return value is one of
  following values (cast to an int)

  <ul>
  <li> \c ArrowCursor - standard arrow cursor
  <li> \c UpArrowCursor - upwards arrow
  <li> \c CrossCursor - crosshair
  <li> \c WaitCursor - hourglass/watch
  <li> \c IbeamCursor - ibeam/text entry
  <li> \c SizeVerCursor - vertical resize
  <li> \c SizeHorCursor - horizontal resize
  <li> \c SizeBDiagCursor - diagonal resize (/)
  <li> \c SizeFDiagCursor - diagonal resize (\)
  <li> \c SizeAllCursor - all directions resize
  <li> \c BlankCursor - blank/invisible cursor
  <li> \c SplitVCursor - vertical splitting
  <li> \c SplitHCursor - horziontal splitting
  <li> \c PointingHandCursor - a pointing hand
  <li> \c ForbiddenCursor - a slashed circle
  <li> \c BitmapCursor - userdefined bitmap cursor
  </ul>

  These correspond to the <a href="#cursors">predefined</a>
  global QCursor objects.

  \sa setShape()
*/

int QCursor::shape() const
{
#ifdef QCURSOR_DEBUG
    qDebug( "QCursor::shape()" );
#endif

    if ( !initialized )
        initialize();
    return data->cshape;
}

/*!
  Sets the cursor to the shape identified by \a shape.

  <ul>
  <li> \c ArrowCursor - standard arrow cursor
  <li> \c UpArrowCursor - upwards arrow
  <li> CrossCursor - crosshair
  <li> \c WaitCursor - hourglass/watch
  <li> \c IbeamCursor - ibeam/text entry
  <li> \c SizeVerCursor - vertical resize
  <li> \c SizeHorCursor - horizontal resize
  <li> \c SizeBDiagCursor - diagonal resize (/)
  <li> \c SizeFDiagCursor - diagonal resize (\)
  <li> \c SizeAllCursor - all directions resize
  <li> \c BlankCursor - blank/invisible cursor
  <li> \c SplitVCursor - vertical splitting
  <li> \c SplitHCursor - horziontal splitting
  <li> \c PointingHandCursor - a pointing hand
  <li> \c ForbiddenCursor - a slashed circle
  <li> \c BitmapCursor - userdefined bitmap cursor
  </ul>

  These correspond to the <a href="#cursors">predefined</a>
  global QCursor objects.

  \sa shape()
*/

void QCursor::setShape( int shape )
{
#ifdef QCURSOR_DEBUG
    qDebug( "QCursor::setShape( shape = %d, this = %p, data = %p)", shape, this, data );
#endif

    if ( !initialized )
        initialize();
    QCursor *c = find_cur( shape );  // find one of the global ones
    if ( !c )           // not found
        c = &cursorTable[ arrowCursorIdx ]; //   then use arrowCursor
    c->data->ref();
    if ( data->deref() )         // make shallow copy
        delete data;
    data = c->data;
}


/*!
  Returns the cursor bitmap, or 0 if it is one of the standard cursors.
*/
const QBitmap *QCursor::bitmap() const
{
#ifdef QCURSOR_DEBUG
    qDebug( "QCursor::bitmap( this = %p, data = %p)", this, data );
#endif

    if ( !initialized )
        initialize();
    return data->bm;
}

/*!
  Returns the cursor bitmap mask, or 0 if it is one of the standard cursors.
*/

const QBitmap *QCursor::mask() const
{
#ifdef QCURSOR_DEBUG
    qDebug( "QCursor::mask( this = %p, data = %p)", this, data );
#endif

    if ( !initialized )
        initialize();
    return data->bmm;
}

/*!
  Returns the cursor hot spot, or (0,0) if it is one of the standard cursors.
*/

QPoint QCursor::hotSpot() const
{
#ifdef QCURSOR_DEBUG
    qDebug( "QCursor::hotSpot( this = %p, data = %p)", this, data );
#endif

    if ( !initialized )
        initialize();
    return QPoint( data->hx, data->hy );
}


/*!
  Returns the window system cursor handle.

  \warning
  Portable in principle, but if you use it you are probably about to do
  something non-portable. Be careful.
*/

HCURSOR QCursor::handle() const
{
#ifdef QCURSOR_DEBUG
    qDebug( "QCursor::handle( this = %p, data = %p)", this, data );
#endif

    if ( !initialized )
        initialize();
    if ( !data )
        return 0;
    if ( !data->hcurs )
        update();
    return ( HCURSOR ) data->hcurs;
}


/*!
  Returns the position of the cursor (hot spot) in global screen
  coordinates.

  You can call QWidget::mapFromGlobal() to translate it to widget
  coordinates.

  \sa setPos(), QWidget::mapFromGlobal(), QWidget::mapToGlobal()
*/

QPoint QCursor::pos()
{
    POINT p;
    GetCursorPos( &p );
    return QPoint( p.x, p.y );
}

/*!
  Moves the cursor (hot spot) to the global screen position \a x and \a y.

  You can call QWidget::mapToGlobal() to translate widget coordinates
  to global screen coordinates.

  \sa pos(), QWidget::mapFromGlobal(), QWidget::mapToGlobal()
*/

void QCursor::setPos( int x, int y )
{
    // Need to check, since some X servers generate null mouse move
    // events, causing looping in applications which call setPos() on
    // every mouse move event.
    //
    if ( pos() == QPoint( x, y ) )
        return ;

    SetCursorPos( x, y );
}

/*!
  \overload void QCursor::setPos ( const QPoint & )
*/


/*!
  \internal Creates the cursor.
*/
HCURSOR qt_createPixmapCursor( HDC hdc, const QPixmap &pm, const QBitmap &bm, const QPoint &hs, const QSize &size )
{
    int w = size.width();
    int h = size.height();
    
    /* Create Bitmaps  */
    HDC hAndMask = CreateCompatibleDC( hdc );
    HDC hXorMask = CreateCompatibleDC( hdc );
    HBITMAP hbAndMask = CreateBitmap( w, h, NULL, 1, NULL );
    HBITMAP hbXorMask = CreateCompatibleBitmap( hdc, w, h );
    /* Select the bitmaps to DC  */
    HBITMAP hbOldAndMask = ( HBITMAP ) SelectObject( hAndMask, hbAndMask );
    HBITMAP hbOldXorMask = ( HBITMAP ) SelectObject( hXorMask, hbXorMask );
   
    /* some BitBlt'ing :) */
    BitBlt( hAndMask, 0, 0, w, h, NULL, 0, 0, WHITENESS );
    BitBlt( hAndMask, 0, 0, bm.width(), bm.height(), bm.handle(), 0, 0, SRCCOPY );
    
    /* There is somewhere a bug when pm is a QBitmap ... */
    BitBlt( hXorMask, 0, 0, w, h, hAndMask, 0, 0, NOTSRCCOPY );
    if ( pm.isQBitmap() ) {
        QPixmap xorMask( w, h );
        xorMask.fill ( Qt::color1 );
        QPainter paint2( &xorMask );

        QPixmap tmp = pm;
        tmp.setMask( bm );
        paint2.drawPixmap ( 0, 0, tmp );
        paint2.end();
        BitBlt( hXorMask, 0, 0, w, h, xorMask.handle(), 0, 0, SRCAND );
    } else
        BitBlt( hXorMask, 0, 0, pm.width(), pm.height(), pm.handle(), 0, 0, SRCAND );
    
    /* Restore */
    SelectObject( hAndMask, hbOldAndMask );
    SelectObject( hXorMask, hbOldXorMask );

    ICONINFO ii;
    ii.fIcon = false;
    ii.xHotspot = hs.x();
    ii.yHotspot = hs.y();
    ii.hbmMask = hbAndMask;
    ii.hbmColor = hbXorMask;

    HCURSOR hCur = CreateIconIndirect ( &ii );
    
    /* free */
    DeleteObject( hbAndMask );
    DeleteObject( hbXorMask );
    DeleteDC( hAndMask );
    DeleteDC( hXorMask );
    
    return hCur;
}


void QCursor::update() const
{
    if ( !initialized )
        initialize();
    register QCursorData *d = data;  // cheat const!
    if ( !d )
        return ;

    if ( d->hcurs )         // already loaded
        return ;

    /* qdnd_win.cpp has a much better implementation of bitmap cursor creation, but currently
       I've no time to merge these two version into one */
    if ( d->cshape == BitmapCursor ) {
        int w = GetSystemMetrics( SM_CXCURSOR );
        int h = GetSystemMetrics( SM_CYCURSOR );

        d->hcurs = qt_createPixmapCursor( qt_display_dc(), *d->bm, *d->bmm, QPoint( data->hx, data->hy ), QSize( w, h ) );
        return ;
    }

    void *sh;         /* Unicode: LPCTSTR, Ansi: LPCSTR */

    switch ( d->cshape ) {
    case ArrowCursor:
        sh = IDC_ARROW;
        break;
    case UpArrowCursor:
        sh = IDC_UPARROW;
        break;
    case CrossCursor:
        sh = IDC_CROSS;
        break;
    case WaitCursor:
        sh = IDC_WAIT;
        break;
    case IbeamCursor:
        sh = IDC_IBEAM;
        break;
    case SizeAllCursor:
        sh = IDC_SIZEALL;
        break;
    case SizeVerCursor:
    case SplitHCursor:
        sh = IDC_SIZENS;
        break;
    case SplitVCursor:
    case SizeHorCursor:
        sh = IDC_SIZEWE;
        break;
    case SizeBDiagCursor:
        sh = IDC_SIZENESW;
        break;
    case SizeFDiagCursor:
        sh = IDC_SIZENWSE;
        break;
    case BlankCursor:
        sh = 0;
        break;
    case PointingHandCursor:
        sh = IDC_HAND;
        break;
    case ForbiddenCursor:
        sh = IDC_NO;
        break;
    case WhatsThisCursor:
        sh = IDC_HELP;
        break;
    case BusyCursor:
        sh = IDC_APPSTARTING;
        break;
    default:
        sh = IDC_ARROW;
        qWarning( "QCursor::update: Invalid cursor shape %d", d->cshape );
        return ;
    }
    d->hcurs = QT_WA_INLINE( LoadCursorW( 0, ( LPCTSTR ) sh ),
                             LoadCursorA( 0, ( LPCSTR ) sh ) );
}

