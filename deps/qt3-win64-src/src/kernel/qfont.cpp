/****************************************************************************
** $Id: qfont.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes
**
** Created : 941207
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

#define QT_FATAL_ASSERT

#include "qfont.h"
#include "qfontdatabase.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"
#include "qpainter.h"
#include "qdict.h"
#include "qcache.h"
#include "qdatastream.h"
#include "qapplication.h"
#include "qcleanuphandler.h"
#include "qstringlist.h"
#ifdef Q_WS_MAC
#include "qpaintdevicemetrics.h"
#endif

#include <private/qunicodetables_p.h>
#include "qfontdata_p.h"
#include "qfontengine_p.h"
#include "qpainter_p.h"
#include "qtextengine_p.h"

// #define QFONTCACHE_DEBUG
#ifdef QFONTCACHE_DEBUG
#  define FC_DEBUG qDebug
#else
#  define FC_DEBUG if (FALSE) qDebug
#endif




bool QFontDef::operator==( const QFontDef &other ) const
{
    /*
      QFontDef comparison is more complicated than just simple
      per-member comparisons.

      When comparing point/pixel sizes, either point or pixelsize
      could be -1.  in This case we have to compare the non negative
      size value.

      This test will fail if the point-sizes differ by 1/2 point or
      more or they do not round to the same value.  We have to do this
      since our API still uses 'int' point-sizes in the API, but store
      deci-point-sizes internally.

      To compare the family members, we need to parse the font names
      and compare the family/foundry strings separately.  This allows
      us to compare e.g. "Helvetica" and "Helvetica [Adobe]" with
      positive results.
    */
    if (pixelSize != -1 && other.pixelSize != -1) {
	if (pixelSize != other.pixelSize)
	    return FALSE;
    } else if (pointSize != -1 && other.pointSize != -1) {
	if (pointSize != other.pointSize
	    && (QABS(pointSize - other.pointSize) >= 5
		|| qRound(pointSize/10.) != qRound(other.pointSize/10.)))
	    return FALSE;
    } else {
	return FALSE;
    }

    if (!ignorePitch && !other.ignorePitch && fixedPitch != other.fixedPitch)
	return FALSE;

    if (stretch != 0 && other.stretch != 0 && stretch != other.stretch)
	return FALSE;

    QString this_family, this_foundry, other_family, other_foundry;
    QFontDatabase::parseFontName(family, this_foundry, this_family);
    QFontDatabase::parseFontName(other.family, other_foundry, other_family);

    return ( styleHint     == other.styleHint
	    && styleStrategy == other.styleStrategy
	    && weight        == other.weight
	    && italic        == other.italic
	    && this_family   == other_family
	    && (this_foundry.isEmpty()
		|| other_foundry.isEmpty()
		|| this_foundry == other_foundry)
#ifdef Q_WS_X11
	    && addStyle == other.addStyle
#endif // Q_WS_X11
	);
}




QFontPrivate::QFontPrivate()
    : engineData( 0 ), paintdevice( 0 ),
      rawMode( FALSE ), underline( FALSE ), overline( FALSE ), strikeOut( FALSE ),
      mask( 0 )
{
#ifdef Q_WS_X11
    screen = QPaintDevice::x11AppScreen();
#else
    screen = 0;
#endif // Q_WS_X11
}

QFontPrivate::QFontPrivate( const QFontPrivate &other )
    : QShared(), request( other.request ), engineData( 0 ),
      paintdevice( other.paintdevice ), screen( other.screen ),
      rawMode( other.rawMode ), underline( other.underline ), overline( other.overline ),
      strikeOut( other.strikeOut ), mask( other.mask )
{
}

QFontPrivate::~QFontPrivate()
{
    if ( engineData )
	engineData->deref();
    engineData = 0;
}

void QFontPrivate::resolve( const QFontPrivate *other )
{
#ifdef QT_CHECK_STATE
    Q_ASSERT( other != 0 );
#endif

    if ( ( mask & Complete ) == Complete ) return;

    // assign the unset-bits with the set-bits of the other font def
    if ( ! ( mask & Family ) )
	request.family = other->request.family;

    if ( ! ( mask & Size ) ) {
	request.pointSize = other->request.pointSize;
	request.pixelSize = other->request.pixelSize;
    }

    if ( ! ( mask & StyleHint ) )
	request.styleHint = other->request.styleHint;

    if ( ! ( mask & StyleStrategy ) )
	request.styleStrategy = other->request.styleStrategy;

    if ( ! ( mask & Weight ) )
	request.weight = other->request.weight;

    if ( ! ( mask & Italic ) )
	request.italic = other->request.italic;

    if ( ! ( mask & FixedPitch ) )
	request.fixedPitch = other->request.fixedPitch;

    if ( ! ( mask & Stretch ) )
	request.stretch = other->request.stretch;

    if ( ! ( mask & Underline ) )
	underline = other->underline;

    if ( ! ( mask & Overline ) )
	overline = other->overline;

    if ( ! ( mask & StrikeOut ) )
	strikeOut = other->strikeOut;
}




QFontEngineData::QFontEngineData()
    : lineWidth( 1 )
{
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    memset( engines, 0, QFont::LastPrivateScript * sizeof( QFontEngine * ) );
#else
    engine = 0;
#endif // Q_WS_X11 || Q_WS_WIN
#ifndef Q_WS_MAC
    memset( widthCache, 0, widthCacheSize*sizeof( uchar ) );
#endif
}

QFontEngineData::~QFontEngineData()
{
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    for ( int i = 0; i < QFont::LastPrivateScript; i++ ) {
	if ( engines[i] )
	    engines[i]->deref();
	engines[i] = 0;
    }
#else
    if ( engine )
	engine->deref();
    engine = 0;
#endif // Q_WS_X11 || Q_WS_WIN
}




/*!
    \class QFont qfont.h
    \brief The QFont class specifies a font used for drawing text.

    \ingroup graphics
    \ingroup appearance
    \ingroup shared
    \mainclass

    When you create a QFont object you specify various attributes that
    you want the font to have. Qt will use the font with the specified
    attributes, or if no matching font exists, Qt will use the closest
    matching installed font. The attributes of the font that is
    actually used are retrievable from a QFontInfo object. If the
    window system provides an exact match exactMatch() returns TRUE.
    Use QFontMetrics to get measurements, e.g. the pixel length of a
    string using QFontMetrics::width().

    Use QApplication::setFont() to set the application's default font.

    If a choosen X11 font does not include all the characters that
    need to be displayed, QFont will try to find the characters in the
    nearest equivalent fonts. When a QPainter draws a character from a
    font the QFont will report whether or not it has the character; if
    it does not, QPainter will draw an unfilled square.

    Create QFonts like this:
    \code
    QFont serifFont( "Times", 10, Bold );
    QFont sansFont( "Helvetica [Cronyx]", 12 );
    \endcode

    The attributes set in the constructor can also be set later, e.g.
    setFamily(), setPointSize(), setPointSizeFloat(), setWeight() and
    setItalic(). The remaining attributes must be set after
    contstruction, e.g. setBold(), setUnderline(), setOverline(),
    setStrikeOut() and setFixedPitch(). QFontInfo objects should be
    created \e after the font's attributes have been set. A QFontInfo
    object will not change, even if you change the font's
    attributes. The corresponding "get" functions, e.g. family(),
    pointSize(), etc., return the values that were set, even though
    the values used may differ. The actual values are available from a
    QFontInfo object.

    If the requested font family is unavailable you can influence the
    \link #fontmatching font matching algorithm\endlink by choosing a
    particular \l{QFont::StyleHint} and \l{QFont::StyleStrategy} with
    setStyleHint(). The default family (corresponding to the current
    style hint) is returned by defaultFamily().

    The font-matching algorithm has a lastResortFamily() and
    lastResortFont() in cases where a suitable match cannot be found.
    You can provide substitutions for font family names using
    insertSubstitution() and insertSubstitutions(). Substitutions can
    be removed with removeSubstitution(). Use substitute() to retrieve
    a family's first substitute, or the family name itself if it has
    no substitutes. Use substitutes() to retrieve a list of a family's
    substitutes (which may be empty).

    Every QFont has a key() which you can use, for example, as the key
    in a cache or dictionary. If you want to store a user's font
    preferences you could use QSettings, writing the font information
    with toString() and reading it back with fromString(). The
    operator<<() and operator>>() functions are also available, but
    they work on a data stream.

    It is possible to set the height of characters shown on the screen
    to a specified number of pixels with setPixelSize(); however using
    setPointSize() has a similar effect and provides device
    independence.

    Under the X Window System you can set a font using its system
    specific name with setRawName().

    Loading fonts can be expensive, especially on X11. QFont contains
    extensive optimizations to make the copying of QFont objects fast,
    and to cache the results of the slow window system functions it
    depends upon.

    \target fontmatching
    The font matching algorithm works as follows:
    \list 1
    \i The specified font family is searched for.
    \i If not found, the styleHint() is used to select a replacement
       family.
    \i Each replacement font family is searched for.
    \i If none of these are found or there was no styleHint(), "helvetica"
       will be searched for.
    \i If "helvetica" isn't found Qt will try the lastResortFamily().
    \i If the lastResortFamily() isn't found Qt will try the
       lastResortFont() which will always return a name of some kind.
    \endlist

    Once a font is found, the remaining attributes are matched in order of
    priority:
    \list 1
    \i fixedPitch()
    \i pointSize() (see below)
    \i weight()
    \i italic()
    \endlist

    If you have a font which matches on family, even if none of the
    other attributes match, this font will be chosen in preference to
    a font which doesn't match on family but which does match on the
    other attributes. This is because font family is the dominant
    search criteria.

    The point size is defined to match if it is within 20% of the
    requested point size. When several fonts match and are only
    distinguished by point size, the font with the closest point size
    to the one requested will be chosen.

    The actual family, font size, weight and other font attributes
    used for drawing text will depend on what's available for the
    chosen family under the window system. A QFontInfo object can be
    used to determine the actual values used for drawing the text.

    Examples:

    \code
    QFont f("Helvetica");
    \endcode
    If you had both an Adobe and a Cronyx Helvetica, you might get
    either.

    \code
    QFont f1( "Helvetica [Cronyx]" );  // Qt 3.x
    QFont f2( "Cronyx-Helvetica" );    // Qt 2.x compatibility
    \endcode
    You can specify the foundry you want in the family name. Both fonts,
    f1 and f2, in the above example will be set to  "Helvetica
    [Cronyx]".

    To determine the attributes of the font actually used in the window
    system, use a QFontInfo object, e.g.
    \code
    QFontInfo info( f1 );
    QString family = info.family();
    \endcode

    To find out font metrics use a QFontMetrics object, e.g.
    \code
    QFontMetrics fm( f1 );
    int pixelWidth = fm.width( "How many pixels wide is this text?" );
    int pixelHeight = fm.height();
    \endcode

    For more general information on fonts, see the
    \link http://www.nwalsh.com/comp.fonts/FAQ/ comp.fonts FAQ.\endlink
    Information on encodings can be found from
    \link http://czyborra.com/ Roman Czyborra's\endlink page.

    \sa QFontMetrics QFontInfo QFontDatabase QApplication::setFont()
    QWidget::setFont() QPainter::setFont() QFont::StyleHint
    QFont::Weight
*/

