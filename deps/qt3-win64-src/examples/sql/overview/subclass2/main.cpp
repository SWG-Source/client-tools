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
    // NOOP
}


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( createConnections() ) {
	InvoiceItemCursor invoiceItemCursor;

	QDataTable *invoiceItemTable = new QDataTable( &invoiceItemCursor );

	app.setMainWidget( invoiceItemTable );

	invoiceItemTable->addColumn( "pricesid", "PriceID" );
	invoiceItemTable->addColumn( "quantity", "Quantity" );
	invoiceItemTable->addColumn( "paiddate", "Paid" );

	invoiceItemTable->refresh();
	invoiceItemTable->show();

	return app.exec();
    }

    return 1;
}
