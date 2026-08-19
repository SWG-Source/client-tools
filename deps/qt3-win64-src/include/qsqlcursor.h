/****************************************************************************
**
** Definition of QSqlCursor class
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

#ifndef QSQLCURSOR_H
#define QSQLCURSOR_H

#ifndef QT_H
#include "qsqlrecord.h"
#include "qstringlist.h"
#include "qsqlquery.h"
#include "qsqlindex.h"
#endif // QT_H

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_EXPORT
#endif

#ifndef QT_NO_SQL

class QSqlDatabase;
class QSqlCursorPrivate;

class QM_EXPORT_SQL QSqlCursor : public QSqlRecord, public QSqlQuery
{
public:
    QSqlCursor( const QString & name = QString::null, bool autopopulate = TRUE, QSqlDatabase* db = 0 );
    QSqlCursor( const QSqlCursor & other );
    QSqlCursor& operator=( const QSqlCursor& other );
    ~QSqlCursor();

    enum Mode {
	ReadOnly = 0,
	Insert = 1,
	Update = 2,
	Delete = 4,
	Writable = 7
    };

    QVariant		value( int i ) const;
    QVariant		value( const QString& name ) const;
    void         	setValue( int i, const QVariant& val );
    void         	setValue( const QString& name, const QVariant& val );
    virtual QSqlIndex	primaryIndex( bool prime = TRUE ) const;
    virtual QSqlIndex	index( const QStringList& fieldNames ) const;
    QSqlIndex		index( const QString& fieldName ) const;
    QSqlIndex		index( const char* fieldName ) const;
    virtual void	setPrimaryIndex( const QSqlIndex& idx );

    virtual void	append( const QSqlFieldInfo& fieldInfo );
    virtual void	insert( int pos, const QSqlFieldInfo& fieldInfo );
    void		remove( int pos );
    void		clear();
    void		setGenerated( const QString& name, bool generated );
    void		setGenerated( int i, bool generated );

    virtual QSqlRecord*	editBuffer( bool copy = FALSE );
    virtual QSqlRecord*	primeInsert();
    virtual QSqlRecord*	primeUpdate();
    virtual QSqlRecord*	primeDelete();
    virtual int		insert( bool invalidate = TRUE );
    virtual int		update( bool invalidate = TRUE );
    virtual int		del( bool invalidate = TRUE );

    virtual void	setMode( int flags );
    int			mode() const;
    virtual void	setCalculated( const QString& name, bool calculated );
    bool		isCalculated( const QString& name ) const;
    virtual void	setTrimmed( const QString& name, bool trim );
    bool		isTrimmed( const QString& name ) const;

    bool		isReadOnly() const;
    bool		canInsert() const;
    bool		canUpdate() const;
    bool		canDelete() const;

    bool		select();
    bool		select( const QSqlIndex& sort );
    bool		select( const QSqlIndex & filter, const QSqlIndex & sort );
    virtual bool	select( const QString & filter, const QSqlIndex & sort = QSqlIndex() );

    virtual void	setSort( const QSqlIndex& sort );
    QSqlIndex		sort() const;
    virtual void	setFilter( const QString& filter );
    QString		filter() const;
    virtual void	setName( const QString& name, bool autopopulate = TRUE );
    QString		name() const;
    QString		toString( const QString& prefix = QString::null,
				const QString& sep = "," ) const;
    bool 		isNull( int i ) const;
    bool 		isNull( const QString& name ) const;
    
protected:
    void		afterSeek();
    bool		exec( const QString & sql );

    virtual QVariant	calculateField( const QString& name );
    virtual int		update( const QString & filter, bool invalidate = TRUE );
    virtual int		del( const QString & filter, bool invalidate = TRUE );

    virtual QString	toString( const QString& prefix, QSqlField* field, const QString& fieldSep ) const;
    virtual QString	toString( QSqlRecord* rec, const QString& prefix, const QString& fieldSep,
				const QString& sep ) const;
    virtual QString	toString( const QSqlIndex& i, QSqlRecord* rec, const QString& prefix,
				const QString& fieldSep, const QString& sep ) const;

private:
    void		sync();
    int			apply( const QString& q, bool invalidate );
    int			applyPrepared( const QString& q, bool invalidate );
    QSqlRecord&		operator=( const QSqlRecord & list );
    void 		append( const QSqlField& field );
    void 		insert( int pos, const QSqlField& field );

    QSqlCursorPrivate*	d;
};




#endif	// QT_NO_SQL
#endif
