/****************************************************************************
**
** Created : 20040828
**
** Copyright (C) 2004 Peter Kuemmel
** Copyright (C) 2004 Chris January
** Copyright (C) 2004-2006 Christian Ehrlicher
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

#include <limits.h>
#include <math.h>

#include "qbitmap.h"
#include "qcstring.h"
#include "qfont.h"
#include "qfontdatabase.h"
#include "qfontengine_p.h"
#include "qlibrary.h"           /* QLibrary::resolve() */
#include "qpainter.h"
#include "qpaintdevice.h"
#include "qstring.h"
#include "qtextcodec.h"
#include "qtextengine_p.h"

#include <private/qunicodetables_p.h>

const double Q_PI = 3.14159265358979323846; // pi

//#define QFONTENGINE_WIN_DEBUG

typedef signed short    qint16;
typedef unsigned short  quint16;
typedef unsigned int    quint32;

/* needed for dynamic loading ... */
static BOOL ( WINAPI * qtGetWorldTransform ) ( HDC hdc, LPXFORM lpXform  ) = NULL;
static BOOL ( WINAPI * qtSetWorldTransform ) ( HDC hdc, CONST XFORM *lpXform ) = NULL;
static bool useWorldTransform = false;

static unsigned char *getCMap(HDC hdc, bool &);
static quint32 getGlyphIndex(unsigned char *table, unsigned int unicode);

HDC    shared_dc            = 0;                // common dc for all fonts
static HFONT stock_sysfont  = 0;
extern Qt::WindowsVersion qt_winver;

extern void qt_draw_transformed_rect( QPainter *p, int x, int y, int w, int h, bool fill );

static inline void drawLines( QPainter *p, QFontEngine *fe, int baseline, int x1, int w, int textFlags )
{
#ifdef QFONTENGINE_WIN_DEBUG
    qDebug ( "qfontengine_win.cpp::drawLines: baseline=%d x1=%d w=%d", baseline, x1, w );
#endif

    int lw = fe->lineThickness();
    if ( textFlags & Qt::Underline ) {
        int pos = fe->underlinePosition();
        qt_draw_transformed_rect( p, x1, baseline + pos, w, lw, TRUE );
    }
    if ( textFlags & Qt::Overline ) {
        int pos = fe->ascent() + 1;
        if ( !pos )
            pos = 1;
        qt_draw_transformed_rect( p, x1, baseline - pos, w, lw, TRUE );
    }
    if ( textFlags & Qt::StrikeOut ) {
        int pos = fe->ascent() / 3;
        if ( !pos )
            pos = 1;
        qt_draw_transformed_rect( p, x1, baseline - pos, w, lw, TRUE );
    }
}

static inline HFONT systemFont()
{
    if (stock_sysfont == 0)
        stock_sysfont = (HFONT)GetStockObject(SYSTEM_FONT);
    return stock_sysfont;
}

QFontEngine::~QFontEngine()
{
    // make sure we aren't by accident still selected
    SelectObject( hdc, systemFont() );
    DeleteDC( hdc );
    if (cmap)
        delete [] cmap;
}

int QFontEngine::lineThickness() const
{
    // ad hoc algorithm
    int score = fontDef.weight * fontDef.pixelSize;
    int lw = score / 700;

    // looks better with thicker line for small pointsizes
    if ( lw < 2 && score >= 1050 )
        lw = 2;
    if ( lw == 0 )
        lw = 1;

    return lw;
}

int QFontEngine::underlinePosition() const
{
    int pos = ( ( lineThickness() * 2 ) + 3 ) / 6;
    return pos ? pos : 1;
}

