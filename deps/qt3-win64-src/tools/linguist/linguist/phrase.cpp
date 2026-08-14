/**********************************************************************
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Linguist.
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

#include "phrase.h"

#include <qapplication.h>
#include <qcstring.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qxml.h>

static QString protect( const QString& str )
{
    QString p = str;
    p.replace( "&", "&amp;" );
    p.replace( "\"", "&quot;" );
    p.replace( ">", "&gt;" );
    p.replace( "<", "&lt;" );
    p.replace( "'", "&apos;" );
    return p;
}

Phrase::Phrase( const QString& source, const QString& target,
		const QString& definition )
    : s( source ), t( target ), d( definition )
{
}

bool operator==( const Phrase& p, const Phrase& q )
{
    return p.source() == q.source() && p.target() == q.target() &&
	   p.definition() == q.definition();
}

class QphHandler : public QXmlDefaultHandler
{
public:
    QphHandler( PhraseBook *phraseBook )
	: pb( phraseBook ), ferrorCount( 0 ) { }

    virtual bool startElement( const QString& namespaceURI,
			       const QString& localName, const QString& qName,
			       const QXmlAttributes& atts );
    virtual bool endElement( const QString& namespaceURI,
			     const QString& localName, const QString& qName );
    virtual bool characters( const QString& ch );
    virtual bool fatalError( const QXmlParseException& exception );

private:
    PhraseBook *pb;
    QString source;
    QString target;
    QString definition;

    QString accum;
    int ferrorCount;
};

bool QphHandler::startElement( const QString& /* namespaceURI */,
			       const QString& /* localName */,
			       const QString& qName,
			       const QXmlAttributes& /* atts */ )
{
    if ( qName == QString("phrase") ) {
	source.truncate( 0 );
	target.truncate( 0 );
	definition.truncate( 0 );
    }
    accum.truncate( 0 );
    return TRUE;
}

bool QphHandler::endElement( const QString& /* namespaceURI */,
			     const QString& /* localName */,
			     const QString& qName )
{
    if ( qName == QString("source") )
	source = accum;
    else if ( qName == QString("target") )
	target = accum;
    else if ( qName == QString("definition") )
	definition = accum;
    else if ( qName == QString("phrase") )
	pb->append( Phrase(source, target, definition) );
    return TRUE;
}

bool QphHandler::characters( const QString& ch )
{
    accum += ch;
    return TRUE;
}

bool QphHandler::fatalError( const QXmlParseException& exception )
{
    if ( ferrorCount++ == 0 ) {
	QString msg;
	msg.sprintf( "Parse error at line %d, column %d (%s).",
		     exception.lineNumber(), exception.columnNumber(),
		     exception.message().latin1() );
	QMessageBox::information( qApp->mainWidget(),
				  QObject::tr("Qt Linguist"), msg );
    }
    return FALSE;
}

bool PhraseBook::load( const QString& filename )
{
    QFile f( filename );
    if ( !f.open(IO_ReadOnly) )
	return FALSE;

    QTextStream t( &f );
    QXmlInputSource in( t );
    QXmlSimpleReader reader;
    // don't click on these!
    reader.setFeature( "http://xml.org/sax/features/namespaces", FALSE );
    reader.setFeature( "http://xml.org/sax/features/namespace-prefixes", TRUE );
    reader.setFeature( "http://trolltech.com/xml/features/report-whitespace"
		       "-only-CharData", FALSE );
    QXmlDefaultHandler *hand = new QphHandler( this );
    reader.setContentHandler( hand );
    reader.setErrorHandler( hand );

    bool ok = reader.parse( in );
    reader.setContentHandler( 0 );
    reader.setErrorHandler( 0 );
    delete hand;
    f.close();
    if ( !ok )
	clear();
    return ok;
}

bool PhraseBook::save( const QString& filename ) const
{
    QFile f( filename );
    if ( !f.open(IO_WriteOnly) )
	return FALSE;

    QTextStream t( &f );
    t.setEncoding(QTextStream::UnicodeUTF8);
    t << "<!DOCTYPE QPH><QPH>\n";
    ConstIterator p;
    for ( p = begin(); p != end(); ++p ) {
	t << "<phrase>\n";
	t << "    <source>" << protect( (*p).source() ) << "</source>\n";
	t << "    <target>" << protect( (*p).target() ) << "</target>\n";
	if ( !(*p).definition().isEmpty() )
	    t << "    <definition>" << protect( (*p).definition() )
	      << "</definition>\n";
	t << "</phrase>\n";
    }
    t << "</QPH>\n";
    f.close();
    return TRUE;
}
