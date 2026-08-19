/****************************************************************************
** $Id: qpixmap_win.cpp 2056 2007-03-02 21:30:07Z chehrlic $
**
** Implementation of QPixmap class for X11
**
** Created : 20030119
**
** Copyright (C) 2003 Holger Schroeder
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2003 Ivan de Jesus Deras Tabora
** Copyright (C) 2004 Tom and Timi Cecka
** Copyright (C) 2004 Christian Ehrlicher
** Copyright (C) 2004 Andreas Hausladen
** Copyright (C) 2004, 2005 Peter Kuemmel
** Copyright (C) 2004, 2005 Christian Ehrlicher
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

#include "qglobal.h"
#include "qt_windows.h"
#include "qpixmap.h"

#include "qbitmap.h"
#include "qimage.h"
#include "qpaintdevicemetrics.h"
#include "qwmatrix.h"

#undef MITSHM

//#define DEBUG_QPIXMAP

/*****************************************************************************
  Internal functions
 *****************************************************************************/
// DWORD - aligned! only usefull for GetDiBits!
#define BytesPerRow(w, d) ((d)==1? ((((w) + 31) & ~31) >> 3): (d)==8? ((((w) + 3) / 4) * 4) : (d)==16? ((((w)*2 + 3)/4) * 4) : (w)*4)
bool qt_GetBitmapBits( const QPixmap *pm, uchar **bits, long &sbpr )
{
    HBITMAP hbm = pm->hbm();
    HDC hdc = pm->handle();
    BITMAP bm;
    GetObjectA( hbm, sizeof( BITMAP ), &bm );
    sbpr = bm.bmWidthBytes; // CE: better than calculating

    *bits = ( uchar* )bm.bmBits;
    if ( *bits )
        return false;

    LPBITMAPINFO bitmapInfo = ( LPBITMAPINFO ) new uchar[ sizeof ( BITMAPINFOHEADER ) + 256 * sizeof ( RGBQUAD ) ];
    memset ( bitmapInfo, 0, sizeof ( BITMAPINFOHEADER ) + 256 * sizeof ( RGBQUAD ) );
    bitmapInfo->bmiHeader.biSize = sizeof ( bitmapInfo->bmiHeader );
    GetDIBits ( hdc, hbm, 0, 0, NULL, bitmapInfo, DIB_RGB_COLORS );
    *bits = new uchar[ bitmapInfo->bmiHeader.biSizeImage ];
    memset ( *bits, 0, bitmapInfo->bmiHeader.biSizeImage );
    if ( bitmapInfo->bmiHeader.biHeight > 0 )
        bitmapInfo->bmiHeader.biHeight = -bitmapInfo->bmiHeader.biHeight;
    GetDIBits ( hdc, hbm, 0, -bitmapInfo->bmiHeader.biHeight, *bits, bitmapInfo, DIB_RGB_COLORS );

    // GetDIBits returns Bits DWORD - aligned!
    sbpr = BytesPerRow( bitmapInfo->bmiHeader.biWidth, bitmapInfo->bmiHeader.biBitCount );
    delete [] bitmapInfo;
    return true;       
}

extern const uchar *qt_get_bitflip_array();  // defined in qimage.cpp

static uchar *flip_bits( const uchar *bits, int len )
{
#ifdef DEBUG_QPIXMAP
    qDebug( "qpixmap_win.cpp: flip_bits" );
#endif

    register const uchar *p = bits;
    const uchar *end = p + len;
    uchar *newdata = new uchar[ len ];
    uchar *b = newdata;
    const uchar *f = qt_get_bitflip_array();
    while ( p < end )
        * b++ = f[ *p++ ];
    return newdata;
}

/*****************************************************************************
  QPixmap member functions
 *****************************************************************************/

/*!
  \internal
  Initializes the pixmap data.
*/

void QPixmap::init( int w, int h, int d, bool bitmap, Optimization optim )
{
#ifdef DEBUG_QPIXMAP
    qDebug( "QPixmap::init( w: %d, h: %d, depth: %d, bitmap: %d, optim: %d .",
            w, h, d, bitmap, optim );
#endif

    static int serial = 0;
    int dd = 32; // int dd = x11Depth();

    if ( optim == DefaultOptim )       // use default optimization
        optim = defOptim;

    data = new QPixmapData;
    CHECK_PTR( data );

    memset( data, 0, sizeof( QPixmapData ) );
    data->count = 1;
    data->uninit = TRUE;
    data->bitmap = bitmap;
    data->ser_no = ++serial;
    data->optim = optim;
    data->mask = 0;
    data->realAlphaBits = 0;

    bool make_null = w == 0 || h == 0;  // create null pixmap
    if ( d == 1 )       // monocrome pixmap
        data->d = 1;
    else if ( d < 0 || d == dd )   // def depth pixmap
        data->d = dd;
    if ( make_null || w < 0 || h < 0 || data->d == 0 ) {
        hdc = 0;
        data->hbm_or_mcpi.hbm = 0;
#if defined(QT_CHECK_RANGE)

        if ( !make_null )
            qWarning( "QPixmap: Invalid pixmap parameters" );
#endif

        return ;
    }
    data->w = w;
    data->h = h;
    data->mcp = 0;

    hdc = CreateCompatibleDC( NULL );

    if ( data->d == 1 ) {
        int bytes = ( w + 7 ) >> 3;
        int newbytes = ( bytes & 1 ) ? bytes + 1 : bytes; //WORD aligned

        uchar *lpBits = ( uchar* ) new uchar[ newbytes * h ];
        memset( lpBits, 0, w * h / 8 );
        data->hbm_or_mcpi.hbm = CreateBitmap( w, h, 1, 1, lpBits ); // is faster
        delete lpBits;

    } else {
        BITMAPINFO bmpInfo;
        void *lpBits;

        bmpInfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
        bmpInfo.bmiHeader.biWidth = data->w;
        bmpInfo.bmiHeader.biHeight = -data->h; //Top down bitmap!!
        bmpInfo.bmiHeader.biPlanes = 1;
        bmpInfo.bmiHeader.biBitCount = data->d;
        bmpInfo.bmiHeader.biCompression = BI_RGB; // No compression
        bmpInfo.bmiHeader.biSizeImage = 0; // Not set when Compression = BI_RGB
        bmpInfo.bmiHeader.biXPelsPerMeter = 0;
        bmpInfo.bmiHeader.biYPelsPerMeter = 0;
        bmpInfo.bmiHeader.biClrUsed = 0; // Asumes that all colors are used
        bmpInfo.bmiHeader.biClrImportant = 0; // Asumes that all colors are important

        data->hbm_or_mcpi.hbm = CreateDIBSection( hdc, &bmpInfo, DIB_RGB_COLORS, &lpBits, NULL, 0 );
    }

    data->old_hbm = ( HBITMAP ) SelectObject( hdc, data->hbm_or_mcpi.hbm );
}

