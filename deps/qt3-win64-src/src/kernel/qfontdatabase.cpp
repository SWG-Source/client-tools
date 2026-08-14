/****************************************************************************
** $Id: qfontdatabase.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of font database class.
**
** Created : 990603
**
** Copyright (C) 1999-2007 Trolltech ASA.  All rights reserved.
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

#include "qfontdatabase.h"

#ifndef QT_NO_FONTDATABASE

#include <qtl.h>
#include <qapplication.h>

#include <private/qunicodetables_p.h>
#include "qfontengine_p.h"

#include <qcleanuphandler.h>

#ifdef Q_WS_X11
#include <locale.h>
#endif
#include <stdlib.h>

//#define QFONTDATABASE_DEBUG
#ifdef QFONTDATABASE_DEBUG
#  define FD_DEBUG qDebug
#else
#  define FD_DEBUG if (FALSE) qDebug
#endif

//#define FONT_MATCH_DEBUG
#ifdef FONT_MATCH_DEBUG
#  define FM_DEBUG qDebug
#else
#  define FM_DEBUG if (FALSE) qDebug
#endif

#if defined(Q_CC_MSVC) && !defined(Q_CC_MSVC_NET)
#  define for if(0){}else for
#endif

static int ucstricmp( const QString &as, const QString &bs )
{
    const QChar *a = as.unicode();
    const QChar *b = bs.unicode();
    if ( a == b )
	return 0;
    if ( a == 0 )
	return 1;
    if ( b == 0 )
	return -1;
    int l=QMIN(as.length(),bs.length());
    while ( l-- && ::lower( *a ) == ::lower( *b ) )
	a++,b++;
    if ( l==-1 )
	return ( as.length()-bs.length() );
    return ::lower( *a ).unicode() - ::lower( *b ).unicode();
}

static int getFontWeight( const QString &weightString )
{
    QString s = weightString.lower();

    // Test in decreasing order of commonness
    if (s == "medium" ||
	s == "normal")
	return QFont::Normal;
    if (s == "bold")
	return QFont::Bold;
    if (s == "demibold" || s == "demi bold")
	return QFont::DemiBold;
    if (s == "black")
	return QFont::Black;
    if (s == "light")
	return QFont::Light;

    if (s.contains("bold")) {
	if (s.contains("demi"))
	    return (int) QFont::DemiBold;
	return (int) QFont::Bold;
    }

    if (s.contains("light"))
	return (int) QFont::Light;

    if (s.contains("black"))
	return (int) QFont::Black;

    return (int) QFont::Normal;
}

#ifdef Q_WS_X11
struct QtFontEncoding
{
    signed int encoding : 16;

    uint xpoint   : 16;
    uint xres     : 8;
    uint yres     : 8;
    uint avgwidth : 16;
    uchar pitch   : 8;
};
#endif // Q_WS_X11

struct QtFontSize
{
    unsigned short pixelSize;

#ifdef Q_WS_X11
    int count;
    QtFontEncoding *encodings;
    QtFontEncoding *encodingID( int id, uint xpoint = 0, uint xres = 0,
				uint yres = 0, uint avgwidth = 0, bool add = FALSE);
#endif // Q_WS_X11
};


#ifdef Q_WS_X11
QtFontEncoding *QtFontSize::encodingID( int id, uint xpoint, uint xres,
					uint yres, uint avgwidth, bool add )
{
    // we don't match using the xpoint, xres and yres parameters, only the id
    for ( int i = 0; i < count; ++i ) {
	if ( encodings[i].encoding == id )
	    return encodings + i;
    }

    if ( !add ) return 0;

    if ( !(count % 4) )
	encodings = ( QtFontEncoding * )
		    realloc( encodings,
			     (((count+4) >> 2 ) << 2 ) * sizeof( QtFontEncoding ) );
    encodings[count].encoding = id;
    encodings[count].xpoint = xpoint;
    encodings[count].xres = xres;
    encodings[count].yres = yres;
    encodings[count].avgwidth = avgwidth;
    encodings[count].pitch = '*';
    return encodings + count++;
}
#endif // Q_WS_X11

struct QtFontStyle
{
    struct Key {
	Key( const QString &styleString );
	Key() : italic( FALSE ), oblique( FALSE ),
		weight( QFont::Normal ), stretch( 0 ) { }
	Key( const Key &o ) : italic( o.italic ), oblique( o.oblique ),
			      weight( o.weight ), stretch( o.stretch ) { }
	uint italic : 1;
	uint oblique : 1;
	signed int  weight : 8;
	signed int stretch : 12;

	bool operator==( const Key & other ) {
	    return ( italic == other.italic &&
		     oblique == other.oblique &&
		     weight == other.weight &&
		     (stretch == 0 || other.stretch == 0 || stretch == other.stretch) );
	}
	bool operator!=( const Key &other ) {
	    return !operator==(other);
	}
	bool operator <( const Key &o ) {
	    int x = (italic << 13) + (oblique << 12) + (weight << 14) + stretch;
	    int y = (o.italic << 13) + (o.oblique << 12) + (o.weight << 14) + o.stretch;
	    return ( x < y );
	}
    };

    QtFontStyle( const Key &k )
	: key( k ), bitmapScalable( FALSE ), smoothScalable( FALSE ),
	  fakeOblique( FALSE ), count( 0 ), pixelSizes( 0 )
    {
#if defined(Q_WS_X11)
	weightName = setwidthName = 0;
#endif // Q_WS_X11
    }

    ~QtFontStyle() {
#ifdef Q_WS_X11
	delete [] weightName;
	delete [] setwidthName;
	while ( count-- )
	    free(pixelSizes[count].encodings);
#endif
	free( pixelSizes );
    }

    Key key;
    bool bitmapScalable : 1;
    bool smoothScalable : 1;
    bool fakeOblique    : 1;
    int count           : 29;
    QtFontSize *pixelSizes;

#ifdef Q_WS_X11
    const char *weightName;
    const char *setwidthName;
#endif // Q_WS_X11

    QtFontSize *pixelSize( unsigned short size, bool = FALSE );
};

QtFontStyle::Key::Key( const QString &styleString )
    : italic( FALSE ), oblique( FALSE ), weight( QFont::Normal ), stretch( 0 )
{
    weight = getFontWeight( styleString );

    if ( styleString.contains( "Italic" ) )
	 italic = TRUE;
    else if ( styleString.contains( "Oblique" ) )
	 oblique = TRUE;
}

QtFontSize *QtFontStyle::pixelSize( unsigned short size, bool add )
{
    for ( int i = 0; i < count; i++ ) {
	if ( pixelSizes[i].pixelSize == size )
	    return pixelSizes + i;
    }
    if ( !add )
	return 0;

    if ( !(count % 8) )
	pixelSizes = (QtFontSize *)
		     realloc( pixelSizes,
			      (((count+8) >> 3 ) << 3) * sizeof(QtFontSize) );
    pixelSizes[count].pixelSize = size;
#ifdef Q_WS_X11
    pixelSizes[count].count = 0;
    pixelSizes[count].encodings = 0;
#endif
    return pixelSizes + (count++);
}

struct QtFontFoundry
{
    QtFontFoundry( const QString &n ) : name( n ), count( 0 ), styles( 0 ) {}
    ~QtFontFoundry() {
	while ( count-- )
	    delete styles[count];
	free( styles );
    }

    QString name;

    int count;
    QtFontStyle **styles;
    QtFontStyle *style( const QtFontStyle::Key &,  bool = FALSE );
};

QtFontStyle *QtFontFoundry::style( const QtFontStyle::Key &key, bool create )
{
    int pos = 0;
    if ( count ) {
	int low = 0;
	int high = count;
	pos = count / 2;
	while ( high > low ) {
	    if ( styles[pos]->key == key )
		return styles[pos];
	    if ( styles[pos]->key < key )
		low = pos + 1;
	    else
		high = pos;
	    pos = (high + low) / 2;
	};
	pos = low;
    }
    if ( !create )
	return 0;

//     qDebug("adding key (weight=%d, italic=%d, oblique=%d stretch=%d) at %d",  key.weight, key.italic, key.oblique, key.stretch, pos );
    if ( !(count % 8) )
	styles = (QtFontStyle **)
		 realloc( styles, (((count+8) >> 3 ) << 3) * sizeof( QtFontStyle * ) );

    memmove( styles + pos + 1, styles + pos, (count-pos)*sizeof(QtFontStyle *) );
    styles[pos] = new QtFontStyle( key );
    count++;
    return styles[pos];
}


struct QtFontFamily
{
    enum ScriptStatus { Unknown = 0, Supported = 1,
			UnSupported_Xft= 2, UnSupported_Xlfd = 4, UnSupported = 6 };

    QtFontFamily(const QString &n )
	:
#ifdef Q_WS_X11
        fixedPitch( TRUE ), hasXft( FALSE ), xftScriptCheck( FALSE ), xlfdLoaded( FALSE ), synthetic(FALSE),
#else
        fixedPitch( FALSE ),
#endif
#ifdef Q_WS_WIN
	scriptCheck( FALSE ),
#endif
#if defined(Q_OS_MAC) && !defined(QWS)
	fixedPitchComputed(FALSE),
#endif
	fullyLoaded( FALSE ),
	  name( n ), count( 0 ), foundries( 0 ) {
	memset( scripts, 0, sizeof( scripts ) );
    }
    ~QtFontFamily() {
	while ( count-- )
	    delete foundries[count];
	free( foundries );
    }

    bool fixedPitch : 1;
#ifdef Q_WS_X11
    bool hasXft : 1;
    bool xftScriptCheck : 1;
    bool xlfdLoaded : 1;
    bool synthetic : 1;
#endif
#ifdef Q_WS_WIN
    bool scriptCheck : 1;
#endif
#if defined(Q_OS_MAC) && !defined(QWS)
    bool fixedPitchComputed : 1;
#endif
    bool fullyLoaded : 1;
    QString name;
    QString rawName;
#ifdef Q_WS_X11
    QCString fontFilename;
    int fontFileIndex;
#endif
#ifdef Q_WS_MAC
    FMFontFamily macFamily;
#endif
#ifdef Q_WS_WIN
    QString english_name;
#endif
    int count;
    QtFontFoundry **foundries;

    unsigned char scripts[QFont::LastPrivateScript];

    QtFontFoundry *foundry( const QString &f, bool = FALSE );
};

QtFontFoundry *QtFontFamily::foundry( const QString &f, bool create )
{
    if ( f.isNull() && count == 1 )
	return foundries[0];

    for ( int i = 0; i < count; i++ ) {
	if ( ucstricmp( foundries[i]->name, f ) == 0 )
	    return foundries[i];
    }
    if ( !create )
	return 0;

    if ( !(count % 8) )
	foundries = (QtFontFoundry **)
		    realloc( foundries,
			     (((count+8) >> 3 ) << 3) * sizeof( QtFontFoundry * ) );

    foundries[count] = new QtFontFoundry( f );
    return foundries[count++];
}

class QFontDatabasePrivate {
public:
    QFontDatabasePrivate() : count( 0 ), families( 0 ) { }
    ~QFontDatabasePrivate() {
	while ( count-- )
	    delete families[count];
	free( families );
    }
    QtFontFamily *family( const QString &f, bool = FALSE );

    int count;
    QtFontFamily **families;
};

QtFontFamily *QFontDatabasePrivate::family( const QString &f, bool create )
{
    int low = 0;
    int high = count;
    int pos = count / 2;
    int res = 1;
    if ( count ) {
	while ( (res = ucstricmp( families[pos]->name, f )) && pos != low ) {
	    if ( res > 0 )
		high = pos;
	    else
		low = pos;
	    pos = (high + low) / 2;
	};
	if ( !res )
	    return families[pos];
    }
    if ( !create )
	return 0;

    if ( res < 0 )
	pos++;

    // qDebug("adding family %s at %d total=%d",  f.latin1(), pos, count);
    if ( !(count % 8) )
	families = (QtFontFamily **)
		   realloc( families,
			    (((count+8) >> 3 ) << 3) * sizeof( QtFontFamily * ) );

    memmove( families + pos + 1, families + pos, (count-pos)*sizeof(QtFontFamily *) );
    families[pos] = new QtFontFamily( f );
    count++;
    return families[pos];
}




#if defined(Q_WS_X11) || defined(Q_WS_WIN)
static const unsigned short sample_chars[QFont::LastPrivateScript][14] =
{
    // European Alphabetic Scripts
    // Latin,
    { 0x0041, 0x0 },
    // Greek,
    { 0x0391, 0x0 },
    // Cyrillic,
    { 0x0410, 0x0 },
    // Armenian,
    { 0x0540, 0x0 },
    // Georgian,
    { 0x10d0, 0x0 },
    // Runic,
    { 0x16a0, 0x0 },
    // Ogham,
    { 0x1680, 0x0 },
    // SpacingModifiers,
    { 0x02c6, 0x0 },
    // CombiningMarks,
    { 0x0300, 0x0 },

    // Middle Eastern Scripts
    // Hebrew,
    { 0x05d0, 0x0 },
    // Arabic,
    { 0x0630, 0x0 },
    // Syriac,
    { 0x0710, 0x0 },
    // Thaana,
    { 0x0780, 0x0 },

    // South and Southeast Asian Scripts
    // Devanagari,
    { 0x0910, 0x0 },
    // Bengali,
    { 0x0990, 0x0 },
    // Gurmukhi,
    { 0x0a10, 0x0 },
    // Gujarati,
    { 0x0a90, 0x0 },
    // Oriya,
    { 0x0b10, 0x0 },
    // Tamil,
    { 0x0b90, 0x0 },
    // Telugu,
    { 0x0c10, 0x0 },
    // Kannada,
    { 0x0c90, 0x0 },
    // Malayalam,
    { 0x0d10, 0x0 },
    // Sinhala,
    { 0x0d90, 0x0 },
    // Thai,
    { 0x0e10, 0x0 },
    // Lao,
    { 0x0e81, 0x0 },
    // Tibetan,
    { 0x0f00, 0x0 },
    // Myanmar,
    { 0x1000, 0x0 },
    // Khmer,
    { 0x1780, 0x0 },

    // East Asian Scripts
    // Han,
    { 0x4e00, 0x0 },
    // Hiragana,
    {  0x3050, 0x4e00, 0x25EF, 0x3012, 0x3013, 0x30FB, 0x30FC, 0x5CE0, 0 },
    // Katakana,
    { 0x30b0, 0x4e00, 0x25EF, 0x3012, 0x3013, 0x30FB, 0x30FC, 0x5CE0, 0 },
    // Hangul,
    { 0xac00, 0x0 },
    // Bopomofo,
    { 0x3110, 0x0 },
    // Yi,
    { 0xa000, 0x0 },

    // Additional Scripts
    // Ethiopic,
    { 0x1200, 0x0 },
    // Cherokee,
    { 0x13a0, 0x0 },
    // CanadianAboriginal,
    { 0x1410, 0x0 },
    // Mongolian,
    { 0x1800, 0x0 },

    // Symbols
    // CurrencySymbols,
    { 0x20aa, 0x0 },
    // LetterlikeSymbols,
    { 0x2103, 0x0 },
    // NumberForms,
    { 0x2160, 0x0 },
    // MathematicalOperators,
    { 0x222b, 0x0 },
    // TechnicalSymbols,
    { 0x2312, 0x0 },
    // GeometricSymbols,
    { 0x2500, 0x0 },
    // MiscellaneousSymbols,
    { 0x2640, 0x0 },
    // EnclosedAndSquare,
    { 0x2460, 0x0 },
    // Braille,
    { 0x2800, 0x0 },

    // Unicode,
    { 0xfffd, 0x0 },

    // some scripts added in Unicode 3.2
    // Tagalog,
    { 0x1700, 0x0 },
    // Hanunoo,
    { 0x1720, 0x0 },
    // Buhid,
    { 0x1740, 0x0 },
    // Tagbanwa,
    { 0x1770, 0x0 },

    // KatakanaHalfWidth
    { 0xff65, 0x0 },

    // Limbu
    { 0x1901, 0x0 },
    // TaiLe
    { 0x1950, 0x0 },

    // NScripts
    { 0x0000, 0x0 },
    // NoScript
    { 0x0000, 0x0 },

    // Han_Japanese
    { 0x4e00, 0x25EF, 0x3012, 0x3013, 0x30FB, 0x5CE0, 0xFF66, 0 },
    // Han_SimplifiedChinese, 0x3400 is optional
    { 0x4e00, 0x201C, 0x3002, 0x6237, 0x9555, 0xFFE5, 0 },
    // Han_TraditionalChinese, 0xF6B1 is optional
    // OR Han_HongkongChinese, 0x3435, 0xE000, 0xF6B1 are optional
    { 0x4e00, 0x201C, 0x3002, 0x6236, 0x9F98, 0xFFE5, 0 },
    // Han_Korean
    { 0x4e00, 0 }
    // Taiwan would be 0x201C, 0x3002, 0x4E00, 0x9F98, 0xFFE5
};

#if defined(Q_WS_X11) && !defined(QT_NO_XFTFREETYPE)
static inline bool requiresOpenType(QFont::Script s)
{
    return (s >= QFont::Syriac && s <= QFont::Sinhala)
		 || (s >= QFont::Myanmar && s <= QFont::Khmer);
}
#endif

static inline bool canRender( QFontEngine *fe, QFont::Script script )
{
    if ( !fe ) return FALSE;

    bool hasChar = true;

    if (!sample_chars[script][0])
        hasChar = false;

    int i = 0;
    while (hasChar && sample_chars[script][i]){
        QChar sample(sample_chars[script][i]);
        if ( !fe->canRender( &sample, 1  ) ) {
             hasChar = false;
#ifdef FONT_MATCH_DEBUG
	FM_DEBUG("    font has NOT char 0x%04x", sample.unicode() );
        } else {
	FM_DEBUG("    font has char 0x%04x", sample.unicode() );
#endif
        }
        ++i;
    }
#if defined(Q_WS_X11) && !defined(QT_NO_XFTFREETYPE)
    if (hasChar && requiresOpenType(script)) {
	QOpenType *ot = fe->openType();
	if (!ot || !ot->supportsScript(script))
	    return FALSE;
    }
#endif

    return hasChar;
}
#endif // Q_WS_X11 || Q_WS_WIN


static QSingleCleanupHandler<QFontDatabasePrivate> qfontdatabase_cleanup;
static QFontDatabasePrivate *db=0;
#define SMOOTH_SCALABLE 0xffff

#if defined( Q_WS_X11 )
#  include "qfontdatabase_x11.cpp"
#elif defined( Q_WS_MAC )
#  include "qfontdatabase_mac.cpp"
#elif defined( Q_WS_WIN )
#  include "qfontdatabase_win.cpp"
#elif defined( Q_WS_QWS )
#  include "qfontdatabase_qws.cpp"
#endif

static QtFontStyle *bestStyle(QtFontFoundry *foundry, const QtFontStyle::Key &styleKey)
{
    int best = 0;
    int dist = 0xffff;

    for ( int i = 0; i < foundry->count; i++ ) {
	QtFontStyle *style = foundry->styles[i];

	int d = QABS( styleKey.weight - style->key.weight );

	if ( styleKey.stretch != 0 && style->key.stretch != 0 ) {
	    d += QABS( styleKey.stretch - style->key.stretch );
	}

	if ( styleKey.italic ) {
	    if ( !style->key.italic )
		d += style->key.oblique ? 0x0001 : 0x1000;
	} else if ( styleKey.oblique ) {
	    if (!style->key.oblique )
		d += style->key.italic ? 0x0001 : 0x1000;
	} else if ( style->key.italic || style->key.oblique ) {
	    d += 0x1000;
	}

	if ( d < dist ) {
	    best = i;
	    dist = d;
	}
    }

    FM_DEBUG( "          best style has distance 0x%x", dist );
    return foundry->styles[best];
}

#if defined(Q_WS_X11)
static QtFontEncoding *findEncoding(QFont::Script script, int styleStrategy,
                                    QtFontSize *size, int force_encoding_id)
{
    QtFontEncoding *encoding = 0;

    if (force_encoding_id >= 0) {
        encoding = size->encodingID(force_encoding_id);
        if (!encoding)
            FM_DEBUG("            required encoding_id not available");
        return encoding;
    }

    if (styleStrategy & (QFont::OpenGLCompatible | QFont::PreferBitmap)) {
        FM_DEBUG("            PreferBitmap and/or OpenGL set, skipping Xft");
    } else {
        encoding = size->encodingID(-1); // -1 == prefer Xft
        if (encoding) return encoding;
    }

    // Xft not available, find an XLFD font, trying the default encoding first
    encoding = size->encodingID(QFontPrivate::defaultEncodingID);

    if (!encoding || !scripts_for_xlfd_encoding[encoding->encoding][script]) {
        // find the first encoding that supports the requested script
        encoding = 0;
        for (int x = 0; !encoding && x < size->count; ++x) {
            const int enc = size->encodings[x].encoding;
            if (scripts_for_xlfd_encoding[enc][script]) {
                encoding = size->encodings + x;
                break;
            }
        }
    }

    return encoding;
}
#endif // Q_WS_X11


#if defined(Q_WS_X11) || defined(Q_WS_WIN)
static
unsigned int bestFoundry( QFont::Script script, unsigned int score, int styleStrategy,
			  const QtFontFamily *family, const QString &foundry_name,
			  QtFontStyle::Key styleKey, int pixelSize, char pitch,
			  QtFontFoundry **best_foundry, QtFontStyle **best_style,
			  QtFontSize **best_size
#ifdef Q_WS_X11
			  , QtFontEncoding **best_encoding, int force_encoding_id
#endif
			  )
{
    Q_UNUSED( script );
    Q_UNUSED( pitch );

    FM_DEBUG( "  REMARK: looking for best foundry for family '%s'", family->name.latin1() );

    for ( int x = 0; x < family->count; ++x ) {
	QtFontFoundry *foundry = family->foundries[x];
	if ( ! foundry_name.isEmpty() &&
	     ucstricmp( foundry->name, foundry_name ) != 0 )
	    continue;

	FM_DEBUG( "          looking for matching style in foundry '%s'",
		  foundry->name.isEmpty() ? "-- none --" : foundry->name.latin1() );

	QtFontStyle *style = bestStyle(foundry, styleKey);

	if ( ! style->smoothScalable && ( styleStrategy & QFont::ForceOutline ) ) {
	    FM_DEBUG( "            ForceOutline set, but not smoothly scalable" );
	    continue;
	}

	int px = -1;
	QtFontSize *size = 0;

	// 1. see if we have an exact matching size
	if (! (styleStrategy & QFont::ForceOutline)) {
	    size = style->pixelSize(pixelSize);
	    if (size) {
                FM_DEBUG("          found exact size match (%d pixels)", size->pixelSize);
                px = size->pixelSize;
            }
	}

	// 2. see if we have a smoothly scalable font
	if (! size && style->smoothScalable && ! (styleStrategy & QFont::PreferBitmap)) {
	    size = style->pixelSize(SMOOTH_SCALABLE);
	    if (size) {
                FM_DEBUG("          found smoothly scalable font (%d pixels)", pixelSize);
                px = pixelSize;
            }
	}

	// 3. see if we have a bitmap scalable font
	if (! size && style->bitmapScalable && (styleStrategy & QFont::PreferMatch)) {
	    size = style->pixelSize(0);
	    if (size) {
                FM_DEBUG("          found bitmap scalable font (%d pixels)", pixelSize);
                px = pixelSize;
            }
	}

#ifdef Q_WS_X11
        QtFontEncoding *encoding = 0;
#endif

	// 4. find closest size match
	if (! size) {
	    unsigned int distance = ~0u;
	    for (int x = 0; x < style->count; ++x) {
#ifdef Q_WS_X11
                encoding =
                    findEncoding(script, styleStrategy, style->pixelSizes + x, force_encoding_id);
                if (!encoding) {
                    FM_DEBUG("          size %3d does not support the script we want",
                             style->pixelSizes[x].pixelSize);
                    continue;
                }
#endif

		unsigned int d = QABS(style->pixelSizes[x].pixelSize - pixelSize);
		if (d < distance) {
		    distance = d;
		    size = style->pixelSizes + x;
		    FM_DEBUG("          best size so far: %3d (%d)", size->pixelSize, pixelSize);
		}
	    }

            if (!size) {
                FM_DEBUG("          no size supports the script we want");
                continue;
            }

	    if (style->bitmapScalable && ! (styleStrategy & QFont::PreferQuality) &&
		(distance * 10 / pixelSize) >= 2) {
		// the closest size is not close enough, go ahead and
		// use a bitmap scaled font
		size = style->pixelSize(0);
		px = pixelSize;
	    } else {
		px = size->pixelSize;
	    }
	}

#ifdef Q_WS_X11
        if (size) {
            encoding = findEncoding(script, styleStrategy, size, force_encoding_id);
            if (!encoding) size = 0;
        }
	if ( ! encoding ) {
	    FM_DEBUG( "          foundry doesn't support the script we want" );
	    continue;
	}
#endif // Q_WS_X11

	unsigned int this_score = 0x0000;
        enum {
            PitchMismatch       = 0x4000,
            StyleMismatch       = 0x2000,
            BitmapScaledPenalty = 0x1000,
            EncodingMismatch    = 0x0002,
            XLFDPenalty         = 0x0001
        };

#ifdef Q_WS_X11
	if ( encoding->encoding != -1 ) {
	    this_score += XLFDPenalty;
	    if ( encoding->encoding != QFontPrivate::defaultEncodingID )
		this_score += EncodingMismatch;
	}
 	if (pitch != '*') {
 	    if ( !( pitch == 'm' && encoding->pitch == 'c' ) && pitch != encoding->pitch )
 		this_score += PitchMismatch;
 	}
#else
        // ignore pitch for asian fonts, some of them misreport it, and they are all
        // fixed pitch anyway.
        if (pitch != '*' && (script <= QFont::NScripts && script != QFont::KatakanaHalfWidth
                             && (script < QFont::Han || script > QFont::Yi))) {
	    if ((pitch == 'm' && !family->fixedPitch)
		|| (pitch == 'p' && family->fixedPitch))
		this_score += PitchMismatch;
	}
#endif
	if ( styleKey != style->key )
	    this_score += StyleMismatch;
	if ( !style->smoothScalable && px != size->pixelSize ) // bitmap scaled
	    this_score += BitmapScaledPenalty;
        if (px != pixelSize) // close, but not exact, size match
            this_score += QABS(px - pixelSize);

	if ( this_score < score ) {
	    FM_DEBUG( "          found a match: score %x best score so far %x",
		      this_score, score );

	    score = this_score;
	    *best_foundry = foundry;
	    *best_style = style;
	    *best_size = size;
#ifdef Q_WS_X11
	    *best_encoding = encoding;
#endif // Q_WS_X11
	} else {
	    FM_DEBUG( "          score %x no better than best %x", this_score, score);
	}
    }

    return score;
}

/*!
    \internal
*/
QFontEngine *
QFontDatabase::findFont( QFont::Script script, const QFontPrivate *fp,
			 const QFontDef &request, int force_encoding_id )
{
#ifndef Q_WS_X11
    Q_UNUSED( force_encoding_id );
#endif

    if ( !db )
	initializeDb();

    QFontEngine *fe = 0;
    if ( fp ) {
	if ( fp->rawMode ) {
	    fe = loadEngine( script, fp, request, 0, 0, 0
#ifdef Q_WS_X11
			     , 0, 0, FALSE
#endif
		);

	    // if we fail to load the rawmode font, use a 12pixel box engine instead
	    if (! fe) fe = new QFontEngineBox( 12 );
	    return fe;
	}

	QFontCache::Key key( request, script,
#ifdef Q_WS_WIN
			     (int)fp->paintdevice, 
#else
			     fp->screen, 
#endif
                             fp->paintdevice
	    );
	fe = QFontCache::instance->findEngine( key );
	if ( fe ) return fe;
    }

#ifdef Q_WS_WIN
    if (request.styleStrategy & QFont::PreferDevice) {
        QFontEngine *fe = loadEngine(script, fp, request, 0, 0, 0);
        if(fe)
            return fe;
    }
#endif

    QString family_name, foundry_name;
    QtFontStyle::Key styleKey;
    styleKey.italic = request.italic;
    styleKey.weight = request.weight;
    styleKey.stretch = request.stretch;
    char pitch = request.ignorePitch ? '*' : request.fixedPitch ? 'm' : 'p';

    parseFontName( request.family, foundry_name, family_name );

#ifdef Q_WS_X11
    if (script == QFont::Han) {
	// modify script according to locale
	static QFont::Script defaultHan = QFont::UnknownScript;
	if (defaultHan == QFont::UnknownScript) {
	    QCString locale = setlocale(LC_ALL, NULL);
	    if (locale.contains("ko"))
		defaultHan = QFont::Han_Korean;
	    else if (locale.contains("zh_TW") || locale.contains("zh_HK"))
		defaultHan = QFont::Han_TraditionalChinese;
	    else if (locale.contains("zh"))
		defaultHan = QFont::Han_SimplifiedChinese;
	    else
		defaultHan = QFont::Han_Japanese;
	}
	script = defaultHan;
    }
#endif

    FM_DEBUG( "QFontDatabase::findFont\n"
	      "  request:\n"
	      "    family: %s [%s], script: %d (%s)\n"
	      "    weight: %d, italic: %d\n"
	      "    stretch: %d\n"
	      "    pixelSize: %d\n"
	      "    pitch: %c",
	      family_name.isEmpty() ? "-- first in script --" : family_name.latin1(),
	      foundry_name.isEmpty() ? "-- any --" : foundry_name.latin1(),
	      script, scriptName( script ).latin1(),
	      request.weight, request.italic, request.stretch, request.pixelSize, pitch );

    bool usesFontConfig = FALSE;
#ifdef QT_XFT2
    if (family_name.isEmpty()
	|| family_name == "Sans Serif"
	|| family_name == "Serif"
	|| family_name == "Monospace") {
	fe = loadFontConfigFont(fp, request, script);
        usesFontConfig = (fe != 0);
    }
    if (!fe)
#endif
    {
	QtFontFamily *best_family = 0;
	QtFontFoundry *best_foundry = 0;
	QtFontStyle *best_style = 0;
	QtFontSize *best_size = 0;
#ifdef Q_WS_X11
	QtFontEncoding *best_encoding = 0;
#endif // Q_WS_X11

	unsigned int score = ~0;

	load( family_name, script );

	for ( int x = 0; x < db->count; ++x ) {
	    QtFontFamily *try_family = db->families[x];
#ifdef Q_WS_X11
            if (try_family->synthetic) // skip generated fontconfig fonts
                continue;
#endif

            if ( !family_name.isEmpty() &&
		 ucstricmp( try_family->name, family_name ) != 0
#ifdef Q_WS_WIN
		 && ucstricmp( try_family->english_name, family_name ) != 0
#endif
                )
		continue;

	    if ( family_name.isEmpty() )
		load( try_family->name, script );

	    uint score_adjust = 0;
	    QFont::Script override_script = script;
	    if ( ! ( try_family->scripts[script] & QtFontFamily::Supported )
		 && script != QFont::Unicode) {
		// family not supported in the script we want
#ifdef Q_WS_X11
		if (script >= QFont::Han_Japanese && script <= QFont::Han_Korean
		    && try_family->scripts[QFont::Han] == QtFontFamily::Supported) {
		    // try with the han script instead, give it a penalty
		    if (override_script == QFont::Han_TraditionalChinese
			&& (try_family->scripts[QFont::Han_SimplifiedChinese] & QtFontFamily::Supported)) {
			override_script = QFont::Han_SimplifiedChinese;
			score_adjust = 200;
		    } else if (override_script == QFont::Han_SimplifiedChinese
                               && (try_family->scripts[QFont::Han_TraditionalChinese] & QtFontFamily::Supported)) {
			override_script = QFont::Han_TraditionalChinese;
			score_adjust = 200;
		    } else {
			override_script = QFont::Han;
			score_adjust = 400;
		    }
		} else
#endif
                    if (family_name.isEmpty()) {
                        continue;
                    } else if (try_family->scripts[QFont::UnknownScript] & QtFontFamily::Supported) {
                        // try with the unknown script (for a symbol font)
                        override_script = QFont::UnknownScript;
#ifndef QT_XFT2
                    } else if (try_family->scripts[QFont::Unicode] & QtFontFamily::Supported) {
                        // try with the unicode script instead
                        override_script = QFont::Unicode;
#endif
                    } else {
                        // family not supported by unicode/unknown scripts
                        continue;
                    }
	    }

	    QtFontFoundry *try_foundry = 0;
	    QtFontStyle *try_style = 0;
	    QtFontSize *try_size = 0;
#ifdef Q_WS_X11
	    QtFontEncoding *try_encoding = 0;
#endif // Q_WS_X11

	    // as we know the script is supported, we can be sure
	    // to find a matching font here.
	    unsigned int newscore =
		bestFoundry( override_script, score, request.styleStrategy,
			     try_family, foundry_name, styleKey, request.pixelSize, pitch,
			     &try_foundry, &try_style, &try_size
#ifdef Q_WS_X11
			     , &try_encoding, force_encoding_id
#endif
		    );
	    if ( try_foundry == 0 ) {
		// the specific foundry was not found, so look for
		// any foundry matching our requirements
		newscore = bestFoundry( override_script, score, request.styleStrategy, try_family,
					QString::null, styleKey, request.pixelSize,
					pitch, &try_foundry, &try_style, &try_size
#ifdef Q_WS_X11
					, &try_encoding, force_encoding_id
#endif
		    );
	    }
	    newscore += score_adjust;

	    if ( newscore < score ) {
		score = newscore;
		best_family = try_family;
		best_foundry = try_foundry;
		best_style = try_style;
		best_size = try_size;
#ifdef Q_WS_X11
		best_encoding = try_encoding;
#endif // Q_WS_X11
	    }
	    if ( newscore < 10 ) // xlfd instead of xft... just accept it
		break;
	}

	if ( best_family != 0 && best_foundry != 0 && best_style != 0
#ifdef Q_WS_X11
	     && best_size != 0 && best_encoding != 0
#endif
	    ) {
	    FM_DEBUG( "  BEST:\n"
		      "    family: %s [%s]\n"
		      "    weight: %d, italic: %d, oblique: %d\n"
		      "    stretch: %d\n"
		      "    pixelSize: %d\n"
		      "    pitch: %c\n"
		      "    encoding: %d\n",
		      best_family->name.latin1(),
		      best_foundry->name.isEmpty() ? "-- none --" : best_foundry->name.latin1(),
		      best_style->key.weight, best_style->key.italic, best_style->key.oblique,
		      best_style->key.stretch, best_size ? best_size->pixelSize : 0xffff,
#ifdef Q_WS_X11
		      best_encoding->pitch, best_encoding->encoding
#else
		      'p', 0
#endif
		);

	    fe = loadEngine( script, fp, request, best_family, best_foundry, best_style
#ifdef Q_WS_X11
			     , best_size, best_encoding, ( force_encoding_id >= 0 )
#endif
		);
	}
	if (fe) {
	    fe->fontDef.family = best_family->name;
	    if ( ! best_foundry->name.isEmpty() ) {
		fe->fontDef.family += QString::fromLatin1( " [" );
		fe->fontDef.family += best_foundry->name;
		fe->fontDef.family += QString::fromLatin1( "]" );
	    }

	    if ( best_style->smoothScalable )
		fe->fontDef.pixelSize = request.pixelSize;
	    else if ( best_style->bitmapScalable &&
		      ( request.styleStrategy & QFont::PreferMatch ) )
		fe->fontDef.pixelSize = request.pixelSize;
	    else
		fe->fontDef.pixelSize = best_size->pixelSize;

	    fe->fontDef.styleHint     = request.styleHint;
	    fe->fontDef.styleStrategy = request.styleStrategy;

	    fe->fontDef.weight        = best_style->key.weight;
	    fe->fontDef.italic        = best_style->key.italic || best_style->key.oblique;
	    fe->fontDef.fixedPitch    = best_family->fixedPitch;
	    fe->fontDef.stretch       = best_style->key.stretch;
	    fe->fontDef.ignorePitch   = FALSE;
	}
    }

    if ( fe ) {
	if ( script != QFont::Unicode && !canRender( fe, script ) ) {
	    FM_DEBUG( "  WARN: font loaded cannot render a sample char" );

	    delete fe;
            fe = 0;
	} else if ( fp ) {
            QFontDef def = request;
            if (def.family.isEmpty()) {
                def.family = fp->request.family;
                def.family = def.family.left(def.family.find(','));
            }
	    QFontCache::Key key( def, script,
#ifdef Q_WS_WIN
				 (int)fp->paintdevice, 
#else
				 fp->screen, 
#endif
                                 fp->paintdevice
		);
	    QFontCache::instance->insertEngine( key, fe );
            if (!usesFontConfig) {
                for ( int i = 0; i < QFont::NScripts; ++i ) {
                    if ( i == script ) continue;

                    if (!canRender(fe, (QFont::Script) i))
                        continue;

                    key.script = i;
                    QFontCache::instance->insertEngine( key, fe );
                }
            }
	}
    }

    if (!fe) {
	if ( !request.family.isEmpty() )
            return 0;

        FM_DEBUG( "returning box engine" );

        fe = new QFontEngineBox( request.pixelSize );
        fe->fontDef = request;

        if ( fp ) {
            QFontCache::Key key( request, script,
#ifdef Q_WS_WIN
                                 (int)fp->paintdevice, 
#else
                                 fp->screen, 
#endif
                                 fp->paintdevice
                );
            QFontCache::instance->insertEngine( key, fe );
        }
    }

    if ( fp ) {
#if defined(Q_WS_X11)
        fe->fontDef.pointSize =
            qRound(10. * qt_pointSize(fe->fontDef.pixelSize, fp->paintdevice, fp->screen));
#elif defined(Q_WS_WIN)
        fe->fontDef.pointSize     = int( double( fe->fontDef.pixelSize ) * 720.0 /
                                         GetDeviceCaps(shared_dc,LOGPIXELSY) );
#else
        fe->fontDef.pointSize     = int( double( fe->fontDef.pixelSize ) * 720.0 /
                                         96.0 );
#endif
    } else {
        fe->fontDef.pointSize = request.pointSize;
    }

    return fe;
}
#endif // Q_WS_X11 || Q_WS_WIN




