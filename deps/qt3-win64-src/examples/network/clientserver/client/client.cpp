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
#include <qvbox.h>
#include <qhbox.h>
#include <qtextview.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtextstream.h>


class Client : public QVBox
{
    Q_OBJECT
public:
    Client( const QString &host, Q_UINT16 port )
    {
	// GUI layout
	infoText = new QTextView( this );
	QHBox *hb = new QHBox( this );
	inputText = new QLineEdit( hb );
	QPushButton *send = new QPushButton( tr("Send") , hb );
	QPushButton *close = new QPushButton( tr("Close connection") , this );
	QPushButton *quit = new QPushButton( tr("Quit") , this );

	connect( send, SIGNAL(clicked()), SLOT(sendToServer()) );
	connect( close, SIGNAL(clicked()), SLOT(closeConnection()) );
	connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );

	// create the socket and connect various of its signals
	socket = new QSocket( this );
	connect( socket, SIGNAL(connected()),
		SLOT(socketConnected()) );
	connect( socket, SIGNAL(connectionClosed()),
		SLOT(socketConnectionClosed()) );
	connect( socket, SIGNAL(readyRead()),
		SLOT(socketReadyRead()) );
	connect( socket, SIGNAL(error(int)),
		SLOT(socketError(int)) );

	// connect to the server
	infoText->append( tr("Trying to connect to the server\n") );
	socket->connectToHost( host, port );
    }

    ~Client()
    {
    }

private slots:
    void closeConnection()
    {
	socket->close();
	if ( socket->state() == QSocket::Closing ) {
	    // We have a delayed close.
	    connect( socket, SIGNAL(delayedCloseFinished()),
		    SLOT(socketClosed()) );
	} else {
	    // The socket is closed.
	    socketClosed();
	}
    }

    void sendToServer()
    {
	// write to the server
	QTextStream os(socket);
	os << inputText->text() << "\n";
	inputText->setText( "" );
    }

    void socketReadyRead()
    {
	// read from the server
	while ( socket->canReadLine() ) {
	    infoText->append( socket->readLine() );
	}
    }

    void socketConnected()
    {
	infoText->append( tr("Connected to server\n") );
    }

    void socketConnectionClosed()
    {
	infoText->append( tr("Connection closed by the server\n") );
    }

    void socketClosed()
    {
	infoText->append( tr("Connection closed\n") );
    }

    void socketError( int e )
    {
	infoText->append( tr("Error number %1 occurred\n").arg(e) );
    }

private:
    QSocket *socket;
    QTextView *infoText;
    QLineEdit *inputText;
};


int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    Client client( argc<2 ? "localhost" : argv[1], 4242 );
    app.setMainWidget( &client );
    client.show();
    return app.exec();
}

#include "client.moc"