void QPixmap::deref()
{
#ifdef DEBUG_QPIXMAP
    qDebug( "QPixmap::deref()" );
#endif

    if ( data && data->deref() ) {   // last reference lost
        if ( data->mask )
            delete data->mask;
        if ( data->hbm_or_mcpi.hbm ) {
            SelectObject( hdc, data->old_hbm );
            if ( !DeleteObject( data->hbm_or_mcpi.hbm ) ) {
#ifdef DEBUG_QPIXMAP
                qDebug( "qpixmap_win.cpp: deref() failed deleting hbm=%X, GetLastError()=%d", data->hbm_or_mcpi.hbm, GetLastError() );
#endif

            }
        }
        if ( hdc )
            DeleteDC( hdc );
        hdc = 0;
        data->hbm_or_mcpi.hbm = 0;
        delete data;
    }
}


/*!
  Constructs a monochrome pixmap which is initialized with the data in \e bits.
  This constructor is protected and used by the QBitmap class.
*/
QPixmap::QPixmap( int w, int h, const uchar *bits, bool isXbitmap )
        : QPaintDevice( QInternal::Pixmap )
{      // for bitmaps only
#ifdef DEBUG_QPIXMAP
    qDebug( "qpixmap_win.cpp:QPixmap::QPixmap( int w, int h, const uchar *bits, bool isXbitmap )" );
#endif

    init( 0, 0, 0, FALSE, defOptim );
    if ( w <= 0 || h <= 0 )                     // create null pixmap
        return ;

    data->uninit = FALSE;
    data->w = w;
    data->h = h;
    data->d = 1;
    uchar *flipped_bits;
    if ( isXbitmap ) {
        flipped_bits = flip_bits( bits, ( ( w + 7 ) >> 3 ) * h );
        bits = flipped_bits;
    } else {                                // not X bitmap, we assumes Windows format
        flipped_bits = 0;
    }

    int bytes = ( w + 7 ) >> 3;
    int newbytes = ( bytes & 1 ) ? bytes + 1 : bytes; //WORD aligned

    uchar *newbits = ( uchar* ) new uchar[ newbytes * h ];
    for ( int i = 0; i < h; i++ ) {
        for ( int j = 0; j < bytes; j++ ) {
            newbits[ ( i * newbytes ) + j ] = ~( bits[ ( i * bytes ) + j ] );
        }
    }
    data->mcp = 0;
    data->hbm_or_mcpi.hbm = CreateBitmap( data->w, data->h, 1, 1, newbits );
#ifdef DEBUG_QPIXMAP

    if ( data->hbm_or_mcpi.hbm == NULL )
        qDebug( "qpixmap_win.cpp:421: Cannot create pixmap, GetLastError()=%d", GetLastError() );
#endif

    hdc = CreateCompatibleDC( NULL ); //NULL parameter mean Screen compatible DC
    data->old_hbm = ( HBITMAP ) SelectObject( hdc, data->hbm_or_mcpi.hbm );

    if ( flipped_bits )
        delete [] flipped_bits;
    if ( newbits )
        delete [] newbits;
}


/*!
  Special-purpose function that detaches the pixmap from shared pixmap data.

  A pixmap is automatically detached by Qt whenever its contents is about
  to change.  This is done in all QPixmap member functions that modify the
  pixmap (fill(), resize(), convertFromImage(), load() etc.), in bitBlt()
  for the destination pixmap and in QPainter::begin() on a pixmap.

  It is possible to modify a pixmap without letting Qt know.
  You can first obtain the \link handle() system-dependent handle\endlink
  and then call system-specific functions (for instance BitBlt under Windows)
  that modifies the pixmap contents.  In this case, you can call detach()
  to cut the pixmap loose from other pixmaps that share data with this one.

  detach() returns immediately if there is just a single reference or if
  the pixmap has not been initialized yet.
*/

void QPixmap::detach()
{
#ifdef DEBUG_QPIXMAP
    qDebug( "qpixmap_win.cpp: detach" );
#endif

    if ( data->uninit || data->count == 1 )
        data->uninit = FALSE;
    else
        *this = copy();
}


