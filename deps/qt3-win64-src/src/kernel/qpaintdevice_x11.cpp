/****************************************************************************
** $Id: qpaintdevice_x11.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QPaintDevice class for X11
**
** Created : 940721
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

#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qt_x11_p.h"


/*!
    \class QPaintDevice qpaintdevice.h
    \brief The QPaintDevice class is the base class of objects that
    can be painted.

    \ingroup graphics
    \ingroup images

    A paint device is an abstraction of a two-dimensional space that
    can be drawn using a QPainter. The drawing capabilities are
    implemented by the subclasses QWidget, QPixmap, QPicture and
    QPrinter.

    The default coordinate system of a paint device has its origin
    located at the top-left position. X increases to the right and Y
    increases downward. The unit is one pixel. There are several ways
    to set up a user-defined coordinate system using the painter, for
    example, using QPainter::setWorldMatrix().

    Example (draw on a paint device):
    \code
    void MyWidget::paintEvent( QPaintEvent * )
    {
	QPainter p;                       // our painter
	p.begin( this );                  // start painting the widget
	p.setPen( red );                  // red outline
	p.setBrush( yellow );             // yellow fill
	p.drawEllipse( 10, 20, 100,100 ); // 100x100 ellipse at position (10, 20)
	p.end();                          // painting done
    }
    \endcode

    The bit block transfer is an extremely useful operation for
    copying pixels from one paint device to another (or to itself). It
    is implemented as the global function bitBlt().

    Example (scroll widget contents 10 pixels to the right):
    \code
    bitBlt( myWidget, 10, 0, myWidget );
    \endcode

    \warning Qt requires that a QApplication object exists before
    any paint devices can be created. Paint devices access window
    system resources, and these resources are not initialized before
    an application object is created.
*/


//
// Some global variables - these are initialized by QColor::initialize()
//

Display *QPaintDevice::x_appdisplay = 0;
int	 QPaintDevice::x_appscreen;

int	  QPaintDevice::x_appdepth;
int	  QPaintDevice::x_appcells;
Qt::HANDLE  QPaintDevice::x_approotwindow;
Qt::HANDLE  QPaintDevice::x_appcolormap;
bool	  QPaintDevice::x_appdefcolormap;
void	 *QPaintDevice::x_appvisual;
bool	  QPaintDevice::x_appdefvisual;

// ### in 4.0, remove the above, and use the below
int	 *QPaintDevice::x_appdepth_arr;
int	 *QPaintDevice::x_appcells_arr;
Qt::HANDLE *QPaintDevice::x_approotwindow_arr;
Qt::HANDLE *QPaintDevice::x_appcolormap_arr;
bool	 *QPaintDevice::x_appdefcolormap_arr;
void	**QPaintDevice::x_appvisual_arr;
bool	 *QPaintDevice::x_appdefvisual_arr;

/*!
    \enum QPaintDevice::PDevCmd
    \internal
*/

/*!
    Constructs a paint device with internal flags \a devflags. This
    constructor can be invoked only from QPaintDevice subclasses.
*/

QPaintDevice::QPaintDevice( uint devflags )
{
    if ( !qApp ) {				// global constructor
#if defined(QT_CHECK_STATE)
	qFatal( "QPaintDevice: Must construct a QApplication before a "
		"QPaintDevice" );
#endif
	return;
    }
    devFlags = devflags;
    painters = 0;
    hd = 0;
    rendhd = 0;
    x11Data = 0;
}

/*!
    Destroys the paint device and frees window system resources.
*/

QPaintDevice::~QPaintDevice()
{
#if defined(QT_CHECK_STATE)
    if ( paintingActive() )
	qWarning( "QPaintDevice: Cannot destroy paint device that is being "
		  "painted" );
#endif
    if ( x11Data && x11Data->deref() ) {
	delete x11Data;
	x11Data = 0;
    }
}


/*
  \internal
  Makes a shallow copy of the X11-specific data of \a fromDevice, if it is not
  null. Otherwise this function sets it to null.
*/

void QPaintDevice::copyX11Data( const QPaintDevice *fromDevice )
{
    setX11Data( fromDevice ? fromDevice->x11Data : 0 );
}