static QString styleString( int weight, bool italic, bool oblique )
{
    QString result;
    if ( weight >= QFont::Black )
	result = "Black";
    else if ( weight >= QFont::Bold )
	result = "Bold";
    else if ( weight >= QFont::DemiBold )
	result = "Demi Bold";
    else if ( weight < QFont::Normal )
	result = "Light";

    if ( italic )
	result += " Italic";
    else if ( oblique )
	result += " Oblique";

    if ( result.isEmpty() )
	result = "Normal";

    return result.simplifyWhiteSpace();
}

/*!
    Returns a string that describes the style of the font \a f. For
    example, "Bold Italic", "Bold", "Italic" or "Normal". An empty
    string may be returned.
*/
QString QFontDatabase::styleString( const QFont &f )
{
    // ### fix oblique here
    return ::styleString( f.weight(), f.italic(), FALSE );
}


/*!
    \class QFontDatabase qfontdatabase.h
    \brief The QFontDatabase class provides information about the fonts available in the underlying window system.

    \ingroup environment
    \ingroup graphics

    The most common uses of this class are to query the database for
    the list of font families() and for the pointSizes() and styles()
    that are available for each family. An alternative to pointSizes()
    is smoothSizes() which returns the sizes at which a given family
    and style will look attractive.

    If the font family is available from two or more foundries the
    foundry name is included in the family name, e.g. "Helvetica
    [Adobe]" and "Helvetica [Cronyx]". When you specify a family you
    can either use the old hyphenated Qt 2.x "foundry-family" format,
    e.g. "Cronyx-Helvetica", or the new bracketed Qt 3.x "family
    [foundry]" format e.g. "Helvetica [Cronyx]". If the family has a
    foundry it is always returned, e.g. by families(), using the
    bracketed format.

    The font() function returns a QFont given a family, style and
    point size.

    A family and style combination can be checked to see if it is
    italic() or bold(), and to retrieve its weight(). Similarly we can
    call isBitmapScalable(), isSmoothlyScalable(), isScalable() and
    isFixedPitch().

    A text version of a style is given by styleString().

    The QFontDatabase class also supports some static functions, for
    example, standardSizes(). You can retrieve the Unicode 3.0
    description of a \link QFont::Script script\endlink using
    scriptName(), and a sample of characters in a script with
    scriptSample().

    Example:
\code
#include <qapplication.h>
#include <qfontdatabase.h>
#include <else.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    QFontDatabase fdb;
    QStringList families = fdb.families();
    for ( QStringList::Iterator f = families.begin(); f != families.end(); ++f ) {
	QString family = *f;
	qDebug( family );
	QStringList styles = fdb.styles( family );
	for ( QStringList::Iterator s = styles.begin(); s != styles.end(); ++s ) {
	    QString style = *s;
	    QString dstyle = "\t" + style + " (";
	    QValueList<int> smoothies = fdb.smoothSizes( family, style );
	    for ( QValueList<int>::Iterator points = smoothies.begin();
		  points != smoothies.end(); ++points ) {
		dstyle += QString::number( *points ) + " ";
	    }
	    dstyle = dstyle.left( dstyle.length() - 1 ) + ")";
	    qDebug( dstyle );
	}
    }
    return 0;
}
\endcode
    This example gets the list of font families, then the list of
    styles for each family and the point sizes that are available for
    each family/style combination.
*/
/*!
    \obsolete
    \fn inline QStringList QFontDatabase::families( bool ) const
*/
/*!
    \obsolete
    \fn inline QStringList QFontDatabase::styles( const QString &family,
					  const QString & ) const
*/
/*!
    \obsolete
    \fn inline QValueList<int> QFontDatabase::pointSizes( const QString &family,
						  const QString &style ,
						  const QString & )
*/

