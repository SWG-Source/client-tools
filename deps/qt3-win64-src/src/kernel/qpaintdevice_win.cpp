/****************************************************************************
** $Id: qpaintdevice_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QPaintDevice class for X11
**
** Created : 20030119
**
** Copyright (C) 2003 Holger Schroeder
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2003 Ivan de Jesus Deras Tabora
** Copyright (C) 2004 Tom and Timi Cecka
** Copyright (C) 2005 Christian Ehrlicher
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

#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qt_windows.h"

//#define DEBUG_QPAINTDEVICE

// NOT REVISED
/*!
  \class QPaintDevice qpaintdevice.h
  \brief The base class of objects that can be painted.

  \ingroup drawing

  A paint device is an abstraction of a two-dimensional space that can be
  drawn using a QPainter.
  The drawing capabilities are implemented by the subclasses: QWidget,
  QPixmap, QPicture and QPrinter.

  The default coordinate system of a paint device has its origin
  located at the top left position. X increases to the right and Y
  increases downwards. The unit is one pixel.  There are several ways
  to set up a user-defined coordinate system using the painter, for
  example by QPainter::setWorldMatrix().

  Example (draw on a paint device):
  \code
    void MyWidget::paintEvent( QPaintEvent * )
    {
 QPainter p;    // our painter
 p.begin( this );   // start painting widget
 p.setPen( red );   // blue outline
 p.setBrush( yellow );   // yellow fill
 p.drawEllipse( 10,20, 100,100 ); // 100x100 ellipse at 10,20
 p.end();    // painting done
    }
  \endcode

  The bit block transfer is an extremely useful operation for copying pixels
  from one paint device to another (or to itself).
  It is implemented as the global function bitBlt().

  Example (scroll widget contents 10 pixels to the right):
  \code
    bitBlt( myWidget, 10,0, myWidget );
  \endcode

  \warning Qt requires that a QApplication object must exist before any paint
  devices can be created.  Paint devices access window system resources, and
  these resources are not initialized before an application object is created.
*/


//
// Some global variables - these are initialized by QColor::initialize()
//

/*Display *QPaintDevice::x_appdisplay = 0;
int  QPaintDevice::x_appscreen;
int  QPaintDevice::x_appdepth;
int  QPaintDevice::x_appcells;
HANDLE  QPaintDevice::x_appcolormap;
bool  QPaintDevice::x_appdefcolormap;
void *QPaintDevice::x_appvisual;
bool  QPaintDevice::x_appdefvisual;

*/
/*!
  Constructs a paint device with internal flags \e devflags.
  This constructor can only be invoked from subclasses of QPaintDevice.
*/

QPaintDevice::QPaintDevice( uint devflags )
{
#ifdef DEBUG_QPAINTDEVICE
    qDebug( "QPaintDevice::QPaintDevice( %d )", devflags );
#endif

    if ( !qApp ) {    // global constructor
#if defined(CHECK_STATE)
        qFatal( "QPaintDevice: Must construct a QApplication before a "
                "QPaintDevice" );
#endif

        return ;
    }
    devFlags = devflags;
    painters = 0;
    hdc = 0;
}

/*!
  Destructs the paint device and frees window system resources.
*/

QPaintDevice::~QPaintDevice()
{
#ifdef DEBUG_QPAINTDEVICE
    qDebug( "QPaintDevice::~QPaintDevice()" );
#endif
#if defined(QT_CHECK_STATE)
    if ( paintingActive() )
	qWarning( "QPaintDevice: Cannot destroy paint device that is being "
		  "painted" );
#endif
}

/*!
  \fn int QPaintDevice::devType() const

  Returns the device type identifier: \c QInternal::Widget, \c
  QInternal::Pixmap, \c QInternal::Printer, \c QInternal::Picture or
  \c QInternal::UndefinedDevice.
*/

/*!
  \fn bool QPaintDevice::isExtDev() const
  Returns TRUE if the device is a so-called external paint device.

  External paint devices cannot be bitBlt()'ed from.
  QPicture and QPrinter are external paint devices.
*/