/*!
    \enum QFont::Script

    This enum represents \link unicode.html Unicode \endlink allocated
    scripts. For exhaustive coverage see \link
    http://www.amazon.com/exec/obidos/ASIN/0201616335/trolltech/t The
    Unicode Standard Version 3.0 \endlink. The following scripts are
    supported:

    Modern European alphabetic scripts (left to right):

    \value Latin consists of most alphabets based on the original Latin alphabet.
    \value Greek covers ancient and modern Greek and Coptic.
    \value Cyrillic covers the Slavic and non-Slavic languages using
	   cyrillic alphabets.
    \value Armenian contains the Armenian alphabet used with the
	   Armenian language.
    \value Georgian covers at least the language Georgian.
    \value Runic covers the known constituents of the Runic alphabets used
	   by the early and medieval societies in the Germanic,
	   Scandinavian, and Anglo-Saxon areas.
    \value Ogham is an alphabetical script used to write a very early
	   form of Irish.
    \value SpacingModifiers are small signs indicating modifications
	   to the preceeding letter.
    \value CombiningMarks consist of diacritical marks not specific to
	   a particular alphabet, diacritical marks used in
	   combination with mathematical and technical symbols, and
	   glyph encodings applied to multiple letterforms.

    Middle Eastern scripts (right to left):

    \value Hebrew is used for writing Hebrew, Yiddish, and some other languages.
    \value Arabic covers the Arabic language as well as Persian, Urdu,
	   Kurdish and some others.
    \value Syriac is used to write the active liturgical languages and
	   dialects of several Middle Eastern and Southeast Indian
	   communities.
    \value Thaana is used to write the Maledivian Dhivehi language.

    South and Southeast Asian scripts (left to right with few historical exceptions):

    \value Devanagari covers classical Sanskrit and modern Hindi as
	   well as several other languages.
    \value Bengali is a relative to Devanagari employed to write the
	   Bengali language used in West Bengal/India and Bangladesh
	   as well as several minority languages.
    \value Gurmukhi is another Devanagari relative used to write Punjabi.
    \value Gujarati is closely related to Devanagari and used to write
	   the Gujarati language of the Gujarat state in India.
    \value Oriya is used to write the Oriya language of Orissa state/India.
    \value Tamil is used to write the Tamil language of Tamil Nadu state/India,
	   Sri Lanka, Singapore and parts of Malaysia as well as some
	   minority languages.
    \value Telugu is used to write the Telugu language of Andhra
	   Pradesh state/India and some minority languages.
    \value Kannada is another South Indian script used to write the
	   Kannada language of Karnataka state/India and some minority
	   languages.
    \value Malayalam is used to write the Malayalam language of Kerala
	   state/India.
    \value Sinhala is used for Sri Lanka's majority language Sinhala
	   and is also employed to write Pali, Sanskrit, and Tamil.
    \value Thai is used to write Thai and other Southeast Asian languages.
    \value Lao is a language and script quite similar to Thai.
    \value Tibetan is the script used to write Tibetan in several
	   countries like Tibet, the bordering Indian regions and
	   Nepal. It is also used in the Buddist philosophy and
	   liturgy of the Mongolian cultural area.
    \value Myanmar is mainly used to write the Burmese language of
	   Myanmar (former Burma).
    \value Khmer is the official language of Kampuchea.

    East Asian scripts (traditionally top-down, right to left, modern
    often horizontal left to right):

    \value Han consists of the CJK (Chinese, Japanese, Korean)
	   idiographic characters.
    \value Hiragana is a cursive syllabary used to indicate phonetics
	   and pronounciation of Japanese words.
    \value Katakana is a non-cursive syllabic script used to write
	   Japanese words with visual emphasis and non-Japanese words
	   in a phonetical manner.
    \value Hangul is a Korean script consisting of alphabetic components.
    \value Bopomofo is a phonetic alphabet for Chinese (mainly Mandarin).
    \value Yi (also called Cuan or Wei) is a syllabary used to write
	   the Yi language of Southwestern China, Myanmar, Laos, and Vietnam.

    Additional scripts that do not fit well into the script categories above:

    \value Ethiopic is a syllabary used by several Central East African languages.
    \value Cherokee is a left-to-right syllabic script used to write
	   the Cherokee language.
    \value CanadianAboriginal consists of the syllabics used by some
	   Canadian aboriginal societies.
    \value Mongolian is the traditional (and recently reintroduced)
	   script used to write Mongolian.

    Symbols:

    \value CurrencySymbols contains currency symbols not encoded in other scripts.
    \value LetterlikeSymbols consists of symbols derived  from
	   ordinary letters of an alphabetical script.
    \value NumberForms are provided for compatibility with other
	   existing character sets.
    \value MathematicalOperators consists of encodings for operators,
	   relations and other symbols like arrows used in a mathematical context.
    \value TechnicalSymbols contains representations for control
	   codes, the space symbol, APL symbols and other symbols
	   mainly used in the context of electronic data processing.
    \value GeometricSymbols covers block elements and geometric shapes.
    \value MiscellaneousSymbols consists of a heterogeneous collection
	   of symbols that do not fit any other Unicode character
	   block, e.g. Dingbats.
    \value EnclosedAndSquare is provided for compatibility with some
	   East Asian standards.
    \value Braille is an international writing system used by blind
	   people. This script encodes the 256 eight-dot patterns with
	   the 64 six-dot patterns as a subset.

    \value Tagalog
    \value Hanunoo
    \value Buhid
    \value Tagbanwa

    \value KatakanaHalfWidth

    \value Limbu (Unicode 4.0)
    \value TaiLe (Unicode 4.0)

    \value Unicode includes all the above scripts.
*/

/*! \internal

    Constructs a font for use on the paint device \a pd using the
    specified font \a data.
*/
QFont::QFont( QFontPrivate *data, QPaintDevice *pd )
{
    d = new QFontPrivate( *data );
    Q_CHECK_PTR( d );
    d->paintdevice = pd;

    // now a single reference
    d->count = 1;
}

/*! \internal
    Detaches the font object from common font data.
*/
void QFont::detach()
{
    if (d->count == 1) {
	if ( d->engineData )
	    d->engineData->deref();
	d->engineData = 0;

	return;
    }

    QFontPrivate *old_d = d;
    d = new QFontPrivate( *old_d );

    /*
      if this font is a copy of the application default font, set the
      fontdef mask to zero to indicate that *nothing* has been
      explicitly set by the programmer.
    */
    const QFont appfont = QApplication::font();
    if ( old_d == appfont.d )
	d->mask = 0;

    if ( old_d->deref() )
	delete old_d;
}

/*!
    Constructs a font object that uses the application's default font.

    \sa QApplication::setFont(), QApplication::font()
*/
QFont::QFont()
{
    const QFont appfont = QApplication::font();
    d = appfont.d;
    d->ref();
}

/*!
    Constructs a font object with the specified \a family, \a
    pointSize, \a weight and \a italic settings.

    If \a pointSize is <= 0 it is set to 1.

    The \a family name may optionally also include a foundry name,
    e.g. "Helvetica [Cronyx]". (The Qt 2.x syntax, i.e.
    "Cronyx-Helvetica", is also supported.) If the \a family is
    available from more than one foundry and the foundry isn't
    specified, an arbitrary foundry is chosen. If the family isn't
    available a family will be set using the \link #fontmatching font
    matching\endlink algorithm.

    \sa Weight, setFamily(), setPointSize(), setWeight(), setItalic(),
    setStyleHint() QApplication::font()
*/
QFont::QFont( const QString &family, int pointSize, int weight, bool italic )
{

    d = new QFontPrivate;
    Q_CHECK_PTR( d );

    d->mask = QFontPrivate::Family;

    if (pointSize <= 0) {
	pointSize = 12;
    } else {
	d->mask |= QFontPrivate::Size;
    }

    if (weight < 0) {
	weight = Normal;
    } else {
	d->mask |= QFontPrivate::Weight | QFontPrivate::Italic;
    }

    d->request.family = family;
    d->request.pointSize = pointSize * 10;
    d->request.pixelSize = -1;
    d->request.weight = weight;
    d->request.italic = italic;
}

/*!
    Constructs a font that is a copy of \a font.
*/
QFont::QFont( const QFont &font )
{
    d = font.d;
    d->ref();
}

/*!
    Destroys the font object and frees all allocated resources.
*/
QFont::~QFont()
{
    if ( d->deref() )
	delete d;
    d = 0;
}

/*!
    Assigns \a font to this font and returns a reference to it.
*/
QFont &QFont::operator=( const QFont &font )
{
    if ( font.d != d ) {
	if ( d->deref() )
	    delete d;
	d = font.d;
	d->ref();
    }

    return *this;
}

/*!
    Returns the requested font family name, i.e. the name set in the
    constructor or the last setFont() call.

    \sa setFamily() substitutes() substitute()
*/
QString QFont::family() const
{
    return d->request.family;
}

/*!
    Sets the family name of the font. The name is case insensitive and
    may include a foundry name.

    The \a family name may optionally also include a foundry name,
    e.g. "Helvetica [Cronyx]". (The Qt 2.x syntax, i.e.
    "Cronyx-Helvetica", is also supported.) If the \a family is
    available from more than one foundry and the foundry isn't
    specified, an arbitrary foundry is chosen. If the family isn't
    available a family will be set using the \link #fontmatching font
    matching\endlink algorithm.

    \sa family(), setStyleHint(), QFontInfo
*/
void QFont::setFamily( const QString &family )
{
    detach();

    d->request.family = family;
#if defined(Q_WS_X11)
    d->request.addStyle = QString::null;
#endif // Q_WS_X11

    d->mask |= QFontPrivate::Family;
}

/*!
    Returns the point size in 1/10ths of a point.

    The returned value will be -1 if the font size has been specified
    in pixels.

    \sa pointSize() pointSizeFloat()
  */
int QFont::deciPointSize() const
{
    return d->request.pointSize;
}

/*!
    Returns the point size of the font. Returns -1 if the font size
    was specified in pixels.

    \sa setPointSize() deciPointSize() pointSizeFloat()
*/
int QFont::pointSize() const
{
    return d->request.pointSize == -1 ? -1 : (d->request.pointSize + 5) / 10;
}

/*!
    Sets the point size to \a pointSize. The point size must be
    greater than zero.

    \sa pointSize() setPointSizeFloat()
*/
void QFont::setPointSize( int pointSize )
{
    if ( pointSize <= 0 ) {

#if defined(QT_CHECK_RANGE)
	qWarning( "QFont::setPointSize: Point size <= 0 (%d)", pointSize );
#endif

	return;
    }

    detach();

    d->request.pointSize = pointSize * 10;
    d->request.pixelSize = -1;

    d->mask |= QFontPrivate::Size;
}

/*!
    Sets the point size to \a pointSize. The point size must be
    greater than zero. The requested precision may not be achieved on
    all platforms.

    \sa pointSizeFloat() setPointSize() setPixelSize()
*/
void QFont::setPointSizeFloat( float pointSize )
{
    if ( pointSize <= 0.0 ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QFont::setPointSize: Point size <= 0 (%f)", pointSize );
#endif
	return;
    }

    detach();

    d->request.pointSize = qRound(pointSize * 10.0);
    d->request.pixelSize = -1;

    d->mask |= QFontPrivate::Size;
}

/*!
    Returns the point size of the font. Returns -1 if the font size was
    specified in pixels.

    \sa pointSize() setPointSizeFloat() pixelSize() QFontInfo::pointSize() QFontInfo::pixelSize()
*/
float QFont::pointSizeFloat() const
{
    return float( d->request.pointSize == -1 ? -10 : d->request.pointSize ) / 10.0;
}

