/****************************************************************************
** $Id: httpd.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/
#include <stdlib.h>
#include <qsocket.h>
#include <qregexp.h>
#include <qserversocket.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qtextstream.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qtextview.h>
#include <qpushbutton.h>

// HttpDaemon is the the class that implements the simple HTTP server.
class HttpDaemon : public QServerSocket
{
    Q_OBJECT
public:
    HttpDaemon( QObject* parent=0 ) :
	QServerSocket(8080,1,parent)
    {
	if ( !ok() ) {
	    qWarning("Failed to bind to port 8080");
	    exit( 1 );
	}
    }

    void newConnection( int socket )
    {
	// When a new client connects, the server constructs a QSocket and all
	// communication with the client is done over this QSocket. QSocket
	// works asynchronouslyl, this means that all the communication is done
	// in the two slots readClient() and discardClient().
	QSocket* s = new QSocket( this );
	connect( s, SIGNAL(readyRead()), this, SLOT(readClient()) );
	connect( s, SIGNAL(delayedCloseFinished()), this, SLOT(discardClient()) );
	s->setSocket( socket );
	emit newConnect();
    }

signals:
    void newConnect();
    void endConnect();
    void wroteToClient();

private slots:
    void readClient()
    {
	// This slot is called when the client sent data to the server. The
	// server looks if it was a get request and sends a very simple HTML
	// document back.
	QSocket* socket = (QSocket*)sender();
	if ( socket->canReadLine() ) {
	    QStringList tokens = QStringList::split( QRegExp("[ \r\n][ \r\n]*"), socket->readLine() );
	    if ( tokens[0] == "GET" ) {
		QTextStream os( socket );
		os.setEncoding( QTextStream::UnicodeUTF8 );
		os << "HTTP/1.0 200 Ok\r\n"
		    "Content-Type: text/html; charset=\"utf-8\"\r\n"
		    "\r\n"
		    "<h1>Nothing to see here</h1>\n";
		socket->close();
		emit wroteToClient();
	    }
	}
    }
    void discardClient()
    {
	QSocket* socket = (QSocket*)sender();
	delete socket;
	emit endConnect();
    }
};


// HttpInfo provides a simple graphical user interface to the server and shows
// the actions of the server.
class HttpInfo : public QVBox
{
    Q_OBJECT
public:
    HttpInfo()
    {
	HttpDaemon *httpd = new HttpDaemon( this );

	QString itext = QString(
		"This is a small httpd example.\n"
		"You can connect with your\n"
		"web browser to port %1"
	    ).arg( httpd->port() );
	QLabel *lb = new QLabel( itext, this );
	lb->setAlignment( AlignHCenter );
	infoText = new QTextView( this );
	QPushButton *quit = new QPushButton( "quit" , this );

	connect( httpd, SIGNAL(newConnect()), SLOT(newConnect()) );
	connect( httpd, SIGNAL(endConnect()), SLOT(endConnect()) );
	connect( httpd, SIGNAL(wroteToClient()), SLOT(wroteToClient()) );
	connect( quit, SIGNAL(pressed()), qApp, SLOT(quit()) );
    }

    ~HttpInfo()
    {
    }

private slots:
    void newConnect()
    {
	infoText->append( "New connection" );
    }
    void endConnect()
    {
	infoText->append( "Connection closed\n\n" );
    }
    void wroteToClient()
    {
	infoText->append( "Wrote to client" );
    }

private:
    QTextView *infoText;
};


int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    HttpInfo info;
    app.setMainWidget( &info );
    info.show();
    return app.exec();
}

#include "httpd.moc"