/*
  \internal
  Makes a deep copy of the X11-specific data of \a fromDevice, if it is not
  null. Otherwise this function sets it to null.
*/

void QPaintDevice::cloneX11Data( const QPaintDevice *fromDevice )
{
    if ( fromDevice && fromDevice->x11Data ) {
	QPaintDeviceX11Data *d = new QPaintDeviceX11Data;
	*d = *fromDevice->x11Data;
	d->count = 0;
	setX11Data( d );
    } else {
	setX11Data( 0 );
    }
}

/*
  \internal
  Makes a shallow copy of the X11-specific data \a d and assigns it to this
  class. This function increments the reference code of \a d.
*/

void QPaintDevice::setX11Data( const QPaintDeviceX11Data* d )
{
    if ( x11Data && x11Data->deref() )
	delete x11Data;
    x11Data = (QPaintDeviceX11Data*)d;
    if ( x11Data )
	x11Data->ref();
}


/*
  \internal
  If \a def is FALSE, returns a deep copy of the x11Data, or 0 if x11Data is 0.
  If \a def is TRUE, makes a QPaintDeviceX11Data struct filled with the default
  values.

  In either case the caller is responsible for deleting the returned
  struct. But notice that the struct is a shared class, so other
  classes might also have a reference to it. The reference count of
  the returned QPaintDeviceX11Data* is 0.
*/

QPaintDeviceX11Data* QPaintDevice::getX11Data( bool def ) const
{
    QPaintDeviceX11Data* res = 0;
    if ( def ) {
	res = new QPaintDeviceX11Data;
	res->x_display = x11AppDisplay();
	res->x_screen = x11AppScreen();
	res->x_depth = x11AppDepth();
	res->x_cells = x11AppCells();
	res->x_colormap = x11Colormap();
	res->x_defcolormap = x11AppDefaultColormap();
	res->x_visual = x11AppVisual();
	res->x_defvisual = x11AppDefaultVisual();
	res->deref();
    } else if ( x11Data ) {
	res = new QPaintDeviceX11Data;
	*res = *x11Data;
	res->count = 0;
    }
    return res;
}


/*!
    \fn int QPaintDevice::devType() const

    \internal

    Returns the device type identifier, which is \c QInternal::Widget
    if the device is a QWidget, \c QInternal::Pixmap if it's a
    QPixmap, \c QInternal::Printer if it's a QPrinter, \c
    QInternal::Picture if it's a QPicture or \c
    QInternal::UndefinedDevice in other cases (which should never
    happen).
*/

/*!
    \fn bool QPaintDevice::isExtDev() const

    Returns TRUE if the device is an external paint device; otherwise
    returns FALSE.

    External paint devices cannot be bitBlt()'ed from. QPicture and
    QPrinter are external paint devices.
*/

/*!
    Returns the window system handle of the paint device, for
    low-level access. Using this function is not portable.

    The HANDLE type varies with platform; see \c qpaintdevice.h and
    \c qwindowdefs.h for details.

    \sa x11Display()
*/
Qt::HANDLE QPaintDevice::handle() const
{
    return hd;
}

/*!
    Returns the window system handle of the paint device for XRender
    support. Use of this function is not portable. This function will
    return 0 if XRender support is not compiled into Qt, if the
    XRender extension is not supported on the X11 display, or if the
    handle could not be created.
*/
Qt::HANDLE QPaintDevice::x11RenderHandle() const
{
#ifndef QT_NO_XFTFREETYPE
    return rendhd ? XftDrawPicture( (XftDraw *) rendhd ) : 0;
#else
    return 0;
#endif // QT_NO_XFTFREETYPE
}


/*!
    \fn Display *QPaintDevice::x11AppDisplay()

    Returns a pointer to the X display global to the application (X11
    only). Using this function is not portable.

    \sa handle()
*/

/*!
    \fn int QPaintDevice::x11AppScreen()

    Returns the screen number on the X display global to the
    application (X11 only). Using this function is not portable.
*/