/*!
    Sets the font size to \a pixelSize pixels.

    Using this function makes the font device dependent. Use
    setPointSize() or setPointSizeFloat() to set the size of the font
    in a device independent manner.

    \sa pixelSize()
*/
void QFont::setPixelSize( int pixelSize )
{
    if ( pixelSize <= 0 ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QFont::setPixelSize: Pixel size <= 0 (%d)", pixelSize );
#endif
	return;
    }

    detach();

    d->request.pixelSize = pixelSize;
    d->request.pointSize = -1;

    d->mask |= QFontPrivate::Size;
}

/*!
    Returns the pixel size of the font if it was set with
    setPixelSize(). Returns -1 if the size was set with setPointSize()
    or setPointSizeFloat().

    \sa setPixelSize() pointSize() QFontInfo::pointSize() QFontInfo::pixelSize()
*/
int QFont::pixelSize() const
{
    return d->request.pixelSize;
}

/*! \obsolete

  Sets the logical pixel height of font characters when shown on
  the screen to \a pixelSize.
*/
void QFont::setPixelSizeFloat( float pixelSize )
{
    setPixelSize( (int)pixelSize );
}

/*!
    Returns TRUE if italic has been set; otherwise returns FALSE.

    \sa setItalic()
*/
bool QFont::italic() const
{
    return d->request.italic;
}

/*!
    If \a enable is TRUE, italic is set on; otherwise italic is set
    off.

    \sa italic(), QFontInfo
*/
void QFont::setItalic( bool enable )
{
    detach();

    d->request.italic = enable;
    d->mask |= QFontPrivate::Italic;
}

/*!
    Returns the weight of the font which is one of the enumerated
    values from \l{QFont::Weight}.

    \sa setWeight(), Weight, QFontInfo
*/
int QFont::weight() const
{
    return d->request.weight;
}

/*!
    \enum QFont::Weight

    Qt uses a weighting scale from 0 to 99 similar to, but not the
    same as, the scales used in Windows or CSS. A weight of 0 is
    ultralight, whilst 99 will be an extremely black.

    This enum contains the predefined font weights:

    \value Light 25
    \value Normal 50
    \value DemiBold 63
    \value Bold 75
    \value Black 87
*/

/*!
    Sets the weight the font to \a weight, which should be a value
    from the \l QFont::Weight enumeration.

    \sa weight(), QFontInfo
*/
void QFont::setWeight( int weight )
{
    if ( weight < 0 || weight > 99 ) {

#if defined(QT_CHECK_RANGE)
	qWarning( "QFont::setWeight: Value out of range (%d)", weight );
#endif

	return;
    }

    detach();

    d->request.weight = weight;
    d->mask |= QFontPrivate::Weight;
}

/*!
    \fn bool QFont::bold() const

    Returns TRUE if weight() is a value greater than \link Weight
    QFont::Normal \endlink; otherwise returns FALSE.

    \sa weight(), setBold(), QFontInfo::bold()
*/

/*!
    \fn void QFont::setBold( bool enable )

    If \a enable is true sets the font's weight to \link Weight
    QFont::Bold \endlink; otherwise sets the weight to \link Weight
    QFont::Normal\endlink.

    For finer boldness control use setWeight().

    \sa bold(), setWeight()
*/

/*!
    Returns TRUE if underline has been set; otherwise returns FALSE.

    \sa setUnderline()
*/
bool QFont::underline() const
{
    return d->underline;
}

/*!
    If \a enable is TRUE, sets underline on; otherwise sets underline
    off.

    \sa underline(), QFontInfo
*/
void QFont::setUnderline( bool enable )
{
    detach();

    d->underline = enable;
    d->mask |= QFontPrivate::Underline;
}

/*!
    Returns TRUE if overline has been set; otherwise returns FALSE.

    \sa setOverline()
*/
bool QFont::overline() const
{
    return d->overline;
}

/*!
  If \a enable is TRUE, sets overline on; otherwise sets overline off.

  \sa overline(), QFontInfo
*/
void QFont::setOverline( bool enable )
{
    detach();

    d->overline = enable;
    d->mask |= QFontPrivate::Overline;
}

/*!
    Returns TRUE if strikeout has been set; otherwise returns FALSE.

    \sa setStrikeOut()
*/
bool QFont::strikeOut() const
{
    return d->strikeOut;
}

/*!
    If \a enable is TRUE, sets strikeout on; otherwise sets strikeout
    off.

    \sa strikeOut(), QFontInfo
*/
void QFont::setStrikeOut( bool enable )
{
    detach();

    d->strikeOut = enable;
    d->mask |= QFontPrivate::StrikeOut;
}

/*!
    Returns TRUE if fixed pitch has been set; otherwise returns FALSE.

    \sa setFixedPitch(), QFontInfo::fixedPitch()
*/
bool QFont::fixedPitch() const
{
    return d->request.fixedPitch;
}

/*!
    If \a enable is TRUE, sets fixed pitch on; otherwise sets fixed
    pitch off.

    \sa fixedPitch(), QFontInfo
*/
void QFont::setFixedPitch( bool enable )
{
    detach();

    d->request.fixedPitch = enable;
    d->request.ignorePitch = FALSE;
    d->mask |= QFontPrivate::FixedPitch;
}

/*!
    Returns the StyleStrategy.

    The style strategy affects the \link #fontmatching font
    matching\endlink algorithm. See \l QFont::StyleStrategy for the
    list of strategies.

    \sa setStyleHint() QFont::StyleHint
*/
QFont::StyleStrategy QFont::styleStrategy() const
{
    return (StyleStrategy) d->request.styleStrategy;
}

/*!
    Returns the StyleHint.

    The style hint affects the \link #fontmatching font
    matching\endlink algorithm. See \l QFont::StyleHint for the list
    of strategies.

    \sa setStyleHint(), QFont::StyleStrategy QFontInfo::styleHint()
*/
QFont::StyleHint QFont::styleHint() const
{
    return (StyleHint) d->request.styleHint;
}

/*!
    \enum QFont::StyleHint

    Style hints are used by the \link #fontmatching font
    matching\endlink algorithm to find an appropriate default family
    if a selected font family is not available.

    \value AnyStyle leaves the font matching algorithm to choose the
	   family. This is the default.

    \value SansSerif the font matcher prefer sans serif fonts.
    \value Helvetica is a synonym for \c SansSerif.

    \value Serif the font matcher prefers serif fonts.
    \value Times is a synonym for \c Serif.

    \value TypeWriter the font matcher prefers fixed pitch fonts.
    \value Courier a synonym for \c TypeWriter.

    \value OldEnglish the font matcher prefers decorative fonts.
    \value Decorative is a synonym for \c OldEnglish.

    \value System the font matcher prefers system fonts.
*/

/*!
    \enum QFont::StyleStrategy

    The style strategy tells the \link #fontmatching font
    matching\endlink algorithm what type of fonts should be used to
    find an appropriate default family.

    The following strategies are available:

    \value PreferDefault the default style strategy. It does not prefer
	   any type of font.
    \value PreferBitmap prefers bitmap fonts (as opposed to outline
	   fonts).
    \value PreferDevice prefers device fonts.
    \value PreferOutline prefers outline fonts (as opposed to bitmap fonts).
    \value ForceOutline forces the use of outline fonts.
    \value NoAntialias don't antialias the fonts.
    \value PreferAntialias antialias if possible.
    \value OpenGLCompatible forces the use of OpenGL compatible
           fonts.

    Any of these may be OR-ed with one of these flags:

    \value PreferMatch prefer an exact match. The font matcher will try to
	   use the exact font size that has been specified.
    \value PreferQuality prefer the best quality font. The font matcher
	   will use the nearest standard point size that the font
	   supports.
*/

/*!
    Sets the style hint and strategy to \a hint and \a strategy,
    respectively.

    If these aren't set explicitly the style hint will default to
    \c AnyStyle and the style strategy to \c PreferDefault.

    Qt does not support style hints on X11 since this information
    is not provided by the window system.

    \sa StyleHint, styleHint(), StyleStrategy, styleStrategy(), QFontInfo
*/
void QFont::setStyleHint( StyleHint hint, StyleStrategy strategy )
{
    detach();

    if ( ( d->mask & ( QFontPrivate::StyleHint | QFontPrivate::StyleStrategy ) ) &&
	 (StyleHint) d->request.styleHint == hint &&
	 (StyleStrategy) d->request.styleStrategy == strategy )
	return;

    d->request.styleHint = hint;
    d->request.styleStrategy = strategy;
    d->mask |= QFontPrivate::StyleHint;
    d->mask |= QFontPrivate::StyleStrategy;

#if defined(Q_WS_X11)
    d->request.addStyle = QString::null;
#endif // Q_WS_X11
}

/*!
    Sets the style strategy for the font to \a s.

    \sa QFont::StyleStrategy
*/
void QFont::setStyleStrategy( StyleStrategy s )
{
    detach();

    if ( ( d->mask & QFontPrivate::StyleStrategy ) &&
	 s == (StyleStrategy)d->request.styleStrategy )
	return;

    d->request.styleStrategy = s;
    d->mask |= QFontPrivate::StyleStrategy;
}


/*!
    \enum QFont::Stretch

    Predefined stretch values that follow the CSS naming convention.

    \value UltraCondensed 50
    \value ExtraCondensed 62
    \value Condensed 75
    \value SemiCondensed 87
    \value Unstretched 100
    \value SemiExpanded 112
    \value Expanded 125
    \value ExtraExpanded 150
    \value UltraExpanded 200

    \sa setStretch() stretch()
*/

/*!
    Returns the stretch factor for the font.

    \sa setStretch()
 */
int QFont::stretch() const
{
    return d->request.stretch;
}

/*!
    Sets the stretch factor for the font.

    The stretch factor changes the width of all characters in the font
    by \a factor percent.  For example, setting \a factor to 150
    results in all characters in the font being 1.5 times ( ie. 150% )
    wider.  The default stretch factor is 100.  The minimum stretch
    factor is 1, and the maximum stretch factor is 4000.

    The stretch factor is only applied to outline fonts.  The stretch
    factor is ignored for bitmap fonts.

    NOTE: QFont cannot stretch XLFD fonts.  When loading XLFD fonts on
    X11, the stretch factor is matched against a predefined set of
    values for the SETWIDTH_NAME field of the XLFD.

    \sa stretch() QFont::StyleStrategy
*/
void QFont::setStretch( int factor )
{
    if ( factor < 1 || factor > 4000 ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QFont::setStretch(): parameter '%d' out of range", factor );
#endif // QT_CHECK_RANGE

	return;
    }

    detach();

    if ( ( d->mask & QFontPrivate::Stretch ) &&
	 d->request.stretch == (uint)factor )
	return;

    d->request.stretch = (uint)factor;
    d->mask |= QFontPrivate::Stretch;
}

/*!
    If \a enable is TRUE, turns raw mode on; otherwise turns raw mode
    off. This function only has an effect under X11.

    If raw mode is enabled, Qt will search for an X font with a
    complete font name matching the family name, ignoring all other
    values set for the QFont. If the font name matches several fonts,
    Qt will use the first font returned by X. QFontInfo \e cannot be
    used to fetch information about a QFont using raw mode (it will
    return the values set in the QFont for all parameters, including
    the family name).

    \warning Do not use raw mode unless you really, really need it! In
    most (if not all) cases, setRawName() is a much better choice.

    \sa rawMode(), setRawName()
*/
void QFont::setRawMode( bool enable )
{
    detach();

    if ( (bool) d->rawMode == enable ) return;

    d->rawMode = enable;
}

