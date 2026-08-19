/****************************************************************************
** $Id: qfont.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Definition of QFont class
**
** Created : 940514
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

#ifndef QFONT_H
#define QFONT_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qstring.h"
#endif // QT_H


class QFontPrivate;                                     /* don't touch */
class QStringList;
class QTextFormatCollection;

class Q_EXPORT QFont
{
public:
    enum StyleHint {
	Helvetica,  SansSerif = Helvetica,
	Times,      Serif = Times,
	Courier,    TypeWriter = Courier,
	OldEnglish, Decorative = OldEnglish,
	System,
	AnyStyle
    };

    enum StyleStrategy {
	PreferDefault    = 0x0001,
	PreferBitmap     = 0x0002,
	PreferDevice     = 0x0004,
	PreferOutline    = 0x0008,
	ForceOutline     = 0x0010,
	PreferMatch      = 0x0020,
	PreferQuality    = 0x0040,
	PreferAntialias  = 0x0080,
	NoAntialias      = 0x0100,
	OpenGLCompatible = 0x0200
    };

    enum Weight {
	Light    = 25,
	Normal   = 50,
	DemiBold = 63,
	Bold     = 75,
	Black	 = 87
    };

    enum Stretch {
	UltraCondensed =  50,
	ExtraCondensed =  62,
	Condensed      =  75,
	SemiCondensed  =  87,
	Unstretched    = 100,
	SemiExpanded   = 112,
	Expanded       = 125,
	ExtraExpanded  = 150,
	UltraExpanded  = 200
    };

    // default font
    QFont();
    // specific font
#ifdef Q_QDOC
    QFont( const QString &family, int pointSize = 12, int weight = Normal,
	   bool italic = FALSE );
#else
    QFont( const QString &family, int pointSize = -1, int weight = -1,
	   bool italic = FALSE );
#endif
    // copy constructor
    QFont( const QFont & );

    ~QFont();

    QString family() const;
    void setFamily( const QString &);

    int pointSize() const;
    float pointSizeFloat() const;
    void setPointSize( int );
    void setPointSizeFloat( float );

    int pixelSize() const;
    void setPixelSize( int );
    void setPixelSizeFloat( float );

    int weight() const;
    void setWeight( int );

    bool bold() const;
    void setBold( bool );

    bool italic() const;
    void setItalic( bool );

    bool underline() const;
    void setUnderline( bool );

    bool overline() const;
    void setOverline( bool );

    bool strikeOut() const;
    void setStrikeOut( bool );

    bool fixedPitch() const;
    void setFixedPitch( bool );

    StyleHint styleHint() const;
    StyleStrategy styleStrategy() const;
    void setStyleHint( StyleHint, StyleStrategy = PreferDefault );
    void setStyleStrategy( StyleStrategy s );

    int stretch() const;
    void setStretch( int );

    // is raw mode still needed?
    bool rawMode() const;
    void setRawMode( bool );

    // dupicated from QFontInfo
    bool exactMatch() const;

    QFont &operator=( const QFont & );
    bool operator==( const QFont & ) const;
    bool operator!=( const QFont & ) const;
    bool isCopyOf( const QFont & ) const;


#ifdef Q_WS_WIN
    HFONT handle() const;
#else // !Q_WS_WIN
    Qt::HANDLE handle() const;
#endif // Q_WS_WIN


    // needed for X11
    void setRawName( const QString & );
    QString rawName() const;

    QString key() const;

    QString toString() const;
    bool fromString(const QString &);

#ifndef QT_NO_STRINGLIST
    static QString substitute(const QString &);
    static QStringList substitutes(const QString &);
    static QStringList substitutions();
    static void insertSubstitution(const QString&, const QString &);
    static void insertSubstitutions(const QString&, const QStringList &);
    static void removeSubstitution(const QString &);
#endif //QT_NO_STRINGLIST
    static void initialize();
    static void cleanup();
#ifndef Q_WS_QWS
    static void cacheStatistics();
#endif

#if defined(Q_WS_QWS)
    void qwsRenderToDisk(bool all=TRUE);
#endif