/*!
    \overload
    \fn int QPaintDevice::x11AppDepth()

    Returns the depth for the default screen of the X display global
    to the application (X11 only). Using this function is not
    portable.

    \sa QPixmap::defaultDepth()
*/

/*!
    \fn int QPaintDevice::x11AppCells()

    Returns the number of entries in the colormap for the default
    screen of the X display global to the application (X11
    only). Using this function is not portable.

    \sa x11Colormap()
*/

/*!
    \fn HANDLE QPaintDevice::x11AppRootWindow()

    Returns the root window for the default screen of the X display
    global to the applicatoin (X11 only). Using this function is not
    portable.
*/

/*!
    \fn HANDLE QPaintDevice::x11AppColormap()

    Returns the colormap for the default screen of the X display
    global to the application (X11 only). Using this function is not
    portable.

    \sa x11Cells()
*/

/*!
    \fn bool QPaintDevice::x11AppDefaultColormap ()

    Returns the default colormap for the default screen of the X
    display global to the application (X11 only). Using this function
    is not portable.

    \sa x11Cells()
*/

/*!
    \fn void* QPaintDevice::x11AppVisual ()

    Returns the Visual for the default screen of the X display global
    to the application (X11 only). Using this function is not
    portable.
*/

/*!
    \fn bool QPaintDevice::x11AppDefaultVisual ()

    Returns TRUE if the Visual used is the default for the default
    screen of the X display global to the application (X11 only);
    otherwise returns FALSE. Using this function is not portable.
*/

/*!
    \fn int QPaintDevice::x11AppDepth( int screen )

    Returns the depth for screen \a screen of the X display global to
    the application (X11 only). Using this function is not portable.

    \sa QPixmap::defaultDepth()
*/

/*!
    \overload
    \fn int QPaintDevice::x11AppCells( int screen )

    Returns the number of entries in the colormap for screen \a screen
    of the X display global to the application (X11 only). Using this
    function is not portable.

    \sa x11Colormap()
*/

/*!
    \overload
    \fn HANDLE QPaintDevice::x11AppRootWindow( int screen )

    Returns the root window for screen \a screen of the X display
    global to the applicatoin (X11 only). Using this function is not
    portable.
*/

/*!
    \overload
    \fn HANDLE QPaintDevice::x11AppColormap( int screen )

    Returns the colormap for screen \a screen of the X display global
    to the application (X11 only). Using this function is not
    portable.

    \sa x11Cells()
*/

/*!
    \overload
    \fn bool QPaintDevice::x11AppDefaultColormap( int screen )

    Returns the default colormap for screen \a screen of the X display
    global to the application (X11 only). Using this function is not
    portable.

    \sa x11Cells()
*/

/*!
    \overload
    \fn void* QPaintDevice::x11AppVisual( int screen )

    Returns the Visual for screen \a screen of the X display global to
    the application (X11 only). Using this function is not portable.
*/

/*!
    \overload
    \fn bool QPaintDevice::x11AppDefaultVisual( int screen )

    Returns TRUE if the Visual used is the default for screen
    \a screen of the X display global to the application (X11 only);
    otherwise returns FALSE. Using this function is not portable.
*/


/*!
    \fn Display *QPaintDevice::x11Display() const

    Returns a pointer to the X display for the paint device (X11
    only). Using this function is not portable.

    \sa handle()
*/

/*!
    \fn int QPaintDevice::x11Screen () const

    Returns the screen number on the X display for the paint device
    (X11 only). Using this function is not portable.
*/

/*!
    \fn int QPaintDevice::x11Depth () const

    Returns the depth of the X display for the paint device (X11
    only). Using this function is not portable.

    \sa QPixmap::defaultDepth()
*/

/*!
    \fn int QPaintDevice::x11Cells () const

    Returns the number of entries in the colormap of the X display for
    the paint device (X11 only). Using this function is not portable.

    \sa x11Colormap()
*/

/*!
    \fn HANDLE QPaintDevice::x11Colormap () const

    Returns the colormap of the X display for the paint device (X11
    only). Using this function is not portable.

    \sa x11Cells()
*/