/*!
    Returns TRUE if a window system font exactly matching the settings
    of this font is available.

    \sa QFontInfo
*/
bool QFont::exactMatch() const
{
    QFontEngine *engine = d->engineForScript( QFont::NoScript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    return d->rawMode ? engine->type() != QFontEngine::Box
			     : d->request == engine->fontDef;
}

/*!
    Returns TRUE if this font is equal to \a f; otherwise returns
    FALSE.

    Two QFonts are considered equal if their font attributes are
    equal. If rawMode() is enabled for both fonts, only the family
    fields are compared.

    \sa operator!=() isCopyOf()
*/
bool QFont::operator==( const QFont &f ) const
{
    return f.d == d || ( f.d->request   == d->request   &&
			 f.d->underline == d->underline &&
			 f.d->overline  == d->overline  &&
			 f.d->strikeOut == d->strikeOut );
}

/*!
    Returns TRUE if this font is different from \a f; otherwise
    returns FALSE.

    Two QFonts are considered to be different if their font attributes
    are different. If rawMode() is enabled for both fonts, only the
    family fields are compared.

    \sa operator==()
*/
bool QFont::operator!=( const QFont &f ) const
{
    return !(operator==( f ));
}

/*!
    Returns TRUE if this font and \a f are copies of each other, i.e.
    one of them was created as a copy of the other and neither has
    been modified since. This is much stricter than equality.

    \sa operator=() operator==()
*/
bool QFont::isCopyOf( const QFont & f ) const
{
    return d == f.d;
}

/*!
    Returns TRUE if raw mode is used for font name matching; otherwise
    returns FALSE.

    \sa setRawMode() rawName()
*/
bool QFont::rawMode() const
{
    return d->rawMode;
}

/*!
    Returns a new QFont that has attributes copied from \a other.
*/
QFont QFont::resolve( const QFont &other ) const
{
    if ( *this == other && d->mask == other.d->mask )
	return *this;

    QFont font( *this );
    font.detach();

    /*
      if this font is a copy of the application default font, set the
      fontdef mask to zero to indicate that *nothing* has been
      explicitly set by the programmer.
    */
    const QFont appfont = QApplication::font();
    if ( d == appfont.d )
	font.d->mask = 0;

    font.d->resolve( other.d );

    return font;
}

#ifndef QT_NO_COMPAT

/*! \obsolete

  Please use QApplication::font() instead.
*/
QFont QFont::defaultFont()
{
    return QApplication::font();
}

/*! \obsolete

  Please use QApplication::setFont() instead.
*/
void QFont::setDefaultFont( const QFont &f )
{
    QApplication::setFont( f );
}


#endif




#ifndef QT_NO_STRINGLIST

/*****************************************************************************
  QFont substitution management
 *****************************************************************************/

typedef QDict<QStringList> QFontSubst;
static QFontSubst *fontSubst = 0;
static QSingleCleanupHandler<QFontSubst> qfont_cleanup_fontsubst;


// create substitution dict
static void initFontSubst()
{
    // default substitutions
    static const char *initTbl[] = {

#if defined(Q_WS_X11)
	"arial",        "helvetica",
	"helv",         "helvetica",
	"tms rmn",      "times",
#elif defined(Q_WS_WIN)
	"times",        "Times New Roman",
	"courier",      "Courier New",
	"helvetica",    "Arial",
#endif

	0,              0
    };

    if (fontSubst)
	return;

    fontSubst = new QFontSubst(17, FALSE);
    Q_CHECK_PTR( fontSubst );
    fontSubst->setAutoDelete( TRUE );
    qfont_cleanup_fontsubst.set(&fontSubst);

    for ( int i=0; initTbl[i] != 0; i += 2 )
	QFont::insertSubstitution(QString::fromLatin1(initTbl[i]),
				  QString::fromLatin1(initTbl[i+1]));
}


/*!
    Returns the first family name to be used whenever \a familyName is
    specified. The lookup is case insensitive.

    If there is no substitution for \a familyName, \a familyName is
    returned.

    To obtain a list of substitutions use substitutes().

    \sa setFamily() insertSubstitutions() insertSubstitution() removeSubstitution()
*/
QString QFont::substitute( const QString &familyName )
{
    initFontSubst();

    QStringList *list = fontSubst->find(familyName);
    if (list && list->count() > 0)
	return *(list->at(0));

    return familyName;
}


/*!
    Returns a list of family names to be used whenever \a familyName
    is specified. The lookup is case insensitive.

    If there is no substitution for \a familyName, an empty list is
    returned.

    \sa substitute() insertSubstitutions() insertSubstitution() removeSubstitution()
 */
QStringList QFont::substitutes(const QString &familyName)
{
    initFontSubst();

    QStringList ret, *list = fontSubst->find(familyName);
    if (list)
	ret += *list;
    return ret;
}


/*!
    Inserts the family name \a substituteName into the substitution
    table for \a familyName.

    \sa insertSubstitutions() removeSubstitution() substitutions() substitute() substitutes()
*/
void QFont::insertSubstitution(const QString &familyName,
			       const QString &substituteName)
{
    initFontSubst();

    QStringList *list = fontSubst->find(familyName);
    if (! list) {
	list = new QStringList;
	fontSubst->insert(familyName, list);
    }

    if (! list->contains(substituteName))
	list->append(substituteName);
}


/*!
    Inserts the list of families \a substituteNames into the
    substitution list for \a familyName.

    \sa insertSubstitution(), removeSubstitution(), substitutions(), substitute()
*/
void QFont::insertSubstitutions(const QString &familyName,
				const QStringList &substituteNames)
{
    initFontSubst();

    QStringList *list = fontSubst->find(familyName);
    if (! list) {
	list = new QStringList;
	fontSubst->insert(familyName, list);
    }

    QStringList::ConstIterator it = substituteNames.begin();
    while (it != substituteNames.end()) {
	if (! list->contains(*it))
	    list->append(*it);
	it++;
    }
}

// ### mark: should be called removeSubstitutions()
/*!
    Removes all the substitutions for \a familyName.

    \sa insertSubstitutions(), insertSubstitution(), substitutions(), substitute()
*/
void QFont::removeSubstitution( const QString &familyName )
{ // ### function name should be removeSubstitutions() or
  // ### removeSubstitutionList()
    initFontSubst();

    fontSubst->remove(familyName);
}


/*!
    Returns a sorted list of substituted family names.

    \sa insertSubstitution(), removeSubstitution(), substitute()
*/
QStringList QFont::substitutions()
{
    initFontSubst();

    QStringList ret;
    QDictIterator<QStringList> it(*fontSubst);

    while (it.current()) {
	ret.append(it.currentKey());
	++it;
    }

    ret.sort();

    return ret;
}

#endif // QT_NO_STRINGLIST


/*  \internal
    Internal function. Converts boolean font settings to an unsigned
    8-bit number. Used for serialization etc.
*/
static Q_UINT8 get_font_bits( const QFontPrivate *f )
{
#ifdef QT_CHECK_STATE
    Q_ASSERT( f != 0 );
#endif

    Q_UINT8 bits = 0;
    if ( f->request.italic )
	bits |= 0x01;
    if ( f->underline )
	bits |= 0x02;
    if ( f->overline )
	bits |= 0x40;
    if ( f->strikeOut )
	bits |= 0x04;
    if ( f->request.fixedPitch )
	bits |= 0x08;
    // if ( f.hintSetByUser )
    // bits |= 0x10;
    if ( f->rawMode )
	bits |= 0x20;
    return bits;
}


#ifndef QT_NO_DATASTREAM

/*  \internal
    Internal function. Sets boolean font settings from an unsigned
    8-bit number. Used for serialization etc.
*/
static void set_font_bits( Q_UINT8 bits, QFontPrivate *f )
{
#ifdef QT_CHECK_STATE
    Q_ASSERT( f != 0 );
#endif

    f->request.italic        = (bits & 0x01) != 0;
    f->underline             = (bits & 0x02) != 0;
    f->overline              = (bits & 0x40) != 0;
    f->strikeOut             = (bits & 0x04) != 0;
    f->request.fixedPitch    = (bits & 0x08) != 0;
    // f->hintSetByUser      = (bits & 0x10) != 0;
    f->rawMode               = (bits & 0x20) != 0;
}

#endif


/*!
    Returns the font's key, a textual representation of a font. It is
    typically used as the key for a cache or dictionary of fonts.

    \sa QMap
*/
QString QFont::key() const
{
    return toString();
}

/*!
    Returns a description of the font. The description is a
    comma-separated list of the attributes, perfectly suited for use
    in QSettings.

    \sa fromString() operator<<()
 */
QString QFont::toString() const
{
    const QChar comma( ',' );
    return family() + comma +
	QString::number(  pointSizeFloat() ) + comma +
	QString::number(       pixelSize() ) + comma +
	QString::number( (int) styleHint() ) + comma +
	QString::number(          weight() ) + comma +
	QString::number( (int)    italic() ) + comma +
	QString::number( (int) underline() ) + comma +
	QString::number( (int) strikeOut() ) + comma +
	QString::number( (int)fixedPitch() ) + comma +
	QString::number( (int)   rawMode() );
}


/*!
    Sets this font to match the description \a descrip. The description
    is a comma-separated list of the font attributes, as returned by
    toString().

    \sa toString() operator>>()
 */
bool QFont::fromString(const QString &descrip)
{
#ifndef QT_NO_STRINGLIST
    QStringList l(QStringList::split(',', descrip));

    int count = (int)l.count();
#else
    int count = 0;
    QString l[11];
    int from = 0;
    int to = descrip.find( ',' );
    while ( to > 0 && count < 11 ) {
	l[count] = descrip.mid( from, to-from );
	count++;
	from = to+1;
	to = descrip.find( ',', from );
    }
#endif // QT_NO_STRINGLIST
    if ( !count || ( count > 2 && count < 9 ) || count > 11 ) {

#ifdef QT_CHECK_STATE
	qWarning("QFont::fromString: invalid description '%s'",
                 descrip.isEmpty() ? "(empty)" : descrip.latin1());
#endif

	return FALSE;
    }

    setFamily(l[0]);
    if ( count > 1 && l[1].toDouble() > 0.0 )
	setPointSizeFloat(l[1].toDouble());
    if ( count == 9 ) {
	setStyleHint((StyleHint) l[2].toInt());
	setWeight(l[3].toInt());
	setItalic(l[4].toInt());
	setUnderline(l[5].toInt());
	setStrikeOut(l[6].toInt());
	setFixedPitch(l[7].toInt());
	setRawMode(l[8].toInt());
    } else if ( count == 10 ) {
	if ( l[2].toInt() > 0 )
	    setPixelSize( l[2].toInt() );
	setStyleHint((StyleHint) l[3].toInt());
	setWeight(l[4].toInt());
	setItalic(l[5].toInt());
	setUnderline(l[6].toInt());
	setStrikeOut(l[7].toInt());
	setFixedPitch(l[8].toInt());
	setRawMode(l[9].toInt());
    }

    return TRUE;
}

#if !defined( Q_WS_QWS )
/*! \internal

  Internal function that dumps font cache statistics.
*/
void QFont::cacheStatistics()
{


}
#endif // !Q_WS_QWS



/*****************************************************************************
  QFont stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM

/*!
    \relates QFont

    Writes the font \a font to the data stream \a s. (toString()
    writes to a text stream.)

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator<<( QDataStream &s, const QFont &font )
{
    if ( s.version() == 1 ) {
	QCString fam( font.d->request.family.latin1() );
	s << fam;
    } else {
	s << font.d->request.family;
    }

    if ( s.version() <= 3 ) {
	Q_INT16 pointSize = (Q_INT16) font.d->request.pointSize;
	if ( pointSize == -1 ) {
#ifdef Q_WS_X11
	    pointSize = (Q_INT16)(font.d->request.pixelSize*720/QPaintDevice::x11AppDpiY());
#else
	    pointSize = (Q_INT16)QFontInfo( font ).pointSize() * 10;
#endif
	}
	s << pointSize;
    } else {
	s << (Q_INT16) font.d->request.pointSize;
	s << (Q_INT16) font.d->request.pixelSize;
    }

    s << (Q_UINT8) font.d->request.styleHint;
    if ( s.version() >= 5 )
	s << (Q_UINT8 ) font.d->request.styleStrategy;
    return s << (Q_UINT8) 0
	     << (Q_UINT8) font.d->request.weight
	     << get_font_bits(font.d);
}


/*!
    \relates QFont

    Reads the font \a font from the data stream \a s. (fromString()
    reads from a text stream.)

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/
QDataStream &operator>>( QDataStream &s, QFont &font )
{
    if (font.d->deref()) delete font.d;

    font.d = new QFontPrivate;
    font.d->mask = QFontPrivate::Complete;

    Q_INT16 pointSize, pixelSize = -1;
    Q_UINT8 styleHint, styleStrategy = QFont::PreferDefault, charSet, weight, bits;

    if ( s.version() == 1 ) {
	QCString fam;
	s >> fam;
	font.d->request.family = QString( fam );
    } else {
	s >> font.d->request.family;
    }

    s >> pointSize;
    if ( s.version() >= 4 )
	s >> pixelSize;
    s >> styleHint;
    if ( s.version() >= 5 )
	s >> styleStrategy;
    s >> charSet;
    s >> weight;
    s >> bits;

    font.d->request.pointSize = pointSize;
    font.d->request.pixelSize = pixelSize;
    font.d->request.styleHint = styleHint;
    font.d->request.styleStrategy = styleStrategy;
    font.d->request.weight = weight;

    set_font_bits( bits, font.d );

    return s;
}

#endif // QT_NO_DATASTREAM




/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

/*!
    \class QFontMetrics qfontmetrics.h
    \brief The QFontMetrics class provides font metrics information.

    \ingroup graphics
    \ingroup shared

    QFontMetrics functions calculate the size of characters and
    strings for a given font. There are three ways you can create a
    QFontMetrics object:

    \list 1
    \i Calling the QFontMetrics constructor with a QFont creates a
    font metrics object for a screen-compatible font, i.e. the font
    cannot be a printer font<sup>*</sup>. If the font is changed
    later, the font metrics object is \e not updated.

    \i QWidget::fontMetrics() returns the font metrics for a widget's
    font. This is equivalent to QFontMetrics(widget->font()). If the
    widget's font is changed later, the font metrics object is \e not
    updated.

    \i QPainter::fontMetrics() returns the font metrics for a
    painter's current font. If the painter's font is changed later, the
    font metrics object is \e not updated.
    \endlist

    <sup>*</sup> If you use a printer font the values returned may be
    inaccurate. Printer fonts are not always accessible so the nearest
    screen font is used if a printer font is supplied.

    Once created, the object provides functions to access the
    individual metrics of the font, its characters, and for strings
    rendered in the font.

    There are several functions that operate on the font: ascent(),
    descent(), height(), leading() and lineSpacing() return the basic
    size properties of the font. The underlinePos(), overlinePos(),
    strikeOutPos() and lineWidth() functions, return the properties of
    the line that underlines, overlines or strikes out the
    characters. These functions are all fast.

    There are also some functions that operate on the set of glyphs in
    the font: minLeftBearing(), minRightBearing() and maxWidth().
    These are by necessity slow, and we recommend avoiding them if
    possible.

    For each character, you can get its width(), leftBearing() and
    rightBearing() and find out whether it is in the font using
    inFont(). You can also treat the character as a string, and use
    the string functions on it.

    The string functions include width(), to return the width of a
    string in pixels (or points, for a printer), boundingRect(), to
    return a rectangle large enough to contain the rendered string,
    and size(), to return the size of that rectangle.

    Example:
    \code
    QFont font( "times", 24 );
    QFontMetrics fm( font );
    int pixelsWide = fm.width( "What's the width of this text?" );
    int pixelsHigh = fm.height();
    \endcode

    \sa QFont QFontInfo QFontDatabase
*/

/*!
    Constructs a font metrics object for \a font.

    The font must be screen-compatible, i.e. a font you use when
    drawing text in \link QWidget widgets\endlink or \link QPixmap
    pixmaps\endlink, not QPicture or QPrinter.

    The font metrics object holds the information for the font that is
    passed in the constructor at the time it is created, and is not
    updated if the font's attributes are changed later.

  Use QPainter::fontMetrics() to get the font metrics when painting.
  This will give correct results also when painting on paint device
  that is not screen-compatible.
*/
QFontMetrics::QFontMetrics( const QFont &font )
    : d( font.d ), painter( 0 ), fscript( QFont::NoScript )
{
    d->ref();
}

/*!
    \overload

    Constructs a font metrics object for \a font using the given \a
    script.
*/
QFontMetrics::QFontMetrics( const QFont &font, QFont::Script script )
    : d( font.d ), painter( 0 ), fscript( script )
{
    d->ref();
}

/*! \internal

  Constructs a font metrics object for the painter's font \a p.
*/
QFontMetrics::QFontMetrics( const QPainter *p )
    : painter ( (QPainter *) p ), fscript( QFont::NoScript )
{
#if defined(CHECK_STATE)
    if ( !painter->isActive() )
	qWarning( "QFontMetrics: Get font metrics between QPainter::begin() "
		  "and QPainter::end()" );
#endif

    if ( painter->testf(QPainter::DirtyFont) )
	painter->updateFont();

    d = painter->pfont ? painter->pfont->d : painter->cfont.d;

#if defined(Q_WS_X11)
    if ( d->screen != p->scrn ) {
	QFontPrivate *new_d = new QFontPrivate( *d );
	Q_CHECK_PTR( new_d );
	d = new_d;
	d->screen = p->scrn;
	d->count = 1;
    } else
#endif // Q_WS_X11
	d->ref();
}

/*!
    Constructs a copy of \a fm.
*/
QFontMetrics::QFontMetrics( const QFontMetrics &fm )
    : d( fm.d ), painter( 0 ),  fscript( fm.fscript )
{
    d->ref();
}

/*!
    Destroys the font metrics object and frees all allocated
    resources.
*/
QFontMetrics::~QFontMetrics()
{
    if ( d->deref() )
	delete d;
}

/*!
    Assigns the font metrics \a fm.
*/
QFontMetrics &QFontMetrics::operator=( const QFontMetrics &fm )
{
    if ( d != fm.d ) {
	if ( d->deref() )
	    delete d;
	d = fm.d;
	d->ref();
    }
    painter = fm.painter;
    return *this;
}

/*!
    Returns the ascent of the font.

    The ascent of a font is the distance from the baseline to the
    highest position characters extend to. In practice, some font
    designers break this rule, e.g. when they put more than one accent
    on top of a character, or to accommodate an unusual character in
    an exotic language, so it is possible (though rare) that this
    value will be too small.

    \sa descent()
*/
int QFontMetrics::ascent() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
    QFontEngine *latin_engine = d->engineForScript( QFont::Latin );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
    Q_ASSERT( latin_engine != 0 );
#endif // QT_CHECK_STATE

    return QMAX(engine->ascent(), latin_engine->ascent());
}


