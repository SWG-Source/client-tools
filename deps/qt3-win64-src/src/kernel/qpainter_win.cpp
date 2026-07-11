/****************************************************************************
** $Id: qpainter_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QPainter class for Windows
**
** Created : 20030119
**
** Copyright (C) 2003 Holger Schroeder
** Copyright (C) 2003, 2004 Christopher January
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2003 Ivan de Jesus Deras Tabora
** Copyright (C) 2004 Ralf Habacker
** Copyright (C) 2004 Andreas Hausladen
** Copyright (C) 2004, 2005 Christian Ehrlicher
** Copyright (C) 2004, 2005 Peter Kuemmel
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

/* some notes from Holger

qpainter-win32-notes

todo: implement custom penstyles and brushstyles.

todo: denseXpattern brushes do also draw background pixels, so
transparent does not work as it should

todo: windows has a white background, x11 has a gray one.

todo: drawing rotated text doesn't work, QPixmap::convertToImage is still
missing...

test1: draw it without any settings except pen color and see if
 the same pixels are set under win and x11.

test2: draw object with pen style NoPen

  test1 test2
moveto  +
lineto  +
drawpoint +
drawline +
drawrect + +
drawroundrect + +
drawellipse + +
drawarc  +
drawpie  + +
drawchord + +
drawlinesegments+
drawpolyline +
drawpolygon + +
drawquadbezier +
fillrect +
drawpolyinternal+ +

drawwinfocusrect seems to be not needed under windows...

drawtext works basically, have to test it per-pixel

this is from testing the x11 qt drawing functions against the windows
gdi ones:

all the following is without setting a special brush or pen style or width,
i only set different pen colors.

generally under windows, the endpoints of line drawing operations are not
painted, but fixing this would be a lot of work for all the different pen
widths, line angles and pen styles, so i leave that stuff like it is for now.
somebody should test it with the non-commercial version for windows...

on windows the drawline function doesn't draw the endpoint of something,
under x11 it does.

under win lineto doesn't draw the endpoint, under x11 it does.

now drawellipse works, the "outer" pixels are on the same bounding rectangle
under win and x11, but they set different pixels on their circle line, but
this should not be a problem...

when drawing a chord with alen 0, under windows nothing is drawn, but under
x11 a single pixel is drawn.

drawlinesegments: under win endpoints are not drawn, under x11 they are...

when you draw an ellipse with no brush and a normal pen, and then draw a
filled ellipse with nopen, under win there remain some pixels, which still
have the border color of the first ellipse and are not overdrawn by the
second ellipse. the same for drawchord, drawpie, drawroundrect.
drawpolygon doesn't draw the lower and right pixels under x11 and under
windows.

the different pen styles nopen, solidline, dashline, dotline,dasdotline and
dashdotdotline are working, but only with no pencapstyle set( so default is
used. otherwise they look different under win and x11... no problem.

*/

#include "qglobal.h"
#include "qpainter.h"
#include "qt_windows.h"

#include "qbitmap.h"
#include "qintdict.h"
#include "qlibrary.h"           /* QLibrary::resolve() */
#include "qpaintdevicemetrics.h"
#include "qpixmapcache.h"
#include "qwidget.h"
#include "qapplication_p.h"
#include "qfontengine_p.h"
#include "qtextengine_p.h"
#include "qtextlayout_p.h"

//#define DEBUG_QPAINTER

/* needed for dynamic loading ... */
static BOOL ( WINAPI * qtAlphaBlend ) (
    HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION ) = NULL;

/* some defines for shorter lines in drawPixmap  */
#define QT_RGB_0 RGB( Qt::color0.red(),Qt::color0.green(), Qt::color0.blue() )
#define QT_RGB_1 RGB( Qt::color1.red(),Qt::color1.green(), Qt::color1.blue() )
#define QT_RGB_P RGB( cpen.color().red(), cpen.color().green(), cpen.color().blue() )
#define QT_RGB_B RGB( backgroundColor().red(), backgroundColor().green(), backgroundColor().blue() )

/* The first or last point of a line isn't painted under win32. Because of this,
   we have to put a pixel by our own...*/
void qt_DrawLastPoint( HDC hdc, int x1, int y1, int x2, int y2, COLORREF color )
{
    if ( x1 > x2 ) {
        SetPixelV( hdc, x1, y1, color );
    } else if ( x1 < x2 ) {
        SetPixelV( hdc, x2, y2, color );
    } else if ( y1 > y2 ) {
        SetPixelV( hdc, x1, y1, color );
    } else if ( y1 < y2 ) {
        SetPixelV( hdc, x2, y2, color );
    }
}

// pm -> extra && extra->bg_pix && !extra->bg_pix
void qt_erase_region( QWidget *w, QPixmap *pm, QPoint ofs, const QRegion &rgn )
{
    RECT r;
    if ( !w )
        return ;
    HWND hWnd = w->winId();

    int prevRop = SetROP2( w->handle(), R2_COPYPEN );
    if ( pm && !pm->isNull() ) {
        GetClientRect( hWnd, &r );
        QRect rr ( QPoint( r.left, r.top ), QPoint( r.right, r.bottom ) );
        QPainter p( w );
        if ( ( pm->hasAlphaChannel() ) && ( qt_winver >= Qt::WV_98 ) && qtAlphaBlend ) {
            QColor e = w->eraseColor();
            // we need a proper background...
            int sw = pm->size().width();
            int sh = pm->size().height();

            QPixmap new_pm( sw, sh );
            new_pm.fill( e );
            HDC hdc = new_pm.handle();

            COLORREF oldT = SetTextColor( hdc, QT_RGB_1 );
            COLORREF oldB = SetBkColor( hdc, QT_RGB_0 );

            BLENDFUNCTION bf;
            bf.BlendOp = AC_SRC_OVER;
            bf.BlendFlags = 0;
            bf.SourceConstantAlpha = 255;
            bf.AlphaFormat = AC_SRC_ALPHA;

            qtAlphaBlend( hdc, 0, 0, sw, sh, pm->handle(), 0, 0, sw, sh, bf );
            SetTextColor( hdc, oldT );
            SetBkColor( hdc, oldB );

            // drawTiledPixmap is now fast enough :)
            p.drawTiledPixmap( rr, new_pm,
                               QPoint( r.left + ( ofs.x() % pm->width() ),
                                       r.top + ( ofs.y() % pm->height() ) ) );
        } else
            if ( pm->mask() ) {
                // we need a proper background...
                int sw = pm->size().width();
                int sh = pm->size().height();

                QPixmap new_pm( sw, sh );
                new_pm.fill( w->eraseColor() );
                HDC hdc = new_pm.handle();

                COLORREF oldT = SetTextColor( hdc, QT_RGB_1 );
                COLORREF oldB = SetBkColor( hdc, QT_RGB_0 );

                BitBlt( hdc, 0, 0, sw, sh, pm->handle(), 0, 0, SRCINVERT );
                BitBlt( hdc, 0, 0, sw, sh, pm->mask() ->handle(), 0, 0, SRCAND );
                BitBlt( hdc, 0, 0, sw, sh, pm->handle(), 0, 0, SRCINVERT );

                SetTextColor( hdc, oldT );
                SetBkColor( hdc, oldB );

                // drawTiledPixmap is now fast enough :)
                p.drawTiledPixmap( rr, new_pm,
                                   QPoint( r.left + ( ofs.x() % pm->width() ),
                                           r.top + ( ofs.y() % pm->height() ) ) );
            } else {
                // drawTiledPixmap is now fast enough :)
                p.drawTiledPixmap( rr, *pm,
                                   QPoint( r.left + ( ofs.x() % pm->width() ),
                                           r.top + ( ofs.y() % pm->height() ) ) );
            }
    } else {
        QColor bg_col = w->eraseColor();
        HBRUSH brush = CreateSolidBrush( RGB( bg_col.red(), bg_col.green(), bg_col.blue() ) );
        if ( rgn.isEmpty() ) {
            GetClientRect( hWnd, &r );
            FillRect( w->handle(), &r, brush );
        } else {
            FillRgn( w->handle(), rgn.handle(), brush );
        }
        DeleteObject( brush );
    }
    SetROP2( w->handle(), prevRop );
}

/*****************************************************************************
  Trigonometric function for QPainter

  We have implemented simple sine and cosine function that are called from
  QPainter::drawPie() and QPainter::drawChord() when drawing the outline of
  pies and chords.
  These functions are slower and less accurate than math.h sin() and cos(),
  but with still around 1/70000th sec. execution time (on a 486DX2-66) and
  8 digits accuracy, it should not be the bottleneck in drawing these shapes.
  The advantage is that you don't have to link in the math library.
 *****************************************************************************/

const double Q_PI = 3.14159265358979323846; // pi
const double Q_2PI = 6.28318530717958647693; // 2*pi
const double Q_PI2 = 1.57079632679489661923; // pi/2

#if defined(_CC_GNU_) && defined(__i386__)

inline double qcos( double a )
{
    double r;
    __asm__ (
        "fcos"
    : "=t" ( r ) : "0" ( a ) );
    return ( r );
}

inline double qsin( double a )
{
    double r;
    __asm__ (
        "fsin"
    : "=t" ( r ) : "0" ( a ) );
    return ( r );
}

double qsincos( double a, bool calcCos = FALSE )
{
    return calcCos ? qcos( a ) : qsin( a );
}

#else

double qsincos( double a, bool calcCos = FALSE )
{
    if ( calcCos )        // calculate cosine
        a -= Q_PI2;
    if ( a >= Q_2PI || a <= -Q_2PI ) {  // fix range: -2*pi < a < 2*pi
        int m = ( int ) ( a / Q_2PI );
        a -= Q_2PI * m;
    }
    if ( a < 0.0 )        // 0 <= a < 2*pi
        a += Q_2PI;
    int sign = a > Q_PI ? -1 : 1;
    if ( a >= Q_PI )
        a = Q_2PI - a;
    if ( a >= Q_PI2 )
        a = Q_PI - a;
    if ( calcCos )
        sign = -sign;
    double a2 = a * a;    // here: 0 <= a < pi/4
    double a3 = a2 * a;    // make taylor sin sum
    double a5 = a3 * a2;
    double a7 = a5 * a2;
    double a9 = a7 * a2;
    double a11 = a9 * a2;
    return ( a -a3 / 6 + a5 / 120 - a7 / 5040 + a9 / 362880 - a11 / 39916800 ) * sign;
}

inline double qsin( double a )
{
    return qsincos( a, FALSE );
}
inline double qcos( double a )
{
    return qsincos( a, TRUE );
}

#endif

/*****************************************************************************
  QPainter member functions
 *****************************************************************************/

/*!
  Internal function that initializes the painter.
*/

void QPainter::initialize()
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::initialize()" );
#endif
    qtAlphaBlend = reinterpret_cast<decltype(qtAlphaBlend)>(QLibrary::resolve( "msimg32.dll", "AlphaBlend" ));
}

/*!
  Internal function that cleans up the painter.
*/

void QPainter::cleanup()
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::cleanup()" );
#endif
    QPointArray::cleanBuffers();
}

/*!
  \internal

  Internal function that destroys up the painter.
*/

void QPainter::destroy()
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::destroy()" );
#endif

}

typedef QIntDict<QPaintDevice> QPaintDeviceDict;
static QPaintDeviceDict *pdev_dict = 0;

/*!
  Redirects all paint command for a paint device \a pdev to another
  paint device \a replacement, unless \a replacement is 0.  If \a
  replacement is 0, the redirection for \a pdev is removed.

  Mostly, you can get better results with less work by calling
  QPixmap::grabWidget() or QPixmap::grapWindow().
*/

void QPainter::init()
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::init()" );
#endif

    d = 0;
    flags = IsStartingUp;
    bg_col = white;                             // default background color
    bg_mode = TransparentMode;                  // default background mode
    rop = CopyROP;                              // default ROP
    tabstops = 0;                               // default tabbing
    tabarray = 0;
    tabarraylen = 0;
    ps_stack = 0;
    wm_stack = 0;
    pdev = 0;
    txop = txinv = 0;
    pfont = 0;
    block_ext = FALSE;
    hdc = 0;
    hpen = 0;
    hfont = 0;
    hbrush = 0;
    pixmapBrush = 0;
}


/*!
  \fn const QFont &QPainter::font() const

  Returns the currently set painter font.
  \sa setFont(), QFont
*/

/*!
  Sets a new painter font.

  This font is used by subsequent drawText() functions.  The text
  color is the same as the pen color.

  \sa font(), drawText()
*/

void QPainter::setFont( const QFont &font )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::setFont( %s )" font.family.latin1(), font.toString().latin1() );
#endif
    if ( !isActive() )
        qWarning( "QPainter::setFont: Will be reset by begin()" );

    if ( cfont.d != font.d ) {
        cfont = font;
        setf( DirtyFont );
    }
}


void QPainter::updateFont()
{
#ifdef DEBUG_QPAINTER
    qDebug( "qpainter_win.cpp: updateFont " );
#endif

    clearf( DirtyFont );
    if ( testf( ExtDev ) ) {
        if ( pfont )
            delete pfont;
        pfont = new QFont( cfont.d, pdev );
        QPDevCmdParam param[ 1 ];
        param[ 0 ].font = &cfont;
        if ( !pdev->cmd( QPaintDevice::PdcSetFont, this, param ) || !hdc )
            return ;
    }
    setf( NoCache );
    updatePen();    // force a non-cached GC
}