/*!
  Returns the default pixmap depth, i.e. the depth a pixmap gets
  if -1 is specified.
  \sa depth()
*/

int QPixmap::defaultDepth()
{
#ifdef DEBUG_QPIXMAP
    qDebug( "QPixmap::defaultDepth()" );
#endif

    return GetDeviceCaps( qt_display_dc(), PLANES ) * GetDeviceCaps( qt_display_dc(), BITSPIXEL );
}


/*!
  \fn QPixmap::Optimization QPixmap::optimization() const

  Returns the optimization setting for this pixmap.

  The default optimization setting is \c QPixmap::NormalOptim. You may
  change this settings in two ways:
  <ul>
  <li> Call setDefaultOptimization() to set the default optimization
  for all new pixmaps.
  <li> Call setOptimization() to set a the optimization for individual
  pixmaps.
  </ul>

  \sa setOptimization(), setDefaultOptimization(), defaultOptimization()
*/

/*!
  Sets pixmap drawing optimization for this pixmap.

  The optimization setting affects pixmap operations, in particular
  drawing of transparent pixmaps (bitBlt() a pixmap with a mask set) and
  pixmap transformations (the xForm() function).

  Pixmap optimization involves keeping intermediate results in a cache
  buffer and use the data in the cache to speed up bitBlt() and xForm().
  The cost is more memory consumption, up to twice as much as an
  unoptimized pixmap.

  Use the setDefaultOptimization() to change the default optimization
  for all new pixmaps.

  \sa optimization(), setDefaultOptimization(), defaultOptimization()
*/

void QPixmap::setOptimization( Optimization optimization )
{
#ifdef DEBUG_QPIXMAP
    qDebug( "TODO qpixmap_win.cpp: setOptimization" );
#endif
    data->optim = optimization == DefaultOptim ? defOptim : optimization;
    /*    if ( optimization == data->optim )
     return;
        detach();
        data->optim = optimization == DefaultOptim ?
         defOptim : optimization;
        if ( data->optim == MemoryOptim && data->ximage ) {
     qSafeXDestroyImage( (XImage*)data->ximage );
     data->ximage = 0;
        }
    */
}


/*!
  Fills the pixmap with the color \e fillColor.
*/

void QPixmap::fill( const QColor &fillColor )
{
#ifdef DEBUG_QPIXMAP
    qDebug( "qpixmap_win.cpp: fill hdc = %X, hbm=%X, fillColor=(%d, %d, %d)", handle(), hbm(), fillColor.red(), fillColor.green(), fillColor.blue() );
#endif

    if ( isNull() ) {
        return ;
    }

    detach();     // detach other references
    RECT r;
    r.top = 0;
    r.left = 0;
    r.right = width();
    r.bottom = height();

    if ( data->bitmap ) {
        HBRUSH hMyBrush = CreateSolidBrush( RGB( fillColor.red(), fillColor.green(), fillColor.blue() ) );
        FillRect( hdc, &r, hMyBrush );
        DeleteObject( hMyBrush );
        return ;
    } else {
        // OK, OK, this requires an explanation: ExtTextOut is a fast way to clear
        // an area, because it doesn't need a BRUSH to fill it ...
        COLORREF oldBkColor = SetBkColor( hdc, RGB( fillColor.red(), fillColor.green(), fillColor.blue() ) );
        bool ret = ExtTextOutA( hdc, 0, 0, ETO_OPAQUE, &r, ( LPCSTR ) "", 0, 0 ) ;
        SetBkColor( hdc, oldBkColor );
#ifdef DEBUG_QPIXMAP

        if ( !ret )
            qDebug( "qpixmap_win.cpp: fill failed GetLastError()=%d", GetLastError() );
#endif

    }
}

/*!
  Internal implementation of the virtual QPaintDevice::metric() function.

  Use the QPaintDeviceMetrics class instead.
*/

int QPixmap::metric( int m ) const
{
#ifdef DEBUG_QPIXMAP
    qDebug( "QPixmap::metric( %d )", m );
#endif

    int val;
    if ( m == QPaintDeviceMetrics::PdmWidth )
        val = width();
    else if ( m == QPaintDeviceMetrics::PdmHeight ) {
        val = height();
    } else {
        HDC ScreenDC = qt_display_dc();
        int ScreenWidth = GetDeviceCaps( ScreenDC, HORZRES ), ScreenHeight = GetDeviceCaps( ScreenDC, VERTRES );
        int ScreenWidthMM = GetDeviceCaps( ScreenDC, HORZSIZE ), ScreenHeightMM = GetDeviceCaps( ScreenDC, VERTSIZE );
        switch ( m ) {
        case QPaintDeviceMetrics::PdmDpiX:
            val = ( ScreenWidth * 254 + ScreenWidthMM * 5 ) / ( ScreenWidthMM * 10 );
            break;
        case QPaintDeviceMetrics::PdmDpiY:
            val = ( ScreenHeight * 254 + ScreenHeightMM * 5 ) / ( ScreenHeightMM * 10 );
            break;
        case QPaintDeviceMetrics::PdmWidthMM:
            val = ( ScreenWidthMM * width() ) / ScreenWidth;
            break;
        case QPaintDeviceMetrics::PdmHeightMM:
            val = ( ScreenHeightMM * height() ) / ScreenHeight;
            break;
        case QPaintDeviceMetrics::PdmNumColors:
            val = 1 << depth();
            break;
        case QPaintDeviceMetrics::PdmDepth:
            val = depth();
            break;
        default:
            val = 0;
            qWarning( "QPixmap::metric: Invalid metric command" );
        }
    }
    return val;
}