/*!
    \obsolete
    \fn inline QValueList<int> QFontDatabase::smoothSizes( const QString &family,
						   const QString &style,
						   const QString & )
*/
/*!
    \obsolete
    \fn inline QFont QFontDatabase::font( const QString &familyName,
				  const QString &style,
				  int pointSize,
				  const QString &)
*/
/*!
    \obsolete
    \fn inline bool QFontDatabase::isBitmapScalable( const QString &family,
					     const QString &style,
					     const QString & ) const
*/

/*!
    \obsolete
    \fn inline bool QFontDatabase::isSmoothlyScalable( const QString &family,
					       const QString &style,
					       const QString & ) const
*/

/*!
    \obsolete
    \fn inline bool QFontDatabase::isScalable( const QString &family,
				       const QString &style,
				       const QString & ) const
*/

/*!
    \obsolete
    \fn inline bool QFontDatabase::isFixedPitch( const QString &family,
					 const QString &style,
					 const QString & ) const
*/

/*!
    \obsolete
    \fn inline bool QFontDatabase::italic( const QString &family,
				   const QString &style,
				   const QString & ) const
*/

/*!
    \obsolete
    \fn inline bool QFontDatabase::bold( const QString &family,
				 const QString &style,
				 const QString & ) const
*/

/*!
    \obsolete
    \fn inline int QFontDatabase::weight( const QString &family,
				  const QString &style,
				  const QString & ) const
*/


