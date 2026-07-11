/****************************************************************************
** $Id: qip.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qsocket.h>
#include <qurlinfo.h>
#include <qurloperator.h>
#include <qtextstream.h>

#include "qip.h"


Qip::Qip()
{
    state = Start;
    socket = new QSocket( this );
    connect( socket, SIGNAL(connected()), SLOT(socketConnected()) );
    connect( socket, SIGNAL(connectionClosed()), SLOT(socketConnectionClosed()) );
    connect( socket, SIGNAL(readyRead()), SLOT(socketReadyRead()) );
    connect( socket, SIGNAL(error(int)), SLOT(socketError(int)) );
}

int Qip::supportedOperations() const
{
    return OpListChildren | OpGet;
}

bool Qip::checkConnection( QNetworkOperation * )
{
    if ( socket->isOpen() )
	return TRUE;

    // don't call connectToHost() if we are already trying to connect
    if ( socket->state() == QSocket::Connecting )
	return FALSE;

    socket->connectToHost( url()->host(), url()->port() != -1 ? url()->port() : infoPort );
    return FALSE;
}

void Qip::operationListChildren( QNetworkOperation * )
{
    QTextStream os(socket);
    os << "LIST " + url()->path() + "\r\n";
    operationInProgress()->setState( StInProgress );
}

void Qip::operationGet( QNetworkOperation * )
{
    QTextStream os(socket);
    os << "GET " + url()->path() + "\r\n";
    operationInProgress()->setState( StInProgress );
}

void Qip::socketConnected()
{
    if ( url() )
	emit connectionStateChanged( ConConnected, tr( "Connected to host %1" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConConnected, tr ("Connected to host" ) );
}

void Qip::socketConnectionClosed()
{
    if ( url() )
	emit connectionStateChanged( ConClosed, tr( "Connection to %1 closed" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConClosed, tr ("Connection closed" ) );
}

void Qip::socketError( int code )
{
    if ( code == QSocket::ErrHostNotFound ||
	 code == QSocket::ErrConnectionRefused ) {
	error( ErrHostNotFound, tr( "Host not found or couldn't connect to: %1\n" ).arg( url()->host() ) );
    } else
	error( ErrUnsupported, tr( "Error" ) );
}

void Qip::socketReadyRead()
{
    // read from the server
    QTextStream stream( socket );
    QString line;
    while ( socket->canReadLine() ) {
	line = stream.readLine();
	if ( line.startsWith( "500" ) ) {
	    error( ErrValid, line.mid( 4 ) ); 
	} else if ( line.startsWith( "550" ) ) {
	    error( ErrFileNotExisting, line.mid( 4 ) ); 
	} else if ( line.startsWith( "212+" ) ) {
	    if ( state != List ) {
		state = List;
	        emit start( operationInProgress() );
	    }
	    QUrlInfo inf;
	    inf.setName( line.mid( 6 ) + QString( ( line[ 4 ] == 'D' ) ? "/" : "" ) );
	    inf.setDir( line[ 4 ] == 'D' );
	    inf.setSymLink( FALSE );
	    inf.setFile( line[ 4 ] == 'F' );
	    inf.setWritable( FALSE );
	    inf.setReadable( TRUE );
	    emit newChild( inf, operationInProgress() );
	} else if ( line.startsWith( "213+" ) ) {
	    state = Data;
	    emit data( line.mid( 4 ).utf8(), operationInProgress() );
	}
	if( line[3] == ' ' && state != Start) {
	    state = Start;
	    operationInProgress()->setState( StDone );
	    emit finished( operationInProgress() );
	}
    }
}

void Qip::error( int code, const QString& msg )
{
    if ( operationInProgress() ) {
	operationInProgress()->setState( StFailed );
	operationInProgress()->setErrorCode( code );
	operationInProgress()->setProtocolDetail( msg );
	clearOperationQueue();
	emit finished( operationInProgress() );
    }
    state = Start;
}