/*!
    \fn bool QPaintDevice::x11DefaultColormap () const

    Returns the default colormap of the X display for the paint device
    (X11 only). Using this function is not portable.

    \sa x11Cells()
*/

/*!
    \fn void* QPaintDevice::x11Visual () const

    Returns the Visual of the X display for the paint device (X11
    only). Using this function is not portable.
*/

/*!
    \fn bool QPaintDevice::x11DefaultVisual () const

    Returns the default Visual of the X display for the paint device
    (X11 only). Using this function is not portable.
*/

static int *dpisX=0, *dpisY=0;
static void create_dpis()
{
    if ( dpisX )
	return;

    Display *dpy = QPaintDevice::x11AppDisplay();
    if ( ! dpy )
	return;

    int i, screens =  ScreenCount( dpy );
    dpisX = new int[ screens ];
    dpisY = new int[ screens ];
    Q_CHECK_PTR( dpisX );
    Q_CHECK_PTR( dpisY );
    for ( i = 0; i < screens; i++ ) {
	dpisX[ i ] = (DisplayWidth(dpy,i) * 254 + DisplayWidthMM(dpy,i)*5)

		     / (DisplayWidthMM(dpy,i)*10);
	dpisY[ i ] = (DisplayHeight(dpy,i) * 254 + DisplayHeightMM(dpy,i)*5)
		     / (DisplayHeightMM(dpy,i)*10);
    }
}

/*!
    Sets the value returned by x11AppDpiX() to \a dpi for screen
    \a screen. The default is determined by the display configuration.
    Changing this value will alter the scaling of fonts and many other
    metrics and is not recommended. Using this function is not
    portable.

    \sa x11SetAppDpiY()
*/
void QPaintDevice::x11SetAppDpiX(int dpi, int screen)
{
    create_dpis();
    if ( ! dpisX )
	return;
    if ( screen < 0 )
	screen = QPaintDevice::x11AppScreen();
    if ( screen > ScreenCount( QPaintDevice::x11AppDisplay() ) )
	return;
    dpisX[ screen ] = dpi;
}

/*!
    \overload

    Sets the value returned by x11AppDpiX() to \a dpi for the default
    screen. The default is determined by the display configuration.
    Changing this value will alter the scaling of fonts and many other
    metrics and is not recommended. Using this function is not
    portable.

*/
// ### REMOVE 4.0
void QPaintDevice::x11SetAppDpiX( int dpi )
{
    QPaintDevice::x11SetAppDpiX( dpi, -1 );
}

/*!
    Sets the value returned by x11AppDpiY() to \a dpi for screen
    \a screen. The default is determined by the display configuration.
    Changing this value will alter the scaling of fonts and many other
    metrics and is not recommended. Using this function is not
    portable.

    \sa x11SetAppDpiX()
*/
void QPaintDevice::x11SetAppDpiY(int dpi, int screen)
{
    create_dpis();
    if ( ! dpisY )
	return;
    if ( screen < 0 )
	screen = QPaintDevice::x11AppScreen();
    if ( screen > ScreenCount( QPaintDevice::x11AppDisplay() ) )
	return;
    dpisY[ screen ] = dpi;
}

/*!
    \overload

    Sets the value returned by x11AppDpiY() to \a dpi for the default
    screen.  The default is determined by the display configuration.
    Changing this value will alter the scaling of fonts and many other
    metrics and is not recommended. Using this function is not
    portable.
*/
// ### REMOVE 4.0
void QPaintDevice::x11SetAppDpiY( int dpi )
{
    QPaintDevice::x11SetAppDpiY( dpi, -1 );
}

/*!
    Returns the horizontal DPI of the X display (X11 only) for screen
    \a screen. Using this function is not portable. See
    QPaintDeviceMetrics for portable access to related information.
    Using this function is not portable.

    \sa x11AppDpiY(), x11SetAppDpiX(), QPaintDeviceMetrics::logicalDpiX()
*/
int QPaintDevice::x11AppDpiX(int screen)
{
    create_dpis();
    if ( ! dpisX )
	return 0;
    if ( screen < 0 )
	screen = QPaintDevice::x11AppScreen();
    if ( screen > ScreenCount( QPaintDevice::x11AppDisplay() ) )
	return 0;
    return dpisX[ screen ];
}