void QPainter::updatePen()
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::updatePen() color: red0x%08x", QT_RGB_P );
#endif

    if ( testf( ExtDev ) ) {
        QPDevCmdParam param[ 1 ];
        param[ 0 ].pen = &cpen;
        if ( !pdev->cmd( QPaintDevice::PdcSetPen, this, param ) || !hdc )
            return ;
    }

    LOGBRUSH lb;
    int ps = cpen.style(); //penstyle
    int cp; //capstyle
    int jn; //joinstyle

    lb.lbStyle = BS_SOLID;
    lb.lbColor = QT_RGB_P;
    lb.lbHatch = 0;
    switch ( ps ) {
    case NoPen:
        ps = PS_NULL;
        break;
    case SolidLine:
        ps = PS_SOLID;
        break;
    case DashLine:
        ps = PS_DASH;
        break;
    case DotLine:
        ps = PS_DOT;
        break;
    case DashDotLine:
        ps = PS_DASHDOT;
        break;
    case DashDotDotLine:
        ps = PS_DASHDOTDOT;
    }

    switch ( cpen.capStyle() ) {
    case SquareCap:
        cp = PS_ENDCAP_SQUARE;
        break;
    case RoundCap:
        cp = PS_ENDCAP_ROUND;
        break;
    case FlatCap:
    default:
        cp = PS_ENDCAP_FLAT;
        break;
    }

    switch ( cpen.joinStyle() ) {
    case BevelJoin:
        jn = PS_JOIN_BEVEL;
        break;
    case RoundJoin:
        jn = PS_JOIN_ROUND;
        break;
    case MiterJoin:
    default:
        jn = PS_JOIN_MITER;
        break;
    }

    int width = ( cpen.width() ? cpen.width() : 1 );       // width of 0 is 1 pixel (see docu)
    if ( ( hpen = ExtCreatePen( PS_GEOMETRIC | ps | cp | jn, width, &lb, 0, NULL ) ) ) {
        SetTextColor( hdc, QT_RGB_P );
        SetBkColor( hdc, QT_RGB_B );
        HPEN hPenOld = ( HPEN ) SelectObject( hdc, hpen );
        if ( hPenOld )
            DeleteObject( hPenOld );
#ifdef DEBUG_QPAINTER

    } else {
        qDebug( "ExtCreatePen failed with error %d", GetLastError() );
#endif

    }
}

void QPainter::updateBrush()
{
    // Holgi TODO the dense patterns do not support transparent drawing,
    // that is they also draw the background...
#ifdef DEBUG_QPAINTER
    qDebug( "qpainter_win.cpp: updateBrush " );
#endif

    static uchar dense1_pat[] = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static uchar dense2_pat[] = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static uchar dense3_pat[] = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static uchar dense4_pat[] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static uchar dense5_pat[] = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static uchar dense6_pat[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static uchar dense7_pat[] = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };

    static uchar hor_pat[] = {   // horizontal pattern
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static uchar ver_pat[] = {   // vertical pattern
                                 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
                                 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
                                 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
                                 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
                                 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
                                 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
    static uchar cross_pat[] = {   // cross pattern
                                   0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
                                   0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
                                   0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
                                   0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
                                   0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
                                   0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
    static uchar bdiag_pat[] = {   // backward diagonal pattern
                                   0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01,
                                   0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
                                   0x02, 0x02, 0x01, 0x01, 0x80, 0x80, 0x40, 0x40 };
    static uchar fdiag_pat[] = {   // forward diagonal pattern
                                   0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40,
                                   0x80, 0x80, 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
                                   0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01 };
    static uchar dcross_pat[] = {   // diagonal cross pattern
                                    0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14, 0x22, 0x22, 0x41, 0x41,
                                    0x80, 0x80, 0x41, 0x41, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
                                    0x22, 0x22, 0x41, 0x41, 0x80, 0x80, 0x41, 0x41 };
    static uchar *pat_tbl[] = {
                                  dense1_pat, dense2_pat, dense3_pat, dense4_pat, dense5_pat,
                                  dense6_pat, dense7_pat,
                                  hor_pat, ver_pat, cross_pat, bdiag_pat, fdiag_pat, dcross_pat };

    if ( testf( ExtDev ) ) {
        QPDevCmdParam param[ 1 ];
        param[ 0 ].brush = &cbrush;
        if ( !pdev->cmd( QPaintDevice::PdcSetBrush, this, param ) || !hdc )
            return ;
    }

    int bs = cbrush.style();
    /*bool cacheIt = !testf(ClipOn|MonoDev|NoCache) &&
     (bs == NoBrush || bs == SolidPattern) &&
     bro.x() == 0 && bro.y() == 0 && rop == CopyROP;*/

    //bool obtained = FALSE;
    //bool internclipok = hasClipping();

    HBRUSH hBrush = 0;
    LOGBRUSH lb;
    pixmapBrush = 0;
    switch ( cbrush.style() ) {
    case NoBrush:
        lb.lbStyle = BS_NULL;
        break;
    case HorPattern:
        lb.lbStyle = BS_HATCHED;
        lb.lbHatch = HS_HORIZONTAL;
        break;
    case VerPattern:
        lb.lbStyle = BS_HATCHED;
        lb.lbHatch = HS_VERTICAL;
        break;
    case CrossPattern:
        lb.lbStyle = BS_HATCHED;
        lb.lbHatch = HS_CROSS;
        break;
    case BDiagPattern:
        lb.lbStyle = BS_HATCHED;
        lb.lbHatch = HS_BDIAGONAL;
        break;
    case FDiagPattern:
        lb.lbStyle = BS_HATCHED;
        lb.lbHatch = HS_FDIAGONAL;
        break;
    case DiagCrossPattern:
        lb.lbStyle = BS_HATCHED;
        lb.lbHatch = HS_DIAGCROSS;
        break;
    case SolidPattern:
        lb.lbStyle = BS_SOLID;
        break;
    default: {
            if ( bs == CustomPattern ) {
                pixmapBrush = 1;
                hBrush = 0;
            } else {
                uchar *pat = 0;    // pattern
                int d = 0;     // defalt pattern size: d*d
                if ( bs >= Dense1Pattern && bs <= DiagCrossPattern ) {
                    pat = pat_tbl[ bs - Dense1Pattern ];
                    if ( bs <= Dense7Pattern )
                        d = 8;
                    else if ( bs <= CrossPattern )
                        d = 24;
                    else
                        d = 16;
                }

                if ( bs == CustomPattern || pat ) {
                    QPixmap * pm;
                    if ( pat ) {
                        QString key;
                        key.sprintf( "$qt-brush$%d", bs );
                        pm = QPixmapCache::find( key );
                        bool del = FALSE;
                        if ( !pm ) {   // not already in pm dict
                            pm = new QBitmap( d, d, pat, TRUE );
                            CHECK_PTR( pm );
                            del = !QPixmapCache::insert( key, pm );
                        }
                        if ( cbrush.data->pixmap )
                            delete cbrush.data->pixmap;
                        cbrush.data->pixmap = new QPixmap( *pm );
                        if ( del )
                            delete pm;
                    }
                    pm = cbrush.data->pixmap;
                    hBrush = CreatePatternBrush( pm->hbm() );
                    if ( hBrush == NULL ) {
#ifdef DEBUG_QPAINTER
                        qDebug( "qpainter_win.cpp: updateBrush failed GetLastError()=%d, %X", ( int ) GetLastError(), ( int ) hBrush );
#endif

                        return ;
                    }
                }
            }
        }
        /*case Dense1Pattern:
               lb.lbStyle = BS_PATTERN;
        hbm = CreateBitmap( 8, 8, 1, 1, (LPBYTE)dense1_pat);
        lb.lbHatch = (LONG)hbm;
        SetTextColor( hdc, RGB( cbrush.color().red(),
         cbrush.color().green(), cbrush.color().blue() ) );
        break;
           case Dense2Pattern:
               lb.lbStyle = BS_PATTERN;
        hbm = CreateBitmap( 8, 8, 1, 1, (LPBYTE)dense2_pat);
        lb.lbHatch = (LONG)hbm;
        SetTextColor( hdc, RGB( cbrush.color().red(),
         cbrush.color().green(), cbrush.color().blue() ) );
        break;
           case Dense3Pattern:
               lb.lbStyle = BS_PATTERN;
        hbm = CreateBitmap( 8, 8, 1, 1, (LPBYTE)dense3_pat);
        lb.lbHatch = (LONG)hbm;
        SetTextColor( hdc, RGB( cbrush.color().red(),
         cbrush.color().green(), cbrush.color().blue() ) );
        break;
           case Dense4Pattern:
               lb.lbStyle = BS_PATTERN;
        hbm = CreateBitmap( 8, 8, 1, 1, (LPBYTE)dense4_pat);
        lb.lbHatch = (LONG)hbm;
        SetTextColor( hdc, RGB( cbrush.color().red(),
         cbrush.color().green(), cbrush.color().blue() ) );
        break;
           case Dense5Pattern:
               lb.lbStyle = BS_PATTERN;
        hbm = CreateBitmap( 8, 8, 1, 1, (LPBYTE)dense5_pat);
        lb.lbHatch = (LONG)hbm;
        SetTextColor( hdc, RGB( cbrush.color().red(),
         cbrush.color().green(), cbrush.color().blue() ) );
        break;
           case Dense6Pattern:
               lb.lbStyle = BS_PATTERN;
        hbm = CreateBitmap( 8, 8, 1, 1, (LPBYTE)dense6_pat);
        lb.lbHatch = (LONG)hbm;
        SetTextColor( hdc, RGB( cbrush.color().red(),
         cbrush.color().green(), cbrush.color().blue() ) );
        break;
           case Dense7Pattern:
               lb.lbStyle = BS_PATTERN;
        hbm = CreateBitmap( 8, 8, 1, 1, (LPBYTE)dense7_pat);
        lb.lbHatch = (LONG)hbm;
        SetTextColor( hdc, RGB( cbrush.color().red(),
         cbrush.color().green(), cbrush.color().blue() ) );
        break;
           case CustomPattern:
           default:
        // cp = CapButt;
        break;*/
    }
    /*NoBrush will not fill shapes (default).
    SolidPattern solid (100%) fill pattern.
    Dense1Pattern 94% fill pattern.
    Dense2Pattern 88% fill pattern.
    Dense3Pattern 63% fill pattern.
    Dense4Pattern 50% fill pattern.
    Dense5Pattern 37% fill pattern.
    Dense6Pattern 12% fill pattern.
    Dense7Pattern 6% fill pattern.
    HorPattern horizontal lines pattern.
    VerPattern vertical lines pattern.
    CrossPattern crossing lines pattern.
    BDiagPattern diagonal lines (directed / ) pattern.
    FDiagPattern diagonal lines (directed \ ) pattern.
    DiagCrossPattern diagonal crossing lines pattern.
    CustomPattern set when a pixmap pattern is being used.
    */
    if ( !pixmapBrush ) {
        lb.lbColor = RGB( cbrush.color().red(), cbrush.color().green(), cbrush.color().blue() );
        if ( !hBrush )
            hBrush = CreateBrushIndirect( &lb );
    }
    HBRUSH hBrushOld = hbrush;
    //SetTextColor(hdc, RGB(cbrush.color().red(), cbrush.color().green(), cbrush.color().blue()));//(COLORREF)cbrush.color().pixel());
    SetBkColor( hdc, RGB( bg_col.red(), bg_col.green(), bg_col.blue() ) ); //bg_col.pixel() );
    if ( hBrush )
        SelectObject( hdc, hBrush );
    else
        SelectObject( hdc, GetStockObject( NULL_BRUSH ) );
    hbrush = hBrush;
    if ( hBrushOld )
        DeleteObject( hBrushOld );

    /*    if ( cacheIt ) {
     if ( gc_brush ) {
         if ( brushRef )
      release_gc( brushRef );
         else
      free_gc( dpy, gc_brush );
     }
     obtained = obtain_gc(&brushRef, &gc_brush, cbrush.color().pixel(), dpy, hd);
     if ( !obtained && !brushRef )
         gc_brush = alloc_gc( dpy, hd, FALSE );
        } else {
     if ( gc_brush ) {
         if ( brushRef ) {
      release_gc( brushRef );
      brushRef = 0;
      gc_brush = alloc_gc( dpy, hd, testf(MonoDev) );
         } else {
      internclipok = TRUE;
         }
     } else {
         gc_brush = alloc_gc( dpy, hd, testf(MonoDev), testf(UsePrivateCx));
     }
        }
    */
    /*    if ( !internclipok ) {
     if ( testf(PaintEventClipOn) ) {
         if ( brushRef &&((QGCC*)brushRef)->clip_serial < gc_cache_clip_serial ) {
      XSetRegion( dpy, gc_brush, paintEventClipRegion->handle() );
      ((QGCC*)brushRef)->clip_serial = gc_cache_clip_serial;
         } else if ( !brushRef ){
      XSetRegion( dpy, gc_brush, paintEventClipRegion->handle() );
         }
     } else if (brushRef && ((QGCC*)brushRef)->clip_serial ) {
         XSetClipMask( dpy, gc_brush, None );
         ((QGCC*)brushRef)->clip_serial = 0;
     }
        }
    */
    /*    if ( obtained )
     return;

        uchar *pat = 0;    // pattern
        int d = 0;     // defalt pattern size: d*d
        int s  = FillSolid;
        if ( bs >= Dense1Pattern && bs <= DiagCrossPattern ) {
     pat = pat_tbl[ bs-Dense1Pattern ];
     if ( bs <= Dense7Pattern )
         d = 8;
     else if ( bs <= CrossPattern )
         d = 24;
     else
         d = 16;
        }
    */
    /*    XSetLineAttributes( dpy, gc_brush, 0, LineSolid, CapButt, JoinMiter );
        XSetForeground( dpy, gc_brush, cbrush.color().pixel() );
        XSetBackground( dpy, gc_brush, bg_col.pixel() );

        if ( bs == CustomPattern || pat ) {
     QPixmap *pm;
     if ( pat ) {
         QString key;
         key.sprintf( "$qt-brush$%d", bs );
         pm = QPixmapCache::find( key );
         bool del = FALSE;
         if ( !pm ) {   // not already in pm dict
      pm = new QBitmap( d, d, pat, TRUE );
      CHECK_PTR( pm );
      del = !QPixmapCache::insert( key, pm );
         }
         if ( cbrush.data->pixmap )
      delete cbrush.data->pixmap;
         cbrush.data->pixmap = new QPixmap( *pm );
         if (del) delete pm;
     }
     pm = cbrush.data->pixmap;
     if ( pm->depth() == 1 ) {
         XSetStipple( dpy, gc_brush, pm->handle() );
         s = bg_mode == TransparentMode ? FillStippled : FillOpaqueStippled;
     } else {
         XSetTile( dpy, gc_brush, pm->handle() );
         s = FillTiled;
     }
        }
        XSetFillStyle( dpy, gc_brush, s );
    */
}


/*!
  Begins painting the paint device \a pd and returns TRUE if successful,
  or FALSE if an error occurs.

  The errors that can occur are serious problems, such as these:

  \code
    p->begin( 0 ); // impossible - paint device cannot be 0

    QPixmap pm( 0, 0 );
    p->begin( pm ); // impossible - pm.isNull();

    p->begin( myWidget );
    p2->begin( myWidget ); // impossible - only one painter at a time
  \endcode

  Note that most of the time, you can use one of the constructors
  instead of begin(), and that end() is automatically done at
  destruction.

  \warning A paint device can only be painted by one painter at a time.

  \sa end(), flush()
*/

bool QPainter::begin( const QPaintDevice *pd, bool /*unclipped*/ )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::begin( %p )", pd );
#endif

    if ( isActive() ) {    // already active painting
        qWarning( "QPainter::begin: Painter is already active."
                  "\n\tYou must end() the painter before a second begin()" );
        return FALSE;
    }
    if ( pd == 0 ) {
        qWarning( "QPainter::begin: Paint device cannot be null" );
        return FALSE;
    }

    const QWidget *copyFrom = 0;
    pdev = redirect( ( QPaintDevice* ) pd );
    if ( pdev ) {
        if ( pd->devType() == QInternal::Widget )
            copyFrom = ( const QWidget * ) pd; // copy widget settings
    } else {
        pdev = ( QPaintDevice * ) pd;
    }

    if ( pdev->isExtDev() && pdev->paintingActive() ) {
        // somebody else is already painting
        qWarning( "QPainter::begin: Another QPainter is already painting "
                  "this device;\n\tAn extended paint device can only be "
                  "painted by one QPainter at a time." );
        return FALSE;
    }

    bool reinit = !testf( IsStartingUp ); // 2nd or 3rd etc. time called
    flags = IsActive | DirtyFont;  // init flags
    int dt = pdev->devType();   // get the device type

    if ( ( pdev->devFlags & QInternal::ExternalDevice ) != 0 )
        setf( ExtDev );
    else if ( dt == QInternal::Pixmap )         // device is a pixmap
        ( ( QPixmap* ) pdev ) ->detach();       // will modify it

    if ( testf( ExtDev ) ) {   // external device
        if ( !pdev->cmd( QPaintDevice::PdcBegin, this, 0 ) ) {
            // could not begin painting
            if ( reinit )
                clearf( IsActive | DirtyFont );
            else
                flags = IsStartingUp;
            pdev = 0;
            return FALSE;
        }
        if ( tabstops )        // update tabstops for device
            setTabStops( tabstops );
        if ( tabarray )        // update tabarray for device
            setTabArray( tabarray );
    }

    pdev->painters++;    // also tell paint device
    bro = curPt = QPoint( 0, 0 );
    if ( reinit ) {
        bg_mode = TransparentMode;  // default background mode
        rop = CopyROP;    // default ROP
        wxmat.reset();    // reset world xform matrix
        txop = txinv = 0;
        if ( dt != QInternal::Widget ) {
            QFont defaultFont;   // default drawing tools
            QPen defaultPen;
            QBrush defaultBrush;
            cfont = defaultFont;  // set these drawing tools
            cpen = defaultPen;
            cbrush = defaultBrush;
            bg_col = white;   // default background color
        }
    }
    wx = wy = vx = vy = 0;   // default view origins

    if ( dt == QInternal::Widget ) {   // device is a widget
        QWidget *w = static_cast<QWidget*>(pdev);
        cfont = w->font();   // use widget font
        cpen = QPen( w->foregroundColor() ); // use widget fg color
        if ( reinit ) {
            QBrush defaultBrush;
            cbrush = defaultBrush;
        }
        bg_col = w->backgroundColor();  // use widget bg color
        ww = vw = w->width();   // default view size
        wh = vh = w->height();
        if ( w->testWFlags( WPaintUnclipped ) ) { // paint direct on device
            setf( NoCache );
            setf( UsePrivateCx );
            updatePen();
            updateBrush();
            // Holgi TODO what does this do?
            /*     XSetSubwindowMode( dpy, gc, IncludeInferiors );
                 XSetSubwindowMode( dpy, gc_brush, IncludeInferiors );
            */
        }
        if( w->testWState( WState_InPaintEvent ) ) {
            hdc = pdev->handle();
        } else {
            hdc = GetDC( w->isDesktop() ? 0 : w->winId() );
        }
    } else if ( dt == QInternal::Pixmap ) {  // device is a pixmap
        QPixmap *pm = static_cast<QPixmap*>(pdev);
        if ( pm->isNull() ) {
            qWarning( "\nQPainter::begin: Cannot paint null pixmap\n" );
            end();
            return FALSE;
        }
        bool mono = pm->depth() == 1;  // monochrome bitmap
        if ( mono ) {
            setf( MonoDev );
            bg_col = color0;
            cpen.setColor( color1 );
        }
        ww = vw = pm->width();   // default view size
        wh = vh = pm->height();
        hdc = pm->handle();
    } else if ( testf( ExtDev ) ) {  // external device
        ww = vw = pdev->metric( QPaintDeviceMetrics::PdmWidth );
        wh = vh = pdev->metric( QPaintDeviceMetrics::PdmHeight );
        hdc = pdev->handle();
    } else {
        hdc = pdev->handle();
    }
    if ( ww == 0 )
        ww = wh = vw = vh = 1024;
    if ( copyFrom ) {    // copy redirected widget
        cfont = copyFrom->font();
        cpen = QPen( copyFrom->foregroundColor() );
        bg_col = copyFrom->backgroundColor();
    }
    if ( testf( ExtDev ) ) {   // external device
        setBackgroundColor( bg_col );  // default background color
        setBackgroundMode( TransparentMode ); // default background mode
    }
    setRasterOp( CopyROP );   // we need this here ( sync with 321nc )
    updateBrush();
    updatePen();

    if ( hdc )
        SelectClipRgn( hdc, 0 );

    // Holgi TODO place somewhere else?
    setBackgroundMode( TransparentMode );
    return TRUE;
}

/*!
  Ends painting.  Any resources used while painting are released.

  Note that while you mostly don't need to call end(), the destructor
  will do it, there is at least one common case, namely double
  buffering.

  \code
    QPainter p( myPixmap, this )
    // ...
    p.end(); // stops drawing on myPixmap
    p.begin( this );
    p.drawPixmap( myPixmap );
  \endcode

  Since you can't draw a QPixmap while it is being painted, it is
  necessary to close the active painter.

  \sa begin(), isActive()
*/

bool QPainter::end()        // end painting
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::end() hdc: 0x%08p", hdc );
#endif

    if ( !isActive() ) {
       qWarning( "QPainter::end: Missing begin() or begin() failed" );
        return FALSE;
    }
    killPStack();

    if ( testf( ExtDev ) )
        pdev->cmd( QPaintDevice::PdcEnd, this, 0 );

    if ( hpen ) {
#ifdef DEBUG_QPAINTER
        qDebug( "QPainter::end() Deleting hpen %p", hpen );
#endif

        hpen = ( HPEN ) SelectObject( hdc, GetStockObject( WHITE_PEN ) );
        if( hpen )
            DeleteObject( hpen );
        hpen = 0;
    }
    if ( hbrush ) {
#ifdef DEBUG_QPAINTER
        qDebug( "QPainter::end() Deleting hbrush %p", hbrush );
#endif

        hbrush = ( HBRUSH ) SelectObject( hdc, GetStockObject( NULL_BRUSH ) );
        if( hbrush )
            DeleteObject( hbrush );
        hbrush = 0;
    }
    if( hdc )
        SelectClipRgn( hdc, NULL );
    int dt = pdev->devType();   // get the device type
    if ( dt == QInternal::Widget ) {   // device is a widget
        QWidget *w = static_cast<QWidget*>(pdev);
        if( !w->testWState( WState_InPaintEvent ) )
            ReleaseDC( w->isDesktop() ? 0 : w->winId(), hdc );
    }

    flags = 0;
    pdev->painters--;
    pdev = 0;
    hdc = 0;
    return TRUE;
}

/*!
    Flushes any buffered drawing operations inside the region \a
    region using clipping mode \a cm.

    The flush may update the whole device if the platform does not
    support flushing to a specified region.

    \sa flush() CoordinateMode
*/
void QPainter::flush( const QRegion &, CoordinateMode )
{
    flush();
}


/*!
    \overload

    Flushes any buffered drawing operations.
*/
void QPainter::flush()
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::flush()" );
#endif
    if ( isActive() )
        GdiFlush();
}


