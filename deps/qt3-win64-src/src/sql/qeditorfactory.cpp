/****************************************************************************
**
** Implementation of QEditorFactory class
**
** Created : 2000-11-17
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

#include "qcleanuphandler.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qspinbox.h"
#include "qcombobox.h"

#include "qeditorfactory.h"
#include "qdatetimeedit.h"

#ifndef QT_NO_SQL_EDIT_WIDGETS

/*!
    \class QEditorFactory qeditorfactory.h
    \brief The QEditorFactory class is used to create editor widgets
    for QVariant data types.

    \ingroup database
    \module sql

    Each editor factory provides the createEditor() function which
    given a QVariant will create and return a QWidget that can edit
    that QVariant. For example if you have a QVariant::String type, a
    QLineEdit would be the default editor returned, whereas a
    QVariant::Int's default editor would be a QSpinBox.

    If you want to create different editors for fields with the same
    data type, subclass QEditorFactory and reimplement the
    createEditor() function.
*/

/*!
    Constructs an editor factory with parent \a parent, called \a name.
*/

QEditorFactory::QEditorFactory ( QObject * parent, const char * name )
    : QObject( parent, name )
{

}

/*!
    Destroys the object and frees any allocated resources.
*/

QEditorFactory::~QEditorFactory()
{

}

static QEditorFactory * defaultfactory = 0;
static QCleanupHandler< QEditorFactory > q_cleanup_editor_factory;

/*!
    Returns an instance of a default editor factory.
*/

QEditorFactory * QEditorFactory::defaultFactory()
{
    if( defaultfactory == 0 ){
	defaultfactory = new QEditorFactory();
	q_cleanup_editor_factory.add( &defaultfactory );
    }

    return defaultfactory;
}

/*!
    Replaces the default editor factory with \a factory.
    \e{QEditorFactory takes ownership of factory, and destroys it
    when it is no longer needed.}
*/

void QEditorFactory::installDefaultFactory( QEditorFactory * factory )
{
    if( factory == 0 || factory == defaultfactory ) return;

    if( defaultfactory != 0 ){
	q_cleanup_editor_factory.remove( &defaultfactory );
	delete defaultfactory;
    }
    defaultfactory = factory;
    q_cleanup_editor_factory.add( &defaultfactory );
}

/*!
    Creates and returns the appropriate editor for the QVariant \a v.
    If the QVariant is invalid, 0 is returned. The \a parent is passed
    to the appropriate editor's constructor.
*/

QWidget * QEditorFactory::createEditor( QWidget * parent, const QVariant & v )
{
    QWidget * w = 0;
    switch( v.type() ){
	case QVariant::Invalid:
	    w = 0;
	    break;
	case QVariant::Bool:
	    w = new QComboBox( parent, "qt_editor_bool" );
	    ((QComboBox *) w)->insertItem( "False" );
	    ((QComboBox *) w)->insertItem( "True" );
	    break;
	case QVariant::UInt:
	    w = new QSpinBox( 0, 999999, 1, parent, "qt_editor_spinbox" );
	    break;
	case QVariant::Int:
	    w = new QSpinBox( -999999, 999999, 1, parent, "qt_editor_int" );
	    break;
	case QVariant::String:
	case QVariant::CString:
	case QVariant::Double:
	    w = new QLineEdit( parent, "qt_editor_double" );
	    ((QLineEdit*)w)->setFrame( FALSE );
	    break;
	case QVariant::Date:
	    w = new QDateEdit( parent, "qt_editor_date" );
	    break;
	case QVariant::Time:
	    w = new QTimeEdit( parent, "qt_editor_time" );
	    break;
	case QVariant::DateTime:
	    w = new QDateTimeEdit( parent, "qt_editor_datetime" );
	    break;
#ifndef QT_NO_LABEL
	case QVariant::Pixmap:
	    w = new QLabel( parent, "qt_editor_pixmap" );
	    break;
#endif
	case QVariant::Palette:
	case QVariant::ColorGroup:
	case QVariant::Color:
	case QVariant::Font:
	case QVariant::Brush:
	case QVariant::Bitmap:
	case QVariant::Cursor:
	case QVariant::Map:
	case QVariant::StringList:
	case QVariant::Rect:
	case QVariant::Size:
	case QVariant::IconSet:
	case QVariant::Point:
	case QVariant::PointArray:
	case QVariant::Region:
	case QVariant::SizePolicy:
	case QVariant::ByteArray:
	default:
	    w = new QWidget( parent, "qt_editor_default" );
	    break;
    }
    return w;
}
#endif // QT_NO_SQL
