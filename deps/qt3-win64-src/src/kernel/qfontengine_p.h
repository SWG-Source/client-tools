/****************************************************************************
** $Id: qfontengine_p.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** ???
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QFONTENGINE_P_H
#define QFONTENGINE_P_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H

#ifdef Q_WS_WIN
#include "qt_windows.h"
#include "qptrdict.h"
#endif

#include "qtextengine_p.h"

class QPaintDevice;

struct glyph_metrics_t;
class QChar;
typedef unsigned short glyph_t;
struct qoffset_t;
typedef int advance_t;
class QOpenType;
struct TransformedFont;

#if defined( Q_WS_X11 ) || defined( Q_WS_WIN) || defined( Q_WS_MAC )
class QFontEngine : public QShared
{
public:
    enum Error {
	NoError,
	OutOfMemory
    };

    enum Type {
	// X11 types
	Box,
	XLFD,
	LatinXLFD,
	Xft,

	// MS Windows types
	Win,
	Uniscribe,

	// Apple MacOS types
	Mac,

	// Trolltech QWS types
	QWS
    };

    QFontEngine() {
	count = 0; cache_count = 0;
#ifdef Q_WS_X11
	transformed_fonts = 0;
#endif
    }
    virtual ~QFontEngine();

    /* returns 0 as glyph index for non existant glyphs */
    virtual Error stringToCMap( const QChar *str, int len, glyph_t *glyphs,
				advance_t *advances, int *nglyphs, bool mirrored ) const = 0;

#ifdef Q_WS_X11
    virtual int cmap() const { return -1; }
    virtual QOpenType *openType() const { return 0; }
#endif

    virtual void draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags ) = 0;

    virtual glyph_metrics_t boundingBox( const glyph_t *glyphs,
					 const advance_t *advances,
					 const qoffset_t *offsets, int numGlyphs ) = 0;
    virtual glyph_metrics_t boundingBox( glyph_t glyph ) = 0;

    virtual int ascent() const = 0;
    virtual int descent() const = 0;
    virtual int leading() const = 0;

    virtual int lineThickness() const;
    virtual int underlinePosition() const;

    virtual int maxCharWidth() const = 0;
    virtual int minLeftBearing() const { return 0; }
    virtual int minRightBearing() const { return 0; }

    virtual const char *name() const = 0;

    virtual bool canRender( const QChar *string,  int len ) = 0;

    virtual void setScale( double ) {}
    virtual double scale() const { return 1.; }

    virtual Type type() const = 0;

    QFontDef fontDef;
    uint cache_cost; // amount of mem used in kb by the font
    int cache_count;

#ifdef Q_WS_WIN
    HDC dc() const;
    void getGlyphIndexes( const QChar *ch, int numChars, glyph_t *glyphs, bool mirrored ) const;
    void getCMap();

    QCString _name;
    HDC		hdc;
    HFONT	hfont;
    LOGFONT     logfont;
    uint	stockFont   : 1;
    uint	paintDevice : 1;
    uint        useTextOutA : 1;
    uint        ttf         : 1;
    uint        symbol      : 1;
    union {
	TEXTMETRICW	w;
	TEXTMETRICA	a;
    } tm;
    int		lw;
    unsigned char *cmap;
    void *script_cache;
    static QPtrDict<QFontEngine> cacheDict;
    short lbearing;
    short rbearing;
#endif // Q_WS_WIN
#ifdef Q_WS_X11
    TransformedFont *transformed_fonts;
#endif
};
#elif defined( Q_WS_QWS )
class QGfx;

class QFontEngine : public QShared
{
public:
    QFontEngine( const QFontDef&, const QPaintDevice * = 0 );
   ~QFontEngine();
    /*QMemoryManager::FontID*/ void *handle() const;

    enum Type {
	// X11 types
	Box,
	XLFD,
	Xft,

	// MS Windows types
	Win,
	Uniscribe,

	// Apple MacOS types
	Mac,

	// Trolltech QWS types
	Qws
    };

    enum TextFlags {
	Underline = 0x01,
	Overline  = 0x02,
	StrikeOut = 0x04
    };

    enum Error {
	NoError,
	OutOfMemory
    };
    /* returns 0 as glyph index for non existant glyphs */
    Error stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool mirrored ) const;

    void draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags );

    glyph_metrics_t boundingBox( const glyph_t *glyphs,
			       const advance_t *advances, const qoffset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;
    int minLeftBearing() const;
    int minRightBearing() const;
    int underlinePosition() const;
    int lineThickness() const;

    Type type() { return Qws; }

    bool canRender( const QChar *string,  int len );
    inline const char *name() const { return 0; }
    QFontDef fontDef;
    /*QMemoryManager::FontID*/ void *id;
    int cache_cost;
    int cache_count;
    int scale;
};
#endif // WIN || X11 || MAC