/*!
  Sets the background color of the painter to \a c.

  The background color is the color that is filled in when drawing
  opaque text, stippled lines and bitmaps.  The background color has
  no effect in transparent background mode (which is the default).

  \sa backgroundColor() setBackgroundMode() BackgroundMode
*/

void QPainter::setBackgroundColor( const QColor &c )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::setBackgroundColor( 0x%08x )", c.rgb() );
#endif

    if ( !isActive() ) {
        qWarning( "QPainter::setBackgroundColor: Call begin() first" );
        return;
    }
    bg_col = c;
    if ( testf( ExtDev ) ) {
        QPDevCmdParam param[ 1 ];
        param[ 0 ].color = &bg_col;
        if ( !pdev->cmd( QPaintDevice::PdcSetBkColor, this, param ) || !hdc )
            return;
    }
}

/*!
  Sets the background mode of the painter to \a m, which must be one
  of \c TransparentMode (the default) and \c OpaqueMode.

  Transparent mode draws stippled lines and text without setting the
  background pixels. Opaque mode fills these space with the current
  background color.

  Note that in order to draw a bitmap or pixmap transparently, you must use
  QPixmap::setMask().

  \sa backgroundMode(), setBackgroundColor()
*/
void QPainter::setBackgroundMode( BGMode m )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::setBackgroundMode( %d )", m );
#endif

    if ( !isActive() ) {
        qWarning( "QPainter::setBackgroundMode: Call begin() first" );
        return;
    }
    if ( m != TransparentMode && m != OpaqueMode ) {
        qWarning( "QPainter::setBackgroundMode: Invalid mode" );
        return;
    }
    bg_mode = m;
    if ( testf( ExtDev ) ) {
        QPDevCmdParam param[ 1 ];
        param[ 0 ].ival = m;
        if ( !pdev->cmd( QPaintDevice::PdcSetBkMode, this, param ) || !hdc )
            return ;
    }
    SetBkMode( hdc, m == OpaqueMode ? OPAQUE : TRANSPARENT );
}

static const int ropCodes[] =
    {                     // ROP translation table
        R2_COPYPEN,          // CopyROP
        R2_MERGEPEN,         // OrROP
        R2_XORPEN,           // XorROP
        R2_MASKNOTPEN,       // NotAndROP EraseROP
        R2_NOTCOPYPEN,       // NotCopyROP
        R2_MERGENOTPEN,      // NotOrROP
        R2_NOTXORPEN,        // NotXorROP
        R2_MASKPEN,          // AndROP
        R2_NOT,              // NotROP
        R2_BLACK,            // ClearROP
        R2_WHITE,            // SetROP
        R2_NOP,              // NopROP
        R2_MASKPENNOT,       // AndNotROP
        R2_MERGEPENNOT,      // OrNotROP
        R2_NOTMASKPEN,       // NandROP
        R2_NOTMERGEPEN      // NorROP
    };

// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/gdi/pantdraw_6n77.asp
static const int rop3Codes[] =
    {                   // ROP translation table
        SRCCOPY,        // CopyROP
        SRCPAINT,       // OrROP
        SRCINVERT,      // XorROP
        SRCERASE,       // NotAndROP EraseROP   
        NOTSRCCOPY,     // NotCopyROP
        MERGEPAINT,     // NotOrROP
        0x00990066,     // NotXorROP    // (NOT src) XOR dst -> DSnx, but it's DSxn - it's the same :)
        SRCAND,         // AndROP
        DSTINVERT,      // NotROP
        BLACKNESS,      // ClearROP
        WHITENESS,      // SetROP
        0x00AA0029,     // NopROP       // D
        0x00220326,     // AndNotROP    // src AND (NOT dst) -> DSna
        0x00DD0228,     // OrNotROP     // src OR (NOT dst)  -> SDno
        0x007700E6,     // NandROP      // NOT (src AND dst) -> DSan
        NOTSRCERASE     // NorROP
    };

/*!
  Sets the raster operation to \a r.  The default is \c CopyROP.
  \sa rasterOp()
*/

void QPainter::setRasterOp( RasterOp r )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::setRasterOp( RasterOp %d )", r );
#endif

    if ( !isActive() ) {
        qWarning( "QPainter::setRasterOp: Call begin() first" );
        return;
    }
    if ( ( uint ) r > LastROP ) {
        qWarning( "QPainter::setRasterOp: Invalid ROP code" );
        return;
    }
    rop = r;
    if ( testf( ExtDev ) ) {
        QPDevCmdParam param[ 1 ];
        param[ 0 ].ival = r;
        if ( !pdev->cmd( QPaintDevice::PdcSetROP, this, param ) || !hdc )
            return ;
    }

    SetROP2( hdc, ropCodes[ rop ] );
}

/*!
  Sets the brush origin to \a (x,y).

  The brush origin specifies the (0,0) coordinate of the painter's
  brush.  This setting only applies to pattern brushes and pixmap
  brushes.

  \sa brushOrigin()
*/
void QPainter::setBrushOrigin( int x, int y )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::setBrushOrigin(%d, %d)", x, y );
#endif

    if ( !isActive() ) {
        qWarning( "QPainter::setBrushOrigin: Call begin() first" );
        return;
    }
    bro = QPoint( x, y );
    if ( testf( ExtDev ) ) {
        QPDevCmdParam param[ 1 ];
        param[ 0 ].point = &bro;
        if ( !pdev->cmd( QPaintDevice::PdcSetBrushOrigin, this, param ) ||
                !hdc )
            return ;
    }
    // Holgi TODO if before call or call before if?
    SetBrushOrgEx( hdc, x, y, NULL );
    //    if ( brushRef )
    updateBrush();    // get non-cached brush GC
    //    XSetTSOrigin( dpy, gc_brush, x, y );
}


/*!
  Enables clipping if \a enable is TRUE, or disables clipping if \a enable
  is FALSE.
  \sa hasClipping(), setClipRect(), setClipRegion()
*/
void QPainter::setClipping( bool enable )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::setClipping ( %d )", enable );
#endif

    if ( !isActive() ) {
        qWarning( "QPainter::setClipping: Will be reset by begin()" );
        return;
    }

    if ( !isActive() || enable == testf( ClipOn ) )
        return ;

    setf( ClipOn, enable );
    if ( testf( ExtDev ) ) {
        QPDevCmdParam param[ 1 ];
        param[ 0 ].ival = enable;
        if ( !pdev->cmd( QPaintDevice::PdcSetClip, this, param ) || !hdc )
            return ;
    }
    if ( enable ) {
        updatePen();
        updateBrush();
        SelectClipRgn( hdc, crgn.handle() );
        //x11SetClipRegion( dpy, gc, rendhd, rgn, gc_brush );
    } else {
        SelectClipRgn( hdc, NULL );
    }
}


/*!
  \overload void QPainter::setClipRect( const QRect &r )

  If the rectangle is invalid, the rectangle will be normalized() before the clipping region is set. This semantics will change in Qt-3
  and an invalid rectangle will clip the painter to an empty rectangle.

  \sa QRect::isValid() QRect::normalize()
*/
void QPainter::setClipRect( const QRect &r, CoordinateMode m )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::setClipRect( %d/%d-%d/%d, %d) ", r.left(), r.top(), r.right(), r.bottom(), m );
#endif

    setClipRegion( QRegion( r ), m );
}

/*!
  Sets the clip region to \a rgn and enables clipping.

  Note that the clip region is given in physical device coordinates and
  \e not subject to any \link coordsys.html coordinate
  transformation.\endlink

  \sa setClipRect(), clipRegion(), setClipping()
*/
void QPainter::setClipRegion( const QRegion &rgn, CoordinateMode m )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::setClipRegion( %p, %d)", rgn.handle(), m );
#endif
    if ( !isActive() )
        qWarning( "QPainter::setClipRegion: Will be reset by begin()" );

    if ( m == CoordDevice )
        crgn = rgn;
    else
        crgn = xmat * rgn;

    if ( testf( ExtDev ) ) {
        QPDevCmdParam param[ 1 ];
        param[ 0 ].rgn = &crgn;
        if ( !pdev->cmd( QPaintDevice::PdcSetClipRegion, this, param ) )
            return ; // device cannot clip
    }
    clearf( ClipOn );    // be sure to update clip rgn
    setClipping( TRUE );
}


/*!
  Internal function for drawing a polygon.
*/

void QPainter::drawPolyInternal( const QPointArray &a, bool /*close*/ )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::drawPolyInternal()" );
#endif

    if ( a.size() < 2 )
        return ;

    int npoints;
    npoints = a.size();
#if defined(Q_CC_MSVC) || defined(Q_CC_BOR)

    POINT *mypoints = new POINT[ npoints ];
#else

    POINT mypoints[ npoints ];
#endif

    int i;
    for ( i = 0; i < npoints ; i++ ) {
        mypoints[ i ].x = a.point( i ).x();
        mypoints[ i ].y = a.point( i ).y();
    }
    Polygon( hdc, mypoints, npoints );
#if defined(Q_CC_MSVC) || defined(Q_CC_BOR)

    delete [] mypoints;
#endif
}


/*!
  Draws/plots a single point at \a (x,y) using the current pen.

  \sa QPen
*/
void QPainter::drawPoint( int x, int y )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::drawPoint( %d, %d)", x, y );
#endif

    if ( !isActive() )
        return ;
    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            QPDevCmdParam param[ 1 ];
            QPoint p( x, y );
            param[ 0 ].point = &p;
            if ( !pdev->cmd( QPaintDevice::PdcDrawPoint, this, param ) ||
                    !hdc )
                return ;
        }
        map( x, y, &x, &y );
    }
    if ( cpen.style() != NoPen ) {
        SetPixelV( hdc, x, y, QT_RGB_P );
    }
}


/*!
  Draws/plots an array of points using the current pen.

  If \a index is non-zero (the default is zero) only points from \a
  index are drawn.  If \a npoints is negative (the default) the rest
  of the points from \a index are drawn.  If is is zero or greater, \a
  npoints points are drawn.
*/
void QPainter::drawPoints( const QPointArray& a, int index, int npoints )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::drawPoints()" );
#endif

    if ( npoints < 0 )
        npoints = a.size() - index;
    if ( index + npoints > ( int ) a.size() )
        npoints = a.size() - index;
    if ( !isActive() || npoints < 1 || index < 0 )
        return ;
    QPointArray pa = a;
    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            QPDevCmdParam param[ 1 ];
            for ( int i = 0; i < npoints; i++ ) {
                QPoint p( pa[ index + i ].x(), pa[ index + i ].y() );
                param[ 0 ].point = &p;
                if ( !pdev->cmd( QPaintDevice::PdcDrawPoint, this, param ) )
                    return ;
            }
            if ( !hdc )
                return ;
        }
        if ( txop != TxNone ) {
            pa = xForm( a, index, npoints );
            if ( pa.size() != a.size() ) {
                index = 0;
                npoints = pa.size();
            }
        }
    }
    // Holgi TODO draw better points
    if ( cpen.style() != NoPen ) {
        for ( int i = 0; i < npoints; i++ ) {
            SetPixelV( hdc, pa[ index + i ].x(), pa[ index + i ].y(),
                       QT_RGB_P );
        }
        MoveToEx( hdc, curPt.x(), curPt.y(), NULL ); //restore old position
    }
}


/*!
  Sets the current pen position to \a (x,y)
  \sa lineTo(), pos()
*/
void QPainter::moveTo( int x, int y )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::moveTo( %d, %d )", x, y );
#endif

    if ( !isActive() )
        return ;
    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            QPDevCmdParam param[ 1 ];
            QPoint p( x, y );
            param[ 0 ].point = &p;
            if ( !pdev->cmd( QPaintDevice::PdcMoveTo, this, param ) || !hdc )
                return ;
        }
        map( x, y, &x, &y );
    }
    MoveToEx( hdc, x, y, NULL );
    curPt = QPoint( x, y );
}

/*!
  Draws a line from the current pen position to \a (x,y) and sets \a
  (x,y) to be the new current pen position.

  \sa QPen moveTo(), drawLine(), pos()
*/

void QPainter::lineTo( int x2, int y2 )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::lineTo( %d, %d )", x2, y2 );
#endif

    if ( !isActive() )
        return ;
    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            QPDevCmdParam param[ 1 ];
            QPoint p( x2, y2 );
            param[ 0 ].point = &p;
            if ( !pdev->cmd( QPaintDevice::PdcLineTo, this, param ) || !hdc )
                return ;
        }
        map( x2, y2, &x2, &y2 );
    }

    int x1 = curPt.x();
    int y1 = curPt.y();
    curPt = QPoint( x2, y2 );

    if ( cpen.style() != NoPen ) {
        LineTo( hdc, x2, y2 );
        qt_DrawLastPoint( hdc, x1, y1, x2, y2, QT_RGB_P );
#ifdef DEBUG_QPAINTER

        qDebug ( "hdc = %x", hdc );
        qDebug ( "x1=%d, y1=%d, x2=%d, y2=%d", x1, y1, x2, y2 );
#endif

    }
}

/*!
  Draws a line from \a (x1,y2) to \a (x2,y2) and sets \a (x2,y2) to be
  the new current pen position.

  \sa QPen
*/
void QPainter::drawLine( int x1, int y1, int x2, int y2 )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::drawLine( %d, %d, %d, %d )", x1, y1, x2, y2 );
#endif

    if ( !isActive() )
        return ;
    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            QPDevCmdParam param[ 2 ];
            QPoint p1( x1, y1 ), p2( x2, y2 );
            param[ 0 ].point = &p1;
            param[ 1 ].point = &p2;
            if ( !pdev->cmd( QPaintDevice::PdcDrawLine, this, param ) || !hdc )
                return ;
        }
        map( x1, y1, &x1, &y1 );
        map( x2, y2, &x2, &y2 );
    }

    curPt = QPoint( x2, y2 );
    if ( cpen.style() != NoPen ) {
        MoveToEx( hdc, x1, y1, NULL );
        LineTo( hdc, x2, y2 );
        qt_DrawLastPoint( hdc, x1, y1, x2, y2, QT_RGB_P );
#ifdef DEBUG_QPAINTER

        qDebug ( "hdc = %x", hdc );
        qDebug ( "x1=%d, y1=%d, x2=%d, y2=%d", x1, y1, x2, y2 );
#endif

    }
}


/*!
  Draws a rectangle with upper left corner at \a (x,y) and with
  width \a w and height \a h.

  \sa QPen, drawRoundRect()
*/
void QPainter::drawRect( int x, int y, int w, int h )
{
#ifdef DEBUG_QPAINTER
    qDebug( "qQPainter::drawRect( x: %d, y: %d, w: %d, h: %d )", x, y, w, h );
#endif

    if ( !isActive() )
        return ;
    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            QPDevCmdParam param[ 1 ];
            QRect r( x, y, w, h );
            param[ 0 ].rect = &r;
            if ( !pdev->cmd( QPaintDevice::PdcDrawRect, this, param ) || !hdc )
                return ;
        }
        if ( txop == TxRotShear ) {  // rotate/shear polygon
            QPointArray a( QRect( x, y, w, h ), TRUE );
            drawPolyInternal( xForm( a ) );
            return ;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return ;
        fix_neg_rect( &x, &y, &w, &h );
    }

    if ( cbrush.style() != NoBrush ) {
        QPixmap * pm = 0;
        if ( pixmapBrush ) {
            pm = cbrush.data->pixmap;
            if ( pm && !pm->isNull() ) {
                //printf("qpainter_win.cpp: drawRect Drawing pixmap brush part 2, x=%d, y=%d, bro.x=%d, bro.y=%d\n", x, y, bro.x(), bro.y());
                //save the clip
                bool clipon = testf( ClipOn );
                QRegion clip = crgn;

                //create the region
                QRegion newclip( QRect( x, y, w, h ) );
                if ( clipon && !clip.isNull() )
                    newclip &= clip;
                setClipRegion( newclip );

                //turn off translation flags
                uint save_flags = flags;
                flags = IsActive | ClipOn;

                //draw the brush
                drawTiledPixmap( x, y, w, h, *pm, x - bro.x(), y - bro.y() );

                //restore translation flags
                flags = save_flags;

                //restore the clip
                if ( clipon )
                    setClipRegion( clip );
                else
                    setClipping( FALSE );
                //printf("\tqpainter_win.cpp: drawRect Drawing pixmap brush part 2 ending ...\n");
            }
        } else {
            //printf("qpainter_win.cpp: drawRect Drawing NON-pixmap brush\n");
            COLORREF oldColor = SetTextColor( hdc, RGB( cbrush.color().red(), cbrush.color().green(), cbrush.color().blue() ) );
            RECT rc = {x, y, x + w, y + h};
            if ( cpen.style() == NoPen ) {
                FillRect( hdc, &rc, hbrush );
                //XFillRectangle( dpy, hd, gc_brush, x, y, w, h );
                return ;
            }
            int lw = cpen.width();
            int lw2 = ( lw + 1 ) / 2;
            rc.left += lw2;
            rc.top += lw2;
            rc.right -= lw;
            rc.bottom -= lw;
            if ( w > lw && h > lw )
                FillRect( hdc, &rc, hbrush );
            //XFillRectangle( dpy, hd, gc_brush, x+lw2, y+lw2, w-lw-1, h-lw-1 );
            SetTextColor( hdc, oldColor );
        }
    }
    if ( cpen.style() != NoPen ) {
        int oldBkMode = SetBkMode( hdc, TRANSPARENT );
        Rectangle( hdc, x, y, x + w, y + h );
        SetBkMode( hdc, oldBkMode );
        //XDrawRectangle( dpy, hd, gc, x, y, w-1, h-1 );
    }

    /*if ( cpen.style() != NoPen )
    Rectangle( hdc, x, y, x+w, y+h );
    else
    Rectangle( hdc, x, y, x+w+1, y+h+1 );*/
}

