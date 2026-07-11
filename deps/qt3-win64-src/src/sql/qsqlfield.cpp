/****************************************************************************
**
** Implementation of QSqlField class
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

#include "qsqlfield.h"

#ifndef QT_NO_SQL


/*!
    \class QSqlField qsqlfield.h
    \brief The QSqlField class manipulates the fields in SQL database tables
    and views.

    \ingroup database
    \module sql

    QSqlField represents the characteristics of a single column in a
    database table or view, such as the data type and column name. A
    field also contains the value of the database column, which can be
    viewed or changed.

    Field data values are stored as QVariants. Using an incompatible
    type is not permitted. For example:

    \code
    QSqlField f( "myfield", QVariant::Int );
    f.setValue( QPixmap() );  // will not work
    \endcode

    However, the field will attempt to cast certain data types to the
    field data type where possible:

    \code
    QSqlField f( "myfield", QVariant::Int );
    f.setValue( QString("123") ); // casts QString to int
    \endcode

    QSqlField objects are rarely created explicitly in application
    code. They are usually accessed indirectly through \l QSqlRecord
    or \l QSqlCursor which already contain a list of fields. For
    example:

    \code
    QSqlCursor cur( "Employee" );        // create cursor using the 'Employee' table
    QSqlField* f = cur.field( "name" );  // use the 'name' field
    f->setValue( "Dave" );               // set field value
    ...
    \endcode

    In practice we rarely need to extract a pointer to a field at all.
    The previous example would normally be written:

    \code
    QSqlCursor cur( "Employee" );
    cur.setValue( "name", "Dave" );
    ...
    \endcode
*/

/*!
    Constructs an empty field called \a fieldName of type \a type.
*/

QSqlField::QSqlField( const QString& fieldName, QVariant::Type type )
    : nm(fieldName), ro(FALSE), nul(FALSE)
{
    d = new QSqlFieldPrivate();
    d->type = type;
    val.cast( type );
}

/*!
    Constructs a copy of \a other.
*/

QSqlField::QSqlField( const QSqlField& other )
    : nm( other.nm ), val( other.val ), ro( other.ro ), nul( other.nul )
{
    d = new QSqlFieldPrivate();
    d->type = other.d->type;
}

/*!
    Sets the field equal to \a other.
*/

QSqlField& QSqlField::operator=( const QSqlField& other )
{
    nm = other.nm;
    val = other.val;
    ro = other.ro;
    nul = other.nul;
    d->type = other.d->type;
    return *this;    
}