/*!
    Returns the descent of the font.

    The descent is the distance from the base line to the lowest point
    characters extend to. (Note that this is different from X, which
    adds 1 pixel.) In practice, some font designers break this rule,
    e.g. to accommodate an unusual character in an exotic language, so
    it is possible (though rare) that this value will be too small.

    \sa ascent()
*/
int QFontMetrics::descent() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
    QFontEngine *latin_engine = d->engineForScript( QFont::Latin );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
    Q_ASSERT( latin_engine != 0 );
#endif // QT_CHECK_STATE

    return QMAX(engine->descent(), latin_engine->descent());
}

/*!
    Returns the height of the font.

    This is always equal to ascent()+descent()+1 (the 1 is for the
    base line).

    \sa leading(), lineSpacing()
*/
int QFontMetrics::height() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
    QFontEngine *latin_engine = d->engineForScript( QFont::Latin );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
    Q_ASSERT( latin_engine != 0 );
#endif // QT_CHECK_STATE

    return (QMAX(engine->ascent(), latin_engine->ascent()) +
	    QMAX(engine->descent(), latin_engine->descent()) + 1);
}

/*!
    Returns the leading of the font.

    This is the natural inter-line spacing.

    \sa height(), lineSpacing()
*/
int QFontMetrics::leading() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
    QFontEngine *latin_engine = d->engineForScript( QFont::Latin );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
    Q_ASSERT( latin_engine != 0 );
#endif // QT_CHECK_STATE

    return QMAX(engine->leading(), latin_engine->leading());
}

/*!
    Returns the distance from one base line to the next.

    This value is always equal to leading()+height().

    \sa height(), leading()
*/
int QFontMetrics::lineSpacing() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
    QFontEngine *latin_engine = d->engineForScript( QFont::Latin );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
    Q_ASSERT( latin_engine != 0 );
#endif // QT_CHECK_STATE

    return (QMAX(engine->leading(), latin_engine->leading()) +
	    QMAX(engine->ascent(), latin_engine->ascent()) +
	    QMAX(engine->descent(), latin_engine->descent()) + 1);
}

/*!
    Returns the minimum left bearing of the font.

    This is the smallest leftBearing(char) of all characters in the
    font.

    Note that this function can be very slow if the font is large.

    \sa minRightBearing(), leftBearing()
*/
int QFontMetrics::minLeftBearing() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
    QFontEngine *latin_engine = d->engineForScript( QFont::Latin );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
    Q_ASSERT( latin_engine != 0 );
#endif // QT_CHECK_STATE

    return QMIN(engine->minLeftBearing(), latin_engine->minLeftBearing());
}

/*!
    Returns the minimum right bearing of the font.

    This is the smallest rightBearing(char) of all characters in the
    font.

    Note that this function can be very slow if the font is large.

    \sa minLeftBearing(), rightBearing()
*/
int QFontMetrics::minRightBearing() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
    QFontEngine *latin_engine = d->engineForScript( QFont::Latin );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
    Q_ASSERT( latin_engine != 0 );
#endif // QT_CHECK_STATE

    return QMIN(engine->minRightBearing(), latin_engine->minRightBearing());
}

/*!
    Returns the width of the widest character in the font.
*/
int QFontMetrics::maxWidth() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
    QFontEngine *lengine = d->engineForScript( QFont::Latin );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
    Q_ASSERT( lengine != 0 );
