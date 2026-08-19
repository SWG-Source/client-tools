/****************************************************************************
**
** Definition of QSqlPropertyMap class
**
** Created : 2000-11-20
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

#include "qsqlpropertymap.h"

#ifndef QT_NO_SQL_FORM

#include "qwidget.h"
#include "qcleanuphandler.h"
#include "qmetaobject.h"
#include "qmap.h"

class QSqlPropertyMapPrivate
{
public:
    QSqlPropertyMapPrivate() {}
    QMap< QString, QString > propertyMap;
};

/*!
    \class QSqlPropertyMap qsqlpropertymap.h
    \brief The QSqlPropertyMap class is used to map widgets to SQL fields.

    \ingroup database
    \module sql

    The SQL module uses Qt \link properties.html object
    properties\endlink to insert and extract values from editor
    widgets.

    This class is used to map editors to SQL fields. This works by
    associating SQL editor class names to the properties used to
    insert and extract values to/from the editor.

    For example, a QLineEdit can be used to edit text strings and
    other data types in QDataTables or QSqlForms. Several properties
    are defined in QLineEdit, but only the \e text property is used to
    insert and extract text from a QLineEdit. Both QDataTable and
    QSqlForm use the global QSqlPropertyMap for inserting and
    extracting values to and from an editor widget. The global
    property map defines several common widgets and properties that
    are suitable for many applications. You can add and remove widget
    properties to suit your specific needs.

    If you want to use custom editors with your QDataTable or
    QSqlForm, you must install your own QSqlPropertyMap for that table
    or form. Example:

    \code
    QSqlPropertyMap *myMap  = new QSqlPropertyMap();
    QSqlForm        *myForm = new QSqlForm( this );
    MyEditor myEditor( this );

    // Set the QSqlForm's record buffer to the update buffer of
    // a pre-existing QSqlCursor called 'cur'.
    myForm->setRecord( cur->primeUpdate() );

    // Install the customized map
    myMap->insert( "MyEditor", "content" );
    myForm->installPropertyMap( myMap ); // myForm now owns myMap
    ...
    // Insert a field into the form that uses a myEditor to edit the
    // field 'somefield'
    myForm->insert( &myEditor, "somefield" );

    // Update myEditor with the value from the mapped database field
    myForm->readFields();
    ...
    // Let the user edit the form
    ...
    // Update the database fields with the values in the form
    myForm->writeFields();
    ...
    \endcode

    You can also replace the global QSqlPropertyMap that is used by
    default. (Bear in mind that QSqlPropertyMap takes ownership of the
    new default map.)

    \code
    QSqlPropertyMap *myMap = new QSqlPropertyMap;

    myMap->insert( "MyEditor", "content" );
    QSqlPropertyMap::installDefaultMap( myMap );
    ...
    \endcode

    \sa QDataTable, QSqlForm, QSqlEditorFactory
*/

/*!

Constructs a QSqlPropertyMap.

The default property mappings used by Qt widgets are:
\table
\header \i Widgets \i Property
\row \i \l QCheckBox,
	\l QRadioButton
     \i checked
\row \i \l QComboBox,
	\l QListBox
     \i currentItem
\row \i \l QDateEdit
     \i date
\row \i \l QDateTimeEdit
     \i dateTime
\row \i \l QTextBrowser
     \i source
\row \i \l QButton,
	\l QDial,
	\l QLabel,
	\l QLineEdit,
	\l QMultiLineEdit,
	\l QPushButton,
	\l QTextEdit,
     \i text
\row \i \l QTimeEdit
     \i time
\row \i \l QLCDNumber,
	\l QScrollBar
	\l QSlider,
	\l QSpinBox
     \i value
\endtable
*/

