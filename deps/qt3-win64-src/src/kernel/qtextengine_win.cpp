/****************************************************************************
** $Id: qtextengine_win.cpp 2068 2007-03-10 20:39:49Z chehrlic $
**
** Text engine classes
**
** Created : 20040828
**
** Copyright (C) 1992-2006 Trolltech ASA. All rights reserved.
** Copyright (C) 2004 Peter Kuemmel
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

#ifdef QT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // QT_THREAD_SUPPORT

#include <qlibrary.h>
#include <qmutex.h>

extern Qt::WindowsVersion qt_winver;

QScriptItemArray::~QScriptItemArray()
{
    clear();
    free( d );
}

void QScriptItemArray::clear()
{
    if ( d ) {
        for ( unsigned int i = 0; i < d->size; i++ ) {
            QScriptItem &si = d->items[ i ];
            if ( si.fontEngine )
                si.fontEngine->deref();
        }
        d->size = 0;
    }
}

void QScriptItemArray::resize( int s )
{
    int alloc = ( ( s + 8 ) >> 3 ) << 3;
    d = ( QScriptItemArrayPrivate * ) realloc( d, sizeof( QScriptItemArrayPrivate ) +
            sizeof( QScriptItem ) * alloc );
    d->alloc = alloc;
}

// these defines are from usp10.h
typedef void *SCRIPT_CACHE;
typedef struct tag_SCRIPT_CONTROL
{
    DWORD uDefaultLanguage : 16;
    DWORD fContextDigits : 1;
    DWORD fInvertPreBoundDir : 1;
    DWORD fInvertPostBoundDir : 1;
    DWORD fLinkStringBefore : 1;
    DWORD fLinkStringAfter : 1;
    DWORD fNeutralOverride : 1;
    DWORD fNumericOverride : 1;
    DWORD fLegacyBidiClass : 1;
    DWORD fReserved : 8;
}
SCRIPT_CONTROL;

typedef struct tag_SCRIPT_STATE
{
    WORD uBidiLevel : 5;
    WORD fOverrideDirection : 1;
    WORD fInhibitSymSwap : 1;
    WORD fCharShape : 1;
    WORD fDigitSubstitute : 1;
    WORD fInhibitLigate : 1;
    WORD fDisplayZWG : 1;
    WORD fArabicNumContext : 1;
    WORD fGcpClusters : 1;
    WORD fReserved : 1;
    WORD fEngineReserved : 2;
}
SCRIPT_STATE;

typedef struct tag_SCRIPT_ITEM
{
    int iCharPos;
    QScriptAnalysis a;
}
SCRIPT_ITEM;

#define USP_E_SCRIPT_NOT_IN_FONT   \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_ITF,0x200)    // Script doesn't exist in font

typedef struct
{
    DWORD langid : 16;
    DWORD fNumeric : 1;
    DWORD fComplex : 1;
    DWORD fNeedsWordBreaking : 1;
    DWORD fNeedsCaretInfo : 1;
    DWORD bCharSet : 8;
    DWORD fControl : 1;
    DWORD fPrivateUseArea : 1;
    DWORD fNeedsCharacterJustify : 1;
    DWORD fInvalidGlyph : 1;
    DWORD fInvalidLogAttr : 1;
    DWORD fCDM : 1;
    DWORD fAmbiguousCharSet : 1;
    DWORD fClusterSizeVaries : 1;
    DWORD fRejectInvalid : 1;
}
SCRIPT_PROPERTIES;

#if defined(Q_OS_TEMP) && UNDER_CE < 400
typedef struct _ABC
{
    int abcA;
    UINT abcB;
    int abcC;
}
ABC;
#endif

// GlyphAttributes == SCRIPT_VISATTR
// qoffset_t = GOFFSET
typedef HRESULT ( WINAPI *fScriptFreeCache ) ( SCRIPT_CACHE * );
typedef HRESULT ( WINAPI *fScriptItemize ) ( const WCHAR *, int, int, const SCRIPT_CONTROL *,
        const SCRIPT_STATE *, SCRIPT_ITEM *, int * );
typedef HRESULT ( WINAPI *fScriptShape ) ( HDC hdc, SCRIPT_CACHE *, const WCHAR *, int, int,
        QScriptAnalysis *, WORD *, WORD *, GlyphAttributes *, int * );
typedef HRESULT ( WINAPI *fScriptPlace ) ( HDC, SCRIPT_CACHE *, const WORD *, int, const GlyphAttributes *, QScriptAnalysis *, int *,
        qoffset_t *, ABC * );
typedef HRESULT ( WINAPI *fScriptTextOut ) ( const HDC, SCRIPT_CACHE *, int, int, UINT, const RECT *, const QScriptAnalysis *,
        const WCHAR *, int, const WORD *, int, const int *, const int *, const qoffset_t * );
typedef HRESULT ( WINAPI *fScriptBreak ) ( const WCHAR *, int, const QScriptAnalysis *, QCharAttributes * );
//typedef HRESULT (WINAPI *fScriptGetFontProperties)(HDC, SCRIPT_CACHE *, SCRIPT_FONTPROPERTIES *);
typedef HRESULT ( WINAPI *fScriptGetProperties ) ( const SCRIPT_PROPERTIES ***, int * );

fScriptFreeCache ScriptFreeCache = 0;
static fScriptItemize ScriptItemize = 0;
static fScriptShape ScriptShape = 0;
static fScriptPlace ScriptPlace = 0;
fScriptTextOut ScriptTextOut = 0;
static fScriptBreak ScriptBreak = 0;
//static fScriptGetFontProperties ScriptGetFontProperties = 0;
static fScriptGetProperties ScriptGetProperties = 0;

static bool resolvedUsp10 = false;
bool hasUsp10 = false;

const SCRIPT_PROPERTIES **script_properties = 0;
int num_scripts = 0;
int usp_latin_script = 0;

static void uspAppendItems( QTextEngine *engine, int &start, int &stop, BidiControl &control, QChar::Direction dir );

static void resolveUsp10()
{
#ifndef QT_NO_LIBRARY
    if ( !resolvedUsp10 ) {
        // need to resolve the security info functions

#ifndef QT_NO_THREAD
        // protect initialization
        QMutexLocker locker( qt_global_mutexpool ?
                             qt_global_mutexpool->get
                             ( ( void* ) & resolveUsp10 ) : 0 );
        // check triedResolve again, since another thread may have already
        // done the initialization
        if ( resolvedUsp10 ) {
            // another thread did initialize the security function pointers,
            // so we shouldn't do it again.
            return ;
        }
#endif

        resolvedUsp10 = true;
        QLibrary lib( "usp10" );
        lib.setAutoUnload ( false );      // Qt4 doesn't need this...

        hasUsp10 = false;

        ScriptFreeCache = ( fScriptFreeCache ) lib.resolve( "ScriptFreeCache" );
        ScriptItemize = ( fScriptItemize ) lib.resolve( "ScriptItemize" );
        ScriptShape = ( fScriptShape ) lib.resolve( "ScriptShape" );
        ScriptPlace = ( fScriptPlace ) lib.resolve( "ScriptPlace" );
        ScriptTextOut = ( fScriptTextOut ) lib.resolve( "ScriptTextOut" );
        ScriptBreak = ( fScriptBreak ) lib.resolve( "ScriptBreak" );
        ScriptGetProperties = ( fScriptGetProperties ) lib.resolve( "ScriptGetProperties" );

        if ( !ScriptFreeCache )
            return ;

        // ### Disable uniscript for windows 9x for now
        if ( qt_winver & Qt::WV_DOS_based )
            return ;


        hasUsp10 = true;
        ScriptGetProperties( &script_properties, &num_scripts );

        // get the usp script for western
        for ( int i = 0; i < num_scripts; i++ ) {
            if ( script_properties[ i ] ->langid == LANG_ENGLISH &&
                    !script_properties[ i ] ->fAmbiguousCharSet ) {
                usp_latin_script = i;
                break;
            }
        }

        appendItems = uspAppendItems;
    }
#endif
}

static unsigned char script_for_win_language[ 0x80 ] = {
            //0x00 LANG_NEUTRAL Neutral
            QFont::Latin,
            //0x01 LANG_ARABIC Arabic
            QFont::Arabic,
            //0x02 LANG_BULGARIAN Bulgarian
            QFont::Latin,
            //0x03 LANG_CATALAN Catalan
            QFont::Latin,
            //0x04 LANG_CHINESE Chinese
            QFont::Han,
            //0x05 LANG_CZECH Czech
            QFont::Latin,
            //0x06 LANG_DANISH Danish
            QFont::Latin,
            //0x07 LANG_GERMAN German
            QFont::Latin,
            //0x08 LANG_GREEK Greek
            QFont::Greek,
            //0x09 LANG_ENGLISH English
            QFont::Latin,
            //0x0a LANG_SPANISH Spanish
            QFont::Latin,
            //0x0b LANG_FINNISH Finnish
            QFont::Latin,
            //0x0c LANG_FRENCH French
            QFont::Latin,
            //0x0d LANG_HEBREW Hebrew
            QFont::Hebrew,
            //0x0e LANG_HUNGARIAN Hungarian
            QFont::Latin,
            //0x0f LANG_ICELANDIC Icelandic
            QFont::Latin,

            //0x10 LANG_ITALIAN Italian
            QFont::Latin,
            //0x11 LANG_JAPANESE Japanese
            QFont::Hiragana,
            //0x12 LANG_KOREAN Korean
            QFont::Hangul,
            //0x13 LANG_DUTCH Dutch
            QFont::Latin,
            //0x14 LANG_NORWEGIAN Norwegian
            QFont::Latin,
            //0x15 LANG_POLISH Polish
            QFont::Latin,
            //0x16 LANG_PORTUGUESE Portuguese
            QFont::Latin,
            QFont::Latin,
            //0x18 LANG_ROMANIAN Romanian
            QFont::Latin,
            //0x19 LANG_RUSSIAN Russian
            QFont::Cyrillic,
            //0x1a LANG_CROATIAN Croatian
            //0x1a LANG_SERBIAN Serbian
            QFont::Latin,
            //0x1b LANG_SLOVAK Slovak
            QFont::Latin,
            //0x1c LANG_ALBANIAN Albanian
            QFont::Latin,
            //0x1d LANG_SWEDISH Swedish
            QFont::Latin,
            //0x1e LANG_THAI Thai
            QFont::Thai,
            //0x1f LANG_TURKISH Turkish
            QFont::Latin,

            //0x20 LANG_URDU Urdu
            QFont::Latin,
            //0x21 LANG_INDONESIAN Indonesian
            QFont::Latin,
            //0x22 LANG_UKRAINIAN Ukrainian
            QFont::Latin,
            //0x23 LANG_BELARUSIAN Belarusian
            QFont::Latin,
            //0x24 LANG_SLOVENIAN Slovenian
            QFont::Latin,
            //0x25 LANG_ESTONIAN Estonian
            QFont::Latin,
            //0x26 LANG_LATVIAN Latvian
            QFont::Latin,
            //0x27 LANG_LITHUANIAN Lithuanian
            QFont::Latin,
            QFont::Latin,
            //0x29 LANG_FARSI Farsi
            QFont::Latin,
            //0x2a LANG_VIETNAMESE Vietnamese
            QFont::Latin,  // ##### maybe use QFont::CombiningMarks instead?
            //0x2b LANG_ARMENIAN Armenian
            QFont::Armenian,
            //0x2c LANG_AZERI Azeri
            QFont::Latin,
            //0x2d LANG_BASQUE Basque
            QFont::Latin,
            QFont::Latin,
            //0x2f LANG_MACEDONIAN FYRO Macedonian
            QFont::Latin,

            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            //0x36 LANG_AFRIKAANS Afrikaans
            QFont::Latin,
            //0x37 LANG_GEORGIAN Georgian
            QFont::Latin,
            //0x38 LANG_FAEROESE Faeroese
            QFont::Latin,
            //0x39 LANG_HINDI Hindi
            QFont::Devanagari,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            //0x3e LANG_MALAY Malay
            QFont::Latin,
            //0x3f LANG_KAZAK Kazak
            QFont::Latin,

            //0x40 LANG_KYRGYZ Kyrgyz
            QFont::Latin,
            //0x41 LANG_SWAHILI Swahili
            QFont::Latin,
            QFont::Latin,
            //0x43 LANG_UZBEK Uzbek
            QFont::Latin,
            //0x44 LANG_TATAR Tatar
            QFont::Latin,
            //0x45 LANG_BENGALI Not supported.
            QFont::Bengali,
            //0x46 LANG_PUNJABI Punjabi
            QFont::Gurmukhi,
            //0x47 LANG_GUJARATI Gujarati
            QFont::Gujarati,
            //0x48 LANG_ORIYA Not supported.
            QFont::Oriya,
            //0x49 LANG_TAMIL Tamil
            QFont::Tamil,
            //0x4a LANG_TELUGU Telugu
            QFont::Telugu,
            //0x4b LANG_KANNADA Kannada
            QFont::Kannada,
            //0x4c LANG_MALAYALAM Not supported.
            QFont::Malayalam,
            //0x4d LANG_ASSAMESE Not supported.
            QFont::Latin,
            //0x4e LANG_MARATHI Marathi
            QFont::Latin,
            //0x4f LANG_SANSKRIT Sanskrit
            QFont::Devanagari,

            //0x50 LANG_MONGOLIAN Mongolian
            QFont::Mongolian,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            //0x56 LANG_GALICIAN Galician
            QFont::Latin,
            //0x57 LANG_KONKANI Konkani
            QFont::Latin,
            //0x58 LANG_MANIPURI Not supported.
            QFont::Latin,
            //0x59 LANG_SINDHI Not supported.
            QFont::Latin,
            //0x5a LANG_SYRIAC Syriac
            QFont::Syriac,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,

            //0x60 LANG_KASHMIRI Not supported.
            QFont::Latin,
            //0x61 LANG_NEPALI Not supported.
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            //0x65 LANG_DIVEHI Divehi
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,

            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            QFont::Latin,
            //0x7f LANG_INVARIANT
            QFont::Latin,
        };

static inline QFont::Script scriptForWinLanguage( DWORD langid )
{
    QFont::Script script = langid < 0x80 ? ( QFont::Script ) script_for_win_language[ langid ] : QFont::Latin;
    // if (script == QFont::Latin)
    //     qWarning("QTextEngine: Uniscribe support internal error: Encountered unhandled language %x", (unsigned int)langid);
    return script;
}

// we're not using Uniscribe's BiDi algorithm, since it is (a) not 100% Unicode compliant and
// (b) seems to work wrongly when trying to use it with a base level != 0.
//
// This function does uses Uniscribe to do the script analysis and creates items from this.
static void uspAppendItems( QTextEngine *engine, int &start, int &stop, BidiControl &control, QChar::Direction dir )
{
    QScriptItemArray & items = engine->items;
    const QChar *text = engine->string.unicode();

    if ( start > stop ) {
        // #### the algorithm is currently not really safe against this. Still needs fixing.
        //         qWarning("QTextEngine: BiDi internal error in uspAppendItems()");
        return ;
    }

    int level = control.level();

    if ( dir != QChar::DirON ) {
        // add level of run (cases I1 & I2)
        if ( level % 2 ) {
            if ( dir == QChar::DirL || dir == QChar::DirAN || dir == QChar::DirEN )
                level++;
        } else {
            if ( dir == QChar::DirR )
                level++;
            else if ( dir == QChar::DirAN || dir == QChar::DirEN )
                level += 2;
        }
    }

    SCRIPT_ITEM s_items[ 256 ];
    SCRIPT_ITEM *usp_items = s_items;

    int numItems;
    HRESULT res = ScriptItemize( ( WCHAR * ) ( text + start ), stop - start + 1, 255, 0, 0, usp_items, &numItems );

    if ( res == E_OUTOFMEMORY ) {
        int alloc = 256;
        usp_items = 0;
        while ( res == E_OUTOFMEMORY ) {
            alloc *= 2;
            usp_items = ( SCRIPT_ITEM * ) realloc( usp_items, alloc * sizeof( SCRIPT_ITEM ) );
            res = ScriptItemize( ( WCHAR * ) ( text + start ), stop - start + 1, alloc - 1, 0, 0, usp_items, &numItems );
        }
    }
    int i;
    for ( i = 0; i < numItems; i++ ) {
        QScriptItem item;
        item.analysis = usp_items[ i ].a;
        item.position = usp_items[ i ].iCharPos + start;
        item.analysis.bidiLevel = level;
        item.analysis.override = control.override();

        int rstart = usp_items[ i ].iCharPos;
        int rstop = usp_items[ i + 1 ].iCharPos - 1;
        bool b = true;
        for ( int j = rstart; j <= rstop; j++ ) {

            unsigned short uc = text[ j + start ].unicode();
            //            if (uc == QChar::ObjectReplacementCharacter || uc == QChar::LineSeparator) {
            if ( uc == 0xfffc || uc == 0x2028 ) {
                item.analysis.script = usp_latin_script;
                item.isObject = true;
                b = true;
            } else if ( uc == 9 ) {
                item.analysis.script = usp_latin_script;
                item.isSpace = true;
                item.isTab = true;
                item.analysis.bidiLevel = control.baseLevel();
                b = true;
            } else if ( b ) {
                b = false;
            } else {
                if ( j - rstart < 32000 )
                    continue;
                rstart = j;
            }

            item.position = j + start;
            items.append( item );
            item.analysis = usp_items[ i ].a;
            item.analysis.bidiLevel = level;
            item.analysis.override = control.override();
            item.isSpace = item.isTab = item.isObject = false;
        }
    }

    if ( usp_items != s_items )
        free( usp_items );

    ++stop;
    start = stop;
}

static QFont::Script uspScriptForItem( const QTextEngine *te, int item )
{
    QScriptItem & si = te->items[ item ];
    if ( hasUsp10 ) {
        const SCRIPT_PROPERTIES * script_prop = script_properties[ si.analysis.script ];
        return scriptForWinLanguage( script_prop->langid );
    }
    return ( QFont::Script ) si.analysis.script;
}

// QTextEngine
extern const q_scriptEngine *qscriptEngines;

void QTextEngine::shape( int item ) const
{
    assert( item < items.size() );
    QScriptItem &si = items[ item ];

    if ( si.num_glyphs )
        return ;

    QFont::Script script = uspScriptForItem( this, item );

    // Just to get the warning away
    int from = si.position;
    const int len = length( item );
    Q_ASSERT( len > 0 );
    Q_UNUSED( len ); // --release warning

    si.glyph_data_offset = used;

    if ( !si.fontEngine ) {
        si.fontEngine = fnt->engineForScript( script );
        if ( si.fontEngine->type() == QFontEngine::Box )
            si.fontEngine = fnt->engineForScript( QFont::Latin );
    }
    si.ascent = si.fontEngine->ascent();
    si.descent = si.fontEngine->descent();
    si.fontEngine->ref();


    if ( hasUsp10 && si.fontEngine->ttf ) {
        int l = len;
        si.analysis.logicalOrder = true;
        HRESULT res = E_OUTOFMEMORY;
        HDC hdc = 0;

        do {
            ensureSpace( l );   // imho better place here

            res = ScriptShape( hdc, &si.fontEngine->script_cache, ( WCHAR * ) string.unicode() + from, len,
                               l, &si.analysis, glyphs( &si ), logClusters( &si ), glyphAttributes( &si ),
                               &si.num_glyphs );
            if ( res == E_PENDING ) {
                hdc = GetDC( 0 );
                SelectObject( hdc, si.fontEngine->hfont );
            } else if ( res == USP_E_SCRIPT_NOT_IN_FONT ) {
                si.analysis.script = 0;
            } else if ( res == E_OUTOFMEMORY ) {
                l += 32;
            } else if ( res != S_OK ) {
                goto fail;
            }
        } while ( res != S_OK );
        do {
            ABC abc;
            ensureSpace( si.num_glyphs );
            res = ScriptPlace( hdc, &si.fontEngine->script_cache, glyphs( &si ), si.num_glyphs,
                               glyphAttributes( &si ), &si.analysis, advances( &si ), offsets( &si ), &abc );
            if ( res == E_PENDING ) {
                Q_ASSERT( hdc == 0 );
                hdc = GetDC( 0 );
                SelectObject( hdc, si.fontEngine->hfont );
            } else if ( res != S_OK ) {
                goto fail;
            }

            si.glyph_data_offset = used;
        } while ( res != S_OK );
fail:
        if ( hdc ) {
            SelectObject( hdc, GetStockObject( DEFAULT_GUI_FONT ) );
            ReleaseDC( 0, hdc );
        }
        if ( res == S_OK ) {
            goto end;
        } else {
            DWORD dwLastErr = GetLastError();
        }
    }
    {
        // non uniscribe code path, also used if uniscribe fails for some reason
        Q_ASSERT( script < QFont::NScripts );

        QShaperItem shaper_item;
        shaper_item.script = script;
        shaper_item.string = &string;
        shaper_item.from = si.position;
        shaper_item.length = length( item );
        shaper_item.font = si.fontEngine;
        shaper_item.num_glyphs = QMAX( num_glyphs - used, shaper_item.length );
        shaper_item.flags = si.analysis.bidiLevel % 2 ? RightToLeft : 0;
        shaper_item.has_positioning = FALSE;

        while ( 1 ) {
            ensureSpace( shaper_item.num_glyphs );
            shaper_item.num_glyphs = num_glyphs - used;
            shaper_item.glyphs = glyphs( &si );
            shaper_item.advances = advances( &si );
            shaper_item.offsets = offsets( &si );
            shaper_item.attributes = glyphAttributes( &si );
            shaper_item.log_clusters = logClusters( &si );
            if ( scriptEngines[ shaper_item.script ].shape( &shaper_item ) )
                break;
        }
        si.num_glyphs = shaper_item.num_glyphs;
    }

end:
    si.analysis.script = script;

    /*
        QGlyphLayout *g = glyphs(&si);
        if (this->font(si).d->kerning)
            fontEngine->doKerning(si.num_glyphs, g, QFlag(option.useDesignMetrics() ? DesignMetrics : 0));
    */
    si.width = 0;
    advance_t *advances = this->advances( &si );
    advance_t *end = advances + si.num_glyphs;
    while ( advances < end )
        si.width += *( advances++ );


    ( ( QTextEngine * ) this ) ->used += si.num_glyphs;
}