/*!
    Returns the window system handle of the paint device, for
    low-level access. Using this function is not portable.

    The HANDLE type varies with platform; see \c qpaintdevice.h and
    \c qwindowdefs.h for details.

    \sa x11Display()
*/
HDC QPaintDevice::handle() const
{
#ifdef DEBUG_QPAINTDEVICE
    qDebug( "QPaintDevice::handle(), DevType=%s", devType() == QInternal::Widget ? "QInternal::Widget" : "Other" );
#endif
    return hdc;
}


/*!
  \fn HDC QPaintDevice::handle() const

  Returns the window system handle of the paint device, for low-level
  access.  <em>Using this function is not portable.</em>

  The HDC type varies with platform; see qpaintdevice.h and qwindowdefs.h
  for details.
*/

/*!
  \fn Display *QPaintDevice::x11AppDisplay()

  Returns a pointer to the X display
  global to the application (X11 only).
  <em>Using this function is not portable.</em>

  \sa handle()
*/

/*!
  \fn int QPaintDevice::x11AppScreen ()

  Returns the screen number on the X display
  global to the application (X11 only).
  <em>Using this function is not portable.</em>
*/

/*!
  \fn int QPaintDevice::x11AppDepth ()

  Returns the depth of the X display
  global to the application (X11 only).
  <em>Using this function is not portable.</em>

  \sa QPixmap::defaultDepth()
*/

/*!
  \fn int QPaintDevice::x11AppCells ()

  Returns the number of entries in the colormap of the X display
  global to the application (X11 only).
  <em>Using this function is not portable.</em>

  \sa x11Colormap()
*/

/*!
  \fn HANDLE QPaintDevice::x11AppColormap ()

  Returns the colormap of the X display
  global to the application (X11 only).
  <em>Using this function is not portable.</em>

  \sa x11Cells()
*/

/*!
  \fn bool QPaintDevice::x11AppDefaultColormap ()

  Returns the default colormap of the X display
  global to the application (X11 only).
  <em>Using this function is not portable.</em>

  \sa x11Cells()
*/

/*!
  \fn void* QPaintDevice::x11AppVisual ()

  Returns the Visual of the X display
  global to the application (X11 only).
  <em>Using this function is not portable.</em>
*/

/*!
  \fn bool QPaintDevice::x11AppDefaultVisual ()

  Returns the default Visual of the X display
  global to the application (X11 only).
  <em>Using this function is not portable.</em>
*/


/*!
  \fn Display *QPaintDevice::x11Display() const

  Returns a pointer to the X display
  for the paint device (X11 only).
  <em>Using this function is not portable.</em>

  \sa handle()
*/

/*!
  \fn int QPaintDevice::x11Screen () const

  Returns the screen number on the X display
  for the paint device (X11 only).
  <em>Using this function is not portable.</em>
*/

/*!
  \fn int QPaintDevice::x11Depth () const

  Returns the depth of the X display
  for the paint device (X11 only).
  <em>Using this function is not portable.</em>

  \sa QPixmap::defaultDepth()
*/

/*!
  \fn int QPaintDevice::x11Cells () const

  Returns the number of entries in the colormap of the X display
  for the paint device (X11 only).
  <em>Using this function is not portable.</em>

  \sa x11Colormap()
*/

/*!
  \fn HANDLE QPaintDevice::x11Colormap () const

  Returns the colormap of the X display
  for the paint device (X11 only).
  <em>Using this function is not portable.</em>

  \sa x11Cells()
*/

/*!
  \fn bool QPaintDevice::x11DefaultColormap () const

  Returns the default colormap of the X display
  for the paint device (X11 only).
  <em>Using this function is not portable.</em>

  \sa x11Cells()
*/

/*!
  \fn void* QPaintDevice::x11Visual () const

  Returns the Visual of the X display
  for the paint device (X11 only).
  <em>Using this function is not portable.</em>
*/

/*!
  \fn bool QPaintDevice::x11DefaultVisual () const

  Returns the default Visual of the X display
  for the paint device (X11 only).
  <em>Using this function is not portable.</em>
*/

