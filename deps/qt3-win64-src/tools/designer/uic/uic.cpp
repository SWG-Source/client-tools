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

bool Uic::isMainWindow = FALSE;

QString Uic::getComment( const QDomNode& n )
{
    QDomNode child = n.firstChild();
    while ( !child.isNull() ) {
	if ( child.toElement().tagName() == "comment" )
	    return child.toElement().firstChild().toText().data();
	child = child.nextSibling();
    }
    return QString::null;
}

QString Uic::mkBool( bool b )
{
    return b? "TRUE" : "FALSE";
}

QString Uic::mkBool( const QString& s )
{
    return mkBool( s == "true" || s == "1" );
}

bool Uic::toBool( const QString& s )
{
    return s == "true" || s.toInt() != 0;
}

QString Uic::fixString( const QString &str, bool encode )
{
    QString s;
    if ( !encode ) {
	s = str;
	s.replace( "\\", "\\\\" );
	s.replace( "\"", "\\\"" );
	s.replace( "\r", "" );
	s.replace( "\n", "\\n\"\n\"" );
    } else {
	QCString utf8 = str.utf8();
	const int l = utf8.length();

        for ( int i = 0; i < l; ++i ) {
            uchar c = (uchar)utf8[i];
            if (c != 0x0d) // skip \r
               s += "\\x" + QString::number(c, 16);

            if (i && (i % 20) == 0)
                s += "\"\n    \"";
        }
    }

    return "\"" + s + "\"";
}

QString Uic::trcall( const QString& sourceText, const QString& comment )
{
    if ( sourceText.isEmpty() && comment.isEmpty() )
	return "QString::null";

    QString t = trmacro;
    bool encode = FALSE;
    if ( t.isNull() ) {
	t = "tr";
	for ( int i = 0; i < (int) sourceText.length(); i++ ) {
	    if ( sourceText[i].unicode() >= 0x80 ) {
		t = "trUtf8";
		encode = TRUE;
		break;
	    }
	}
    }

    if ( comment.isEmpty() ) {
	return t + "( " + fixString( sourceText, encode ) + " )";
    } else {
	return t + "( " + fixString( sourceText, encode ) + ", " +
	       fixString( comment, encode ) + " )";
    }
}

QString Uic::mkStdSet( const QString& prop )
{
    return QString( "set" ) + prop[0].upper() + prop.mid(1);
}



/*!
  \class Uic uic.h
  \brief User Interface Compiler

  The class Uic encapsulates the user interface compiler (uic).
 */
Uic::Uic( const QString &fn, const char *outputFn, QTextStream &outStream,
	  QDomDocument doc, bool decl, bool subcl, const QString &trm,
	  const QString& subClass, bool omitForwardDecls )
    : out( outStream ), trout( &languageChangeBody ),
      outputFileName( outputFn ), trmacro( trm ), nofwd( omitForwardDecls )
{
    fileName = fn;
    writeFunctImpl = TRUE;
    defMargin = BOXLAYOUT_DEFAULT_MARGIN;
    defSpacing = BOXLAYOUT_DEFAULT_SPACING;
    externPixmaps = FALSE;
    indent = "    "; // default indent

    item_used = cg_used = pal_used = 0;

    layouts << "hbox" << "vbox" << "grid";
    tags = layouts;
    tags << "widget";

    pixmapLoaderFunction = getPixmapLoaderFunction( doc.firstChild().toElement() );
    nameOfClass = getFormClassName( doc.firstChild().toElement() );

    uiFileVersion = doc.firstChild().toElement().attribute("version");
    stdsetdef = toBool( doc.firstChild().toElement().attribute("stdsetdef") );

    if ( doc.firstChild().isNull() || doc.firstChild().firstChild().isNull() )
	return;
    QDomElement e = doc.firstChild().firstChild().toElement();
    QDomElement widget;
    while ( !e.isNull() ) {
	if ( e.tagName() == "widget" ) {
	    widget = e;
	} else if ( e.tagName() == "pixmapinproject" ) {
	    externPixmaps = TRUE;
	} else if ( e.tagName() == "layoutdefaults" ) {
	    defSpacing = e.attribute( "spacing", defSpacing.toString() );
	    defMargin = e.attribute( "margin", defMargin.toString() );
	} else if ( e.tagName() == "layoutfunctions" ) {
	    defSpacing = e.attribute( "spacing", defSpacing.toString() );
	    bool ok;
	    defSpacing.toInt( &ok );
	    if ( !ok ) {
		QString buf = defSpacing.toString();
		defSpacing = buf.append( "()" );
	    }
	    defMargin = e.attribute( "margin", defMargin.toString() );
	    defMargin.toInt( &ok );
	    if ( !ok ) {
		QString buf = defMargin.toString();
		defMargin = buf.append( "()" );
	    }
	}
	e = e.nextSibling().toElement();
    }
    e = widget;

    if ( nameOfClass.isEmpty() )
	nameOfClass = getObjectName( e );
    namespaces = QStringList::split( "::", nameOfClass );
    bareNameOfClass = namespaces.last();
    namespaces.remove( namespaces.fromLast() );

    if ( subcl ) {
	if ( decl )
	    createSubDecl( e, subClass );
	else
	    createSubImpl( e, subClass );
    } else {
	if ( decl )
	    createFormDecl( e );
	else
	    createFormImpl( e );
    }

}