/*!
    \overload

    Returns the horizontal DPI of the X display (X11 only) for the
    default screen. Using this function is not portable. See
    QPaintDeviceMetrics for portable access to related information.
    Using this function is not portable.
*/
int QPaintDevice::x11AppDpiX()
{
    return QPaintDevice::x11AppDpiX( -1 );
}

/*!
    Returns the vertical DPI of the X11 display (X11 only) for screen
    \a screen.  Using this function is not portable. See
    QPaintDeviceMetrics for portable access to related information.
    Using this function is not portable.

    \sa x11AppDpiX(), x11SetAppDpiY(), QPaintDeviceMetrics::logicalDpiY()
*/
int QPaintDevice::x11AppDpiY( int screen )
{
    create_dpis();
    if ( ! dpisY )
	return 0;
    if ( screen < 0 )
	screen = QPaintDevice::x11AppScreen();
    if ( screen > ScreenCount( QPaintDevice::x11AppDisplay() ) )
	return 0;
    return dpisY[ screen ];
}

/*!
    \overload

    Returns the vertical DPI of the X11 display (X11 only) for the
    default screen.  Using this function is not portable. See
    QPaintDeviceMetrics for portable access to related information.
    Using this function is not portable.

    \sa x11AppDpiX(), x11SetAppDpiY(), QPaintDeviceMetrics::logicalDpiY()
*/
int QPaintDevice::x11AppDpiY()
{
    return QPaintDevice::x11AppDpiY( -1 );
}

/*!
    \fn bool QPaintDevice::paintingActive() const

    Returns TRUE if the device is being painted, i.e. someone has
    called QPainter::begin() but not yet called QPainter::end() for
    this device; otherwise returns FALSE.

    \sa QPainter::isActive()
*/

/*!
    Internal virtual function that interprets drawing commands from
    the painter.

    Implemented by subclasses that have no direct support for drawing
    graphics (external paint devices, for example, QPicture).
*/

bool QPaintDevice::cmd( int, QPainter *, QPDevCmdParam * )
{
#if defined(QT_CHECK_STATE)
    qWarning( "QPaintDevice::cmd: Device has no command interface" );
#endif
    return FALSE;
}

/*!
    \internal

    Internal virtual function that returns paint device metrics.

    Please use the QPaintDeviceMetrics class instead.
*/

int QPaintDevice::metric( int ) const
{
#if defined(QT_CHECK_STATE)
    qWarning( "QPaintDevice::metrics: Device has no metric information" );
#endif
    return 0;
}

/*!
    \internal

    Internal virtual function. Reserved for future use.

    Please use the QFontMetrics class instead.
*/

int QPaintDevice::fontMet( QFont *, int, const char *, int ) const
{
    return 0;
}

/*!
    \internal

    Internal virtual function. Reserved for future use.

    Please use the QFontInfo class instead.
*/

int QPaintDevice::fontInf( QFont *, int ) const
{
    return 0;
}


//
// Internal functions for simple GC caching for blt'ing masked pixmaps.
// This cache is used when the pixmap optimization is set to Normal
// and the pixmap size doesn't exceed 128x128.
//

static bool      init_mask_gc = FALSE;
static const int max_mask_gcs = 11;		// suitable for hashing

struct mask_gc {
    GC	gc;
    int mask_no;
};

static mask_gc gc_vec[max_mask_gcs];


static void cleanup_mask_gc()
{
    Display *dpy = QPaintDevice::x11AppDisplay();
    init_mask_gc = FALSE;
    for ( int i=0; i<max_mask_gcs; i++ ) {
	if ( gc_vec[i].gc )
	    XFreeGC( dpy, gc_vec[i].gc );
    }
}

