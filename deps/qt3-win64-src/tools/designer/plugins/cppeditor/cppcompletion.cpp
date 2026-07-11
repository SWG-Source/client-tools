/**********************************************************************
**
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Designer.
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
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "cppcompletion.h"
#include <qobject.h>
#include <qmetaobject.h>
#include <qobjectlist.h>
#include <qregexp.h>

CppEditorCompletion::CppEditorCompletion( Editor *e )
    : EditorCompletion( e )
{
}

bool CppEditorCompletion::doObjectCompletion( const QString &objName )
{
    if ( !ths )
	return FALSE;
    QString object( objName );
    int i = -1;
    if ( ( i = object.findRev( "->" ) ) != -1 )
	object = object.mid( i + 2 );
    if ( ( i = object.findRev( "." ) ) != -1 )
	object = object.mid( i + 1 );
    object = object.simplifyWhiteSpace();
    QObject *obj = 0;
    if ( ths->name() == object || object == "this" ) {
	obj = ths;
    } else {
	obj = ths->child( object );
    }

    if ( !obj )
	return FALSE;

    QValueList<CompletionEntry> lst;

    if ( obj->children() ) {
	for ( QObjectListIt cit( *obj->children() ); cit.current(); ++cit ) {
	    QString s( cit.current()->name() );
	    if ( s.find( " " ) == -1 && s.find( "qt_" ) == -1 && s.find( "unnamed" ) == -1 ) {
		CompletionEntry c;
		c.type = "variable";
		c.text = s;
		c.prefix = "";
		lst << c;
	    }
	}
    }

    QStrList props = obj->metaObject()->propertyNames( TRUE );
    for ( QPtrListIterator<char> pit( props ); pit.current(); ++pit ) {
	QString f( pit.current() );
	QChar c = f[ 0 ];
	f.remove( (uint)0, 1 );
	f.prepend( c.upper() );
	f.prepend( "set" );

	CompletionEntry ce;
	ce.type = "property";
	ce.text = f;
	ce.postfix = "()";

	if ( lst.find( ce ) == lst.end() )
	    lst << ce;
    }

    QStrList slts = obj->metaObject()->slotNames( TRUE );
    for ( QPtrListIterator<char> sit( slts ); sit.current(); ++sit ) {
	QString f( sit.current() );
	f = f.left( f.find( "(" ) );
	CompletionEntry c;
	c.type = "slot";
	c.text = f;
	c.postfix = "()";
	if ( lst.find( c ) == lst.end() )
	    lst << c;
    }

    if ( lst.isEmpty() )
	return FALSE;

    showCompletion( lst );
    return TRUE;
}

QValueList<QStringList> CppEditorCompletion::functionParameters( const QString &expr, QChar &separator,
						     QString &prefix, QString &postfix )
{
    Q_UNUSED( prefix );
    Q_UNUSED( postfix );
    separator = ',';
    if ( !ths )
	return QValueList<QStringList>();
    QString func;
    QString objName;
    int i = -1;

    i = expr.findRev( "->" );
    if ( i == -1 )
	i = expr.findRev( "." );
    else
	++i;
    if ( i == -1 ) {
	i = expr.findRev( " " );

	if ( i == -1 )
	    i = expr.findRev( "\t" );
	else
	    objName = ths->name();

	if ( i == -1 && expr[ 0 ] != ' ' && expr[ 0 ] != '\t' )
	    objName = ths->name();
    }

    if ( !objName.isEmpty() ) {
	func = expr.mid( i + 1 );
	func = func.simplifyWhiteSpace();
    } else {
	func = expr.mid( i + 1 );
	func = func.simplifyWhiteSpace();
	QString ex( expr );
	ex.remove( i, 0xFFFFFF );
	if ( ex[ (int)ex.length() - 1 ] == '-' )
	    ex.remove( ex.length() - 1, 1 );
	int j = -1;
	j = ex.findRev( "->" );
	if ( j == -1 )
	    j = ex.findRev( "." );
	else
	    ++j;
	if ( j == -1 ) {
	    j = ex.findRev( " " );

	    if ( j == -1 )
		j = ex.findRev( "\t" );
	    else
		objName = ths->name();

	    if ( j == -1 )
		objName = ths->name();
	}
	objName = ex.mid( j + 1 );
	objName = objName.simplifyWhiteSpace();
    }

    QObject *obj = 0;
    if ( ths->name() == objName || objName == "this" ) {
	obj = ths;
    } else {
	obj = ths->child( objName );
    }

    if ( !obj )
	return QValueList<QStringList>();

    QStrList slts = obj->metaObject()->slotNames( TRUE );
    for ( QPtrListIterator<char> sit( slts ); sit.current(); ++sit ) {
	QString f( sit.current() );
	f = f.left( f.find( "(" ) );
	if ( f == func ) {
	    f = QString( sit.current() );
	    f.remove( (uint)0, f.find( "(" ) + 1 );
	    f = f.left( f.find( ")" ) );
	    QStringList lst = QStringList::split( ',', f );
	    if ( !lst.isEmpty() ) {
		QValueList<QStringList> l;
		l << lst;
		return l;
	    }
	}
    }

    const QMetaProperty *prop =
	obj->metaObject()->
	property( obj->metaObject()->findProperty( func[ 3 ].lower() + func.mid( 4 ), TRUE ), TRUE );
    if ( prop ) {
	QValueList<QStringList> l;
	l << QStringList( prop->type() );
	return l;
    }

    return QValueList<QStringList>();
}

void CppEditorCompletion::setContext( QObject *this_ )
{
    ths = this_;
}
