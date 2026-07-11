/*
$Id: structureparser.cpp 1439 2006-05-05 18:17:01Z chehrlic $
*/

#include "structureparser.h"

#include <qstring.h>
#include <qlistview.h>

StructureParser::StructureParser( QListView * t )
                : QXmlDefaultHandler()
{
    setListView( t );
}

void StructureParser::setListView( QListView * t )
{
    table = t;
    table->setSorting( -1 );
    table->addColumn( "Qualified name" );
    table->addColumn( "Namespace" );
}

bool StructureParser::startElement( const QString& namespaceURI,
                                    const QString& ,
                                    const QString& qName,
                                    const QXmlAttributes& attributes)
{
    QListViewItem * element;

    if ( ! stack.isEmpty() ){
	QListViewItem *lastChild = stack.top()->firstChild();
	if ( lastChild ) {
	    while ( lastChild->nextSibling() )
		lastChild = lastChild->nextSibling();
	}
	element = new QListViewItem( stack.top(), lastChild, qName, namespaceURI );
    } else {
	element = new QListViewItem( table, qName, namespaceURI );
    }
    stack.push( element );
    element->setOpen( TRUE );

    if ( attributes.length() > 0 ) {
	for ( int i = 0 ; i < attributes.length(); i++ ) {
	    new QListViewItem( element, attributes.qName(i), attributes.uri(i) );
	}
    }
    return TRUE;
}

bool StructureParser::endElement( const QString&, const QString&,
                                  const QString& )
{
    stack.pop();
    return TRUE;
}
