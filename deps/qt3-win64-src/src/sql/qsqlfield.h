/****************************************************************************
**
** Definition of QSqlField class
**
** Created : 2000-11-03
**
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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

#ifndef QSQLFIELD_H
#define QSQLFIELD_H

#ifndef QT_H
#include "qstring.h"
#include "qvariant.h"
#endif // QT_H

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_EXPORT
#endif

#ifndef QT_NO_SQL

class QSqlFieldPrivate
{
public:
    QVariant::Type type;
};

class QM_EXPORT_SQL QSqlField
{
public:
    QSqlField( const QString& fieldName = QString::null, QVariant::Type type = QVariant::Invalid );
    QSqlField( const QSqlField& other );
    QSqlField& operator=( const QSqlField& other );
    bool operator==(const QSqlField& other) const;
    virtual ~QSqlField();

    virtual void	setValue( const QVariant& value );
    virtual QVariant	value() const;
    virtual void	setName( const QString& name );
    QString		name() const;
    virtual void	setNull();
    bool		isNull() const;
    virtual void	setReadOnly( bool readOnly );
    bool		isReadOnly() const;
    void		clear( bool nullify = TRUE );
    QVariant::Type	type() const;

private:
    QString       nm;
    QVariant      val;
    uint          ro: 1;
    uint          nul: 1;
    QSqlFieldPrivate* d;
};

inline QVariant QSqlField::value() const
{ return val; }

inline QString QSqlField::name() const
{ return nm; }

inline bool QSqlField::isNull() const
{ return nul; }

inline bool QSqlField::isReadOnly() const
{ return ro; }

inline QVariant::Type QSqlField::type() const
{ return d->type; }


/******************************************/
/*******     QSqlFieldInfo Class     ******/
/******************************************/

struct QSqlFieldInfoPrivate;

class QM_EXPORT_SQL QSqlFieldInfo
{
public:
    QSqlFieldInfo( const QString& name = QString::null,
		   QVariant::Type typ = QVariant::Invalid,
		   int required = -1,
		   int len = -1,
		   int prec = -1,
		   const QVariant& defValue = QVariant(),
		   int sqlType = 0,
		   bool generated = TRUE,
		   bool trim = FALSE,
		   bool calculated = FALSE );
    QSqlFieldInfo( const QSqlFieldInfo & other );
    QSqlFieldInfo( const QSqlField & other, bool generated = TRUE );
    virtual ~QSqlFieldInfo();
    QSqlFieldInfo& operator=( const QSqlFieldInfo& other );
    bool operator==( const QSqlFieldInfo& f ) const;

    QSqlField		toField() const;
    int			isRequired() const;
    QVariant::Type	type() const;
    int			length() const;
    int			precision() const;
    QVariant		defaultValue() const;
    QString		name() const;
    int			typeID() const;
    bool		isGenerated() const;
    bool		isTrim() const;
    bool		isCalculated() const;

    virtual void	setTrim( bool trim );
    virtual void	setGenerated( bool gen );
    virtual void	setCalculated( bool calc );

private:
    QSqlFieldInfoPrivate* d;
};


#endif	// QT_NO_SQL
#endif