/*!
  Converts the pixmap to an image. Returns a null image if the operation
  failed.

  If the pixmap has 1 bit depth, the returned image will also be 1
  bits deep.  If the pixmap has 2-8 bit depth, the returned image
  has 8 bit depth.  If the pixmap has greater than 8 bit depth, the
  returned image has 32 bit depth.

  \bug Does not support 2 or 4 bit display hardware.

  \bug Alpha masks on monochrome images are ignored.

  \sa convertFromImage()
*/

QImage QPixmap::convertToImage() const
{
#ifdef DEBUG_QPIXMAP
    qDebug( "QPixmap::convertToImage()" );
#endif

    if ( data->w == 0 || hdc == 0 )
        return QImage(); // null image

    int w = data->w;
    int h = data->h;
    int d = data->d;
    int ncols = 2;

    if ( d != 1 ) {
        d = 32;
        ncols = 0;
    }

    QImage image( w, h, d, ncols, QImage::BigEndian );
    if ( d == 1 ) {
        image.setNumColors( 2 );
        image.setColor( 0, Qt::black.rgb() );
        image.setColor( 1, Qt::white.rgb() );
    }

    uchar *bits;
    long sbpr;
    bool freeBits = qt_GetBitmapBits( this, &bits, sbpr );
    uchar* sptr = ( uchar* ) bits, *srow = sptr;
    uchar* drow;
    int x, y;

    for ( y = 0; y < h; y++ ) {
        drow = image.scanLine( y );
        srow = sptr + y * sbpr;
        if ( d == 1 ) {
            for ( x = 0; x < w; x += 8 ) {
                *( drow++ ) = *( srow++ );
            }
        } else {
            QRgb* ib = ( QRgb* ) image.scanLine( y );
            if ( data->realAlphaBits ) {
                for ( x = 0; x < w; x++ ) {
                    RGBQUAD* pix = ( RGBQUAD* ) & srow[ x * sizeof( RGBQUAD ) ];
                    int alpha = pix->rgbReserved;
                    if ( alpha == 0 ) {
                        ib[ x ] = qRgba( 0, 0, 0, 0 );
                    } else if ( alpha != 0xff ) {
                        int red = pix->rgbRed * 0xff / alpha;
                        int blue = pix->rgbBlue * 0xff / alpha;
                        int green = pix->rgbGreen * 0xff / alpha;
                        ib[ x ] = qRgba( red, green, blue, alpha );
                    } else {
                        ib[ x ] = qRgba( pix->rgbRed, pix->rgbGreen, pix->rgbBlue, 0xff );
                    }
                }
            } else {
                for ( x = 0; x < w; x++ ) {
                    RGBQUAD* pix = ( RGBQUAD* ) & srow[ x * sizeof( RGBQUAD ) ];
                    ib[ x ] = qRgba( pix->rgbRed, pix->rgbGreen, pix->rgbBlue, 0xff );
                }
            }
        }
    }

    if ( data->realAlphaBits ) {
        // already set values above...
        image.setAlphaBuffer( TRUE );
    } else {
        if ( data->mask && d != 1 ) {
            QImage alpha = data->mask->convertToImage();
            image.setAlphaBuffer( TRUE );
            for ( y = 0; y < image.height(); y++ ) {
                uchar* mb = alpha.scanLine( y );
                QRgb* ib = ( QRgb* ) image.scanLine( y );
                uchar bit = 0x80;
                int i = image.width();
                while ( i-- ) {
                    if ( *mb & bit )
                        * ib &= 0x00ffffff;
                    else
                        *ib |= 0xff000000;
                    bit /= 2;
                    if ( !bit )
                        mb++, bit = 0x80; // ROL
                    ib++;
                }
            }
        }
    }
    if ( freeBits )
        delete [] bits;

    return image;
}


/*!
  Converts an image and sets this pixmap. Returns TRUE if successful.

  The \a conversion_flags argument is a bitwise-OR from the following choices.
  The options marked \e (default) are the choice if no other choice from the
  list is included (they are zero):

  <dl>
   <dt>Color/Mono preference (ignored for QBitmap)
   <dd>
    <ul>
     <li> \c AutoColor (default) - If the \e image has \link
        QImage::depth() depth\endlink 1 and contains only
        black and white pixels, then the pixmap becomes monochrome.
     <li> \c ColorOnly - The pixmap is dithered/converted to the
        \link defaultDepth() native display depth\endlink.
     <li> \c MonoOnly - The pixmap becomes monochrome.  If necessary,
        it is dithered using the chosen dithering algorithm.
    </ul>
   <dt>Dithering mode preference, for RGB channels
   <dd>
    <ul>
     <li> \c DiffuseDither (default) - a high quality dither
     <li> \c OrderedDither - a faster more ordered dither
     <li> \c ThresholdDither - no dithering, closest color is used
    </ul>
   <dt>Dithering mode preference, for alpha channel
   <dd>
    <ul>
     <li> \c DiffuseAlphaDither - a high quality dither
     <li> \c OrderedAlphaDither - a faster more ordered dither
     <li> \c ThresholdAlphaDither (default) - no dithering
    </ul>
   <dt>Color matching versus dithering preference
   <dd>
    <ul>
     <li> \c PreferDither - always dither 32-bit images when
  the image
  is being converted to 8-bits.
  This is the default when converting to a pixmap.
     <li> \c AvoidDither - only dither 32-bit images if
  the image
  has more than 256 colors and it
  is being converted to 8-bits.
  This is the default when an image is converted
  for the purpose of saving to a file.
    </ul>
  </dl>

  Passing 0 for \a conversion_flags gives all the default options.

  Note that even though a QPixmap with depth 1 behaves much like a
  QBitmap, isQBitmap() returns FALSE.

  If a pixmap with depth 1 is painted with color0 and color1 and
  converted to an image, the pixels painted with color0 will produce
  pixel index 0 in the image and those painted with color1 will produce
  pixel index 1.

  \bug Does not support 2 or 4 bit display hardware.

  \sa convertToImage(), isQBitmap(), QImage::convertDepth(), defaultDepth(),
  QImage::hasAlphaBuffer()
*/