void QFontEngine::getCMap()
{
    QT_WA({
        ttf = (bool)(tm.w.tmPitchAndFamily & TMPF_TRUETYPE);
    } , {
        ttf = (bool)(tm.a.tmPitchAndFamily & TMPF_TRUETYPE);
    });
//    HDC hdc = shared_dc;
//    SelectObject(hdc, hfont);
    bool symb = false;
    cmap = ttf ? ::getCMap(hdc, symb) : 0;
    if (!cmap) {
        ttf = false;
        symb = false;
    }
    symbol = symb;
    script_cache = 0;
    // !Qt4 designToDevice = 1.;
    // !Qt4 unitsPerEm = tm.w.tmHeight;
    if(cmap) {
        QT_WA( {
            OUTLINETEXTMETRICW metric;
            GetOutlineTextMetricsW(hdc, sizeof(OUTLINETEXTMETRICW), &metric);
            // !Qt4 designToDevice = (float)metric.otmTextMetrics.tmHeight/(float)metric.otmEMSquare;
            // !Qt4 unitsPerEm = metric.otmEMSquare;
        }, {
            OUTLINETEXTMETRICA metric;
            GetOutlineTextMetricsA(hdc, sizeof(OUTLINETEXTMETRICA), &metric);
            // !Qt4 designToDevice = (float)metric.otmTextMetrics.tmHeight/(float)metric.otmEMSquare;
            // !Qt4 unitsPerEm = metric.otmEMSquare;
        } )
    // !Qt4 kerning_pairs = getKerning(hdc, designToDevice);
    }
}

inline unsigned int getChar(const QChar *str, int &i, const int len)
{
    unsigned int uc = str[i].unicode();
    if (uc >= 0xd800 && uc < 0xdc00 && i < len-1) {
        uint low = str[i+1].unicode();
       if (low >= 0xdc00 && low < 0xe000) {
            uc = (uc - 0xd800)*0x400 + (low - 0xdc00) + 0x10000;
            ++i;
        }
    }
    return uc;
}

void QFontEngine::getGlyphIndexes( const QChar *str, int numChars, glyph_t *glyphs, bool mirrored ) const
{
    if (mirrored) {
        if (symbol) {
            for (int i = 0; i < numChars; ++i) {
                unsigned int uc = getChar(str, i, numChars);
                *glyphs = getGlyphIndex(cmap, uc);
                if(!*glyphs && uc < 0x100)
                    *glyphs = getGlyphIndex(cmap, uc + 0xf000);
                glyphs++;
            }
        } else if (ttf) {
            for (int i = 0; i < numChars; ++i) {
                unsigned int uc = getChar(str, i, numChars);
                *glyphs = getGlyphIndex(cmap, ::mirroredChar(uc).unicode());
                glyphs++;
            }
        } else {
            ushort first, last;
            QT_WA({
                first = tm.w.tmFirstChar;
                last = tm.w.tmLastChar;
            }, {
                first = tm.a.tmFirstChar;
                last = tm.a.tmLastChar;
            });
            for (int i = 0; i < numChars; ++i) {
                ushort ucs = ::mirroredChar(str->unicode()).unicode();
                if (ucs >= first && ucs <= last)
                    *glyphs = ucs;
                else
                    *glyphs = 0;
                glyphs++;
                str++;
            }
        }
    } else {
        if (symbol) {
            for (int i = 0; i < numChars; ++i) {
                unsigned int uc = getChar(str, i, numChars);
                *glyphs = getGlyphIndex(cmap, uc);
                if(!*glyphs && uc < 0x100)
                    *glyphs = getGlyphIndex(cmap, uc + 0xf000);
                glyphs++;
            }
        } else if (ttf) {
            for (int i = 0; i < numChars; ++i) {
                unsigned int uc = getChar(str, i, numChars);
                *glyphs = getGlyphIndex(cmap, uc);
                glyphs++;
            }
        } else {
            ushort first, last;
            QT_WA({
                first = tm.w.tmFirstChar;
                last = tm.w.tmLastChar;
            }, {
                first = tm.a.tmFirstChar;
                last = tm.a.tmLastChar;
            });
            for (int i = 0; i < numChars; ++i) {
                if (str->unicode() >= first && str->unicode() <= last)
                    *glyphs = str->unicode();
                else
                    *glyphs = 0;
                glyphs++;
                str++;
            }
        }
    }
    return;
}

// ------------------------------------------------------------------
// The box font engine
// ------------------------------------------------------------------
QFontEngineBox::QFontEngineBox( int size )
        : _size( size )
{
    cache_cost = sizeof( QFontEngineBox );
    cmap = 0;
}

QFontEngineBox::~QFontEngineBox()
{}