enum IndicFeatures {
    CcmpFeature,
    InitFeature,
    NuktaFeature,
    AkhantFeature,
    RephFeature,
    BelowFormFeature,
    HalfFormFeature,
    PostFormFeature,
    VattuFeature,
    PreSubstFeature,
    AboveSubstFeature,
    BelowSubstFeature,
    PostSubstFeature,
    HalantFeature
};

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
class QFontEngineBox : public QFontEngine
{
public:
    QFontEngineBox( int size );
    ~QFontEngineBox();

    Error stringToCMap( const QChar *str,  int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool mirrored ) const;

    void draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags );

    virtual glyph_metrics_t boundingBox( const glyph_t *glyphs,
				    const advance_t *advances, const qoffset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;
    int minLeftBearing() const { return 0; }
    int minRightBearing() const { return 0; }

#ifdef Q_WS_X11
    int cmap() const;
#endif
    const char *name() const;

    bool canRender( const QChar *string,  int len );

    Type type() const;
    inline int size() const { return _size; }

private:
    friend class QFontPrivate;
    int _size;
};
#endif

#ifdef Q_WS_X11
#include "qt_x11_p.h"


struct TransformedFont
{
    float xx;
    float xy;
    float yx;
    float yy;
    union {
	Font xlfd_font;
#ifndef QT_NO_XFTFREETYPE
	XftFont *xft_font;
#endif
    };
    TransformedFont *next;
};

#ifndef QT_NO_XFTFREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#include "ftxopen.h"

class QTextCodec;

class QFontEngineXft : public QFontEngine
{
public:
    QFontEngineXft( XftFont *font, XftPattern *pattern, int cmap );
    ~QFontEngineXft();

    QOpenType *openType() const;

    Error stringToCMap( const QChar *str,  int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool mirrored ) const;

    void draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags );

    virtual glyph_metrics_t boundingBox( const glyph_t *glyphs,
				    const advance_t *advances, const qoffset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int lineThickness() const;
    int underlinePosition() const;
    int maxCharWidth() const;
    int minLeftBearing() const;
    int minRightBearing() const;

    int cmap() const;
    const char *name() const;

    void setScale( double scale );
    double scale() const { return _scale; }

    bool canRender( const QChar *string,  int len );

    Type type() const;
    XftPattern *pattern() const { return _pattern; }
    FT_Face face() const { return _face; }
    XftFont *font() const { return _font; }

    void recalcAdvances( int len, glyph_t *glyphs, advance_t *advances );

private:
    friend class QFontPrivate;
    friend class QOpenType;
    XftFont *_font;
    XftPattern *_pattern;
    FT_Face _face;
    QOpenType *_openType;
    int _cmap;
    short lbearing;
    short rbearing;
    float _scale;
    enum { widthCacheSize = 0x800, cmapCacheSize = 0x500 };
    unsigned char widthCache[widthCacheSize];
    glyph_t cmapCache[cmapCacheSize];
};
#endif

class QFontEngineLatinXLFD;

class QFontEngineXLFD : public QFontEngine
{
public:
    QFontEngineXLFD( XFontStruct *fs, const char *name, int cmap );
    ~QFontEngineXLFD();

    Error stringToCMap( const QChar *str,  int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool mirrored ) const;

    void draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags );

    virtual glyph_metrics_t boundingBox( const glyph_t *glyphs,
				    const advance_t *advances, const qoffset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;
    int minLeftBearing() const;
    int minRightBearing() const;

    int cmap() const;
    const char *name() const;

    bool canRender( const QChar *string,  int len );

    void setScale( double scale );
    double scale() const { return _scale; }
    Type type() const;

    Qt::HANDLE handle() const { return (Qt::HANDLE) _fs->fid; }

private:
    friend class QFontPrivate;
    XFontStruct *_fs;
    QCString _name;
    QTextCodec *_codec;
    float _scale; // needed for printing, to correctly scale font metrics for bitmap fonts
    int _cmap;
    short lbearing;
    short rbearing;
    enum XlfdTransformations {
	XlfdTrUnknown,
	XlfdTrSupported,
	XlfdTrUnsupported
    };
    XlfdTransformations xlfd_transformations;

    friend class QFontEngineLatinXLFD;
};

class QFontEngineLatinXLFD : public QFontEngine
{
public:
    QFontEngineLatinXLFD( XFontStruct *xfs, const char *name, int cmap );
    ~QFontEngineLatinXLFD();

    Error stringToCMap( const QChar *str,  int len, glyph_t *glyphs,
			advance_t *advances, int *nglyphs, bool mirrored ) const;

    void draw( QPainter *p, int x, int y, const QTextEngine *engine,
	       const QScriptItem *si, int textFlags );