static inline QRgb qt_conv16ToRgb( ushort c )
{
    static const int qt_rbits = ( 565 / 100 );
    static const int qt_gbits = ( 565 / 10 % 10 );
    static const int qt_bbits = ( 565 % 10 );
    static const int qt_red_shift = qt_bbits + qt_gbits - ( 8 - qt_rbits );
    static const int qt_green_shift = qt_bbits - ( 8 - qt_gbits );
    static const int qt_neg_blue_shift = 8 - qt_bbits;
    static const int qt_blue_mask = ( 1 << qt_bbits ) - 1;
    static const int qt_green_mask = ( 1 << ( qt_gbits + qt_bbits ) ) - ( ( 1 << qt_bbits ) - 1 );
    static const int qt_red_mask = ( 1 << ( qt_rbits + qt_gbits + qt_bbits ) ) - ( 1 << ( qt_gbits + qt_bbits ) );

    const int r = ( c & qt_red_mask );
    const int g = ( c & qt_green_mask );
    const int b = ( c & qt_blue_mask );
    const int tr = r >> qt_red_shift;
    const int tg = g >> qt_green_shift;
    const int tb = b << qt_neg_blue_shift;

    return qRgb( tr, tg, tb );
}

static inline void qt_setPixel( RGBQUAD *pix, int q, bool bRealAlpha )
{
    if ( bRealAlpha ) {
        int alpha = qAlpha( q );

        pix->rgbReserved = alpha;
        if ( alpha != 0xff ) {
            pix->rgbRed = qRed( q ) * alpha / 0xff;
            pix->rgbBlue = qBlue( q ) * alpha / 0xff;
            pix->rgbGreen = qGreen( q ) * alpha / 0xff;
        } else {
            pix->rgbRed = qRed( q );
            pix->rgbBlue = qBlue( q );
            pix->rgbGreen = qGreen( q );
        }
    } else {
        pix->rgbReserved = 0xff;
        pix->rgbRed = qRed( q );
        pix->rgbBlue = qBlue( q );
        pix->rgbGreen = qGreen( q );
    }
}