/*!
  Draws a Windows focus rectangle with upper left corner at \a (x,y) and with
  width \a w and height \a h.

  This function draws a stippled XOR rectangle that is used to indicate
  keyboard focus (when QApplication::style() is \c WindowStyle).

  \warning This function draws nothing if the coordinate system has been
  \link rotate() rotated\endlink or \link shear() sheared\endlink.

  \sa drawRect(), QApplication::style()
*/
void QPainter::drawWinFocusRect( int x, int y, int w, int h )
{
    drawWinFocusRect( x, y, w, h, color0 );
}

/*!
  Draws a Windows focus rectangle with upper left corner at \a (x,y) and with
  width \a w and height \a h using a pen color that contrasts with \a bgColor.

  This function draws a stippled rectangle (XOR is not used) that is
  used to indicate keyboard focus (when the QApplication::style() is
  \c WindowStyle).

  The pen color used to draw the rectangle is either white or black
  depending on the color of \a bgColor (see QColor::gray()).

  \warning This function draws nothing if the coordinate system has been
  \link rotate() rotated\endlink or \link shear() sheared\endlink.

  \sa drawRect(), QApplication::style()
*/
void QPainter::drawWinFocusRect( int x, int y, int w, int h,
                                 const QColor &bgColor )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::drawWinFocusRect( %d, %d, %d, %d, rgb: %02x/%02x/%02x )",
            x, y, w, h, bgColor.red(), bgColor.green(), bgColor.blue() );
#endif

    if ( !isActive() || txop == TxRotShear )
        return ;

    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            QPDevCmdParam param[ 1 ];
            QRect r( x, y, w, h );
            param[ 0 ].rect = &r;
            if ( !pdev->cmd( QPaintDevice::PdcDrawRect, this, param ) || !hdc ) {
                return ;
            }
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return ;
        fix_neg_rect( &x, &y, &w, &h );
    }

    SetBkColor( hdc, RGB( bg_col.red(), bg_col.green(), bg_col.blue() ) );

    RECT myrect;
    myrect.left = x;
    myrect.top = y;
    myrect.right = x + w;
    myrect.bottom = y + h;

    DrawFocusRect( hdc, &myrect );
}

/*! \overload void QPainter::drawRoundRect( int x, int y, int w, int h )

  As the main version of the function, but with the roundness
  arguments fixed at 25.
*/


/*! \overload void QPainter::drawRoundRect( const QRect & )

  As the main version of the function, but with the roundness
  arguments fixed at 25.
*/


/*!
  Draws a rectangle with round corners at \a (x,y), with width \a w
  and height \a h.

  The \a xRnd and \a yRnd arguments specify how rounded the corners
  should be.  0 is angled corners, 99 is maximum roundedness.

  The width and height include all of the drawn lines.

  \sa drawRect(), QPen
*/

void QPainter::drawRoundRect( int x, int y, int w, int h, int xRnd, int yRnd )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::drawRoundRect( %d,%d, %d,%d, %d, %d), x, y, w, h, xRnd, yRnd" );
#endif

    if ( !isActive() )
        return ;
    if ( xRnd <= 0 || yRnd <= 0 ) {
        drawRect( x, y, w, h );   // draw normal rectangle
        return ;
    }
    if ( xRnd >= 100 )        // fix ranges
        xRnd = 99;
    if ( yRnd >= 100 )
        yRnd = 99;
    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            QPDevCmdParam param[ 3 ];
            QRect r( x, y, w, h );
            param[ 0 ].rect = &r;
            param[ 1 ].ival = xRnd;
            param[ 2 ].ival = yRnd;
            if ( !pdev->cmd( QPaintDevice::PdcDrawRoundRect, this, param ) ||
                    !hdc )
                return ;
        }
        if ( txop == TxRotShear ) {  // rotate/shear polygon
            if ( w <= 0 || h <= 0 )
                fix_neg_rect( &x, &y, &w, &h );
            w--;
            h--;
            int rxx = w * xRnd / 200;
            int ryy = h * yRnd / 200;
            // were there overflows?
            if ( rxx < 0 )
                rxx = w / 200 * xRnd;
            if ( ryy < 0 )
                ryy = h / 200 * yRnd;
            int rxx2 = 2 * rxx;
            int ryy2 = 2 * ryy;
            QPointArray a[ 4 ];
            a[ 0 ].makeArc( x, y, rxx2, ryy2, 1 * 16 * 90, 16 * 90, xmat );
            a[ 1 ].makeArc( x, y + h - ryy2, rxx2, ryy2, 2 * 16 * 90, 16 * 90, xmat );
            a[ 2 ].makeArc( x + w - rxx2, y + h - ryy2, rxx2, ryy2, 3 * 16 * 90, 16 * 90, xmat );
            a[ 3 ].makeArc( x + w - rxx2, y, rxx2, ryy2, 0 * 16 * 90, 16 * 90, xmat );
            // ### is there a better way to join QPointArrays?
            QPointArray aa;
            aa.resize( a[ 0 ].size() + a[ 1 ].size() + a[ 2 ].size() + a[ 3 ].size() );
            uint j = 0;
            for ( int k = 0; k < 4; k++ ) {
                for ( uint i = 0; i < a[ k ].size(); i++ ) {
                    aa.setPoint( j, a[ k ].point( i ) );
                    j++;
                }
            }
            drawPolyInternal( aa );
            return ;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return ;
        fix_neg_rect( &x, &y, &w, &h );
    }
    int rx = ( w * xRnd ) / 200;
    int ry = ( h * yRnd ) / 200;
    int rx2 = 2 * rx;
    int ry2 = 2 * ry;
    if ( cpen.style() != NoPen )
        RoundRect( hdc, x, y, x + w, y + h, rx2, ry2 );
    else
        RoundRect( hdc, x, y, x + w + 1, y + h + 1, rx2, ry2 );
}

/*!
  Draws an ellipse with center at \a (x+w/2,y+h/2) and size \a (w,h).
*/
void QPainter::drawEllipse( int x, int y, int w, int h )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::drawEllipse( %d, %d, %d, %d )" x, y, w, h );
#endif

    if ( !isActive() )
        return ;
    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            QPDevCmdParam param[ 1 ];
            QRect r( x, y, w, h );
            param[ 0 ].rect = &r;
            if ( !pdev->cmd( QPaintDevice::PdcDrawEllipse, this, param ) ||
                    !hdc )
                return ;
        }
        if ( txop == TxRotShear ) {  // rotate/shear polygon
            QPointArray a;
            a.makeArc( x, y, w, h, 0, 360 * 16, xmat );
            drawPolyInternal( a );
            return ;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    //    w--;
    //    h--;
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return ;
        fix_neg_rect( &x, &y, &w, &h );
    }
    /*    if ( cbrush.style() != NoBrush ) {  // draw filled ellipse
     XFillArc( dpy, hd, gc_brush, x, y, w, h, 0, 360*64 );
     if ( cpen.style() == NoPen ) {
         XDrawArc( dpy, hd, gc_brush, x, y, w, h, 0, 360*64 );
         return;
     }
        }
    */
    if ( cpen.style() != NoPen )      // draw outline
        Ellipse( hdc, x, y, x + w, y + h ); //was one pixel too small before...
    else
        Ellipse( hdc, x, y, x + w + 1, y + h + 1 ); //was one pixel too small before...
}


/*!
  Draws an arc defined by the rectangle \a (x,y,w,h), the start
  angle \a a and the arc length \a alen.

  The angles \a a and \a alen are 1/16th of a degree, i.e. a full
  circle equals 5760 (16*360). Positive values of \a a and \a alen mean
  counter-clockwise while negative values mean clockwise direction.
  Zero degrees is at the 3'o clock position.

  Example:
  \code
    QPainter p( myWidget );
    p.drawArc( 10,10, 70,100, 100*16, 160*16 ); // draws a "(" arc
  \endcode

  \sa drawPie(), drawChord()
*/
void QPainter::drawArc( int x, int y, int w, int h, int a, int alen )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::drawArc( %d,%d, %d,%d, %d, %d), x, y, w, h, a, aLen" );
#endif

    if ( !isActive() )
        return ;
    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            QPDevCmdParam param[ 3 ];
            QRect r( x, y, w, h );
            param[ 0 ].rect = &r;
            param[ 1 ].ival = a;
            param[ 2 ].ival = alen;
            if ( !pdev->cmd( QPaintDevice::PdcDrawArc, this, param ) ||
                    !hdc )
                return ;
        }
        if ( txop == TxRotShear ) {  // rotate/shear
            QPointArray pa;
            pa.makeArc( x, y, w, h, a, alen, xmat ); // arc polyline
            drawPolyInternal( pa, FALSE );
            return ;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    /*    w--;
        h--;
    */
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return ;
        fix_neg_rect( &x, &y, &w, &h );
    }
    if ( cpen.style() != NoPen ) {   // draw pie outline
        double w2 = 0.5 * w;   // with, height in ellipsis
        double h2 = 0.5 * h;
        double xc = ( double ) x + w2;
        double yc = ( double ) y + h2;
        double ra1 = Q_PI / 2880.0 * a;  // convert a,alen to radians
        double ra2 = ra1 + Q_PI / 2880.0 * alen;
        //int xic = qRound(xc);
        //int yic = qRound(yc);

        if ( alen == 0 )
            return ;
        if ( alen > 0 ) {
            SetArcDirection( hdc, AD_COUNTERCLOCKWISE );
        } else {
            SetArcDirection( hdc, AD_CLOCKWISE );
        }
        Arc( hdc, x, y, x + w, y + h, qRound( xc + qcos( ra1 ) * w2 ), qRound( yc - qsin( ra1 ) * h2 ),
             qRound( xc + qcos( ra2 ) * w2 ), qRound( yc - qsin( ra2 ) * h2 ) );
    }
    //    if ( cpen.style() != NoPen )
    // XDrawArc( dpy, hd, gc, x, y, w, h, a*4, alen*4 );
}