//static int dpiX=0,dpiY=0;
/*extern void     qX11ClearFontNameCache(); // defined in qfont_x11.cpp
*/
/*!
  Sets the value returned by x11AppDpiX().  The default is determined
  by the display configuration.  Changing this value will alter the
  scaling of fonts and many other metrics and is not recommended.

  \sa x11SetAppDpiY()
*/
/*void QPaintDevice::x11SetAppDpiX(int dpi)
{
    dpiX = dpi;
    qX11ClearFontNameCache();
}
*/
/*!
  Sets the value returned by x11AppDpiY().  The default is determined
  by the display configuration.  Changing this value will alter the
  scaling of fonts and many other metrics and is not recommended.

  \sa x11SetAppDpiX()
*/
/*void QPaintDevice::x11SetAppDpiY(int dpi)
{
    dpiY = dpi;
    qX11ClearFontNameCache();
}
*/
/*!
  Returns the horizontal DPI of the X display (X11 only).
  <em>Using this function is not portable.</em> See QPaintDeviceMetrics
  for portable access to related information.

  \sa x11AppDpiY(), x11SetAppDpiX(), QPaintDeviceMetrics::logicalDpiX()
*/
/*int QPaintDevice::x11AppDpiX()
{
    if ( !dpiX ) {
 Display *dpy = x11AppDisplay();
 int scr = x11AppScreen();
 if ( dpy ) {
     dpiX =
  (DisplayWidth(dpy,scr) * 254 + DisplayWidthMM(dpy,scr)*5)
         / (DisplayWidthMM(dpy,scr)*10);
 }
    }
    return dpiX;
}
*/
/*!
  Returns the vertical DPI of the X11 display (X11 only).
  <em>Using this function is not portable.</em> See QPaintDeviceMetrics
  for portable access to related information.

  \sa x11AppDpiX(), x11SetAppDpiY(), QPaintDeviceMetrics::logicalDpiY()
*/
/*int QPaintDevice::x11AppDpiY()
{
    if ( !dpiY ) {
 Display *dpy = x11AppDisplay();
 int scr = x11AppScreen();
 if ( dpy )
     dpiY =
  (DisplayHeight(dpy,scr) * 254 + DisplayHeightMM(dpy,scr)*5)
         / (DisplayHeightMM(dpy,scr)*10);
    }
    return dpiY;
}
*/

/*!
  \fn bool QPaintDevice::paintingActive() const
  Returns TRUE if the device is being painted, i.e. someone has called
  QPainter::begin() and not yet QPainter::end() for this device.
  \sa QPainter::isActive()
*/

/*!
  Internal virtual function that interprets drawing commands from
  the painter.

  Implemented by subclasses that have no direct support for drawing
  graphics (external paint devices, for example QPicture).
*/

bool QPaintDevice::cmd( int, QPainter *, QPDevCmdParam * )
{
#ifdef DEBUG_QPAINTDEVICE
    qDebug( "QPaintDevice::cmd()" );
#endif
#if defined(CHECK_STATE)

    qWarning( "QPaintDevice::cmd: Device has no command interface" );
#endif

    return FALSE;
}

/*!
  Internal virtual function that returns paint device metrics.

  Please use the QPaintDeviceMetrics class instead.
*/

int QPaintDevice::metric( int ) const
{
#ifdef DEBUG_QPAINTDEVICE
    qDebug( "QPaintDevice::metric()" );
#endif
#if defined(CHECK_STATE)

    qWarning( "QPaintDevice::metrics: Device has no metric information" );
#endif

    return 0;
}

/*!
  Internal virtual function. Reserved for future use.

  \internal
  Please use the QFontMetrics class instead.
*/

int QPaintDevice::fontMet( QFont *, int, const char *, int ) const
{
#ifdef DEBUG_QPAINTDEVICE
    qDebug( "QPaintDevice::fontMet()" );
#endif

    return 0;
}

/*!
  Internal virtual function. Reserved for future use.

  \internal
  Please use the QFontInfo class instead.
*/

int QPaintDevice::fontInf( QFont *, int ) const
{
#ifdef DEBUG_QPAINTDEVICE
    qDebug( "QPaintDevice::fontInf()" );
#endif

    return 0;
}

