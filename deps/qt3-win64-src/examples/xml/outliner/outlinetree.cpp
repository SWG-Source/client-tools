/****************************************************************************
** $Id: outlinetree.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "outlinetree.h"
#include <qfile.h>
#include <qmessagebox.h>

OutlineTree::OutlineTree( const QString fileName, QWidget *parent, const char *name )
    : QListView( parent, name )
{
    // div. configuration of the list view
    addColumn( "Outlines" );
    setSorting( -1 );
    setRootIsDecorated( TRUE );

    // read the XML file and create DOM tree
    QFile opmlFile( fileName );
    if ( !opmlFile.open( IO_ReadOnly ) ) {
	QMessageBox::critical( 0,
		tr( "Critical Error" ),
		tr( "Cannot open file %1" ).arg( fileName ) );
	return;
    }
    if ( !domTree.setContent( &opmlFile ) ) {
	QMessageBox::critical( 0,
		tr( "Critical Error" ),
		tr( "Parsing error for file %1" ).arg( fileName ) );
	opmlFile.close();
	return;
    }
    opmlFile.close();

    // get the header information from the DOM
    QDomElement root = domTree.documentElement();
    QDomNode node;
    node = root.firstChild();
    while ( !node.isNull() ) {
	if ( node.isElement() && node.nodeName() == "head" ) {
	    QDomElement header = node.toElement();
	    getHeaderInformation( header );
	    break;
	}
	node = node.nextSibling();
    }
    // create the tree view out of the DOM
    node = root.firstChild();
    while ( !node.isNull() ) {
	if ( node.isElement() && node.nodeName() == "body" ) {
	    QDomElement body = node.toElement();
	    buildTree( 0, body );
	    break;
	}
	node = node.nextSibling();
    }
}

OutlineTree::~OutlineTree()
{
}

void OutlineTree::getHeaderInformation( const QDomElement &header )
{
    // visit all children of the header element and look if you can make
    // something with it
    QDomNode node = header.firstChild();
    while ( !node.isNull() ) {
	if ( node.isElement() ) {
	    // case for the different header entries
	    if ( node.nodeName() == "title" ) {
		QDomText textChild = node.firstChild().toText();
		if ( !textChild.isNull() ) {
		    setColumnText( 0, textChild.nodeValue() );
		}
	    }
	}
	node = node.nextSibling();
    }
}

void OutlineTree::buildTree( QListViewItem *parentItem, const QDomElement &parentElement )
{
    QListViewItem *thisItem = 0;
    QDomNode node = parentElement.firstChild();
    while ( !node.isNull() ) {
	if ( node.isElement() && node.nodeName() == "outline" ) {
	    // add a new list view item for the outline
	    if ( parentItem == 0 )
		thisItem = new QListViewItem( this, thisItem );
	    else
		thisItem = new QListViewItem( parentItem, thisItem );
	    thisItem->setText( 0, node.toElement().attribute( "text" ) );
	    // recursive build of the tree
	    buildTree( thisItem, node.toElement() );
	}
	node = node.nextSibling();
    }
}