/*! Extracts a pixmap loader function from \a e
 */
QString Uic::getPixmapLoaderFunction( const QDomElement& e )
{
    QDomElement n;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "pixmapfunction" )
	    return n.firstChild().toText().data();
    }
    return QString::null;
}


/*! Extracts the forms class name from \a e
 */
QString Uic::getFormClassName( const QDomElement& e )
{
    QDomElement n;
    QString cn;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "class" ) {
	    QString s = n.firstChild().toText().data();
	    int i;
	    while ( ( i = s.find(' ' )) != -1 )
		s[i] = '_';
	    cn = s;
	}
    }
    return cn;
}

/*! Extracts a class name from \a e.
 */
QString Uic::getClassName( const QDomElement& e )
{
    QString s = e.attribute( "class" );
    if ( s.isEmpty() && e.tagName() == "toolbar" )
	s = "QToolBar";
    else if ( s.isEmpty() && e.tagName() == "menubar" )
	s = "QMenuBar";
    return s;
}

/*! Returns TRUE if database framework code is generated, else FALSE.
*/

bool Uic::isFrameworkCodeGenerated( const QDomElement& e )
{
    QDomElement n = getObjectProperty( e, "frameworkCode" );
    if ( n.attribute("name") == "frameworkCode" &&
	 !DomTool::elementToVariant( n.firstChild().toElement(), QVariant( TRUE, 0 ) ).toBool() )
	return FALSE;
    return TRUE;
}

/*! Extracts an object name from \a e. It's stored in the 'name'
 property.
 */
QString Uic::getObjectName( const QDomElement& e )
{
    QDomElement n = getObjectProperty( e, "name" );
    if ( n.firstChild().toElement().tagName() == "cstring" )
	return n.firstChild().toElement().firstChild().toText().data();
    return QString::null;
}

/*! Extracts an layout name from \a e. It's stored in the 'name'
 property of the preceeding sibling (the first child of a QLayoutWidget).
 */
QString Uic::getLayoutName( const QDomElement& e )
{
    QDomElement p = e.parentNode().toElement();
    QString name;

    if ( getClassName(p) != "QLayoutWidget" )
	name = "Layout";

    QDomElement n = getObjectProperty( p, "name" );
    if ( n.firstChild().toElement().tagName() == "cstring" ) {
	name.prepend( n.firstChild().toElement().firstChild().toText().data() );
	return QStringList::split( "::", name ).last();
    }
    return e.tagName();
}


QString Uic::getDatabaseInfo( const QDomElement& e, const QString& tag )
{
    QDomElement n;
    QDomElement n1;
    int child = 0;
    // database info is a stringlist stored in this order
    if ( tag == "connection" )
	child = 0;
    else if ( tag == "table" )
	child = 1;
    else if ( tag == "field" )
	child = 2;
    else
	return QString::null;
    n = getObjectProperty( e, "database" );
    if ( n.firstChild().toElement().tagName() == "stringlist" ) {
	    // find correct stringlist entry
	    QDomElement n1 = n.firstChild().firstChild().toElement();
	    for ( int i = 0; i < child && !n1.isNull(); ++i )
		n1 = n1.nextSibling().toElement();
	    if ( n1.isNull() )
		return QString::null;
	    return n1.firstChild().toText().data();
    }
    return QString::null;
}


void Uic::registerLayouts( const QDomElement &e )
{
    if ( layouts.contains(e.tagName())) {
	createObjectDecl(e);
	QString t = e.tagName();
	if ( t == "vbox" || t == "hbox" || t == "grid" )
	    createSpacerDecl( e );
    }

    QDomNodeList nl = e.childNodes();
    for ( int i = 0; i < (int) nl.length(); ++i )
	registerLayouts( nl.item(i).toElement() );
}


/*!
  Returns include file for class \a className or a null string.
 */
QString Uic::getInclude( const QString& className )
{
    int wid = WidgetDatabase::idFromClassName( className );
    if ( wid != -1 )
	return WidgetDatabase::includeFile( wid );
    return QString::null;
}


