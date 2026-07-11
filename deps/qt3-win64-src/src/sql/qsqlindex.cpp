/****************************************************************************
**
** Implementation of QSqlIndex class
**
** Created : 2000-11-03
**
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
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

#include "qsqlindex.h"

#ifndef QT_NO_SQL

#include "qsqlcursor.h"

/*!
    \class QSqlIndex qsqlindex.h
    \brief The QSqlIndex class provides functions to manipulate and
    describe QSqlCursor and QSqlDatabase indexes.

    \ingroup database
    \module sql

    This class is used to describe and manipulate QSqlCursor and
    QSqlDatabase indexes. An index refers to a single table or view
    in a database. Information about the fields that comprise the
    index can be used to generate SQL statements, or to affect the
    behavior of a \l QSqlCursor object.

    Normally, QSqlIndex objects are created by \l QSqlDatabase or
    QSqlCursor.
*/

/*!
    Constructs an empty index using the cursor name \a cursorname and
    index name \a name.
*/

QSqlIndex::QSqlIndex( const QString& cursorname, const QString& name )
    : QSqlRecord(), cursor(cursorname), nm(name)
{

}

/*!
    Constructs a copy of \a other.
*/

QSqlIndex::QSqlIndex( const QSqlIndex& other )
    : QSqlRecord(other), cursor(other.cursor), nm(other.nm), sorts(other.sorts)
{
}

/*!
    Sets the index equal to \a other.
*/

QSqlIndex& QSqlIndex::operator=( const QSqlIndex& other )
{
    cursor = other.cursor;
    nm = other.nm;
    sorts = other.sorts;
    QSqlRecord::operator=( other );
    return *this;
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlIndex::~QSqlIndex()
{

}

/*!
    Sets the name of the index to \a name.
*/

void QSqlIndex::setName( const QString& name )
{
    nm = name;
}

/*!
    \fn QString QSqlIndex::name() const

    Returns the name of the index.
*/

/*!
    Appends the field \a field to the list of indexed fields. The
    field is appended with an ascending sort order.
*/

void QSqlIndex::append( const QSqlField& field )
{
    append( field, FALSE );
}

/*!
    \overload

    Appends the field \a field to the list of indexed fields. The
    field is appended with an ascending sort order, unless \a desc is
    TRUE.
*/

void QSqlIndex::append( const QSqlField& field, bool desc )
{
    sorts.append( desc );
    QSqlRecord::append( field );
}


/*!
    Returns TRUE if field \a i in the index is sorted in descending
    order; otherwise returns FALSE.
*/

bool QSqlIndex::isDescending( int i ) const
{
    if ( sorts.at( i ) != sorts.end() )
	return sorts[i];
    return FALSE;
}

/*!
    If \a desc is TRUE, field \a i is sorted in descending order.
    Otherwise, field \a i is sorted in ascending order (the default).
    If the field does not exist, nothing happens.
*/

void QSqlIndex::setDescending( int i, bool desc )
{
    if ( sorts.at( i ) != sorts.end() )
	sorts[i] = desc;
}

/*!
    \reimp

    Returns a comma-separated list of all the index's field names as a
    string. This string is suitable, for example, for generating a
    SQL SELECT statement. Only generated fields are included in the
    list (see \l{isGenerated()}). If a \a prefix is specified, e.g. a
    table name, it is prepended before all field names in the form:

    "\a{prefix}.<fieldname>"

    If \a sep is specified, each field is separated by \a sep. If \a
    verbose is TRUE (the default), each field contains a suffix
    indicating an ASCending or DESCending sort order.
*/

QString QSqlIndex::toString( const QString& prefix, const QString& sep, bool verbose ) const
{
    QString s;
    bool comma = FALSE;
    for ( uint i = 0; i < count(); ++i ) {
	if( comma )
	    s += sep + " ";
	s += createField( i, prefix, verbose );
	comma = TRUE;
    }
    return s;
}

/*!
    \reimp

    Returns a list of all the index's field names. Only generated
    fields are included in the list (see \l{isGenerated()}). If a \a
    prefix is specified, e.g. a table name, all fields are prefixed in
    the form:

    "\a{prefix}.<fieldname>"

    If \a verbose is TRUE (the default), each field contains a suffix
    indicating an ASCending or DESCending sort order.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QStringList list = myIndex.toStringList();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

*/
QStringList QSqlIndex::toStringList( const QString& prefix, bool verbose ) const
{
    QStringList s;
    for ( uint i = 0; i < count(); ++i )
	s += createField( i, prefix, verbose );
    return s;
}

/*! \internal

  Creates a string representing the field number \a i using prefix \a
  prefix. If \a verbose is TRUE, ASC or DESC is included in the field
  description if the field is sorted in ASCending or DESCending order.
*/

QString QSqlIndex::createField( int i, const QString& prefix, bool verbose ) const
{
    QString f;
    if ( !prefix.isEmpty() )
	f += prefix + ".";
    f += field( i )->name();
    if ( verbose )
	f += " " + QString( ( isDescending( i ) ? "DESC" : "ASC" ) );
    return f;
}

/*!
    Returns an index based on the field descriptions in \a l and the
    cursor \a cursor. The field descriptions should be in the same
    format that toStringList() produces, for example, a surname field
    in the people table might be in one of these forms: "surname",
    "surname DESC" or "people.surname ASC".

    \sa toStringList()
*/

QSqlIndex QSqlIndex::fromStringList( const QStringList& l, const QSqlCursor* cursor )
{
    QSqlIndex newSort;
    for ( uint i = 0; i < l.count(); ++i ) {
	QString f = l[ i ];
	bool desc = FALSE;
	if ( f.mid( f.length()-3 ) == "ASC" )
	    f = f.mid( 0, f.length()-3 );
	if ( f.mid( f.length()-4 ) == "DESC" ) {
	    desc = TRUE;
	    f = f.mid( 0, f.length()-4 );
	}
	int dot = f.findRev( '.' );
	if ( dot != -1 )
	    f = f.mid( dot+1 );
	const QSqlField* field = cursor->field( f.simplifyWhiteSpace() );
	if ( field )
	    newSort.append( *field, desc );
	else
	    qWarning( "QSqlIndex::fromStringList: unknown field: '" + f + "'" );
    }
    return newSort;
}

/*!
    \fn QString QSqlIndex::cursorName() const

    Returns the name of the cursor which the index is associated with.
*/


/*!
    Sets the name of the cursor that the index is associated with to
    \a cursorName.
*/
void QSqlIndex::setCursorName( const QString& cursorName )
{
    cursor = cursorName;
}

#endif