/*!
    Creates a font database object.
*/
QFontDatabase::QFontDatabase()
{
    createDatabase();

    d = db;
}


/*! Returns a sorted list of the names of the available font families.

    If a family exists in several foundries, the returned name for
    that font is in the form "family [foundry]". Examples: "Times
    [Adobe]", "Times [Cronyx]", "Palatino".
*/
QStringList QFontDatabase::families() const
{
    load();

    QStringList flist;
    for ( int i = 0; i < d->count; i++ ) {
	QtFontFamily *f = d->families[i];
	if ( f->count == 0 )
	    continue;
	if ( f->count == 1 ) {
	    flist.append( f->name );
	} else {
	    for ( int j = 0; j < f->count; j++ ) {
		QString str = f->name;
		QString foundry = f->foundries[j]->name;
		if ( !foundry.isEmpty() ) {
		    str += " [";
		    str += foundry;
		    str += "]";
		}
		flist.append( str );
	    }
	}
    }
    return flist;
}

/*!
    \overload

    Returns a sorted list of the available font families which support
    the Unicode script \a script.

    If a family exists in several foundries, the returned name for
    that font is in the form "family [foundry]". Examples: "Times
    [Adobe]", "Times [Cronyx]", "Palatino".
*/
QStringList QFontDatabase::families( QFont::Script script ) const
{
    load();

    QStringList flist;
    for ( int i = 0; i < d->count; i++ ) {
	QtFontFamily *f = d->families[i];
	if ( f->count == 0 )
	    continue;
	if (!(f->scripts[script] & QtFontFamily::Supported))
	    continue;
	if ( f->count == 1 ) {
	    flist.append( f->name );
	} else {
	    for ( int j = 0; j < f->count; j++ ) {
		QString str = f->name;
		QString foundry = f->foundries[j]->name;
		if ( !foundry.isEmpty() ) {
		    str += " [";
		    str += foundry;
		    str += "]";
		}
		flist.append( str );
	    }
	}
    }
    return flist;
}