QFontEngine::Error QFontEngineBox::stringToCMap( const QChar *, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool ) const
{
    if ( *nglyphs < len ) {
        *nglyphs = len;
        return OutOfMemory;
    }

    for (int i = 0; i < len; i++) {
        glyphs[i] = 0;
        advances[i] = _size;
    }

    *nglyphs = len;
    return NoError;
}

void QFontEngineBox::draw( QPainter *p, int x, int y, const QTextEngine *, const QScriptItem *si, int textFlags )
{
}

glyph_metrics_t QFontEngineBox::boundingBox( const glyph_t *, const advance_t *, const qoffset_t *, int numGlyphs )
{
    glyph_metrics_t overall;
    overall.x = overall.y = 0;
    overall.width = _size * numGlyphs;
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
    return ( l > 0 ) ? l : 1;
}

int QFontEngineBox::maxCharWidth() const
{
    return _size;
}

const char *QFontEngineBox::name() const
{
    return "null";
}

bool QFontEngineBox::canRender( const QChar *, int )
{
    return true;
}

QFontEngine::Type QFontEngineBox::type() const
{
    return Box;
}

// ------------------------------------------------------------------
// Win font engine
// ------------------------------------------------------------------
QFontEngineWin::QFontEngineWin( const char *nameIn, HDC hdcIn, HFONT hfIn, bool bStockFont, LOGFONT lfIn )
{
    _name = nameIn;
    hfont = hfIn;
    hdc = CreateCompatibleDC( hdcIn ? hdcIn : shared_dc );
    logfont = lfIn;

    SelectObject( hdc, hfont );
    stockFont = bStockFont;

    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;

    BOOL res = QT_WA_INLINE( GetTextMetricsW( hdc, &tm.w ), GetTextMetricsA( hdc, &tm.a ) );
    if ( !res )
        qWarning( "QFontEngineWin: GetTextMetrics failed" );

    cache_cost = tm.w.tmHeight * tm.w.tmAveCharWidth * 2000;
    getCMap();

    useTextOutA = false;
#ifndef Q_OS_TEMP
    // TextOutW doesn't work for symbol fonts on Windows 95!
    // since we're using glyph indices we don't care for ttfs about this!
    if ( qt_winver == Qt::WV_95 && !ttf &&
            ( _name == QString( "Marlett" ).latin1() || _name == QString( "Symbol" ).latin1() ||
              _name == QString( "Webdings" ).latin1() || _name == QString( "Wingdings" ).latin1() ) )
        useTextOutA = true;
#endif

    memset( widthCache, 0, sizeof( widthCache ) );

    if ( qt_winver >= Qt::WV_NT && !useWorldTransform ) {
        qtGetWorldTransform = reinterpret_cast<decltype(qtGetWorldTransform)>(QLibrary::resolve( "gdi32.dll", "GetWorldTransform" ));
        qtSetWorldTransform = reinterpret_cast<decltype(qtSetWorldTransform)>(QLibrary::resolve( "gdi32.dll", "SetWorldTransform" ));
        useWorldTransform = ( qtGetWorldTransform && qtSetWorldTransform );
    }
}

QFontEngine::Error QFontEngineWin::stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool mirrored ) const
{
    QFontEngineWin *that = const_cast<QFontEngineWin*>(this);

    if ( *nglyphs < len ) {
        *nglyphs = len;
        return OutOfMemory;
    }

    getGlyphIndexes(str, len, glyphs, mirrored);

    HGDIOBJ oldFont = SelectObject(hdc, hfont);
    int overhang = (qt_winver & Qt::WV_DOS_based) ? tm.a.tmOverhang : 0;
    int glyph_pos = 0;
    for(int i = 0; i < len; i++) {
        bool surrogate = (str[i].unicode() >= 0xd800 && str[i].unicode() < 0xdc00 && i < len-1
                          && str[i+1].unicode() >= 0xdc00 && str[i+1].unicode() < 0xe000);
        unsigned int glyph = glyphs[i];
        if (advances) {
            advances[glyph_pos] = (glyph < widthCacheSize) ? widthCache[glyph] : 0;
            // font-width cache failed
            if (advances[glyph_pos] == 0) {
                SIZE size = {0, 0};
                GetTextExtentPoint32W(hdc, (wchar_t *)str + i, surrogate ? 2 : 1, &size);
                size.cx -= overhang;
                advances[glyph_pos] = size.cx;
                // if glyph's within cache range, store it for later
                if (glyph < widthCacheSize && size.cx > 0 && size.cx < 0x100)
                    that->widthCache[glyph] = size.cx;
            }
        }
        if (surrogate)
            ++i;
        ++glyph_pos;
    }
    SelectObject(hdc, oldFont);
    return NoError;
}

