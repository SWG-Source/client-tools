/****************************************************************************
** $Id: client.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qfiledialog.h>

#include "qip.h"
#include "client.h"




ClientInfo::ClientInfo( QWidget *parent, const char *name ) :
    ClientInfoBase( parent, name )
{
    connect( btnOpen, SIGNAL(clicked()), SLOT(downloadFile()) );
    connect( btnQuit, SIGNAL(clicked()), qApp, SLOT(quit()) );
    connect( &op, SIGNAL( data( const QByteArray &, QNetworkOperation * ) ),
	     this, SLOT( newData( const QByteArray & ) ) );
}

void ClientInfo::downloadFile()
{
    // under Windows you must not use the native file dialog
    QString file = getOpenFileName();
    if ( !file.isEmpty() ) {
	infoText->clear();
	// download the data
	op = file;
	op.get();
    }
}

QString ClientInfo::getOpenFileName()
{
    static QString workingDirectory( "qip://localhost/" );

    QFileDialog dlg( workingDirectory, QString::null, 0, 0, TRUE );
    dlg.setCaption( QFileDialog::tr( "Open" ) );
    dlg.setMode( QFileDialog::ExistingFile );
    QString result;
    if ( dlg.exec() == QDialog::Accepted ) {
	result = dlg.selectedFile();
	workingDirectory = dlg.url();
    }
    return result;
}

void ClientInfo::newData( const QByteArray &ba )
{
    infoText->append( QString::fromUtf8( ba ) );
}