/*!
    Returns a list of the styles available for the font family \a
    family. Some example styles: "Light", "Light Italic", "Bold",
    "Oblique", "Demi". The list may be empty.
*/
QStringList QFontDatabase::styles( const QString &family ) const
{
    QString familyName,  foundryName;
    parseFontName( family, foundryName, familyName );

    load( familyName );

    QStringList l;
    QtFontFamily *f = d->family( familyName );
    if ( !f )
	return l;

    QtFontFoundry allStyles( foundryName );
    for ( int j = 0; j < f->count; j++ ) {
	QtFontFoundry *foundry = f->foundries[j];
	if ( foundryName.isEmpty() || ucstricmp( foundry->name, foundryName ) == 0 ) {
	    for ( int k = 0; k < foundry->count; k++ ) {
		QtFontStyle::Key ke( foundry->styles[k]->key );
		ke.stretch = 0;
		allStyles.style( ke,  TRUE );
	    }
	}
    }

    for ( int i = 0; i < allStyles.count; i++ )
	l.append( ::styleString( allStyles.styles[i]->key.weight,
				 allStyles.styles[i]->key.italic,
				 allStyles.styles[i]->key.oblique ) );
    return l;
}

/*!
    Returns TRUE if the font that has family \a family and style \a
    style is fixed pitch; otherwise returns FALSE.
*/