// CCJ: TODO: doesn't support scaling, etc.
void QFontEngineWin::draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags )
{
    if ( !si->num_glyphs )
        return;

    HDC hdc = p->handle();  //don't touch QFontEngineWin::hdc
    int oldBkMode = SetBkMode( hdc, TRANSPARENT );

    SetTextAlign ( hdc, GetTextAlign( hdc ) | TA_BASELINE | TA_LEFT );

    int xorig = x;
    int yorig = y;

    glyph_t *glyphs = engine->glyphs( si );
    advance_t *advances = engine->advances( si );
    qoffset_t *offsets = engine->offsets( si );

    int xpos = x;

    // CCJ - need to call SelectObject here otherwise
    // we might end up using the wrong font
    HFONT oldFont = ( HFONT ) SelectObject( hdc, hfont );
    unsigned int options = ttf ? ETO_GLYPH_INDEX : 0;

#ifdef QFONTENGINE_WIN_DEBUG
    qDebug ( "QFontEngineWin::draw str=%s x=%d y=%d", QString ( ( QChar * ) glyphs, si->num_glyphs ).latin1 (), x, y );
#endif

    if ( useTextOutA ) {
        for ( int i = 0; i < si->num_glyphs; i++ ) {
            QChar c( glyphs[ i ] );
            QString s( c );
            QCString cstr = s.local8Bit();
            TextOutA( hdc, qRound( x + offsets[ i ].x ), qRound( y + offsets[ i ].y ),
                      cstr.data(), cstr.length() );
            x += advances[ i ];
            glyphs++;
        }
    } else {
        HFONT hfnt, hfntPrev;
        LOGFONT logfont;
        XFORM prevXForm;
        int prevGMMode;
        XFORM xf;
        DWORD w = 0;
        bool fast = true;

        for ( int i = 0; i < si->num_glyphs; i++ ) {
            if ( offsets[ i ].x != 0 || offsets[ i ].y != 0 ) {
                fast = false;
                break;
            }
            w += advances[ i ];
        }
        if ( p->txop >= QPainter::TxRotShear ) {
            QWMatrix &m = p->xmat;
            if ( useWorldTransform ) {
                xf.eM11 = m.m11();
                xf.eM12 = m.m12();
                xf.eM21 = m.m21();
                xf.eM22 = m.m22();
                xf.eDx = m.dx();
                xf.eDy = m.dy();
                prevGMMode = SetGraphicsMode( hdc, GM_ADVANCED );
                qtGetWorldTransform( hdc, &prevXForm );
                qtSetWorldTransform( hdc, &xf );

                if ( fast ) {
                    ExtTextOutW( hdc, x, y, options, 0, ( LPCWSTR ) glyphs, si->num_glyphs, advances ) ;
                    x += w;
                } else {
                    for ( int i = 0; i < si->num_glyphs; i++ ) {
                        int xp = x + offsets[ i ].x;
                        int yp = y + offsets[ i ].y;
                        if ( xp < SHRT_MAX && xp > SHRT_MIN ) {
                            TextOutW( hdc, xp, yp, ( LPCWSTR ) glyphs[ i ], 1 );
                        }
                        advance_t adv = advances[ i ];
                        x += adv;
                    }
                }
                qtSetWorldTransform( hdc, &prevXForm );
                if ( prevGMMode != GM_ADVANCED )
                    SetGraphicsMode( hdc, prevGMMode );
            } else {
                GetObject( hdc, sizeof( LOGFONT ), &logfont );
                logfont.lfEscapement = qRound( -atan2( m.m12(), m.m11() ) * 1800.0 / Q_PI );
                logfont.lfOrientation = logfont.lfEscapement;
                hfnt = CreateFontIndirect( &logfont );
                hfntPrev = ( HFONT ) SelectObject( hdc, hfnt );

                if ( fast ) {
                    int xp, yp;
                    p->map ( x, y, &xp, &yp );

                    ExtTextOutW( hdc, xp, yp, options, 0, ( LPCWSTR ) glyphs, si->num_glyphs, advances ) ;
                    x += w;
                } else {
                    for ( int i = 0; i < si->num_glyphs; i++ ) {
                        int xp = x + offsets[ i ].x;
                        int yp = y + offsets[ i ].y;
                        if ( xp < SHRT_MAX && xp > SHRT_MIN ) {
                            p->map ( xp, yp, &xp, &yp );
                            TextOutW( hdc, xp, yp, ( LPCWSTR ) glyphs[ i ], 1 );
                        }
                        advance_t adv = advances[ i ];
                        x += adv;
                    }
                }
                SelectObject( hdc, hfntPrev );
                DeleteObject( hfnt );
            }
        } else {
            if ( fast ) {
                int xp, yp;
                p->map ( x, y, &xp, &yp );

                ExtTextOutW( hdc, xp, yp, options, 0,
                             ( LPCWSTR ) glyphs,
                             si->num_glyphs, advances ) ;
                x += w;
            } else {
                for ( int i = 0; i < si->num_glyphs; i++ ) {
                    int xp = x + offsets[ i ].x;
                    int yp = y + offsets[ i ].y;
                    if ( xp < SHRT_MAX && xp > SHRT_MIN ) {
                        p->map ( xp, yp, &xp, &yp );
                        ExtTextOutW( hdc, xp, yp, options, 0,
                                    ( LPCWSTR ) &glyphs[ i ], 1,
                                    ( INT * ) &advances[ i ] );
                    }
                    x += advances[ i ];
                }
            }
        }
    }
    if ( textFlags != 0 )
        drawLines( p, this, yorig, xorig, x - xpos, textFlags );

    SelectObject( hdc, oldFont );
    SetBkMode( hdc, oldBkMode );
}