    // a copy of this lives in qunicodetables.cpp, as we can't include
    // qfont.h it in tools/. Do not modify without changing the script
    // enum in qunicodetable_p.h aswell.
    enum Script {
	// European Alphabetic Scripts
	Latin,
	Greek,
	Cyrillic,
	Armenian,
	Georgian,
	Runic,
	Ogham,
	SpacingModifiers,
	CombiningMarks,

	// Middle Eastern Scripts
	Hebrew,
	Arabic,
	Syriac,
	Thaana,

	// South and Southeast Asian Scripts
	Devanagari,
	Bengali,
	Gurmukhi,
	Gujarati,
	Oriya,
	Tamil,
	Telugu,
	Kannada,
	Malayalam,
	Sinhala,
	Thai,
	Lao,
	Tibetan,
	Myanmar,
	Khmer,

	// East Asian Scripts
	Han,
	Hiragana,
	Katakana,
	Hangul,
	Bopomofo,
	Yi,

	// Additional Scripts
	Ethiopic,
	Cherokee,
	CanadianAboriginal,
	Mongolian,

	// Symbols
	CurrencySymbols,
	LetterlikeSymbols,
	NumberForms,
	MathematicalOperators,
	TechnicalSymbols,
	GeometricSymbols,
	MiscellaneousSymbols,
	EnclosedAndSquare,
	Braille,

	Unicode,

	// some scripts added in Unicode 3.2
	Tagalog,
	Hanunoo,
	Buhid,
	Tagbanwa,

	KatakanaHalfWidth,

	// from Unicode 4.0
	Limbu,
	TaiLe,

	// End
#if !defined(Q_QDOC)
	NScripts,
	UnknownScript = NScripts,

	NoScript,

	// ----------------------------------------
	// Dear User, you can see values > NScript,
	// but they are internal - do not touch.

	Han_Japanese,
	Han_SimplifiedChinese,
	Han_TraditionalChinese,
	Han_Korean,

	LastPrivateScript
#endif
    };

    QString defaultFamily() const;
    QString lastResortFamily() const;
    QString lastResortFont() const;

#ifndef QT_NO_COMPAT

    static QFont defaultFont();
    static void setDefaultFont( const QFont & );

#endif // QT_NO_COMPAT

    QFont resolve( const QFont & ) const;

protected:
    // why protected?
    bool dirty() const;
    int deciPointSize() const;

private:
    QFont( QFontPrivate *, QPaintDevice *pd );

    void detach();

#if defined(Q_WS_MAC)
    void macSetFont(QPaintDevice *);
#elif defined(Q_WS_X11)
    void x11SetScreen( int screen = -1 );
    int x11Screen() const;
#endif

    friend class QFontMetrics;
    friend class QFontInfo;
    friend class QPainter;
    friend class QPSPrinterFont;
    friend class QApplication;
    friend class QWidget;
    friend class QTextFormatCollection;
    friend class QTextLayout;
    friend class QTextItem;
    friend class QGLContext;
#if defined(Q_WS_X11) && !defined(QT_NO_XFTFREETYPE)
    friend Qt::HANDLE qt_xft_handle(const QFont &font);
#endif
#ifndef QT_NO_DATASTREAM
    friend Q_EXPORT QDataStream &operator<<( QDataStream &, const QFont & );
    friend Q_EXPORT QDataStream &operator>>( QDataStream &, QFont & );
#endif

    QFontPrivate *d;
};


inline bool QFont::bold() const
{ return weight() > Normal; }


inline void QFont::setBold( bool enable )
{ setWeight( enable ? Bold : Normal ); }




/*****************************************************************************
  QFont stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QFont & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QFont & );
#endif


#endif // QFONT_H
