/****************************************************************************
**
** Implementation of QSqlForm class
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

#include "qsqlform.h"

#ifndef QT_NO_SQL_FORM

#include "qsqlfield.h"
#include "qsqlpropertymap.h"
#include "qsqlrecord.h"
#include "qstringlist.h"
#include "qwidget.h"
#include "qdict.h"

class QSqlFormPrivate
{
public:
    QSqlFormPrivate() : propertyMap( 0 ), buf( 0 ), dirty( FALSE ) {}
    ~QSqlFormPrivate() { if ( propertyMap ) delete propertyMap; }
    QStringList fld;
    QDict<QWidget> wgt;
    QMap< QWidget *, QSqlField * > map;
    QSqlPropertyMap * propertyMap;
    QSqlRecord* buf;
    bool dirty;
};

/*!
    \class QSqlForm
    \brief The QSqlForm class creates and manages data entry forms
    tied to SQL databases.

    \ingroup database
    \mainclass
    \module sql

    Typical use of a QSqlForm consists of the following steps:
    \list
    \i Create the widgets you want to appear in the form.
    \i Create a cursor and navigate to the record to be edited.
    \i Create the QSqlForm.
    \i Set the form's record buffer to the cursor's update buffer.
    \i Insert each widget and the field it is to edit into the form.
    \i Use readFields() to update the editor widgets with values from
    the database's fields.
    \i Display the form and let the user edit values etc.
    \i Use writeFields() to update the database's field values with
    the values in the editor widgets.
    \endlist

    Note that a QSqlForm does not access the database directly, but
    most often via QSqlFields which are part of a QSqlCursor. A
    QSqlCursor::insert(), QSqlCursor::update() or QSqlCursor::del()
    call is needed to actually write values to the database.

    Some sample code to initialize a form successfully:

    \code
    QLineEdit  myEditor( this );
    QSqlForm   myForm( this );
    QSqlCursor myCursor( "mytable" );

    // Execute a query to make the cursor valid
    myCursor.select();
    // Move the cursor to a valid record (the first record)
    myCursor.next();
    // Set the form's record pointer to the cursor's edit buffer (which
    // contains the current record's values)
    myForm.setRecord( myCursor.primeUpdate() );

    // Insert a field into the form that uses myEditor to edit the
    // field 'somefield' in 'mytable'
    myForm.insert( &myEditor, "somefield" );

    // Update myEditor with the value from the mapped database field
    myForm.readFields();
    ...
    // Let the user edit the form
    ...
    // Update the database
    myForm.writeFields();  // Update the cursor's edit buffer from the form
    myCursor.update();	// Update the database from the cursor's buffer
    \endcode

    If you want to use custom editors for displaying and editing data
    fields, you must install a custom QSqlPropertyMap. The form
    uses this object to get or set the value of a widget.

    Note that \link designer-manual.book Qt Designer\endlink provides
    a visual means of creating data-aware forms.

    \sa installPropertyMap(), QSqlPropertyMap
*/


/*!
    Constructs a QSqlForm with parent \a parent and called \a name.
*/
QSqlForm::QSqlForm( QObject * parent, const char * name )
    : QObject( parent, name )
{
    d = new QSqlFormPrivate();
}

/*!
    Destroys the object and frees any allocated resources.
*/
QSqlForm::~QSqlForm()
{
    delete d;
}

/*!
    Installs a custom QSqlPropertyMap. This is useful if you plan to
    create your own custom editor widgets.

    QSqlForm takes ownership of \a pmap, so \a pmap is deleted when
    QSqlForm goes out of scope.

    \sa QDataTable::installEditorFactory()
*/
void QSqlForm::installPropertyMap( QSqlPropertyMap * pmap )
{
    if( d->propertyMap )
	delete d->propertyMap;
    d->propertyMap = pmap;
}

/*!
    Sets \a buf as the record buffer for the form. To force the
    display of the data from \a buf, use readFields().

    \sa readFields() writeFields()
*/

void QSqlForm::setRecord( QSqlRecord* buf )
{
    d->dirty = TRUE;
    d->buf = buf;
}

/*!
    Inserts a \a widget, and the name of the \a field it is to be
    mapped to, into the form. To actually associate inserted widgets
    with an edit buffer, use setRecord().

    \sa setRecord()
*/

void QSqlForm::insert( QWidget * widget, const QString& field )
{
    d->dirty = TRUE;
    d->wgt.insert( field, widget );
    d->fld += field;
}

/*!
    \overload

    Removes \a field from the form.
*/

void QSqlForm::remove( const QString& field )
{
    d->dirty = TRUE;
    if ( d->fld.find( field ) != d->fld.end() )
	d->fld.remove( d->fld.find( field ) );
    d->wgt.remove( field );
}

/*!
    \overload

    Inserts a \a widget, and the \a field it is to be mapped to, into
    the form.
*/

void QSqlForm::insert( QWidget * widget, QSqlField * field )
{
    d->map[widget] = field;
}

