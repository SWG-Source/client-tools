/****************************************************************************
** $Id: qfont_win.cpp 2052 2007-02-22 18:58:41Z chehrlic $
**
** Implementation of QFont, QFontMetrics and QFontInfo classes for Windows
**
** Created : 20030119
**
** Copyright (C) 2003 Holger Schroeder
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2003 Ivan de Jesus Deras Tabora
** Copyright (C) 2004 Tom and Timi Cecka
** Copyright (C) 2004 Ralf Habacker
** Copyright (C) 2004 Andreas Hausladen
** Copyright (C) 2004 Peter Kuemmel
** Copyright (C) 2004 Christian Ehrlicher
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

#define QT_FATAL_ASSERT

// REVISED: brad

#include "qplatformdefs.h"

#include "qfont.h"
#include "qapplication.h"
#include "qcleanuphandler.h"
#include "qfontinfo.h"
#include "qfontdatabase.h"
#include "qfontmetrics.h"
#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qtextcodec.h"

#include <private/qfontcodecs_p.h>
#include <private/qunicodetables_p.h>
#include "qfontdata_p.h"
#include "qfontengine_p.h"
#include "qtextengine_p.h"

#include "qt_windows.h"

#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>



#define QFONTLOADER_DEBUG
#define QFONTLOADER_DEBUG_VERBOSE

Q_EXPORT bool qt_has_xft = FALSE;
bool qt_use_antialiasing = FALSE;

#define QT_CHECK_STATE

extern HDC shared_dc;                // common dc for all fonts

// ### maybe move to qapplication_win
QFont qt_LOGFONTtoQFont( LOGFONT& lf, bool /*scale*/ )
{
    QString family = QT_WA_INLINE( QString::fromUcs2( ( ushort* ) lf.lfFaceName ),
                                   QString::fromLocal8Bit( ( char* ) lf.lfFaceName ) );
    QFont qf( family );
    qf.setItalic( lf.lfItalic );
    if ( lf.lfWeight != FW_DONTCARE ) {
        int weight;
        if ( lf.lfWeight < 400 )
            weight = QFont::Light;
        else if ( lf.lfWeight < 600 )
            weight = QFont::Normal;
        else if ( lf.lfWeight < 700 )
            weight = QFont::DemiBold;
        else if ( lf.lfWeight < 800 )
            weight = QFont::Bold;
        else
            weight = QFont::Black;
        qf.setWeight( weight );
    }
    int lfh = QABS( lf.lfHeight );
    Q_ASSERT( shared_dc );
    qf.setPointSize( lfh * 72.0 / GetDeviceCaps( shared_dc, LOGPIXELSY ) );
    qf.setUnderline( false );
    qf.setOverline( false );
    qf.setStrikeOut( false );
    return qf;
}

static inline double pixelSize( const QFontDef &request, QPaintDevice *paintdevice )
{
    double pSize;
    int dpi = GetDeviceCaps( qt_display_dc(), LOGPIXELSY );
    if ( request.pointSize != -1 )
        pSize = request.pointSize * dpi / 72.;
    else
        pSize = request.pixelSize;
    return pSize / 10;
}

static inline double pointSize( const QFontDef &fd, QPaintDevice *paintdevice )
{
    double pSize;
    int dpi = GetDeviceCaps( qt_display_dc(), LOGPIXELSY );
    if ( fd.pointSize < 0 )
        pSize = fd.pixelSize * 72. / ( ( float ) dpi );
    else
        pSize = fd.pointSize;
    return pSize / 10;
}


typedef QMap<QFont::Script, QString> FallbackMap;
static FallbackMap *fallbackMap = 0;
static QSingleCleanupHandler<FallbackMap> qt_fallback_font_family_cleanup;

static void ensure_fallback_map()
{
    if ( fallbackMap )
        return ;
    fallbackMap = new FallbackMap;
    qt_fallback_font_family_cleanup.set( &fallbackMap );
}

//333
// Returns the user-configured fallback family for the specified script.
QString qt_fallback_font_family( QFont::Script script )
{
    QString ret;

    if ( fallbackMap ) {
        FallbackMap::ConstIterator it, end = fallbackMap->end();
        it = fallbackMap->find( script );
        if ( it != end )
            ret = it.data();
    }

    return ret;
}


QFont::Script QFontPrivate::defaultScript = QFont::UnknownScript;
//int QFontPrivate::defaultEncodingID = -1;

