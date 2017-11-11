/****************************************************************************
** $Id: qt/qvariant.h   3.3.4   edited May 27 2003 $
**
** Definition of QVariant class
**
** Created : 990414
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

#ifndef QVARIANT_H
#define QVARIANT_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

#ifndef QT_NO_VARIANT
class QString;
class QCString;
class QFont;
class QPixmap;
class QBrush;
class QRect;
class QPoint;
class QImage;
class QSize;
class QColor;
class QPalette;
class QColorGroup;
class QIconSet;
class QDataStream;
class QPointArray;
class QRegion;
class QBitmap;
class QCursor;
class QStringList;
class QSizePolicy;
class QDate;
class QTime;
class QDateTime;
class QBitArray;
class QKeySequence;
class QPen;
// Some headers rejected after QVariant declaration for GCC 2.7.* compatibility
class QVariant;
#ifndef QT_NO_TEMPLATE_VARIANT
template <class T> class QValueList;
template <class T> class QValueListConstIterator;
template <class T> class QValueListNode;
template <class Key, class T> class QMap;
template <class Key, class T> class QMapConstIterator;
#endif

class Q_EXPORT QVariant
{
public:
    enum Type {
	Invalid,
	Map,
	List,
	String,
	StringList,
	Font,
	Pixmap,
	Brush,
	Rect,
	Size,
	Color,
	Palette,
	ColorGroup,
	IconSet,
	Point,
	Image,
	Int,
	UInt,
	Bool,
	Double,
	CString,
	PointArray,
	Region,
	Bitmap,
	Cursor,
	SizePolicy,
	Date,
	Time,
	DateTime,
	ByteArray,
	BitArray,
	KeySequence,
	Pen,
	LongLong,
	ULongLong
    };

    QVariant();
    ~QVariant();
    QVariant( const QVariant& );
#ifndef QT_NO_DATASTREAM
    QVariant( QDataStream& s );
#endif
    QVariant( const QString& );
    QVariant( const QCString& );
    QVariant( const char* );
#ifndef QT_NO_STRINGLIST
    QVariant( const QStringList& );
#endif
    QVariant( const QFont& );
    QVariant( const QPixmap& );
    QVariant( const QImage& );
    QVariant( const QBrush& );
    QVariant( const QPoint& );
    QVariant( const QRect& );
    QVariant( const QSize& );
    QVariant( const QColor& );
    QVariant( const QPalette& );
    QVariant( const QColorGroup& );
    QVariant( const QIconSet& );
    QVariant( const QPointArray& );
    QVariant( const QRegion& );
    QVariant( const QBitmap& );
    QVariant( const QCursor& );
    QVariant( const QDate& );
    QVariant( const QTime& );
    QVariant( const QDateTime& );
    QVariant( const QByteArray& );
    QVariant( const QBitArray& );
#ifndef QT_NO_ACCEL
    QVariant( const QKeySequence& );
#endif
    QVariant( const QPen& );
#ifndef QT_NO_TEMPLATE_VARIANT
    QVariant( const QValueList<QVariant>& );
    QVariant( const QMap<QString,QVariant>& );
#endif
    QVariant( int );
    QVariant( uint );
    QVariant( Q_LLONG );
    QVariant( Q_ULLONG );
    // ### Problems on some compilers ?
    QVariant( bool, int );
    QVariant( double );
    QVariant( QSizePolicy );

    QVariant& operator= ( const QVariant& );
    bool operator==( const QVariant& ) const;
    bool operator!=( const QVariant& ) const;

    Type type() const;
    const char* typeName() const;

    bool canCast( Type ) const;
    bool cast( Type );

    bool isValid() const;
    bool isNull() const;

    void clear();

    const QString toString() const;
    const QCString toCString() const;
#ifndef QT_NO_STRINGLIST
    const QStringList toStringList() const;
#endif
    const QFont toFont() const;
    const QPixmap toPixmap() const;
    const QImage toImage() const;
    const QBrush toBrush() const;
    const QPoint toPoint() const;
    const QRect toRect() const;
    const QSize toSize() const;
    const QColor toColor() const;
    const QPalette toPalette() const;
    const QColorGroup toColorGroup() const;
    const QIconSet toIconSet() const;
    const QPointArray toPointArray() const;
    const QBitmap toBitmap() const;
    const QRegion toRegion() const;
    const QCursor toCursor() const;
    const QDate toDate() const;
    const QTime toTime() const;
    const QDateTime toDateTime() const;
    const QByteArray toByteArray() const;
    const QBitArray toBitArray() const;
#ifndef QT_NO_ACCEL
    const QKeySequence toKeySequence() const;
#endif
    const QPen toPen() const;
    int toInt( bool * ok=0 ) const;
    uint toUInt( bool * ok=0 ) const;
    Q_LLONG toLongLong( bool * ok=0 ) const;
    Q_ULLONG toULongLong( bool * ok=0 ) const;
    bool toBool() const;
    double toDouble( bool * ok=0 ) const;
#ifndef QT_NO_TEMPLATE_VARIANT
    const QValueList<QVariant> toList() const;
    const QMap<QString,QVariant> toMap() const;
#endif
    QSizePolicy toSizePolicy() const;

#ifndef QT_NO_TEMPLATE_VARIANT
    QValueListConstIterator<QString> stringListBegin() const;
    QValueListConstIterator<QString> stringListEnd() const;
    QValueListConstIterator<QVariant> listBegin() const;
    QValueListConstIterator<QVariant> listEnd() const;
    QMapConstIterator<QString,QVariant> mapBegin() const;
    QMapConstIterator<QString,QVariant> mapEnd() const;
    QMapConstIterator<QString,QVariant> mapFind( const QString& ) const;
#endif
    QString& asString();
    QCString& asCString();
#ifndef QT_NO_STRINGLIST
    QStringList& asStringList();
#endif
    QFont& asFont();
    QPixmap& asPixmap();
    QImage& asImage();
    QBrush& asBrush();
    QPoint& asPoint();
    QRect& asRect();
    QSize& asSize();
    QColor& asColor();
    QPalette& asPalette();
    QColorGroup& asColorGroup();
    QIconSet& asIconSet();
    QPointArray& asPointArray();
    QBitmap& asBitmap();
    QRegion& asRegion();
    QCursor& asCursor();
    QDate& asDate();
    QTime& asTime();
    QDateTime& asDateTime();
    QByteArray& asByteArray();
    QBitArray& asBitArray();
#ifndef QT_NO_ACCEL
    QKeySequence& asKeySequence();
#endif
    QPen& asPen();
    int& asInt();
    uint& asUInt();
    Q_LLONG& asLongLong();
    Q_ULLONG& asULongLong();
    bool& asBool();
    double& asDouble();
#ifndef QT_NO_TEMPLATE_VARIANT
    QValueList<QVariant>& asList();
    QMap<QString,QVariant>& asMap();
#endif
    QSizePolicy& asSizePolicy();

#ifndef QT_NO_DATASTREAM
    void load( QDataStream& );
    void save( QDataStream& ) const;
#endif
    static const char* typeToName( Type typ );
    static Type nameToType( const char* name );

private:
    void detach();

    class Private : public QShared
    {
    public:
	Private();
	Private( Private* );
	~Private();

	void clear();

	Type typ;
	union
	{
	    uint u;
	    int i;
	    Q_LLONG ll;
	    Q_ULLONG ull;
	    bool b;
	    double d;
	    void *ptr;
	} value;
	uint is_null : 1; // ## 4.0 merge with typ
    };

    Private* d;

public:
    void* rawAccess( void* ptr = 0, Type typ = Invalid, bool deepCopy = FALSE );
};

// down here for GCC 2.7.* compatibility
#ifndef QT_H
#include "qvaluelist.h"
#include "qstringlist.h"
#include "qmap.h"
#endif // QT_H

inline QVariant::Type QVariant::type() const
{
    return d->typ;
}

inline bool QVariant::isValid() const
{
    return (d->typ != Invalid);
}

#ifndef QT_NO_TEMPLATE_VARIANT
inline QValueListConstIterator<QString> QVariant::stringListBegin() const
{
    if ( d->typ != StringList )
	return QValueListConstIterator<QString>();
    return ((const QStringList*)d->value.ptr)->begin();
}

inline QValueListConstIterator<QString> QVariant::stringListEnd() const
{
    if ( d->typ != StringList )
	return QValueListConstIterator<QString>();
    return ((const QStringList*)d->value.ptr)->end();
}

inline QValueListConstIterator<QVariant> QVariant::listBegin() const
{
    if ( d->typ != List )
	return QValueListConstIterator<QVariant>();
    return ((const QValueList<QVariant>*)d->value.ptr)->begin();
}

inline QValueListConstIterator<QVariant> QVariant::listEnd() const
{
    if ( d->typ != List )
	return QValueListConstIterator<QVariant>();
    return ((const QValueList<QVariant>*)d->value.ptr)->end();
}

inline QMapConstIterator<QString,QVariant> QVariant::mapBegin() const
{
    if ( d->typ != Map )
	return QMapConstIterator<QString,QVariant>();
    return ((const QMap<QString,QVariant>*)d->value.ptr)->begin();
}

inline QMapConstIterator<QString,QVariant> QVariant::mapEnd() const
{
    if ( d->typ != Map )
	return QMapConstIterator<QString,QVariant>();
    return ((const QMap<QString,QVariant>*)d->value.ptr)->end();
}

inline QMapConstIterator<QString,QVariant> QVariant::mapFind( const QString& key ) const
{
    if ( d->typ != Map )
	return QMapConstIterator<QString,QVariant>();
    return ((const QMap<QString,QVariant>*)d->value.ptr)->find( key );
}
#endif

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant& p );
Q_EXPORT QDataStream& operator>> ( QDataStream& s, QVariant::Type& p );
Q_EXPORT QDataStream& operator<< ( QDataStream& s, const QVariant::Type p );
#endif

#endif //QT_NO_VARIANT
#endif // QVARIANT_H