static GC cache_mask_gc( Display *dpy, Drawable hd, int mask_no, Pixmap mask )
{
    if ( !init_mask_gc ) {			// first time initialization
	init_mask_gc = TRUE;
	qAddPostRoutine( cleanup_mask_gc );
	for ( int i=0; i<max_mask_gcs; i++ )
	    gc_vec[i].gc = 0;
    }
    mask_gc *p = &gc_vec[mask_no % max_mask_gcs];
    if ( !p->gc || p->mask_no != mask_no ) {	// not a perfect match
	if ( !p->gc ) {				// no GC
	    p->gc = XCreateGC( dpy, hd, 0, 0 );
	    XSetGraphicsExposures( dpy, p->gc, False );
	}
	XSetClipMask( dpy, p->gc, mask );
	p->mask_no = mask_no;
    }
    return p->gc;
}


/*!
    \relates QPaintDevice

    Copies a block of pixels from \a src to \a dst, perhaps merging
    each pixel according to the \link Qt::RasterOp raster operation \endlink
    \a rop. \a sx, \a sy
    is the top-left pixel in \a src (0, 0) by default, \a dx, \a dy is
    the top-left position in \a dst and \a sw, \a sh is the size of
    the copied block (all of \a src by default).

    The most common values for \a rop are CopyROP and XorROP; the \l
    Qt::RasterOp documentation defines all the possible values.

    If \a ignoreMask is FALSE (the default) and \a src is a
    masked QPixmap, the entire blit is masked by \a{src}->mask().

    If \a src, \a dst, \a sw or \a sh is 0, bitBlt() does nothing. If
    \a sw or \a sh is negative bitBlt() copies starting at \a sx (and
    respectively, \a sy) and ending at the right end (respectively,
    bottom) of \a src.

    \a src must be a QWidget or QPixmap. You cannot blit from a
    QPrinter, for example. bitBlt() does nothing if you attempt to
    blit from an unsupported device.

    bitBlt() does nothing if \a src has a greater depth than \e dst.
    If you need to for example, draw a 24-bit pixmap on an 8-bit
    widget, you must use drawPixmap().
*/