#endif // QT_CHECK_STATE

    return QMAX(engine->maxCharWidth(), lengine->maxCharWidth());
}

/*!
    Returns TRUE if character \a ch is a valid character in the font;
    otherwise returns FALSE.
*/
bool QFontMetrics::inFont(QChar ch) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch );

    QFontEngine *engine = d->engineForScript( script );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    if ( engine->type() == QFontEngine::Box ) return FALSE;
    return engine->canRender( &ch, 1 );
}

/*! \fn int QFontMetrics::leftBearing( QChar ch ) const
    Returns the left bearing of character \a ch in the font.

    The left bearing is the right-ward distance of the left-most pixel
    of the character from the logical origin of the character. This
    value is negative if the pixels of the character extend to the
    left of the logical origin.

    See width(QChar) for a graphical description of this metric.

    \sa rightBearing(), minLeftBearing(), width()
*/
#if !defined(Q_WS_WIN) && !defined(Q_WS_QWS)
int QFontMetrics::leftBearing(QChar ch) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch );

    QFontEngine *engine = d->engineForScript( script );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    if ( engine->type() == QFontEngine::Box ) return 0;

    glyph_t glyphs[10];
    int nglyphs = 9;
    engine->stringToCMap( &ch, 1, glyphs, 0, &nglyphs, FALSE );
    // ### can nglyphs != 1 happen at all? Not currently I think
    glyph_metrics_t gi = engine->boundingBox( glyphs[0] );
    return gi.x;
}
#endif // !Q_WS_WIN

/*! \fn int QFontMetrics::rightBearing(QChar ch) const
    Returns the right bearing of character \a ch in the font.

    The right bearing is the left-ward distance of the right-most
    pixel of the character from the logical origin of a subsequent
    character. This value is negative if the pixels of the character
    extend to the right of the width() of the character.

    See width() for a graphical description of this metric.

    \sa leftBearing(), minRightBearing(), width()
*/
#if !defined(Q_WS_WIN) && !defined(Q_WS_QWS)
int QFontMetrics::rightBearing(QChar ch) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch );

    QFontEngine *engine = d->engineForScript( script );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    if ( engine->type() == QFontEngine::Box ) return 0;

    glyph_t glyphs[10];
    int nglyphs = 9;
    engine->stringToCMap( &ch, 1, glyphs, 0, &nglyphs, FALSE );
    // ### can nglyphs != 1 happen at all? Not currently I think
    glyph_metrics_t gi = engine->boundingBox( glyphs[0] );
    return gi.xoff - gi.x - gi.width;
}
#endif // !Q_WS_WIN


#ifndef Q_WS_QWS
/*!
    Returns the width in pixels of the first \a len characters of \a
    str. If \a len is negative (the default), the entire string is
    used.

    Note that this value is \e not equal to boundingRect().width();
    boundingRect() returns a rectangle describing the pixels this
    string will cover whereas width() returns the distance to where
    the next string should be drawn.

    \sa boundingRect()
*/
int QFontMetrics::width( const QString &str, int len ) const
{
    if (len < 0)
	len = str.length();
    if (len == 0)
	return 0;

    int pos = 0;
    int width = 0;
#ifndef Q_WS_MAC
    const QChar *ch = str.unicode();

    while (pos < len) {
	unsigned short uc = ch->unicode();
	if (uc < QFontEngineData::widthCacheSize && d->engineData && d->engineData->widthCache[uc])
	    width += d->engineData->widthCache[uc];
	else {
	    QFont::Script script;
	    SCRIPT_FOR_CHAR( script, *ch );

	    if (script >= QFont::Arabic && script <= QFont::Khmer)
		break;
            if ( ::category( *ch ) != QChar::Mark_NonSpacing && !qIsZeroWidthChar(ch->unicode())) {
                QFontEngine *engine = d->engineForScript( script );
#ifdef QT_CHECK_STATE
                Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

                glyph_t glyphs[8];
                advance_t advances[8];
                int nglyphs = 7;
                engine->stringToCMap( ch, 1, glyphs, advances, &nglyphs, FALSE );

                // ### can nglyphs != 1 happen at all? Not currently I think
                if ( uc < QFontEngineData::widthCacheSize && advances[0] > 0 && advances[0] < 0x100 )
                    d->engineData->widthCache[ uc ] = advances[0];
                width += advances[0];
            }
	}
	++pos;
	++ch;
    }
    if ( pos < len ) {
#endif
	QTextEngine layout( str, d );
	layout.itemize( QTextEngine::WidthOnly );
	width += layout.width( pos, len-pos );
#ifndef Q_WS_MAC
    }
#endif
    return width;
}
#endif

/*! \fn int QFontMetrics::width( QChar ch ) const

    <img src="bearings.png" align=right>

    Returns the logical width of character \a ch in pixels. This is a
    distance appropriate for drawing a subsequent character after \a
    ch.

    Some of the metrics are described in the image to the right. The
    central dark rectangles cover the logical width() of each
    character. The outer pale rectangles cover the leftBearing() and
    rightBearing() of each character. Notice that the bearings of "f"
    in this particular font are both negative, while the bearings of
    "o" are both positive.

    \warning This function will produce incorrect results for Arabic
    characters or non spacing marks in the middle of a string, as the
    glyph shaping and positioning of marks that happens when
    processing strings cannot be taken into account. Use charWidth()
    instead if you aren't looking for the width of isolated
    characters.

    \sa boundingRect(), charWidth()
*/

/*! \fn int QFontMetrics::width( char c ) const

  \overload
  \obsolete

  Provided to aid porting from Qt 1.x.
*/

/*! \fn int QFontMetrics::charWidth( const QString &str, int pos ) const
    Returns the width of the character at position \a pos in the
    string \a str.

    The whole string is needed, as the glyph drawn may change
    depending on the context (the letter before and after the current
    one) for some languages (e.g. Arabic).

    This function also takes non spacing marks and ligatures into
    account.
*/

#ifndef Q_WS_QWS
/*!
    Returns the bounding rectangle of the first \a len characters of
    \a str, which is the set of pixels the text would cover if drawn
    at (0, 0).

    If \a len is negative (the default), the entire string is used.

    Note that the bounding rectangle may extend to the left of (0, 0),
    e.g. for italicized fonts, and that the text output may cover \e
    all pixels in the bounding rectangle.

    Newline characters are processed as normal characters, \e not as
    linebreaks.

    Due to the different actual character heights, the height of the
    bounding rectangle of e.g. "Yes" and "yes" may be different.

    \sa width(), QPainter::boundingRect()
*/
QRect QFontMetrics::boundingRect( const QString &str, int len ) const
{
    if (len < 0)
	len = str.length();
    if (len == 0)
	return QRect();

    QTextEngine layout( str, d );
    layout.itemize( QTextEngine::NoBidi|QTextEngine::SingleLine );
    glyph_metrics_t gm = layout.boundingBox( 0, len );
    return QRect( gm.x, gm.y, gm.width, gm.height );
}
#endif

/*!
    Returns the rectangle that is covered by ink if the character
    specified by \a ch were to be drawn at the origin of the coordinate
    system.

    Note that the bounding rectangle may extend to the left of (0, 0),
    e.g. for italicized fonts, and that the text output may cover \e
    all pixels in the bounding rectangle. For a space character the rectangle
    will usually be empty.

    Note that the rectangle usually extends both above and below the
    base line.
    
    \warning The width of the returned rectangle is not the advance width
    of the character. Use boundingRect(const QString &) or width() instead.
    
    \sa width() 
*/
QRect QFontMetrics::boundingRect( QChar ch ) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch );

    QFontEngine *engine = d->engineForScript( script );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    glyph_t glyphs[10];
    int nglyphs = 9;
    engine->stringToCMap( &ch, 1, glyphs, 0, &nglyphs, FALSE );
    glyph_metrics_t gi = engine->boundingBox( glyphs[0] );
    return QRect( gi.x, gi.y, gi.width, gi.height );
}

/*!
    \overload

    Returns the bounding rectangle of the first \a len characters of
    \a str, which is the set of pixels the text would cover if drawn
    at (0, 0). The drawing, and hence the bounding rectangle, is
    constrained to the rectangle (\a x, \a y, \a w, \a h).

    If \a len is negative (which is the default), the entire string is
    used.

    The \a flgs argument is the bitwise OR of the following flags:
    \list
    \i \c AlignAuto aligns to the left border for all languages except
	  Arabic and Hebrew where it aligns to the right.
    \i \c AlignLeft aligns to the left border.
    \i \c AlignRight aligns to the right border.
    \i \c AlignJustify produces justified text.
    \i \c AlignHCenter aligns horizontally centered.
    \i \c AlignTop aligns to the top border.
    \i \c AlignBottom aligns to the bottom border.
    \i \c AlignVCenter aligns vertically centered
    \i \c AlignCenter (== \c{AlignHCenter | AlignVCenter})
    \i \c SingleLine ignores newline characters in the text.
    \i \c ExpandTabs expands tabs (see below)
    \i \c ShowPrefix interprets "&amp;x" as "<u>x</u>", i.e. underlined.
    \i \c WordBreak breaks the text to fit the rectangle.
    \endlist

    Horizontal alignment defaults to \c AlignAuto and vertical
    alignment defaults to \c AlignTop.

    If several of the horizontal or several of the vertical alignment
    flags are set, the resulting alignment is undefined.

    These flags are defined in \c qnamespace.h.

    If \c ExpandTabs is set in \a flgs, then: if \a tabarray is
    non-null, it specifies a 0-terminated sequence of pixel-positions
    for tabs; otherwise if \a tabstops is non-zero, it is used as the
    tab spacing (in pixels).

    Note that the bounding rectangle may extend to the left of (0, 0),
    e.g. for italicized fonts, and that the text output may cover \e
    all pixels in the bounding rectangle.

    Newline characters are processed as linebreaks.

    Despite the different actual character heights, the heights of the
    bounding rectangles of "Yes" and "yes" are the same.

    The bounding rectangle given by this function is somewhat larger
    than that calculated by the simpler boundingRect() function. This
    function uses the \link minLeftBearing() maximum left \endlink and
    \link minRightBearing() right \endlink font bearings as is
    necessary for multi-line text to align correctly. Also,
    fontHeight() and lineSpacing() are used to calculate the height,
    rather than individual character heights.

    The \a intern argument should not be used.

    \sa width(), QPainter::boundingRect(), Qt::AlignmentFlags
*/
QRect QFontMetrics::boundingRect( int x, int y, int w, int h, int flgs,
				  const QString& str, int len, int tabstops,
				  int *tabarray, QTextParag **intern ) const
{
    if ( len < 0 )
	len = str.length();

    int tabarraylen=0;
    if (tabarray)
	while (tabarray[tabarraylen])
	    tabarraylen++;

    QRect rb;
    QRect r(x, y, w, h);
    qt_format_text( QFont( d, d->paintdevice ), r, flgs|Qt::DontPrint, str, len, &rb,
		    tabstops, tabarray, tabarraylen, intern, 0 );

    return rb;
}

