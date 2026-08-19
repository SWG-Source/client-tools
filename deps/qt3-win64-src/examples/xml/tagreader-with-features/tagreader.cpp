/****************************************************************************
** $Id: tagreader.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "structureparser.h"
#include <qapplication.h>
#include <qfile.h>
#include <qxml.h>
#include <qlistview.h>
#include <qgrid.h>
#include <qmainwindow.h>
#include <qlabel.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    QFile xmlFile( argc == 2 ? argv[1] : "fnord.xml" );
    QXmlInputSource source( &xmlFile );

    QXmlSimpleReader reader;

    QGrid * container = new QGrid( 3 );

    QListView * nameSpace = new QListView( container, "table_namespace" );
    StructureParser * handler = new StructureParser( nameSpace );
    reader.setContentHandler( handler );
    reader.parse( source );

    QListView * namespacePrefix = new QListView( container,
                                                 "table_namespace_prefix" );
    handler->setListView( namespacePrefix );
    reader.setFeature( "http://xml.org/sax/features/namespace-prefixes",
                       TRUE );
    source.reset();
    reader.parse( source );

    QListView * prefix = new QListView( container, "table_prefix");
    handler->setListView( prefix );
    reader.setFeature( "http://xml.org/sax/features/namespaces", FALSE );
    source.reset();
    reader.parse( source );

    // namespace label
    (void) new QLabel(
             "Default:\n"
             "http://xml.org/sax/features/namespaces: TRUE\n"
             "http://xml.org/sax/features/namespace-prefixes: FALSE\n",
             container );

    // namespace prefix label
    (void) new QLabel(
             "\n"
             "http://xml.org/sax/features/namespaces: TRUE\n"
             "http://xml.org/sax/features/namespace-prefixes: TRUE\n",
             container );

    // prefix label
    (void) new QLabel(
             "\n"
             "http://xml.org/sax/features/namespaces: FALSE\n"
             "http://xml.org/sax/features/namespace-prefixes: TRUE\n",
             container );


    app.setMainWidget( container );
    container->show();
    return app.exec();
}
