/****************************************************************************
** $Id: server.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qtextview.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <stdlib.h>

#include "server.h"



ServerInfo::ServerInfo( Q_UINT16 port, QWidget *parent, const char *name ) :
    ServerInfoBase( parent, name )
{
    SimpleServer *server = new SimpleServer( port, this, "simple server" );
    connect( server, SIGNAL(newConnect()), SLOT(newConnect()) );
    connect( btnQuit, SIGNAL(clicked()), qApp, SLOT(quit()) );
}

void ServerInfo::newConnect()
{
    infoText->append( tr( "New connection\n" ) );
}


SimpleServer::SimpleServer( Q_UINT16 port, QObject* parent, const char *name ) :
    QServerSocket( port, 1, parent, name )
{
    if ( !ok() ) {
	QMessageBox::critical( 0, tr( "Error" ), tr( "Failed to bind to port %1" ).arg( port ) );
	exit(1);
    }
}

void SimpleServer::newConnection( int socket )
{
    (void)new ClientSocket( socket, &info, this, "client socket" );
    emit newConnect();
}


ClientSocket::ClientSocket( int sock, InfoData *i, QObject *parent, const char *name ) :
    QSocket( parent, name ), info( i )
{
    connect( this, SIGNAL(readyRead()), SLOT(readClient()) );
    connect( this, SIGNAL(connectionClosed()), SLOT(connectionClosed()) );
    setSocket( sock );
}

void ClientSocket::readClient()
{
    QTextStream stream( this );
    QStringList answer;
    while ( canReadLine() ) {
	stream << processCommand( stream.readLine() );
    }
}

QString ClientSocket::processCommand( const QString& command )
{
    QString answer;
    QString com = command.simplifyWhiteSpace ();
    if ( com.startsWith( "LIST" ) ) {
	bool ok;
	QStringList nodes = info->list( com.mid( 5 ), &ok );
	if ( ok ) {
	    for ( QStringList::Iterator it = nodes.begin(); it != nodes.end(); ++it ) 
		answer += "212+" + *it + "\r\n";
	    answer += "212 \r\n";
	} else 
	    answer += "550 Invalid path\r\n";	
    } else if ( com.startsWith( "GET " ) ) {
	bool ok;
	QStringList data = QStringList::split( '\n', info->get( com.mid( 4 ), &ok ), TRUE );
	if ( ok ) {
	    for ( QStringList::Iterator it = data.begin(); it != data.end(); ++it ) 
		answer += "213+" + *it + "\r\n";
	    answer += "213 \r\n";
	} else 
	    answer += "550 Info not found\r\n";	
    } else 
	answer += "500 Syntax error\r\n";	

    return answer;
}

void ClientSocket::connectionClosed()
{
    delete this;
}