glyph_metrics_t QFontEngineWin::boundingBox( const glyph_t *glyphs, const advance_t *advances, const qoffset_t *offsets, int numGlyphs )
{
    if (numGlyphs == 0)
        return glyph_metrics_t();

    int w = 0;
    const advance_t *end = advances + numGlyphs;
    while(end > advances) {
        --end;
        w += *end;
    }

    return glyph_metrics_t(0, -tm.w.tmAscent, w, tm.w.tmHeight, w, 0);
}

glyph_metrics_t QFontEngineWin::boundingBox( glyph_t glyph )
{
#ifndef Q_OS_TEMP
    GLYPHMETRICS gm;

//    HDC hdc = shared_dc;
//    SelectObject(hdc, hfont);
    if(!ttf) {
        SIZE s = {0, 0};
        WCHAR ch = glyph;
        BOOL res = GetTextExtentPoint32W(hdc, &ch, 1, &s);
        Q_UNUSED(res);
        int overhang = (qt_winver & Qt::WV_DOS_based) ? tm.a.tmOverhang : 0;
        return glyph_metrics_t(0, -tm.a.tmAscent, s.cx, tm.a.tmHeight, s.cx-overhang, 0);
    } else {
        DWORD res = 0;
        MAT2 mat;
        mat.eM11.value = mat.eM22.value = 1;
        mat.eM11.fract = mat.eM22.fract = 0;
        mat.eM21.value = mat.eM12.value = 0;
        mat.eM21.fract = mat.eM12.fract = 0;
        QT_WA({
            res = GetGlyphOutlineW(hdc, glyph, GGO_METRICS|GGO_GLYPH_INDEX, &gm, 0, 0, &mat);
        } , {
            res = GetGlyphOutlineA(hdc, glyph, GGO_METRICS|GGO_GLYPH_INDEX, &gm, 0, 0, &mat);
        });
        if (res != GDI_ERROR)
            return glyph_metrics_t(gm.gmptGlyphOrigin.x, -gm.gmptGlyphOrigin.y,
                                  gm.gmBlackBoxX, gm.gmBlackBoxY, gm.gmCellIncX, gm.gmCellIncY);
    }
#endif
    return glyph_metrics_t();
}