/*!
    Returns the size in pixels of the first \a len characters of \a
    str.

    If \a len is negative (the default), the entire string is used.

    The \a flgs argument is the bitwise OR of the following flags:
    \list
    \i \c SingleLine ignores newline characters.
    \i \c ExpandTabs expands tabs (see below)
    \i \c ShowPrefix interprets "&amp;x" as "<u>x</u>", i.e. underlined.
    \i \c WordBreak breaks the text to fit the rectangle.
    \endlist

    These flags are defined in \c qnamespace.h.

    If \c ExpandTabs is set in \a flgs, then: if \a tabarray is
    non-null, it specifies a 0-terminated sequence of pixel-positions
    for tabs; otherwise if \a tabstops is non-zero, it is used as the
    tab spacing (in pixels).

    Newline characters are processed as linebreaks.

    Despite the different actual character heights, the heights of the
    bounding rectangles of "Yes" and "yes" are the same.

    The \a intern argument should not be used.

    \sa boundingRect()
*/
QSize QFontMetrics::size( int flgs, const QString &str, int len, int tabstops,
			  int *tabarray, QTextParag **intern ) const
{
    return boundingRect(0,0,0,0,flgs,str,len,tabstops,tabarray,intern).size();
}

/*!
    Returns the distance from the base line to where an underscore
    should be drawn.

    \sa overlinePos(), strikeOutPos(), lineWidth()
*/
int QFontMetrics::underlinePos() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    return engine->underlinePosition();
}

/*!
    Returns the distance from the base line to where an overline
    should be drawn.

    \sa underlinePos(), strikeOutPos(), lineWidth()
*/
int QFontMetrics::overlinePos() const
{
    int pos = ascent() + 1;
    return pos > 0 ? pos : 1;
}

/*!
    Returns the distance from the base line to where the strikeout
    line should be drawn.

    \sa underlinePos(), overlinePos(), lineWidth()
*/
int QFontMetrics::strikeOutPos() const
{
    int pos = ascent() / 3;
    return pos > 0 ? pos : 1;
}

/*!
    Returns the width of the underline and strikeout lines, adjusted
    for the point size of the font.

    \sa underlinePos(), overlinePos(), strikeOutPos()
*/
int QFontMetrics::lineWidth() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    return engine->lineThickness();
}




/*****************************************************************************
  QFontInfo member functions
 *****************************************************************************/

/*!
    \class QFontInfo qfontinfo.h

    \brief The QFontInfo class provides general information about fonts.

    \ingroup graphics
    \ingroup shared

    The QFontInfo class provides the same access functions as QFont,
    e.g. family(), pointSize(), italic(), weight(), fixedPitch(),
    styleHint() etc. But whilst the QFont access functions return the
    values that were set, a QFontInfo object returns the values that
    apply to the font that will actually be used to draw the text.

    For example, when the program asks for a 25pt Courier font on a
    machine that has a non-scalable 24pt Courier font, QFont will
    (normally) use the 24pt Courier for rendering. In this case,
    QFont::pointSize() returns 25 and QFontInfo::pointSize() returns
    24.

    There are three ways to create a QFontInfo object.
    \list 1
    \i Calling the QFontInfo constructor with a QFont creates a font
    info object for a screen-compatible font, i.e. the font cannot be
    a printer font<sup>*</sup>. If the font is changed later, the font
    info object is \e not updated.

    \i QWidget::fontInfo() returns the font info for a widget's font.
    This is equivalent to calling QFontInfo(widget->font()). If the
    widget's font is changed later, the font info object is \e not
    updated.

    \i QPainter::fontInfo() returns the font info for a painter's
    current font. If the painter's font is changed later, the font
    info object is \e not updated.
    \endlist

    <sup>*</sup> If you use a printer font the values returned may be
    inaccurate. Printer fonts are not always accessible so the nearest
    screen font is used if a printer font is supplied.

    \sa QFont QFontMetrics QFontDatabase
*/

/*!
    Constructs a font info object for \a font.

    The font must be screen-compatible, i.e. a font you use when
    drawing text in \link QWidget widgets\endlink or \link QPixmap
    pixmaps\endlink, not QPicture or QPrinter.

    The font info object holds the information for the font that is
    passed in the constructor at the time it is created, and is not
    updated if the font's attributes are changed later.

    Use QPainter::fontInfo() to get the font info when painting.
    This will give correct results also when painting on paint device
    that is not screen-compatible.
*/
QFontInfo::QFontInfo( const QFont &font )
    : d( font.d ), painter( 0 ), fscript( QFont::NoScript )
{
    d->ref();
}

/*!
    Constructs a font info object for \a font using the specified \a
    script.
*/
QFontInfo::QFontInfo( const QFont &font, QFont::Script script )
    : d( font.d ), painter( 0 ), fscript( script )
{
    d->ref();
}

/*! \internal

  Constructs a font info object from the painter's font \a p.
*/
QFontInfo::QFontInfo( const QPainter *p )
    : painter( 0 ), fscript( QFont::NoScript )
{
    QPainter *painter = (QPainter *) p;

#if defined(CHECK_STATE)
    if ( !painter->isActive() )
	qWarning( "QFontInfo: Get font info between QPainter::begin() "
		  "and QPainter::end()" );
#endif

    painter->setf( QPainter::FontInf );
    if ( painter->testf(QPainter::DirtyFont) )
	painter->updateFont();
    if ( painter->pfont )
	d = painter->pfont->d;
    else
	d = painter->cfont.d;
    d->ref();
}

/*!
    Constructs a copy of \a fi.
*/
QFontInfo::QFontInfo( const QFontInfo &fi )
    : d(fi.d), painter(0), fscript( fi.fscript )
{
    d->ref();
}

/*!
    Destroys the font info object.
*/
QFontInfo::~QFontInfo()
{
    if ( d->deref() )
	delete d;
}

/*!
    Assigns the font info in \a fi.
*/
QFontInfo &QFontInfo::operator=( const QFontInfo &fi )
{
    if ( d != fi.d ) {
	if ( d->deref() )
	    delete d;
	d = fi.d;
	d->ref();
    }
    painter = 0;
    fscript = fi.fscript;
    return *this;
}

/*!
    Returns the family name of the matched window system font.

    \sa QFont::family()
*/
QString QFontInfo::family() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE
    return engine->fontDef.family;
}

/*!
    Returns the point size of the matched window system font.

    \sa QFont::pointSize()
*/
int QFontInfo::pointSize() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE
    return ( engine->fontDef.pointSize + 5 ) / 10;
}

/*!
    Returns the pixel size of the matched window system font.

    \sa QFont::pointSize()
*/
int QFontInfo::pixelSize() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE
    return engine->fontDef.pixelSize;
}

/*!
    Returns the italic value of the matched window system font.

    \sa QFont::italic()
*/
bool QFontInfo::italic() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE
    return engine->fontDef.italic;
}

/*!
    Returns the weight of the matched window system font.

    \sa QFont::weight(), bold()
*/
int QFontInfo::weight() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE
    return engine->fontDef.weight;

}

/*!
    \fn bool QFontInfo::bold() const

    Returns TRUE if weight() would return a value greater than \c
    QFont::Normal; otherwise returns FALSE.

    \sa weight(), QFont::bold()
*/

/*!
    Returns the underline value of the matched window system font.

  \sa QFont::underline()

  \internal

  Here we read the underline flag directly from the QFont.
  This is OK for X11 and for Windows because we always get what we want.
*/
bool QFontInfo::underline() const
{
    return d->underline;
}

/*!
    Returns the overline value of the matched window system font.

    \sa QFont::overline()

    \internal

    Here we read the overline flag directly from the QFont.
    This is OK for X11 and for Windows because we always get what we want.
*/
bool QFontInfo::overline() const
{
    return d->overline;
}

/*!
    Returns the strikeout value of the matched window system font.

  \sa QFont::strikeOut()

  \internal Here we read the strikeOut flag directly from the QFont.
  This is OK for X11 and for Windows because we always get what we want.
*/
bool QFontInfo::strikeOut() const
{
    return d->strikeOut;
}

/*!
    Returns the fixed pitch value of the matched window system font.

    \sa QFont::fixedPitch()
*/
bool QFontInfo::fixedPitch() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE
#ifdef Q_OS_MAC
    if (!engine->fontDef.fixedPitchComputed) {
	QChar ch[2] = { QChar('i'), QChar('m') };
	glyph_t g[2];
	int l = 2;
	advance_t a[2];
	engine->stringToCMap(ch, 2, g, a, &l, FALSE);
	engine->fontDef.fixedPitch = a[0] == a[1];
	engine->fontDef.fixedPitchComputed = TRUE;
    }
#endif
    return engine->fontDef.fixedPitch;
}

/*!
    Returns the style of the matched window system font.

    Currently only returns the style hint set in QFont.

    \sa QFont::styleHint() QFont::StyleHint
*/
QFont::StyleHint QFontInfo::styleHint() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE
    return (QFont::StyleHint) engine->fontDef.styleHint;
}

/*!
    Returns TRUE if the font is a raw mode font; otherwise returns
    FALSE.

    If it is a raw mode font, all other functions in QFontInfo will
    return the same values set in the QFont, regardless of the font
    actually used.

    \sa QFont::rawMode()
*/
bool QFontInfo::rawMode() const
{
    return d->rawMode;
}

/*!
    Returns TRUE if the matched window system font is exactly the same
    as the one specified by the font; otherwise returns FALSE.

    \sa QFont::exactMatch()
*/
bool QFontInfo::exactMatch() const
{
    QFontEngine *engine = d->engineForScript( (QFont::Script) fscript );
#ifdef QT_CHECK_STATE
    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    return d->rawMode ? engine->type() != QFontEngine::Box
			     : d->request == engine->fontDef;
}




// **********************************************************************
// QFontCache
// **********************************************************************

#ifdef QFONTCACHE_DEBUG
// fast timeouts for debugging
static const int fast_timeout =   1000;  // 1s
static const int slow_timeout =   5000;  // 5s
#else
static const int fast_timeout =  10000; // 10s
static const int slow_timeout = 300000; //  5m
#endif // QFONTCACHE_DEBUG

QFontCache *QFontCache::instance = 0;
const uint QFontCache::min_cost = 4*1024; // 4mb

static QSingleCleanupHandler<QFontCache> cleanup_fontcache;


QFontCache::QFontCache()
    : QObject( qApp, "global font cache" ), total_cost( 0 ), max_cost( min_cost ),
      current_timestamp( 0 ), fast( FALSE ), timer_id( -1 )
{
    Q_ASSERT( instance == 0 );
    instance = this;
    cleanup_fontcache.set( &instance );
}

QFontCache::~QFontCache()
{
    {
	EngineDataCache::Iterator it = engineDataCache.begin(),
				 end = engineDataCache.end();
	while ( it != end ) {
	    if ( it.data()->count == 0 )
		delete it.data();
	    else
		FC_DEBUG("QFontCache::~QFontCache: engineData %p still has refcount %d",
			 it.data(), it.data()->count);
	    ++it;
	}
    }
    EngineCache::Iterator it = engineCache.begin(),
			 end = engineCache.end();
    while ( it != end ) {
	if ( it.data().data->count == 0 ) {
	    if ( --it.data().data->cache_count == 0 ) {
		FC_DEBUG("QFontCache::~QFontCache: deleting engine %p key=(%d / %d %d %d %d %d)",
			 it.data().data, it.key().script, it.key().def.pointSize,
			 it.key().def.pixelSize, it.key().def.weight, it.key().def.italic,
			 it.key().def.fixedPitch);

		delete it.data().data;
	    }
	} else {
	    FC_DEBUG("QFontCache::~QFontCache: engine = %p still has refcount %d",
		     it.data().data, it.data().data->count);
	}
	++it;
    }
    instance = 0;
}