void Uic::createActionDecl( const QDomElement& e )
{
    QString objClass = e.tagName() == "action" ? "QAction" : "QActionGroup";
    QString objName = getObjectName( e );
    if ( objName.isEmpty() )
	return;
    out << "    " << objClass << "* " << objName << ";" << endl;
    if ( e.tagName() == "actiongroup" ) {
	for ( QDomElement n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( n.tagName() == "action" || n.tagName() == "actiongroup" )
		createActionDecl( n );
	}
    }
}

void Uic::createToolbarDecl( const QDomElement &e )
{
    if ( e.tagName() == "toolbar" )
	out << "    " << "QToolBar *" << getObjectName( e ) << ";" << endl;
}

void Uic::createMenuBarDecl( const QDomElement &e )
{
    if ( e.tagName() == "item" ) {
	out << "    " << "QPopupMenu *" << e.attribute( "name" ) << ";" << endl;
	createPopupMenuDecl( e );
    }
}

void Uic::createPopupMenuDecl( const QDomElement &e )
{
    for ( QDomElement n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "action" || n.tagName() == "actiongroup" ) {
	    QDomElement n2 = n.nextSibling().toElement();
	    if ( n2.tagName() == "item" ) { // the action has a sub menu
		out << "    " << "QPopupMenu *" << n2.attribute( "name" ) << ";" << endl;
		createPopupMenuDecl( n2 );
		n = n2;
	    }
	}
    }
}

void Uic::createActionImpl( const QDomElement &n, const QString &parent )
{
    for ( QDomElement ae = n; !ae.isNull(); ae = ae.nextSibling().toElement() ) {
	QString objName = registerObject( getObjectName( ae ) );
	if ( ae.tagName() == "action" )
	    out << indent << objName << " = new QAction( " << parent << ", \"" << objName << "\" );" << endl;
	else if ( ae.tagName() == "actiongroup" )
	    out << indent << objName << " = new QActionGroup( " << parent << ", \"" << objName << "\" );" << endl;
	else
	    continue;
	bool subActionsDone = FALSE;
	bool hasMenuText = FALSE;
	QString actionText;
	for ( QDomElement n2 = ae.firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement() ) {
	    if ( n2.tagName() == "property" ) {
		bool stdset = stdsetdef;
		if ( n2.hasAttribute( "stdset" ) )
		    stdset = toBool( n2.attribute( "stdset" ) );
		QString prop = n2.attribute("name");
		if ( prop == "name" )
		    continue;
		QString value = setObjectProperty( "QAction", objName, prop, n2.firstChild().toElement(), stdset );
		if ( value.isEmpty() )
		    continue;

		QString call = objName + "->";
		if ( stdset ) {
		    call += mkStdSet( prop ) + "( ";
		} else {
		    call += "setProperty( \"" + prop + "\", ";
		}
		call += value + " );";

		if (prop == "menuText")
		    hasMenuText = TRUE;
		else if (prop == "text")
		    actionText = value;

		if ( n2.firstChild().toElement().tagName() == "string" ) {
		    trout << indent << call << endl;
		} else {
		    out << indent << call << endl;
		}
	    } else if ( !subActionsDone && ( n2.tagName() == "actiongroup" || n2.tagName() == "action" ) ) {
		createActionImpl( n2, objName );
		subActionsDone = TRUE;
	    }
	}
	// workaround for loading pre-3.3 files expecting bogus QAction behavior
	if (!hasMenuText && !actionText.isEmpty() && uiFileVersion < "3.3")
	    trout << indent << objName << "->setMenuText(" << actionText << ");" << endl;
    }
}

QString get_dock( const QString &d )
{
    if ( d == "0" )
	return "DockUnmanaged";
    if ( d == "1" )
	return "DockTornOff";
    if ( d == "2" )
	return "DockTop";
    if ( d == "3" )
	return "DockBottom";
    if ( d == "4" )
	return "DockRight";
    if ( d == "5" )
	return "DockLeft";
    if ( d == "6" )
	return "DockMinimized";
    return "";
}