int QFontEngineWin::ascent() const
{
    return ( &tm.a ) ->tmAscent;
}

int QFontEngineWin::descent() const
{
    return ( &tm.a ) ->tmDescent;
}

int QFontEngineWin::leading() const
{
    return ( &tm.a ) ->tmInternalLeading + ( &tm.a ) ->tmExternalLeading;
}

int QFontEngineWin::maxCharWidth() const
{
    return ( &tm.a ) ->tmMaxCharWidth;
}

static const ushort char_table[] =
    {
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

static const int char_table_entries = sizeof( char_table ) / sizeof( ushort );

int QFontEngineWin::minLeftBearing() const
{
    if ( lbearing == SHRT_MIN )
        minRightBearing(); // calculates both

    return lbearing;
}

int QFontEngineWin::minRightBearing() const
{
    if ( rbearing == SHRT_MIN ) {
        QFontEngineWin * that = ( QFontEngineWin * ) this;
        that->lbearing = that->rbearing = 0;
        QChar *ch = ( QChar * ) char_table;
        glyph_t glyphs[ char_table_entries ];
        int ng = char_table_entries;
        stringToCMap( ch, char_table_entries, glyphs, 0, &ng, false );
        while ( --ng ) {
            if ( glyphs[ ng ] ) {
                glyph_metrics_t gi = that->boundingBox( glyphs[ ng ] );
                if ( gi.xoff ) {
#ifdef QFONTENGINE_WIN_DEBUG
                    qDebug ( "QFontEngineWin::minRightBearing() glyph: %d char: %c x = %d, xoff = %d", glyphs[ ng ], ch[ ng ].latin1 (), gi.x, gi.xoff );
#endif

                    that->lbearing = QMIN( lbearing, gi.x );
                    that->rbearing = QMIN( rbearing, gi.xoff - gi.x - gi.width );
                }
            }
        }
    }
    return rbearing;
}


const char *QFontEngineWin::name() const
{
    return _name;
}

bool QFontEngineWin::canRender( const QChar *string, int len )
{
    if (symbol) {
        for (int i = 0; i < len; ++i) {
            unsigned int uc = getChar(string, i, len);
            if (getGlyphIndex(cmap, uc) == 0) {
                if (uc < 0x100) {
                    if (getGlyphIndex(cmap, uc + 0xf000) == 0)
                        return false;
                } else {
                    return false;
                }
            }
        }
    } else if (ttf) {
        for (int i = 0; i < len; ++i) {
            unsigned int uc = getChar(string, i, len);
            if (getGlyphIndex(cmap, uc) == 0)
                return false;
        }
    } else {
        QT_WA({
            while(len--) {
                if (tm.w.tmFirstChar > string->unicode() || tm.w.tmLastChar < string->unicode())
                    return false;
            }
        }, {
            while(len--) {
                if (tm.a.tmFirstChar > string->unicode() || tm.a.tmLastChar < string->unicode())
                    return false;
            }
        });
    }
    return true;
}

QFontEngine::Type QFontEngineWin::type() const
{
    return Win;
}

// ----------------------------------------------------------------------------
// True type support methods
// ----------------------------------------------------------------------------

#define MAKE_TAG(ch1, ch2, ch3, ch4) (\
    (((DWORD)(ch4)) << 24) | \
    (((DWORD)(ch3)) << 16) | \
    (((DWORD)(ch2)) << 8) | \
    ((DWORD)(ch1)) \
   )

static inline quint32 getUInt(unsigned char *p)
{
    quint32 val;
    val = *p++ << 24;
    val |= *p++ << 16;
    val |= *p++ << 8;
    val |= *p;

    return val;
}

static inline quint16 getUShort(unsigned char *p)
{
    quint16 val;
    val = *p++ << 8;
    val |= *p;

    return val;
}

static inline void tag_to_string(char *string, quint32 tag)
{
    string[0] = (tag >> 24)&0xff;
    string[1] = (tag >> 16)&0xff;
    string[2] = (tag >> 8)&0xff;
    string[3] = tag&0xff;
    string[4] = 0;
}

