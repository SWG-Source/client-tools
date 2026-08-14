/****************************************************************************
** $Id: qtextengine_p.h 2051 2007-02-21 10:04:20Z chehrlic $
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

#ifndef QTEXTENGINE_P_H
#define QTEXTENGINE_P_H

#ifndef QT_H
#include "qglobal.h"
#include "qstring.h"
#include "qnamespace.h"
#include <private/qfontdata_p.h>
#endif // QT_H

#include <stdlib.h>
#ifndef Q_OS_TEMP
#include <assert.h>
#endif // Q_OS_TEMP

class QFontPrivate;
class QString;

class QOpenType;
class QPainter;

// this uses the same coordinate system as Qt, but a different one to freetype and Xft.
// * y is usually negative, and is equal to the ascent.
// * negative yoff means the following stuff is drawn higher up.
// the characters bounding rect is given by QRect( x,y,width,height), it's advance by
// xoo and yoff
struct glyph_metrics_t
{
    inline glyph_metrics_t() {
	x = 100000;
	y = 100000;
	width = 0;
	height = 0;
	xoff = 0;
	yoff = 0;
    }
    inline glyph_metrics_t( int _x, int _y, int _width, int _height, int _xoff, int _yoff ) {
	x = _x;
	y = _y;
	width = _width;
	height = _height;
	xoff = _xoff;
	yoff = _yoff;
    }
    int x;
    int y;
    int width;
    int height;
    int xoff;
    int yoff;
};

#if defined( Q_WS_X11 ) || defined ( Q_WS_QWS )
typedef unsigned short glyph_t;

struct qoffset_t {
    short x;
    short y;
};

typedef int advance_t;

struct QScriptAnalysis
{
    unsigned short script    : 7;
    unsigned short bidiLevel : 6;  // Unicode Bidi algorithm embedding level (0-61)
    unsigned short override  : 1;  // Set when in LRO/RLO embedding
    unsigned short reserved  : 2;
    bool operator == ( const QScriptAnalysis &other ) {
	return
	    script == other.script &&
	    bidiLevel == other.bidiLevel;
	// ###
// 	    && override == other.override;
    }

};

#elif defined( Q_WS_MAC )

typedef unsigned short glyph_t;

struct qoffset_t {
    short x;
    short y;
};

typedef int advance_t;

struct QScriptAnalysis
{
    unsigned short script    : 7;
    unsigned short bidiLevel : 6;  // Unicode Bidi algorithm embedding level (0-61)
    unsigned short override  : 1;  // Set when in LRO/RLO embedding
    unsigned short reserved  : 2;
    bool operator == ( const QScriptAnalysis &other ) {
	return
	    script == other.script &&
	    bidiLevel == other.bidiLevel;
	// ###
// 	    && override == other.override;
    }

};

#elif defined( Q_WS_WIN )

// do not change the definitions below unless you know what you are doing!
// it is designed to be compatible with the types found in uniscribe.

typedef unsigned short glyph_t;

struct qoffset_t {
    int x;
    int y;
};

typedef int advance_t;

struct QScriptAnalysis {
    unsigned short script         :10;
    unsigned short rtl            :1;
    unsigned short layoutRTL      :1;
    unsigned short linkBefore     :1;
    unsigned short linkAfter      :1;
    unsigned short logicalOrder   :1;
    unsigned short noGlyphIndex   :1;
    unsigned short bidiLevel         :5;
    unsigned short override          :1;
    unsigned short inhibitSymSwap    :1;
    unsigned short charShape         :1;
    unsigned short digitSubstitute   :1;
    unsigned short inhibitLigate     :1;
    unsigned short fDisplayZWG        :1;
    unsigned short arabicNumContext  :1;
    unsigned short gcpClusters       :1;
    unsigned short reserved          :1;
    unsigned short engineReserved    :2;
};

inline bool operator == ( const QScriptAnalysis &sa1, const QScriptAnalysis &sa2 )
{
    return
	sa1.script == sa2.script &&
	sa1.bidiLevel == sa2.bidiLevel;
	// ###
// 	    && override == other.override;
}

#endif

// enum and struct are  made to be compatible with Uniscribe, dont change unless you know what you're doing.
struct GlyphAttributes {
    // highest value means highest priority for justification. Justification is done by first inserting kashidas
    // starting with the highest priority positions, then stretching spaces, afterwards extending inter char
    // spacing, and last spacing between arabic words.
    // NoJustification is for example set for arabic where no Kashida can be inserted or for diacritics.
    enum Justification {
        NoJustification= 0,   // Justification can't be applied after this glyph
        Arabic_Space   = 1,   // This glyph represents a space inside arabic text
        Character      = 2,   // Inter-character justification point follows this glyph
        Space          = 4,   // This glyph represents a blank outside an Arabic run
        Arabic_Normal  = 7,   // Normal Middle-Of-Word glyph that connects to the right (begin)
        Arabic_Waw     = 8,    // Next character is final form of Waw/Ain/Qaf/Fa
        Arabic_BaRa    = 9,   // Next two chars are Ba + Ra/Ya/AlefMaksura
        Arabic_Alef    = 10,  // Next character is final form of Alef/Tah/Lam/Kaf/Gaf
        Arabic_HaaDal  = 11,  // Next character is final form of Haa/Dal/Taa Marbutah
        Arabic_Seen    = 12,  // Initial or Medial form Of Seen/Sad
        Arabic_Kashida = 13   // Kashida(U+640) in middle of word
    };
    unsigned short justification   :4;  // Justification class
    unsigned short clusterStart    :1;  // First glyph of representation of cluster
    unsigned short mark            :1;  // needs to be positioned around base char
    unsigned short zeroWidth       :1;  // ZWJ, ZWNJ etc, with no width, currently used as "Don't print" for ZWSP
    unsigned short reserved        :1;
    unsigned short combiningClass  :8;
};

// also this is compatible to uniscribe. Do not change.
struct QCharAttributes {
    uchar softBreak      :1;     // Potential linebreak point _before_ this character
    uchar whiteSpace     :1;     // A unicode whitespace character, except NBSP, ZWNBSP
    uchar charStop       :1;     // Valid cursor position (for left/right arrow)
    uchar wordStop       :1;     // Valid cursor position (for ctrl + left/right arrow)
    uchar invalid        :1;
    uchar reserved       :3;
};

inline bool qIsZeroWidthChar(ushort uc)
{
    return (uc >= 0x200b && uc <= 0x200f /* ZW Space, ZWNJ, ZWJ, LRM and RLM */)
            || (uc >= 0x2028 && uc <= 0x202f /* LS, PS, LRE, RLE, PDF, LRO, RLO, NNBSP */)
            || (uc >= 0x206a && uc <= 0x206f /* ISS, ASS, IAFS, AFS, NADS, NODS */);
}