void Uic::createToolbarImpl( const QDomElement &n, const QString &parentClass, const QString &parent )
{
    QDomNodeList nl = n.elementsByTagName( "toolbar" );
    for ( int i = 0; i < (int) nl.length(); i++ ) {
	QDomElement ae = nl.item( i ).toElement();
	QString dock = get_dock( ae.attribute( "dock" ) );
	QString objName = getObjectName( ae );
 	out << indent << objName << " = new QToolBar( QString(\"\"), this, " << dock << " ); " << endl;
	createObjectImpl( ae, parentClass, parent );
	for ( QDomElement n2 = ae.firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement() ) {
	    if ( n2.tagName() == "action" ) {
		out << indent << n2.attribute( "name" ) << "->addTo( " << objName << " );" << endl;
	    } else if ( n2.tagName() == "separator" ) {
		out << indent << objName << "->addSeparator();" << endl;
	    } else if ( n2.tagName() == "widget" ) {
		if ( n2.attribute( "class" ) != "Spacer" ) {
		    createObjectImpl( n2, "QToolBar", objName );
		} else {
		    QString child = createSpacerImpl( n2, parentClass, parent, objName );
		    out << indent << "QApplication::sendPostedEvents( " << objName
			<< ", QEvent::ChildInserted );" << endl;
		    out << indent << objName << "->boxLayout()->addItem( " << child << " );" << endl;
		}
	    }
	}
    }
}

void Uic::createMenuBarImpl( const QDomElement &n, const QString &parentClass, const QString &parent )
{
    QString objName = getObjectName( n );
    out << indent << objName << " = new QMenuBar( this, \"" << objName << "\" );" << endl;
    createObjectImpl( n, parentClass, parent );
    int i = 0;
    QDomElement c = n.firstChild().toElement();
    while ( !c.isNull() ) {
	if ( c.tagName() == "item" ) {
	    QString itemName = c.attribute( "name" );
	    out << endl;
	    out << indent << itemName << " = new QPopupMenu( this );" << endl;
	    createPopupMenuImpl( c, parentClass, itemName );
	    out << indent << objName << "->insertItem( QString(\"\"), " << itemName << ", " << i << " );" << endl;
	    QString findItem(objName + "->findItem(%1)");
	    findItem = findItem.arg(i);
	    trout << indent << "if (" << findItem << ")" << endl;
	    trout << indent << indent << findItem << "->setText( " << trcall( c.attribute( "text" ) ) << " );" << endl;
	} else if ( c.tagName() == "separator" ) {
	    out << endl;
	    out << indent << objName << "->insertSeparator( " << i << " );" << endl;
	}
	c = c.nextSibling().toElement();
	i++;
    }
}

void Uic::createPopupMenuImpl( const QDomElement &e, const QString &parentClass, const QString &parent )
{
    int i = 0;
    for ( QDomElement n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "action" || n.tagName() == "actiongroup" ) {
	    QDomElement n2 = n.nextSibling().toElement();
	    if ( n2.tagName() == "item" ) { // the action has a sub menu
		QString itemName = n2.attribute( "name" );
		QString itemText = n2.attribute( "text" );
		out << indent << itemName << " = new QPopupMenu( this );" << endl;
		out << indent << parent << "->insertItem( "  << n.attribute( "name" ) << "->iconSet(), ";
		out << trcall( itemText ) << ", " << itemName << " );" << endl;
		trout << indent << parent << "->changeItem( " << parent << "->idAt( " << i << " ), ";
		trout << trcall( itemText ) << " );" << endl;
		createPopupMenuImpl( n2, parentClass, itemName );
		n = n2;
	    } else {
		out << indent << n.attribute( "name" ) << "->addTo( " << parent << " );" << endl;
	    }
	} else if ( n.tagName() == "separator" ) {
	    out << indent << parent << "->insertSeparator();" << endl;
	}
	++i;
    }
}

/*!
  Creates implementation of an listbox item tag.
*/

QString Uic::createListBoxItemImpl( const QDomElement &e, const QString &parent,
				    QString *value )
{
    QDomElement n = e.firstChild().toElement();
    QString txt;
    QString com;
    QString pix;
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QString attrib = n.attribute( "name" );
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( attrib == "text" ) {
		txt = v.toString();
		com = getComment( n );
	    } else if ( attrib == "pixmap" ) {
		pix = v.toString();
		if ( !pix.isEmpty() && !pixmapLoaderFunction.isEmpty() ) {
		    pix.prepend( pixmapLoaderFunction + "( " + QString( externPixmaps ? "\"" : "" ) );
		    pix.append(  QString( externPixmaps ? "\"" : "" ) + " )" );
		}
	    }
	}
	n = n.nextSibling().toElement();
    }

    if ( value )
	*value = trcall( txt, com );

    if ( pix.isEmpty() ) {
	return parent + "->insertItem( " + trcall( txt, com ) + " );";
    } else {
	return parent + "->insertItem( " + pix + ", " + trcall( txt, com ) + " );";
    }
}

/*!
  Creates implementation of an iconview item tag.
*/