bool QFontDatabase::isFixedPitch(const QString &family,
				 const QString &style) const
{
    Q_UNUSED(style);

    QString familyName,  foundryName;
    parseFontName( family, foundryName, familyName );

    load( familyName );

    QtFontFamily *f = d->family( familyName );
#if defined(Q_OS_MAC) && !defined(QWS)
    if (f) {
	if (!f->fixedPitchComputed) {
	    QFontMetrics fm(familyName);
	    f->fixedPitch = fm.width('i') == fm.width('m');
	    f->fixedPitchComputed = TRUE;
	}
    }
#endif

    return ( f && f->fixedPitch );
}

/*!
    Returns TRUE if the font that has family \a family and style \a
    style is a scalable bitmap font; otherwise returns FALSE. Scaling
    a bitmap font usually produces an unattractive hardly readable
    result, because the pixels of the font are scaled. If you need to
    scale a bitmap font it is better to scale it to one of the fixed
    sizes returned by smoothSizes().

    \sa isScalable(), isSmoothlyScalable()
*/
bool QFontDatabase::isBitmapScalable( const QString &family,
				      const QString &style) const
{
    bool bitmapScalable = FALSE;
    QString familyName,  foundryName;
    parseFontName( family, foundryName, familyName );

    load( familyName );

    QtFontStyle::Key styleKey( style );

    QtFontFamily *f = d->family( familyName );
    if ( !f ) return bitmapScalable;

    for ( int j = 0; j < f->count; j++ ) {
	QtFontFoundry *foundry = f->foundries[j];
	if ( foundryName.isEmpty() || ucstricmp( foundry->name, foundryName ) == 0 ) {
	    for ( int k = 0; k < foundry->count; k++ )
		if ((style.isEmpty() || foundry->styles[k]->key == styleKey) &&
                    foundry->styles[k]->bitmapScalable && !foundry->styles[k]->smoothScalable) {
		    bitmapScalable = TRUE;
		    goto end;
		}
	}
    }
 end:
    return bitmapScalable;
}


/*!
    Returns TRUE if the font that has family \a family and style \a
    style is smoothly scalable; otherwise returns FALSE. If this
    function returns TRUE, it's safe to scale this font to any size,
    and the result will always look attractive.

    \sa isScalable(), isBitmapScalable()
*/
bool  QFontDatabase::isSmoothlyScalable( const QString &family,
                                         const QString &style) const
{
    bool smoothScalable = FALSE;
    QString familyName,  foundryName;
    parseFontName( family, foundryName, familyName );

    load( familyName );

    QtFontStyle::Key styleKey( style );

    QtFontFamily *f = d->family( familyName );
    if ( !f ) return smoothScalable;

    for ( int j = 0; j < f->count; j++ ) {
	QtFontFoundry *foundry = f->foundries[j];
	if ( foundryName.isEmpty() || ucstricmp( foundry->name, foundryName ) == 0 ) {
	    for ( int k = 0; k < foundry->count; k++ )
		if ((style.isEmpty() || foundry->styles[k]->key == styleKey) && foundry->styles[k]->smoothScalable) {
		    smoothScalable = TRUE;
		    goto end;
		}
	}
    }
 end:
    return smoothScalable;
}

/*!
    Returns TRUE if the font that has family \a family and style \a
    style is scalable; otherwise returns FALSE.

    \sa isBitmapScalable(), isSmoothlyScalable()
*/
bool  QFontDatabase::isScalable( const QString &family,
                                 const QString &style) const
{
    if ( isSmoothlyScalable( family, style) )
        return TRUE;

    return isBitmapScalable( family, style);
}


/*!
    Returns a list of the point sizes available for the font that has
    family \a family and style \a style. The list may be empty.

    \sa smoothSizes(), standardSizes()
*/
QValueList<int> QFontDatabase::pointSizes( const QString &family,
					   const QString &style)
{
#if defined(Q_WS_MAC)
    // windows and macosx are always smoothly scalable
    Q_UNUSED( family );
    Q_UNUSED( style );
    return standardSizes();
#else
    bool smoothScalable = FALSE;
    QString familyName,  foundryName;
    parseFontName( family, foundryName, familyName );

    load( familyName );

    QtFontStyle::Key styleKey( style );

    QValueList<int> sizes;

    QtFontFamily *fam = d->family( familyName );
    if ( !fam ) return sizes;

    for ( int j = 0; j < fam->count; j++ ) {
	QtFontFoundry *foundry = fam->foundries[j];
	if ( foundryName.isEmpty() || ucstricmp( foundry->name, foundryName ) == 0 ) {
	    QtFontStyle *style = foundry->style( styleKey );
	    if ( !style ) continue;

	    if ( style->smoothScalable ) {
		smoothScalable = TRUE;
		goto end;
	    }
	    for ( int l = 0; l < style->count; l++ ) {
		const QtFontSize *size = style->pixelSizes + l;

		if (size->pixelSize != 0 && size->pixelSize != USHRT_MAX) {
#ifdef Q_WS_X11
		    const uint pointSize = qRound(qt_pointSize(size->pixelSize, 0, -1));
#else
		    const uint pointSize = size->pixelSize; // embedded uses 72dpi
#endif
		    if (! sizes.contains(pointSize))
			sizes.append(pointSize);
		}
	    }
	}
    }
 end:
    if ( smoothScalable )
	return standardSizes();

    qHeapSort( sizes );
    return sizes;
#endif
}

/*!
    Returns a QFont object that has family \a family, style \a style
    and point size \a pointSize. If no matching font could be created,
    a QFont object that uses the application's default font is
    returned.
*/
QFont QFontDatabase::font( const QString &family, const QString &style,
                           int pointSize)
{
    QString familyName,  foundryName;
    parseFontName( family, foundryName, familyName );

    load( familyName );

    QtFontFoundry allStyles( foundryName );
    QtFontFamily *f = d->family( familyName );
    if ( !f ) return QApplication::font();

    for ( int j = 0; j < f->count; j++ ) {
	QtFontFoundry *foundry = f->foundries[j];
	if ( foundryName.isEmpty() || ucstricmp( foundry->name, foundryName ) == 0 ) {
	    for ( int k = 0; k < foundry->count; k++ )
		allStyles.style( foundry->styles[k]->key,  TRUE );
	}
    }

    QtFontStyle::Key styleKey( style );
    QtFontStyle *s = bestStyle(&allStyles, styleKey);

    if ( !s ) // no styles found?
	return QApplication::font();
    return QFont( family, pointSize, s->key.weight,
		  s->key.italic ? TRUE : s->key.oblique ? TRUE : FALSE );
}


/*!
    Returns the point sizes of a font that has family \a family and
    style \a style that will look attractive. The list may be empty.
    For non-scalable fonts and bitmap scalable fonts, this function
    is equivalent to pointSizes().

  \sa pointSizes(), standardSizes()
*/
QValueList<int> QFontDatabase::smoothSizes( const QString &family,
					    const QString &style)
{
#ifdef Q_WS_WIN
    Q_UNUSED( family );
    Q_UNUSED( style );
    return QFontDatabase::standardSizes();
#else
    bool smoothScalable = FALSE;
    QString familyName,  foundryName;
    parseFontName( family, foundryName, familyName );

    load( familyName );

    QtFontStyle::Key styleKey( style );

    QValueList<int> sizes;

    QtFontFamily *fam = d->family( familyName );
    if ( !fam )
	return sizes;

    for ( int j = 0; j < fam->count; j++ ) {
	QtFontFoundry *foundry = fam->foundries[j];
	if ( foundryName.isEmpty() ||
	     ucstricmp( foundry->name, foundryName ) == 0 ) {
	    QtFontStyle *style = foundry->style( styleKey );
	    if ( !style ) continue;

	    if ( style->smoothScalable ) {
		smoothScalable = TRUE;
		goto end;
	    }
	    for ( int l = 0; l < style->count; l++ ) {
		const QtFontSize *size = style->pixelSizes + l;

		if ( size->pixelSize != 0 && size->pixelSize != USHRT_MAX ) {
#ifdef Q_WS_X11
		    const uint pointSize = qRound(qt_pointSize(size->pixelSize, 0, -1));
#else
		    const uint pointSize = size->pixelSize; // embedded uses 72dpi
#endif
		    if (! sizes.contains(pointSize))
			sizes.append( pointSize );
		}
	    }
	}
    }
 end:
    if ( smoothScalable )
	return QFontDatabase::standardSizes();

    qHeapSort( sizes );
    return sizes;
#endif
}


/*!
    Returns a list of standard font sizes.

    \sa smoothSizes(), pointSizes()
*/
QValueList<int> QFontDatabase::standardSizes()
{
    QValueList<int> ret;
    static const unsigned short standard[] =
	{ 6, 7, 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72, 0 };
    const unsigned short *sizes = standard;
    while ( *sizes ) ret << *sizes++;
    return ret;
}


/*!
    Returns TRUE if the font that has family \a family and style \a
    style is italic; otherwise returns FALSE.

    \sa weight(), bold()
*/
bool QFontDatabase::italic( const QString &family,
                            const QString &style) const
{
    QString familyName,  foundryName;
    parseFontName( family, foundryName, familyName );

    load( familyName );

    QtFontFoundry allStyles( foundryName );
    QtFontFamily *f = d->family( familyName );
    if ( !f ) return FALSE;

    for ( int j = 0; j < f->count; j++ ) {
	QtFontFoundry *foundry = f->foundries[j];
	if ( foundryName.isEmpty() || ucstricmp( foundry->name, foundryName ) == 0 ) {
	    for ( int k = 0; k < foundry->count; k++ )
		allStyles.style( foundry->styles[k]->key,  TRUE );
	}
    }

    QtFontStyle::Key styleKey( style );
    QtFontStyle *s = allStyles.style( styleKey );
    return s && s->key.italic;
}


