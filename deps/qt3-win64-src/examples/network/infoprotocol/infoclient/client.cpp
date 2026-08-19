/****************************************************************************
** $Id: client.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qsocket.h>
#include <qapplication.h>
#include <qtextedit.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <qlistbox.h>

#include "client.h"


ClientInfo::ClientInfo( QWidget *parent, const char *name ) :
    ClientInfoBase( parent, name ), socket( 0 )
{
    edHost->setText( "localhost" );
    edPort->setText( QString::number( (uint)infoPort ) );

    connect( infoList, SIGNAL(selected(const QString&)), SLOT(selectItem(const QString&)) );
    connect( btnConnect, SIGNAL(clicked()), SLOT(connectToServer()) );
    connect( btnBack, SIGNAL(clicked()), SLOT(stepBack()) );
    connect( btnQuit, SIGNAL(clicked()), qApp, SLOT(quit()) );
}


void ClientInfo::connectToServer()
{
    delete socket;
    socket = new QSocket( this );
    connect( socket, SIGNAL(connected()), SLOT(socketConnected()) );
    connect( socket, SIGNAL(connectionClosed()), SLOT(socketConnectionClosed()) );
    connect( socket, SIGNAL(readyRead()), SLOT(socketReadyRead()) );
    connect( socket, SIGNAL(error(int)), SLOT(socketError(int)) );

    socket->connectToHost( edHost->text(), edPort->text().toInt() );
}

void ClientInfo::selectItem( const QString& item )
{
    // item in listBox selected, use LIST or GET depending of the node type.
    if ( item.endsWith( "/" ) ) {
	sendToServer( List, infoPath->text() + item );
	infoPath->setText( infoPath->text() + item );
    } else 
	sendToServer( Get, infoPath->text() + item );
}

void ClientInfo::stepBack()
{
    // go back (up) in path hierarchy
    int i = infoPath->text().findRev( '/', -2 );
    if ( i > 0 ) 
	infoPath->setText( infoPath->text().left( i + 1 ) );
    else
	infoPath->setText( "/" );
    infoList->clear();
    sendToServer( List, infoPath->text() );
}


void ClientInfo::socketConnected()
{
    sendToServer( List, "/" );
}

void ClientInfo::sendToServer( Operation op, const QString& location )
{
    QString line;
    switch (op) {
	case List:
	    infoList->clear();
	    line = "LIST " + location;
	    break;
	case Get:
	    line = "GET " + location;
	    break;
    }
    infoText->clear();
    QTextStream os(socket);
    os << line << "\r\n";
}

void ClientInfo::socketReadyRead()
{
    QTextStream stream( socket );
    QString line;
    while ( socket->canReadLine() ) {
	line = stream.readLine();
	if ( line.startsWith( "500" ) || line.startsWith( "550" ) ) {
	    infoText->append( tr( "error: " ) + line.mid( 4 ) );
	} else if ( line.startsWith( "212+" ) ) {
	    infoList->insertItem( line.mid( 6 ) + QString( ( line[ 4 ] == 'D' ) ? "/" : "" ) );
	} else if ( line.startsWith( "213+" ) ) {
	    infoText->append( line.mid( 4 ) );
	}
    }
}


void ClientInfo::socketConnectionClosed()
{
    infoText->clear();
    infoText->append( tr( "error: Connection closed by the server\n" ) );
}

void ClientInfo::socketError( int code )
{
    infoText->clear();
    infoText->append( tr( "error: Error number %1 occurred\n" ).arg( code ) );
}