QString Uic::createIconViewItemImpl( const QDomElement &e, const QString &parent )
{
    QDomElement n = e.firstChild().toElement();
    QString txt;
    QString com;
    QString pix;
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QString attrib = n.attribute( "name" );
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( attrib == "text" ) {
		txt = v.toString();
		com = getComment( n );
	    } else if ( attrib == "pixmap" ) {
		pix = v.toString();
		if ( !pix.isEmpty() && !pixmapLoaderFunction.isEmpty() ) {
		    pix.prepend( pixmapLoaderFunction + "( " + QString( externPixmaps ? "\"" : "" ) );
		    pix.append( QString( externPixmaps ? "\"" : "" ) + " )" );
		}
	    }
	}
	n = n.nextSibling().toElement();
    }

    if ( pix.isEmpty() )
	return "(void) new QIconViewItem( " + parent + ", " + trcall( txt, com ) + " );";
    else
	return "(void) new QIconViewItem( " + parent + ", " + trcall( txt, com ) + ", " + pix + " );";
}

/*!
  Creates implementation of an listview item tag.
*/

QString Uic::createListViewItemImpl( const QDomElement &e, const QString &parent,
				     const QString &parentItem )
{
    QString s;

    QDomElement n = e.firstChild().toElement();

    bool hasChildren = e.elementsByTagName( "item" ).count() > 0;
    QString item;

    if ( hasChildren ) {
	item = registerObject( "item" );
	s = indent + "QListViewItem * " + item + " = ";
    } else {
	item = "item";
	if ( item_used )
	    s = indent + item + " = ";
	else
	    s = indent + "QListViewItem * " + item + " = ";
	item_used = TRUE;
    }

    if ( !parentItem.isEmpty() )
	s += "new QListViewItem( " + parentItem + ", " + lastItem + " );\n";
    else
	s += "new QListViewItem( " + parent + ", " + lastItem + " );\n";

    QStringList texts;
    QStringList pixmaps;
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QString attrib = n.attribute("name");
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( attrib == "text" )
		texts << v.toString();
	    else if ( attrib == "pixmap" ) {
		QString pix = v.toString();
		if ( !pix.isEmpty() && !pixmapLoaderFunction.isEmpty() ) {
		    pix.prepend( pixmapLoaderFunction + "( " + QString( externPixmaps ? "\"" : "" ) );
		    pix.append( QString( externPixmaps ? "\"" : "" ) + " )" );
		}
		pixmaps << pix;
	    }
	} else if ( n.tagName() == "item" ) {
	    s += indent + item + "->setOpen( TRUE );\n";
	    s += createListViewItemImpl( n, parent, item );
	}
	n = n.nextSibling().toElement();
    }

    for ( int i = 0; i < (int)texts.count(); ++i ) {
	if ( !texts[ i ].isEmpty() )
	    s += indent + item + "->setText( " + QString::number( i ) + ", " + trcall( texts[ i ] ) + " );\n";
	if ( !pixmaps[ i ].isEmpty() )
	    s += indent + item + "->setPixmap( " + QString::number( i ) + ", " + pixmaps[ i ] + " );\n";
    }

    lastItem = item;
    return s;
}

/*!
  Creates implementation of an listview column tag.
*/

QString Uic::createListViewColumnImpl( const QDomElement &e, const QString &parent,
				       QString *value )
{
    QDomElement n = e.firstChild().toElement();
    QString txt;
    QString com;
    QString pix;
    bool clickable = FALSE, resizable = FALSE;
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QString attrib = n.attribute("name");
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( attrib == "text" ) {
		txt = v.toString();
		com = getComment( n );
	    } else if ( attrib == "pixmap" ) {
		pix = v.toString();
		if ( !pix.isEmpty() && !pixmapLoaderFunction.isEmpty() ) {
		    pix.prepend( pixmapLoaderFunction + "( " + QString( externPixmaps ? "\"" : "" ) );
		    pix.append( QString( externPixmaps ? "\"" : "" ) + " )" );
		}
	    } else if ( attrib == "clickable" )
		clickable = v.toBool();
	    else if ( attrib == "resizable" || attrib == "resizeable" )
		resizable = v.toBool();
	}
	n = n.nextSibling().toElement();
    }

    if ( value )
	*value = trcall( txt, com );

    QString s;
    s = indent + parent + "->addColumn( " + trcall( txt, com ) + " );\n";
    if ( !pix.isEmpty() )
	s += indent + parent + "->header()->setLabel( " + parent + "->header()->count() - 1, " + pix + ", " + trcall( txt, com ) + " );\n";
    if ( !clickable )
	s += indent + parent + "->header()->setClickEnabled( FALSE, " + parent + "->header()->count() - 1 );\n";
    if ( !resizable )
	s += indent + parent + "->header()->setResizeEnabled( FALSE, " + parent + "->header()->count() - 1 );\n";
    return s;
}