/*!
    Returns TRUE if the font that has family \a family and style \a
    style is bold; otherwise returns FALSE.

    \sa italic(), weight()
*/
bool QFontDatabase::bold( const QString &family,
			  const QString &style) const
{
    QString familyName,  foundryName;
    parseFontName( family, foundryName, familyName );

    load( familyName );

    QtFontFoundry allStyles( foundryName );
    QtFontFamily *f = d->family( familyName );
    if ( !f ) return FALSE;

    for ( int j = 0; j < f->count; j++ ) {
	QtFontFoundry *foundry = f->foundries[j];
	if ( foundryName.isEmpty() ||
	     ucstricmp( foundry->name, foundryName ) == 0 ) {
	    for ( int k = 0; k < foundry->count; k++ )
		allStyles.style( foundry->styles[k]->key,  TRUE );
	}
    }

    QtFontStyle::Key styleKey( style );
    QtFontStyle *s = allStyles.style( styleKey );
    return s && s->key.weight >= QFont::Bold;
}


/*!
    Returns the weight of the font that has family \a family and style
    \a style. If there is no such family and style combination,
    returns -1.

    \sa italic(), bold()
*/
int QFontDatabase::weight( const QString &family,
                           const QString &style) const
{
    QString familyName,  foundryName;
    parseFontName( family, foundryName, familyName );

    load( familyName );

    QtFontFoundry allStyles( foundryName );
    QtFontFamily *f = d->family( familyName );
    if ( !f ) return -1;

    for ( int j = 0; j < f->count; j++ ) {
	QtFontFoundry *foundry = f->foundries[j];
	if ( foundryName.isEmpty() ||
	     ucstricmp( foundry->name, foundryName ) == 0 ) {
	    for ( int k = 0; k < foundry->count; k++ )
		allStyles.style( foundry->styles[k]->key,  TRUE );
	}
    }

    QtFontStyle::Key styleKey( style );
    QtFontStyle *s = allStyles.style( styleKey );
    return s ? s->key.weight : -1;
}


/*!
    Returns a string that gives a default description of the \a script
    (e.g. for displaying to the user in a dialog).  The name matches
    the name of the script as defined by the Unicode 3.0 standard.

    \sa QFont::Script
*/
QString QFontDatabase::scriptName(QFont::Script script)
{
    const char *name = 0;

    switch (script) {
    case QFont::Latin:
	name = QT_TRANSLATE_NOOP("QFont",  "Latin");
	break;
    case QFont::Greek:
	name = QT_TRANSLATE_NOOP("QFont",  "Greek" );
	break;
    case QFont::Cyrillic:
	name = QT_TRANSLATE_NOOP("QFont",  "Cyrillic" );
	break;
    case QFont::Armenian:
	name = QT_TRANSLATE_NOOP("QFont",  "Armenian" );
	break;
    case QFont::Georgian:
	name = QT_TRANSLATE_NOOP("QFont",  "Georgian" );
	break;
    case QFont::Runic:
	name = QT_TRANSLATE_NOOP("QFont",  "Runic" );
	break;
    case QFont::Ogham:
	name = QT_TRANSLATE_NOOP("QFont",  "Ogham" );
	break;
    case QFont::SpacingModifiers:
	name = QT_TRANSLATE_NOOP("QFont",  "SpacingModifiers" );
	break;
    case QFont::CombiningMarks:
	name = QT_TRANSLATE_NOOP("QFont",  "CombiningMarks" );
	break;
    case QFont::Hebrew:
	name = QT_TRANSLATE_NOOP("QFont",  "Hebrew" );
	break;
    case QFont::Arabic:
	name = QT_TRANSLATE_NOOP("QFont",  "Arabic" );
	break;
    case QFont::Syriac:
	name = QT_TRANSLATE_NOOP("QFont",  "Syriac" );
	break;
    case QFont::Thaana:
	name = QT_TRANSLATE_NOOP("QFont",  "Thaana" );
	break;
    case QFont::Devanagari:
	name = QT_TRANSLATE_NOOP("QFont",  "Devanagari" );
	break;
    case QFont::Bengali:
	name = QT_TRANSLATE_NOOP("QFont",  "Bengali" );
	break;
    case QFont::Gurmukhi:
	name = QT_TRANSLATE_NOOP("QFont",  "Gurmukhi" );
	break;
    case QFont::Gujarati:
	name = QT_TRANSLATE_NOOP("QFont",  "Gujarati" );
	break;
    case QFont::Oriya:
	name = QT_TRANSLATE_NOOP("QFont",  "Oriya" );
	break;
    case QFont::Tamil:
	name = QT_TRANSLATE_NOOP("QFont",  "Tamil" );
	break;
    case QFont::Telugu:
	name = QT_TRANSLATE_NOOP("QFont",  "Telugu" );
	break;
    case QFont::Kannada:
	name = QT_TRANSLATE_NOOP("QFont",  "Kannada" );
	break;
    case QFont::Malayalam:
	name = QT_TRANSLATE_NOOP("QFont",  "Malayalam" );
	break;
    case QFont::Sinhala:
	name = QT_TRANSLATE_NOOP("QFont",  "Sinhala" );
	break;
    case QFont::Thai:
	name = QT_TRANSLATE_NOOP("QFont",  "Thai" );
	break;
    case QFont::Lao:
	name = QT_TRANSLATE_NOOP("QFont",  "Lao" );
	break;
    case QFont::Tibetan:
	name = QT_TRANSLATE_NOOP("QFont",  "Tibetan" );
	break;
    case QFont::Myanmar:
	name = QT_TRANSLATE_NOOP("QFont",  "Myanmar" );
	break;
    case QFont::Khmer:
	name = QT_TRANSLATE_NOOP("QFont",  "Khmer" );
	break;
    case QFont::Han:
	name = QT_TRANSLATE_NOOP("QFont",  "Han" );
	break;
    case QFont::Hiragana:
	name = QT_TRANSLATE_NOOP("QFont",  "Hiragana" );
	break;
    case QFont::Katakana:
	name = QT_TRANSLATE_NOOP("QFont",  "Katakana" );
	break;
    case QFont::Hangul:
	name = QT_TRANSLATE_NOOP("QFont",  "Hangul" );
	break;
    case QFont::Bopomofo:
	name = QT_TRANSLATE_NOOP("QFont",  "Bopomofo" );
	break;
    case QFont::Yi:
	name = QT_TRANSLATE_NOOP("QFont",  "Yi" );
	break;
    case QFont::Ethiopic:
	name = QT_TRANSLATE_NOOP("QFont",  "Ethiopic" );
	break;
    case QFont::Cherokee:
	name = QT_TRANSLATE_NOOP("QFont",  "Cherokee" );
	break;
    case QFont::CanadianAboriginal:
	name = QT_TRANSLATE_NOOP("QFont",  "Canadian Aboriginal" );
	break;
    case QFont::Mongolian:
	name = QT_TRANSLATE_NOOP("QFont",  "Mongolian" );
	break;

    case QFont::CurrencySymbols:
	name = QT_TRANSLATE_NOOP("QFont",  "Currency Symbols" );
	break;

    case QFont::LetterlikeSymbols:
	name = QT_TRANSLATE_NOOP("QFont",  "Letterlike Symbols" );
	break;

    case QFont::NumberForms:
	name = QT_TRANSLATE_NOOP("QFont",  "Number Forms" );
	break;

    case QFont::MathematicalOperators:
	name = QT_TRANSLATE_NOOP("QFont",  "Mathematical Operators" );
	break;

    case QFont::TechnicalSymbols:
	name = QT_TRANSLATE_NOOP("QFont",  "Technical Symbols" );
	break;

    case QFont::GeometricSymbols:
	name = QT_TRANSLATE_NOOP("QFont",  "Geometric Symbols" );
	break;

    case QFont::MiscellaneousSymbols:
	name = QT_TRANSLATE_NOOP("QFont",  "Miscellaneous Symbols" );
	break;

    case QFont::EnclosedAndSquare:
	name = QT_TRANSLATE_NOOP("QFont",  "Enclosed and Square" );
	break;

    case QFont::Braille:
	name = QT_TRANSLATE_NOOP("QFont",  "Braille" );
	break;

    case QFont::Unicode:
	name = QT_TRANSLATE_NOOP("QFont",  "Unicode" );
	break;

    case QFont::Tagalog:
	name = QT_TRANSLATE_NOOP( "QFont", "Tagalog" );
	break;

    case QFont::Hanunoo:
	name = QT_TRANSLATE_NOOP( "QFont", "Hanunoo" );
	break;

    case QFont::Buhid:
	name = QT_TRANSLATE_NOOP( "QFont", "Buhid" );
    	break;

    case QFont::Tagbanwa:
	name = QT_TRANSLATE_NOOP( "QFont", "Tagbanwa" );
	break;

    case QFont::KatakanaHalfWidth:
	name = QT_TRANSLATE_NOOP( "QFont", "Katakana Half-Width Forms" );
	break;

    case QFont::Han_Japanese:
	name = QT_TRANSLATE_NOOP( "QFont", "Han (Japanese)" );
	break;

    case QFont::Han_SimplifiedChinese:
	name = QT_TRANSLATE_NOOP( "QFont", "Han (Simplified Chinese)" );
	break;

    case QFont::Han_TraditionalChinese:
	name = QT_TRANSLATE_NOOP( "QFont", "Han (Traditional Chinese)" );
	break;

    case QFont::Han_Korean:
	name = QT_TRANSLATE_NOOP( "QFont", "Han (Korean)" );
	break;

    default:
	name = QT_TRANSLATE_NOOP( "QFont", "Unknown Script" );
	break;
    }

    return qApp ? qApp->translate("QFont", name) : QString::fromLatin1(name);
}


