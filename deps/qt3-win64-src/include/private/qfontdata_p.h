/****************************************************************************
** $Id: qfontdata_p.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Definition of internal QFontData struct
**
** Created : 941229
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

#ifndef QFONTDATA_P_H
#define QFONTDATA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of internal files.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//
//

#include "qobject.h"
#include "qfont.h"
#include "qpaintdevicemetrics.h"

// forwards
class QFontEngine;
class QPaintDevice;


struct QFontDef
{
    inline QFontDef()
	: pointSize( -1 ), pixelSize( -1 ),
	  styleHint( QFont::AnyStyle ), styleStrategy( QFont::PreferDefault ),
	  weight( 50 ), italic( FALSE ), fixedPitch( FALSE ), stretch( 100 ),
	  ignorePitch(TRUE)
#ifdef Q_WS_MAC
	  ,fixedPitchComputed(FALSE)
#endif
    {
    }

    QString family;

#ifdef Q_WS_X11
    QString addStyle;
#endif // Q_WS_X11

    int pointSize;
    int pixelSize;

    uint styleHint     : 8;
    uint styleStrategy : 16;

    uint weight     :  7; // 0-99
    uint italic     :  1;
    uint fixedPitch :  1;
    uint stretch    : 12; // 0-400

    uint ignorePitch : 1;
    uint fixedPitchComputed : 1; // for Mac OS X only
    uint reserved   : 14; // for future extensions

    bool operator==( const QFontDef &other ) const;
    inline bool operator<( const QFontDef &other ) const
    {
	if ( pixelSize != other.pixelSize ) return pixelSize < other.pixelSize;
	if ( weight != other.weight ) return weight < other.weight;
	if ( italic != other.italic ) return italic < other.italic;
	if ( stretch != other.stretch ) return stretch < other.stretch;
	if ( styleHint != other.styleHint ) return styleHint < other.styleHint;
	if ( styleStrategy != other.styleStrategy ) return styleStrategy < other.styleStrategy;
	if ( family != other.family ) return family < other.family;

#ifdef Q_WS_X11
	if ( addStyle != other.addStyle ) return addStyle < other.addStyle;
#endif // Q_WS_X11

	return FALSE;
    }
};

class QFontEngineData : public QShared
{
public:
    QFontEngineData();
    ~QFontEngineData();

    uint lineWidth;

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    QFontEngine *engines[QFont::LastPrivateScript];
#else
    QFontEngine *engine;
#endif // Q_WS_X11 || Q_WS_WIN
#ifndef Q_WS_MAC
    enum { widthCacheSize = 0x500 };
    uchar widthCache[widthCacheSize];
#endif
};


class QFontPrivate : public QShared
{
public:
    static QFont::Script defaultScript;
#ifdef Q_WS_X11
    static int defaultEncodingID;
#endif // Q_WS_X11

    QFontPrivate();
    QFontPrivate( const QFontPrivate &other );
    ~QFontPrivate();

    void load( QFont::Script script );
    QFontEngine *engineForScript( QFont::Script script ) const {
	if ( script == QFont::NoScript )
	    script = QFontPrivate::defaultScript;
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
	if ( ! engineData || ! engineData->engines[script] )
	    ((QFontPrivate *) this)->load( script );
	return engineData->engines[script];
#else
        if ( ! engineData || ! engineData->engine )
	    ((QFontPrivate *) this)->load( script );
        return engineData->engine;
#endif // Q_WS_X11 || Q_WS_WIN
    }

    QFontDef request;
    QFontEngineData *engineData;
    QPaintDevice *paintdevice;
    int screen;

    uint rawMode    :  1;
    uint underline  :  1;
    uint overline   :  1;
    uint strikeOut  :  1;

    enum {
	Family        = 0x0001,
	Size          = 0x0002,
	StyleHint     = 0x0004,
	StyleStrategy = 0x0008,
	Weight        = 0x0010,
	Italic        = 0x0020,
	Underline     = 0x0040,
	Overline      = 0x0080,
	StrikeOut     = 0x0100,
	FixedPitch    = 0x0200,
	Stretch       = 0x0400,
	Complete      = 0x07ff
    };

    uint mask;

    void resolve( const QFontPrivate *other );
};


class QFontCache : public QObject
{
public:
    static QFontCache *instance;

    QFontCache();
    ~QFontCache();

#ifdef Q_WS_QWS
    void clear();
#endif
    // universal key structure.  QFontEngineDatas and QFontEngines are cached using
    // the same keys
    struct Key {
	Key() : script(0), screen( 0 ), dpi(0) { }
	Key( const QFontDef &d, QFont::Script c, int s, QPaintDevice *pdev )
            : script(c), screen(s) {
            def = d;
#ifdef Q_WS_X11
            dpi = pdev ? QPaintDeviceMetrics(pdev).logicalDpiY() : 0;
#else
            Q_UNUSED(pdev);
            dpi = 0;
#endif
        }

	QFontDef def;
	int script;
	int screen;
        int dpi;

	inline bool operator<( const Key &other ) const
	{
	    if ( script != other.script ) return script < other.script;
	    if ( screen != other.screen ) return screen < other.screen;
	    if ( dpi != other.dpi ) return dpi < other.dpi;
	    return def < other.def;
	}
	inline bool operator==( const Key &other ) const
	{ return def == other.def && script == other.script &&
                screen == other.screen && dpi == other.dpi; }
    };

    // QFontEngineData cache
    typedef QMap<Key,QFontEngineData*> EngineDataCache;
    EngineDataCache engineDataCache;

    QFontEngineData *findEngineData( const Key &key ) const;
    void insertEngineData( const Key &key, QFontEngineData *engineData );

    // QFontEngine cache
    struct Engine {
	Engine() : data( 0 ), timestamp( 0 ), hits( 0 ) { }
	Engine( QFontEngine *d ) : data( d ), timestamp( 0 ), hits( 0 ) { }

	QFontEngine *data;
	uint timestamp;
	uint hits;
    };

    typedef QMap<Key,Engine> EngineCache;
    EngineCache engineCache;

    QFontEngine *findEngine( const Key &key );
    void insertEngine( const Key &key, QFontEngine *engine );

#if defined(Q_WS_WIN) || defined(Q_WS_QWS)
    void cleanupPrinterFonts();
#endif

    private:
    void increaseCost( uint cost );
    void decreaseCost( uint cost );
    void timerEvent( QTimerEvent *event );

    static const uint min_cost;
    uint total_cost, max_cost;
    uint current_timestamp;
    bool fast;
    int timer_id;
};

#endif // QFONTDATA_P_H