QString Uic::createTableRowColumnImpl( const QDomElement &e, const QString &parent,
				       QString *value )
{
    QString objClass = getClassName( e.parentNode().toElement() );
    QDomElement n = e.firstChild().toElement();
    QString txt;
    QString com;
    QString pix;
    QString field;
    bool isRow = e.tagName() == "row";
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QString attrib = n.attribute("name");
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( attrib == "text" ) {
		txt = v.toString();
		com = getComment( n );
	    } else if ( attrib == "pixmap" ) {
		pix = v.toString();
		if ( !pix.isEmpty() && !pixmapLoaderFunction.isEmpty() ) {
		    pix.prepend( pixmapLoaderFunction + "( " + QString( externPixmaps ? "\"" : "" ) );
		    pix.append( QString( externPixmaps ? "\"" : "" ) + " )" );
		}
	    } else if ( attrib == "field" )
		field = v.toString();
	}
	n = n.nextSibling().toElement();
    }

    if ( value )
	*value = trcall( txt, com );

    // ### This generated code sucks! We have to set the number of
    // rows/cols before and then only do setLabel/()
    // ### careful, though, since QDataTable has an API which makes this code pretty good

    QString s;
    if ( isRow ) {
	s = indent + parent + "->setNumRows( " + parent + "->numRows() + 1 );\n";
	if ( pix.isEmpty() )
	    s += indent + parent + "->verticalHeader()->setLabel( " + parent + "->numRows() - 1, "
		 + trcall( txt, com ) + " );\n";
	else
	    s += indent + parent + "->verticalHeader()->setLabel( " + parent + "->numRows() - 1, "
		 + pix + ", " + trcall( txt, com ) + " );\n";
    } else {
	if ( objClass == "QTable" ) {
	    s = indent + parent + "->setNumCols( " + parent + "->numCols() + 1 );\n";
	    if ( pix.isEmpty() )
		s += indent + parent + "->horizontalHeader()->setLabel( " + parent + "->numCols() - 1, "
		     + trcall( txt, com ) + " );\n";
	    else
		s += indent + parent + "->horizontalHeader()->setLabel( " + parent + "->numCols() - 1, "
		     + pix + ", " + trcall( txt, com ) + " );\n";
	} else if ( objClass == "QDataTable" ) {
	    if ( !txt.isEmpty() && !field.isEmpty() ) {
		if ( pix.isEmpty() )
		    out << indent << parent << "->addColumn( " << fixString( field ) << ", " << trcall( txt, com ) << " );" << endl;
		else
		    out << indent << parent << "->addColumn( " << fixString( field ) << ", " << trcall( txt, com ) << ", " << pix << " );" << endl;
	    }
	}
    }
    return s;
}

/*!
  Creates the implementation of a layout tag. Called from createObjectImpl().
 */