void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QPaintDevice *src, int sx, int sy, int sw, int sh,
	     Qt::RasterOp rop, bool ignoreMask )
{
    if ( !src || !dst ) {
#if defined(QT_CHECK_NULL)
	Q_ASSERT( src != 0 );
	Q_ASSERT( dst != 0 );
#endif
	return;
    }
    if ( !src->handle() || src->isExtDev() )
	return;

    QPaintDevice *pdev = QPainter::redirect( dst );
    if ( pdev )
	dst = pdev;

    int ts = src->devType();			// from device type
    int td = dst->devType();			// to device type
    Display *dpy = src->x11Display();

    if ( sw <= 0 ) {				// special width
	if ( sw < 0 )
	    sw = src->metric( QPaintDeviceMetrics::PdmWidth ) - sx;
	else
	    return;
    }
    if ( sh <= 0 ) {				// special height
	if ( sh < 0 )
	    sh = src->metric( QPaintDeviceMetrics::PdmHeight ) - sy;
	else
	    return;
    }

    if ( dst->paintingActive() && dst->isExtDev() ) {
	QPixmap *pm;				// output to picture/printer
	bool	 tmp_pm = TRUE;
	if ( ts == QInternal::Pixmap ) {
	    pm = (QPixmap*)src;
	    if ( sx != 0 || sy != 0 ||
		 sw != pm->width() || sh != pm->height() || ignoreMask ) {
		QPixmap *tmp = new QPixmap( sw, sh, pm->depth() );
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
	} else if ( ts == QInternal::Widget ) {// bitBlt to temp pixmap
	    pm = new QPixmap( sw, sh );
	    Q_CHECK_PTR( pm );
	    bitBlt( pm, 0, 0, src, sx, sy, sw, sh );
	} else {
#if defined(QT_CHECK_RANGE)
	    qWarning( "bitBlt: Cannot bitBlt from device" );
#endif
	    return;
	}
	QPDevCmdParam param[3];
    QRect r(dx, dy, pm->width(), pm->height());
	param[0].rect	= &r;
	param[1].pixmap = pm;
	dst->cmd( QPaintDevice::PdcDrawPixmap, 0, param );
	if ( tmp_pm )
	    delete pm;
	return;
    }

    switch ( ts ) {
    case QInternal::Widget:
    case QInternal::Pixmap:
    case QInternal::System:			// OK, can blt from these
	break;
    default:
#if defined(QT_CHECK_RANGE)
	qWarning( "bitBlt: Cannot bitBlt from device type %x", ts );
#endif
	return;
    }
    switch ( td ) {
    case QInternal::Widget:
    case QInternal::Pixmap:
    case QInternal::System:			// OK, can blt to these
	break;
    default:
#if defined(QT_CHECK_RANGE)
	qWarning( "bitBlt: Cannot bitBlt to device type %x", td );
#endif
	return;
    }

    static const short ropCodes[] = {			// ROP translation table
	GXcopy, GXor, GXxor, GXandInverted,
	GXcopyInverted, GXorInverted, GXequiv, GXand,
	GXinvert, GXclear, GXset, GXnoop,
	GXandReverse, GXorReverse, GXnand, GXnor
    };
    if ( rop > Qt::LastROP ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "bitBlt: Invalid ROP code" );
#endif
	return;
    }

    if ( dst->handle() == 0 ) {
#if defined(QT_CHECK_NULL)
	qWarning( "bitBlt: Cannot bitBlt to device" );
#endif
	return;
    }

    bool mono_src;
    bool mono_dst;
    bool include_inferiors = FALSE;
    bool graphics_exposure = FALSE;
    QPixmap *src_pm;
    QBitmap *mask;

    if ( ts == QInternal::Pixmap ) {
	src_pm = (QPixmap*)src;
    	if ( src_pm->x11Screen() != dst->x11Screen() )
    	    src_pm->x11SetScreen( dst->x11Screen() );
	mono_src = src_pm->depth() == 1;
	mask = ignoreMask ? 0 : src_pm->data->mask;
    } else {
	src_pm = 0;
	mono_src = FALSE;
	mask = 0;
	include_inferiors = ((QWidget*)src)->testWFlags(Qt::WPaintUnclipped);
	graphics_exposure = td == QInternal::Widget;
    }
    if ( td == QInternal::Pixmap ) {
      	if ( dst->x11Screen() != src->x11Screen() )
      	    ((QPixmap*)dst)->x11SetScreen( src->x11Screen() );
	mono_dst = ((QPixmap*)dst)->depth() == 1;
	((QPixmap*)dst)->detach();		// changes shared pixmap
    } else {
	mono_dst = FALSE;
	include_inferiors = include_inferiors ||
			    ((QWidget*)dst)->testWFlags(Qt::WPaintUnclipped);
    }

    if ( mono_dst && !mono_src ) {	// dest is 1-bit pixmap, source is not
#if defined(QT_CHECK_RANGE)
	qWarning( "bitBlt: Incompatible destination pixmap" );
#endif
	return;
    }

#ifndef QT_NO_XRENDER
    if (src_pm && !mono_src && src_pm->data->alphapm && !ignoreMask ) {
	// use RENDER to do the blit
	QPixmap *alpha = src_pm->data->alphapm;
	if (src->x11RenderHandle() &&
	    alpha->x11RenderHandle() &&
	    dst->x11RenderHandle()) {
	    XRenderPictureAttributes pattr;
	    ulong picmask = 0;
	    if (include_inferiors) {
		pattr.subwindow_mode = IncludeInferiors;
		picmask |= CPSubwindowMode;
	    }
	    if (graphics_exposure) {
		pattr.graphics_exposures = TRUE;
		picmask |= CPGraphicsExposure;
	    }
	    if (picmask)
		XRenderChangePicture(dpy, dst->x11RenderHandle(), picmask, &pattr);
	    XRenderComposite(dpy, PictOpOver, src->x11RenderHandle(),
			     alpha->x11RenderHandle(), dst->x11RenderHandle(),
			     sx, sy, sx, sy, dx, dy, sw, sh);
	    // restore attributes
	    pattr.subwindow_mode = ClipByChildren;
	    pattr.graphics_exposures = FALSE;
	    if (picmask)
		XRenderChangePicture(dpy, dst->x11RenderHandle(), picmask, &pattr);
	    return;
	}
    }
#endif

    GC gc;

    if ( mask && !mono_src ) {			// fast masked blt
	bool temp_gc = FALSE;
	if ( mask->data->maskgc ) {
	    gc = (GC)mask->data->maskgc;	// we have a premade mask GC
	} else {
	    if ( FALSE && src_pm->optimization() == QPixmap::NormalOptim ) { // #### cache disabled
		// Compete for the global cache
		gc = cache_mask_gc( dpy, dst->handle(),
				    mask->data->ser_no,
				    mask->handle() );
	    } else {
		// Create a new mask GC. If BestOptim, we store the mask GC
		// with the mask (not at the pixmap). This way, many pixmaps
		// which have a common mask will be optimized at no extra cost.
		gc = XCreateGC( dpy, dst->handle(), 0, 0 );
		XSetGraphicsExposures( dpy, gc, False );
		XSetClipMask( dpy, gc, mask->handle() );
		if ( src_pm->optimization() == QPixmap::BestOptim ) {
		    mask->data->maskgc = gc;
		} else {
		    temp_gc = TRUE;
		}
	    }
	}
	XSetClipOrigin( dpy, gc, dx-sx, dy-sy );
	if ( rop != Qt::CopyROP )		// use non-default ROP code
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

	if ( temp_gc )				// delete temporary GC
	    XFreeGC( dpy, gc );
	else if ( rop != Qt::CopyROP )		// restore ROP
	    XSetFunction( dpy, gc, GXcopy );
	return;
    }

    gc = qt_xget_temp_gc( dst->x11Screen(), mono_dst );		// get a reusable GC

    if ( rop != Qt::CopyROP )			// use non-default ROP code
	XSetFunction( dpy, gc, ropCodes[rop] );

    if ( mono_src && mono_dst && src == dst ) { // dst and src are the same bitmap
	XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh, dx, dy );
    } else if ( mono_src ) {			// src is bitmap
	XGCValues gcvals;
	ulong	  valmask = GCBackground | GCForeground | GCFillStyle |
			    GCStipple | GCTileStipXOrigin | GCTileStipYOrigin;
	if ( td == QInternal::Widget ) {	// set GC colors
	    QWidget *w = (QWidget *)dst;
	    gcvals.background = w->backgroundColor().pixel( dst->x11Screen() );
	    gcvals.foreground = w->foregroundColor().pixel( dst->x11Screen() );
	    if ( include_inferiors ) {
		valmask |= GCSubwindowMode;
		gcvals.subwindow_mode = IncludeInferiors;
	    }
	} else if ( mono_dst ) {
	    gcvals.background = 0;
	    gcvals.foreground = 1;
	} else {
	    gcvals.background = Qt::white.pixel( dst->x11Screen() );
	    gcvals.foreground = Qt::black.pixel( dst->x11Screen() );
	}

	gcvals.fill_style  = FillOpaqueStippled;
	gcvals.stipple	   = src->handle();
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

    } else {					// src is pixmap/widget

	if ( graphics_exposure )		// widget to widget
	    XSetGraphicsExposures( dpy, gc, True );
	if ( include_inferiors ) {
	    XSetSubwindowMode( dpy, gc, IncludeInferiors );
	    XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
		       dx, dy );
	    XSetSubwindowMode( dpy, gc, ClipByChildren );
	} else {
	    XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
		       dx, dy );
	}
	if ( graphics_exposure )		// reset graphics exposure
	    XSetGraphicsExposures( dpy, gc, False );
    }

    if ( rop != Qt::CopyROP )			// restore ROP
	XSetFunction( dpy, gc, GXcopy );
}


/*!
    \relates QPaintDevice

    \overload void bitBlt( QPaintDevice *dst, const QPoint &dp, const QPaintDevice *src, const QRect &sr, RasterOp rop )

    Overloaded bitBlt() with the destination point \a dp and source
    rectangle \a sr.
*/


/*!
  \internal
*/
// makes it possible to add a setResolution as we have in QPrinter for all
// paintdevices without breaking bin compatibility.
void QPaintDevice::setResolution( int )
{
}

/*!\internal
*/
int QPaintDevice::resolution() const
{
    return metric( QPaintDeviceMetrics::PdmDpiY );
}
