/**********************************************************************
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

#include "uic.h"
#include "parser.h"
#include "widgetdatabase.h"
#include "domtool.h"
#include <qfile.h>
#include <qstringlist.h>
#include <qdatetime.h>
#define NO_STATIC_COLORS
#include <globaldefs.h>
#include <qregexp.h>
#include <stdio.h>
#include <stdlib.h>

/*!
  Creates a declaration ( headerfile ) for a subclass \a subClass
  of the form given in \a e

  \sa createSubImpl()
 */
void Uic::createSubDecl( const QDomElement &e, const QString& subClass )
{
    QDomElement n;
    QDomNodeList nl;
    int i;

    QString objClass = getClassName( e );
    if ( objClass.isEmpty() )
	return;

    out << "class " << subClass << " : public " << nameOfClass << endl;
    out << "{" << endl;

/* tmake ignore Q_OBJECT */
    out << "    Q_OBJECT" << endl;
    out << endl;
    out << "public:" << endl;

    // constructor
    if ( objClass == "QDialog" || objClass == "QWizard" ) {
	out << "    " << subClass << "( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );" << endl;
    } else { // standard QWidget
	out << "    " << subClass << "( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );" << endl;
    }

    // destructor
    out << "    ~" << subClass << "();" << endl;
    out << endl;

    // find additional functions
    QStringList publicSlots, protectedSlots, privateSlots;
    QStringList publicSlotTypes, protectedSlotTypes, privateSlotTypes;
    QStringList publicSlotSpecifier, protectedSlotSpecifier, privateSlotSpecifier;
    QStringList publicFuncts, protectedFuncts, privateFuncts;
    QStringList publicFunctRetTyp, protectedFunctRetTyp, privateFunctRetTyp;
    QStringList publicFunctSpec, protectedFunctSpec, privateFunctSpec;

    nl = e.parentNode().toElement().elementsByTagName( "slot" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	n = nl.item(i).toElement();
	if ( n.parentNode().toElement().tagName() != "slots"
	     && n.parentNode().toElement().tagName() != "connections" )
	    continue;
	if ( n.attribute( "language", "C++" ) != "C++" )
	    continue;
	QString returnType = n.attribute( "returnType", "void" );
	QString functionName = n.firstChild().toText().data().stripWhiteSpace();
	if ( functionName.endsWith( ";" ) )
	    functionName = functionName.left( functionName.length() - 1 );
	QString specifier = n.attribute( "specifier" );
	QString access = n.attribute( "access" );
	if ( access == "protected" ) {
	    protectedSlots += functionName;
	    protectedSlotTypes += returnType;
	    protectedSlotSpecifier += specifier;
	} else if ( access == "private" ) {
	    privateSlots += functionName;
	    privateSlotTypes += returnType;
	    privateSlotSpecifier += specifier;
	} else {
	    publicSlots += functionName;
	    publicSlotTypes += returnType;
	    publicSlotSpecifier += specifier;
	}
    }

    nl = e.parentNode().toElement().elementsByTagName( "function" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	n = nl.item(i).toElement();
	if ( n.parentNode().toElement().tagName() != "functions" )
	    continue;
	if ( n.attribute( "language", "C++" ) != "C++" )
	    continue;
	QString returnType = n.attribute( "returnType", "void" );
	QString functionName = n.firstChild().toText().data().stripWhiteSpace();
	if ( functionName.endsWith( ";" ) )
	    functionName = functionName.left( functionName.length() - 1 );
	QString specifier = n.attribute( "specifier" );
	QString access = n.attribute( "access" );
	if ( access == "protected" ) {
	    protectedFuncts += functionName;
	    protectedFunctRetTyp += returnType;
	    protectedFunctSpec += specifier;
	} else if ( access == "private" ) {
	    privateFuncts += functionName;
	    privateFunctRetTyp += returnType;
	    privateFunctSpec += specifier;
	} else {
	    publicFuncts += functionName;
	    publicFunctRetTyp += returnType;
	    publicFunctSpec += specifier;
	}
    }

    if ( !publicFuncts.isEmpty() )
	writeFunctionsSubDecl( publicFuncts, publicFunctRetTyp, publicFunctSpec );

    // create public additional slots
    if ( !publicSlots.isEmpty() ) {
	out << "public slots:" << endl;
	writeFunctionsSubDecl( publicSlots, publicSlotTypes, publicSlotSpecifier );
    }

    if ( !protectedFuncts.isEmpty() ) {
	out << "protected:" << endl;
	writeFunctionsSubDecl( protectedFuncts, protectedFunctRetTyp, protectedFunctSpec );
    }

    // create protected additional slots
    if ( !protectedSlots.isEmpty() ) {
	out << "protected slots:" << endl;
	writeFunctionsSubDecl( protectedSlots, protectedSlotTypes, protectedSlotSpecifier );
    }

    if ( !privateFuncts.isEmpty() ) {
	out << "private:" << endl;
	writeFunctionsSubDecl( privateFuncts, privateFunctRetTyp, privateFunctSpec );
    }

    // create private additional slots
    if ( !privateSlots.isEmpty() ) {
	out << "private slots:" << endl;
	writeFunctionsSubDecl( privateSlots, privateSlotTypes, privateSlotSpecifier );
    }
    out << "};" << endl;
}