/*!
  Draws a pie defined by the rectangle \a (x,y,w,h), the start
  angle \a a and the arc length \a alen.

  The pie is filled with the current brush().

  The angles \a a and \a alen are 1/16th of a degree, i.e. a full
  circle equals 5760 (16*360). Positive values of \a a and \a alen mean
  counter-clockwise while negative values mean clockwise direction.
  Zero degrees is at the 3'o clock position.

  \sa drawArc(), drawChord()
*/
void QPainter::drawPie( int x, int y, int w, int h, int a, int alen )
{
#ifdef DEBUG_QPAINTER
    qDebug( "qpainter_win.cpp: drawPie " );
#endif
    // Holgi TODO
    // Make sure "a" is 0..360*16, as otherwise a*4 may overflow 16 bits.
    if ( a > ( 360 * 16 ) ) {
        a = a % ( 360 * 16 );
    } else if ( a < 0 ) {
        a = a % ( 360 * 16 );
        if ( a < 0 )
            a += ( 360 * 16 );
    }

    if ( !isActive() )
        return ;
    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            QPDevCmdParam param[ 3 ];
            QRect r( x, y, w, h );
            param[ 0 ].rect = &r;
            param[ 1 ].ival = a;
            param[ 2 ].ival = alen;
            if ( !pdev->cmd( QPaintDevice::PdcDrawPie, this, param ) || !hdc )
                return ;
        }
        if ( txop == TxRotShear ) {  // rotate/shear
            QPointArray pa;
            pa.makeArc( x, y, w, h, a, alen, xmat ); // arc polyline
            int n = pa.size();
            int cx, cy;
            xmat.map( x + w / 2, y + h / 2, &cx, &cy );
            pa.resize( n + 2 );
            pa.setPoint( n, cx, cy ); // add legs
            pa.setPoint( n + 1, pa.at( 0 ) );
            drawPolyInternal( pa );
            return ;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    //    XSetArcMode( dpy, gc_brush, ArcPieSlice );
    // not needed
    //    w--;
    //    h--;
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return ;
        fix_neg_rect( &x, &y, &w, &h );
    }

    //    GC g = gc;
    bool nopen = cpen.style() == NoPen;

    /*    if ( cbrush.style() != NoBrush ) {  // draw filled pie
     XFillArc( dpy, hd, gc_brush, x, y, w, h, a*4, alen*4 );
     if ( nopen ) {
         g = gc_brush;
         nopen = FALSE;
     }
        }
    */
    /*    if ( !nopen ) {   // draw pie outline
     double w2 = 0.5*w;   // with, height in ellipsis
     double h2 = 0.5*h;
     double xc = (double)x+w2;
     double yc = (double)y+h2;
     double ra1 = Q_PI/2880.0*a;  // convert a,alen to radians
     double ra2 = ra1 + Q_PI/2880.0*alen;
     int xic = qRound(xc);
     int yic = qRound(yc);
     XDrawLine( dpy, hd, g, xic, yic,
         qRound(xc + qcos(ra1)*w2), qRound(yc - qsin(ra1)*h2));
     XDrawLine( dpy, hd, g, xic, yic,
         qRound(xc + qcos(ra2)*w2), qRound(yc - qsin(ra2)*h2));
     XDrawArc( dpy, hd, g, x, y, w, h, a*4, alen*4 );
        }
    */
    double w2 = 0.5 * w;   // with, height in ellipsis
    double h2 = 0.5 * h;
    double xc = ( double ) x + w2;
    double yc = ( double ) y + h2;
    double ra1 = Q_PI / 2880.0 * a;  // convert a,alen to radians
    double ra2 = ra1 + Q_PI / 2880.0 * alen;
    //int xic = qRound(xc);
    //int yic = qRound(yc);

    if ( alen == 0 )
        return ;
    if ( alen > 0 ) {
        SetArcDirection( hdc, AD_COUNTERCLOCKWISE );
    } else {
        SetArcDirection( hdc, AD_CLOCKWISE );
    }
    if ( !nopen )         // draw pie outline
        Pie( hdc, x, y, x + w, y + h, qRound( xc + qcos( ra1 ) * w2 ), qRound( yc - qsin( ra1 ) * h2 ),
             qRound( xc + qcos( ra2 ) * w2 ), qRound( yc - qsin( ra2 ) * h2 ) );
    else
        Pie( hdc, x, y, x + w + 1, y + h + 1,
             qRound( xc + qcos( ra1 ) * w2 ), qRound( yc - qsin( ra1 ) * h2 ),
             qRound( xc + qcos( ra2 ) * w2 ), qRound( yc - qsin( ra2 ) * h2 ) );
}


/*!
  Draws a chord defined by the rectangle \a (x,y,w,h), the start
  angle \a a and the arc length \a alen.

  The chord is filled with the current brush().

  The angles \a a and \a alen are 1/16th of a degree, i.e. a full
  circle equals 5760 (16*360). Positive values of \a a and \a alen mean
  counter-clockwise while negative values mean clockwise direction.
  Zero degrees is at the 3'o clock position.

  \sa drawArc(), drawPie()
*/

void QPainter::drawChord( int x, int y, int w, int h, int a, int alen )
{
#ifdef DEBUG_QPAINTER
    qDebug( "qpainter_win.cpp: drawChord " );
#endif
    // Holgi TODO
    if ( !isActive() )
        return ;
    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            QPDevCmdParam param[ 3 ];
            QRect r( x, y, w, h );
            param[ 0 ].rect = &r;
            param[ 1 ].ival = a;
            param[ 2 ].ival = alen;
            if ( !pdev->cmd( QPaintDevice::PdcDrawChord, this, param ) || !hdc )
                return ;
        }
        if ( txop == TxRotShear ) {  // rotate/shear
            QPointArray pa;
            pa.makeArc( x, y, w - 1, h - 1, a, alen, xmat ); // arc polygon
            int n = pa.size();
            pa.resize( n + 1 );
            pa.setPoint( n, pa.at( 0 ) );  // connect endpoints
            drawPolyInternal( pa );
            return ;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return ;
        fix_neg_rect( &x, &y, &w, &h );
    }

    bool nopen = cpen.style() == NoPen;

    double w2 = 0.5 * w;   // with, height in ellipsis
    double h2 = 0.5 * h;
    double xc = ( double ) x + w2;
    double yc = ( double ) y + h2;
    double ra1 = Q_PI / 2880.0 * a;  // convert a,alen to radians
    double ra2 = ra1 + Q_PI / 2880.0 * alen;
    //int xic = qRound(xc);
    //int yic = qRound(yc);

    if ( alen == 0 )
        return ;
    if ( alen > 0 ) {
        SetArcDirection( hdc, AD_COUNTERCLOCKWISE );
    } else {
        SetArcDirection( hdc, AD_CLOCKWISE );
    }

    if ( !nopen )         // draw pie outline
        Chord( hdc, x, y, x + w, y + h, qRound( xc + qcos( ra1 ) * w2 ), qRound( yc - qsin( ra1 ) * h2 ),
               qRound( xc + qcos( ra2 ) * w2 ), qRound( yc - qsin( ra2 ) * h2 ) );
    else
        Chord( hdc, x, y, x + w + 1, y + h + 1,
               qRound( xc + qcos( ra1 ) * w2 ), qRound( yc - qsin( ra1 ) * h2 ),
               qRound( xc + qcos( ra2 ) * w2 ), qRound( yc - qsin( ra2 ) * h2 ) );
    /*    if ( !nopen ) {    // draw chord outline
     double w2 = 0.5*w;   // with, height in ellipsis
     double h2 = 0.5*h;
     double xc = (double)x+w2;
     double yc = (double)y+h2;
     double ra1 = Q_PI/2880.0*a;  // convert a,alen to radians
     double ra2 = ra1 + Q_PI/2880.0*alen;
     XDrawLine( dpy, hd, g,
         qRound(xc + qcos(ra1)*w2), qRound(yc - qsin(ra1)*h2),
         qRound(xc + qcos(ra2)*w2), qRound(yc - qsin(ra2)*h2));
     XDrawArc( dpy, hd, g, x, y, w, h, a*4, alen*4 );
        }
        XSetArcMode( dpy, gc_brush, ArcPieSlice );
    */
}


/*!
  Draws \a nlines separate lines from points defined in \a a, starting
  at a[\a index] (\a index defaults to 0). If \a nlines is -1 (the
  defauls) all points until the end of the array are used
  (i.e. (a.size()-index)/2 lines are drawn).

  Draws the 1st line from \a a[index] to \a a[index+1].
  Draws the 2nd line from \a a[index+2] to \a a[index+3] etc.

  \sa drawPolyline(), drawPolygon(), QPen
*/

void QPainter::drawLineSegments( const QPointArray &a, int index, int nlines )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::drawLineSegments( a, index: %d, nlines: %d ", index, nlines );
#endif

    if ( nlines < 0 )
        nlines = a.size() / 2 - index / 2;
    if ( index + nlines * 2 > ( int ) a.size() )
        nlines = ( a.size() - index ) / 2;
    if ( !isActive() || nlines < 1 || index < 0 )
        return ;
    QPointArray pa = a;
    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            if ( nlines != ( int ) pa.size() / 2 ) {
                pa = QPointArray( nlines * 2 );
                for ( int i = 0; i < nlines*2; i++ )
                    pa.setPoint( i, a.point( index + i ) );
                index = 0;
            }
            QPDevCmdParam param[ 1 ];
            param[ 0 ].ptarr = ( QPointArray* ) & pa;
            if ( !pdev->cmd( QPaintDevice::PdcDrawLineSegments, this, param ) || !hdc )
                return ;
        }
        if ( txop != TxNone ) {
            pa = xForm( a, index, nlines * 2 );
            if ( pa.size() != a.size() ) {
                index = 0;
                nlines = pa.size() / 2;
            }
        }
    }
    if ( cpen.style() != NoPen ) {
        POINT * ptPoints = ( POINT* ) & pa.data() [ index ];
        for ( int i = 0; i < nlines; i++ ) {
            Polyline( hdc, ptPoints, 2 );
            qt_DrawLastPoint( hdc, ptPoints[ 0 ].x, ptPoints[ 0 ].y, ptPoints[ 1 ].x, ptPoints[ 1 ].y, QT_RGB_P );
            ptPoints += 2;
        }
    }
}


/*!
  Draws the polyline defined by the \a npoints points in \a a starting
  at \a a[index].  (\a index defaults to 0.)

  If \a npoints is -1 (the default) all points until the end of the
  array are used (i.e. a.size()-index-1 line segments are drawn).

  \sa drawLineSegments(), drawPolygon(), QPen
*/

void QPainter::drawPolyline( const QPointArray &a, int index, int npoints )
{
#ifdef DEBUG_QPAINTER
    qDebug( "qpainter_win.cpp: drawPolyline " );
#endif

    QPoint myOldPoint = curPt;
    if ( npoints < 0 )
        npoints = a.size() - index;
    if ( index + npoints > ( int ) a.size() )
        npoints = a.size() - index;
    if ( !isActive() || npoints < 2 || index < 0 )
        return ;
    QPointArray pa = a;
    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            if ( npoints != ( int ) pa.size() ) {
                pa = QPointArray( npoints );
                for ( int i = 0; i < npoints; i++ )
                    pa.setPoint( i, a.point( index + i ) );
                index = 0;
            }
            QPDevCmdParam param[ 1 ];
            param[ 0 ].ptarr = ( QPointArray* ) & pa;
            if ( !pdev->cmd( QPaintDevice::PdcDrawPolyline, this, param ) || !hdc )
                return ;
        }
        if ( txop != TxNone ) {
            pa = xForm( a, index, npoints );
            if ( pa.size() != a.size() ) {
                index = 0;
                npoints = pa.size();
            }
        }
    }

    /*Note from Ivan Deras:
    Super Trick here: POINT and QPoint are compatible at structure level and then QPointArray is compatible
    at structure level too with POINT array, because of that we can pass QPoint* as parameter to POINT*.
    Check the definition for QPoint ...
    */
    if ( cpen.style() != NoPen ) {
#ifdef DEBUG_QPAINTER
        qDebug ( "drawing polyLine with pen: rgb= 0x%08x", QT_RGB_P );
        qDebug ( "npoints = %d", npoints );
        for ( int j = 0; j < npoints; j++ ) {
            int px = pa[ index + j ].x();
            int py = pa[ index + j ].y();

            qDebug ( "[%d] x=%d y=%d", j, px, py );
        }
#endif
        POINT* ptPoints = ( POINT* ) & pa.data() [ index ];
        Polyline ( hdc, ptPoints, npoints );
        qt_DrawLastPoint( hdc, pa[ index ].x(), pa[ index ].y(), pa[ index + 1 ].x(), pa[ index + 1 ].y(), QT_RGB_P );
        qt_DrawLastPoint( hdc, ptPoints[ npoints - 2 ].x, ptPoints[ npoints - 2 ].y, ptPoints[ npoints - 1 ].x, ptPoints[ npoints - 1 ].y, QT_RGB_P );
    }
}


/*!
  Draws the polygon defined by the \a npoints points in \a a starting at
  \a a[index].  (\a index defaults to 0.)

  If \a npoints is -1 (the default) all points until the end of the
  array are used (i.e. a.size()-index line segments define the
  polygon).

  The first point is always connected to the last point.

  The polygon is filled with the current brush().
  If \a winding is TRUE, the polygon is filled using the winding
  fill algorithm. If \a winding is FALSE, the polygon is filled using the
  even-odd (alternative) fill algorithm.

  \sa drawLineSegments(), drawPolyline(), QPen
*/

void QPainter::drawPolygon( const QPointArray &a, bool winding,
                            int index, int npoints )
{
#ifdef DEBUG_QPAINTER
    qDebug( "qpainter_win.cpp: drawPolygon " );
#endif
    // Holgi TODO
    if ( npoints < 0 )
        npoints = a.size() - index;
    if ( index + npoints > ( int ) a.size() )
        npoints = a.size() - index;
    if ( !isActive() || npoints < 2 || index < 0 )
        return ;
    QPointArray pa = a;
    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            if ( npoints != ( int ) a.size() ) {
                pa = QPointArray( npoints );
                for ( int i = 0; i < npoints; i++ )
                    pa.setPoint( i, a.point( index + i ) );
                index = 0;
            }
            QPDevCmdParam param[ 2 ];
            param[ 0 ].ptarr = ( QPointArray* ) & pa;
            param[ 1 ].ival = winding;
            if ( !pdev->cmd( QPaintDevice::PdcDrawPolygon, this, param ) || !hdc )
                return ;
        }
        if ( txop != TxNone ) {
            pa = xForm( a, index, npoints );
            if ( pa.size() != a.size() ) {
                index = 0;
                npoints = pa.size();
            }
        }
    }
    if ( winding )        // set to winding fill rule
        SetPolyFillMode( hdc, WINDING );
    else
        SetPolyFillMode( hdc, ALTERNATE );

    if ( pa[ index ] != pa[ index + npoints - 1 ] ) {   // close open pointarray
        pa.detach();
        pa.resize( index + npoints + 1 );
        pa.setPoint( index + npoints, pa[ index ] );
        npoints++;
    }

    /*    if ( cbrush.style() != NoBrush ) {  // draw filled polygon
     XFillPolygon( dpy, hd, gc_brush,
            (XPoint*)(pa.shortPoints( index, npoints )),
            npoints, Complex, CoordModeOrigin );
        }
    */
    //    if ( cpen.style() != NoPen ) {  // draw outline
    /* XDrawLines( dpy, hd, gc, (XPoint*)(pa.shortPoints( index, npoints )),
          npoints, CoordModeOrigin );
    */
