/****************************************************************************
** $Id: smtp.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "smtp.h"

#include <qtextstream.h>
#include <qsocket.h>
#include <qdns.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qregexp.h>


Smtp::Smtp( const QString &from, const QString &to,
	    const QString &subject,
	    const QString &body )
{
    socket = new QSocket( this );
    connect ( socket, SIGNAL( readyRead() ),
	      this, SLOT( readyRead() ) );
    connect ( socket, SIGNAL( connected() ),
	      this, SLOT( connected() ) );

    mxLookup = new QDns( to.mid( to.find( '@' )+1 ), QDns::Mx );
    connect( mxLookup, SIGNAL(resultsReady()),
	     this, SLOT(dnsLookupHelper()) );

    message = QString::fromLatin1( "From: " ) + from +
	      QString::fromLatin1( "\nTo: " ) + to +
	      QString::fromLatin1( "\nSubject: " ) + subject +
	      QString::fromLatin1( "\n\n" ) + body + "\n";
    message.replace( QString::fromLatin1( "\n" ),
		     QString::fromLatin1( "\r\n" ) );
    message.replace( QString::fromLatin1( "\r\n.\r\n" ),
		     QString::fromLatin1( "\r\n..\r\n" ) );

    this->from = from;
    rcpt = to;

    state = Init;
}


Smtp::~Smtp()
{
    delete t;
    delete socket;
}


void Smtp::dnsLookupHelper()
{
    QValueList<QDns::MailServer> s = mxLookup->mailServers();
    if ( s.isEmpty() ) {
	if ( !mxLookup->isWorking() )
	    emit status( tr( "Error in MX record lookup" ) );
	return;
    }

    emit status( tr( "Connecting to %1" ).arg( s.first().name ) );

    socket->connectToHost( s.first().name, 25 );
    t = new QTextStream( socket );
}


void Smtp::connected()
{
    emit status( tr( "Connected to %1" ).arg( socket->peerName() ) );
}

void Smtp::readyRead()
{
    // SMTP is line-oriented
    if ( !socket->canReadLine() )
	return;

    QString responseLine;
    do {
	responseLine = socket->readLine();
	response += responseLine;
    } while( socket->canReadLine() && responseLine[3] != ' ' );
    responseLine.truncate( 3 );

    if ( state == Init && responseLine[0] == '2' ) {
	// banner was okay, let's go on
	*t << "HELO there\r\n";
	state = Mail;
    } else if ( state == Mail && responseLine[0] == '2' ) {
	// HELO response was okay (well, it has to be)
	*t << "MAIL FROM: <" << from << ">\r\n";
	state = Rcpt;
    } else if ( state == Rcpt && responseLine[0] == '2' ) {
	*t << "RCPT TO: <" << rcpt << ">\r\n";
	state = Data;
    } else if ( state == Data && responseLine[0] == '2' ) {
	*t << "DATA\r\n";
	state = Body;
    } else if ( state == Body && responseLine[0] == '3' ) {
	*t << message << ".\r\n";
	state = Quit;
    } else if ( state == Quit && responseLine[0] == '2' ) {
	*t << "QUIT\r\n";
	// here, we just close.
	state = Close;
	emit status( tr( "Message sent" ) );
    } else if ( state == Close ) {
	deleteLater();
	return;
    } else {
	// something broke.
	QMessageBox::warning( qApp->activeWindow(),
			      tr( "Qt Mail Example" ),
			      tr( "Unexpected reply from SMTP server:\n\n" ) +
			      response );
	state = Close;
    }

    response = "";
}