QString Uic::createLayoutImpl( const QDomElement &e, const QString& parentClass, const QString& parent, const QString& layout )
{
    QDomElement n;
    QString objClass, objName;
    objClass = e.tagName();

    QString qlayout = "QVBoxLayout";
    if ( objClass == "hbox" )
	qlayout = "QHBoxLayout";
    else if ( objClass == "grid" )
	qlayout = "QGridLayout";

    bool isGrid = e.tagName() == "grid" ;
    objName = registerObject( getLayoutName( e ) );
    layoutObjects += objName;

    QString margin = DomTool::readProperty( e, "margin", defMargin ).toString();
    QString spacing = DomTool::readProperty( e, "spacing", defSpacing ).toString();
    QString resizeMode = DomTool::readProperty( e, "resizeMode", QString::null ).toString();

    QString optcells;
    if ( isGrid )
	optcells = "1, 1, ";
    if ( (parentClass == "QGroupBox" || parentClass == "QButtonGroup") && layout.isEmpty() ) {
	// special case for group box
	out << indent << parent << "->setColumnLayout(0, Qt::Vertical );" << endl;
	out << indent << parent << "->layout()->setSpacing( " << spacing << " );" << endl;
	out << indent << parent << "->layout()->setMargin( " << margin << " );" << endl;
	out << indent << objName << " = new " << qlayout << "( " << parent << "->layout() );" << endl;
	out << indent << objName << "->setAlignment( Qt::AlignTop );" << endl;
    } else {
	out << indent << objName << " = new " << qlayout << "( ";
	if ( layout.isEmpty() )
	    out << parent;
	else {
	    out << "0";
	    if ( !DomTool::hasProperty( e, "margin" ) )
		margin = "0";
	}
	out << ", " << optcells << margin << ", " << spacing << ", \"" << objName << "\"); " << endl;
    }
    if ( !resizeMode.isEmpty() )
	out << indent << objName << "->setResizeMode( QLayout::" << resizeMode << " );" << endl;

    if ( !isGrid ) {
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( n.tagName() == "spacer" ) {
		QString child = createSpacerImpl( n, parentClass, parent, objName );
		out << indent << objName << "->addItem( " << child << " );" << endl;
	    } else if ( tags.contains( n.tagName() ) ) {
		QString child = createObjectImpl( n, parentClass, parent, objName );
		if ( isLayout( child ) )
		    out << indent << objName << "->addLayout( " << child << " );" << endl;
		else
		    out << indent << objName << "->addWidget( " << child << " );" << endl;
	    }
	}
    } else {
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	    QDomElement ae = n;
	    int row = ae.attribute( "row" ).toInt();
	    int col = ae.attribute( "column" ).toInt();
	    int rowspan = ae.attribute( "rowspan" ).toInt();
	    int colspan = ae.attribute( "colspan" ).toInt();
	    if ( rowspan < 1 )
		rowspan = 1;
	    if ( colspan < 1 )
		colspan = 1;
	    if ( n.tagName() == "spacer" ) {
		QString child = createSpacerImpl( n, parentClass, parent, objName );
		if ( rowspan * colspan != 1 )
		    out << indent << objName << "->addMultiCell( " << child << ", "
			<< row << ", " << ( row + rowspan - 1 ) << ", " << col << ", " << ( col  + colspan - 1 ) << " );" << endl;
		else
		    out << indent << objName << "->addItem( " << child << ", "
			<< row << ", " << col << " );" << endl;
	    } else if ( tags.contains( n.tagName() ) ) {
		QString child = createObjectImpl( n, parentClass, parent, objName );
		out << endl;
		QString o = "Widget";
		if ( isLayout( child ) )
		    o = "Layout";
		if ( rowspan * colspan != 1 )
		    out << indent << objName << "->addMultiCell" << o << "( " << child << ", "
			<< row << ", " << ( row + rowspan - 1 ) << ", " << col << ", " << ( col  + colspan - 1 ) << " );" << endl;
		else
		    out << indent << objName << "->add" << o << "( " << child << ", "
			<< row << ", " << col << " );" << endl;
	    }
	}
    }

    return objName;
}



QString Uic::createSpacerImpl( const QDomElement &e, const QString& /*parentClass*/, const QString& /*parent*/, const QString& /*layout*/)
{
    QDomElement n;
    QString objClass, objName;
    objClass = e.tagName();
    objName = registerObject( getObjectName( e ) );

    QSize size = DomTool::readProperty( e, "sizeHint", QSize( 0, 0 ) ).toSize();
    QString sizeType = DomTool::readProperty( e, "sizeType", "Expanding" ).toString();
    bool isVspacer = DomTool::readProperty( e, "orientation", "Horizontal" ) == "Vertical";

    if ( sizeType != "Expanding" && sizeType != "MinimumExpanding" &&
	 DomTool::hasProperty( e, "geometry" ) ) { // compatibility Qt 2.2
	QRect geom = DomTool::readProperty( e, "geometry", QRect(0,0,0,0) ).toRect();
	size = geom.size();
    }

    if ( isVspacer )
	out << "    " << objName << " = new QSpacerItem( "
	    << size.width() << ", " << size.height()
	    << ", QSizePolicy::Minimum, QSizePolicy::" << sizeType << " );" << endl;
    else
	out << "    " << objName << " = new QSpacerItem( "
	    << size.width() << ", " << size.height()
	    << ", QSizePolicy::" << sizeType << ", QSizePolicy::Minimum );" << endl;

    return objName;
}

static const char* const ColorRole[] = {
    "Foreground", "Button", "Light", "Midlight", "Dark", "Mid",
    "Text", "BrightText", "ButtonText", "Base", "Background", "Shadow",
    "Highlight", "HighlightedText", "Link", "LinkVisited", 0
};


/*!
  Creates a colorgroup with name \a name from the color group \a cg
 */
