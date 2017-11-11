/****************************************************************************
** $Id: qt/qfontdatabase.h   3.3.4   edited May 30 2003 $
**
** Definition of the QFontDatabase class
**
** Created : 981126
**
** Copyright (C) 1999-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
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

#ifndef QFONTDATABASE_H
#define QFONTDATABASE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qfont.h"
#include "qvaluelist.h"
#endif // QT_H


#ifndef QT_NO_FONTDATABASE

class QFontStylePrivate; /* Don't touch! */
struct QtFontStyle;
struct QtFontFamily;
struct QtFontFoundry;
struct QFontDef;
class QFontEngine;
#ifdef Q_WS_QWS
class QDiskFont;
#endif

class QFontDatabasePrivate;

class Q_EXPORT QFontDatabase
{
public:
    static QValueList<int> standardSizes();

    QFontDatabase();

    QStringList families() const;
    QStringList families( QFont::Script ) const;
    QStringList styles( const QString & ) const;
    QValueList<int> pointSizes( const QString &, const QString & = QString::null);
    QValueList<int> smoothSizes( const QString &, const QString &);
    QString styleString( const QFont &);

    QFont font( const QString &, const QString &, int);

    bool isBitmapScalable( const QString &, const QString & = QString::null) const;
    bool isSmoothlyScalable( const QString &, const QString & = QString::null) const;
    bool isScalable( const QString &, const QString & = QString::null) const;
    bool isFixedPitch( const QString &, const QString & = QString::null) const;

    bool italic( const QString &, const QString &) const;
    bool bold( const QString &, const QString &) const;
    int weight( const QString &, const QString &) const;

    static QString scriptName(QFont::Script);
    static QString scriptSample(QFont::Script);

#ifdef Q_WS_QWS
    static void qwsAddDiskFont( QDiskFont *qdf );
#endif

    // For source compatibility with < 3.0
#ifndef QT_NO_COMPAT

    QStringList families(bool) const;
    QStringList styles( const QString &, const QString & ) const;
    QValueList<int> pointSizes( const QString &, const QString &, const QString & );
    QValueList<int> smoothSizes( const QString &, const QString &, const QString & );

    QFont font( const QString &, const QString &, int, const QString &);

    bool isBitmapScalable( const QString &, const QString &, const QString & ) const;
    bool isSmoothlyScalable( const QString &, const QString &, const QString & ) const;
    bool isScalable( const QString &, const QString &, const QString & ) const;
    bool isFixedPitch( const QString &, const QString &, const QString & ) const;

    bool italic( const QString &, const QString &, const QString & ) const;
    bool bold( const QString &, const QString &, const QString & ) const;
    int weight( const QString &, const QString &, const QString & ) const;

#endif // QT_NO_COMPAT

private:
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    static QFontEngine *findFont( QFont::Script script, const QFontPrivate *fp,
				  const QFontDef &request, int force_encoding_id = -1 );
#endif // Q_WS_X11

    static void createDatabase();

    static void parseFontName(const QString &name, QString &foundry, QString &family);

    friend struct QFontDef;
    friend class QFontPrivate;
    friend class QFontDialog;
    friend class QFontEngineLatinXLFD;

    QFontDatabasePrivate *d;
};


#ifndef QT_NO_COMPAT

inline QStringList QFontDatabase::families( bool ) const
{
    return families();
}

inline QStringList QFontDatabase::styles( const QString &family,
					  const QString & ) const
{
    return styles(family);
}

inline QValueList<int> QFontDatabase::pointSizes( const QString &family,
						  const QString &style ,
						  const QString & )
{
    return pointSizes(family, style);
}

inline QValueList<int> QFontDatabase::smoothSizes( const QString &family,
						   const QString &style,
						   const QString & )
{
    return smoothSizes(family, style);
}

inline QFont QFontDatabase::font( const QString &familyName,
				  const QString &style,
				  int pointSize,
				  const QString &)
{
    return font(familyName, style, pointSize);
}

inline bool QFontDatabase::isBitmapScalable( const QString &family,
					     const QString &style,
					     const QString & ) const
{
    return isBitmapScalable(family, style);
}

inline bool QFontDatabase::isSmoothlyScalable( const QString &family,
					       const QString &style,
					       const QString & ) const
{
    return isSmoothlyScalable(family, style);
}

inline bool QFontDatabase::isScalable( const QString &family,
				       const QString &style,
				       const QString & ) const
{
    return isScalable(family, style);
}

inline bool QFontDatabase::isFixedPitch( const QString &family,
					 const QString &style,
					 const QString & ) const
{
    return isFixedPitch(family, style);
}

inline bool QFontDatabase::italic( const QString &family,
				   const QString &style,
				   const QString & ) const
{
    return italic(family, style);
}

inline bool QFontDatabase::bold( const QString &family,
				 const QString &style,
				 const QString & ) const
{
    return bold(family, style);
}

inline int QFontDatabase::weight( const QString &family,
				  const QString &style,
				  const QString & ) const
{
    return weight(family, style);
}

#endif // QT_NO_COMPAT

#endif // QT_NO_FONTDATABASE

#endif // QFONTDATABASE_H