/*!
  Internal function that initializes the font system.

  \internal
  The font cache and font dict do not alloc the keys. The key is a QString
  which is shared between QFontPrivate and QXFontName.
*/
void QFont::initialize()
{
    if ( QFontCache::instance )
        return ;
    shared_dc = CreateCompatibleDC( qt_display_dc() );
    if ( !shared_dc )
        qWarning( "QFont::initialize: CreateCompatibleDC failed" );
    new QFontCache();

    // get some sample text based on the users locale. we use this to determine the
    // default script for the font system
    QTextCodec *codec = QTextCodec::codecForLocale();
    QCString oldlctime = setlocale( LC_TIME, 0 );
    QCString lctime = setlocale( LC_TIME, "" );

    time_t ttmp = time( 0 );
    struct tm *tt = 0;
    char samp[ 64 ];
    QString sample;

    if ( ttmp != -1 ) {
#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
        // use the reentrant versions of localtime() where available
        tm res;
        tt = localtime_r( &ttmp, &res );
#else

        tt = localtime( &ttmp );
#endif // QT_THREAD_SUPPORT && _POSIX_THREAD_SAFE_FUNCTIONS

        if ( tt != 0 && strftime( samp, 64, "%A%B", tt ) > 0 )
            if ( codec )
                sample = codec->toUnicode( samp );
    }

    if ( ! sample.isNull() && ! sample.isEmpty() ) {
        QFont::Script cs = QFont::NoScript, tmp;
        const QChar *uc = sample.unicode();
        QFontPrivate *priv = new QFontPrivate;

        for ( uint i = 0; i < sample.length(); i++ ) {
            SCRIPT_FOR_CHAR( tmp, *uc );
            uc++;
            if ( tmp != cs && tmp != QFont::UnknownScript ) {
                cs = tmp;
                break;
            }
        }
        delete priv;

        if ( cs != QFont::UnknownScript )
            QFontPrivate::defaultScript = cs;
    }

    setlocale( LC_TIME, oldlctime.data() );
    
    qAddPostRoutine( QFont::cleanup );
}

/*! \internal

  Internal function that cleans up the font system.
*/
void QFont::cleanup()
{
    delete QFontCache::instance;
    DeleteDC( shared_dc );
    shared_dc = 0;
}

/*! \internal
    Returns a QFontEngine for the specified \a script that matches the
    QFontDef \e request member variable.
*/
void QFontPrivate::load( QFont::Script script )
{
    // NOTE: the X11 and Windows implementations of this function are
    // identical... if you change one, change both.

#ifdef QT_CHECK_STATE
    // sanity checks
    if ( !QFontCache::instance )
        qWarning( "Must construct a QApplication before a QFont" );
    Q_ASSERT( script >= 0 && script < QFont::LastPrivateScript );
#endif // QT_CHECK_STATE

    QFontDef req = request;
    req.pixelSize = qRound( pixelSize( req, paintdevice ) );

    // set the point size to 0 to get better caching
    req.pointSize = 0;

    if ( ! engineData ) {
        QFontCache::Key key( req, QFont::NoScript, screen, paintdevice );

        // look for the requested font in the engine data cache
        engineData = QFontCache::instance->findEngineData( key );

        if ( ! engineData ) {
            // create a new one
            engineData = new QFontEngineData;
            QFontCache::instance->insertEngineData( key, engineData );
        } else {
            engineData->ref();
        }
    }

    // set it to the actual pointsize, so QFontInfo will do the right thing
    req.pointSize = qRound( pointSize( request, paintdevice ) );

    // the cached engineData could have already loaded the engine we want
    if ( engineData->engines[ script ] )
        return ;

    // load the font
    QFontEngine *engine = 0;
    //    double scale = 1.0; // ### TODO: fix the scale calculations

    // list of families to try
    QStringList family_list;

    if ( !req.family.isEmpty() ) {
        family_list = QStringList::split( ',', req.family );

        // append the substitute list for each family in family_list
        QStringList subs_list;
        QStringList::ConstIterator it = family_list.begin(), end = family_list.end();
        for ( ; it != end; ++it )
            subs_list += QFont::substitutes( *it );
        family_list += subs_list;

#ifndef QT_XFT2
        // with Xft2, we want to use fontconfig to determine better fallbacks,
        // otherwise we might run into trouble with default fonts as "serif"

        // append the default fallback font for the specified script
        extern QString qt_fallback_font_family( QFont::Script );
        QString fallback = qt_fallback_font_family( script );
        if ( ! fallback.isEmpty() && ! family_list.contains( fallback ) )
            family_list << fallback;

        // add the default family
        QString defaultFamily = QApplication::font().family();
        if ( ! family_list.contains( defaultFamily ) )
            family_list << defaultFamily;

        // add QFont::defaultFamily() to the list, for compatibility with
        // previous versions
        family_list << QApplication::font().defaultFamily();
#endif // QT_XFT2

    }

    // null family means find the first font matching the specified script
    family_list << QString::null;

    QStringList::ConstIterator it = family_list.begin(), end = family_list.end();
    for ( ; ! engine && it != end; ++it ) {
        req.family = *it;
        //qDebug("calling findFont with %s for %d",req.family.latin1(),script);
        engine = QFontDatabase::findFont( script, this, req );
        if ( engine ) {
            if ( engine->type() != QFontEngine::Box )
                break;

            if ( ! req.family.isEmpty() )
                engine = 0;

            continue;
        }
    }

    engine->ref();
    engineData->engines[ script ] = engine;
}


/*!
    Returns TRUE if the font attributes have been changed and the font
    has to be (re)loaded; otherwise returns FALSE.
*/
bool QFont::dirty() const
{
    return d->engineData == 0;
}

