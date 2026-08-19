/**********************************************************************
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the Qt Assistant.
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

#include "docuparser.h"
#include "profile.h"

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qstring.h>
#include <qxml.h>

QDataStream &operator>>( QDataStream &s, ContentItem &ci )
{
    s >> ci.title;
    s >> ci.reference;
    s >> ci.depth;
    return s;
}

QDataStream &operator<<( QDataStream &s, const ContentItem &ci )
{
    s << ci.title;
    s << ci.reference;
    s << ci.depth;
    return s;
}

const QString DocuParser::DocumentKey = "/Qt Assistant/" + QString(QT_VERSION_STR) + "/";

DocuParser *DocuParser::createParser( const QString &fileName )
{
    QFile file( fileName );
    if( !file.open( IO_ReadOnly ) ) {	
	return 0;	
    }
    
    QString str;
    int read = 0;
    int maxlen = 1024;
    int majVer = 0, minVer = 0, serVer = 0;
    static QRegExp re( "assistantconfig +version=\"(\\d)\\.(\\d)\\.(\\d)\"", FALSE );
    Q_ASSERT( re.isValid() );
    while( read != -1 ) {
 	read = file.readLine( str, maxlen );
	if( re.search( str ) >= 0 ) {
	    majVer = re.cap( 1 ).toInt();
	    minVer = re.cap( 2 ).toInt();
	    serVer = re.cap( 3 ).toInt();
	}
    }

    if( majVer == 3 && minVer >= 2 ) 
	return new DocuParser320;
    
    return new DocuParser310;	
}


bool DocuParser::parse( QFile *file )
{
    QXmlInputSource source( file );
    QXmlSimpleReader reader;
    reader.setContentHandler( this );
    reader.setErrorHandler( this );
    setFileName( QFileInfo( *file ).absFilePath() );
    return reader.parse( source );
}


QString DocuParser::errorProtocol() const
{
    return errorProt;
}


QValueList<ContentItem> DocuParser::getContentItems()
{
    return contentList;
}


QPtrList<IndexItem> DocuParser::getIndexItems()
{
    return indexList;
}

QString DocuParser::absolutify( const QString &name ) const
{
    QFileInfo orgPath( name );
    if( orgPath.isRelative() )
	return QFileInfo( fname ).dirPath() + QDir::separator() + name;
    return name;
}


void DocuParser310::addTo( Profile *p )
{
    p->addDCFTitle( fname, docTitle );
    p->addDCFIcon( docTitle, iconName );
    p->addDCFIndexPage( docTitle, conURL );
}


bool DocuParser310::startDocument()
{
    state = StateInit;
    errorProt = "";

    contentRef = "";
    indexRef = "";
    depth = 0;
    contentList.clear();
    indexList.clear();

    return TRUE;
}


bool DocuParser310::startElement( const QString &, const QString &,
			       const QString &qname,
			       const QXmlAttributes &attr )
{
    if (qname == "DCF" && state == StateInit) {
	state = StateContent;
	contentRef = absolutify( attr.value( "ref" ) );
	conURL = contentRef;
	docTitle = attr.value( "title" );
	iconName = absolutify( attr.value( "icon" ) );
	contentList.append( ContentItem( docTitle, contentRef, depth ) );
    } else if (qname == "section" && (state == StateContent || state == StateSect)) {
	state = StateSect;
	contentRef = absolutify( attr.value( "ref" ) );
	title = attr.value( "title" );
	depth++;
	contentList.append( ContentItem( title, contentRef, depth ) );
    } else if (qname == "keyword" && state == StateSect) {
	state = StateKeyword;
	indexRef = absolutify( attr.value( "ref" ) );
    } else
	return FALSE;
    return TRUE;
}

bool DocuParser310::endElement( const QString &nameSpace, const QString &localName,
			     const QString &qName )
{
    switch( state ) {
    case StateInit:
	break;
    case StateContent:
	state = StateInit;
	break;
    case StateSect:
	state = --depth ? StateSect : StateContent;
	break;
    case StateKeyword:
	state = StateSect;
	break;
    }
    return TRUE;
}


bool DocuParser310::characters( const QString& ch )
{
    QString str = ch.simplifyWhiteSpace();
    if ( str.isEmpty() )
	return TRUE;

    switch ( state ) {
	case StateInit:
        case StateContent:
        case StateSect:
            return FALSE;
	    break;
        case StateKeyword:
	    indexList.append( new IndexItem( str, indexRef ) );
	    break;
	default:
            return FALSE;
    }
    return TRUE;
}


bool DocuParser310::fatalError( const QXmlParseException& exception )
{
    errorProt += QString( "fatal parsing error: %1 in line %2, column %3\n" )
        .arg( exception.message() )
        .arg( exception.lineNumber() )
        .arg( exception.columnNumber() );

    return QXmlDefaultHandler::fatalError( exception );
}


DocuParser320::DocuParser320()
    : prof( new Profile )
{
}


void DocuParser320::addTo( Profile *p )
{
    QMap<QString,QString>::ConstIterator it;

    for (it = prof->dcfTitles.begin(); it != prof->dcfTitles.end(); ++it)
	p->dcfTitles[it.key()] = *it;

    for (it = prof->icons.begin(); it != prof->icons.end(); ++it)
	p->icons[it.key()] = *it;

    for (it = prof->indexPages.begin(); it != prof->indexPages.end(); ++it)
	p->indexPages[it.key()] = *it;
}


bool DocuParser320::startDocument()
{
    state = StateInit;
    errorProt = "";

    contentRef = "";
    indexRef = "";
    depth = 0;
    contentList.clear();
    indexList.clear();

    prof->addDCF( fname );

    return TRUE;
}

bool DocuParser320::startElement( const QString &, const QString &,
			       const QString &qname,
			       const QXmlAttributes &attr )
{
    QString lower = qname.lower();

    switch( state ) {
	
    case StateInit:
	if( lower == "assistantconfig" )	    
	    state = StateDocRoot;
	break;

    case StateDocRoot:
	if( lower == "dcf" ) {
	    state = StateContent;
	    contentRef = absolutify( attr.value( "ref" ) );
	    conURL = contentRef;
	    docTitle = attr.value( "title" );
	    iconName = absolutify( attr.value( "icon" ) );
	    contentList.append( ContentItem( docTitle, contentRef, depth ) );
	} else if( lower == "profile" ) {
	    state = StateProfile;
	}
	break;

    case StateSect:
	if ( lower == "keyword" && state == StateSect ) {
	    state = StateKeyword;
	    indexRef = absolutify( attr.value( "ref" ) );
	    break;
	} // else if (lower == "section")
    case StateContent:
	if( lower == "section" ) {
	    state = StateSect;
	    contentRef = absolutify( attr.value( "ref" ) );
	    title = attr.value( "title" );
	    depth++;
	    contentList.append( ContentItem( title, contentRef, depth ) );
	}
	break;

    case StateProfile:
	if( lower == "property" ) {
	    state = StateProperty;
	    propertyName = attr.value( "name" );
	}
	break;

    case StateProperty:
	break;
    }    
    
    return TRUE;
}

bool DocuParser320::endElement( const QString &nameSpace,
				const QString &localName,
				const QString &qName )
{
    switch( state ) {
    case StateInit:
	break;
    case StateDocRoot:
	state = StateInit;
	break;
    case StateProfile:
	state = StateDocRoot;
	break;
    case StateProperty:
	state = StateProfile;
	if( propertyName.isEmpty() || propertyValue.isEmpty() )
	    return FALSE;
	{
	    static const QStringList lst = QStringList() << "startpage" << "abouturl"
							 << "applicationicon" << "assistantdocs";
	    if (lst.contains(propertyName))
		propertyValue = absolutify( propertyValue );
	}
	prof->addProperty( propertyName, propertyValue );
	break;
    case StateContent:
	if( !iconName.isEmpty() ) prof->addDCFIcon( docTitle, iconName );
	if( contentRef.isEmpty() )
	    return FALSE;
	prof->addDCFIndexPage( docTitle, conURL );
	prof->addDCFTitle( fname, docTitle );
	state = StateDocRoot;
	break;
    case StateSect:
	state = --depth ? StateSect : StateContent;
	break;
    case StateKeyword:
	state = StateSect;
	break;
    }
    return TRUE;
}

bool DocuParser320::characters( const QString& ch )
{
    QString str = ch.simplifyWhiteSpace();
    if ( str.isEmpty() )
	return TRUE;

    switch ( state ) {
    case StateInit:
    case StateContent:
    case StateSect:
	return FALSE;
	break;
    case StateKeyword:
	indexList.append( new IndexItem( str, indexRef ) );
	break;
    case StateProperty:
        propertyValue = ch;	
	break;
    default:
	return FALSE;
    }
    return TRUE;
}

bool DocuParser320::fatalError( const QXmlParseException& exception )
{
    errorProt += QString( "fatal parsing error: %1 in line %2, column %3\n" )
        .arg( exception.message() )
        .arg( exception.lineNumber() )
        .arg( exception.columnNumber() );

    return QXmlDefaultHandler::fatalError( exception );
}