    virtual glyph_metrics_t boundingBox( const glyph_t *glyphs,
					 const advance_t *advances,
					 const qoffset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;
    int minLeftBearing() const;
    int minRightBearing() const;

    int cmap() const { return -1; } // ###
    const char *name() const;

    bool canRender( const QChar *string,  int len );

    void setScale( double scale );
    double scale() const { return _engines[0]->scale(); }
    Type type() const { return LatinXLFD; }

    Qt::HANDLE handle() const { return ((QFontEngineXLFD *) _engines[0])->handle(); }

private:
    void findEngine( const QChar &ch );

    QFontEngine **_engines;
    int _count;

    glyph_t   glyphIndices [0x200];
    advance_t glyphAdvances[0x200];
    glyph_t euroIndex;
    advance_t euroAdvance;
};

class QScriptItem;
class QTextEngine;

#ifndef QT_NO_XFTFREETYPE

#include "qscriptengine_p.h"
#include "qtextengine_p.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "ftxopen.h"

enum { PositioningProperties = 0x80000000 };

class QOpenType
{
public:
    QOpenType(QFontEngineXft *fe);
    ~QOpenType();

    struct Features {
        uint tag;
        uint property;
    };

    bool supportsScript(unsigned int script) {
        Q_ASSERT(script < QFont::NScripts);
        return supported_scripts[script];
    }
    void selectScript(unsigned int script, const Features *features = 0);

    bool shape(QShaperItem *item, const unsigned int *properties = 0);
    bool positionAndAdd(QShaperItem *item, bool doLogClusters = TRUE);

    OTL_GlyphItem glyphs() const { return otl_buffer->in_string; }
    int len() const { return otl_buffer->in_length; }
    void setProperty(int index, uint property) { otl_buffer->in_string[index].properties = property; }


private:
    bool checkScript(unsigned int script);
    QFontEngine *fontEngine;
    FT_Face face;
    TTO_GDEF gdef;
    TTO_GSUB gsub;
    TTO_GPOS gpos;
    bool supported_scripts[QFont::NScripts];
    FT_ULong current_script;
    bool positioned : 1;
    OTL_Buffer otl_buffer;
    GlyphAttributes *tmpAttributes;
    unsigned int *tmpLogClusters;
    int length;
    int orig_nglyphs;
    int loadFlags;
};

#endif // QT_NO_XFTFREETYPE

#elif defined( Q_WS_MAC )
#include "qt_mac.h"
#include <qmap.h>
#include <qcache.h>

class QFontEngineMac : public QFontEngine
{
#if 0
    ATSFontMetrics *info;
#else
    FontInfo *info;
#endif
    int psize;
    FMFontFamily fmfam;
    QMacFontInfo *internal_fi;
    mutable ATSUTextLayout mTextLayout;
    enum { widthCacheSize = 0x500 };
    mutable unsigned char widthCache[widthCacheSize];
    friend class QFont;
    friend class QGLContext;
    friend class QFontPrivate;
    friend class QMacSetFontInfo;

public:
    QFontEngineMac();
    ~QFontEngineMac();

    Error stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool mirrored ) const;

    void draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags );

    glyph_metrics_t boundingBox( const glyph_t *glyphs,
			       const advance_t *advances, const qoffset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    int ascent() const { return (int)info->ascent; }
    int descent() const { return (int)info->descent; }
    int leading() const { return (int)info->leading; }
#if 0
    int maxCharWidth() const { return (int)info->maxAdvanceWidth; }
#else
    int maxCharWidth() const { return info->widMax; }
#endif

    const char *name() const { return "ATSUI"; }

    bool canRender( const QChar *string,  int len );

    Type type() const { return QFontEngine::Mac; }

    void calculateCost();

    enum { WIDTH=0x01, DRAW=0x02, EXISTS=0x04 };
    int doTextTask(const QChar *s, int pos, int use_len, int len, uchar task, int =-1, int y=-1,
		   QPaintDevice *dev=NULL, const QRegion *rgn=NULL) const;
};

#elif defined( Q_WS_WIN )

class QFontEngineWin : public QFontEngine
{
public:
    QFontEngineWin( const char *name, HDC, HFONT, bool, LOGFONT );

    Error stringToCMap( const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs, bool mirrored ) const;

    void draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags );

    glyph_metrics_t boundingBox( const glyph_t *glyphs,
			       const advance_t *advances, const qoffset_t *offsets, int numGlyphs );
    glyph_metrics_t boundingBox( glyph_t glyph );

    int ascent() const;
    int descent() const;
    int leading() const;
    int maxCharWidth() const;
    int minLeftBearing() const;
    int minRightBearing() const;

    const char *name() const;

    bool canRender( const QChar *string,  int len );

    Type type() const;

    enum { widthCacheSize = 0x800, cmapCacheSize = 0x500 };
    unsigned char widthCache[widthCacheSize];
};

#if 0
class QFontEngineUniscribe : public QFontEngineWin
{
public:
    void draw( QPainter *p, int x, int y, const QTextEngine *engine, const QScriptItem *si, int textFlags );
    bool canRender( const QChar *string,  int len );

    Type type() const;
};
#endif

#endif // Q_WS_WIN

#endif
