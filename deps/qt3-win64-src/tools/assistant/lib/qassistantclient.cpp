/**********************************************************************
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the QAssistantClient library.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qassistantclient.h"

#include <qsocket.h>
#include <qtextstream.h>
#include <qprocess.h>
#include <qtimer.h>
#include <qfileinfo.h>

class QAssistantClientPrivate
{
    friend class QAssistantClient;
    QStringList arguments;
};

static QMap<const QAssistantClient*,QAssistantClientPrivate*> *dpointers = 0;

static QAssistantClientPrivate *data( const QAssistantClient *client, bool create=FALSE )
{
    if( !dpointers )
	dpointers = new QMap<const QAssistantClient*,QAssistantClientPrivate*>;
    QAssistantClientPrivate *d = (*dpointers)[client];
    if( !d && create ) {
	d = new QAssistantClientPrivate;
	dpointers->insert( client, d );
    }
    return d;
}

/*!
    \class QAssistantClient
    \brief The QAssistantClient class provides a means of using Qt
    Assistant as an application's help tool.

    Using Qt Assistant is simple: Create a QAssistantClient instance,
    then call showPage() as often as necessary to show your help
    pages. When you call showPage(), Qt Assistant will be launched if
    it isn't already running.

    The QAssistantClient instance can open (openAssistant()) or close
    (closeAssistant()) Qt Assistant whenever required. If Qt Assistant
    is open, isOpen() returns TRUE.

    One QAssistantClient instance interacts with one Qt Assistant
    instance, so every time you call openAssistant(), showPage() or
    closeAssistant() they are applied to the particular Qt Assistant
    instance associated with the QAssistantClient.

    When you call openAssistant() the assistantOpened() signal is
    emitted. Similarly when closeAssistant() is called,
    assistantClosed() is emitted. In either case, if an error occurs,
    error() is emitted.

    This class is not included in the Qt library itself. To use it you
    must link against \c libqassistantclient.a (Unix) or \c
    qassistantclient.lib (Windows), which is built into \c INSTALL/lib
    if you built the Qt tools (\c INSTALL is the directory where Qt is
    installed). If you use qmake, then you can simply add the following
    line to your pro file:

    \code
	LIBS += -lqassistantclient
    \endcode

    See also "Adding Documentation to Qt Assistant" in the \link
    assistant.book Qt Assistant manual\endlink.
*/

/*!
    \fn void QAssistantClient::assistantOpened()

    This signal is emitted when Qt Assistant is open and the
    client-server communication is set up.
*/

/*!
    \fn void QAssistantClient::assistantClosed()

    This signal is emitted when the connection to Qt Assistant is
    closed. This happens when the user exits Qt Assistant, or when an
    error in the server or client occurs, or if closeAssistant() is
    called.
*/

/*!
    \fn void QAssistantClient::error( const QString &msg )

    This signal is emitted if Qt Assistant cannot be started or if an
    error occurs during the initialization of the connection between
    Qt Assistant and the calling application. The \a msg provides an
    explanation of the error.
*/

/*!
    Constructs an assistant client object. The \a path specifies the
    path to the Qt Assistant executable. If \a path is an empty
    string the system path (\c{%PATH%} or \c $PATH) is used.

    The assistant client object is a child of \a parent and is called
    \a name.
*/
QAssistantClient::QAssistantClient( const QString &path, QObject *parent, const char *name )
    : QObject( parent, name ), host ( "localhost" )
{
    if ( path.isEmpty() )
	assistantCommand = "assistant";
    else {
	QFileInfo fi( path );
	if ( fi.isDir() )
	    assistantCommand = path + "/assistant";
	else
	    assistantCommand = path;
    }

#if defined(Q_OS_MACX)
    assistantCommand += ".app/Contents/MacOS/assistant";
#elif defined(Q_WS_WIN)
    if (!assistantCommand.endsWith(".exe"))
        assistantCommand += ".exe";
#endif
    socket = new QSocket( this );
    connect( socket, SIGNAL( connected() ),
	    SLOT( socketConnected() ) );
    connect( socket, SIGNAL( connectionClosed() ),
	    SLOT( socketConnectionClosed() ) );
    connect( socket, SIGNAL( error( int ) ),
	    SLOT( socketError( int ) ) );
    opened = FALSE;
    proc = new QProcess( this );
    port = 0;
    pageBuffer = "";
    connect( proc, SIGNAL( readyReadStderr() ),
	     this, SLOT( readStdError() ) );
}