#if defined(Q_CC_MSVC) || defined(Q_CC_BOR)
    POINT *mypoints = new POINT[ npoints ];
#else

    POINT mypoints[ npoints ];
#endif

    int i;
    for ( i = 0; i < npoints ; i++ ) {
        mypoints[ i ].x = pa.point( index + i ).x();
        mypoints[ i ].y = pa.point( index + i ).y();
    }

    Polygon( hdc, mypoints, npoints );
#if defined(Q_CC_MSVC) || defined(Q_CC_BOR)

    delete [] mypoints;
#endif
    //    }
    /*    if ( winding )    // set to normal fill rule
     XSetFillRule( dpy, gc_brush, EvenOddRule );
    */
    if ( winding )        // set to normal fill rule
        SetPolyFillMode( hdc, ALTERNATE );
}

/*!
    Draws the convex polygon defined by the \a npoints points in \a pa
    starting at \a pa[index] (\a index defaults to 0).

    If the supplied polygon is not convex, the results are undefined.

    On some platforms (e.g. X Window), this is faster than
    drawPolygon().
*/
void QPainter::drawConvexPolygon( const QPointArray &pa,
                                  int index, int npoints )
{
#ifdef DEBUG_QPAINTER
    qDebug( "qpainter_win.cpp: drawConvexPolygon" );
#endif
    //    global_polygon_shape = Convex;
    drawPolygon( pa, FALSE, index, npoints );
    //    global_polygon_shape = Complex;
}



/*!
    Draws a cubic Bezier curve defined by the control points in \a a,
    starting at \a a[index] (\a index defaults to 0).

    Control points after \a a[index + 3] are ignored. Nothing happens
    if there aren't enough control points.
*/

void QPainter::drawCubicBezier( const QPointArray &a, int index )
{
#ifdef DEBUG_QPAINTER
    qDebug( "QPainter::drawCubicBezier()" );
#endif

    if ( !isActive() )
        return ;
    if ( a.size() - index < 4 ) {
#if defined(QT_CHECK_RANGE)
        qWarning( "QPainter::drawCubicBezier: Cubic Bezier needs 4 control "
                  "points" );
#endif

        return ;
    }
    QPointArray pa( a );
    if ( index != 0 || a.size() > 4 ) {
        pa = QPointArray( 4 );
        for ( int i = 0; i < 4; i++ )
            pa.setPoint( i, a.point( index + i ) );
    }
    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) ) {
            QPDevCmdParam param[ 1 ];
            param[ 0 ].ptarr = ( QPointArray* ) & pa;
            if ( !pdev->cmd(QPaintDevice::PdcDrawCubicBezier, this, param) || !hdc )
                return;
        }
        if ( txop != TxNone )
            pa = xForm( pa );
    }
    if ( cpen.style() != NoPen ) {
        pa = pa.cubicBezier();
        int npoints = pa.count();

        POINT* ptPoints = ( POINT* ) & pa.data() [ 0 ];
        Polyline ( hdc, ptPoints, npoints );
        qt_DrawLastPoint( hdc, pa[ 0 ].x(), pa[ 0 ].y(), pa[ 1 ].x(), pa[ 1 ].y(), QT_RGB_P );
        qt_DrawLastPoint( hdc, ptPoints[ npoints - 2 ].x, ptPoints[ npoints - 2 ].y, ptPoints[ npoints - 1 ].x, ptPoints[ npoints - 1 ].y, QT_RGB_P );
    }
}



/*!
  Draws a cubic Bezier curve defined by the control points in \a a,
  starting at \a a[index].  (\a index defaults to 0.)

  Control points after \a a[index+3] are ignored.  Nothing happens if
  there aren't enough control points.
*/

/*void QPainter::drawQuadBezier( const QPointArray &a, int index )
{
#ifdef DEBUG_QPAINTER
 qDebug("qpainter_win.cpp: drawQuadBezier ");
#endif
    if ( !isActive() )
 return;
    if ( a.size() - index < 4 ) {
#if defined(QT_CHECK_RANGE)
 qWarning( "QPainter::drawQuadBezier: Cubic Bezier needs 4 control "
   "points" );
#endif
 return;
    }
    QPointArray pa( a );
    if ( index != 0 || a.size() > 4 ) {
 pa = QPointArray( 4 );
 for ( int i=0; i<4; i++ )
     pa.setPoint( i, a.point(index+i) );
    }
    if ( testf(ExtDev|VxF|WxF) ) {
 if ( testf(ExtDev) ) {
     QPDevCmdParam param[1];
     param[0].ptarr = (QPointArray*)&pa;
     if ( !pdev->cmd(QPaintDevice::PdcDrawQuadBezier,this,param) || !hdc )
  return;
 }
 if ( txop != TxNone )
     pa = xForm( pa );
    }
    if ( cpen.style() != NoPen ) {
 int i;
 POINT mypoints[4];
 for ( i = 0 ; i < 4 ; i++ )
 {
  mypoints[i].x = pa.point(i).x();
  mypoints[i].y = pa.point(i).y();
 }
 PolyBezier( hdc, mypoints, 4 );
*/ /* XDrawLines( dpy, hd, gc, (XPoint*)pa.shortPoints(), pa.size(),
      CoordModeOrigin);
*/ /*    }
}*/


/*!
  Draws a pixmap at \a (x,y) by copying a part of \a pixmap into the
  paint device.

  \a (x,y) specify the top-left point in the paint device that is to
  be drawn onto.  \a (sx,sy) specify the top-left point in \a pixmap
  that is to be drawn (the default is (0,0).  \a (sw,sh) specify the
  size of the pixmap that is to be drawn (the default, (-1,-1), means
  all the way to the right/bottom of the pixmap).

  \sa bitBlt(), QPixmap::setMask()
*/

void QPainter::drawPixmap( int x, int y, const QPixmap &pixmap,
                           int sx, int sy, int sw, int sh )
{
#ifdef DEBUG_QPAINTER
    qDebug( "qpainter_win.cpp: drawPixmap x: %d, y: %d, sx: %d, sy: %d, sw: %d, sh: %d.",
            x, y, sx, sy, sw, sh );
#endif

    if ( !isActive() || pixmap.isNull() )
        return ;

    // right/bottom
    if ( sw < 0 )
        sw = pixmap.width() - sx;
    if ( sh < 0 )
        sh = pixmap.height() - sy;

    // Sanity-check clipping
    if ( sx < 0 ) {
        x -= sx;
        sw += sx;
        sx = 0;
    }
    if ( sw + sx > pixmap.width() )
        sw = pixmap.width() - sx;
    if ( sy < 0 ) {
        y -= sy;
        sh += sy;
        sy = 0;
    }
    if ( sh + sy > pixmap.height() )
        sh = pixmap.height() - sy;

    if ( sw <= 0 || sh <= 0 )
        return ;

    if ( testf( ExtDev | VxF | WxF ) ) {
        if ( testf( ExtDev ) || txop == TxScale || txop == TxRotShear ) {
            if ( sx != 0 || sy != 0 ||
                    sw != pixmap.width() || sh != pixmap.height() ) {
                QPixmap tmp( sw, sh, pixmap.depth() );
                bitBlt( &tmp, 0, 0, &pixmap, sx, sy, sw, sh, CopyROP, TRUE );
                if ( pixmap.mask() ) {
                    QBitmap mask( sw, sh );
                    bitBlt( &mask, 0, 0, pixmap.mask(), sx, sy, sw, sh,
                            CopyROP, TRUE );
                    tmp.setMask( mask );
                }

                drawPixmap( x, y, tmp );
                return ;
            }
            if ( testf( ExtDev ) ) {
                QPDevCmdParam param[ 2 ];
                QRect r( x, y, pixmap.width(), pixmap.height() );
                param[ 0 ].rect = &r;
                param[ 1 ].pixmap = &pixmap;

                if ( !pdev->cmd( QPaintDevice::PdcDrawPixmap, this, param ) || !hdc )
                    return ;
            }
            if ( txop == TxScale || txop == TxRotShear ) {
                QWMatrix mat( m11(), m12(),
                              m21(), m22(),
                              dx(), dy() );

                mat = QPixmap::trueMatrix( mat, sw, sh );
                QPixmap pm = pixmap.xForm( mat );
                if ( !pm.mask() && txop == TxRotShear ) {
                    QBitmap bm_clip( sw, sh, 1 );
                    bm_clip.fill( color1 );
                    pm.setMask( bm_clip.xForm( mat ) );
                }
                map( x, y, &x, &y );  // compute position of pixmap
                int dx, dy;
                mat.map( 0, 0, &dx, &dy );
                uint save_flags = flags;
                flags = IsActive | ( save_flags & ClipOn );
                drawPixmap( x - dx, y - dy, pm );
                flags = save_flags;
                return ;
            }
        }
        map( x, y, &x, &y );
    }

    QBitmap *mask = ( QBitmap * ) pixmap.mask();
    bool mono = pixmap.depth() == 1;
    bool selfmask = pixmap.data->selfmask;
    bool alphamask = ( pixmap.data->realAlphaBits && ( qt_winver >= Qt::WV_98 ) && qtAlphaBlend );

    if ( !mask && !mono ) {
        BitBlt( hdc, x, y, sw, sh, pixmap.handle(), sx, sy, rop3Codes[ rop ] );
        return ;
    }

    if ( alphamask ) {
        BLENDFUNCTION bf;
        bf.BlendOp = AC_SRC_OVER;
        bf.BlendFlags = 0;
        bf.SourceConstantAlpha = 255;
        bf.AlphaFormat = AC_SRC_ALPHA;

        qtAlphaBlend( hdc, x, y, sw, sh, pixmap.handle(), sx, sy, sw, sh, bf );
    } else
        if ( mask ) {
            /*
            The parts with '0' of the mask are transparent,
            and the parts with '1' gets the background color
            or the color of the bitmap.

            qcolor.cpp:
             QT_STATIC_CONST_IMPL QColor & Qt::color0 = stdcol[0];
             QT_STATIC_CONST_IMPL QColor & Qt::color1  = stdcol[1];
             ...
             stdcol[ 0].d.argb = qRgb(255,255,255);
             stdcol[ 1].d.argb = 0;
            */

            if ( mono ) {
                // black out in destination the parts where the mask bits are '1', set colors:
                COLORREF oldT = SetTextColor( hdc, QT_RGB_1 ); // Color of mask '1' becomes '0,0,0'
                COLORREF oldB = SetBkColor( hdc, QT_RGB_0 );  // Color of mask '0' becomes '255,255,255'

                if ( !selfmask ) {
                    // black out in destination the parts where the mask bits are '1',
                    // colors already set.
                    // do bytewise raster operation AND with the destination colors:
                    BitBlt( hdc, x, y, sw, sh, mask->handle(), sx, sy, SRCAND );

                    // fill new black part of destination with background color:
                    SetTextColor( hdc, QT_RGB_B ); // Color of mask '1' becomes background color
                    SetBkColor( hdc, QT_RGB_1 );  // Color of mask '0' becomes '0,0,0'
                    // then do bytewise raster operation OR, transparent parts remain unchanged:
                    BitBlt( hdc, x, y, sw, sh, mask->handle(), sx, sy, SRCPAINT );

                    // colors for next SRCAND
                    SetTextColor( hdc, QT_RGB_1 ); // Color of bitmap '1' becomes '0,0,0'
                    SetBkColor( hdc, QT_RGB_0 );  // Color of bitmap '0' becomes '255,255,255'
                }

                // black out in destination the parts for the monochrome bitmap
                // and don't touch the transparent parts,
                // colors already set:
                BitBlt( hdc, x, y, sw, sh, pixmap.handle(), sx, sy, SRCAND );

                // paint pixmap with  arbitrary color in the black parts
                SetTextColor( hdc, QT_RGB_P ); // Color of the '1' is 'custom'
                SetBkColor( hdc, QT_RGB_1 );  // Color of the '0' is '0,0,0'
                // then do bytewise raster operation OR, transparent parts remain unchanged
                BitBlt( hdc, x, y, sw, sh, pixmap.handle(), sx, sy, SRCPAINT );

                SetTextColor( hdc, oldT );
                SetBkColor( hdc, oldB );
            } else {
                COLORREF oldT = SetTextColor( hdc, QT_RGB_1 );
                COLORREF oldB = SetBkColor( hdc, QT_RGB_0 );

                BitBlt( hdc, x, y, sw, sh, pixmap.handle(), sx, sy, SRCINVERT );
                BitBlt( hdc, x, y, sw, sh, mask->handle(), sx, sy, SRCAND );
                BitBlt( hdc, x, y, sw, sh, pixmap.handle(), sx, sy, SRCINVERT );

                SetTextColor( hdc, oldT );
                SetBkColor( hdc, oldB );
            }
        } else { // ( !mask && mono )
            COLORREF oldT = SetTextColor( hdc, QT_RGB_P );
            COLORREF oldB = SetBkColor( hdc, QT_RGB_B );

            BitBlt( hdc, x, y, sw, sh, pixmap.handle(), sx, sy, SRCCOPY );

            SetTextColor( hdc, oldT );
            SetBkColor( hdc, oldB );
        }
}