/*!
    Removes a \a widget, and hence the field it's mapped to, from the
    form.
*/

void QSqlForm::remove( QWidget * widget )
{
    d->map.remove( widget );
}

/*!
    Clears the values in all the widgets, and the fields they are
    mapped to, in the form. If \a nullify is TRUE (the default is
    FALSE), each field is also set to NULL.
*/
void QSqlForm::clearValues( bool nullify )
{
    QMap< QWidget *, QSqlField * >::Iterator it;
    for( it = d->map.begin(); it != d->map.end(); ++it ){
	QSqlField* f = (*it);
	if ( f )
	    f->clear( nullify );
    }
    readFields();
}

/*!
    Removes every widget, and the fields they're mapped to, from the form.
*/
void QSqlForm::clear()
{
    d->dirty = TRUE;
    d->fld.clear();
    clearMap();
}

/*!
    Returns the number of widgets in the form.
*/
uint QSqlForm::count() const
{
    return (uint)d->map.count();
}

/*!
    Returns the \a{i}-th widget in the form. Useful for traversing
    the widgets in the form.
*/
QWidget * QSqlForm::widget( uint i ) const
{
    QMap< QWidget *, QSqlField * >::ConstIterator it;
    uint cnt = 0;

    if( i > d->map.count() ) return 0;
    for( it = d->map.begin(); it != d->map.end(); ++it ){
	if( cnt++ == i )
	    return it.key();
    }
    return 0;
}

/*!
    Returns the widget that field \a field is mapped to.
*/
QWidget * QSqlForm::fieldToWidget( QSqlField * field ) const
{
    QMap< QWidget *, QSqlField * >::ConstIterator it;
    for( it = d->map.begin(); it != d->map.end(); ++it ){
	if( *it == field )
	    return it.key();
    }
    return 0;
}

/*!
    Returns the SQL field that widget \a widget is mapped to.
*/
QSqlField * QSqlForm::widgetToField( QWidget * widget ) const
{
    if( d->map.contains( widget ) )
	return d->map[widget];
    else
	return 0;
}

/*!
    Updates the widgets in the form with current values from the SQL
    fields they are mapped to.
*/
void QSqlForm::readFields()
{
    sync();
    QSqlField * f;
    QMap< QWidget *, QSqlField * >::Iterator it;
    QSqlPropertyMap * pmap = (d->propertyMap == 0) ?
			     QSqlPropertyMap::defaultMap() : d->propertyMap;
    for(it = d->map.begin() ; it != d->map.end(); ++it ){
	f = widgetToField( it.key() );
	if( !f )
	    continue;
	pmap->setProperty( it.key(), f->value() );
    }
}

/*!
    Updates the SQL fields with values from the widgets they are
    mapped to. To actually update the database with the contents of
    the record buffer, use QSqlCursor::insert(), QSqlCursor::update()
    or QSqlCursor::del() as appropriate.
*/
void QSqlForm::writeFields()
{
    sync();
    QSqlField * f;
    QMap< QWidget *, QSqlField * >::Iterator it;
    QSqlPropertyMap * pmap = (d->propertyMap == 0) ?
			     QSqlPropertyMap::defaultMap() : d->propertyMap;

    for(it = d->map.begin() ; it != d->map.end(); ++it ){
	f = widgetToField( it.key() );
	if( !f )
	    continue;
	f->setValue( pmap->property( it.key() ) );
    }
}

/*!
    Updates the widget \a widget with the value from the SQL field it
    is mapped to. Nothing happens if no SQL field is mapped to the \a
    widget.
*/
void QSqlForm::readField( QWidget * widget )
{
    sync();
    QSqlField * field = 0;
    QSqlPropertyMap * pmap = (d->propertyMap == 0) ?
			     QSqlPropertyMap::defaultMap() : d->propertyMap;
    field = widgetToField( widget );
    if( field )
	pmap->setProperty( widget, field->value() );
}

/*!
    Updates the SQL field with the value from the \a widget it is
    mapped to. Nothing happens if no SQL field is mapped to the \a
    widget.
*/
void QSqlForm::writeField( QWidget * widget )
{
    sync();
    QSqlField * field = 0;
    QSqlPropertyMap * pmap = (d->propertyMap == 0) ?
			     QSqlPropertyMap::defaultMap() : d->propertyMap;
    field = widgetToField( widget );
    if( field )
	field->setValue( pmap->property( widget ) );
}

/*! \internal
*/

void QSqlForm::sync()
{
    if ( d->dirty ) {
	clearMap();
	if ( d->buf ) {
	    for ( uint i = 0; i < d->fld.count(); ++i )
		insert( d->wgt[ d->fld[ i ] ], d->buf->field( d->fld[ i ] ) );
	}
    }
    d->dirty = FALSE;
}

/*! \internal

  Clears the internal map of widget/field associations
*/

void QSqlForm::clearMap()
{
    d->map.clear();
}

#endif // QT_NO_SQL
