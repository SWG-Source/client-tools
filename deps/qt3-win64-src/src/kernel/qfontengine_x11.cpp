/****************************************************************************
** $Id: qfontengine_x11.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** ???
**
** Copyright (C) 2003-2007 Trolltech ASA.  All rights reserved.
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
** licenses for Unix/X11 or for Qt/Embedded may use this file in accordance
** with the Qt Commercial License Agreement provided with the Software.
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

#include "qfontengine_p.h"

// #define FONTENGINE_DEBUG

#include <qwidget.h>
#include <qcstring.h>
#include <qtextcodec.h>

#include "qbitmap.h"
#include "qfontdatabase.h"
#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qimage.h"

#include "qt_x11_p.h"

#include "qfont.h"
#include "qtextengine_p.h"

#include <private/qunicodetables_p.h>

#include <limits.h>

// defined in qfontdatbase_x11.cpp
extern int qt_mib_for_xlfd_encoding( const char *encoding );
extern int qt_xlfd_encoding_id( const char *encoding );

extern void qt_draw_transformed_rect( QPainter *p, int x, int y, int w, int h, bool fill );

static void drawLines( QPainter *p, QFontEngine *fe, int baseline, int x1, int w, int textFlags )
{
    int lw = fe->lineThickness();
    if ( textFlags & Qt::Underline ) {
    	int pos = fe->underlinePosition();
	qt_draw_transformed_rect( p, x1, baseline+pos, w, lw, TRUE );
    }
    if ( textFlags & Qt::Overline ) {
	int pos = fe->ascent()+1;
	if ( !pos ) pos = 1;
	qt_draw_transformed_rect( p, x1, baseline-pos, w, lw, TRUE );
    }
    if ( textFlags & Qt::StrikeOut ) {
	int pos = fe->ascent()/3;
	if ( !pos ) pos = 1;
	qt_draw_transformed_rect( p, x1, baseline-pos, w, lw, TRUE );
    }
}


inline static void qSafeXDestroyImage( XImage *x )
{
    if ( x->data ) {
	free( x->data );
	x->data = 0;
    }
    XDestroyImage( x );
}

extern bool qt_xForm_helper( const QWMatrix &trueMat, int xoffset,
                             int type, int depth,
                             uchar *dptr, int dbpl, int p_inc, int dHeight,
                             uchar *sptr, int sbpl, int sWidth, int sHeight
    );

static QBitmap transform(Display *dpy, const QBitmap &source, int xoff,  int yoff, int w, int h, const QWMatrix &matrix)
{
    int ws = source.width();
    int hs = source.height();

    bool invertible;
    QWMatrix mat = matrix.invert( &invertible );		// invert matrix

    if (!invertible )
	return QBitmap();
    mat.translate(xoff, yoff);

    XImage *xi = XGetImage(dpy, source.handle(), 0, 0, ws, hs, AllPlanes, XYPixmap);

    if ( !xi )
        return QBitmap();

    int sbpl = xi->bytes_per_line;
    uchar *sptr = (uchar *)xi->data;

    int dbpl = (w+7)/8;
    int dbytes = dbpl*h;

    uchar *dptr = (uchar *)malloc( dbytes );	// create buffer for bits
    memset( dptr, 0, dbytes );

    int type = xi->bitmap_bit_order == MSBFirst ? QT_XFORM_TYPE_MSBFIRST : QT_XFORM_TYPE_LSBFIRST;
    int	xbpl, p_inc;
    xbpl  = (w+7)/8;
    p_inc = dbpl - xbpl;

    bool ok = qt_xForm_helper( mat, xi->xoffset, type, 1, dptr, xbpl, p_inc, h, sptr, sbpl, ws, hs );
    qSafeXDestroyImage(xi);
    QBitmap bm;
    if (ok) {
        bm = QBitmap( w, h, dptr, QImage::systemBitOrder() != QImage::BigEndian );
    } else {
#if defined(QT_CHECK_RANGE)
	qWarning( "QFontEngineXft::tranform: xform failed");
#endif
    }

    free( dptr );
    return bm;
}


static void drawScaled(int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags,
                       Display *dpy, GC gc, QPaintDevice *pdev, QFontEngine *fe,
                       const QWMatrix &xmat, float scale)
{
    // font doesn't support transformations, need to do it by hand
    int w = qRound(si->width/scale);
    int h = qRound((si->ascent + si->descent + 1)/scale);
    if (w == 0 || h == 0)
        return;
    QWMatrix mat1 = xmat;
    mat1.scale(scale, scale);

    w += h; // add some pixels to width because of italic correction
    QBitmap bm( w, h, TRUE );     // create bitmap
    QPainter paint;
    paint.begin( &bm );             // draw text in bitmap
    fe->draw( &paint, 0, si->ascent/scale, engine, si, textFlags );
    paint.end();

    QRect pdevRect;
    if (pdev->devType() == QInternal::Widget)
        pdevRect = ((QWidget *)pdev)->rect();
    else if (pdev->devType() == QInternal::Pixmap)
        pdevRect = ((QPixmap *)pdev)->rect();
    else
        return;


    QRect br = mat1.mapRect(QRect(x, y - si->ascent, w, h));
    QRect br2 = br & pdevRect;
    if (br2.width() <= 0 || br2.height() <= 0
        || br2.width() >= 32768 || br2.height() >= 32768)
        return;
    QWMatrix mat = QPixmap::trueMatrix( mat1, w, h );
    QBitmap wx_bm = ::transform(dpy, bm, br2.x() - br.x(), br2.y() - br.y(), br2.width(), br2.height(), mat);
    if ( wx_bm.isNull() )
        return;

    x = br2.x();
    y = br2.y();

    Qt::HANDLE hd = pdev->handle();
    XSetFillStyle( dpy, gc, FillStippled );
    XSetStipple( dpy, gc, wx_bm.handle() );
    XSetTSOrigin( dpy, gc, x, y );
    XFillRectangle( dpy, hd, gc, x, y, wx_bm.width(), wx_bm.height() );
    XSetTSOrigin( dpy, gc, 0, 0 );
    XSetFillStyle( dpy, gc, FillSolid );
}


QFontEngine::~QFontEngine()
{
}

int QFontEngine::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if ( lw < 2 && score >= 1050 ) lw = 2;
    if ( lw == 0 ) lw = 1;

    return lw;
}

int QFontEngine::underlinePosition() const
{
    int pos = ( ( lineThickness() * 2 ) + 3 ) / 6;
    return pos ? pos : 1;
}

// ------------------------------------------------------------------
// The box font engine
// ------------------------------------------------------------------


QFontEngineBox::QFontEngineBox( int size )
    : _size( size )
{
    cache_cost = sizeof( QFontEngineBox );
}

QFontEngineBox::~QFontEngineBox()
{
}

QFontEngine::Error QFontEngineBox::stringToCMap( const QChar *, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    memset( glyphs, 0, len * sizeof( glyph_t ) );
    *nglyphs = len;

    if ( advances ) {
	for ( int i = 0; i < len; i++ )
	    *(advances++) = _size;
    }
    return NoError;
}

void QFontEngineBox::draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags )
{
    Display *dpy = QPaintDevice::x11AppDisplay();
    Qt::HANDLE hd = p->device()->handle();
    GC gc = p->gc;

#ifdef FONTENGINE_DEBUG
    p->save();
    p->setBrush( Qt::white );
    glyph_metrics_t ci = boundingBox( glyphs, offsets, numGlyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    p->drawRect( x + ci.x, y + 50 + ci.y, ci.width, ci.height );
    qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
    int xp = x;
    int yp = y;
#endif

    GlyphAttributes *glyphAttributes = engine->glyphAttributes( si );

    if ( p->txop > QPainter::TxTranslate ) {
	int xp = x;
	int yp = _size + 2;
	int s = _size - 3;
	for (int k = 0; k < si->num_glyphs; k++) {
	    if (!glyphAttributes[k].zeroWidth)
    	        qt_draw_transformed_rect( p, xp, yp, s, s, FALSE );
	    xp += _size;
	}
    } else {
	if ( p->txop == QPainter::TxTranslate )
	    p->map( x, y, &x, &y );

	XRectangle rects[64];

	int gl = 0;
	while (gl < si->num_glyphs) {
	    int toDraw = QMIN(64, si->num_glyphs-gl);
	    int adv = toDraw*_size;
	    if (x + adv < SHRT_MAX && x > SHRT_MIN) {
                    int ng = 0;
		for (int k = 0; k < toDraw; k++) {
            	    if (!glyphAttributes[gl + k].zeroWidth) {
                            rects[ng].x = x + (k * _size);
                            rects[ng].y = y - _size + 2;
                            rects[ng].width = rects[k].height = _size - 3;
                            ++ng;
                        }
		}
		XDrawRectangles(dpy, hd, gc, rects, ng);
	    }
	    gl += toDraw;
	    x += adv;
	}
    }

    if ( textFlags != 0 )
	drawLines( p, this, y, x, si->num_glyphs*_size, textFlags );

#ifdef FONTENGINE_DEBUG
    x = xp;
    y = yp;
    p->save();
    p->setPen( Qt::red );
    for ( int i = 0; i < numGlyphs; i++ ) {
	glyph_metrics_t ci = boundingBox( glyphs[i] );
	x += offsets[i].x;
	y += offsets[i].y;
	p->drawRect( x + ci.x, y + 50 + ci.y, ci.width, ci.height );
	qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offset=(%d/%d)", i, ci.x, ci.y, ci.width, ci.height,
	       ci.xoff, ci.yoff, offsets[i].x, offsets[i].y );
	x += ci.xoff;
	y += ci.yoff;
    }
    p->restore();
#endif
}

glyph_metrics_t QFontEngineBox::boundingBox( const glyph_t *, const advance_t *, const qoffset_t *, int numGlyphs )
{
    glyph_metrics_t overall;
    overall.x = overall.y = 0;
    overall.width = _size*numGlyphs;
    overall.height = _size;
    overall.xoff = overall.width;
    overall.yoff = 0;
    return overall;
}

glyph_metrics_t QFontEngineBox::boundingBox( glyph_t )
{
    return glyph_metrics_t( 0, _size, _size, _size, _size, 0 );
}



int QFontEngineBox::ascent() const
{
    return _size;
}

int QFontEngineBox::descent() const
{
    return 0;
}

int QFontEngineBox::leading() const
{
    int l = qRound( _size * 0.15 );
    return (l > 0) ? l : 1;
}

int QFontEngineBox::maxCharWidth() const
{
    return _size;
}

int QFontEngineBox::cmap() const
{
    return -1;
}

const char *QFontEngineBox::name() const
{
    return "null";
}

bool QFontEngineBox::canRender( const QChar *, int )
{
    return TRUE;
}

QFontEngine::Type QFontEngineBox::type() const
{
    return Box;
}




// ------------------------------------------------------------------
// Xlfd cont engine
// ------------------------------------------------------------------

static inline XCharStruct *charStruct( XFontStruct *xfs, uint ch )
{
    XCharStruct *xcs = 0;
    unsigned char r = ch>>8;
    unsigned char c = ch&0xff;
    if ( r >= xfs->min_byte1 &&
	 r <= xfs->max_byte1 &&
	 c >= xfs->min_char_or_byte2 &&
	 c <= xfs->max_char_or_byte2) {
	if ( !xfs->per_char )
	    xcs = &(xfs->min_bounds);
	else {
	    xcs = xfs->per_char + ((r - xfs->min_byte1) *
				   (xfs->max_char_or_byte2 -
				    xfs->min_char_or_byte2 + 1)) +
		  (c - xfs->min_char_or_byte2);
	    if (xcs->width == 0 && xcs->ascent == 0 &&  xcs->descent == 0)
		xcs = 0;
	}
    }
    return xcs;
}

QFontEngineXLFD::QFontEngineXLFD( XFontStruct *fs, const char *name, int mib )
    : _fs( fs ), _name( name ), _codec( 0 ), _scale( 1. ), _cmap( mib )
{
    if ( _cmap ) _codec = QTextCodec::codecForMib( _cmap );

    cache_cost = (((fs->max_byte1 - fs->min_byte1) *
		   (fs->max_char_or_byte2 - fs->min_char_or_byte2 + 1)) +
		  fs->max_char_or_byte2 - fs->min_char_or_byte2);
    cache_cost = ((fs->max_bounds.ascent + fs->max_bounds.descent) *
		  (fs->max_bounds.width * cache_cost / 8));
    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;

#if 1
    // Server side transformations do not seem to work correctly for
    // all types of fonts (for example, it works for bdf/pcf fonts,
    // but not for ttf).  It also seems to be extermely server
    // dependent.  The best thing is to just disable server side
    // transformations until either server support matures or we
    // figure out a better way to do it.
    xlfd_transformations = XlfdTrUnsupported;
#else
    xlfd_transformations = XlfdTrUnknown;

    // Hummingbird's Exceed X server will substitute 'fixed' for any
    // known fonts, and it doesn't seem to support transformations, so
    // we should never try to use xlfd transformations with it
    if (strstr(ServerVendor(QPaintDevice::x11AppDisplay()), "Hummingbird"))
	xlfd_transformations = XlfdTrUnsupported;
#endif
}

QFontEngineXLFD::~QFontEngineXLFD()
{
    XFreeFont( QPaintDevice::x11AppDisplay(), _fs );
    _fs = 0;
    TransformedFont *trf = transformed_fonts;
    while ( trf ) {
	XUnloadFont( QPaintDevice::x11AppDisplay(), trf->xlfd_font );
	TransformedFont *tmp = trf;
	trf = trf->next;
	delete tmp;
    }
}

QFontEngine::Error QFontEngineXLFD::stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool mirrored ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    if ( _codec ) {
	bool haveNbsp = FALSE;
	for ( int i = 0; i < len; i++ )
	    if ( str[i].unicode() == 0xa0 ) {
		haveNbsp = TRUE;
		break;
	    }

	QChar *chars = (QChar *)str;
	if ( haveNbsp || mirrored ) {
	    chars = (QChar *)malloc( len*sizeof(QChar) );
	    for ( int i = 0; i < len; i++ )
		chars[i] = (str[i].unicode() == 0xa0 ? 0x20 :
			    (mirrored ? ::mirroredChar(str[i]).unicode() : str[i].unicode()));
	}
	_codec->fromUnicodeInternal( chars, glyphs, len );
	if (chars != str)
	    free( chars );
    } else {
	glyph_t *g = glyphs + len;
	const QChar *c = str + len;
	if ( mirrored ) {
	    while ( c != str )
		*(--g) = (--c)->unicode() == 0xa0 ? 0x20 : ::mirroredChar(*c).unicode();
	} else {
	    while ( c != str )
		*(--g) = (--c)->unicode() == 0xa0 ? 0x20 : c->unicode();
	}
    }
    *nglyphs = len;

    if ( advances ) {
	glyph_t *g = glyphs + len;
	advance_t *a = advances + len;
	XCharStruct *xcs;
	// inlined for better perfomance
	if ( !_fs->per_char ) {
	    xcs = &_fs->min_bounds;
	    while ( a != advances )
		*(--a) = xcs->width;
	}
	else if ( !_fs->max_byte1 ) {
	    XCharStruct *base = _fs->per_char - _fs->min_char_or_byte2;
	    while ( g-- != glyphs ) {
		unsigned int gl = *g;
		xcs = (gl >= _fs->min_char_or_byte2 && gl <= _fs->max_char_or_byte2) ?
		      base + gl : 0;
		*(--a) = (!xcs || (!xcs->width && !xcs->ascent && !xcs->descent)) ? _fs->ascent : xcs->width;
	    }
	}
	else {
	    while ( g != glyphs ) {
		xcs = charStruct( _fs, *(--g) );
		*(--a) = (xcs ? xcs->width : _fs->ascent);
	    }
	}
	if ( _scale != 1. ) {
	    for ( int i = 0; i < len; i++ )
		advances[i] = qRound(advances[i]*_scale);
	}
    }
    return NoError;
}

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static bool x_font_load_error = FALSE;
static int x_font_errorhandler(Display *, XErrorEvent *)
{
    x_font_load_error = TRUE;
    return 0;
}

#if defined(Q_C_CALLBACKS)
}
#endif


void QFontEngineXLFD::draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags )
{
    if ( !si->num_glyphs )
	return;

//     qDebug("QFontEngineXLFD::draw( %d, %d, numglyphs=%d", x, y, si->num_glyphs );

    Display *dpy = QPaintDevice::x11AppDisplay();
    Qt::HANDLE hd = p->device()->handle();
    GC gc = p->gc;

    bool transform = FALSE;
    int xorig = x;
    int yorig = y;

    Qt::HANDLE font_id = _fs->fid;
    if ( p->txop > QPainter::TxTranslate || _scale < 0.9999 || _scale > 1.0001  ) {
	bool degenerate = QABS( p->m11()*p->m22() - p->m12()*p->m21() ) < 0.01;
	if ( !degenerate && xlfd_transformations != XlfdTrUnsupported ) {
	    // need a transformed font from the server
	    QCString xlfd_transformed = _name;
	    int field = 0;
	    char *data = xlfd_transformed.data();
	    int pos = 0;
	    while ( field < 7 ) {
		if ( data[pos] == '-' )
		    field++;
		pos++;
	    }
	    int endPos = pos;
	    while ( data[endPos] != '-' )
		endPos++;
	    float size = xlfd_transformed.mid( pos, endPos-pos ).toInt();
	    float mat[4];
	    mat[0] = p->m11()*size*_scale;
	    mat[1] = -p->m12()*size*_scale;
	    mat[2] = -p->m21()*size*_scale;
	    mat[3] = p->m22()*size*_scale;

	    // check if we have it cached
	    TransformedFont *trf = transformed_fonts;
	    TransformedFont *prev = 0;
	    int i = 0;
	    while ( trf ) {
		if ( trf->xx == mat[0] &&
		     trf->xy == mat[1] &&
		     trf->yx == mat[2] &&
		     trf->yy == mat[3] )
		    break;
		TransformedFont *tmp = trf;
		trf = trf->next;
		if (i > 10) {
		    XUnloadFont( QPaintDevice::x11AppDisplay(), tmp->xlfd_font );
		    delete tmp;
		    prev->next = trf;
		} else {
		    prev = tmp;
		}
		++i;
	    }
	    if ( trf ) {
		if ( prev ) {
		    // move to beginning of list
		    prev->next = trf->next;
		    trf->next = transformed_fonts;
		    transformed_fonts = trf;
		}
		font_id = trf->xlfd_font;
	    } else {
		QCString matrix="[";
		for ( int i = 0; i < 4; i++ ) {
		    float f = mat[i];
		    if ( f < 0 ) {
			matrix += '~';
			f = -f;
		    }
		    matrix += QString::number( f, 'f', 5 ).latin1();
		    matrix += ' ';
		}
		matrix += ']';
		//qDebug("m: %2.2f %2.2f %2.2f %2.2f, matrix=%s", p->m11(), p->m12(), p->m21(), p->m22(), matrix.data());
		xlfd_transformed.replace( pos, endPos-pos, matrix );

		x_font_load_error = FALSE;
		XErrorHandler old_handler = XSetErrorHandler( x_font_errorhandler );
		font_id = XLoadFont( dpy, xlfd_transformed.data() );
		XSync( dpy, FALSE );
		XSetErrorHandler( old_handler );
		if ( x_font_load_error ) {
		    //qDebug( "couldn't load transformed font" );
		    font_id = _fs->fid;
		    xlfd_transformations = XlfdTrUnsupported;
		} else {
		    TransformedFont *trf = new TransformedFont;
		    trf->xx = mat[0];
		    trf->xy = mat[1];
		    trf->yx = mat[2];
		    trf->yy = mat[3];
		    trf->xlfd_font = font_id;
		    trf->next = transformed_fonts;
		    transformed_fonts = trf;
		}
	    }
	}
	if ( degenerate || xlfd_transformations == XlfdTrUnsupported ) {
	    // XServer or font don't support server side transformations, need to do it by hand
            float tmp = _scale;
            _scale = 1.;
            drawScaled(x, y, engine, si, textFlags, dpy, p->gc, p->device(), this, p->xmat, tmp);
            _scale = tmp;
	    return;
        }
	transform = TRUE;
    } else if ( p->txop == QPainter::TxTranslate ) {
	p->map( x, y, &x, &y );
    }

    XSetFont(dpy, gc, font_id);

#ifdef FONTENGINE_DEBUG
    p->save();
    p->setBrush( Qt::white );
    glyph_metrics_t ci = boundingBox( glyphs, advances, offsets, si->num_glyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    p->drawRect( x + ci.x, y + 100 + ci.y, ci.width, ci.height );
    qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
    int xp = x;
    int yp = y;
#endif

    glyph_t *glyphs = engine->glyphs( si );
    advance_t *advances = engine->advances( si );
    qoffset_t *offsets = engine->offsets( si );

    XChar2b ch[256];
    XChar2b *chars = ch;
    if ( si->num_glyphs > 255 )
	chars = (XChar2b *)malloc( si->num_glyphs*sizeof(XChar2b) );

    for (int i = 0; i < si->num_glyphs; i++) {
	chars[i].byte1 = glyphs[i] >> 8;
	chars[i].byte2 = glyphs[i] & 0xff;
    }

    int xpos = x;
    GlyphAttributes *glyphAttributes = engine->glyphAttributes( si );

    if ( si->analysis.bidiLevel % 2 ) {
	int i = si->num_glyphs;
	while( i-- ) {
	    advance_t adv = advances[i];
	    // 	    qDebug("advance = %d/%d", adv.x, adv.y );
	    x += adv;
	    glyph_metrics_t gi = boundingBox( glyphs[i] );
	    int xp = x-offsets[i].x-gi.xoff;
	    int yp = y+offsets[i].y-gi.yoff;
	    if ( transform )
		p->map( xp, yp, &xp, &yp );
	    if (!glyphAttributes[i].zeroWidth && xp < SHRT_MAX && xp > SHRT_MIN)
		XDrawString16(dpy, hd, gc, xp, yp, chars+i, 1 );
	}
    } else {
	if ( transform || si->hasPositioning ) {
	    int i = 0;
	    while( i < si->num_glyphs ) {
		int xp = x+offsets[i].x;
		int yp = y+offsets[i].y;
		if ( transform )
		    p->map( xp, yp, &xp, &yp );
		if (!glyphAttributes[i].zeroWidth && xp < SHRT_MAX && xp > SHRT_MIN)
		    XDrawString16(dpy, hd, gc, xp, yp, chars+i, 1 );
		advance_t adv = advances[i];
		// 	    qDebug("advance = %d/%d", adv.x, adv.y );
		x += adv;
		i++;
	    }
	} else {
	    // we can take a shortcut
	    int gl = 0;
	    while (gl < si->num_glyphs) {
		int toDraw = QMIN(64, si->num_glyphs-gl);
		int adv = 0;
		for (int i = gl; i < gl+toDraw; ++i)
		    adv += advances[i];
		if (x + adv < SHRT_MAX && x > SHRT_MIN)
		    XDrawString16(dpy, hd, gc, x, y, chars+gl, toDraw);
		gl += toDraw;
		x += adv;
	    }
	}
    }

    if ( chars != ch )
	free( chars );

    if ( textFlags != 0 )
	drawLines( p, this, yorig, xorig, x-xpos, textFlags );

#ifdef FONTENGINE_DEBUG
    x = xp;
    y = yp;
    p->save();
    p->setPen( Qt::red );
    for ( int i = 0; i < si->num_glyphs; i++ ) {
	glyph_metrics_t ci = boundingBox( glyphs[i] );
	p->drawRect( x + ci.x + offsets[i].x, y + 100 + ci.y + offsets[i].y, ci.width, ci.height );
	qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=(%d/%d)", i, ci.x, ci.y, ci.width, ci.height,
	       ci.xoff, ci.yoff, offsets[i].x, offsets[i].y,
	       advances[i].x, advances[i].y);
	x += advances[i].x;
	y += advances[i].y;
    }
    p->restore();
#endif
}

glyph_metrics_t QFontEngineXLFD::boundingBox( const glyph_t *glyphs, const advance_t *advances, const qoffset_t *offsets, int numGlyphs )
{
    int i;

    glyph_metrics_t overall;
    int ymax = 0;
    int xmax = 0;
    for (i = 0; i < numGlyphs; i++) {
	XCharStruct *xcs = charStruct( _fs, glyphs[i] );
	if (xcs) {
	    int x = overall.xoff + offsets[i].x - xcs->lbearing;
	    int y = overall.yoff + offsets[i].y - xcs->ascent;
	    overall.x = QMIN( overall.x, x );
	    overall.y = QMIN( overall.y, y );
	    xmax = QMAX( xmax, overall.xoff + offsets[i].x + xcs->rbearing );
	    ymax = QMAX( ymax, y + xcs->ascent + xcs->descent );
	    overall.xoff += qRound(advances[i]/_scale);
	} else {
	    int size = _fs->ascent;
	    overall.x = QMIN(overall.x, overall.xoff );
	    overall.y = QMIN(overall.y, overall.yoff - size );
	    ymax = QMAX( ymax, overall.yoff );
	    overall.xoff += size;
	    xmax = QMAX( xmax, overall.xoff );
	}
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;

    if ( _scale != 1. ) {
	overall.x = qRound(overall.x * _scale);
	overall.y = qRound(overall.y * _scale);
	overall.height = qRound(overall.height * _scale);
	overall.width = qRound(overall.width * _scale);
	overall.xoff = qRound(overall.xoff * _scale);
	overall.yoff = qRound(overall.yoff * _scale);
    }
    return overall;
}

glyph_metrics_t QFontEngineXLFD::boundingBox( glyph_t glyph )
{
    glyph_metrics_t gm;
    // ### scale missing!
    XCharStruct *xcs = charStruct( _fs, glyph );
    if (xcs) {
	gm = glyph_metrics_t( xcs->lbearing, -xcs->ascent, xcs->rbearing- xcs->lbearing, xcs->ascent + xcs->descent, xcs->width, 0 );
    } else {
	int size = _fs->ascent;
	gm = glyph_metrics_t( 0, size, size, size, size, 0 );
    }
    if ( _scale != 1. ) {
	gm.x = qRound(gm.x * _scale);
	gm.y = qRound(gm.y * _scale);
	gm.height = qRound(gm.height * _scale);
	gm.width = qRound(gm.width * _scale);
	gm.xoff = qRound(gm.xoff * _scale);
	gm.yoff = qRound(gm.yoff * _scale);
    }
    return gm;
}


int QFontEngineXLFD::ascent() const
{
    return qRound(_fs->ascent*_scale);
}

int QFontEngineXLFD::descent() const
{
    return qRound((_fs->descent-1)*_scale);
}

int QFontEngineXLFD::leading() const
{
    int l = qRound((QMIN(_fs->ascent, _fs->max_bounds.ascent)
		    + QMIN(_fs->descent, _fs->max_bounds.descent)) * _scale * 0.15 );
    return (l > 0) ? l : 1;
}

int QFontEngineXLFD::maxCharWidth() const
{
    return qRound(_fs->max_bounds.width*_scale);
}


// Loads the font for the specified script
static inline int maxIndex(XFontStruct *f) {
    return (((f->max_byte1 - f->min_byte1) *
	     (f->max_char_or_byte2 - f->min_char_or_byte2 + 1)) +
	    f->max_char_or_byte2 - f->min_char_or_byte2);
}

int QFontEngineXLFD::minLeftBearing() const
{
    if ( lbearing == SHRT_MIN ) {
	if ( _fs->per_char ) {
	    XCharStruct *cs = _fs->per_char;
	    int nc = maxIndex(_fs) + 1;
	    int mx = cs->lbearing;

	    for (int c = 1; c < nc; c++) {
		// ignore the bearings for characters whose ink is
		// completely outside the normal bounding box
		if ((cs[c].lbearing <= 0 && cs[c].rbearing <= 0) ||
		    (cs[c].lbearing >= cs[c].width && cs[c].rbearing >= cs[c].width))
		    continue;

		int nmx = cs[c].lbearing;

		if (nmx < mx)
		    mx = nmx;
	    }

	    ((QFontEngineXLFD *)this)->lbearing = mx;
	} else
	    ((QFontEngineXLFD *)this)->lbearing = _fs->min_bounds.lbearing;
    }
    return qRound (lbearing*_scale);
}

int QFontEngineXLFD::minRightBearing() const
{
    if ( rbearing == SHRT_MIN ) {
	if ( _fs->per_char ) {
	    XCharStruct *cs = _fs->per_char;
	    int nc = maxIndex(_fs) + 1;
	    int mx = cs->rbearing;

	    for (int c = 1; c < nc; c++) {
		// ignore the bearings for characters whose ink is
		// completely outside the normal bounding box
		if ((cs[c].lbearing <= 0 && cs[c].rbearing <= 0) ||
		    (cs[c].lbearing >= cs[c].width && cs[c].rbearing >= cs[c].width))
		    continue;

		int nmx = cs[c].rbearing;

		if (nmx < mx)
		    mx = nmx;
	    }

	    ((QFontEngineXLFD *)this)->rbearing = mx;
	} else
	    ((QFontEngineXLFD *)this)->rbearing = _fs->min_bounds.rbearing;
    }
    return qRound (rbearing*_scale);
}

int QFontEngineXLFD::cmap() const
{
    return _cmap;
}

const char *QFontEngineXLFD::name() const
{
    return _name;
}

bool QFontEngineXLFD::canRender( const QChar *string, int len )
{
    glyph_t glyphs[256];
    int nglyphs = 255;
    glyph_t *g = glyphs;
    if ( stringToCMap( string, len, g, 0, &nglyphs, FALSE ) == OutOfMemory ) {
	g = (glyph_t *)malloc( nglyphs*sizeof(glyph_t) );
	stringToCMap( string, len, g, 0, &nglyphs, FALSE );
    }

    bool allExist = TRUE;
    for ( int i = 0; i < nglyphs; i++ ) {
	if ( !g[i] || !charStruct( _fs, g[i] ) ) {
	    allExist = FALSE;
	    break;
	}
    }

    if ( g != glyphs )
	free( g );

    return allExist;
}


void QFontEngineXLFD::setScale( double scale )
{
    _scale = scale;
}


QFontEngine::Type QFontEngineXLFD::type() const
{
    return XLFD;
}


// ------------------------------------------------------------------
// LatinXLFD engine
// ------------------------------------------------------------------

static const int engine_array_inc = 4;

QFontEngineLatinXLFD::QFontEngineLatinXLFD( XFontStruct *xfs, const char *name,
					    int mib )
{
    _engines = new QFontEngine*[ engine_array_inc ];
    _engines[0] = new QFontEngineXLFD( xfs, name, mib );
    _count = 1;

    cache_cost = _engines[0]->cache_cost;

    memset( glyphIndices, 0, sizeof( glyphIndices ) );
    memset( glyphAdvances, 0, sizeof( glyphAdvances ) );
    euroIndex = 0;
    euroAdvance = 0;
}

QFontEngineLatinXLFD::~QFontEngineLatinXLFD()
{
    for ( int i = 0; i < _count; ++i ) {
	delete _engines[i];
	_engines[i] = 0;
    }
    delete [] _engines;
    _engines = 0;
}

void QFontEngineLatinXLFD::findEngine( const QChar &ch )
{
    if ( ch.unicode() == 0 ) return;

    static const char *alternate_encodings[] = {
	"iso8859-1",
	"iso8859-2",
	"iso8859-3",
	"iso8859-4",
	"iso8859-9",
	"iso8859-10",
	"iso8859-13",
	"iso8859-14",
	"iso8859-15",
	"hp-roman8"
    };
    static const int mib_count = sizeof( alternate_encodings ) / sizeof( const char * );

    // see if one of the above mibs can map the char we want
    QTextCodec *codec = 0;
    int which = -1;
    int i;
    for ( i = 0; i < mib_count; ++i ) {
	const int mib = qt_mib_for_xlfd_encoding( alternate_encodings[i] );
	bool skip = FALSE;
	for ( int e = 0; e < _count; ++e ) {
	    if ( _engines[e]->cmap() == mib ) {
		skip = TRUE;
		break;
	    }
	}
	if ( skip ) continue;

	codec = QTextCodec::codecForMib( mib );
	if ( codec && codec->canEncode( ch ) ) {
	    which = i;
	    break;
	}
    }

    if ( ! codec || which == -1 )
	return;

    const int enc_id = qt_xlfd_encoding_id( alternate_encodings[which] );
    QFontDef req = fontDef;
    QFontEngine *engine = QFontDatabase::findFont( QFont::Latin, 0, req, enc_id );
    if ( ! engine ) {
	req.family = QString::null;
	engine = QFontDatabase::findFont( QFont::Latin, 0, req, enc_id );
	if ( ! engine ) return;
    }
    engine->setScale( scale() );

    if ( ! ( _count % engine_array_inc ) ) {
	// grow the engines array
	QFontEngine **old = _engines;
	int new_size =
	    ( ( ( _count+engine_array_inc ) / engine_array_inc ) * engine_array_inc );
	_engines = new QFontEngine*[new_size];
	for ( i = 0; i < _count; ++i )
	    _engines[i] = old[i];
	delete [] old;
    }

    _engines[_count] = engine;
    const int hi = _count << 8;
    ++_count;

    unsigned short chars[0x201];
    glyph_t glyphs[0x201];
    advance_t advances[0x201];
    for ( i = 0; i < 0x200; ++i )
	chars[i] = i;
    chars[0x200] = 0x20ac;
    int glyphCount = 0x201;
    engine->stringToCMap( (const QChar *) chars, 0x201, glyphs, advances, &glyphCount, FALSE );

    // merge member data with the above
    for ( i = 0; i < 0x200; ++i ) {
	if ( glyphIndices[i] != 0 || glyphs[i] == 0 ) continue;
	glyphIndices[i] = glyphs[i] >= 0x2100 ? glyphs[i] : hi | glyphs[i];
	glyphAdvances[i] = advances[i];
    }
    if (!euroIndex && glyphs[0x200]) {
	euroIndex = hi | glyphs[0x200];
	euroAdvance = advances[0x200];
    }
}

QFontEngine::Error
QFontEngineLatinXLFD::stringToCMap( const QChar *str, int len, glyph_t *glyphs,
				    advance_t *advances, int *nglyphs, bool mirrored ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

    int i;
    bool missing = FALSE;
    const QChar *c = str+len;
    glyph_t *g = glyphs+len;
    if ( advances ) {
	int asc = ascent();
	advance_t *a = advances+len;
	if ( mirrored ) {
	    while ( c != str ) {
		--c;
		--g;
		--a;
		if ( c->unicode() < 0x200 ) {
		    unsigned short ch = ::mirroredChar(*c).unicode();
		    *g = glyphIndices[ch];
		    *a = glyphAdvances[ch];
		} else {
		    if ( c->unicode() == 0x20ac ) {
			*g = euroIndex;
			*a = euroAdvance;
		    } else {
			*g = 0;
			*a = asc;
		    }
		}
		missing = ( missing || ( *g == 0 ) );
	    }
	} else {
	    while ( c != str ) {
		--c;
		--g;
		--a;
		if ( c->unicode() < 0x200 ) {
		    *g = glyphIndices[c->unicode()];
		    *a = glyphAdvances[c->unicode()];
		} else {
		    if ( c->unicode() == 0x20ac ) {
			*g = euroIndex;
			*a = euroAdvance;
		    } else {
			*g = 0;
			*a = asc;
		    }
		}
		missing = ( missing || ( *g == 0 ) );
	    }
	}
    } else {
	if ( mirrored ) {
	    while ( c != str ) {
		--c;
		--g;
		*g = ( ( c->unicode() < 0x200 ) ? glyphIndices[::mirroredChar(*c).unicode()]
		       : (c->unicode() == 0x20ac) ? euroIndex : 0 );
		missing = ( missing || ( *g == 0 ) );
	    }
	} else {
	    while ( c != str ) {
		--c;
		--g;
		*g = ( ( c->unicode() < 0x200 ) ? glyphIndices[c->unicode()]
		       : (c->unicode() == 0x20ac) ? euroIndex : 0 );
		missing = ( missing || ( *g == 0 ) );
	    }
	}
    }

    if ( missing ) {
	for ( i = 0; i < len; ++i ) {
	    unsigned short uc = str[i].unicode();
	    if ( glyphs[i] != 0 || (uc >= 0x200 && uc != 0x20ac) )
		continue;

	    QFontEngineLatinXLFD *that = (QFontEngineLatinXLFD *) this;
	    that->findEngine( str[i] );
	    glyphs[i] = (uc == 0x20ac ? euroIndex : that->glyphIndices[uc]);
	    if ( advances )
		advances[i] = (uc == 0x20ac ? euroAdvance : glyphAdvances[uc]);
	}
    }

    *nglyphs = len;
    return NoError;
}

void QFontEngineLatinXLFD::draw( QPainter *p, int x, int y, const QTextEngine *engine,
				 const QScriptItem *si, int textFlags )
{
    if ( !si->num_glyphs ) return;

    glyph_t *glyphs = engine->glyphs( si );
    advance_t *advances = engine->advances( si );
    int which = glyphs[0] >> 8;
    if (which > 0x20)
        which = 0;

    int start = 0;
    int end, i;
    for ( end = 0; end < si->num_glyphs; ++end ) {
	int e = glyphs[end] >> 8;
        if (e > 0x20)
            e = 0;
	if ( e == which ) continue;

	// set the high byte to zero
        if (which != 0) {
            for ( i = start; i < end; ++i )
                glyphs[i] = glyphs[i] & 0xff;
        }

	// draw the text
	QScriptItem si2 = *si;
	si2.glyph_data_offset = si->glyph_data_offset + start;
	si2.num_glyphs = end - start;
	_engines[which]->draw( p, x, y, engine, &si2, textFlags );

	// reset the high byte for all glyphs and advance to the next sub-string
        const int hi = which << 8;
        for ( i = start; i < end; ++i ) {
            glyphs[i] = hi | glyphs[i];
            x += advances[i];
        }

	// change engine
	start = end;
	which = e;
    }

    // set the high byte to zero
    if (which != 0) {
        for ( i = start; i < end; ++i )
            glyphs[i] = glyphs[i] & 0xff;
    }
    // draw the text
    QScriptItem si2 = *si;
    si2.glyph_data_offset = si->glyph_data_offset + start;
    si2.num_glyphs = end - start;
    _engines[which]->draw( p, x, y, engine, &si2, textFlags );

    // reset the high byte for all glyphs
    if (which != 0) {
        const int hi = which << 8;
        for ( i = start; i < end; ++i )
            glyphs[i] = hi | glyphs[i];
    }
}

glyph_metrics_t QFontEngineLatinXLFD::boundingBox( const glyph_t *glyphs_const,
						   const advance_t *advances,
						   const qoffset_t *offsets,
						   int numGlyphs )
{
    if ( numGlyphs <= 0 ) return glyph_metrics_t();

    glyph_metrics_t overall;

    glyph_t *glyphs = (glyph_t *) glyphs_const;
    int which = glyphs[0] >> 8;
    if (which > 0x20)
        which = 0;

    int start = 0;
    int end, i;
    for ( end = 0; end < numGlyphs; ++end ) {
	int e = glyphs[end] >> 8;
        if (e > 0x20)
            e = 0;
	if ( e == which ) continue;

	// set the high byte to zero
        if (which != 0) {
            for ( i = start; i < end; ++i )
                glyphs[i] = glyphs[i] & 0xff;
        }

	// merge the bounding box for this run
	const glyph_metrics_t gm =
	    _engines[which]->boundingBox( glyphs + start,
					  advances + start,
					  offsets + start,
					  end - start );

	overall.x = QMIN( overall.x, gm.x );
	overall.y = QMIN( overall.y, gm.y );
	overall.width = overall.xoff + gm.width;
	overall.height = QMAX( overall.height + overall.y, gm.height + gm.y ) -
			 QMIN( overall.y, gm.y );
	overall.xoff += gm.xoff;
	overall.yoff += gm.yoff;

	// reset the high byte for all glyphs
        if (which != 0) {
            const int hi = which << 8;
            for ( i = start; i < end; ++i )
                glyphs[i] = hi | glyphs[i];
        }

	// change engine
	start = end;
	which = e;
    }

    // set the high byte to zero
    if (which != 0) {
        for ( i = start; i < end; ++i )
            glyphs[i] = glyphs[i] & 0xff;
    }

    // merge the bounding box for this run
    const glyph_metrics_t gm =
	_engines[which]->boundingBox( glyphs + start,
				      advances + start,
				      offsets + start,
				      end - start );

    overall.x = QMIN( overall.x, gm.x );
    overall.y = QMIN( overall.y, gm.y );
    overall.width = overall.xoff + gm.width;
    overall.height = QMAX( overall.height + overall.y, gm.height + gm.y ) -
		     QMIN( overall.y, gm.y );
    overall.xoff += gm.xoff;
    overall.yoff += gm.yoff;

    // reset the high byte for all glyphs
    if (which != 0) {
        const int hi = which << 8;
        for ( i = start; i < end; ++i )
            glyphs[i] = hi | glyphs[i];
    }

    return overall;
}

glyph_metrics_t QFontEngineLatinXLFD::boundingBox( glyph_t glyph )
{
    int engine = glyph >> 8;
    if (engine > 0x20)
        engine = 0;
    Q_ASSERT( engine < _count );
    return _engines[engine]->boundingBox( engine > 0 ? glyph & 0xff : glyph );
}

int QFontEngineLatinXLFD::ascent() const
{
    return _engines[0]->ascent();
}

int QFontEngineLatinXLFD::descent() const
{
    return _engines[0]->descent();
}

int QFontEngineLatinXLFD::leading() const
{
    return _engines[0]->leading();
}

int QFontEngineLatinXLFD::maxCharWidth() const
{
    return _engines[0]->maxCharWidth();
}

int QFontEngineLatinXLFD::minLeftBearing() const
{
    return _engines[0]->minLeftBearing();
}

int QFontEngineLatinXLFD::minRightBearing() const
{
    return _engines[0]->minRightBearing();
}

const char *QFontEngineLatinXLFD::name() const
{
    return _engines[0]->name();
}

bool QFontEngineLatinXLFD::canRender( const QChar *string, int len )
{
    bool all = TRUE;
    int i;
    for ( i = 0; i < len; ++i ) {
	if ( string[i].unicode() >= 0x200 ||
	     glyphIndices[string[i].unicode()] == 0 ) {
	    if (string[i].unicode() != 0x20ac || euroIndex == 0)
		all = FALSE;
	    break;
	}
    }

    if ( all )
	return TRUE;

    all = TRUE;
    for ( i = 0; i < len; ++i ) {
	if ( string[i].unicode() >= 0x200 ) {
	    if (string[i].unicode() == 0x20ac) {
		if (euroIndex)
		    continue;

		findEngine(string[i]);
		if (euroIndex)
		    continue;
	    }
	    all = FALSE;
	    break;
	}
	if ( glyphIndices[string[i].unicode()] != 0 ) continue;

	findEngine( string[i] );
	if ( glyphIndices[string[i].unicode()] == 0 ) {
	    all = FALSE;
	    break;
	}
    }

    return all;
}

void QFontEngineLatinXLFD::setScale( double scale )
{
    int i;
    for ( i = 0; i < _count; ++i )
	_engines[i]->setScale( scale );
    unsigned short chars[0x200];
    for ( i = 0; i < 0x200; ++i )
	chars[i] = i;
    int glyphCount = 0x200;
    _engines[0]->stringToCMap( (const QChar *)chars, 0x200,
			       glyphIndices, glyphAdvances, &glyphCount, FALSE );
}


// ------------------------------------------------------------------
// Xft cont engine
// ------------------------------------------------------------------
// #define FONTENGINE_DEBUG

#ifndef QT_NO_XFTFREETYPE
class Q_HackPaintDevice : public QPaintDevice
{
public:
    inline Q_HackPaintDevice() : QPaintDevice( 0 ) {}
    inline XftDraw *xftDrawHandle() const {
	return (XftDraw *)rendhd;
    }

};

#ifdef QT_XFT2
static inline void getGlyphInfo( XGlyphInfo *xgi, XftFont *font, int glyph )
{
    FT_UInt x = glyph;
    XftGlyphExtents( QPaintDevice::x11AppDisplay(), font, &x, 1, xgi );
}
#else
static inline XftFontStruct *getFontStruct( XftFont *font )
{
    if (font->core)
	return 0;
    return font->u.ft.font;
}

static inline void getGlyphInfo(XGlyphInfo *xgi, XftFont *font, int glyph)
{

    XftTextExtents32(QPaintDevice::x11AppDisplay(), font, (XftChar32 *) &glyph, 1, xgi);
}
#endif // QT_XFT2

static inline FT_Face lockFTFace( XftFont *font )
{
#ifdef QT_XFT2
    return XftLockFace( font );
#else
    if (font->core) return 0;
    return font->u.ft.font->face;
#endif // QT_XFT2
}

static inline void unlockFTFace( XftFont *font )
{
#ifdef QT_XFT2
    XftUnlockFace( font );
#else
    Q_UNUSED( font );
#endif // QT_XFT2
}



QFontEngineXft::QFontEngineXft( XftFont *font, XftPattern *pattern, int cmap )
    : _font( font ), _pattern( pattern ), _openType( 0 ), _cmap( cmap )
{
    _face = lockFTFace( _font );

#ifndef QT_XFT2
    XftFontStruct *xftfs = getFontStruct( _font );
    if ( xftfs ) {
	// dirty hack: we set the charmap in the Xftfreetype to -1, so
	// XftFreetype assumes no encoding and really draws glyph
	// indices. The FT_Face still has the Unicode encoding to we
	// can convert from Unicode to glyph index
	xftfs->charmap = -1;
    }
#else
    _cmap = -1;
    // Xft maps Unicode and adobe roman for us.
    for (int i = 0; i < _face->num_charmaps; ++i) {
        FT_CharMap cm = _face->charmaps[i];
//          qDebug("font has charmap %x", cm->encoding);
        if (cm->encoding == ft_encoding_adobe_custom
            || cm->encoding == ft_encoding_symbol) {
//              qDebug("font has adobe custom or ms symbol charmap");
            _cmap = i;
            break;
        }
    }
#endif // QT_XFT2


    cache_cost = _font->height * _font->max_advance_width *
		 ( _face ? _face->num_glyphs : 1024 );

    // if the Xft font is not antialiased, it uses bitmaps instead of
    // 8-bit alpha maps... adjust the cache_cost to reflect this
    Bool antialiased = TRUE;
    if ( XftPatternGetBool( pattern, XFT_ANTIALIAS,
			    0, &antialiased ) == XftResultMatch &&
	 ! antialiased ) {
	cache_cost /= 8;
    }
    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;

    memset( widthCache, 0, sizeof(widthCache) );
    memset( cmapCache, 0, sizeof(cmapCache) );
}

QFontEngineXft::~QFontEngineXft()
{
    delete _openType;
    unlockFTFace( _font );

    XftFontClose( QPaintDevice::x11AppDisplay(),_font );
    XftPatternDestroy( _pattern );
    _font = 0;
    _pattern = 0;
    TransformedFont *trf = transformed_fonts;
    while ( trf ) {
	XftFontClose( QPaintDevice::x11AppDisplay(), trf->xft_font );
	TransformedFont *tmp = trf;
	trf = trf->next;
	delete tmp;
    }
}

#ifdef QT_XFT2
static glyph_t getAdobeCharIndex(XftFont *font, int cmap, uint ucs4)
{
    FT_Face _face = XftLockFace( font );
    FT_Set_Charmap(_face, _face->charmaps[cmap]);
    glyph_t g = FT_Get_Char_Index(_face, ucs4);
    XftUnlockFace(font);
    return g;
}
#endif

QFontEngine::Error QFontEngineXft::stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool mirrored ) const
{
    if ( *nglyphs < len ) {
	*nglyphs = len;
	return OutOfMemory;
    }

#ifdef QT_XFT2
    if (_cmap != -1) {
	for ( int i = 0; i < len; ++i ) {
	    unsigned short uc = str[i].unicode();
            if (mirrored)
                uc = ::mirroredChar(str[i]).unicode();
	    glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
	    if ( !glyphs[i] ) {
		glyph_t glyph = XftCharIndex(0, _font, uc);
                if (!glyph)
                    glyph = getAdobeCharIndex(_font, _cmap, uc);
		glyphs[i] = glyph;
		if ( uc < cmapCacheSize )
		    ((QFontEngineXft *)this)->cmapCache[uc] = glyph;
	    }
	}
    } else if ( mirrored ) {
	for ( int i = 0; i < len; ++i ) {
	    unsigned short uc = ::mirroredChar(str[i]).unicode();
	    glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
	    if ( !glyphs[i] ) {
		if (uc == 0xa0)
		    uc = 0x20;
		glyph_t glyph = XftCharIndex(0, _font, uc);
		glyphs[i] = glyph;
		if ( uc < cmapCacheSize )
		    ((QFontEngineXft *)this)->cmapCache[uc] = glyph;
	    }
	}
    } else {
	for ( int i = 0; i < len; ++i ) {
	    unsigned short uc = str[i].unicode();
	    glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
	    if ( !glyphs[i] ) {
		if (uc == 0xa0)
		    uc = 0x20;
		glyph_t glyph = XftCharIndex(0, _font, uc);
		glyphs[i] = glyph;
		if ( uc < cmapCacheSize )
		    ((QFontEngineXft *)this)->cmapCache[uc] = glyph;
	    }
	}
    }

    if ( advances ) {
	for ( int i = 0; i < len; i++ ) {
	    FT_UInt glyph = *(glyphs + i);
	    advances[i] = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
	    if ( !advances[i] ) {
		XGlyphInfo gi;
		XftGlyphExtents( QPaintDevice::x11AppDisplay(), _font, &glyph, 1, &gi );
		advances[i] = gi.xOff;
		if ( glyph < widthCacheSize && gi.xOff > 0 && gi.xOff < 0x100 )
		    ((QFontEngineXft *)this)->widthCache[glyph] = gi.xOff;
	    }
	}
	if ( _scale != 1. ) {
	    for ( int i = 0; i < len; i++ )
		advances[i] = qRound(advances[i]*_scale);
	}
    }
#else
    if ( !_face ) {
	if ( mirrored ) {
	    for ( int i = 0; i < len; i++ )
		glyphs[i] = ::mirroredChar(str[i]).unicode();
	} else {
	    for ( int i = 0; i < len; i++ )
		glyphs[i] = str[i].unicode();
	}
    } else {
	if ( _cmap == 1 ) {
	    // symbol font
	    for ( int i = 0; i < len; i++ ) {
		unsigned short uc = str[i].unicode();
		glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
		if ( !glyphs[i] ) {
		    glyph_t glyph = FT_Get_Char_Index( _face, uc );
		    if(!glyph && uc < 0x100)
			glyph = FT_Get_Char_Index( _face, uc+0xf000 );
		    glyphs[i] = glyph;
		    if ( uc < cmapCacheSize )
			((QFontEngineXft *)this)->cmapCache[uc] = glyph;
		}
	    }
	} else if ( mirrored ) {
	    for ( int i = 0; i < len; i++ ) {
		unsigned short uc = ::mirroredChar(str[i]).unicode();
		glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
		if ( !glyphs[i] ) {
		    glyph_t glyph = FT_Get_Char_Index( _face, uc );
		    glyphs[i] = glyph;
		    if ( uc < cmapCacheSize )
			((QFontEngineXft *)this)->cmapCache[uc] = glyph;
		}
	    }
	} else {
	    for ( int i = 0; i < len; i++ ) {
		unsigned short uc = str[i].unicode();
		glyphs[i] = uc < cmapCacheSize ? cmapCache[uc] : 0;
		if ( !glyphs[i] ) {
		    glyph_t glyph = FT_Get_Char_Index( _face, uc );
		    glyphs[i] = glyph;
		    if ( uc < cmapCacheSize )
			((QFontEngineXft *)this)->cmapCache[uc] = glyph;
		}
	    }
	}
    }

    if ( advances ) {
	for ( int i = 0; i < len; i++ ) {
	    XftChar16 glyph = *(glyphs + i);
	    advances[i] = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
	    if ( !advances[i] ) {
		XGlyphInfo gi;
		XftTextExtents16(QPaintDevice::x11AppDisplay(), _font, &glyph, 1, &gi);
		advances[i] = gi.xOff;
		if ( glyph < widthCacheSize && gi.xOff > 0 && gi.xOff < 0x100 )
		    ((QFontEngineXft *)this)->widthCache[glyph] = gi.xOff;
	    }
	}
	if ( _scale != 1. ) {
	    for ( int i = 0; i < len; i++ )
		advances[i] = qRound(advances[i]*_scale);
	}
    }
#endif // QT_XFT2

    *nglyphs = len;
    return NoError;
}


void QFontEngineXft::recalcAdvances( int len, glyph_t *glyphs, advance_t *advances )
{

#ifdef QT_XFT2
    for ( int i = 0; i < len; i++ ) {
	FT_UInt glyph = *(glyphs + i);
	advances[i] = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
	if ( !advances[i] ) {
	    XGlyphInfo gi;
	    XftGlyphExtents( QPaintDevice::x11AppDisplay(), _font, &glyph, 1, &gi );
	    advances[i] = gi.xOff;
	    if ( glyph < widthCacheSize && gi.xOff > 0 && gi.xOff < 0x100 )
		((QFontEngineXft *)this)->widthCache[glyph] = gi.xOff;
	}
	if ( _scale != 1. ) {
	    for ( int i = 0; i < len; i++ )
		advances[i] = qRound(advances[i]*_scale);
	}
    }
#else
    for ( int i = 0; i < len; i++ ) {
	XftChar16 glyph = *(glyphs + i);
	advances[i] = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
	if ( !advances[i] ) {
	    XGlyphInfo gi;
	    XftTextExtents16(QPaintDevice::x11AppDisplay(), _font, &glyph, 1, &gi);
	    advances[i] = gi.xOff;
	    if ( glyph < widthCacheSize && gi.xOff > 0 && gi.xOff < 0x100 )
		((QFontEngineXft *)this)->widthCache[glyph] = gi.xOff;
	}
    }
    if ( _scale != 1. ) {
	for ( int i = 0; i < len; i++ )
	    advances[i] = qRound(advances[i]*_scale);
    }
#endif // QT_XFT2
}

//#define FONTENGINE_DEBUG
void QFontEngineXft::draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags )
{
    if ( !si->num_glyphs )
	return;

    Display *dpy = QPaintDevice::x11AppDisplay();

    int xorig = x;
    int yorig = y;

    GlyphAttributes *glyphAttributes = engine->glyphAttributes( si );

    XftFont *fnt = _font;
    bool transform = FALSE;
    if ( p->txop >= QPainter::TxScale || p->rop != Qt::CopyROP || _scale < 0.9999 || _scale > 1.001) {
        bool can_scale = (_face->face_flags & FT_FACE_FLAG_SCALABLE) && p->rop == Qt::CopyROP;
        double size = (p->m11()*p->m22() - p->m12()*p->m21())*_scale*_scale*fontDef.pixelSize*fontDef.pixelSize;
        if (size > 256*256 || _scale < .9999 || _scale > 1.001)
            can_scale = FALSE;
	if (!can_scale) {
	    // font doesn't support transformations, need to do it by hand
            float tmp = _scale;
            _scale = 1.;
            drawScaled(x, y, engine, si, textFlags, dpy, p->gc, p->device(), this, p->xmat, tmp);
            _scale = tmp;
	    return;
	}

	XftMatrix *mat = 0;
	XftPatternGetMatrix( _pattern, XFT_MATRIX, 0, &mat );
	XftMatrix m2;
	m2.xx = p->m11()*_scale;
	m2.xy = -p->m21()*_scale;
	m2.yx = -p->m12()*_scale;
	m2.yy = p->m22()*_scale;

	// check if we have it cached
	TransformedFont *trf = transformed_fonts;
	TransformedFont *prev = 0;
	int i = 0;
	while ( trf ) {
	    if ( trf->xx == (float)m2.xx &&
		 trf->xy == (float)m2.xy &&
		 trf->yx == (float)m2.yx &&
		 trf->yy == (float)m2.yy )
		break;
	    TransformedFont *tmp = trf;
	    trf = trf->next;
	    if (i > 10) {
		XftFontClose( QPaintDevice::x11AppDisplay(), tmp->xft_font );
		delete tmp;
		prev->next = trf;
	    } else {
		prev = tmp;
	    }
	    ++i;
	}
	if ( trf ) {
	    if ( prev ) {
		// move to beginning of list
		prev->next = trf->next;
		trf->next = transformed_fonts;
		transformed_fonts = trf;
	    }
	    fnt = trf->xft_font;
	} else {
	    if ( mat )
		XftMatrixMultiply( &m2, &m2, mat );

            XftPattern *pattern = XftPatternDuplicate( _pattern );
	    XftPatternDel( pattern, XFT_MATRIX );
	    XftPatternAddMatrix( pattern, XFT_MATRIX, &m2 );

	    fnt = XftFontOpenPattern( dpy, pattern );
#ifndef QT_XFT2
	    XftFontStruct *xftfs = getFontStruct( fnt );
	    if ( xftfs ) {
		// dirty hack: we set the charmap in the Xftfreetype to -1, so
		// XftFreetype assumes no encoding and really draws glyph
		// indices. The FT_Face still has the Unicode encoding to we
		// can convert from Unicode to glyph index
		xftfs->charmap = -1;
	    }
#endif // QT_XFT2
	    TransformedFont *trf = new TransformedFont;
	    trf->xx = (float)m2.xx;
	    trf->xy = (float)m2.xy;
	    trf->yx = (float)m2.yx;
	    trf->yy = (float)m2.yy;
	    trf->xft_font = fnt;
	    trf->next = transformed_fonts;
	    transformed_fonts = trf;
	}
	transform = TRUE;
    } else if ( p->txop == QPainter::TxTranslate ) {
	p->map( x, y, &x, &y );
    }

    glyph_t *glyphs = engine->glyphs( si );
    advance_t *advances = engine->advances( si );
    qoffset_t *offsets = engine->offsets( si );

    const QColor &pen = p->cpen.color();
    XftDraw *draw = ((Q_HackPaintDevice *)p->pdev)->xftDrawHandle();

    XftColor col;
    col.color.red = pen.red () | pen.red() << 8;
    col.color.green = pen.green () | pen.green() << 8;
    col.color.blue = pen.blue () | pen.blue() << 8;
    col.color.alpha = 0xffff;
    col.pixel = pen.pixel();
#ifdef FONTENGINE_DEBUG
    qDebug("===== drawing %d glyphs reverse=%s ======", si->num_glyphs, si->analysis.bidiLevel % 2?"TRUE":"FALSE" );
    p->save();
    p->setBrush( Qt::white );
    glyph_metrics_t ci = boundingBox( glyphs, advances, offsets, si->num_glyphs );
    p->drawRect( x + ci.x, y + ci.y, ci.width, ci.height );
    p->drawRect( x + ci.x, y + 100 + ci.y, ci.width, ci.height );
    qDebug("bounding rect=%d %d (%d/%d)", ci.x, ci.y, ci.width, ci.height );
    p->restore();
    int yp = y;
    int xp = x;
#endif

    if ( textFlags != 0 )
	drawLines( p, this, yorig, xorig, si->width, textFlags );


    if ( si->isSpace )
	return;

    if ( transform || si->hasPositioning ) {
	if ( si->analysis.bidiLevel % 2 ) {
	    int i = si->num_glyphs;
	    while( i-- ) {
		int xp = x + offsets[i].x;
		int yp = y + offsets[i].y;
		if ( transform )
		    p->map( xp, yp, &xp, &yp );
#ifdef QT_XFT2
		FT_UInt glyph = *(glyphs + i);
		if (!glyphAttributes[i].zeroWidth && xp < SHRT_MAX && xp > SHRT_MIN)
		    XftDrawGlyphs( draw, &col, fnt, xp, yp, &glyph, 1 );
#else
		if (!glyphAttributes[i].zeroWidth && xp < SHRT_MAX && xp > SHRT_MIN)
		    XftDrawString16( draw, &col, fnt, xp, yp, (XftChar16 *) (glyphs+i), 1);
#endif // QT_XFT2
#ifdef FONTENGINE_DEBUG
		glyph_metrics_t gi = boundingBox( glyphs[i] );
		p->drawRect( x+offsets[i].x+gi.x, y+offsets[i].y+100+gi.y, gi.width, gi.height );
		p->drawLine( x+offsets[i].x, y + 150 + 5*i , x+offsets[i].x+advances[i], y + 150 + 5*i );
		p->drawLine( x+offsets[i].x, y + 152 + 5*i , x+offsets[i].x+gi.xoff, y + 152 + 5*i );
		qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=%d", i, gi.x, gi.y, gi.width, gi.height,
		       gi.xoff, gi.yoff, offsets[i].x, offsets[i].y, advances[i]);
#endif
		x += advances[i];
	    }
	} else {
	    int i = 0;
	    while ( i < si->num_glyphs ) {
		int xp = x + offsets[i].x;
		int yp = y + offsets[i].y;
		if ( transform )
		    p->map( xp, yp, &xp, &yp );
#ifdef QT_XFT2
		FT_UInt glyph = *(glyphs + i);
		if (!glyphAttributes[i].zeroWidth && xp < SHRT_MAX && xp > SHRT_MIN)
		    XftDrawGlyphs( draw, &col, fnt, xp, yp, &glyph, 1 );
#else
		if (!glyphAttributes[i].zeroWidth && xp < SHRT_MAX && xp > SHRT_MIN)
		    XftDrawString16( draw, &col, fnt, xp, yp, (XftChar16 *) (glyphs+i), 1 );
#endif // QT_XFT2
		// 	    qDebug("advance = %d/%d", adv.x, adv.y );
		x += advances[i];
		i++;
	    }
	}
    } else {
	// Xft has real trouble drawing the glyphs on their own.
	// Drawing them as one string increases performance significantly.
#ifdef QT_XFT2
	// #### we should use a different method anyways on Xft2
	FT_UInt g[64];
	int gl = 0;
	while (gl < si->num_glyphs) {
	    int toDraw = QMIN(64, si->num_glyphs-gl);
	    int adv = 0;
	    if ( si->analysis.bidiLevel % 2 ) {
		for ( int i = 0; i < toDraw; i++ ) {
		    g[i] = glyphs[si->num_glyphs-1-(gl+i)];
		    adv += advances[si->num_glyphs-1-(gl+i)];
		}
	    } else {
		for ( int i = 0; i < toDraw; i++ ) {
		    g[i] = glyphs[gl+i];
		    adv += advances[gl+i];
		}
	    }
	    if (x + adv < SHRT_MAX && x > SHRT_MIN)
		XftDrawGlyphs( draw, &col, fnt, x, y, g, toDraw );
	    gl += toDraw;
	    x += adv;
	}
#else
	XftChar16 g[64];
	int gl = 0;
	while (gl < si->num_glyphs) {
	    int toDraw = QMIN(64, si->num_glyphs-gl);
	    int adv = 0;
	    if ( si->analysis.bidiLevel % 2 ) {
		for ( int i = 0; i < toDraw; i++ ) {
		    g[i] = glyphs[si->num_glyphs-1-(gl+i)];
		    adv += advances[si->num_glyphs-1-(gl+i)];
		}
	    } else {
		for ( int i = 0; i < toDraw; i++ ) {
		    g[i] = glyphs[gl+i];
		    adv += advances[gl+i];
		}
	    }
	    if (x + adv < SHRT_MAX && x > SHRT_MIN)
		XftDrawString16( draw, &col, fnt, x, y, g, toDraw );
	    gl += toDraw;
	    x += adv;
	}
#endif // QT_XFT2
    }

#ifdef FONTENGINE_DEBUG
    if ( !si->analysis.bidiLevel % 2 ) {
	x = xp;
	y = yp;
	p->save();
	p->setPen( Qt::red );
	for ( int i = 0; i < si->num_glyphs; i++ ) {
	    glyph_metrics_t ci = boundingBox( glyphs[i] );
	    p->drawRect( x + ci.x + offsets[i].x, y + 100 + ci.y + offsets[i].y, ci.width, ci.height );
	    qDebug("bounding ci[%d]=%d %d (%d/%d) / %d %d   offs=(%d/%d) advance=%d", i, ci.x, ci.y, ci.width, ci.height,
		   ci.xoff, ci.yoff, offsets[i].x, offsets[i].y, advances[i]);
	    x += advances[i];
	}
	p->restore();
    }
#endif
}

glyph_metrics_t QFontEngineXft::boundingBox( const glyph_t *glyphs, const advance_t *advances, const qoffset_t *offsets, int numGlyphs )
{
    XGlyphInfo xgi;

    glyph_metrics_t overall;
    int ymax = 0;
    int xmax = 0;
    if (_scale != 1) {
        for (int i = 0; i < numGlyphs; i++) {
            getGlyphInfo( &xgi, _font, glyphs[i] );
            int x = overall.xoff + offsets[i].x - xgi.x;
            int y = overall.yoff + offsets[i].y - xgi.y;
            overall.x = QMIN( overall.x, x );
            overall.y = QMIN( overall.y, y );
            xmax = QMAX( xmax, x + xgi.width );
            ymax = QMAX( ymax, y + xgi.height );
            overall.xoff += qRound(advances[i]/_scale);
        }
	overall.x = qRound(overall.x * _scale);
	overall.y = qRound(overall.y * _scale);
	overall.xoff = qRound(overall.xoff * _scale);
	overall.yoff = qRound(overall.yoff * _scale);
	xmax = qRound(xmax * _scale);
	ymax = qRound(ymax * _scale);
    } else {
        for (int i = 0; i < numGlyphs; i++) {
            getGlyphInfo( &xgi, _font, glyphs[i] );
            int x = overall.xoff + offsets[i].x - xgi.x;
            int y = overall.yoff + offsets[i].y - xgi.y;
            overall.x = QMIN( overall.x, x );
            overall.y = QMIN( overall.y, y );
            xmax = QMAX( xmax, x + xgi.width );
            ymax = QMAX( ymax, y + xgi.height );
            overall.xoff += advances[i];
        }
    }
    overall.height = ymax - overall.y;
    overall.width = xmax - overall.x;
    return overall;
}

glyph_metrics_t QFontEngineXft::boundingBox( glyph_t glyph )
{
    XGlyphInfo xgi;
    getGlyphInfo( &xgi, _font, glyph );
    glyph_metrics_t gm = glyph_metrics_t( -xgi.x, -xgi.y, xgi.width, xgi.height, xgi.xOff, -xgi.yOff );
    if ( _scale != 1. ) {
	gm.x = qRound(gm.x * _scale);
	gm.y = qRound(gm.y * _scale);
	gm.height = qRound(gm.height * _scale);
	gm.width = qRound(gm.width * _scale);
	gm.xoff = qRound(gm.xoff * _scale);
	gm.yoff = qRound(gm.yoff * _scale);
    }
    return gm;
}



int QFontEngineXft::ascent() const
{
    return qRound(_font->ascent*_scale);
}

int QFontEngineXft::descent() const
{
    return qRound((_font->descent-1)*_scale);
}

// #### use Freetype to determine this
int QFontEngineXft::leading() const
{
    int l = qRound(QMIN( _font->height - (_font->ascent + _font->descent),
			((_font->ascent + _font->descent) >> 4)*_scale ));
    return (l > 0) ? l : 1;
}

// #### use Freetype to determine this
int QFontEngineXft::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if ( lw < 2 && score >= 1050 ) lw = 2;
    if ( lw == 0 ) lw = 1;

    return lw;
}

// #### use Freetype to determine this
int QFontEngineXft::underlinePosition() const
{
    int pos = ( ( lineThickness() * 2 ) + 3 ) / 6;
    return pos ? pos : 1;
}

int QFontEngineXft::maxCharWidth() const
{
    return qRound(_font->max_advance_width*_scale);
}

static const ushort char_table[] = {
	40,
	67,
	70,
	75,
	86,
	88,
	89,
	91,
	102,
	114,
	124,
	127,
	205,
	645,
	884,
	922,
	1070,
	12386
};

static const int char_table_entries = sizeof(char_table)/sizeof(ushort);


int QFontEngineXft::minLeftBearing() const
{
    if ( lbearing == SHRT_MIN )
	minRightBearing(); // calculates both

    return lbearing;
}

int QFontEngineXft::minRightBearing() const
{
    if ( rbearing == SHRT_MIN ) {
	QFontEngineXft *that = (QFontEngineXft *)this;
	that->lbearing = that->rbearing = 0;
	QChar *ch = (QChar *)char_table;
	glyph_t glyphs[char_table_entries];
	int ng = char_table_entries;
	stringToCMap(ch, char_table_entries, glyphs, 0, &ng, FALSE);
	while (--ng) {
	    if (glyphs[ng]) {
		glyph_metrics_t gi = that->boundingBox( glyphs[ng] );
		if (gi.xoff) {
		    that->lbearing = QMIN(lbearing, gi.x);
		    that->rbearing = QMIN(rbearing, gi.xoff - gi.x - gi.width);
		}
	    }
	}
    }

    return rbearing;
}

int QFontEngineXft::cmap() const
{
    return _cmap;
}

const char *QFontEngineXft::name() const
{
    return "xft";
}

void QFontEngineXft::setScale( double scale )
{
    _scale = scale;
}

bool QFontEngineXft::canRender( const QChar *string, int len )
{
    bool allExist = TRUE;

#ifdef QT_XFT2
    if (_cmap != -1) {
        for ( int i = 0; i < len; i++ ) {
            if (!XftCharExists(0, _font, string[i].unicode())
                && getAdobeCharIndex(_font, _cmap, string[i].unicode()) == 0) {
                allExist = FALSE;
                break;
            }
        }
    } else {
        for ( int i = 0; i < len; i++ ) {
            if (!XftCharExists(0, _font, string[i].unicode())) {
                allExist = FALSE;
                break;
            }
        }
    }
#else
    glyph_t glyphs[256];
    int nglyphs = 255;
    glyph_t *g = glyphs;
    if ( stringToCMap( string, len, g, 0, &nglyphs, FALSE ) == OutOfMemory ) {
	g = (glyph_t *)malloc( nglyphs*sizeof(glyph_t) );
	stringToCMap( string, len, g, 0, &nglyphs, FALSE );
    }

    for ( int i = 0; i < nglyphs; i++ ) {
	if ( !XftGlyphExists(QPaintDevice::x11AppDisplay(), _font, g[i]) ) {
	    allExist = FALSE;
	    break;
	}
    }

    if ( g != glyphs )
	free( g );
#endif // QT_XFT2

    return allExist;
}

QOpenType *QFontEngineXft::openType() const
{
//     qDebug("openTypeIface requested!");
    if ( _openType )
	return _openType;

    if ( !_face || ! FT_IS_SFNT( _face ) )
	return 0;

    QFontEngineXft *that = (QFontEngineXft *)this;
    that->_openType = new QOpenType(that);
    return _openType;
}


QFontEngine::Type QFontEngineXft::type() const
{
    return Xft;
}
#endif


//  --------------------------------------------------------------------------------------------------------------------
// Open type support
//  --------------------------------------------------------------------------------------------------------------------

#ifndef QT_NO_XFTFREETYPE

#include "qscriptengine_p.h"

//#define OT_DEBUG

#ifdef OT_DEBUG
static inline char *tag_to_string(FT_ULong tag)
{
    static char string[5];
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
    return string;
}
#endif

#define DefaultLangSys 0xffff
#define DefaultScript FT_MAKE_TAG('D', 'F', 'L', 'T')

enum {
    RequiresGsub = 1,
    RequiresGpos = 2
};

struct OTScripts {
    unsigned int tag;
    int flags;
};

static const OTScripts ot_scripts [] = {
// 	// European Alphabetic Scripts
// 	Latin,
    { FT_MAKE_TAG( 'l', 'a', 't', 'n' ), 0 },
// 	Greek,
    { FT_MAKE_TAG( 'g', 'r', 'e', 'k' ), 0 },
// 	Cyrillic,
    { FT_MAKE_TAG( 'c', 'y', 'r', 'l' ), 0 },
// 	Armenian,
    { FT_MAKE_TAG( 'a', 'r', 'm', 'n' ), 0 },
// 	Georgian,
    { FT_MAKE_TAG( 'g', 'e', 'o', 'r' ), 0 },
// 	Runic,
    { FT_MAKE_TAG( 'r', 'u', 'n', 'r' ), 0 },
// 	Ogham,
    { FT_MAKE_TAG( 'o', 'g', 'a', 'm' ), 0 },
// 	SpacingModifiers,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), 0 },
// 	CombiningMarks,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), 0 },

// 	// Middle Eastern Scripts
// 	Hebrew,
    { FT_MAKE_TAG( 'h', 'e', 'b', 'r' ), 1 },
// 	Arabic,
    { FT_MAKE_TAG( 'a', 'r', 'a', 'b' ), 1 },
// 	Syriac,
    { FT_MAKE_TAG( 's', 'y', 'r', 'c' ), 1 },
// 	Thaana,
    { FT_MAKE_TAG( 't', 'h', 'a', 'a' ), 1 },

// 	// South and Southeast Asian Scripts
// 	Devanagari,
    { FT_MAKE_TAG( 'd', 'e', 'v', 'a' ), 1 },
// 	Bengali,
    { FT_MAKE_TAG( 'b', 'e', 'n', 'g' ), 1 },
// 	Gurmukhi,
    { FT_MAKE_TAG( 'g', 'u', 'r', 'u' ), 1 },
// 	Gujarati,
    { FT_MAKE_TAG( 'g', 'u', 'j', 'r' ), 1 },
// 	Oriya,
    { FT_MAKE_TAG( 'o', 'r', 'y', 'a' ), 1 },
// 	Tamil,
    { FT_MAKE_TAG( 't', 'a', 'm', 'l' ), 1 },
// 	Telugu,
    { FT_MAKE_TAG( 't', 'e', 'l', 'u' ), 1 },
// 	Kannada,
    { FT_MAKE_TAG( 'k', 'n', 'd', 'a' ), 1 },
// 	Malayalam,
    { FT_MAKE_TAG( 'm', 'l', 'y', 'm' ), 1 },
// 	Sinhala,
    // ### could not find any OT specs on this
    { FT_MAKE_TAG( 's', 'i', 'n', 'h' ), 1 },
// 	Thai,
    { FT_MAKE_TAG( 't', 'h', 'a', 'i' ), 1 },
// 	Lao,
    { FT_MAKE_TAG( 'l', 'a', 'o', ' ' ), 1 },
// 	Tibetan,
    { FT_MAKE_TAG( 't', 'i', 'b', 't' ), 1 },
// 	Myanmar,
    { FT_MAKE_TAG( 'm', 'y', 'm', 'r' ), 1 },
// 	Khmer,
    { FT_MAKE_TAG( 'k', 'h', 'm', 'r' ), 1 },

// 	// East Asian Scripts
// 	Han,
    { FT_MAKE_TAG( 'h', 'a', 'n', 'i' ), 0 },
// 	Hiragana,
    { FT_MAKE_TAG( 'k', 'a', 'n', 'a' ), 0 },
// 	Katakana,
    { FT_MAKE_TAG( 'k', 'a', 'n', 'a' ), 0 },
// 	Hangul,
    { FT_MAKE_TAG( 'h', 'a', 'n', 'g' ), 1 },
// 	Bopomofo,
    { FT_MAKE_TAG( 'b', 'o', 'p', 'o' ), 0 },
// 	Yi,
    { FT_MAKE_TAG( 'y', 'i', ' ', ' ' ), 0 },

// 	// Additional Scripts
// 	Ethiopic,
    { FT_MAKE_TAG( 'e', 't', 'h', 'i' ), 0 },
// 	Cherokee,
    { FT_MAKE_TAG( 'c', 'h', 'e', 'r' ), 0 },
// 	CanadianAboriginal,
    { FT_MAKE_TAG( 'c', 'a', 'n', 's' ), 0 },
// 	Mongolian,
    { FT_MAKE_TAG( 'm', 'o', 'n', 'g' ), 0 },
// 	// Symbols
// 	CurrencySymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), 0 },
// 	LetterlikeSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), 0 },
// 	NumberForms,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), 0 },
// 	MathematicalOperators,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), 0 },
// 	TechnicalSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), 0 },
// 	GeometricSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), 0 },
// 	MiscellaneousSymbols,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), 0 },
// 	EnclosedAndSquare,
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), 0 },
// 	Braille,
    { FT_MAKE_TAG( 'b', 'r', 'a', 'i' ), 0 },
//                Unicode, should be used
    { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), 0 }
    // ### where are these?
// 	{ FT_MAKE_TAG( 'b', 'y', 'z', 'm' ), 0 },
//     { FT_MAKE_TAG( 'D', 'F', 'L', 'T' ), 0 },
    // ### Hangul Jamo
//     { FT_MAKE_TAG( 'j', 'a', 'm', 'o' ), 0 },
};

QOpenType::QOpenType(QFontEngineXft *fe)
    : fontEngine(fe), gdef(0), gsub(0), gpos(0), current_script(0)
{
    face = fe->face();
    otl_buffer_new(face->memory, &otl_buffer);
    tmpAttributes = 0;
    tmpLogClusters = 0;

    FT_Error error;
    if ((error = TT_Load_GDEF_Table(face, &gdef))) {
#ifdef OT_DEBUG
        qDebug("error loading gdef table: %d", error);
#endif
        gdef = 0;
    }

    if ((error = TT_Load_GSUB_Table(face, &gsub, gdef))) {
        gsub = 0;
#ifdef OT_DEBUG
        if (error != FT_Err_Table_Missing) {
            qDebug("error loading gsub table: %d", error);
        } else {
            qDebug("face doesn't have a gsub table");
        }
#endif
    }

    if ((error = TT_Load_GPOS_Table(face, &gpos, gdef))) {
        gpos = 0;
#ifdef OT_DEBUG
        qDebug("error loading gpos table: %d", error);
#endif
    }

    for (uint i = 0; i < QFont::NScripts; ++i)
        supported_scripts[i] = checkScript(i);
}

QOpenType::~QOpenType()
{
    if (gpos)
        TT_Done_GPOS_Table(gpos);
    if (gsub)
        TT_Done_GSUB_Table(gsub);
    if (gdef)
        TT_Done_GDEF_Table(gdef);
    if (otl_buffer)
        otl_buffer_free(otl_buffer);
    if (tmpAttributes)
        free(tmpAttributes);
    if (tmpLogClusters)
        free(tmpLogClusters);
}

bool QOpenType::checkScript(unsigned int script)
{
    assert(script < QFont::NScripts);

    uint tag = ot_scripts[script].tag;
    int requirements = ot_scripts[script].flags;

    if (requirements & RequiresGsub) {
        if (!gsub)
            return FALSE;

        FT_UShort script_index;
        FT_Error error = TT_GSUB_Select_Script(gsub, tag, &script_index);
        if (error) {
#ifdef OT_DEBUG
            qDebug("could not select script %d in GSub table: %d", (int)script, error);
#endif
            return FALSE;
        }
    }

    if (requirements & RequiresGpos) {
        if (!gpos)
            return FALSE;

        FT_UShort script_index;
        FT_Error error = TT_GPOS_Select_Script(gpos, script, &script_index);
        if (error) {
#ifdef OT_DEBUG
            qDebug("could not select script in gpos table: %d", error);
#endif
            return FALSE;
        }

    }
    return TRUE;
}


void QOpenType::selectScript(unsigned int script, const Features *features)
{
    if (current_script == script)
        return;

    assert(script < QFont::NScripts);
    // find script in our list of supported scripts.
    uint tag = ot_scripts[script].tag;

    if (gsub && features) {
#ifdef OT_DEBUG
        {
            TTO_FeatureList featurelist = gsub->FeatureList;
            int numfeatures = featurelist.FeatureCount;
            qDebug("gsub table has %d features", numfeatures);
            for(int i = 0; i < numfeatures; i++) {
                TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
                qDebug("   feature '%s'", tag_to_string(r->FeatureTag));
            }
        }
#endif
        TT_GSUB_Clear_Features(gsub);
        FT_UShort script_index;
        FT_Error error = TT_GSUB_Select_Script(gsub, tag, &script_index);
        if (!error) {
#ifdef OT_DEBUG
            qDebug("script %s has script index %d", tag_to_string(script), script_index);
#endif
            while (features->tag) {
                FT_UShort feature_index;
                error = TT_GSUB_Select_Feature(gsub, features->tag, script_index, 0xffff, &feature_index);
                if (!error) {
#ifdef OT_DEBUG
                    qDebug("  adding feature %s", tag_to_string(features->tag));
#endif
                    TT_GSUB_Add_Feature(gsub, feature_index, features->property);
                }
                ++features;
            }
        }
    }

    if (gpos) {
        TT_GPOS_Clear_Features(gpos);
        FT_UShort script_index;
        FT_Error error = TT_GPOS_Select_Script(gpos, tag, &script_index);
        if (!error) {
#ifdef OT_DEBUG
            {
                TTO_FeatureList featurelist = gpos->FeatureList;
                int numfeatures = featurelist.FeatureCount;
                qDebug("gpos table has %d features", numfeatures);
                for(int i = 0; i < numfeatures; i++) {
                    TTO_FeatureRecord *r = featurelist.FeatureRecord + i;
                    FT_UShort feature_index;
                    TT_GPOS_Select_Feature(gpos, r->FeatureTag, script_index, 0xffff, &feature_index);
                    qDebug("   feature '%s'", tag_to_string(r->FeatureTag));
                }
            }
#endif
            FT_ULong *feature_tag_list;
            error = TT_GPOS_Query_Features(gpos, script_index, 0xffff, &feature_tag_list);
            if (!error) {
                while (*feature_tag_list) {
                    FT_UShort feature_index;
                    error = TT_GPOS_Select_Feature(gpos, *feature_tag_list, script_index, 0xffff, &feature_index);
                    if (!error)
                        TT_GPOS_Add_Feature(gpos, feature_index, PositioningProperties);
                    ++feature_tag_list;
                }
            }
        }
    }

    current_script = script;
}

#ifdef OT_DEBUG
static void dump_string(OTL_Buffer buffer)
{
    for (uint i = 0; i < buffer->in_length; ++i) {
        qDebug("    %x: cluster=%d", buffer->in_string[i].gindex, buffer->in_string[i].cluster);
    }
}
#endif

extern void qt_heuristicPosition(QShaperItem *item);

bool QOpenType::shape(QShaperItem *item, const unsigned int *properties)
{
    length = item->num_glyphs;

    otl_buffer_clear(otl_buffer);

    tmpAttributes = (GlyphAttributes *) realloc(tmpAttributes, length*sizeof(GlyphAttributes));
    tmpLogClusters = (unsigned int *) realloc(tmpLogClusters, length*sizeof(unsigned int));
    for (int i = 0; i < length; ++i) {
        otl_buffer_add_glyph(otl_buffer, item->glyphs[i], properties ? properties[i] : 0, i);
        tmpAttributes[i] = item->attributes[i];
        tmpLogClusters[i] = item->log_clusters[i];
    }

#ifdef OT_DEBUG
    qDebug("-----------------------------------------");
//     qDebug("log clusters before shaping:");
//     for (int j = 0; j < length; j++)
//         qDebug("    log[%d] = %d", j, item->log_clusters[j]);
    qDebug("original glyphs: %p", item->glyphs);
    for (int i = 0; i < length; ++i)
        qDebug("   glyph=%4x", otl_buffer->in_string[i].gindex);
//     dump_string(otl_buffer);
#endif

    loadFlags = FT_LOAD_DEFAULT;

    if (gsub) {
        uint error = TT_GSUB_Apply_String(gsub, otl_buffer);
        if (error && error != TTO_Err_Not_Covered)
            return false;
    }

#ifdef OT_DEBUG
//     qDebug("log clusters before shaping:");
//     for (int j = 0; j < length; j++)
//         qDebug("    log[%d] = %d", j, item->log_clusters[j]);
    qDebug("shaped glyphs:");
    for (int i = 0; i < length; ++i)
        qDebug("   glyph=%4x", otl_buffer->in_string[i].gindex);
    qDebug("-----------------------------------------");
//     dump_string(otl_buffer);
#endif

    return true;
}

bool QOpenType::positionAndAdd(QShaperItem *item, bool doLogClusters)
{
    if (gpos) {
#ifdef Q_WS_X11
        Q_ASSERT(fontEngine->type() == QFontEngine::Xft);
        face = lockFTFace(static_cast<QFontEngineXft *>(fontEngine)->font());
#endif
        memset(otl_buffer->positions, 0, otl_buffer->in_length*sizeof(OTL_PositionRec));
        // #### check that passing "FALSE,FALSE" is correct
        TT_GPOS_Apply_String(face, gpos, loadFlags, otl_buffer, FALSE, FALSE);
#ifdef Q_WS_X11
        unlockFTFace(static_cast<QFontEngineXft *>(fontEngine)->font());
#endif
    }

    // make sure we have enough space to write everything back
    if (item->num_glyphs < (int)otl_buffer->in_length) {
        item->num_glyphs = otl_buffer->in_length;
        return FALSE;
    }

    for (unsigned int i = 0; i < otl_buffer->in_length; ++i) {
        item->glyphs[i] = otl_buffer->in_string[i].gindex;
        item->attributes[i] = tmpAttributes[otl_buffer->in_string[i].cluster];
        if (i && otl_buffer->in_string[i].cluster == otl_buffer->in_string[i-1].cluster)
            item->attributes[i].clusterStart = FALSE;
    }
    item->num_glyphs = otl_buffer->in_length;

    if (doLogClusters) {
        // we can't do this for indic, as we pass the stuf in syllables and it's easier to do it in the shaper.
        unsigned short *logClusters = item->log_clusters;
        int clusterStart = 0;
        int oldCi = 0;
        for (unsigned int i = 0; i < otl_buffer->in_length; ++i) {
            int ci = otl_buffer->in_string[i].cluster;
            //         qDebug("   ci[%d] = %d mark=%d, cmb=%d, cs=%d",
            //                i, ci, glyphAttributes[i].mark, glyphAttributes[i].combiningClass, glyphAttributes[i].clusterStart);
            if (!item->attributes[i].mark && item->attributes[i].clusterStart && ci != oldCi) {
                for (int j = oldCi; j < ci; j++)
                    logClusters[j] = clusterStart;
                clusterStart = i;
                oldCi = ci;
            }
        }
        for (int j = oldCi; j < length; j++)
            logClusters[j] = clusterStart;
    }

    // calulate the advances for the shaped glyphs
//     qDebug("unpositioned: ");
    static_cast<QFontEngineXft *>(item->font)->recalcAdvances(item->num_glyphs, item->glyphs, item->advances);

    // positioning code:
    if (gpos) {
	float scale = item->font->scale();
        OTL_Position positions = otl_buffer->positions;

//         qDebug("positioned glyphs:");
        for (unsigned int i = 0; i < otl_buffer->in_length; i++) {
//             qDebug("    %d:\t orig advance: (%d/%d)\tadv=(%d/%d)\tpos=(%d/%d)\tback=%d\tnew_advance=%d", i,
//                    glyphs[i].advance.x.toInt(), glyphs[i].advance.y.toInt(),
//                    (int)(positions[i].x_advance >> 6), (int)(positions[i].y_advance >> 6),
//                    (int)(positions[i].x_pos >> 6), (int)(positions[i].y_pos >> 6),
//                    positions[i].back, positions[i].new_advance);
            // ###### fix the case where we have y advances. How do we handle this in Uniscribe?????
            if (positions[i].new_advance) {
                item->advances[i] = item->flags & QTextEngine::RightToLeft
                                    ? -qRound((positions[i].x_advance >> 6)*scale)
                                    :  qRound((positions[i].x_advance >> 6)*scale);
            } else {
                item->advances[i] += item->flags & QTextEngine::RightToLeft
                                     ? -qRound((positions[i].x_advance >> 6)*scale)
                                     :  qRound((positions[i].x_advance >> 6)*scale);
            }
            int back = 0;
	  item->offsets[i].x = qRound((positions[i].x_pos >> 6)*scale);
	  item->offsets[i].y = qRound((positions[i].y_pos >> 6)*scale);
            while (positions[i-back].back) {
                back += positions[i - back].back;
                item->offsets[i].x += qRound((positions[i - back].x_pos >> 6)*scale);
	      item->offsets[i].y += qRound((positions[i - back].y_pos >> 6)*scale);
            }
	  item->offsets[i].y = -item->offsets[i].y;
            back = positions[i].back;
            if (item->flags & QTextEngine::RightToLeft) {
                while (back--) {
                    item->offsets[i].x -= item->advances[i-back];
                }
            } else {
                while (back) {
                    item->offsets[i].x -= item->advances[i-back];
                    --back;
                }
            }
//             qDebug("   ->\tadv=%d\tpos=(%d/%d)",
//                    glyphs[i].advance.x.toInt(), glyphs[i].offset.x.toInt(), glyphs[i].offset.y.toInt());
        }
        item->has_positioning = TRUE;
    } else {
        qt_heuristicPosition(item);
    }

#ifdef OT_DEBUG
//     if (doLogClusters) {
//         qDebug("log clusters after shaping:");
//         for (int j = 0; j < length; j++)
//             qDebug("    log[%d] = %d", j, item->log_clusters[j]);
//     }
    qDebug("final glyphs:");
    for (int i = 0; i < (int)otl_buffer->in_length; ++i)
        qDebug("   glyph=%4x char_index=%d mark: %d cmp: %d, clusterStart: %d advance=%d offset=%d/%d",
               item->glyphs[i], otl_buffer->in_string[i].cluster, item->attributes[i].mark,
               item->attributes[i].combiningClass, item->attributes[i].clusterStart,
               item->advances[i],
               item->offsets[i].x, item->offsets[i].y);
    qDebug("-----------------------------------------");
#endif
    return TRUE;
}

#endif