bool QPixmap::convertFromImage( const QImage &img, int conversion_flags )
{
#ifdef DEBUG_QPIXMAP
    qDebug( "QPixmap::convertFromImage( w/h/d: %d/%d/%d, flags: %d)", img.width(), img.height(), img.depth(), conversion_flags );
#endif

    if ( img.isNull() ) {
#if defined(QT_CHECK_NULL)
        warning( "QPixmap::convertFromImage: Cannot convert a null image" );
#endif

        return FALSE;
    }

    detach();     // detach other references
    QImage image = img;
    int d = image.depth();
    int dd = defaultDepth();
    bool force_mono = ( dd == 1 || isQBitmap() ||
                        ( conversion_flags & ColorMode_Mask ) == MonoOnly );
    if ( force_mono ) {                         // must be monochrome
        if ( d != 1 ) {
            image = image.convertDepth( 1, conversion_flags );  // dither
            d = 1;
        }
    } else {                                    // can be both
        bool conv8 = FALSE;
        if ( d > 8 && dd <= 8 ) {               // convert to 8 bit
            if ( ( conversion_flags & DitherMode_Mask ) == AutoDither )
                conversion_flags = ( conversion_flags & ~DitherMode_Mask )
                                   | PreferDither;
            conv8 = TRUE;
        } else if ( ( conversion_flags & ColorMode_Mask ) == ColorOnly ) {
            conv8 = d == 1;                     // native depth wanted
        } else if ( d == 1 ) {
            if ( image.numColors() == 2 ) {
                QRgb c0 = image.color( 0 );       // Auto: convert to best
                QRgb c1 = image.color( 1 );
                conv8 = QMIN( c0, c1 ) != qRgb( 0, 0, 0 ) || QMAX( c0, c1 ) != qRgb( 255, 255, 255 );
            } else {
                // eg. 1-color monochrome images (they do exist).
                conv8 = TRUE;
            }
        }
        if ( conv8 ) {
            image = image.convertDepth( 8, conversion_flags );
            d = 8;
        }
    }

    int w = image.width();
    int h = image.height();

    if ( width() == w && height() == h && depth() == d ) {
        // same size etc., use the existing pixmap
        detach();

        if ( data->mask ) {                     // get rid of the mask
            delete data->mask;
            data->mask = 0;
        }
    } else {
        // different size or depth, make a new pixmap
        QPixmap pm( w, h, d == 1 ? 1 : 32 );
        pm.data->bitmap = data->bitmap;        // keep is-a flag
        pm.data->optim = data->optim;          // keep optimization flag
        *this = pm;
    }

    if ( d == 1 ) {
        // just to be sure that we don't have an alpha channel somewhere...
        QRgb rgbWhite = Qt::white.rgb() & 0x00ffffff;
        QRgb rgbBlack = Qt::black.rgb() & 0x00ffffff;
        QRgb rgbCol0 = image.color( 0 ) & 0x00ffffff;
        QRgb rgbCol1 = image.color( 1 ) & 0x00ffffff;

        // make sure image.color(0) == color0 (white) and image.color(1) == color1 (black)
        if ( ( rgbCol0 == rgbWhite ) && ( rgbCol1 == rgbBlack ) ) {
            image.invertPixels();
            image.setColor( 0, Qt::black.rgb() );
            image.setColor( 1, Qt::white.rgb() );
        }
        char * bits;
        uchar *tmp_bits;
        int bpl = ( ( w + 15 ) / 16 ) * 2;
        int ibpl = image.bytesPerLine();

        if ( image.bitOrder() == QImage::LittleEndian || bpl != ibpl ) {
            tmp_bits = new uchar[ bpl * h ];
            bits = ( char * ) tmp_bits;
            uchar *p, *b, *end;
            int y;
            if ( image.bitOrder() == QImage::LittleEndian ) {
                const uchar * f = qt_get_bitflip_array();
                b = tmp_bits;
                for ( y = 0; y < h; y++ ) {
                    p = image.scanLine( y );
                    end = p + bpl;
                    while ( p < end )
                        * b++ = f[ *p++ ];
                }
            } else {    // just copy
                b = tmp_bits;
                p = image.scanLine( 0 );
                for ( y = 0; y < h; y++ ) {
                    memcpy( b, p, bpl );
                    b += bpl;
                    p += ibpl;
                }
            }
        } else {
            bits = ( char * ) image.bits();
            tmp_bits = 0;
        }

        if ( data->hbm_or_mcpi.hbm ) {
            SelectObject ( hdc, data->old_hbm );
            if ( !DeleteObject( data->hbm_or_mcpi.hbm ) ) {
#ifdef DEBUG_QPIXMAP
                qDebug( "qpixmap_win.cpp: deref() failed deleting hbm=%X, GetLastError()=%d",
                        data->hbm_or_mcpi.hbm, GetLastError() );
#endif

            }
        }
        data->hbm_or_mcpi.hbm = CreateBitmap ( w, h, 1, 1, bits );
        data->old_hbm = ( HBITMAP ) SelectObject ( hdc, data->hbm_or_mcpi.hbm );

        if ( tmp_bits )         // Avoid purify complaint
            delete [] tmp_bits;

        if ( image.hasAlphaBuffer() ) {
            QBitmap m;
            m = image.createAlphaMask( conversion_flags );
            setMask( m );
        }
        data->w = w;
        data->h = h;
        data->d = 1;
        data->uninit = FALSE;
        return true;
    }

    if ( !hdc )
        qDebug( "Qt: internal: No hdc! %s %d", __FILE__, __LINE__ );

    uchar *dptr;
    long dbpr;
    bool freeBits = qt_GetBitmapBits( this, &dptr, dbpr );
    uchar *drow = dptr;

    QRgb q = 0;
    int sdepth = image.depth();
    QImage::Endian sord = image.bitOrder();
    const uchar* f = qt_get_bitflip_array();

    bool salpha = ( ( sdepth == 8 || sdepth == 16 || sdepth == 32 ) && img.hasAlphaBuffer() );

    if ( salpha )
        data->realAlphaBits = (uchar*)1;
    else
        data->realAlphaBits = 0;
    for ( int y = 0; y < h; y++ ) {
        /* MSDN:
       For example, if the alpha channel value is x, the red, green and blue
       channels must be multiplied by x and divided by 0xff prior to the call */
        uchar* ptEnd = drow + dbpr;

        uchar* srow = image.scanLine( y );
        switch ( sdepth ) {
        case 1: {
                for ( int x = 0; x < w; x += 8, drow++ ) {
                    if ( sord == QImage::LittleEndian )
                        * drow = f[ *( srow + ( x >> 3 ) ) ];
                    else
                        *drow = *( srow + ( x >> 3 ) );
                }
                break;
            }
        case 8: {
                for ( int x = 0; x < w; x++, drow += sizeof( RGBQUAD ) ) {
                    q = image.color( *( srow + x ) );
                    qt_setPixel( ( RGBQUAD* ) drow, q, salpha );
                }
                break;
            }
        case 16: {
                for ( int x = 0; x < w; x++, drow += sizeof( RGBQUAD ) ) {
                    q = qt_conv16ToRgb( *( ( ( ushort * ) srow ) + x ) );
                    qt_setPixel( ( RGBQUAD* ) drow, q, salpha );
                }
                break;
            }
        case 32: {
                for ( int x = 0; x < w; x++, drow += sizeof( RGBQUAD ) ) {
                    q = *( ( ( QRgb * ) srow ) + x );
                    qt_setPixel( ( RGBQUAD* ) drow, q, salpha );
                }
                break;
            }
        default:
            qDebug( "Qt: internal: Oops: Forgot a depth %s:%d", __FILE__, __LINE__ );
            break;
        }
        //Padding
        while ( drow != ptEnd )
            * ( drow++ ) = 0;
    }
    data->uninit = FALSE;

    if ( salpha ) {
        QBitmap m;
        m = img.createAlphaMask( conversion_flags );
        setMask( m );
    }
    if ( freeBits )
        delete [] dptr;
    return TRUE;
}