void Uic::writeFunctionsSubDecl( const QStringList &fuLst, const QStringList &typLst, const QStringList &specLst )
{
    QValueListConstIterator<QString> it, it2, it3;
    for ( it = fuLst.begin(), it2 = typLst.begin(), it3 = specLst.begin();
	  it != fuLst.end(); ++it, ++it2, ++it3 ) {
	QString type = *it2;
	if ( type.isEmpty() )
	    type = "void";
	if ( *it3 == "non virtual" )
	    continue;
	out << "    " << type << " " << (*it) << ";" << endl;
    }
    out << endl;
}

/*!
  Creates an implementation for a subclass \a subClass of the form
  given in \a e

  \sa createSubDecl()
 */
void Uic::createSubImpl( const QDomElement &e, const QString& subClass )
{
    QDomElement n;
    QDomNodeList nl;
    int i;

    QString objClass = getClassName( e );
    if ( objClass.isEmpty() )
	return;

    // constructor
    if ( objClass == "QDialog" || objClass == "QWizard" ) {
	out << "/* " << endl;
	out << " *  Constructs a " << subClass << " which is a child of 'parent', with the " << endl;
	out << " *  name 'name' and widget flags set to 'f' " << endl;
	out << " *" << endl;
	out << " *  The " << objClass.mid(1).lower() << " will by default be modeless, unless you set 'modal' to" << endl;
	out << " *  TRUE to construct a modal " << objClass.mid(1).lower() << "." << endl;
	out << " */" << endl;
	out << subClass << "::" << subClass << "( QWidget* parent,  const char* name, bool modal, WFlags fl )" << endl;
	out << "    : " << nameOfClass << "( parent, name, modal, fl )" << endl;
    } else { // standard QWidget
	out << "/* " << endl;
	out << " *  Constructs a " << subClass << " which is a child of 'parent', with the " << endl;
	out << " *  name 'name' and widget flags set to 'f' " << endl;
	out << " */" << endl;
	out << subClass << "::" << subClass << "( QWidget* parent,  const char* name, WFlags fl )" << endl;
	out << "    : " << nameOfClass << "( parent, name, fl )" << endl;
    }
    out << "{" << endl;
    out << "}" << endl;
    out << endl;

    // destructor
    out << "/*  " << endl;
    out << " *  Destroys the object and frees any allocated resources" << endl;
    out << " */" << endl;
    out << subClass << "::~" << subClass << "()" << endl;
    out << "{" << endl;
    out << "    // no need to delete child widgets, Qt does it all for us" << endl;
    out << "}" << endl;
    out << endl;


    // find additional functions
    QStringList publicSlots, protectedSlots, privateSlots;
    QStringList publicSlotTypes, protectedSlotTypes, privateSlotTypes;
    QStringList publicSlotSpecifier, protectedSlotSpecifier, privateSlotSpecifier;
    QStringList publicFuncts, protectedFuncts, privateFuncts;
    QStringList publicFunctRetTyp, protectedFunctRetTyp, privateFunctRetTyp;
    QStringList publicFunctSpec, protectedFunctSpec, privateFunctSpec;

    nl = e.parentNode().toElement().elementsByTagName( "slot" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	n = nl.item(i).toElement();
	if ( n.parentNode().toElement().tagName() != "slots"
	     && n.parentNode().toElement().tagName() != "connections" )
	    continue;
	if ( n.attribute( "language", "C++" ) != "C++" )
	    continue;
	QString returnType = n.attribute( "returnType", "void" );
	QString functionName = n.firstChild().toText().data().stripWhiteSpace();
	if ( functionName.endsWith( ";" ) )
	    functionName = functionName.left( functionName.length() - 1 );
	QString specifier = n.attribute( "specifier" );
	QString access = n.attribute( "access" );
	if ( access == "protected" ) {
	    protectedSlots += functionName;
	    protectedSlotTypes += returnType;
	    protectedSlotSpecifier += specifier;
	} else if ( access == "private" ) {
	    privateSlots += functionName;
	    privateSlotTypes += returnType;
	    privateSlotSpecifier += specifier;
	} else {
	    publicSlots += functionName;
	    publicSlotTypes += returnType;
	    publicSlotSpecifier += specifier;
	}
    }

    nl = e.parentNode().toElement().elementsByTagName( "function" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	n = nl.item(i).toElement();
	if ( n.parentNode().toElement().tagName() != "functions" )
	    continue;
	if ( n.attribute( "language", "C++" ) != "C++" )
	    continue;
	QString returnType = n.attribute( "returnType", "void" );
	QString functionName = n.firstChild().toText().data().stripWhiteSpace();
	if ( functionName.endsWith( ";" ) )
	    functionName = functionName.left( functionName.length() - 1 );
	QString specifier = n.attribute( "specifier" );
	QString access = n.attribute( "access" );
	if ( access == "protected" ) {
	    protectedFuncts += functionName;
	    protectedFunctRetTyp += returnType;
	    protectedFunctSpec += specifier;
	} else if ( access == "private" ) {
	    privateFuncts += functionName;
	    privateFunctRetTyp += returnType;
	    privateFunctSpec += specifier;
	} else {
	    publicFuncts += functionName;
	    publicFunctRetTyp += returnType;
	    publicFunctSpec += specifier;
	}
    }

    if ( !publicFuncts.isEmpty() )
	writeFunctionsSubImpl( publicFuncts, publicFunctRetTyp, publicFunctSpec, subClass, "public function" );

    // create stubs for public additional slots
    if ( !publicSlots.isEmpty() )
	writeFunctionsSubImpl( publicSlots, publicSlotTypes, publicSlotSpecifier, subClass, "public slot" );

    if ( !protectedFuncts.isEmpty() )
	writeFunctionsSubImpl( protectedFuncts, protectedFunctRetTyp, protectedFunctSpec, subClass, "protected function" );

    // create stubs for protected additional slots
    if ( !protectedSlots.isEmpty() )
	writeFunctionsSubImpl( protectedSlots, protectedSlotTypes, protectedSlotSpecifier, subClass, "protected slot" );

    if ( !privateFuncts.isEmpty() )
	writeFunctionsSubImpl( privateFuncts, privateFunctRetTyp, privateFunctSpec, subClass, "private function" );

    // create stubs for private additional slots
    if ( !privateSlots.isEmpty() )
	writeFunctionsSubImpl( privateSlots, privateSlotTypes, privateSlotSpecifier, subClass, "private slot" );
}

void Uic::writeFunctionsSubImpl( const QStringList &fuLst, const QStringList &typLst, const QStringList &specLst,
				 const QString &subClass, const QString &descr )
{
    QValueListConstIterator<QString> it, it2, it3;
    for ( it = fuLst.begin(), it2 = typLst.begin(), it3 = specLst.begin();
	  it != fuLst.end(); ++it, ++it2, ++it3 ) {
	QString type = *it2;
	if ( type.isEmpty() )
	    type = "void";
	if ( *it3 == "non virtual" )
	    continue;
	out << "/*" << endl;
	out << " * " << descr << endl;
	out << " */" << endl;
	out << type << " " << subClass << "::" << (*it) << endl;
	out << "{" << endl;
	out << "    qWarning( \"" << subClass << "::" << (*it) << " not yet implemented!\" );" << endl;
	out << "}" << endl << endl;
    }
    out << endl;
}