/*!
  Draws a tiled \a pixmap in the specified rectangle.

  \a (x,y) specify the top-left point in the paint device that is to
  be drawn onto.  \a (sx,sy) specify the top-left point in \a pixmap
  that is to be drawn (the default is (0,0).

  Calling drawTiledPixmap() is similar to calling drawPixmap() several
  times to fill (tile) an area with a pixmap, but is potentially
  much more efficient depending on the underlying window system.

  \sa drawPixmap()
*/

void QPainter::drawTiledPixmap( int x, int y, int w, int h,
                                const QPixmap &pixmap, int sx, int sy )
{
#ifdef DEBUG_QPAINTER
    qDebug( "qpainter_win.cpp: drawTiledPixmap(x=%d, y=%d, w=%d, h=%d):(sx=%d, sy=%d)", x, y, w, h, sx, sy );
#endif

    if ( qt_winver == Qt::WV_95 ) {
        int yPos, xPos, drawH, drawW, yOff, xOff;
        int sw = pixmap.width();
        int sh = pixmap.height();
        if ( !sw || !sh )
            return ;
        if ( sx < 0 )
            sx = sw - -sx % sw;
        else
            sx = sx % sw;
        if ( sy < 0 )
            sy = sh - -sy % sh;
        else
            sy = sy % sh;

        yPos = y;
        yOff = sy;
        while ( yPos < y + h ) {
            drawH = pixmap.height() - yOff;    // Cropping first row
            if ( yPos + drawH > y + h )        // Cropping last row
                drawH = y + h - yPos;
            xPos = x;
            xOff = sx;
            while ( xPos < x + w ) {
                drawW = pixmap.width() - xOff;  // Cropping first column
                if ( xPos + drawW > x + w )     // Cropping last column
                    drawW = x + w - xPos;
                drawPixmap( xPos, yPos, pixmap, xOff, yOff, drawW, drawH );
                xPos += drawW;
                xOff = 0;
            }
            yPos += drawH;
            yOff = 0;
        }
    } else {
        int xOffset = -sx;
        int yOffset = -sy;
        // not win95 :)
        QPixmap pm ( w, h );    // pixmap
        QBitmap bm ( w, h );    // mask

        // there is no need to save old brushes & origins since pm and bm are temp
        HDC my_hdc = pm.handle();
        HBRUSH newBrush = CreatePatternBrush( pixmap.hbm() );
        HBRUSH hOldBrush = ( HBRUSH ) SelectObject( my_hdc, newBrush );
        SetBrushOrgEx( my_hdc, xOffset, yOffset, NULL );
        PatBlt( my_hdc, 0, 0, w, h, PATCOPY );
        // This is needed for proper delete of newBrush
        SelectObject( my_hdc, hOldBrush );
        DeleteObject( newBrush );

        // and mask
        if ( pixmap.mask() ) {
            QPixmap tmp_pm( w, h );
            my_hdc = bm.handle();
            my_hdc = tmp_pm.handle();
            newBrush = CreatePatternBrush( pixmap.mask() ->hbm() );
            hOldBrush = ( HBRUSH ) SelectObject( my_hdc, newBrush );
            SetBrushOrgEx( my_hdc, xOffset, yOffset, NULL );
            PatBlt( my_hdc, 0, 0, w, h, PATCOPY );
            SelectObject( my_hdc, hOldBrush );
            DeleteObject( newBrush );
            BitBlt( bm.handle(), 0, 0, w, h, my_hdc, 0, 0, SRCCOPY );

            pm.setMask( bm );
        }
        // draw it on correct pixmap
        drawPixmap( x, y, pm, 0, 0 );
    }
}


//
// Generate a string that describes a transformed bitmap. This string is used
// to insert and find bitmaps in the global pixmap cache.
//

static QString gen_text_bitmap_key( const QWMatrix &m, const QFont &font,
                                    const QString &str, int pos, int len )
{
    QString fk = font.key();
    int sz = 4 * 2 + len * 2 + fk.length() * 2 + sizeof( double ) * 6;
    QByteArray buf( sz );
    uchar *p = ( uchar * ) buf.data();
    *( ( double* ) p ) = m.m11();
    p += sizeof( double );
    *( ( double* ) p ) = m.m12();
    p += sizeof( double );
    *( ( double* ) p ) = m.m21();
    p += sizeof( double );
    *( ( double* ) p ) = m.m22();
    p += sizeof( double );
    *( ( double* ) p ) = m.dx();
    p += sizeof( double );
    *( ( double* ) p ) = m.dy();
    p += sizeof( double );
    QChar h1( '$' );
    QChar h2( 'q' );
    QChar h3( 't' );
    QChar h4( '$' );
    *( ( QChar* ) p ) = h1;
    p += 2;
    *( ( QChar* ) p ) = h2;
    p += 2;
    *( ( QChar* ) p ) = h3;
    p += 2;
    *( ( QChar* ) p ) = h4;
    p += 2;
    memcpy( ( char* ) p, ( char* ) ( str.unicode() + pos ), len * 2 );
    p += len * 2;
    memcpy( ( char* ) p, ( char* ) fk.unicode(), fk.length() * 2 );
    p += fk.length() * 2;
    return QString( ( QChar* ) buf.data(), buf.size() / 2 );
}

static QBitmap *get_text_bitmap( const QString &key )
{
#ifdef DEBUG_QPAINTER
    qDebug( "qpainter_win.cpp: get_text_bitmap " );
#endif

    return ( QBitmap* ) QPixmapCache::find( key );
}

static void ins_text_bitmap( const QString &key, QBitmap *bm )
{
#ifdef DEBUG_QPAINTER
    qDebug( "qpainter_win.cpp: ins_text_bitmap " );
#endif

    if ( !QPixmapCache::insert( key, bm ) )     // cannot insert pixmap
        delete bm;
}

/*!
  Draws at most \a len characters from \a str at position \a (x,y).

  \a (x,y) is the base line position.  Note that the meaning of \a y
  is not the same for the two drawText() varieties.
*/

void QPainter::drawText( int x, int y, const QString &str, int len, TextDirection dir )
{
#ifdef DEBUG_QPAINTER
    qDebug( "qpainter_win.cpp: drawText" );
#endif

    if ( len < 0 )
        len = str.length();
    if ( len == 0 )
        return ;
    drawText( x, y, str, 0, len, dir );
}


void QPainter::drawText( int x, int y, const QString &str, int pos, int len, QPainter::TextDirection dir )
{
#ifdef DEBUG_QPAINTER
    qDebug( "qpainter_win.cpp: drawText len: %d str: ascii: %s", len, str.ascii() );
#endif

    if ( !isActive() )
        return ;
    if ( len < 0 )
        len = str.length() - pos;
    if ( len == 0 || pos >= ( int ) str.length() )     // empty string
        return ;
    if ( pos + len > ( int ) str.length() )
        len = str.length() - pos;

    if ( testf( DirtyFont ) ) {
        updateFont();
    }

    if ( testf( ExtDev ) && pdev->devType() != QInternal::Printer ) {
        QPDevCmdParam param[ 3 ];
        QPoint p( x, y );
        QString string = str.mid( pos, len );
        param[ 0 ].point = &p;
        param[ 1 ].str = &string;
        param[ 2 ].ival = QFont::Latin;
        if ( !pdev->cmd( QPaintDevice::PdcDrawText2, this, param ) || !hdc )
            return ;
    }

    bool simple = str.simpleText();
    // we can't take the complete string here as we would otherwise
    // get quadratic behaviour when drawing long strings in parts.
    // we do however need some chars around the part we paint to get arabic shaping correct.
    // ### maybe possible to remove after cursor restrictions work in QRT
    int start;
    int end;
    if ( simple ) {
        start = pos;
        end = pos + len;
    } else {
        start = QMAX( 0, pos - 8 );
        end = QMIN( ( int ) str.length(), pos + len + 8 );
    }
    QConstString cstr( str.unicode() + start, end - start );
    pos -= start;

    QTextEngine engine( cstr.string(), pfont ? pfont->d : cfont.d );
    QTextLayout layout( &engine );

    // this is actually what beginLayout does. Inlined here, so we can
    // avoid the bidi algorithm if we don't need it.
    engine.itemize( simple ? QTextEngine::NoBidi | QTextEngine::SingleLine : QTextEngine::Full | QTextEngine::SingleLine );
    engine.currentItem = 0;
    engine.firstItemInLine = -1;

    if ( !simple ) {
        layout.setBoundary( pos );
        layout.setBoundary( pos + len );
    }

    if ( dir != Auto ) {
        int level = dir == RTL ? 1 : 0;
        for ( int i = engine.items.size(); i >= 0; i-- )
            engine.items[ i ].analysis.bidiLevel = level;
    }

    // small hack to force skipping of unneeded items
    start = 0;
    while ( engine.items[ start ].position < pos )
        ++start;
    engine.currentItem = start;
    layout.beginLine( 0xfffffff );
    end = start;
    while ( !layout.atEnd() && layout.currentItem().from() < pos + len ) {
        layout.addCurrentItem();
        end++;
    }
    QFontMetrics fm( fontMetrics() );
    int ascent = fm.ascent(), descent = fm.descent();
    int left, right;
    layout.endLine( 0, 0, Qt::SingleLine | Qt::AlignLeft, &ascent, &descent, &left, &right );

    // do _not_ call endLayout() here, as it would clean up the shaped items and we would do shaping another time
    // for painting.

    int textFlags = 0;
    if ( cfont.d->underline )
        textFlags |= Qt::Underline;
    if ( cfont.d->overline )
        textFlags |= Qt::Overline;
    if ( cfont.d->strikeOut )
        textFlags |= Qt::StrikeOut;

    if ( bg_mode == OpaqueMode )
        qt_draw_background( this, x, y - ascent, right - left, ascent + descent + 1 );

    for ( int i = start; i < end; i++ ) {
        QTextItem ti;
        ti.item = i;
        ti.engine = &engine;

        drawTextItem( x, y - ascent, ti, textFlags );
    }
    layout.d = 0;
}

/*!
  Returns the current position of the  pen.

  \sa moveTo()
 */
QPoint QPainter::pos() const
{
    return curPt;
}

/*! \internal
    Draws the text item \a ti at position \a (x, y ).

    This method ignores the painters background mode and
    color. drawText and qt_format_text have to do it themselves, as
    only they know the extents of the complete string.

    It ignores the font set on the painter as the text item has one of its own.

    The underline and strikeout parameters of the text items font are
    ignored aswell. You'll need to pass in the correct flags to get
    underlining and strikeout.
*/
void QPainter::drawTextItem( int x, int y, const QTextItem & ti, int textFlags )
{
    if ( testf( ExtDev ) ) {
        QPDevCmdParam param[ 2 ];
        QPoint p( x, y );
        param[ 0 ].point = &p;
        param[ 1 ].textItem = &ti;
        bool retval = pdev->cmd( QPaintDevice::PdcDrawTextItem, this, param );

        if ( !retval || !hdc )
            return ;
    }

    QTextEngine *engine = ti.engine;
    QScriptItem *si = &engine->items[ ti.item ];

    engine->shape( ti.item );
    QFontEngine *fe = si->fontEngine;
    assert( fe != 0 );

    x += si->x;
    y += si->y;

    fe->draw( this, x, y, engine, si, textFlags );
}

void qt_draw_transformed_rect( QPainter * p, int x, int y, int w, int h, bool fill )
{
    RECT rect;
    int xp = x, yp = y;
    p->map( xp, yp, &xp, &yp );
    rect.left = xp;
    rect.top = yp;
    xp = x + w;
    yp = y + h;
    p->map( xp, yp, &xp, &yp );
    rect.right = xp;
    rect.bottom = yp;

#ifdef DEBUG_QPAINTER
    qDebug ( "qt_draw_transformed_rect: %d,%d - %d,%d", rect.left, rect.top, rect.right, rect.bottom );
#endif

    if ( fill ) {
        HBRUSH oldBrush = p->hbrush;
        p->hbrush = CreateSolidBrush ( RGB( p->cpen.color().red(), p->cpen.color().green(), p->cpen.color().blue() ) );
        FillRect ( p->hdc, &rect, p->hbrush );
        DeleteObject ( p->hbrush );
        p->hbrush = oldBrush;
    } else {
        MoveToEx ( p->hdc, rect.left, rect.top, NULL );
        LineTo ( p->hdc, rect.right, rect.top );
        LineTo ( p->hdc, rect.right, rect.bottom );
        LineTo ( p->hdc, rect.left, rect.bottom );
        LineTo ( p->hdc, rect.left, rect.top );
    }
}

void qt_draw_background( QPainter * p, int x, int y, int w, int h )
{
    if ( p->testf( QPainter::ExtDev ) ) {
        if ( p->pdev->devType() == QInternal::Printer )
            p->fillRect( x, y, w, h, p->bg_col );
        return ;
    }

    RECT rect;
    int xp = x, yp = y;
    p->map( xp, yp, &xp, &yp );
    rect.left = xp;
    rect.top = yp;
    xp = x + w;
    yp = y + h;
    p->map( xp, yp, &xp, &yp );
    rect.right = xp;
    rect.bottom = yp;

    COLORREF oldBkColor = SetBkColor( p->handle(), RGB( p->bg_col.red(), p->bg_col.green(), p->bg_col.blue() ) );
    ExtTextOutA( p->handle(), 0, 0, ETO_OPAQUE, &rect, ( LPCSTR ) "", 0, 0 );
    SetBkColor( p->handle(), oldBkColor );
}
