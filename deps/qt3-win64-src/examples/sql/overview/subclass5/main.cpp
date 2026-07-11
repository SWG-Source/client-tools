/****************************************************************************
** $Id: main.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "main.h"
#include <qdatatable.h>

InvoiceItemCursor::InvoiceItemCursor() :
    QSqlCursor( "invoiceitem" )
{
    QSqlFieldInfo productName( "productname", QVariant::String );
    append( productName );
    setCalculated( productName.name(), TRUE );

    QSqlFieldInfo productPrice( "price", QVariant::Double );
    append( productPrice );
    setCalculated( productPrice.name(), TRUE );

    QSqlFieldInfo productCost( "cost", QVariant::Double );
    append( productCost );
    setCalculated( productCost.name(), TRUE );
}


QVariant InvoiceItemCursor::calculateField( const QString & name )
{

    if ( name == "productname" ) {
	QSqlQuery query( "SELECT name FROM prices WHERE id=" +
		     field( "pricesid" )->value().toString() );
	if ( query.next() )
	    return query.value( 0 );
    }
    else if ( name == "price" ) {
	QSqlQuery query( "SELECT price FROM prices WHERE id=" +
		     field( "pricesid" )->value().toString() );
	if ( query.next() )
	    return query.value( 0 );
    }
    else if ( name == "cost" ) {
	QSqlQuery query( "SELECT price FROM prices WHERE id=" +
		     field( "pricesid" )->value().toString() );
	if ( query.next() )
	    return QVariant( query.value( 0 ).toDouble() *
			     value( "quantity").toDouble() );
    }

    return QVariant( QString::null );
}


QSqlRecord *InvoiceItemCursor::primeInsert()
{
    QSqlRecord *buffer = editBuffer();
    QSqlQuery query( "SELECT NEXTVAL( 'invoiceitem_seq' )" );
    if ( query.next() )
	buffer->setValue( "id", query.value( 0 ) );
    buffer->setValue( "paiddate", QDate::currentDate() );
    buffer->setValue( "quantity", 1 );

    return buffer;
}


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( createConnections() ) {
	InvoiceItemCursor invoiceItemCursor;

	QDataTable *invoiceItemTable = new QDataTable( &invoiceItemCursor );

	app.setMainWidget( invoiceItemTable );

	invoiceItemTable->addColumn( "productname", "Product" );
	invoiceItemTable->addColumn( "price",	    "Price" );
	invoiceItemTable->addColumn( "quantity",    "Quantity" );
	invoiceItemTable->addColumn( "cost",	    "Cost" );
	invoiceItemTable->addColumn( "paiddate",    "Paid" );

	invoiceItemTable->refresh();
	invoiceItemTable->show();

	return app.exec();
    }

    return 1;
}