/*!
    Returns TRUE if the field is equal to \a other; otherwise returns
    FALSE. Fields are considered equal when the following field
    properties are the same:

    \list
    \i \c name()
    \i \c isNull()
    \i \c value()
    \i \c isReadOnly()
    \endlist

*/
bool QSqlField::operator==(const QSqlField& other) const
{
    return ( nm == other.nm &&
	     val == other.val &&
	     ro == other.ro &&
	     nul == other.nul &&
	     d->type == other.d->type );
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlField::~QSqlField()
{
    delete d;
}


/*!
    \fn QVariant QSqlField::value() const

    Returns the value of the field as a QVariant.
*/

/*!
    Sets the value of the field to \a value. If the field is read-only
    (isReadOnly() returns TRUE), nothing happens. If the data type of
    \a value differs from the field's current data type, an attempt is
    made to cast it to the proper type. This preserves the data type
    of the field in the case of assignment, e.g. a QString to an
    integer data type. For example:

    \code
    QSqlCursor cur( "Employee" );                 // 'Employee' table
    QSqlField* f = cur.field( "student_count" );  // an integer field
    ...
    f->setValue( myLineEdit->text() );		  // cast the line edit text to an integer
    \endcode

    \sa isReadOnly()
*/

void QSqlField::setValue( const QVariant& value )
{
    if ( isReadOnly() )
	return;
    if ( value.type() != d->type ) {
	if ( !val.canCast( d->type ) )
	     qWarning("QSqlField::setValue: %s cannot cast from %s to %s",
		      nm.local8Bit().data(), value.typeName(), QVariant::typeToName( d->type ) );
    }
    val = value;

    if ( value.isNull() )
	nul = TRUE;
    else
	nul = val.type() == QVariant::Invalid;
}

/*!
    Clears the value of the field. If the field is read-only, nothing
    happens. If \a nullify is TRUE (the default), the field is set to
    NULL.
*/

void QSqlField::clear( bool nullify )
{
    if ( isReadOnly() )
	return;
    QVariant v;
    v.cast( type() );
    val = v;
    if ( nullify )
	nul = TRUE;
}

/*!
    \fn void QSqlField::setName( const QString& name )

    Sets the name of the field to \a name.
*/

void QSqlField::setName( const QString& name )
{
    nm = name;
}

/*!
    \fn void QSqlField::setNull()

    Sets the field to NULL and clears the value using clear(). If the
    field is read-only, nothing happens.

    \sa isReadOnly() clear()
*/

void QSqlField::setNull()
{
    clear( TRUE );
}

/*!
    \fn void QSqlField::setReadOnly( bool readOnly )

    Sets the read only flag of the field's value to \a readOnly.

    \sa setValue()
*/
void QSqlField::setReadOnly( bool readOnly )
{
    ro = readOnly;
}

/*!
    \fn QString QSqlField::name() const

    Returns the name of the field.
*/

/*!
    \fn QVariant::Type QSqlField::type() const

    Returns the field's type as stored in the database.
    Note that the actual value might have a different type,
    Numerical values that are too large to store in a long
    int or double are usually stored as strings to prevent
    precision loss.
*/

/*!
    \fn bool QSqlField::isReadOnly() const

    Returns TRUE if the field's value is read only; otherwise returns
    FALSE.
*/

/*!
    \fn bool QSqlField::isNull() const

    Returns TRUE if the field is currently NULL; otherwise returns
    FALSE.
*/


/******************************************/
/*******     QSqlFieldInfo Impl      ******/
/******************************************/

struct QSqlFieldInfoPrivate
{
    int required, len, prec, typeID;
    uint generated: 1;
    uint trim: 1;
    uint calculated: 1;
    QString name;
    QString typeName;
    QVariant::Type typ;
    QVariant defValue;
};

/*!
    \class QSqlFieldInfo qsqlfield.h
    \brief The QSqlFieldInfo class stores meta data associated with a SQL field.

    \ingroup database
    \module sql

    QSqlFieldInfo objects only store meta data; field values are
    stored in QSqlField objects.

    All values must be set in the constructor, and may be retrieved
    using isRequired(), type(), length(), precision(), defaultValue(),
    name(), isGenerated() and typeID().
*/

/*!
    Constructs a QSqlFieldInfo with the following parameters:
    \table
    \row \i \a name  \i the name of the field.
    \row \i \a typ   \i the field's type in a QVariant.
    \row \i \a required  \i greater than 0 if the field is required, 0
    if its value can be NULL and less than 0 if it cannot be
    determined whether the field is required or not.
    \row \i \a len  \i the length of the field. Note that for
    non-character types some databases return either the length in
    bytes or the number of digits. -1 signifies that the length cannot
    be determined.
    \row \i \a prec  \i the precision of the field, or -1 if the field
    has no precision or it cannot be determined.
    \row \i \a defValue  \i the default value that is inserted into
    the table if none is specified by the user. QVariant() if there is
    no default value or it cannot be determined.
    \row \i \a typeID  \i the internal typeID of the database system
    (only useful for low-level programming). 0 if unknown.
    \row \i \a generated  \i TRUE indicates that this field should be
    included in auto-generated SQL statments, e.g. in QSqlCursor.
    \row \i \a trim  \i TRUE indicates that widgets should remove
    trailing whitespace from character fields. This does not affect
    the field value but only its representation inside widgets.
    \row \i \a calculated  \i TRUE indicates that the value of this
    field is calculated. The value of calculated fields can by
    modified by subclassing QSqlCursor and overriding
    QSqlCursor::calculateField().
    \endtable
*/
QSqlFieldInfo::QSqlFieldInfo( const QString& name,
		   QVariant::Type typ,
		   int required,
		   int len,
		   int prec,
		   const QVariant& defValue,
		   int typeID,
		   bool generated,
		   bool trim,
		   bool calculated )
{
    d = new QSqlFieldInfoPrivate();
    d->name = name;
    d->typ = typ;
    d->required = required;
    d->len = len;
    d->prec = prec;
    d->defValue = defValue;
    d->typeID = typeID;
    d->generated = generated;
    d->trim = trim;
    d->calculated = calculated;
}

/*!
    Constructs a copy of \a other.
*/
QSqlFieldInfo::QSqlFieldInfo( const QSqlFieldInfo & other )
{
    d = new QSqlFieldInfoPrivate( *(other.d) );
}

/*!
    Creates a QSqlFieldInfo object with the type and the name of the
    QSqlField \a other. If \a generated is TRUE this field will be
    included in auto-generated SQL statments, e.g. in QSqlCursor.
*/
QSqlFieldInfo::QSqlFieldInfo( const QSqlField & other, bool generated )
{
    d = new QSqlFieldInfoPrivate();
    d->name = other.name();
    d->typ = other.type();
    d->required = -1;
    d->len = -1;
    d->prec = -1;
    d->typeID = 0;
    d->generated = generated;
    d->trim = FALSE;
    d->calculated = FALSE;
}

/*!
    Destroys the object and frees any allocated resources.
*/
QSqlFieldInfo::~QSqlFieldInfo()
{
    delete d;
}

/*!
    Assigns \a other to this field info and returns a reference to it.
*/
QSqlFieldInfo& QSqlFieldInfo::operator=( const QSqlFieldInfo& other )
{
    delete d;
    d = new QSqlFieldInfoPrivate( *(other.d) );
    return *this;
}

/*!
    Returns TRUE if this fieldinfo is equal to \a f; otherwise returns
    FALSE.

    Two field infos are considered equal if all their attributes
    match.
*/
bool QSqlFieldInfo::operator==( const QSqlFieldInfo& f ) const
{
    return ( d->name == f.d->name &&
		d->typ == f.d->typ &&
		d->required == f.d->required &&
		d->len == f.d->len &&
		d->prec == f.d->prec &&
		d->defValue == f.d->defValue &&
		d->typeID == f.d->typeID &&
		d->generated == f.d->generated &&
		d->trim == f.d->trim &&
		d->calculated == f.d->calculated );
}

/*!
    Returns an empty QSqlField based on the information in this
    QSqlFieldInfo.
*/
QSqlField QSqlFieldInfo::toField() const
{ return QSqlField( d->name, d->typ ); }

/*!
    Returns a value greater than 0 if the field is required (NULL
    values are not allowed), 0 if it isn't required (NULL values are
    allowed) or less than 0 if it cannot be determined whether the
    field is required or not.
*/
int QSqlFieldInfo::isRequired() const
{ return d->required; }

/*!
    Returns the field's type or QVariant::Invalid if the type is
    unknown.
*/
QVariant::Type QSqlFieldInfo::type() const
{ return d->typ; }

/*!
    Returns the field's length. For fields storing text the return
    value is the maximum number of characters the field can hold. For
    non-character fields some database systems return the number of
    bytes needed or the number of digits allowed. If the length cannot
    be determined -1 is returned.
*/
int QSqlFieldInfo::length() const
{ return d->len; }

/*!
    Returns the field's precision or -1 if the field has no precision
    or it cannot be determined.
*/
int QSqlFieldInfo::precision() const
{ return d->prec; }

/*!
    Returns the field's default value or an empty QVariant if the
    field has no default value or the value couldn't be determined.
    The default value is the value inserted in the database when it
    is not explicitly specified by the user.
*/
QVariant QSqlFieldInfo::defaultValue() const
{ return d->defValue; }

/*!
    Returns the name of the field in the SQL table.
*/
QString QSqlFieldInfo::name() const
{ return d->name; }

/*!
    Returns the internal type identifier as returned from the database
    system. The return value is 0 if the type is unknown.

    \warning This information is only useful for low-level database
    programming and is \e not database independent.
*/
int QSqlFieldInfo::typeID() const
{ return d->typeID; }

/*!
    Returns TRUE if the field should be included in auto-generated
    SQL statments, e.g. in QSqlCursor; otherwise returns FALSE.

    \sa setGenerated()
*/
bool QSqlFieldInfo::isGenerated() const
{ return d->generated; }

/*!
    Returns TRUE if trailing whitespace should be removed from
    character fields; otherwise returns FALSE.

    \sa setTrim()
*/
bool QSqlFieldInfo::isTrim() const
{ return d->trim; }

/*!
    Returns TRUE if the field is calculated; otherwise returns FALSE.

    \sa setCalculated()
*/
bool QSqlFieldInfo::isCalculated() const
{ return d->calculated; }

/*!
    If \a trim is TRUE widgets should remove trailing whitespace from
    character fields. This does not affect the field value but only
    its representation inside widgets.

    \sa isTrim()
*/
void QSqlFieldInfo::setTrim( bool trim )
{ d->trim = trim; }

/*!
    \a gen set to FALSE indicates that this field should not appear
    in auto-generated SQL statements (for example in QSqlCursor).

    \sa isGenerated()
*/
void QSqlFieldInfo::setGenerated( bool gen )
{ d->generated = gen; }

/*!
    \a calc set to TRUE indicates that this field is a calculated
    field. The value of calculated fields can by modified by subclassing
    QSqlCursor and overriding QSqlCursor::calculateField().

    \sa isCalculated()
*/
void QSqlFieldInfo::setCalculated( bool calc )
{ d->calculated = calc; }

#endif