class QFontEngine;

struct QScriptItem
{
    inline QScriptItem() : position( 0 ), isSpace( FALSE ), isTab( FALSE ),
			   isObject( FALSE ), hasPositioning( FALSE ),
			   descent( -1 ), ascent( -1 ), width( -1 ),
			   x( 0 ), y( 0 ), num_glyphs( 0 ), glyph_data_offset( 0 ),
			   fontEngine( 0 ) { }
    int position;
    QScriptAnalysis analysis;
    unsigned short isSpace  : 1;
    unsigned short isTab    : 1;
    unsigned short isObject : 1;
    unsigned short hasPositioning : 1;
    unsigned short complex : 1; // Windows only
    unsigned short private_use : 1; // Windows only
    unsigned short reserved : 10;
    short descent;
    int ascent;
    int width;
    int x;
    int y;
    int num_glyphs;
    int glyph_data_offset;
    QFontEngine *fontEngine;
};

struct QScriptItemArrayPrivate
{
    unsigned int alloc;
    unsigned int size;
    QScriptItem items[1];
};

class QScriptItemArray
{
public:
    QScriptItemArray() : d( 0 ) {}
    ~QScriptItemArray();

    inline QScriptItem &operator[] (int i) const {return d->items[i];   }
    inline void append( const QScriptItem &item ) {
	if ( d->size == d->alloc )
	    resize( d->size + 1 );
	d->items[d->size] = item;
	d->size++;
    }
    inline int size() const { return d ? d->size : 0; }

    void resize( int s );
    void clear();

    QScriptItemArrayPrivate *d;
private:
#ifdef Q_DISABLE_COPY
    QScriptItemArray( const QScriptItemArray & );
    QScriptItemArray &operator = ( const QScriptItemArray & );
#endif
};

class QFontPrivate;

class QTextEngine {
public:
    QTextEngine( const QString &str, QFontPrivate *f );
    ~QTextEngine();

    enum Mode {
	Full = 0x00,
	NoBidi = 0x01,
	SingleLine = 0x02,
	WidthOnly = 0x07
    };

    void itemize( int mode = Full );

    static void bidiReorder( int numRuns, const Q_UINT8 *levels, int *visualOrder );

    const QCharAttributes *attributes();
    void shape( int item ) const;

    // ### we need something for justification

    enum Edge {
	Leading,
	Trailing
    };
    enum ShaperFlag {
        RightToLeft = 0x0001,
        Mirrored = 0x0001
    };

    int width( int charFrom, int numChars ) const;
    glyph_metrics_t boundingBox( int from,  int len ) const;

    QScriptItemArray items;
    QString string;
    QFontPrivate *fnt;
    int lineWidth;
    int widthUsed;
    int firstItemInLine;
    int currentItem;
    QChar::Direction direction : 5;
    unsigned int haveCharAttributes : 1;
    unsigned int widthOnly : 1;
    unsigned int reserved : 25;

    int length( int item ) const {
	const QScriptItem &si = items[item];
	int from = si.position;
	item++;
	return ( item < items.size() ? items[item].position : string.length() ) - from;
    }
    void splitItem( int item, int pos );

    unsigned short *logClustersPtr;
    glyph_t *glyphPtr;
    advance_t *advancePtr;
    qoffset_t *offsetsPtr;
    GlyphAttributes *glyphAttributesPtr;

    inline unsigned short *logClusters( const QScriptItem *si ) const
	{ return logClustersPtr+si->position; }
    inline glyph_t *glyphs( const QScriptItem *si ) const
	{ return glyphPtr+si->glyph_data_offset; }
    inline advance_t *advances( const QScriptItem *si ) const
	{ return advancePtr+si->glyph_data_offset; }
    inline qoffset_t *offsets( const QScriptItem *si ) const
	{ return offsetsPtr+si->glyph_data_offset; }
    inline GlyphAttributes *glyphAttributes( const QScriptItem *si ) const
	{ return glyphAttributesPtr+si->glyph_data_offset; }

    void reallocate( int totalGlyphs );
    inline void ensureSpace( int nGlyphs ) const {
	if ( num_glyphs - used < nGlyphs )
	    ((QTextEngine *)this)->reallocate( ( (used + nGlyphs + 16) >> 4 ) << 4 );
    }

    int allocated;
    void **memory;
    int num_glyphs;
    int used;
};

#endif