/*!
  Grabs the contents of a window and makes a pixmap out of it.
  Returns the pixmap.

  The arguments \e (x,y) specify the offset in the window, while
  \e (w,h) specify the width and height of the area to be copied.

  If \e w is negative, the function copies everything to the right
  border of the window.  If \e h is negative, the function copies
  everything to the bottom of the window.

  Note that grabWindows() grabs pixels from the screen, not from the
  window.  This means that If there is another window partially or
  entirely over the one you grab, you get pixels from the overlying
  window too.

  Note also that the mouse cursor is generally not grabbed.

  The reason we use a window identifier and not a QWidget is to enable
  grabbing of windows that are not part of the application, window
  system frames, and so on.

  \warning Grabbing an area outside the screen is not safe in general.
  This depends on the underlying window system.

  \sa grabWidget()
*/

QPixmap QPixmap::grabWindow( WId window, int x, int y, int w, int h )
{
#ifdef DEBUG_QPIXMAP
    qDebug( "QPixmap::grabWindow (%d,%d,%d,%d,%d)", ( int ) window, x, y, w, h );
#endif

    if ( w == 0 || h == 0 )
        return QPixmap();

    RECT rect;
    if ( ! GetClientRect( window, &rect ) )
        return QPixmap();

    if ( w < 0 )
        w = ( rect.right - rect.left ) - x;
    if ( h < 0 )
        h = ( rect.bottom - rect.top ) - y;

    QPixmap pm( w, h );
    pm.data->uninit = FALSE;

    HDC dest_handle = pm.handle();
    HDC src_handle = GetDC( window );
    BitBlt( dest_handle, 0, 0, w, h, src_handle, x, y, SRCCOPY );
    ReleaseDC( window, src_handle );

    return pm;
}


/*!
  Returns a copy of the pixmap that is transformed using \e matrix.

  Qt uses this function to implement rotated text on window systems
  that do not support such complex features.

  Example of how to manually draw a rotated text at (100,200) in a widget:
  \code
    char    *str = "Trolls R Qt"; // text to be drawn
    QFont    f( "Charter", 24 ); // use Charter 24pt font
    QPixmap  pm( 8, 8 );
    QPainter p;
    QRect    r;    // text bounding rectangle
    QPoint   bl;   // text baseline position

    p.begin( &pm );   // first get the bounding
    p.setFont( f );   //   text rectangle
    r = p.fontMetrics().boundingRect(str);
    bl = -r.topLeft();   // get baseline position
    p.end();

    pm.resize( r.size() );  // resize to fit the text
    pm.fill( white );   // fills pm with white
    p.begin( &pm );   // begin painting pm
    p.setFont( f );   // set the font
    p.setPen( blue );   // set blue text color
    p.drawText( bl, str );  // draw the text
    p.end();    // painting done

    QWMatrix m;    // transformation matrix
    m.rotate( -33.4 );   // rotate coordinate system
    QPixmap rp = pm.xForm( m );  // rp is rotated pixmap

    QWMatrix t = QPixmap::trueMatrix( m, pm.width(), pm.height() );
    int x, y;
    t.map( bl.x(),bl.y(), &x,&y ); // get pm's baseline pos in rp

    bitBlt( myWidget, 100-x, 200-y, // blt rp into a widget
     &rp, 0, 0, -1, -1 );
  \endcode

  This example outlines how Qt implements rotated text under X11.
  The font calculation is the most tedious part. The rotation itself is
  only 3 lines of code.

  If you want to draw rotated text, you do not have to implement all the
  code above. The code below does exactly the same thing as the example
  above, except that it uses a QPainter.

  \code
    char    *str = "Trolls R Qt"; // text to be drawn
    QFont    f( "Charter", 24 ); // use Charter 24pt font
    QPainter p;

    p.begin( myWidget );
    p.translate( 100, 200 );  // translates coord system
    p.rotate( -33.4 );   // rotates it counterclockwise
    p.setFont( f );
    p.drawText( 0, 0, str );
    p.end();
  \endcode

  \bug 2 and 4 bits pixmaps are not supported.

  \sa trueMatrix(), QWMatrix, QPainter::setWorldMatrix()
*/