/*!
    Destroys the assistant client object and frees up all allocated
    resources.
*/
QAssistantClient::~QAssistantClient()
{
    if ( proc && proc->isRunning() ) {
	proc->tryTerminate();
	proc->kill();
    }

    if( dpointers ) {
	QAssistantClientPrivate *d = (*dpointers)[ this ];
	if( d ) {
	    dpointers->remove( this );
	    delete d;
	    if( dpointers->isEmpty() ) {
		delete dpointers;
		dpointers = 0;
	    }
	}
    }
}

/*!
    This function opens Qt Assistant and sets up the client-server
    communiction between the application and Qt Assistant. If it is
    already open, this function does nothing. If an error occurs,
    error() is emitted.

    \sa assistantOpened()
*/
void QAssistantClient::openAssistant()
{
    if ( proc->isRunning() )
	return;
    proc->clearArguments();
    proc->addArgument( assistantCommand );
    proc->addArgument( "-server" );
    if( !pageBuffer.isEmpty() ) {
        proc->addArgument( "-file" );
        proc->addArgument( pageBuffer );
    }

    QAssistantClientPrivate *d = data( this );
    if( d ) {
	QStringList::ConstIterator it = d->arguments.begin();
	while( it!=d->arguments.end() ) {
	    proc->addArgument( *it );
	    ++it;
	}
    }

    if ( !proc->launch( QString::null ) ) {
	emit error( tr( "Cannot start Qt Assistant '%1'" )
		    .arg( proc->arguments().join( " " ) ) );
	return;
    }
    connect( proc, SIGNAL( readyReadStdout() ),
	     this, SLOT( readPort() ) );
}

void QAssistantClient::readPort()
{
    QString p = proc->readLineStdout();
    Q_UINT16 port = p.toUShort();
    if ( port == 0 ) {
	emit error( tr( "Cannot connect to Qt Assistant." ) );
	return;
    }
    socket->connectToHost( host, port );
    disconnect( proc, SIGNAL( readyReadStdout() ),
		this, SLOT( readPort() ) );
}

/*!
    Use this function to close Qt Assistant.

    \sa assistantClosed()
*/
void QAssistantClient::closeAssistant()
{
    if ( !opened )
	return;
    proc->tryTerminate();
    proc->kill();
}

/*!
    Call this function to make Qt Assistant show a particular \a page.
    The \a page is a filename (e.g. \c myhelpfile.html). See "Adding
    Documentation to Qt Assistant" in the \link assistant.book Qt
    Assistant manual\endlink for further information.

    If Qt Assistant hasn't been \link openAssistant() opened\endlink
    yet, this function will do nothing. You can use isOpen() to
    determine whether Qt Assistant is up and running, or you can
    connect to the asssistantOpened() signal.

    \sa isOpen(), assistantOpened()
*/
void QAssistantClient::showPage( const QString &page )
{
    if ( !opened ) {
	pageBuffer = page;
	openAssistant();
	pageBuffer = QString::null;	
	return;
    }
    QTextStream os( socket );
    os << page << "\n";
}

/*!
    \property QAssistantClient::open
    \brief Whether Qt Assistant is open.

*/
bool QAssistantClient::isOpen() const
{
    return opened;
}

void QAssistantClient::socketConnected()
{
    opened = TRUE;
    if ( !pageBuffer.isEmpty() )
	showPage( pageBuffer );
    emit assistantOpened();
}

void QAssistantClient::socketConnectionClosed()
{
    opened = FALSE;
    emit assistantClosed();
}

void QAssistantClient::socketError( int i )
{
    if ( i == QSocket::ErrConnectionRefused )
	emit error( tr( "Could not connect to Assistant: Connection refused" ) );
    else if ( i == QSocket::ErrHostNotFound )
	emit error( tr( "Could not connect to Assistant: Host not found" ) );
    else
	emit error( tr( "Communication error" ) );
}

void QAssistantClient::readStdError()
{
    QString errmsg;
    while ( proc->canReadLineStderr() ) {
	errmsg += proc->readLineStderr();
	errmsg += "\n";
    }
    if (!errmsg.isEmpty())
	emit error( tr( errmsg.simplifyWhiteSpace() ) );
}

/*!
    Sets the command line arguments used when Qt Assistant is
    started to \a args.
*/
void QAssistantClient::setArguments( const QStringList &args )
{
    QAssistantClientPrivate *d = data( this, TRUE );
    d->arguments = args;
}