void Uic::createColorGroupImpl( const QString& name, const QDomElement& e )
{
    QColorGroup cg;
    int r = -1;
    QDomElement n = e.firstChild().toElement();
    QString color;
    while ( !n.isNull() ) {
	if ( n.tagName() == "color" ) {
	    r++;
	    QColor col = DomTool::readColor( n );
	    color = "QColor( %1, %2, %3)";
	    color = color.arg( col.red() ).arg( col.green() ).arg( col.blue() );
	    if ( col == white )
		color = "white";
	    else if ( col == black )
	    color = "black";
	    if ( n.nextSibling().toElement().tagName() != "pixmap" ) {
		out << indent << name << ".setColor( QColorGroup::" << ColorRole[r] << ", " << color << " );" << endl;
	    }
	} else if ( n.tagName() == "pixmap" ) {
	    QString pixmap = n.firstChild().toText().data();
	    if ( !pixmapLoaderFunction.isEmpty() ) {
		pixmap.prepend( pixmapLoaderFunction + "( " + QString( externPixmaps ? "\"" : "" ) );
		pixmap.append( QString( externPixmaps ? "\"" : "" ) + " )" );
	    }
	    out << indent << name << ".setBrush( QColorGroup::"
		<< ColorRole[r] << ", QBrush( " << color << ", " << pixmap << " ) );" << endl;
	}
	n = n.nextSibling().toElement();
    }
}

/*!
  Auxiliary function to load a color group. The colorgroup must not
  contain pixmaps.
 */
QColorGroup Uic::loadColorGroup( const QDomElement &e )
{
    QColorGroup cg;
    int r = -1;
    QDomElement n = e.firstChild().toElement();
    QColor col;
    while ( !n.isNull() ) {
	if ( n.tagName() == "color" ) {
	    r++;
	    cg.setColor( (QColorGroup::ColorRole)r, (col = DomTool::readColor( n ) ) );
	}
	n = n.nextSibling().toElement();
    }
    return cg;
}

/*!  Returns TRUE if the widget properties specify that it belongs to
  the database \a connection and \a table.
*/

bool Uic::isWidgetInTable( const QDomElement& e, const QString& connection, const QString& table )
{
    QString conn = getDatabaseInfo( e, "connection" );
    QString tab = getDatabaseInfo( e, "table" );
    if ( conn == connection && tab == table )
	return TRUE;
    return FALSE;
}

/*!
  Registers all database connections, cursors and forms.
*/

void Uic::registerDatabases( const QDomElement& e )
{
    QDomElement n;
    QDomNodeList nl;
    int i;
    nl = e.parentNode().toElement().elementsByTagName( "widget" );
    for ( i = 0; i < (int) nl.length(); ++i ) {
	n = nl.item(i).toElement();
	QString conn = getDatabaseInfo( n, "connection"  );
	QString tab = getDatabaseInfo( n, "table"  );
	QString fld = getDatabaseInfo( n, "field"  );
	if ( !conn.isNull() ) {
	    dbConnections += conn;
	    if ( !tab.isNull() ) {
		dbCursors[conn] += tab;
		if ( !fld.isNull() )
		    dbForms[conn] += tab;
	    }
	}
    }
}

/*!
  Registers an object with name \a name.

  The returned name is a valid variable identifier, as similar to \a
  name as possible and guaranteed to be unique within the form.

  \sa registeredName(), isObjectRegistered()
 */
QString Uic::registerObject( const QString& name )
{
    if ( objectNames.isEmpty() ) {
	// some temporary variables we need
	objectNames += "img";
	objectNames += "item";
	objectNames += "cg";
	objectNames += "pal";
    }

    QString result = name;
    int i;
    while ( ( i = result.find(' ' )) != -1  ) {
	result[i] = '_';
    }

    if ( objectNames.contains( result ) ) {
	int i = 2;
	while ( objectNames.contains( result + "_" + QString::number(i) ) )
	    i++;
	result += "_";
	result += QString::number(i);
    }
    objectNames += result;
    objectMapper.insert( name, result );
    return result;
}

/*!
  Returns the registered name for the original name \a name
  or \a name if \a name  wasn't registered.

  \sa registerObject(), isObjectRegistered()
 */
QString Uic::registeredName( const QString& name )
{
    if ( !objectMapper.contains( name ) )
	return name;
    return objectMapper[name];
}

/*!
  Returns whether the object \a name was registered yet or not.
 */
bool Uic::isObjectRegistered( const QString& name )
{
    return objectMapper.contains( name );
}


/*!
  Unifies the entries in stringlist \a list. Should really be a QStringList feature.
 */
QStringList Uic::unique( const QStringList& list )
{
    if ( list.isEmpty() )
	return list;

    QStringList result;
    for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
	if ( !result.contains(*it) )
	    result += *it;
    }
    return result;
}



/*!
  Creates an instance of class \a objClass, with parent \a parent and name \a objName
 */
QString Uic::createObjectInstance( const QString& objClass, const QString& parent, const QString& objName )
{

    if ( objClass.mid( 1 ) == "ComboBox" ) {
	return objClass + "( FALSE, " + parent + ", \"" + objName + "\" )";
    }
    return objClass + "( " + parent + ", \"" + objName + "\" )";
}

bool Uic::isLayout( const QString& name ) const
{
    return layoutObjects.contains( name );
}