static quint32 getGlyphIndex(unsigned char *table, unsigned int unicode)
{
    unsigned short format = getUShort(table);
    if (format == 0) {
        if (unicode < 256)
            return (int) *(table+6+unicode);
    } else if (format == 4) {
        if(unicode > 0xffff)
            return 0;
        quint16 segCountX2 = getUShort(table + 6);
        unsigned char *ends = table + 14;
        quint16 endIndex = 0;
        int i = 0;
        for (; i < segCountX2/2 && (endIndex = getUShort(ends + 2*i)) < unicode; i++);

        unsigned char *idx = ends + segCountX2 + 2 + 2*i;
        quint16 startIndex = getUShort(idx);

        if (startIndex > unicode)
            return 0;

        idx += segCountX2;
        qint16 idDelta = (qint16)getUShort(idx);
        idx += segCountX2;
        quint16 idRangeoffset_t = (quint16)getUShort(idx);

        quint16 glyphIndex;
        if (idRangeoffset_t) {
            quint16 id = getUShort(idRangeoffset_t + 2*(unicode - startIndex) + idx);
            if (id)
                glyphIndex = (idDelta + id) % 0x10000;
            else
                glyphIndex = 0;
        } else {
            glyphIndex = (idDelta + unicode) % 0x10000;
        }
        return glyphIndex;
    } else if (format == 12) {
        quint32 nGroups = getUInt(table + 12);

        table += 16; // move to start of groups

        int left = 0, right = nGroups - 1;
        while (left <= right) {
            int middle = left + ( ( right - left ) >> 1 );

            quint32 startCharCode = getUInt(table + 12*middle);
            if(unicode < startCharCode)
                right = middle - 1;
            else {
                quint32 endCharCode = getUInt(table + 12*middle + 4);
                if(unicode <= endCharCode)
                    return getUInt(table + 12*middle + 8) + unicode - startCharCode;
                left = middle + 1;
            }
        }
    } else {
        qDebug("QFontEngineWin::cmap table of format %d not implemented", format);
    }

    return 0;
}

static unsigned char *getCMap(HDC hdc, bool &symbol)
{
    const DWORD CMAP = MAKE_TAG('c', 'm', 'a', 'p');

    unsigned char header[8];

    // get the CMAP header and the number of encoding tables
    DWORD bytes =
#ifndef Q_OS_TEMP
        GetFontData(hdc, CMAP, 0, &header, 4);
#else
        0;
#endif
    if (bytes == GDI_ERROR)
        return 0;
    {
        unsigned short version = getUShort(header);
        if (version != 0)
            return 0;
    }

    unsigned short numTables = getUShort(header+2);
    unsigned char *maps = new unsigned char[8*numTables];

    // get the encoding table and look for Unicode
#ifndef Q_OS_TEMP
    bytes = GetFontData(hdc, CMAP, 4, maps, 8*numTables);
#endif
    if (bytes == GDI_ERROR)
        return 0;

    quint32 version = 0;
    unsigned int unicode_table = 0;
    for (int n = 0; n < numTables; n++) {
        quint32 v = getUInt(maps + 8*n);
        // accept both symbol and Unicode encodings. prefer unicode.
        if(v == 0x00030001 || v == 0x00030000 || v == 0x0003000a) {
            if (v > version) {
                version = v;
                unicode_table = getUInt(maps + 8*n + 4);
            }
        }
    }
    symbol = version == 0x00030000;

    if (!unicode_table) {
        // qDebug("no unicode table found");
        return 0;
    }

    delete [] maps;

    // get the header of the unicode table
#ifndef Q_OS_TEMP
    bytes = GetFontData(hdc, CMAP, unicode_table, &header, 8);
#endif
    if (bytes == GDI_ERROR)
        return 0;

    unsigned short format = getUShort(header);
    unsigned int length;
    if(format < 8)
        length = getUShort(header+2);
    else
        length = getUInt(header+4);
    unsigned char *unicode_data = new unsigned char[length];

    // get the cmap table itself
#ifndef Q_OS_TEMP
    bytes = GetFontData(hdc, CMAP, unicode_table, unicode_data, length);
#endif
    if (bytes == GDI_ERROR) {
        delete [] unicode_data;
        return 0;
    }
    return unicode_data;
}