#ifdef Q_WS_QWS
void QFontCache::clear()
{
    {
	EngineDataCache::Iterator it = engineDataCache.begin(),
				 end = engineDataCache.end();
	while ( it != end ) {
	    QFontEngineData *data = it.data();
	    if ( data->engine )
		data->engine->deref();
	    data->engine = 0;
	    ++it;
	}
    }

    EngineCache::Iterator it = engineCache.begin(),
			 end = engineCache.end();
    while ( it != end ) {
	if ( it.data().data->count == 0 ) {
	    if ( --it.data().data->cache_count == 0 ) {
		FC_DEBUG("QFontCache::~QFontCache: deleting engine %p key=(%d / %d %d %d %d %d)",
			 it.data().data, it.key().script, it.key().def.pointSize,
			 it.key().def.pixelSize, it.key().def.weight, it.key().def.italic,
			 it.key().def.fixedPitch);
		delete it.data().data;
	    }
	} else {
	    FC_DEBUG("QFontCache::~QFontCache: engine = %p still has refcount %d",
		     it.data().data, it.data().data->count);
	}
	++it;
    }
}
#endif

QFontEngineData *QFontCache::findEngineData( const Key &key ) const
{
    EngineDataCache::ConstIterator it = engineDataCache.find( key ),
				  end = engineDataCache.end();
    if ( it == end ) return 0;

    // found
    return it.data();
}

void QFontCache::insertEngineData( const Key &key, QFontEngineData *engineData )
{
    FC_DEBUG( "QFontCache: inserting new engine data %p", engineData );

    engineDataCache.insert( key, engineData );
    increaseCost( sizeof( QFontEngineData ) );
}

QFontEngine *QFontCache::findEngine( const Key &key )
{
    EngineCache::Iterator it = engineCache.find( key ),
			 end = engineCache.end();
    if ( it == end ) return 0;

    // found... update the hitcount and timestamp
    it.data().hits++;
    it.data().timestamp = ++current_timestamp;

    FC_DEBUG( "QFontCache: found font engine\n"
	    "  %p: timestamp %4u hits %3u ref %2d/%2d, type '%s'",
	    it.data().data, it.data().timestamp, it.data().hits,
	    it.data().data->count, it.data().data->cache_count,
	    it.data().data->name() );

    return it.data().data;
}

void QFontCache::insertEngine( const Key &key, QFontEngine *engine )
{
    FC_DEBUG( "QFontCache: inserting new engine %p", engine );

    Engine data( engine );
    data.timestamp = ++current_timestamp;

    engineCache.insert( key, data );

    // only increase the cost if this is the first time we insert the engine
    if ( engine->cache_count == 0 )
	increaseCost( engine->cache_cost );

    ++engine->cache_count;
}

void QFontCache::increaseCost( uint cost )
{
    cost = ( cost + 512 ) / 1024; // store cost in kb
    cost = cost > 0 ? cost : 1;
    total_cost += cost;

    FC_DEBUG( "  COST: increased %u kb, total_cost %u kb, max_cost %u kb",
	    cost, total_cost, max_cost );

    if ( total_cost > max_cost) {
	max_cost = total_cost;

	if ( timer_id == -1 || ! fast ) {
	    FC_DEBUG( "  TIMER: starting fast timer (%d ms)", fast_timeout );

	    if (timer_id != -1) killTimer( timer_id );
	    timer_id = startTimer( fast_timeout );
	    fast = TRUE;
	}
    }
}

void QFontCache::decreaseCost( uint cost )
{
    cost = ( cost + 512 ) / 1024; // cost is stored in kb
    cost = cost > 0 ? cost : 1;
    Q_ASSERT( cost <= total_cost );
    total_cost -= cost;

    FC_DEBUG( "  COST: decreased %u kb, total_cost %u kb, max_cost %u kb",
	    cost, total_cost, max_cost );
}

#if defined(Q_WS_WIN ) || defined (Q_WS_QWS)
void QFontCache::cleanupPrinterFonts()
{
    FC_DEBUG( "QFontCache::cleanupPrinterFonts" );

    {
	FC_DEBUG( "  CLEAN engine data:" );

	// clean out all unused engine datas
	EngineDataCache::Iterator it = engineDataCache.begin(),
				 end = engineDataCache.end();
	while ( it != end ) {
	    if ( it.key().screen == 0 ) {
		++it;
		continue;
	    }

	    if( it.data()->count > 0 ) {
#ifdef Q_WS_WIN
		for(int i = 0; i < QFont::LastPrivateScript; ++i) {
		    if( it.data()->engines[i] ) {
			it.data()->engines[i]->deref();
			it.data()->engines[i] = 0;
		    }
		}
#else
		if ( it.data()->engine ) {
		    it.data()->engine->deref();
		    it.data()->engine = 0;
		}
#endif
		++it;
	    } else {

		EngineDataCache::Iterator rem = it++;

		decreaseCost( sizeof( QFontEngineData ) );

		FC_DEBUG( "    %p", rem.data() );

		delete rem.data();
		engineDataCache.remove( rem );
	    }
	}
    }

    EngineCache::Iterator it = engineCache.begin(),
			 end = engineCache.end();
    while( it != end ) {
	if ( it.data().data->count > 0 || it.key().screen == 0) {
	    ++it;
	    continue;
	}

	FC_DEBUG( "    %p: timestamp %4u hits %2u ref %2d/%2d, type '%s'",
		  it.data().data, it.data().timestamp, it.data().hits,
		  it.data().data->count, it.data().data->cache_count,
		  it.data().data->name() );

	if ( --it.data().data->cache_count == 0 ) {
	    FC_DEBUG( "    DELETE: last occurence in cache" );

	    decreaseCost( it.data().data->cache_cost );
	    delete it.data().data;
	}

	engineCache.remove( it++ );
    }
}
#endif

void QFontCache::timerEvent( QTimerEvent * )
{
    FC_DEBUG( "QFontCache::timerEvent: performing cache maintenance (timestamp %u)",
	      current_timestamp );

    if ( total_cost <= max_cost && max_cost <= min_cost ) {
	FC_DEBUG( "  cache redused sufficiently, stopping timer" );

	killTimer( timer_id );
	timer_id = -1;
	fast = FALSE;

	return;
    }

    // go through the cache and count up everything in use
    uint in_use_cost = 0;

    {
	FC_DEBUG( "  SWEEP engine data:" );

	// make sure the cost of each engine data is at least 1kb
        const uint engine_data_cost =
	    sizeof( QFontEngineData ) > 1024 ? sizeof( QFontEngineData ) : 1024;

	EngineDataCache::ConstIterator it = engineDataCache.begin(),
	                              end = engineDataCache.end();
	for ( ; it != end; ++it ) {
#ifdef QFONTCACHE_DEBUG
	    FC_DEBUG( "    %p: ref %2d", it.data(), it.data()->count );

#  if defined(Q_WS_X11) || defined(Q_WS_WIN)
	    // print out all engines
	    for ( int i = 0; i < QFont::LastPrivateScript; ++i ) {
		if ( ! it.data()->engines[i] ) continue;
		FC_DEBUG( "      contains %p", it.data()->engines[i] );
	    }
#  endif // Q_WS_X11 || Q_WS_WIN
#endif // QFONTCACHE_DEBUG

	    if ( it.data()->count > 0 )
		in_use_cost += engine_data_cost;
	}
    }

    {
	FC_DEBUG( "  SWEEP engine:" );

	EngineCache::ConstIterator it = engineCache.begin(),
				  end = engineCache.end();
	for ( ; it != end; ++it ) {
	    FC_DEBUG( "    %p: timestamp %4u hits %2u ref %2d/%2d, cost %u bytes",
		      it.data().data, it.data().timestamp, it.data().hits,
		      it.data().data->count, it.data().data->cache_count,
		      it.data().data->cache_cost );

	    if ( it.data().data->count > 0 )
		in_use_cost += it.data().data->cache_cost / it.data().data->cache_count;
	}

	// attempt to make up for rounding errors
	in_use_cost += (uint)engineCache.count();
    }

    in_use_cost = ( in_use_cost + 512 ) / 1024; // cost is stored in kb

    /*
      calculate the new maximum cost for the cache

      NOTE: in_use_cost is *not* correct due to rounding errors in the
      above algorithm.  instead of worrying about getting the
      calculation correct, we are more interested in speed, and use
      in_use_cost as a floor for new_max_cost
    */
    uint new_max_cost = QMAX( QMAX( max_cost / 2, in_use_cost ), min_cost );

    FC_DEBUG( "  after sweep, in use %u kb, total %u kb, max %u kb, new max %u kb",
	      in_use_cost, total_cost, max_cost, new_max_cost );

    if ( new_max_cost == max_cost ) {
	if ( fast ) {
	    FC_DEBUG( "  cannot shrink cache, slowing timer" );

	    killTimer( timer_id );
	    timer_id = startTimer( slow_timeout );
	    fast = FALSE;
	}

	return;
    } else if ( ! fast ) {
	FC_DEBUG( "  dropping into passing gear" );

	killTimer( timer_id );
	timer_id = startTimer( fast_timeout );
	fast = TRUE;
    }

    max_cost = new_max_cost;

    {
	FC_DEBUG( "  CLEAN engine data:" );

	// clean out all unused engine datas
	EngineDataCache::Iterator it = engineDataCache.begin(),
				 end = engineDataCache.end();
	while ( it != end ) {
	    if ( it.data()->count > 0 ) {
		++it;
		continue;
	    }

	    EngineDataCache::Iterator rem = it++;

	    decreaseCost( sizeof( QFontEngineData ) );

	    FC_DEBUG( "    %p", rem.data() );

	    delete rem.data();
	    engineDataCache.remove( rem );
	}
    }

    // clean out the engine cache just enough to get below our new max cost
    uint current_cost;
    do {
	current_cost = total_cost;

	EngineCache::Iterator it = engineCache.begin(),
			     end = engineCache.end();
	// determine the oldest and least popular of the unused engines
	uint oldest = ~0;
	uint least_popular = ~0;

	for ( ; it != end; ++it ) {
	    if ( it.data().data->count > 0 ) continue;

	    if ( it.data().timestamp < oldest &&
		 it.data().hits <= least_popular ) {
		oldest = it.data().timestamp;
		least_popular = it.data().hits;
	    }
	}

	FC_DEBUG( "    oldest %u least popular %u", oldest, least_popular );

	for ( it = engineCache.begin(); it != end; ++it ) {
	    if ( it.data().data->count == 0 &&
		 it.data().timestamp == oldest &&
		 it.data().hits == least_popular)
		break;
	}

	if ( it != end ) {
	    FC_DEBUG( "    %p: timestamp %4u hits %2u ref %2d/%2d, type '%s'",
		      it.data().data, it.data().timestamp, it.data().hits,
		      it.data().data->count, it.data().data->cache_count,
		      it.data().data->name() );

	    if ( --it.data().data->cache_count == 0 ) {
		FC_DEBUG( "    DELETE: last occurence in cache" );

		decreaseCost( it.data().data->cache_cost );
		delete it.data().data;
	    } else {
		/*
		  this particular font engine is in the cache multiple
		  times...  set current_cost to zero, so that we can
		  keep looping to get rid of all occurences
		*/
		current_cost = 0;
	    }

	    engineCache.remove( it );
	}
    } while ( current_cost != total_cost && total_cost > max_cost );
}