QPixmap QPixmap::xForm( const QWMatrix &matrix ) const
{
#ifdef DEBUG_QPIXMAP
    qDebug( "qpixmap_win.cpp: xForm" );
#endif

    int w, h;    // size of target pixmap
    int ws, hs;    // size of source pixmap
    uchar *dptr;    // data in target pixmap
    int dbytes;   // bytes per line/bytes total
    uchar *sptr;    // data in original pixmap
    long sbpr, dbpr;    // bytes per line in original
    int bpp;     // bits per pixel

    if ( isNull() )         // this is a null pixmap
        return copy();

    ws = width();
    hs = height();

    QWMatrix mat( matrix.m11(), matrix.m12(), matrix.m21(), matrix.m22(), 0., 0. );

    if ( matrix.m12() == 0.0F && matrix.m21() == 0.0F ) {
        if ( matrix.m11() == 1.0F && matrix.m22() == 1.0F )
            return *this;           // identity matrix
        h = qRound( matrix.m22()*hs );
        w = qRound( matrix.m11()*ws );
        h = QABS( h );
        w = QABS( w );
    } else {                    // rotation or shearing
        QPointArray a( QRect(0,0,ws+1,hs+1) );
        a = mat.map( a );
        QRect r = a.boundingRect().normalize();
        w = r.width()-1;
        h = r.height()-1;
    }

    mat = trueMatrix( mat, ws, hs ); // true matrix

    bool invertible;
    mat = mat.invert( &invertible );  // invert matrix

    if ( h == 0 || w == 0 || !invertible ) { // error, return null pixmap
        QPixmap pm;
        pm.data->bitmap = data->bitmap;
        pm.data->mask = data->mask;
        return pm;
    }

    bool freeBits = qt_GetBitmapBits( this, &sptr, sbpr );
    ws = width();
    hs = height();

    QPixmap pm( w, h, depth(), optimization() );
    bool copyBits = qt_GetBitmapBits( &pm, &dptr, dbpr );
    bpp = depth();
    dbytes = dbpr * h;

    if ( bpp == 8 )
        memset( dptr, white.pixel(), dbytes );
    else
        memset( dptr, 0, dbytes );

    int xbpr;
    if ( bpp == 1 ) {
        /* We have to be word-aligned */
        int bytes = ( w + 7 ) >> 3;
        xbpr = ( bytes & 1 ) ? bytes + 1 : bytes; //WORD aligned
    } else {
        xbpr = ( ( w * bpp ) / 8 );
    }
#ifdef DEBUG_QPIXMAP

    qDebug( "qpixmap_win.cpp: xForm (bpp=%d, dptr=%X, xbpl=%d, dbpr=%d, h=%d, sptr=%X, sbpr=%d, ws=%d, hs=%d)", bpp, ( int ) dptr, xbpr, dbpr, h, ( int ) sptr, sbpr, ws, hs );
#endif

    // TODO: use QPainter::drawPixmap() for 32 bpp images like Qt4 does
    int p_inc = copyBits ? 0 : dbpr - xbpr;

    if ( !qt_xForm_helper( mat, 0, QT_XFORM_TYPE_WINDOWSPIXMAP, bpp,
                           dptr, xbpr, p_inc, h,
                           sptr, sbpr, ws, hs ) ) {
#if defined(QT_CHECK_RANGE)
        qWarning( "Qt: QPixmap::xForm: display not supported (bpp=%d)", bpp );
#endif

        QPixmap pm;
        return pm;
    }

    if ( copyBits ) {
        DeleteObject( pm.hbm() );
        pm.data->hbm_or_mcpi.hbm = CreateBitmap( w, h, 1, 1, dptr );
        delete [] dptr;
    }
    if ( freeBits )
        delete [] sptr;

    if ( depth() == 1 ) {
        if ( data->mask ) {
            if ( data->selfmask )                   // pixmap == mask
                pm.setMask( *( ( QBitmap* ) ( &pm ) ) );
            else
                pm.setMask( data->mask->xForm( matrix ) );
        }
    } else if ( data->mask && data->realAlphaBits == 0 ) {
        pm.setMask( data->mask->xForm( matrix ) );
    }
    if( bpp == 32 )
        pm.data->realAlphaBits = (uchar*)1;
    else
        pm.data->realAlphaBits = data->realAlphaBits;

    return pm;
}


/*!
  Returns TRUE if painting with this pixmap might not necessarily
  paint all pixels in its rectangular area.
*/
bool QPixmap::hasAlpha() const
{
    return data->mask || hasAlphaChannel();
}

/*!
    Returns TRUE if the pixmap has an alpha channel; otherwise it
    returns FALSE.

    NOTE: If the pixmap has a mask but not alpha channel, this
    function returns FALSE.

    \sa hasAlpha() mask()
*/
bool QPixmap::hasAlphaChannel() const
{
    return ( data->realAlphaBits != 0 );
}

// These functions are internal and used by Windows 9x only
HDC QPixmap::multiCellHandle() const
{
    return 0;
}

HBITMAP QPixmap::multiCellBitmap() const
{
    return 0;
}

int QPixmap::multiCellOffset() const
{
    return data->hbm_or_mcpi.mcpi->offset;
}

int QPixmap::allocCell()
{
    return 0;
}

void QPixmap::freeCell( bool /*= FALSE*/ )
{}

/*!
    \relates QPixmap

    Copies a block of pixels from \a src to \a dst.  The alpha channel
    and mask data (if any) is also copied from \a src.  NOTE: \a src
    is \e not alpha blended or masked when copied to \a dst.  Use
    bitBlt() or QPainter::drawPixmap() to perform alpha blending or
    masked drawing.

    \a sx, \a sy is the top-left pixel in \a src (0, 0 by default), \a
    dx, \a dy is the top-left position in \a dst and \a sw, \sh is the
    size of the copied block (all of \a src by default).

    If \a src, \a dst, \a sw or \a sh is 0 (zero), copyBlt() does
    nothing.  If \a sw or \a sh is negative, copyBlt() copies starting
    at \a sx (and respectively, \a sy) and ending at the right edge
    (and respectively, the bottom edge) of \a src.

    copyBlt() does nothing if \a src and \a dst have different depths.
*/
Q_EXPORT void copyBlt( QPixmap *dst, int dx, int dy,
                       const QPixmap *src, int sx, int sy, int sw, int sh )
{
    if ( ! dst || ! src || sw == 0 || sh == 0 || dst->depth() != src->depth() ) {
#ifdef QT_CHECK_NULL
        Q_ASSERT( dst != 0 );
        Q_ASSERT( src != 0 );
#endif

        return ;
    }

    // copy pixel data
    bitBlt( dst, dx, dy, src, sx, sy, sw, sh, Qt::CopyROP, TRUE );

    // copy mask data
    if ( src->data->mask ) {
        if ( ! dst->data->mask ) {
            dst->data->mask = new QBitmap( dst->width(), dst->height() );

            // new masks are fully opaque by default
            dst->data->mask->fill( Qt::color1 );
        }

        bitBlt( dst->data->mask, dx, dy,
                src->data->mask, sx, sy, sw, sh, Qt::CopyROP, TRUE );
    }

}