/*!
  \relates QPaintDevice
  This function copies a block of pixels from one paint device to another
  (bitBlt means bit block transfer).

  \arg \e dst is the paint device to copy to.
  \arg \e dx and \e dy is the position to copy to.
  \arg \e src is the paint device to copy from.
  \arg \e sx and \e sy is the position to copy from.
  \arg \e sw and \e sh is the width and height of the block to be copied.
  \arg \e rop defines the raster operation to be used when copying.

  If \e sw is 0 or \e sh is 0, then bitBlt will do nothing.

  If \e sw is negative, then bitBlt calculates <code>sw = src->width -
  sx.</code> If \e sh is negative, then bitBlt calculates <code>sh =
  src->height - sy.</code>

  The \e rop argument can be one of:
  <ul>
  <li> \c CopyROP:     dst = src.
  <li> \c OrROP:       dst = src OR dst.
  <li> \c XorROP:      dst = src XOR dst.
  <li> \c NotAndROP:   dst = (NOT src) AND dst
  <li> \c NotCopyROP:  dst = NOT src
  <li> \c NotOrROP:    dst = (NOT src) OR dst
  <li> \c NotXorROP:   dst = (NOT src) XOR dst
  <li> \c AndROP       dst = src AND dst
  <li> \c NotROP:      dst = NOT dst
  <li> \c ClearROP:    dst = 0
  <li> \c SetROP:      dst = 1
  <li> \c NopROP:      dst = dst
  <li> \c AndNotROP:   dst = src AND (NOT dst)
  <li> \c OrNotROP:    dst = src OR (NOT dst)
  <li> \c NandROP:     dst = NOT (src AND dst)
  <li> \c NorROP:      dst = NOT (src OR dst)
  </ul>

  The \e ignoreMask argument (default FALSE) applies where \e src is
  a QPixmap with a \link QPixmap::setMask() mask\endlink.
  If \e ignoreMask is TRUE, bitBlt ignores the pixmap's mask.

  BitBlt has two restrictions:
  <ol>
  <li> The \e src device must be QWidget or QPixmap.  You cannot copy pixels
  from a picture or a printer (external device).
  <li> The \e src device may not have pixel depth greater than \e dst.
  You cannot copy from an 8 bit pixmap to a 1 bit pixmap.
  </ol>
*/