/*!
    Returns a string with sample characters from \a script.

    \sa QFont::Script
*/
QString QFontDatabase::scriptSample(QFont::Script script)
{
    QString sample = "AaBb";

    switch (script) {
    case QFont::Latin:
	// This is cheating... we only show latin-1 characters so that we don't
	// end up loading lots of fonts - at least on X11...
	sample += QChar(0x00C3);
	sample += QChar(0x00E1);
	sample += "Zz";
	break;
    case QFont::Greek:
	sample += QChar(0x0393);
	sample += QChar(0x03B1);
	sample += QChar(0x03A9);
	sample += QChar(0x03C9);
	break;
    case QFont::Cyrillic:
	sample += QChar(0x0414);
	sample += QChar(0x0434);
	sample += QChar(0x0436);
	sample += QChar(0x0402);
	break;
    case QFont::Armenian:
	sample += QChar(0x053f);
	sample += QChar(0x054f);
	sample += QChar(0x056f);
	sample += QChar(0x057f);
	break;
    case QFont::Georgian:
	sample += QChar(0x10a0);
	sample += QChar(0x10b0);
	sample += QChar(0x10c0);
	sample += QChar(0x10d0);
	break;
    case QFont::Runic:
	sample += QChar(0x16a0);
	sample += QChar(0x16b0);
	sample += QChar(0x16c0);
	sample += QChar(0x16d0);
	break;
    case QFont::Ogham:
	sample += QChar(0x1681);
	sample += QChar(0x1687);
	sample += QChar(0x1693);
	sample += QChar(0x168d);
	break;



    case QFont::Hebrew:
        sample += QChar(0x05D0);
        sample += QChar(0x05D1);
        sample += QChar(0x05D2);
        sample += QChar(0x05D3);
	break;
    case QFont::Arabic:
	sample += QChar(0x0628);
	sample += QChar(0x0629);
        sample += QChar(0x062A);
        sample += QChar(0x063A);
	break;
    case QFont::Syriac:
	sample += QChar(0x0715);
	sample += QChar(0x0725);
	sample += QChar(0x0716);
	sample += QChar(0x0726);
	break;
    case QFont::Thaana:
	sample += QChar(0x0784);
	sample += QChar(0x0794);
	sample += QChar(0x078c);
	sample += QChar(0x078d);
	break;



    case QFont::Devanagari:
	sample += QChar(0x0905);
	sample += QChar(0x0915);
	sample += QChar(0x0925);
	sample += QChar(0x0935);
	break;
    case QFont::Bengali:
	sample += QChar(0x0986);
	sample += QChar(0x0996);
	sample += QChar(0x09a6);
	sample += QChar(0x09b6);
	break;
    case QFont::Gurmukhi:
	sample += QChar(0x0a05);
	sample += QChar(0x0a15);
	sample += QChar(0x0a25);
	sample += QChar(0x0a35);
	break;
    case QFont::Gujarati:
	sample += QChar(0x0a85);
	sample += QChar(0x0a95);
	sample += QChar(0x0aa5);
	sample += QChar(0x0ab5);
	break;
    case QFont::Oriya:
	sample += QChar(0x0b06);
	sample += QChar(0x0b16);
	sample += QChar(0x0b2b);
	sample += QChar(0x0b36);
	break;
    case QFont::Tamil:
	sample += QChar(0x0b89);
	sample += QChar(0x0b99);
	sample += QChar(0x0ba9);
	sample += QChar(0x0bb9);
	break;
    case QFont::Telugu:
	sample += QChar(0x0c05);
	sample += QChar(0x0c15);
	sample += QChar(0x0c25);
	sample += QChar(0x0c35);
	break;
    case QFont::Kannada:
	sample += QChar(0x0c85);
	sample += QChar(0x0c95);
	sample += QChar(0x0ca5);
	sample += QChar(0x0cb5);
	break;
    case QFont::Malayalam:
	sample += QChar(0x0d05);
	sample += QChar(0x0d15);
	sample += QChar(0x0d25);
	sample += QChar(0x0d35);
	break;
    case QFont::Sinhala:
	sample += QChar(0x0d90);
	sample += QChar(0x0da0);
	sample += QChar(0x0db0);
	sample += QChar(0x0dc0);
	break;
    case QFont::Thai:
	sample += QChar(0x0e02);
	sample += QChar(0x0e12);
	sample += QChar(0x0e22);
	sample += QChar(0x0e32);
	break;
    case QFont::Lao:
	sample += QChar(0x0e8d);
	sample += QChar(0x0e9d);
	sample += QChar(0x0ead);
	sample += QChar(0x0ebd);
	break;
    case QFont::Tibetan:
	sample += QChar(0x0f00);
	sample += QChar(0x0f01);
	sample += QChar(0x0f02);
	sample += QChar(0x0f03);
	break;
    case QFont::Myanmar:
	sample += QChar(0x1000);
	sample += QChar(0x1001);
	sample += QChar(0x1002);
	sample += QChar(0x1003);
	break;
    case QFont::Khmer:
	sample += QChar(0x1780);
	sample += QChar(0x1790);
	sample += QChar(0x17b0);
	sample += QChar(0x17c0);
	break;



    case QFont::Han:
	sample += QChar(0x6f84);
	sample += QChar(0x820a);
	sample += QChar(0x61a9);
	sample += QChar(0x9781);
	break;
    case QFont::Hiragana:
	sample += QChar(0x3050);
	sample += QChar(0x3060);
	sample += QChar(0x3070);
	sample += QChar(0x3080);
	break;
    case QFont::Katakana:
	sample += QChar(0x30b0);
	sample += QChar(0x30c0);
	sample += QChar(0x30d0);
	sample += QChar(0x30e0);
	break;
    case QFont::Hangul:
	sample += QChar(0xac00);
	sample += QChar(0xac11);
	sample += QChar(0xac1a);
	sample += QChar(0xac2f);
	break;
    case QFont::Bopomofo:
	sample += QChar(0x3105);
	sample += QChar(0x3115);
	sample += QChar(0x3125);
	sample += QChar(0x3129);
	break;
    case QFont::Yi:
	sample += QChar(0xa1a8);
	sample += QChar(0xa1a6);
	sample += QChar(0xa200);
	sample += QChar(0xa280);
	break;



    case QFont::Ethiopic:
	sample += QChar(0x1200);
	sample += QChar(0x1240);
	sample += QChar(0x1280);
	sample += QChar(0x12c0);
	break;
    case QFont::Cherokee:
	sample += QChar(0x13a0);
	sample += QChar(0x13b0);
	sample += QChar(0x13c0);
	sample += QChar(0x13d0);
	break;
    case QFont::CanadianAboriginal:
	sample += QChar(0x1410);
	sample += QChar(0x1500);
	sample += QChar(0x15f0);
	sample += QChar(0x1650);
	break;
    case QFont::Mongolian:
	sample += QChar(0x1820);
	sample += QChar(0x1840);
	sample += QChar(0x1860);
	sample += QChar(0x1880);
	break;


    case QFont::CurrencySymbols:
    case QFont::LetterlikeSymbols:
    case QFont::NumberForms:
    case QFont::MathematicalOperators:
    case QFont::TechnicalSymbols:
    case QFont::GeometricSymbols:
    case QFont::MiscellaneousSymbols:
    case QFont::EnclosedAndSquare:
    case QFont::Braille:
	break;


    case QFont::Unicode:
	sample += QChar(0x0174);
	sample += QChar(0x0628);
	sample += QChar(0x0e02);
	sample += QChar(0x263A);
	sample += QChar(0x3129);
	sample += QChar(0x61a9);
	sample += QChar(0xac2f);
	break;



    default:
	sample += QChar(0xfffd);
	sample += QChar(0xfffd);
	sample += QChar(0xfffd);
	sample += QChar(0xfffd);
	break;
    }

    return sample;
}




/*!
  \internal

  This makes sense of the font family name:

  1) if the family name contains a '-' (ie. "Adobe-Courier"), then we
  split at the '-', and use the string as the foundry, and the string to
  the right as the family

  2) if the family name contains a '[' and a ']', then we take the text
  between the square brackets as the foundry, and the text before the
  square brackets as the family (ie. "Arial [Monotype]")
*/
void QFontDatabase::parseFontName(const QString &name, QString &foundry, QString &family)
{
    if ( name.contains('-') ) {
	int i = name.find('-');
	foundry = name.left( i );
	family = name.right( name.length() - i - 1 );
    } else if ( name.contains('[') && name.contains(']')) {
	int i = name.find('[');
	int li = name.findRev(']');

	if (i < li) {
	    foundry = name.mid(i + 1, li - i - 1);
	    if (name[i - 1] == ' ')
		i--;
	    family = name.left(i);
	}
    } else {
	foundry = QString::null;
	family = name;
    }
}

#endif // QT_NO_FONTDATABASE
