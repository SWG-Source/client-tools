/****************************************************************************
** $Id: qt/qtranslator.h   3.3.4   edited Jun 19 2003 $
**
** Definition of the translator class
**
** Created : 980906
**
** Copyright (C) 1998-99 by Trolltech AS.  All rights reserved.
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


#ifndef QTRANSLATOR_H
#define QTRANSLATOR_H

#ifndef QT_H
#include "qobject.h"
#include "qvaluelist.h"
#endif // QT_H

#ifndef QT_NO_TRANSLATION

class QTranslatorPrivate;

class Q_EXPORT QTranslatorMessage
{
public:
    QTranslatorMessage();
    QTranslatorMessage( const char * context,
			const char * sourceText,
			const char * comment,
			const QString& translation = QString::null );
    QTranslatorMessage( QDataStream & );
    QTranslatorMessage( const QTranslatorMessage & m );

    QTranslatorMessage & operator=( const QTranslatorMessage & m );

    uint hash() const { return h; }
    const char *context() const { return cx; }
    const char *sourceText() const { return st; }
    const char *comment() const { return cm; }

    void setTranslation( const QString & translation ) { tn = translation; }
    QString translation() const { return tn; }

    enum Prefix { NoPrefix, Hash, HashContext, HashContextSourceText,
		  HashContextSourceTextComment };
    void write( QDataStream & s, bool strip = FALSE,
		Prefix prefix = HashContextSourceTextComment ) const;
    Prefix commonPrefix( const QTranslatorMessage& ) const;

    bool operator==( const QTranslatorMessage& m ) const;
    bool operator!=( const QTranslatorMessage& m ) const
    { return !operator==( m ); }
    bool operator<( const QTranslatorMessage& m ) const;
    bool operator<=( const QTranslatorMessage& m ) const
    { return !m.operator<( *this ); }
    bool operator>( const QTranslatorMessage& m ) const
    { return m.operator<( *this ); }
    bool operator>=( const QTranslatorMessage& m ) const
    { return !operator<( m ); }

private:
    uint h;
    QCString cx;
    QCString st;
    QCString cm;
    QString tn;

    enum Tag { Tag_End = 1, Tag_SourceText16, Tag_Translation, Tag_Context16,
	       Tag_Hash, Tag_SourceText, Tag_Context, Tag_Comment,
	       Tag_Obsolete1 };
};


class Q_EXPORT QTranslator: public QObject
{
    Q_OBJECT
public:
    QTranslator( QObject * parent = 0, const char * name = 0 );
    ~QTranslator();

#ifndef QT_NO_COMPAT
    QString find( const char *context, const char *sourceText, const char * comment = 0 ) const {
	return findMessage( context, sourceText, comment ).translation();
    }
#endif
    virtual QTranslatorMessage findMessage( const char *, const char *,
					    const char * = 0 ) const;

    bool load( const QString & filename,
	       const QString & directory = QString::null,
	       const QString & search_delimiters = QString::null,
	       const QString & suffix = QString::null );
    bool load( const uchar *data, int len ) {
	clear();
	return do_load( data, len );
    }

    void clear();

#ifndef QT_NO_TRANSLATION_BUILDER
    enum SaveMode { Everything, Stripped };

    bool save( const QString & filename, SaveMode mode = Everything );

    void insert( const QTranslatorMessage& );
    void insert( const char *context, const char *sourceText, const QString &translation ) {
	insert( QTranslatorMessage(context, sourceText, "", translation) );
    }
    void remove( const QTranslatorMessage& );
    void remove( const char *context, const char *sourceText ) {
	remove( QTranslatorMessage(context, sourceText, "") );
    }
    bool contains( const char *, const char *, const char * comment = 0 ) const;

    void squeeze( SaveMode = Everything );
    void unsqueeze();

    QValueList<QTranslatorMessage> messages() const;
#endif

    bool isEmpty() const;

private:
#if defined(Q_DISABLE_COPY)
    QTranslator( const QTranslator & );
    QTranslator &operator=( const QTranslator & );
#endif

    bool do_load( const uchar *data, int len );

    QTranslatorPrivate * d;
};

#endif // QT_NO_TRANSLATION

#endif