QSqlPropertyMap::QSqlPropertyMap()
{
    d = new QSqlPropertyMapPrivate();
    const struct MapData {
	const char *classname;
	const char *property;
    } mapData[] = {
	{ "QButton", 		"text" },
	{ "QCheckBox", 		"checked" },
	{ "QRadioButton",	"checked" },
	{ "QComboBox", 		"currentItem" },
	{ "QDateEdit", 		"date" },
	{ "QDateTimeEdit",	"dateTime" },
	{ "QDial", 		"value" },
	{ "QLabel", 		"text" },
	{ "QLCDNumber",		"value" },
	{ "QLineEdit",		"text" },
	{ "QListBox",		"currentItem" },
	{ "QMultiLineEdit",	"text" },
	{ "QPushButton",	"text" },
	{ "QScrollBar",		"value" },
	{ "QSlider",		"value" },
	{ "QSpinBox",		"value" },
	{ "QTextBrowser",	"source" },
	{ "QTextEdit",		"text" },
	{ "QTextView",		"text" },
	{ "QTimeEdit",		"time" }
    };

    const MapData *m = mapData;
    for ( uint i = 0; i < sizeof(mapData)/sizeof(MapData); i++, m++ )
	d->propertyMap.insert( m->classname, m->property );
}

/*!
    Destroys the QSqlPropertyMap.

    Note that if the QSqlPropertyMap is installed with
    installPropertyMap() the object it was installed into, e.g. the
    QSqlForm, takes ownership and will delete the QSqlPropertyMap when
    necessary.
*/
QSqlPropertyMap::~QSqlPropertyMap()
{
    delete d;
}

/*!
    Returns the mapped property of \a widget as a QVariant.
*/
QVariant QSqlPropertyMap::property( QWidget * widget )
{
    if( !widget ) return QVariant();
    const QMetaObject* mo = widget->metaObject();
    while ( mo && !d->propertyMap.contains( QString( mo->className() ) ) )
	mo = mo->superClass();

    if ( !mo ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlPropertyMap::property: %s does not exist", widget->metaObject()->className() );
#endif
	return QVariant();
    }
    return widget->property( d->propertyMap[ mo->className() ] );
}

/*!
    Sets the property of \a widget to \a value.
*/
void QSqlPropertyMap::setProperty( QWidget * widget, const QVariant & value )
{
    if( !widget ) return;

    QMetaObject* mo = widget->metaObject();
    while ( mo && !d->propertyMap.contains( QString( mo->className() ) ) )
	mo = mo->superClass();
    if ( !mo ) {
#ifdef QT_CHECK_RANGE
	qWarning("QSqlPropertyMap::setProperty: %s not handled by QSqlPropertyMap", widget->metaObject()->className() );
#endif
	return;
    }

    widget->setProperty( d->propertyMap[ mo->className() ], value );
}

/*!
  Insert a new classname/property pair, which is used for custom SQL
  field editors. There \e must be a \c Q_PROPERTY clause in the \a
  classname class declaration for the \a property.
*/
void QSqlPropertyMap::insert( const QString & classname,
			      const QString & property )
{
    d->propertyMap[ classname ] = property;
}

/*!
    Removes \a classname from the map.
*/
void QSqlPropertyMap::remove( const QString & classname )
{
    d->propertyMap.remove( classname );
}

static QSqlPropertyMap * defaultmap = 0;
static QCleanupHandler< QSqlPropertyMap > qsql_cleanup_property_map;

/*!
    Returns the application global QSqlPropertyMap.
*/
QSqlPropertyMap * QSqlPropertyMap::defaultMap()
{
    if( defaultmap == 0 ){
	defaultmap = new QSqlPropertyMap();
	qsql_cleanup_property_map.add( &defaultmap );
    }
    return defaultmap;
}

/*!
    Replaces the global default property map with \a map. All
    QDataTable and QSqlForm instantiations will use this new map for
    inserting and extracting values to and from editors.
    \e{QSqlPropertyMap takes ownership of \a map, and destroys it
    when it is no longer needed.}
*/
void QSqlPropertyMap::installDefaultMap( QSqlPropertyMap * map )
{
    if( map == 0 ) return;

    if( defaultmap != 0 ){
	qsql_cleanup_property_map.remove( &defaultmap );
	delete defaultmap;
    }
    defaultmap = map;
    qsql_cleanup_property_map.add( &defaultmap );
}

#endif // QT_NO_SQL_FORM