/*!
    Returns the window system handle to the font, for low-level
    access. Using this function is \e not portable.
*/
HFONT QFont::handle() const
{
    QFontEngine * engine = d->engineForScript( QFontPrivate::defaultScript );
#ifdef QT_CHECK_STATE

    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    switch ( engine->type() ) {
    case QFontEngine::Win:
        return ( ( QFontEngineWin * ) engine ) ->hfont;

    default:
        break;
    }
    return 0;
}

QString QFont::rawName() const
{
    return family();
}

void QFont::setRawName( const QString &name )
{
    setFamily( name );
}

QString QFont::defaultFamily() const
{
    switch ( d->request.styleHint ) {
    case QFont::Times:
        return QString::fromLatin1( "Times New Roman" );
    case QFont::Courier:
        return QString::fromLatin1( "Courier New" );
    case QFont::Decorative:
        return QString::fromLatin1( "Bookman Old Style" );
    case QFont::Helvetica:
        return QString::fromLatin1( "Arial" );
    case QFont::System:
    default:
        return QString::fromLatin1( "MS Sans Serif" );
    }
}

QString QFont::lastResortFamily() const
{
    return QString::fromLatin1( "helvetica" );
}

QString QFont::lastResortFont() const
{
    return QString::fromLatin1( "arial" );
}


// **********************************************************************
// QFontMetrics member methods
// **********************************************************************
int QFontMetrics::width( QChar ch ) const
{
    unsigned short uc = ch.unicode();
    if ( uc < QFontEngineData::widthCacheSize &&
            d->engineData && d->engineData->widthCache[ uc ] )
        return d->engineData->widthCache[ uc ];

    if ( ::category( ch ) == QChar::Mark_NonSpacing )
        return 0;

    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch );

    QFontEngine *engine = d->engineForScript( script );
#ifdef QT_CHECK_STATE

    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    glyph_t glyphs[ 8 ];
    advance_t advances[ 8 ];
    int nglyphs = 7;
    engine->stringToCMap( &ch, 1, glyphs, advances, &nglyphs, FALSE );

    // ### can nglyphs != 1 happen at all? Not currently I think
    if ( uc < QFontEngineData::widthCacheSize && advances[ 0 ] > 0 && advances[ 0 ] < 0x100 )
        d->engineData->widthCache[ uc ] = advances[ 0 ];

    return advances[ 0 ];
}

int QFontMetrics::charWidth( const QString &str, int pos ) const
{
    if ( pos < 0 || pos > ( int ) str.length() )
        return 0;

    const QChar &ch = str.unicode() [ pos ];
    if ( ch.unicode() < QFontEngineData::widthCacheSize &&
            d->engineData && d->engineData->widthCache[ ch.unicode() ] )
        return d->engineData->widthCache[ ch.unicode() ];

    if ( ::category( ch ) == QChar::Mark_NonSpacing )
        return 0;

    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch );

    int width;

    if ( script >= QFont::Arabic && script <= QFont::Khmer ) {
        // complex script shaping. Have to do some hard work
        int from = QMAX( 0, pos - 8 );
        int to = QMIN( ( int ) str.length(), pos + 8 );
        QConstString cstr( str.unicode() + from, to - from );
        QTextEngine layout( cstr.string(), d );
        layout.itemize( QTextEngine::WidthOnly );
        width = layout.width( pos - from, 1 );
    } else {
        QFontEngine *engine = d->engineForScript( script );
#ifdef QT_CHECK_STATE

        Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

        glyph_t glyphs[ 8 ];
        advance_t advances[ 8 ];
        int nglyphs = 7;
        engine->stringToCMap( &ch, 1, glyphs, advances, &nglyphs, FALSE );
        width = advances[ 0 ];
    }
    if ( ch.unicode() < QFontEngineData::widthCacheSize && width > 0 && width < 0x100 )
        d->engineData->widthCache[ ch.unicode() ] = width;
    return width;
}

// from qfont but excudet for Q_WS_WIN

int QFontMetrics::leftBearing( QChar ch ) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch );

    QFontEngine *engine = d->engineForScript( script );
#ifdef QT_CHECK_STATE

    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    if ( engine->type() == QFontEngine::Box )
        return 0;

    glyph_t glyphs[ 10 ];
    int nglyphs = 9;
    engine->stringToCMap( &ch, 1, glyphs, 0, &nglyphs, FALSE );
    // ### can nglyphs != 1 happen at all? Not currently I think
    glyph_metrics_t gi = engine->boundingBox( glyphs[ 0 ] );
    return gi.x;
}

int QFontMetrics::rightBearing( QChar ch ) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR( script, ch );

    QFontEngine *engine = d->engineForScript( script );
#ifdef QT_CHECK_STATE

    Q_ASSERT( engine != 0 );
#endif // QT_CHECK_STATE

    if ( engine->type() == QFontEngine::Box )
        return 0;

    glyph_t glyphs[ 10 ];
    int nglyphs = 9;
    engine->stringToCMap( &ch, 1, glyphs, 0, &nglyphs, FALSE );
    // ### can nglyphs != 1 happen at all? Not currently I think
    glyph_metrics_t gi = engine->boundingBox( glyphs[ 0 ] );
    return gi.xoff - gi.x - gi.width;
}