void bitBlt( QPaintDevice *dst, int dx, int dy,
             const QPaintDevice *src, int sx, int sy, int sw, int sh,
             Qt::RasterOp rop, bool ignoreMask )
{
#ifdef DEBUG_QPAINTDEVICE
    qDebug( "bitBlt( dst: 0x%08p, dx: %d, dy: %d, src: 0x%08p, sx: %d sy: %d, sw: %d, sh: %d, rop: %d, ignoreMask: %d",
            dst, dx, dy, src, sx, sy, sw, sh, rop, ignoreMask );
#endif

    if ( !src || !dst ) {
#if defined(CHECK_NULL)
        ASSERT( src != 0 );
        ASSERT( dst != 0 );
#endif

        return ;
    }
    HDC dsthandle = dst->handle();
    HDC srchandle = src->handle();

    if ( !srchandle || src->isExtDev() ) {
#ifdef DEBUG_QPAINTDEVICE
        qDebug( "stop 1" );
#endif
        return ;
    }

    int ts = src->devType();   // from device type
    int td = dst->devType();   // to device type
    //    Display *dpy = src->x11Display();

    if ( sw <= 0 ) {    // special width
        if ( sw < 0 )
            sw = src->metric( QPaintDeviceMetrics::PdmWidth ) - sx;
        else {
#ifdef DEBUG_QPAINTDEVICE
            qDebug( "stop 2" );
#endif
            return ;
        }
    }
    if ( sh <= 0 ) {    // special height
        if ( sh < 0 )
            sh = src->metric( QPaintDeviceMetrics::PdmHeight ) - sy;
        else {
#ifdef DEBUG_QPAINTDEVICE
            qDebug( "stop 3" );
#endif
            return ;
        }
    }
#ifdef DEBUG_QPAINTDEVICE
    qDebug( "qpaintdevice_win.cpp: bitBlt after updating: sw: %d, sh: %d", sw, sh );
#endif

    if ( dst->paintingActive() && dst->isExtDev() ) {
        QPixmap * pm;    // output to picture/printer
        bool tmp_pm = TRUE;
        if ( ts == QInternal::Pixmap ) {
            pm = ( QPixmap* ) src;
            if ( sx != 0 || sy != 0 ||
                    sw != pm->width() || sh != pm->height() || ignoreMask ) {
                QPixmap * tmp = new QPixmap( sw, sh, pm->depth() );
                bitBlt( tmp, 0, 0, pm, sx, sy, sw, sh, Qt::CopyROP, TRUE );
                if ( pm->mask() && !ignoreMask ) {
                    QBitmap mask( sw, sh );
                    bitBlt( &mask, 0, 0, pm->mask(), sx, sy, sw, sh,
                            Qt::CopyROP, TRUE );
                    tmp->setMask( mask );
                }
                pm = tmp;
            } else {
                tmp_pm = FALSE;
            }
        } else if ( ts == QInternal::Widget ) { // bitBlt to temp pixmap
            pm = new QPixmap( sw, sh );
            CHECK_PTR( pm );
            bitBlt( pm, 0, 0, src, sx, sy, sw, sh );
        } else {
#if defined(CHECK_RANGE)
            qWarning( "bitBlt: Cannot bitBlt from device" );
#endif

#ifdef DEBUG_QPAINTDEVICE
            qDebug( "stop 4" );
#endif
            return ;
        }
        QPDevCmdParam param[ 3 ];
        QPoint p( dx, dy );
        param[ 0 ].point = &p;
        param[ 1 ].pixmap = pm;
        dst->cmd( QPaintDevice::PdcDrawPixmap, 0, param );
        if ( tmp_pm )
            delete pm;
#ifdef DEBUG_QPAINTDEVICE
        qDebug( "stop 5" );
#endif
        return ;
    }

    switch ( ts ) {
    case QInternal::Widget:
    case QInternal::Pixmap:
    case QInternal::System:      // OK, can blt from these
        break;
    default:
#if defined(CHECK_RANGE)

        qWarning( "bitBlt: Cannot bitBlt from device type %x", ts );
#endif

#ifdef DEBUG_QPAINTDEVICE
        qDebug( "stop 6" );
#endif
        return ;
    }
    switch ( td ) {
    case QInternal::Widget:
    case QInternal::Pixmap:
    case QInternal::System:      // OK, can blt to these
        break;
    default:
#if defined(CHECK_RANGE)

        qWarning( "bitBlt: Cannot bitBlt to device type %x", td );
#endif

#ifdef DEBUG_QPAINTDEVICE
        qDebug( "stop 7" );
#endif
        return ;
    }
    // found from msdn:
    // msdn home > msdn library > windows gdi >
    // painting and drawing > painting and drawing reference >
    // raster operation codes... (tell me if you find that page without
    // this reference ;-)

    static unsigned int ropCodes[] = {   // ROP translation table
                                         SRCCOPY,
                                         SRCPAINT,
                                         SRCINVERT,
                                         0x00220326,
                                         NOTSRCCOPY,
                                         MERGEPAINT,
                                         0x00990066,
                                         SRCAND,
                                         DSTINVERT,
                                         BLACKNESS,
                                         WHITENESS,
                                         0x00AA0029,
                                         SRCERASE,
                                         0x00DD0228,
                                         0x007700E6,
                                         NOTSRCERASE
                                     };
    if ( rop > Qt::LastROP ) {
#if defined(CHECK_RANGE)
        qWarning( "bitBlt: Invalid ROP code" );
#endif

#ifdef DEBUG_QPAINTDEVICE
        qDebug( "stop 8" );
#endif
        return ;
    }

    if ( dsthandle == 0 ) {
#if defined(CHECK_NULL)
        qWarning( "bitBlt: Cannot bitBlt to device" );
#endif

#ifdef DEBUG_QPAINTDEVICE
        qDebug( "stop 9" );
#endif
        return ;
    }

    bool mono_src;
    bool mono_dst;
    bool include_inferiors = FALSE;
    bool graphics_exposure = FALSE;
    QPixmap *src_pm;
    QBitmap *mask;

    if ( ts == QInternal::Pixmap ) {
        src_pm = ( QPixmap* ) src;
        mono_src = src_pm->depth() == 1;
        mask = ignoreMask ? 0 : src_pm->data->mask;
    } else {
        src_pm = 0;
        mono_src = FALSE;
        mask = 0;
        include_inferiors = ( ( QWidget* ) src ) ->testWFlags( Qt::WPaintUnclipped );
        graphics_exposure = td == QInternal::Widget;
    }
    if ( td == QInternal::Pixmap ) {
        mono_dst = ( ( QPixmap* ) dst ) ->depth() == 1;
        ( ( QPixmap* ) dst ) ->detach();  // changes shared pixmap
    } else {
        mono_dst = FALSE;
        include_inferiors = include_inferiors ||
                            ( ( QWidget* ) dst ) ->testWFlags( Qt::WPaintUnclipped );
    }

    if ( mono_dst && !mono_src ) { // dest is 1-bit pixmap, source is not
#if defined(CHECK_RANGE)
        qWarning( "bitBlt: Incompatible destination pixmap" );
#endif

#ifdef DEBUG_QPAINTDEVICE
        qDebug( "stop 10" );
#endif
        return ;
    }

    // do simple windows bitblt...
    //void bitBlt( QPaintDevice *dst, int dx, int dy,
    //      const QPaintDevice *src, int sx, int sy, int sw, int sh,
    //      Qt::RasterOp rop, bool ignoreMask )

    // BitBlt(dest, xd, yd, w, h, src, xs, ys, rop);
#ifdef DEBUG_QPAINTDEVICE
    qDebug( "  bitBlt: sw: %d, sh: %d, dx: %d, dy: %d, sx: %d, sy: %d,",
            sw, sh, dx, dy, sx, sy );


    qDebug( "  bitBlt: dst->handle: %p, src->handle: %p",
            dsthandle, srchandle );
    qDebug( " BitBlt(dst->handle(), dx, dy, sw, sh, src->handle(), sx, sy, ropCodes[rop]=%d);", ropCodes[ rop ] );
#endif
    // BitBlt(dst->handle(), dx + 80, dy, sw, sh, src->handle(), sx, sy, SRCCOPY );

    /*    GC gc;*/

    if ( mask && !mono_src ) {   // fast masked blt
#ifdef DEBUG_QPAINTDEVICE
        qDebug( "  bitBlt drawing masked Pixmap" );
#endif

        HDC hTmpDC = CreateCompatibleDC( dsthandle );
        HBITMAP hTmpBmp = CreateCompatibleBitmap( dsthandle, sw, sh );

        HBITMAP hOldBmp1 = ( HBITMAP ) SelectObject( hTmpDC, hTmpBmp );
        BitBlt( hTmpDC, 0, 0, sw, sh, srchandle, sx, sy, ropCodes[ rop ] );

        BitBlt( dsthandle, dx, dy, sw, sh, hTmpDC, 0, 0, SRCINVERT );
        BitBlt( dsthandle, dx, dy, sw, sh, mask->handle(), sx, sy, SRCAND );
        BitBlt( dsthandle, dx, dy, sw, sh, hTmpDC, 0, 0, SRCINVERT );

        SelectObject( hTmpDC, hOldBmp1 );
        DeleteObject( hTmpBmp );
        DeleteDC( hTmpDC );

        /*if (!BitBlt(dsthandle, dx, dy, sw, sh, srchandle, sx, sy, ropCodes[rop] )) {
        #ifdef DEBUG_QPAINTDEVICE
         qDebug("  bitBlt failed!!!!!!!!!!!, %d", GetLastError());
        #endif
        }*/

        /*bool temp_gc = FALSE;
        if ( mask->data->maskgc ) {
            gc = (GC)mask->data->maskgc; // we have a premade mask GC
        } else {
            if ( src_pm->optimization() == QPixmap::NormalOptim ) {
         // Compete for the global cache
         gc = cache_mask_gc( dpy, dst->handle(),
               mask->data->ser_no,
               mask->handle() );
            } else {
         // Create a new mask GC. If BestOptim, we store the mask GC
         // with the mask (not at the pixmap). This way, many pixmaps
         // which have a common mask will be optimized at no extra cost.
         gc = XCreateGC( dpy, dst->handle(), 0, 0 );
         XSetGraphicsExposures( dpy, gc, FALSE );
         XSetClipMask( dpy, gc, mask->handle() );
         if ( src_pm->optimization() == QPixmap::BestOptim ) {
             mask->data->maskgc = gc;
         } else {
             temp_gc = TRUE;
         }
            }
        }
        XSetClipOrigin( dpy, gc, dx-sx, dy-sy );
        if ( rop != Qt::CopyROP )  // use non-default ROP code
            XSetFunction( dpy, gc, ropCodes[rop] );
        if ( include_inferiors ) {
            XSetSubwindowMode( dpy, gc, IncludeInferiors );
            XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
                dx, dy );
            XSetSubwindowMode( dpy, gc, ClipByChildren );
        } else {
            XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
                dx, dy );
        }

        if ( temp_gc )    // delete temporary GC
            XFreeGC( dpy, gc );
        else if ( rop != Qt::CopyROP )  // restore ROP
            XSetFunction( dpy, gc, GXcopy );*/
        return ;
    }
    if ( !BitBlt( dsthandle, dx, dy, sw, sh, srchandle, sx, sy, ropCodes[ rop ] ) ) {
#ifdef DEBUG_QPAINTDEVICE
        qDebug( "  bitBlt failed!!!!!!!!!!!, %d", GetLastError() );
#endif

    }
#ifdef DEBUG_QPAINTDEVICE

    qDebug( "no stop" );
#endif

    return ;

    /*gc = qt_xget_temp_gc( mono_dst );  // get a reusable GC

    if ( rop != Qt::CopyROP )   // use non-default ROP code
    XSetFunction( dpy, gc, ropCodes[rop] );

    if ( mono_src ) {    // src is bitmap
    XGCValues gcvals;
    ulong   valmask = GCBackground | GCForeground | GCFillStyle |
       GCStipple | GCTileStipXOrigin | GCTileStipYOrigin;
    if ( td == QInternal::Widget ) { // set GC colors
     QWidget *w = (QWidget *)dst;
     gcvals.background = w->backgroundColor().pixel();
     gcvals.foreground = w->foregroundColor().pixel();
     if ( include_inferiors ) {
    valmask |= GCSubwindowMode;
    gcvals.subwindow_mode = IncludeInferiors;
     }
    } else if ( mono_dst ) {
     gcvals.background = 0;
     gcvals.foreground = 1;
    } else {
     gcvals.background = Qt::white.pixel();
     gcvals.foreground = Qt::black.pixel();
    }

    gcvals.fill_style  = FillOpaqueStippled;
    gcvals.stipple    = src->handle();
    gcvals.ts_x_origin = dx - sx;
    gcvals.ts_y_origin = dy - sy;

    bool clipmask = FALSE;
    if ( mask ) {
     if ( ((QPixmap*)src)->data->selfmask ) {
    gcvals.fill_style = FillStippled;
     } else {
    XSetClipMask( dpy, gc, mask->handle() );
    XSetClipOrigin( dpy, gc, dx-sx, dy-sy );
    clipmask = TRUE;
     }
    }

    XChangeGC( dpy, gc, valmask, &gcvals );
    XFillRectangle( dpy,dst->handle(), gc, dx, dy, sw, sh );

    valmask = GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin;
    gcvals.fill_style  = FillSolid;
    gcvals.ts_x_origin = 0;
    gcvals.ts_y_origin = 0;
    if ( include_inferiors ) {
     valmask |= GCSubwindowMode;
     gcvals.subwindow_mode = ClipByChildren;
    }
    XChangeGC( dpy, gc, valmask, &gcvals );

    if ( clipmask ) {
     XSetClipOrigin( dpy, gc, 0, 0 );
     XSetClipMask( dpy, gc, None );
    }

    } else {     // src is pixmap/widget

    if ( graphics_exposure )  // widget to widget
     XSetGraphicsExposures( dpy, gc, TRUE );
    if ( include_inferiors ) {
     XSetSubwindowMode( dpy, gc, IncludeInferiors );
     XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
         dx, dy );
     XSetSubwindowMode( dpy, gc, ClipByChildren );
    } else {
     XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
         dx, dy );
    }
    if ( graphics_exposure )  // reset graphics exposure
     XSetGraphicsExposures( dpy, gc, FALSE );
    }

    if ( rop != Qt::CopyROP )   // restore ROP
    XSetFunction( dpy, gc, GXcopy );*/
}




/*!
  \fn void bitBlt( QPaintDevice *dst, const QPoint &dp, const QPaintDevice *src, const QRect &sr, RasterOp rop )

  Overloaded bitBlt() with the destination point \e dp and source rectangle
  \e sr.

  \relates QPaintDevice
*/

/*!
  \internal
*/
// makes it possible to add a setResolution as we have in QPrinter for all
// paintdevices without breaking bin compatibility.
void QPaintDevice::setResolution( int )
{}

/*!\internal
*/
int QPaintDevice::resolution() const
{
    return metric( QPaintDeviceMetrics::PdmDpiY );
}
