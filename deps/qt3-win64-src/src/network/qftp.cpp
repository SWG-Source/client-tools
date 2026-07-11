/****************************************************************************
** $Id: qftp.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QFtp class.
**
** Created : 970521
**
** Copyright (C) 1997-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech ASA of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qftp.h"

#ifndef QT_NO_NETWORKPROTOCOL_FTP

#include "qsocket.h"
#include "qsocketdevice.h"
#include "qurlinfo.h"
#include "qurloperator.h"
#include "qstringlist.h"
#include "qregexp.h"
#include "qtimer.h"
#include "qfileinfo.h"
#include "qptrdict.h" // binary compatibility

#ifndef QT_NO_TEXTCODEC
#include "qtextcodec.h"
#endif

//#define QFTPPI_DEBUG
//#define QFTPDTP_DEBUG


class QFtpPI;

class QFtpDTP : public QObject
{
    Q_OBJECT

public:
    enum ConnectState {
	CsHostFound,
	CsConnected,
	CsClosed,
	CsHostNotFound,
	CsConnectionRefused
    };

    QFtpDTP( QFtpPI *p, QObject *parent=0, const char *name=0 );

    void setData( QByteArray * );
    void setDevice( QIODevice * );
    void writeData();

    void setBytesTotal( int bytes )
    {
	bytesTotal = bytes;
	bytesDone = 0;
	emit dataTransferProgress( bytesDone, bytesTotal );
    }

    bool hasError() const;
    QString errorMessage() const;
    void clearError();

    void connectToHost( const QString & host, Q_UINT16 port )
    { socket.connectToHost( host, port ); }

    QSocket::State socketState() const
    { return socket.state(); }

    Q_ULONG bytesAvailable() const
    { return socket.bytesAvailable(); }

    Q_LONG readBlock( char *data, Q_ULONG maxlen )
    {
	Q_LONG read = socket.readBlock( data, maxlen );
	bytesDone += read;
	return read;
    }

    QByteArray readAll()
    {
	QByteArray tmp = socket.readAll();
	bytesDone += tmp.size();
	return tmp;
    }

    void abortConnection();

    static bool parseDir( const QString &buffer, const QString &userName, QUrlInfo *info );

signals:
    void listInfo( const QUrlInfo& );
    void readyRead();
    void dataTransferProgress( int, int );

    void connectState( int );

private slots:
    void socketConnected();
    void socketReadyRead();
    void socketError( int );
    void socketConnectionClosed();
    void socketBytesWritten( int );

private:
    void clearData()
    {
	is_ba = FALSE;
	data.dev = 0;
    }

    QSocket socket;
    QFtpPI *pi;
    QString err;
    int bytesDone;
    int bytesTotal;
    bool callWriteData;

    // If is_ba is TRUE, ba is used; ba is never 0.
    // Otherwise dev is used; dev can be 0 or not.
    union {
	QByteArray *ba;
	QIODevice *dev;
    } data;
    bool is_ba;
};

class QFtpPI : public QObject
{
    Q_OBJECT

public:
    QFtpPI( QObject *parent = 0 );

    void connectToHost( const QString &host, Q_UINT16 port );

    bool sendCommands( const QStringList &cmds );
    bool sendCommand( const QString &cmd )
    { return sendCommands( QStringList( cmd ) ); }

    void clearPendingCommands();
    void abort();

    QString currentCommand() const
    { return currentCmd; }

    bool rawCommand;

    QFtpDTP dtp; // the PI has a DTP which is not the design of RFC 959, but it
		 // makes the design simpler this way
signals:
    void connectState( int );
    void finished( const QString& );
    void error( int, const QString& );
    void rawFtpReply( int, const QString& );

private slots:
    void hostFound();
    void connected();
    void connectionClosed();
    void delayedCloseFinished();
    void readyRead();
    void error( int );

    void dtpConnectState( int );

private:
    // the states are modelled after the generalized state diagram of RFC 959,
    // page 58
    enum State {
	Begin,
	Idle,
	Waiting,
	Success,
	Failure
    };

    enum AbortState {
	None,
	AbortStarted,
	WaitForAbortToFinish
    };

    bool processReply();
    bool startNextCmd();

    QSocket commandSocket;
    QString replyText;
    char replyCode[3];
    State state;
    AbortState abortState;
    QStringList pendingCommands;
    QString currentCmd;

    bool waitForDtpToConnect;
    bool waitForDtpToClose;
};

/**********************************************************************
 *
 * QFtpCommand implemenatation
 *
 *********************************************************************/
class QFtpCommand
{
public:
    QFtpCommand( QFtp::Command cmd, QStringList raw );
    QFtpCommand( QFtp::Command cmd, QStringList raw, const QByteArray &ba );
    QFtpCommand( QFtp::Command cmd, QStringList raw, QIODevice *dev );
    ~QFtpCommand();

    int id;
    QFtp::Command command;
    QStringList rawCmds;

    // If is_ba is TRUE, ba is used; ba is never 0.
    // Otherwise dev is used; dev can be 0 or not.
    union {
	QByteArray *ba;
	QIODevice *dev;
    } data;
    bool is_ba;

    static int idCounter;
};

int QFtpCommand::idCounter = 0;

QFtpCommand::QFtpCommand( QFtp::Command cmd, QStringList raw )
    : command(cmd), rawCmds(raw), is_ba(FALSE)
{
    id = ++idCounter;
    data.dev = 0;
}

QFtpCommand::QFtpCommand( QFtp::Command cmd, QStringList raw, const QByteArray &ba )
    : command(cmd), rawCmds(raw), is_ba(TRUE)
{
    id = ++idCounter;
    data.ba = new QByteArray( ba );
}

QFtpCommand::QFtpCommand( QFtp::Command cmd, QStringList raw, QIODevice *dev )
    : command(cmd), rawCmds(raw), is_ba(FALSE)
{
    id = ++idCounter;
    data.dev = dev;
}

QFtpCommand::~QFtpCommand()
{
    if ( is_ba )
	delete data.ba;
}

/**********************************************************************
 *
 * QFtpDTP implemenatation
 *
 *********************************************************************/
QFtpDTP::QFtpDTP( QFtpPI *p, QObject *parent, const char *name ) :
    QObject( parent, name ),
    socket( 0, "QFtpDTP_socket" ),
    pi( p ),
    callWriteData( FALSE )
{
    clearData();

    connect( &socket, SIGNAL( connected() ),
	     SLOT( socketConnected() ) );
    connect( &socket, SIGNAL( readyRead() ),
	     SLOT( socketReadyRead() ) );
    connect( &socket, SIGNAL( error(int) ),
	     SLOT( socketError(int) ) );
    connect( &socket, SIGNAL( connectionClosed() ),
	     SLOT( socketConnectionClosed() ) );
    connect( &socket, SIGNAL( bytesWritten(int) ),
	     SLOT( socketBytesWritten(int) ) );
}

void QFtpDTP::setData( QByteArray *ba )
{
    is_ba = TRUE;
    data.ba = ba;
}

void QFtpDTP::setDevice( QIODevice *dev )
{
    is_ba = FALSE;
    data.dev = dev;
}

void QFtpDTP::writeData()
{
    if ( is_ba ) {
#if defined(QFTPDTP_DEBUG)
	qDebug( "QFtpDTP::writeData: write %d bytes", data.ba->size() );
#endif
	if ( data.ba->size() == 0 )
	    emit dataTransferProgress( 0, bytesTotal );
	else
	    socket.writeBlock( data.ba->data(), data.ba->size() );
	socket.close();
	clearData();
    } else if ( data.dev ) {
	callWriteData = FALSE;
	const int blockSize = 16*1024;
	char buf[blockSize];
	while ( !data.dev->atEnd() && socket.bytesToWrite()==0 ) {
	    Q_LONG read = data.dev->readBlock( buf, blockSize );
#if defined(QFTPDTP_DEBUG)
	    qDebug( "QFtpDTP::writeData: writeBlock() of size %d bytes", (int)read );
#endif
	    socket.writeBlock( buf, read );
	    if ( !data.dev )
		return; // this can happen when a command is aborted
	}
	if ( data.dev->atEnd() ) {
	    if ( bytesDone==0 && socket.bytesToWrite()==0 )
		emit dataTransferProgress( 0, bytesTotal );
	    socket.close();
	    clearData();
	} else {
	    callWriteData = TRUE;
	}
    }
}

inline bool QFtpDTP::hasError() const
{
    return !err.isNull();
}

inline QString QFtpDTP::errorMessage() const
{
    return err;
}

inline void QFtpDTP::clearError()
{
    err = QString::null;
}

void QFtpDTP::abortConnection()
{
#if defined(QFTPDTP_DEBUG)
    qDebug( "QFtpDTP::abortConnection" );
#endif
    callWriteData = FALSE;
    clearData();

    socket.clearPendingData();
    socket.close();
}

bool QFtpDTP::parseDir( const QString &buffer, const QString &userName, QUrlInfo *info )
{
    QStringList lst = QStringList::split( " ", buffer );

    if ( lst.count() < 9 )
	return FALSE;

    QString tmp;

    // permissions
    tmp = lst[ 0 ];

    if ( tmp[ 0 ] == QChar( 'd' ) ) {
	info->setDir( TRUE );
	info->setFile( FALSE );
	info->setSymLink( FALSE );
    } else if ( tmp[ 0 ] == QChar( '-' ) ) {
	info->setDir( FALSE );
	info->setFile( TRUE );
	info->setSymLink( FALSE );
    } else if ( tmp[ 0 ] == QChar( 'l' ) ) {
	info->setDir( TRUE ); // #### todo
	info->setFile( FALSE );
	info->setSymLink( TRUE );
    } else {
	return FALSE;
    }

    static int user = 0;
    static int group = 1;
    static int other = 2;
    static int readable = 0;
    static int writable = 1;
    static int executable = 2;

    bool perms[ 3 ][ 3 ];
    perms[0][0] = (tmp[ 1 ] == 'r');
    perms[0][1] = (tmp[ 2 ] == 'w');
    perms[0][2] = (tmp[ 3 ] == 'x');
    perms[1][0] = (tmp[ 4 ] == 'r');
    perms[1][1] = (tmp[ 5 ] == 'w');
    perms[1][2] = (tmp[ 6 ] == 'x');
    perms[2][0] = (tmp[ 7 ] == 'r');
    perms[2][1] = (tmp[ 8 ] == 'w');
    perms[2][2] = (tmp[ 9 ] == 'x');

    // owner
    tmp = lst[ 2 ];
    info->setOwner( tmp );

    // group
    tmp = lst[ 3 ];
    info->setGroup( tmp );

    // ### not correct
    info->setWritable( ( userName == info->owner() && perms[ user ][ writable ] ) ||
	perms[ other ][ writable ] );
    info->setReadable( ( userName == info->owner() && perms[ user ][ readable ] ) ||
	perms[ other ][ readable ] );

    int p = 0;
    if ( perms[ user ][ readable ] )
	p |= QUrlInfo::ReadOwner;
    if ( perms[ user ][ writable ] )
	p |= QUrlInfo::WriteOwner;
    if ( perms[ user ][ executable ] )
	p |= QUrlInfo::ExeOwner;
    if ( perms[ group ][ readable ] )
	p |= QUrlInfo::ReadGroup;
    if ( perms[ group ][ writable ] )
	p |= QUrlInfo::WriteGroup;
    if ( perms[ group ][ executable ] )
	p |= QUrlInfo::ExeGroup;
    if ( perms[ other ][ readable ] )
	p |= QUrlInfo::ReadOther;
    if ( perms[ other ][ writable ] )
	p |= QUrlInfo::WriteOther;
    if ( perms[ other ][ executable ] )
	p |= QUrlInfo::ExeOther;
    info->setPermissions( p );

    // size
    tmp = lst[ 4 ];
    info->setSize( tmp.toInt() );

    // date and time
    QTime time;
    QString dateStr;
    dateStr += "Sun ";
    lst[ 5 ][ 0 ] = lst[ 5 ][ 0 ].upper();
    dateStr += lst[ 5 ];
    dateStr += ' ';
    dateStr += lst[ 6 ];
    dateStr += ' ';

    if ( lst[ 7 ].contains( ":" ) ) {
	time = QTime( lst[ 7 ].left( 2 ).toInt(), lst[ 7 ].right( 2 ).toInt() );
	dateStr += QString::number( QDate::currentDate().year() );
    } else {
	dateStr += lst[ 7 ];
    }

    QDate date = QDate::fromString( dateStr );
    info->setLastModified( QDateTime( date, time ) );

    if ( lst[ 7 ].contains( ":" ) ) {
	const int futureTolerance = 600;
	if( info->lastModified().secsTo( QDateTime::currentDateTime() ) < -futureTolerance ) {
	    QDateTime dt = info->lastModified();
	    QDate d = dt.date();
	    d.setYMD(d.year()-1, d.month(), d.day());
	    dt.setDate(d);
	    info->setLastModified(dt);
	}
    }

    // name
    if ( info->isSymLink() )
	info->setName( lst[ 8 ].stripWhiteSpace() );
    else {
	QString n;
	for ( uint i = 8; i < lst.count(); ++i )
	    n += lst[ i ] + " ";
	n = n.stripWhiteSpace();
	info->setName( n );
    }
    return TRUE;
}

void QFtpDTP::socketConnected()
{
#if !defined (Q_WS_QWS)
    // Use a large send buffer to reduce the number
    // of writeBlocks when download and uploading files.
    // The actual size used here (128k) is default on most
    // Unixes.
    socket.socketDevice()->setSendBufferSize(128 * 1024);
    socket.socketDevice()->setReceiveBufferSize(128 * 1024);
#endif

    bytesDone = 0;
#if defined(QFTPDTP_DEBUG)
    qDebug( "QFtpDTP::connectState( CsConnected )" );
#endif
    emit connectState( QFtpDTP::CsConnected );
}

void QFtpDTP::socketReadyRead()
{
    if ( pi->currentCommand().isEmpty() ) {
	socket.close();
#if defined(QFTPDTP_DEBUG)
	qDebug( "QFtpDTP::connectState( CsClosed )" );
#endif
	emit connectState( QFtpDTP::CsClosed );
	return;
    }

    if ( pi->currentCommand().startsWith("LIST") ) {
	while ( socket.canReadLine() ) {
	    QUrlInfo i;
	    QString line = socket.readLine();
#if defined(QFTPDTP_DEBUG)
	    qDebug( "QFtpDTP read (list): '%s'", line.latin1() );
#endif
	    if ( parseDir( line, "", &i ) ) {
		emit listInfo( i );
	    } else {
		// some FTP servers don't return a 550 if the file or directory
		// does not exist, but rather write a text to the data socket
		// -- try to catch these cases
		if ( line.endsWith( "No such file or directory\r\n" ) )
		    err = line;
	    }
	}
    } else {
	if ( !is_ba && data.dev ) {
	    QByteArray ba( socket.bytesAvailable() );
	    Q_LONG bytesRead = socket.readBlock( ba.data(), ba.size() );
	    if ( bytesRead < 0 ) {
		// ### error handling
		return;
	    }
	    ba.resize( bytesRead );
	    bytesDone += bytesRead;
#if defined(QFTPDTP_DEBUG)
	    qDebug( "QFtpDTP read: %d bytes (total %d bytes)", (int)bytesRead, bytesDone );
#endif
	    emit dataTransferProgress( bytesDone, bytesTotal );
            if (data.dev)       // make sure it wasn't deleted in the slot
                data.dev->writeBlock( ba );
	} else {
#if defined(QFTPDTP_DEBUG)
	    qDebug( "QFtpDTP readyRead: %d bytes available (total %d bytes read)", (int)bytesAvailable(), bytesDone );
#endif
	    emit dataTransferProgress( bytesDone+socket.bytesAvailable(), bytesTotal );
	    emit readyRead();
	}
    }
}

void QFtpDTP::socketError( int e )
{
    if ( e == QSocket::ErrHostNotFound ) {
#if defined(QFTPDTP_DEBUG)
	qDebug( "QFtpDTP::connectState( CsHostNotFound )" );
#endif
	emit connectState( QFtpDTP::CsHostNotFound );
    } else if ( e == QSocket::ErrConnectionRefused ) {
#if defined(QFTPDTP_DEBUG)
	qDebug( "QFtpDTP::connectState( CsConnectionRefused )" );
#endif
	emit connectState( QFtpDTP::CsConnectionRefused );
    }
}

void QFtpDTP::socketConnectionClosed()
{
    if ( !is_ba && data.dev ) {
	clearData();
    }
#if defined(QFTPDTP_DEBUG)
    qDebug( "QFtpDTP::connectState( CsClosed )" );
#endif
    emit connectState( QFtpDTP::CsClosed );
}

void QFtpDTP::socketBytesWritten( int bytes )
{
    bytesDone += bytes;
#if defined(QFTPDTP_DEBUG)
    qDebug( "QFtpDTP::bytesWritten( %d )", bytesDone );
#endif
    emit dataTransferProgress( bytesDone, bytesTotal );
    if ( callWriteData )
	writeData();
}

/**********************************************************************
 *
 * QFtpPI implemenatation
 *
 *********************************************************************/
QFtpPI::QFtpPI( QObject *parent ) :
    QObject( parent ),
    rawCommand(FALSE),
    dtp( this ),
    commandSocket( 0, "QFtpPI_socket" ),
    state( Begin ), abortState( None ),
    currentCmd( QString::null ),
    waitForDtpToConnect( FALSE ),
    waitForDtpToClose( FALSE )
{
    connect( &commandSocket, SIGNAL(hostFound()),
	    SLOT(hostFound()) );
    connect( &commandSocket, SIGNAL(connected()),
	    SLOT(connected()) );
    connect( &commandSocket, SIGNAL(connectionClosed()),
	    SLOT(connectionClosed()) );
    connect( &commandSocket, SIGNAL(delayedCloseFinished()),
	    SLOT(delayedCloseFinished()) );
    connect( &commandSocket, SIGNAL(readyRead()),
	    SLOT(readyRead()) );
    connect( &commandSocket, SIGNAL(error(int)),
	    SLOT(error(int)) );

    connect( &dtp, SIGNAL(connectState(int)),
	     SLOT(dtpConnectState(int)) );
}

void QFtpPI::connectToHost( const QString &host, Q_UINT16 port )
{
    emit connectState( QFtp::HostLookup );
    commandSocket.connectToHost( host, port );
}

/*
  Sends the sequence of commands \a cmds to the FTP server. When the commands
  are all done the finished() signal is emitted. When an error occurs, the
  error() signal is emitted.

  If there are pending commands in the queue this functions returns FALSE and
  the \a cmds are not added to the queue; otherwise it returns TRUE.
*/
bool QFtpPI::sendCommands( const QStringList &cmds )
{
    if ( !pendingCommands.isEmpty() )
	return FALSE;

    if ( commandSocket.state()!=QSocket::Connected || state!=Idle ) {
	emit error( QFtp::NotConnected, QFtp::tr( "Not connected" ) );
	return TRUE; // there are no pending commands
    }

    pendingCommands = cmds;
    startNextCmd();
    return TRUE;
}

void QFtpPI::clearPendingCommands()
{
    pendingCommands.clear();
    dtp.abortConnection();
    currentCmd = QString::null;
    state = Idle;
}

void QFtpPI::abort()
{
    pendingCommands.clear();

    if ( abortState != None )
	// ABOR already sent
	return;

    abortState = AbortStarted;
#if defined(QFTPPI_DEBUG)
    qDebug( "QFtpPI send: ABOR" );
#endif
    commandSocket.writeBlock( "ABOR\r\n", 6 );

    if ( currentCmd.startsWith("STOR ") )
	dtp.abortConnection();
}

void QFtpPI::hostFound()
{
    emit connectState( QFtp::Connecting );
}

void QFtpPI::connected()
{
    state = Begin;
#if defined(QFTPPI_DEBUG)
//    qDebug( "QFtpPI state: %d [connected()]", state );
#endif
    emit connectState( QFtp::Connected );
}

void QFtpPI::connectionClosed()
{
    commandSocket.close();
    emit connectState( QFtp::Unconnected );
}

void QFtpPI::delayedCloseFinished()
{
    emit connectState( QFtp::Unconnected );
}

void QFtpPI::error( int e )
{
    if ( e == QSocket::ErrHostNotFound ) {
	emit connectState( QFtp::Unconnected );
	emit error( QFtp::HostNotFound,
		    QFtp::tr( "Host %1 not found" ).arg( commandSocket.peerName() ) );
    } else if ( e == QSocket::ErrConnectionRefused ) {
	emit connectState( QFtp::Unconnected );
	emit error( QFtp::ConnectionRefused,
		    QFtp::tr( "Connection refused to host %1" ).arg( commandSocket.peerName() ) );
    }
}

void QFtpPI::readyRead()
{
    if ( waitForDtpToClose )
	return;

    while ( commandSocket.canReadLine() ) {
	// read line with respect to line continuation
	QString line = commandSocket.readLine();
	if ( replyText.isEmpty() ) {
	    if ( line.length() < 3 ) {
		// ### protocol error
		return;
	    }
	    const int lowerLimit[3] = {1,0,0};
	    const int upperLimit[3] = {5,5,9};
	    for ( int i=0; i<3; i++ ) {
		replyCode[i] = line[i].digitValue();
		if ( replyCode[i]<lowerLimit[i] || replyCode[i]>upperLimit[i] ) {
		    // ### protocol error
		    return;
		}
	    }
	}
	QString endOfMultiLine;
	endOfMultiLine[0] = '0' + replyCode[0];
	endOfMultiLine[1] = '0' + replyCode[1];
	endOfMultiLine[2] = '0' + replyCode[2];
	endOfMultiLine[3] = ' ';
	QString lineCont( endOfMultiLine );
	lineCont[3] = '-';
	QString lineLeft4 = line.left(4);

	while ( lineLeft4 != endOfMultiLine ) {
	    if ( lineLeft4 == lineCont )
		replyText += line.mid( 4 ); // strip 'xyz-'
	    else
		replyText += line;
	    if ( !commandSocket.canReadLine() )
		return;
	    line = commandSocket.readLine();
	    lineLeft4 = line.left(4);
	}
	replyText += line.mid( 4 ); // strip reply code 'xyz '
	if ( replyText.endsWith("\r\n") )
	    replyText.truncate( replyText.length()-2 );

	if ( processReply() )
	    replyText = "";
    }
}

/*
  Process a reply from the FTP server.

  Returns TRUE if the reply was processed or FALSE if the reply has to be
  processed at a later point.
*/
bool QFtpPI::processReply()
{
#if defined(QFTPPI_DEBUG)
//    qDebug( "QFtpPI state: %d [processReply() begin]", state );
    if ( replyText.length() < 400 )
	qDebug( "QFtpPI recv: %d %s", 100*replyCode[0]+10*replyCode[1]+replyCode[2], replyText.latin1() );
    else
	qDebug( "QFtpPI recv: %d (text skipped)", 100*replyCode[0]+10*replyCode[1]+replyCode[2] );
#endif

    // process 226 replies ("Closing Data Connection") only when the data
    // connection is really closed to avoid short reads of the DTP
    if ( 100*replyCode[0]+10*replyCode[1]+replyCode[2] == 226 ) {
	if ( dtp.socketState() != QSocket::Idle ) {
	    waitForDtpToClose = TRUE;
	    return FALSE;
	}
    }

    switch ( abortState ) {
	case AbortStarted:
	    abortState = WaitForAbortToFinish;
	    break;
	case WaitForAbortToFinish:
	    abortState = None;
	    return TRUE;
	default:
	    break;
    }

    // get new state
    static const State table[5] = {
	/* 1yz   2yz      3yz   4yz      5yz */
	Waiting, Success, Idle, Failure, Failure
    };
    switch ( state ) {
	case Begin:
	    if ( replyCode[0] == 1 ) {
		return TRUE;
	    } else if ( replyCode[0] == 2 ) {
		state = Idle;
		emit finished( QFtp::tr( "Connected to host %1" ).arg( commandSocket.peerName() ) );
		break;
	    }
	    // ### error handling
	    return TRUE;
	case Waiting:
	    if ( replyCode[0]<0 || replyCode[0]>5 )
		state = Failure;
	    else
		state = table[ replyCode[0] - 1 ];
	    break;
	default:
	    // ### spontaneous message
	    return TRUE;
    }
#if defined(QFTPPI_DEBUG)
//    qDebug( "QFtpPI state: %d [processReply() intermediate]", state );
#endif

    // special actions on certain replies
    int replyCodeInt = 100*replyCode[0] + 10*replyCode[1] + replyCode[2];
    emit rawFtpReply( replyCodeInt, replyText );
    if ( rawCommand ) {
	rawCommand = FALSE;
    } else if ( replyCodeInt == 227 ) {
	// 227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
        // rfc959 does not define this response precisely, and gives
        // both examples where the parenthesis are used, and where
        // they are missing. We need to scan for the address and host
        // info.
	QRegExp addrPortPattern("(\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)");
	if (addrPortPattern.search(replyText) == -1) {
#if defined(QFTPPI_DEBUG)
	    qDebug( "QFtp: bad 227 response -- address and port information missing" );
#endif
	    // ### error handling
	} else {
	    QStringList lst = addrPortPattern.capturedTexts();
	    QString host = lst[1] + "." + lst[2] + "." + lst[3] + "." + lst[4];
	    Q_UINT16 port = ( lst[5].toUInt() << 8 ) + lst[6].toUInt();
	    waitForDtpToConnect = TRUE;
	    dtp.connectToHost( host, port );
	}
    } else if ( replyCodeInt == 230 ) {
	if ( currentCmd.startsWith("USER ") && pendingCommands.count()>0 &&
		pendingCommands.first().startsWith("PASS ") ) {
	    // no need to send the PASS -- we are already logged in
	    pendingCommands.pop_front();
	}
	// 230 User logged in, proceed.
	emit connectState( QFtp::LoggedIn );
    } else if ( replyCodeInt == 213 ) {
	// 213 File status.
	if ( currentCmd.startsWith("SIZE ") )
	    dtp.setBytesTotal( replyText.simplifyWhiteSpace().toInt() );
    } else if ( replyCode[0]==1 && currentCmd.startsWith("STOR ") ) {
	dtp.writeData();
    }

    // react on new state
    switch ( state ) {
	case Begin:
	    // ### should never happen
	    break;
	case Success:
	    // ### success handling
	    state = Idle;
	    // no break!
	case Idle:
	    if ( dtp.hasError() ) {
		emit error( QFtp::UnknownError, dtp.errorMessage() );
		dtp.clearError();
	    }
	    startNextCmd();
	    break;
	case Waiting:
	    // ### do nothing
	    break;
	case Failure:
	    emit error( QFtp::UnknownError, replyText );
	    state = Idle;
	    startNextCmd();
	    break;
    }
#if defined(QFTPPI_DEBUG)
//    qDebug( "QFtpPI state: %d [processReply() end]", state );
#endif
    return TRUE;
}

#ifndef QT_NO_TEXTCODEC
QM_EXPORT_FTP QTextCodec *qt_ftp_filename_codec = 0;
#endif

/*
  Starts next pending command. Returns FALSE if there are no pending commands,
  otherwise it returns TRUE.
*/
bool QFtpPI::startNextCmd()
{
    if ( waitForDtpToConnect )
	// don't process any new commands until we are connected
	return TRUE;

#if defined(QFTPPI_DEBUG)
    if ( state != Idle )
	qDebug( "QFtpPI startNextCmd: Internal error! QFtpPI called in non-Idle state %d", state );
#endif
    if ( pendingCommands.isEmpty() ) {
	currentCmd = QString::null;
	emit finished( replyText );
	return FALSE;
    }
    currentCmd = pendingCommands.first();
    pendingCommands.pop_front();
#if defined(QFTPPI_DEBUG)
    qDebug( "QFtpPI send: %s", currentCmd.left( currentCmd.length()-2 ).latin1() );
#endif
    state = Waiting;
#ifndef QT_NO_TEXTCODEC
    if ( qt_ftp_filename_codec ) {
	int len = 0;
	QCString enc = qt_ftp_filename_codec->fromUnicode(currentCmd,len);
	commandSocket.writeBlock( enc.data(), len );
    } else
#endif
    {
	commandSocket.writeBlock( currentCmd.latin1(), currentCmd.length() );
    }
    return TRUE;
}

void QFtpPI::dtpConnectState( int s )
{
    switch ( s ) {
	case QFtpDTP::CsClosed:
	    if ( waitForDtpToClose ) {
		// there is an unprocessed reply
		if ( processReply() )
		    replyText = "";
		else
		    return;
	    }
	    waitForDtpToClose = FALSE;
	    readyRead();
	    return;
	case QFtpDTP::CsConnected:
	    waitForDtpToConnect = FALSE;
	    startNextCmd();
	    return;
	case QFtpDTP::CsHostNotFound:
	case QFtpDTP::CsConnectionRefused:
	    emit error( QFtp::ConnectionRefused,
			QFtp::tr( "Connection refused for data connection" ) );
	    startNextCmd();
	    return;
	default:
	    return;
    }
}

/**********************************************************************
 *
 * QFtpPrivate
 *
 *********************************************************************/
class QFtpPrivate
{
public:
    QFtpPrivate() :
	close_waitForStateChange(FALSE),
	state( QFtp::Unconnected ),
	error( QFtp::NoError ),
	npWaitForLoginDone( FALSE )
    { pending.setAutoDelete( TRUE ); }

    QFtpPI pi;
    QPtrList<QFtpCommand> pending;
    bool close_waitForStateChange;
    QFtp::State state;
    QFtp::Error error;
    QString errorString;

    bool npWaitForLoginDone;
};

static QPtrDict<QFtpPrivate> *d_ptr = 0;
static void cleanup_d_ptr()
{
    delete d_ptr;
    d_ptr = 0;
}
static QFtpPrivate* d( const QFtp* foo )
{
    if ( !d_ptr ) {
	d_ptr = new QPtrDict<QFtpPrivate>;
	d_ptr->setAutoDelete( TRUE );
	qAddPostRoutine( cleanup_d_ptr );
    }
    QFtpPrivate* ret = d_ptr->find( (void*)foo );
    if ( ! ret ) {
	ret = new QFtpPrivate;
	d_ptr->replace( (void*) foo, ret );
    }
    return ret;
}

static void delete_d( const QFtp* foo )
{
    if ( d_ptr )
	d_ptr->remove( (void*) foo );
}

/**********************************************************************
 *
 * QFtp implementation
 *
 *********************************************************************/
/*!
    \class QFtp qftp.h
    \brief The QFtp class provides an implementation of the FTP protocol.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    This class provides two different interfaces: one is the
    QNetworkProtocol interface that allows you to use FTP through the
    QUrlOperator abstraction. The other is a direct interface to FTP
    that gives you lower-level access to the FTP protocol for finer
    control. Using the direct interface you can also execute arbitrary
    FTP commands.

    Don't mix the two interfaces, since the behavior is not
    well-defined.

    If you want to use QFtp with the QNetworkProtocol interface, you
    do not use it directly, but rather through a QUrlOperator, for
    example:

    \code
    QUrlOperator op( "ftp://ftp.trolltech.com" );
    op.listChildren(); // Asks the server to provide a directory listing
    \endcode

    This code will only work if the QFtp class is registered; to
    register the class, you must call qInitNetworkProtocols() before
    using a QUrlOperator with QFtp.

    The rest of this descrption describes the direct interface to FTP.

    The class works asynchronously, so there are no blocking
    functions. If an operation cannot be executed immediately, the
    function will still return straight away and the operation will be
    scheduled for later execution. The results of scheduled operations
    are reported via signals. This approach depends on the event loop
    being in operation.

    The operations that can be scheduled (they are called "commands"
    in the rest of the documentation) are the following:
    connectToHost(), login(), close(), list(), cd(), get(), put(),
    remove(), mkdir(), rmdir(), rename() and rawCommand().

    All of these commands return a unique identifier that allows you
    to keep track of the command that is currently being executed.
    When the execution of a command starts, the commandStarted()
    signal with the command's identifier is emitted. When the command
    is finished, the commandFinished() signal is emitted with the
    command's identifier and a bool that indicates whether the command
    finished with an error.

    In some cases, you might want to execute a sequence of commands,
    e.g. if you want to connect and login to a FTP server. This is
    simply achieved:

    \code
    QFtp *ftp = new QFtp( this ); // this is an optional QObject parent
    ftp->connectToHost( "ftp.trolltech.com" );
    ftp->login();
    \endcode

    In this case two FTP commands have been scheduled. When the last
    scheduled command has finished, a done() signal is emitted with
    a bool argument that tells you whether the sequence finished with
    an error.

    If an error occurs during the execution of one of the commands in
    a sequence of commands, all the pending commands (i.e. scheduled,
    but not yet executed commands) are cleared and no signals are
    emitted for them.

    Some commands, e.g. list(), emit additional signals to report
    their results.

    Example: If you want to download the INSTALL file from Trolltech's
    FTP server, you would write this:

    \code
    ftp->connectToHost( "ftp.trolltech.com" );  // id == 1
    ftp->login();                               // id == 2
    ftp->cd( "qt" );                            // id == 3
    ftp->get( "INSTALL" );                      // id == 4
    ftp->close();                               // id == 5
    \endcode

    For this example the following sequence of signals is emitted
    (with small variations, depending on network traffic, etc.):

    \code
    start( 1 )
    stateChanged( HostLookup )
    stateChanged( Connecting )
    stateChanged( Connected )
    finished( 1, FALSE )

    start( 2 )
    stateChanged( LoggedIn )
    finished( 2, FALSE )

    start( 3 )
    finished( 3, FALSE )

    start( 4 )
    dataTransferProgress( 0, 3798 )
    dataTransferProgress( 2896, 3798 )
    readyRead()
    dataTransferProgress( 3798, 3798 )
    readyRead()
    finished( 4, FALSE )

    start( 5 )
    stateChanged( Closing )
    stateChanged( Unconnected )
    finished( 5, FALSE )

    done( FALSE )
    \endcode

    The dataTransferProgress() signal in the above example is useful
    if you want to show a \link QProgressBar progressbar \endlink to
    inform the user about the progress of the download. The
    readyRead() signal tells you that there is data ready to be read.
    The amount of data can be queried then with the bytesAvailable()
    function and it can be read with the readBlock() or readAll()
    function.

    If the login fails for the above example, the signals would look
    like this:

    \code
    start( 1 )
    stateChanged( HostLookup )
    stateChanged( Connecting )
    stateChanged( Connected )
    finished( 1, FALSE )

    start( 2 )
    finished( 2, TRUE )

    done( TRUE )
    \endcode

    You can then get details about the error with the error() and
    errorString() functions.

    The functions currentId() and currentCommand() provide more
    information about the currently executing command.

    The functions hasPendingCommands() and clearPendingCommands()
    allow you to query and clear the list of pending commands.

    The safest and easiest way to use the FTP protocol is to use
    QUrlOperator() or the FTP commands described above. If you are an
    experienced network programmer and want to have complete control
    you can use rawCommand() to execute arbitrary FTP commands.

    \sa \link network.html Qt Network Documentation \endlink QNetworkProtocol, QUrlOperator QHttp
*/

/*!
    Constructs a QFtp object.
*/
QFtp::QFtp() : QNetworkProtocol()
{
    init();
}

/*!
    Constructs a QFtp object. The \a parent and \a name parameters
    are passed to the QObject constructor.
*/
QFtp::QFtp( QObject *parent, const char *name ) : QNetworkProtocol()
{
    if ( parent )
	parent->insertChild( this );
    setName( name );
    init();
}

void QFtp::init()
{
    QFtpPrivate *d = ::d( this );
    d->errorString = tr( "Unknown error" );

    connect( &d->pi, SIGNAL(connectState(int)),
	    SLOT(piConnectState(int)) );
    connect( &d->pi, SIGNAL(finished(const QString&)),
	    SLOT(piFinished(const QString&)) );
    connect( &d->pi, SIGNAL(error(int,const QString&)),
	    SLOT(piError(int,const QString&)) );
    connect( &d->pi, SIGNAL(rawFtpReply(int,const QString&)),
	    SLOT(piFtpReply(int,const QString&)) );

    connect( &d->pi.dtp, SIGNAL(readyRead()),
	    SIGNAL(readyRead()) );
    connect( &d->pi.dtp, SIGNAL(dataTransferProgress(int,int)),
	    SIGNAL(dataTransferProgress(int,int)) );
    connect( &d->pi.dtp, SIGNAL(listInfo(const QUrlInfo&)),
	    SIGNAL(listInfo(const QUrlInfo&)) );
}

/*!
    \enum QFtp::State

    This enum defines the connection state:

    \value Unconnected There is no connection to the host.
    \value HostLookup A host name lookup is in progress.
    \value Connecting An attempt to connect to the host is in progress.
    \value Connected Connection to the host has been achieved.
    \value LoggedIn Connection and user login have been achieved.
    \value Closing The connection is closing down, but it is not yet
    closed. (The state will be \c Unconnected when the connection is
    closed.)

    \sa stateChanged() state()
*/
/*!
    \enum QFtp::Error

    This enum identifies the error that occurred.

    \value NoError No error occurred.
    \value HostNotFound The host name lookup failed.
    \value ConnectionRefused The server refused the connection.
    \value NotConnected Tried to send a command, but there is no connection to
    a server.
    \value UnknownError An error other than those specified above
    occurred.

    \sa error()
*/

/*!
    \enum QFtp::Command

    This enum is used as the return value for the currentCommand() function.
    This allows you to perform specific actions for particular
    commands, e.g. in a FTP client, you might want to clear the
    directory view when a list() command is started; in this case you
    can simply check in the slot connected to the start() signal if
    the currentCommand() is \c List.

    \value None No command is being executed.
    \value ConnectToHost connectToHost() is being executed.
    \value Login login() is being executed.
    \value Close close() is being executed.
    \value List list() is being executed.
    \value Cd cd() is being executed.
    \value Get get() is being executed.
    \value Put put() is being executed.
    \value Remove remove() is being executed.
    \value Mkdir mkdir() is being executed.
    \value Rmdir rmdir() is being executed.
    \value Rename rename() is being executed.
    \value RawCommand rawCommand() is being executed.

    \sa currentCommand()
*/

/*!
    \fn void QFtp::stateChanged( int state )

    This signal is emitted when the state of the connection changes.
    The argument \a state is the new state of the connection; it is
    one of the \l State values.

    It is usually emitted in response to a connectToHost() or close()
    command, but it can also be emitted "spontaneously", e.g. when the
    server closes the connection unexpectedly.

    \sa connectToHost() close() state() State
*/

/*!
    \fn void QFtp::listInfo( const QUrlInfo &i );

    This signal is emitted for each directory entry the list() command
    finds. The details of the entry are stored in \a i.

    \sa list()
*/

/*!
    \fn void QFtp::commandStarted( int id )

    This signal is emitted when processing the command identified by
    \a id starts.

    \sa commandFinished() done()
*/

/*!
    \fn void QFtp::commandFinished( int id, bool error )

    This signal is emitted when processing the command identified by
    \a id has finished. \a error is TRUE if an error occurred during
    the processing; otherwise \a error is FALSE.

    \sa commandStarted() done() error() errorString()
*/

/*!
    \fn void QFtp::done( bool error )

    This signal is emitted when the last pending command has finished;
    (it is emitted after the last command's commandFinished() signal).
    \a error is TRUE if an error occurred during the processing;
    otherwise \a error is FALSE.

    \sa commandFinished() error() errorString()
*/

/*!
    \fn void QFtp::readyRead()

    This signal is emitted in response to a get() command when there
    is new data to read.

    If you specify a device as the second argument in the get()
    command, this signal is \e not emitted; instead the data is
    written directly to the device.

    You can read the data with the readAll() or readBlock() functions.

    This signal is useful if you want to process the data in chunks as
    soon as it becomes available. If you are only interested in the
    complete data, just connect to the commandFinished() signal and
    read the data then instead.

    \sa get() readBlock() readAll() bytesAvailable()
*/

/*!
    \fn void QFtp::dataTransferProgress( int done, int total )

    This signal is emitted in response to a get() or put() request to
    indicate the current progress of the download or upload.

    \a done is the amount of data that has already been transferred
    and \a total is the total amount of data to be read or written. It
    is possible that the QFtp class is not able to determine the total
    amount of data that should be transferred, in which case \a total
    is 0. (If you connect this signal to a QProgressBar, the progress
    bar shows a busy indicator if the total is 0).

    \warning \a done and \a total are not necessarily the size in
    bytes, since for large files these values might need to be
    "scaled" to avoid overflow.

    \sa get() put() QProgressBar::setProgress()
*/

/*!
    \fn void QFtp::rawCommandReply( int replyCode, const QString &detail );

    This signal is emitted in response to the rawCommand() function.
    \a replyCode is the 3 digit reply code and \a detail is the text
    that follows the reply code.

    \sa rawCommand()
*/

/*!
    Connects to the FTP server \a host using port \a port.

    The stateChanged() signal is emitted when the state of the
    connecting process changes, e.g. to \c HostLookup, then \c
    Connecting, then \c Connected.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa stateChanged() commandStarted() commandFinished()
*/
int QFtp::connectToHost( const QString &host, Q_UINT16 port )
{
    QStringList cmds;
    cmds << host;
    cmds << QString::number( (uint)port );
    return addCommand( new QFtpCommand( ConnectToHost, cmds ) );
}

/*!
    Logs in to the FTP server with the username \a user and the
    password \a password.

    The stateChanged() signal is emitted when the state of the
    connecting process changes, e.g. to \c LoggedIn.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::login( const QString &user, const QString &password )
{
    QStringList cmds;
    cmds << ( QString("USER ") + ( user.isNull() ? QString("anonymous") : user ) + "\r\n" );
    cmds << ( QString("PASS ") + ( password.isNull() ? QString("anonymous@") : password ) + "\r\n" );
    return addCommand( new QFtpCommand( Login, cmds ) );
}

/*!
    Closes the connection to the FTP server.

    The stateChanged() signal is emitted when the state of the
    connecting process changes, e.g. to \c Closing, then \c
    Unconnected.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa stateChanged() commandStarted() commandFinished()
*/
int QFtp::close()
{
    return addCommand( new QFtpCommand( Close, QStringList("QUIT\r\n") ) );
}

/*!
    Lists the contents of directory \a dir on the FTP server. If \a
    dir is empty, it lists the contents of the current directory.

    The listInfo() signal is emitted for each directory entry found.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa listInfo() commandStarted() commandFinished()
*/
int QFtp::list( const QString &dir )
{
    QStringList cmds;
    cmds << "TYPE A\r\n";
    cmds << "PASV\r\n";
    if ( dir.isEmpty() )
	cmds << "LIST\r\n";
    else
	cmds << ( "LIST " + dir + "\r\n" );
    return addCommand( new QFtpCommand( List, cmds ) );
}

/*!
    Changes the working directory of the server to \a dir.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::cd( const QString &dir )
{
    return addCommand( new QFtpCommand( Cd, QStringList("CWD "+dir+"\r\n") ) );
}

/*!
    Downloads the file \a file from the server.

    If \a dev is 0, then the readyRead() signal is emitted when there
    is data available to read. You can then read the data with the
    readBlock() or readAll() functions.

    If \a dev is not 0, the data is written directly to the device \a
    dev. Make sure that the \a dev pointer is valid for the duration
    of the operation (it is safe to delete it when the
    commandFinished() signal is emitted). In this case the readyRead()
    signal is \e not emitted and you cannot read data with the
    readBlock() or readAll() functions.

    If you don't read the data immediately it becomes available, i.e.
    when the readyRead() signal is emitted, it is still available
    until the next command is started.

    For example, if you want to present the data to the user as soon
    as there is something available, connect to the readyRead() signal
    and read the data immediately. On the other hand, if you only want
    to work with the complete data, you can connect to the
    commandFinished() signal and read the data when the get() command
    is finished.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa readyRead() dataTransferProgress() commandStarted()
    commandFinished()
*/
int QFtp::get( const QString &file, QIODevice *dev )
{
    QStringList cmds;
    cmds << ( "SIZE " + file + "\r\n" );
    cmds << "TYPE I\r\n";
    cmds << "PASV\r\n";
    cmds << ( "RETR " + file + "\r\n" );
    if ( dev )
	return addCommand( new QFtpCommand( Get, cmds, dev ) );
    return addCommand( new QFtpCommand( Get, cmds ) );
}

/*!
    \overload

    Writes the data \a data to the file called \a file on the server.
    The progress of the upload is reported by the
    dataTransferProgress() signal.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa dataTransferProgress() commandStarted() commandFinished()
*/
int QFtp::put( const QByteArray &data, const QString &file )
{
    QStringList cmds;
    cmds << "TYPE I\r\n";
    cmds << "PASV\r\n";
    cmds << ( "ALLO " + QString::number(data.size()) + "\r\n" );
    cmds << ( "STOR " + file + "\r\n" );
    return addCommand( new QFtpCommand( Put, cmds, data ) );
}

/*!
    Reads the data from the IO device \a dev, and writes it to the
    file called \a file on the server. The data is read in chunks from
    the IO device, so this overload allows you to transmit large
    amounts of data without the need to read all the data into memory
    at once.

    Make sure that the \a dev pointer is valid for the duration of the
    operation (it is safe to delete it when the commandFinished() is
    emitted).
*/
int QFtp::put( QIODevice *dev, const QString &file )
{
    QStringList cmds;
    cmds << "TYPE I\r\n";
    cmds << "PASV\r\n";
    if ( !dev->isSequentialAccess() )
	cmds << ( "ALLO " + QString::number(dev->size()) + "\r\n" );
    cmds << ( "STOR " + file + "\r\n" );
    return addCommand( new QFtpCommand( Put, cmds, dev ) );
}

/*!
    Deletes the file called \a file from the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::remove( const QString &file )
{
    return addCommand( new QFtpCommand( Remove, QStringList("DELE "+file+"\r\n") ) );
}

/*!
    Creates a directory called \a dir on the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::mkdir( const QString &dir )
{
    return addCommand( new QFtpCommand( Mkdir, QStringList("MKD "+dir+"\r\n") ) );
}

/*!
    Removes the directory called \a dir from the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::rmdir( const QString &dir )
{
    return addCommand( new QFtpCommand( Rmdir, QStringList("RMD "+dir+"\r\n") ) );
}

/*!
    Renames the file called \a oldname to \a newname on the server.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa commandStarted() commandFinished()
*/
int QFtp::rename( const QString &oldname, const QString &newname )
{
    QStringList cmds;
    cmds << ( "RNFR " + oldname + "\r\n" );
    cmds << ( "RNTO " + newname + "\r\n" );
    return addCommand( new QFtpCommand( Rename, cmds ) );
}

/*!
    Sends the raw FTP command \a command to the FTP server. This is
    useful for low-level FTP access. If the operation you wish to
    perform has an equivalent QFtp function, we recommend using the
    function instead of raw FTP commands since the functions are
    easier and safer.

    The function does not block and returns immediately. The command
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    commandStarted() and commandFinished().

    When the command is started the commandStarted() signal is
    emitted. When it is finished the commandFinished() signal is
    emitted.

    \sa rawCommandReply() commandStarted() commandFinished()
*/
int QFtp::rawCommand( const QString &command )
{
    QString cmd = command.stripWhiteSpace() + "\r\n";
    return addCommand( new QFtpCommand( RawCommand, QStringList(cmd) ) );
}

/*!
    Returns the number of bytes that can be read from the data socket
    at the moment.

    \sa get() readyRead() readBlock() readAll()
*/
Q_ULONG QFtp::bytesAvailable() const
{
    QFtpPrivate *d = ::d( this );
    return d->pi.dtp.bytesAvailable();
}

/*!
    Reads \a maxlen bytes from the data socket into \a data and
    returns the number of bytes read. Returns -1 if an error occurred.

    \sa get() readyRead() bytesAvailable() readAll()
*/
Q_LONG QFtp::readBlock( char *data, Q_ULONG maxlen )
{
    QFtpPrivate *d = ::d( this );
    return d->pi.dtp.readBlock( data, maxlen );
}

/*!
    Reads all the bytes available from the data socket and returns
    them.

    \sa get() readyRead() bytesAvailable() readBlock()
*/
QByteArray QFtp::readAll()
{
    QFtpPrivate *d = ::d( this );
    return d->pi.dtp.readAll();
}

/*!
    Aborts the current command and deletes all scheduled commands.

    If there is an unfinished command (i.e. a command for which the
    commandStarted() signal has been emitted, but for which the
    commandFinished() signal has not been emitted), this function
    sends an \c ABORT command to the server. When the server replies
    that the command is aborted, the commandFinished() signal with the
    \c error argument set to \c TRUE is emitted for the command. Due
    to timing issues, it is possible that the command had already
    finished before the abort request reached the server, in which
    case, the commandFinished() signal is emitted with the \c error
    argument set to \c FALSE.

    For all other commands that are affected by the abort(), no
    signals are emitted.

    If you don't start further FTP commands directly after the
    abort(), there won't be any scheduled commands and the done()
    signal is emitted.

    \warning Some FTP servers, for example the BSD FTP daemon (version
    0.3), wrongly return a positive reply even when an abort has
    occurred. For these servers the commandFinished() signal has its
    error flag set to \c FALSE, even though the command did not
    complete successfully.

    \sa clearPendingCommands()
*/
void QFtp::abort()
{
    QFtpPrivate *d = ::d( this );
    if ( d->pending.isEmpty() )
	return;

    clearPendingCommands();
    d->pi.abort();
}

/*!
    Returns the identifier of the FTP command that is being executed
    or 0 if there is no command being executed.

    \sa currentCommand()
*/
int QFtp::currentId() const
{
    QFtpPrivate *d = ::d( this );
    QFtpCommand *c = d->pending.getFirst();
    if ( c == 0 )
	return 0;
    return c->id;
}

/*!
    Returns the command type of the FTP command being executed or \c
    None if there is no command being executed.

    \sa currentId()
*/
QFtp::Command QFtp::currentCommand() const
{
    QFtpPrivate *d = ::d( this );
    QFtpCommand *c = d->pending.getFirst();
    if ( c == 0 )
	return None;
    return c->command;
}

/*!
    Returns the QIODevice pointer that is used by the FTP command to read data
    from or store data to. If there is no current FTP command being executed or
    if the command does not use an IO device, this function returns 0.

    This function can be used to delete the QIODevice in the slot connected to
    the commandFinished() signal.

    \sa get() put()
*/
QIODevice* QFtp::currentDevice() const
{
    QFtpPrivate *d = ::d( this );
    QFtpCommand *c = d->pending.getFirst();
    if ( !c )
	return 0;
    if ( c->is_ba )
	return 0;
    return c->data.dev;
}

/*!
    Returns TRUE if there are any commands scheduled that have not yet
    been executed; otherwise returns FALSE.

    The command that is being executed is \e not considered as a
    scheduled command.

    \sa clearPendingCommands() currentId() currentCommand()
*/
bool QFtp::hasPendingCommands() const
{
    QFtpPrivate *d = ::d( this );
    return d->pending.count() > 1;
}

/*!
    Deletes all pending commands from the list of scheduled commands.
    This does not affect the command that is being executed. If you
    want to stop this this as well, use abort().

    \sa hasPendingCommands() abort()
*/
void QFtp::clearPendingCommands()
{
    QFtpPrivate *d = ::d( this );
    QFtpCommand *c = 0;
    if ( d->pending.count() > 0 )
	c = d->pending.take( 0 );
    d->pending.clear();
    if ( c )
	d->pending.append( c );
}

/*!
    Returns the current state of the object. When the state changes,
    the stateChanged() signal is emitted.

    \sa State stateChanged()
*/
QFtp::State QFtp::state() const
{
    QFtpPrivate *d = ::d( this );
    return d->state;
}

/*!
    Returns the last error that occurred. This is useful to find out
    what when wrong when receiving a commandFinished() or a done()
    signal with the \c error argument set to \c TRUE.

    If you start a new command, the error status is reset to \c NoError.
*/
QFtp::Error QFtp::error() const
{
    QFtpPrivate *d = ::d( this );
    return d->error;
}

/*!
    Returns a human-readable description of the last error that
    occurred. This is useful for presenting a error message to the
    user when receiving a commandFinished() or a done() signal with
    the \c error argument set to \c TRUE.

    The error string is often (but not always) the reply from the
    server, so it is not always possible to translate the string. If
    the message comes from Qt, the string has already passed through
    tr().
*/
QString QFtp::errorString() const
{
    QFtpPrivate *d = ::d( this );
    return d->errorString;
}

int QFtp::addCommand( QFtpCommand *cmd )
{
    QFtpPrivate *d = ::d( this );
    d->pending.append( cmd );

    if ( d->pending.count() == 1 )
	// don't emit the commandStarted() signal before the id is returned
	QTimer::singleShot( 0, this, SLOT(startNextCommand()) );

    return cmd->id;
}

void QFtp::startNextCommand()
{
    QFtpPrivate *d = ::d( this );

    QFtpCommand *c = d->pending.getFirst();
    if ( c == 0 )
	return;

    d->error = NoError;
    d->errorString = tr( "Unknown error" );

    if ( bytesAvailable() )
	readAll(); // clear the data
    emit commandStarted( c->id );

    if ( c->command == ConnectToHost ) {
	d->pi.connectToHost( c->rawCmds[0], c->rawCmds[1].toUInt() );
    } else {
	if ( c->command == Put ) {
	    if ( c->is_ba ) {
		d->pi.dtp.setData( c->data.ba );
		d->pi.dtp.setBytesTotal( c->data.ba->size() );
	    } else if ( c->data.dev && (c->data.dev->isOpen() || c->data.dev->open(IO_ReadOnly) ) ) {
		d->pi.dtp.setDevice( c->data.dev );
		if ( c->data.dev->isSequentialAccess() )
		    d->pi.dtp.setBytesTotal( 0 );
		else
		    d->pi.dtp.setBytesTotal( c->data.dev->size() );
	    }
	} else if ( c->command == Get ) {
	    if ( !c->is_ba && c->data.dev ) {
		d->pi.dtp.setDevice( c->data.dev );
	    }
	} else if ( c->command == Close ) {
	    d->state = QFtp::Closing;
	    emit stateChanged( d->state );
	}
	if ( !d->pi.sendCommands( c->rawCmds ) ) {
	    // ### error handling (this case should not happen)
	}
    }
}

void QFtp::piFinished( const QString& )
{
    QFtpPrivate *d = ::d( this );
    QFtpCommand *c = d->pending.getFirst();
    if ( c == 0 )
	return;

    if ( c->command == Close ) {
	// The order of in which the slots are called is arbitrary, so
	// disconnect the SIGNAL-SIGNAL temporary to make sure that we
	// don't get the commandFinished() signal before the stateChanged()
	// signal.
	if ( d->state != QFtp::Unconnected ) {
	    d->close_waitForStateChange = TRUE;
	    return;
	}
    }
    emit commandFinished( c->id, FALSE );

    d->pending.removeFirst();
    if ( d->pending.isEmpty() ) {
	emit done( FALSE );
    } else {
	startNextCommand();
    }
}

void QFtp::piError( int errorCode, const QString &text )
{
    QFtpPrivate *d = ::d( this );
    QFtpCommand *c = d->pending.getFirst();

    // non-fatal errors
    if ( c->command==Get && d->pi.currentCommand().startsWith("SIZE ") ) {
	d->pi.dtp.setBytesTotal( -1 );
	return;
    } else if ( c->command==Put && d->pi.currentCommand().startsWith("ALLO ") ) {
	return;
    }

    d->error = (Error)errorCode;
    switch ( currentCommand() ) {
	case ConnectToHost:
	    d->errorString = tr( "Connecting to host failed:\n%1" ).arg( text );
	    break;
	case Login:
	    d->errorString = tr( "Login failed:\n%1" ).arg( text );
	    break;
	case List:
	    d->errorString = tr( "Listing directory failed:\n%1" ).arg( text );
	    break;
	case Cd:
	    d->errorString = tr( "Changing directory failed:\n%1" ).arg( text );
	    break;
	case Get:
	    d->errorString = tr( "Downloading file failed:\n%1" ).arg( text );
	    break;
	case Put:
	    d->errorString = tr( "Uploading file failed:\n%1" ).arg( text );
	    break;
	case Remove:
	    d->errorString = tr( "Removing file failed:\n%1" ).arg( text );
	    break;
	case Mkdir:
	    d->errorString = tr( "Creating directory failed:\n%1" ).arg( text );
	    break;
	case Rmdir:
	    d->errorString = tr( "Removing directory failed:\n%1" ).arg( text );
	    break;
	default:
	    d->errorString = text;
	    break;
    }

    d->pi.clearPendingCommands();
    clearPendingCommands();
    emit commandFinished( c->id, TRUE );

    d->pending.removeFirst();
    if ( d->pending.isEmpty() )
	emit done( TRUE );
    else
	startNextCommand();
}

void QFtp::piConnectState( int state )
{
    QFtpPrivate *d = ::d( this );
    d->state = (State)state;
    emit stateChanged( d->state );
    if ( d->close_waitForStateChange ) {
	d->close_waitForStateChange = FALSE;
	piFinished( tr( "Connection closed" ) );
    }
}

void QFtp::piFtpReply( int code, const QString &text )
{
    if ( currentCommand() == RawCommand ) {
	QFtpPrivate *d = ::d( this );
	d->pi.rawCommand = TRUE;
	emit rawCommandReply( code, text );
    }
}

/*!
    Destructor.
*/
QFtp::~QFtp()
{
    abort();
    close();
    delete_d( this );
}

/**********************************************************************
 *
 * QFtp implementation of the QNetworkProtocol interface
 *
 *********************************************************************/
/*!  \reimp
*/
void QFtp::operationListChildren( QNetworkOperation *op )
{
    op->setState( StInProgress );

    cd( ( url()->path().isEmpty() ? QString( "/" ) : url()->path() ) );
    list();
    emit start( op );
}

/*!  \reimp
*/
void QFtp::operationMkDir( QNetworkOperation *op )
{
    op->setState( StInProgress );

    mkdir( op->arg( 0 ) );
}

/*!  \reimp
*/
void QFtp::operationRemove( QNetworkOperation *op )
{
    op->setState( StInProgress );

    cd( ( url()->path().isEmpty() ? QString( "/" ) : url()->path() ) );
    remove( QUrl( op->arg( 0 ) ).path() );
}

/*!  \reimp
*/
void QFtp::operationRename( QNetworkOperation *op )
{
    op->setState( StInProgress );

    cd( ( url()->path().isEmpty() ? QString( "/" ) : url()->path() ) );
    rename( op->arg( 0 ), op->arg( 1 ));
}

/*!  \reimp
*/
void QFtp::operationGet( QNetworkOperation *op )
{
    op->setState( StInProgress );

    QUrl u( op->arg( 0 ) );
    get( u.path() );
}

/*!  \reimp
*/
void QFtp::operationPut( QNetworkOperation *op )
{
    op->setState( StInProgress );

    QUrl u( op->arg( 0 ) );
    put( op->rawArg(1), u.path() );
}

/*!  \reimp
*/
bool QFtp::checkConnection( QNetworkOperation *op )
{
    QFtpPrivate *d = ::d( this );
    if ( state() == Unconnected && !d->npWaitForLoginDone ) {
	connect( this, SIGNAL(listInfo(const QUrlInfo&)),
		this, SLOT(npListInfo(const QUrlInfo&)) );
	connect( this, SIGNAL(done(bool)),
		this, SLOT(npDone(bool)) );
	connect( this, SIGNAL(stateChanged(int)),
		this, SLOT(npStateChanged(int)) );
	connect( this, SIGNAL(dataTransferProgress(int,int)),
		this, SLOT(npDataTransferProgress(int,int)) );
	connect( this, SIGNAL(readyRead()),
		this, SLOT(npReadyRead()) );

	d->npWaitForLoginDone = TRUE;
	switch ( op->operation() ) {
	    case OpGet:
	    case OpPut:
		{
		    QUrl u( op->arg( 0 ) );
		    connectToHost( u.host(), u.port() != -1 ? u.port() : 21 );
		}
		break;
	    default:
		connectToHost( url()->host(), url()->port() != -1 ? url()->port() : 21 );
		break;
	}
	QString user = url()->user().isEmpty() ? QString( "anonymous" ) : url()->user();
	QString pass = url()->password().isEmpty() ? QString( "anonymous@" ) : url()->password();
	login( user, pass );
    }

    if ( state() == LoggedIn )
	return TRUE;
    return FALSE;
}

/*!  \reimp
*/
int QFtp::supportedOperations() const
{
    return OpListChildren | OpMkDir | OpRemove | OpRename | OpGet | OpPut;
}

/*! \internal
    Parses the string, \a buffer, which is one line of a directory
    listing which came from the FTP server, and sets the values which
    have been parsed to the url info object, \a info.
*/
void QFtp::parseDir( const QString &buffer, QUrlInfo &info )
{
    QFtpDTP::parseDir( buffer, url()->user(), &info );
}

void QFtp::npListInfo( const QUrlInfo & i )
{
    if ( url() ) {
	QRegExp filt( url()->nameFilter(), FALSE, TRUE );
	if ( i.isDir() || filt.search( i.name() ) != -1 ) {
	    emit newChild( i, operationInProgress() );
	}
    } else {
	emit newChild( i, operationInProgress() );
    }
}

void QFtp::npDone( bool err )
{
    QFtpPrivate *d = ::d( this );

    bool emitFinishedSignal = FALSE;
    QNetworkOperation *op = operationInProgress();
    if ( op ) {
	if ( err ) {
	    op->setProtocolDetail( errorString() );
	    op->setState( StFailed );
	    if ( error() == HostNotFound ) {
		op->setErrorCode( (int)ErrHostNotFound );
	    } else {
		switch ( op->operation() ) {
		    case OpListChildren:
			op->setErrorCode( (int)ErrListChildren );
			break;
		    case OpMkDir:
			op->setErrorCode( (int)ErrMkDir );
			break;
		    case OpRemove:
			op->setErrorCode( (int)ErrRemove );
			break;
		    case OpRename:
			op->setErrorCode( (int)ErrRename );
			break;
		    case OpGet:
			op->setErrorCode( (int)ErrGet );
			break;
		    case OpPut:
			op->setErrorCode( (int)ErrPut );
			break;
		}
	    }
	    emitFinishedSignal = TRUE;
	} else if ( !d->npWaitForLoginDone ) {
	    switch ( op->operation() ) {
		case OpRemove:
		    emit removed( op );
		    break;
		case OpMkDir:
		    {
			QUrlInfo inf( op->arg( 0 ), 0, "", "", 0, QDateTime(),
				QDateTime(), TRUE, FALSE, FALSE, TRUE, TRUE, TRUE );
			emit newChild( inf, op );
			emit createdDirectory( inf, op );
		    }
		    break;
		case OpRename:
		    emit itemChanged( operationInProgress() );
		    break;
		default:
		    break;
	    }
	    op->setState( StDone );
	    emitFinishedSignal = TRUE;
	}
    }
    d->npWaitForLoginDone = FALSE;

    if ( state() == Unconnected ) {
	disconnect( this, SIGNAL(listInfo(const QUrlInfo&)),
		    this, SLOT(npListInfo(const QUrlInfo&)) );
	disconnect( this, SIGNAL(done(bool)),
		    this, SLOT(npDone(bool)) );
	disconnect( this, SIGNAL(stateChanged(int)),
		    this, SLOT(npStateChanged(int)) );
	disconnect( this, SIGNAL(dataTransferProgress(int,int)),
		    this, SLOT(npDataTransferProgress(int,int)) );
	disconnect( this, SIGNAL(readyRead()),
		    this, SLOT(npReadyRead()) );
    }

    // emit the finished() signal at the very end to avoid reentrance problems
    if ( emitFinishedSignal )
	emit finished( op );
}

void QFtp::npStateChanged( int state )
{
    if ( url() ) {
	if ( state == Connecting )
	    emit connectionStateChanged( ConHostFound, tr( "Host %1 found" ).arg( url()->host() ) );
	else if ( state == Connected )
	    emit connectionStateChanged( ConConnected, tr( "Connected to host %1" ).arg( url()->host() ) );
	else if ( state == Unconnected )
	    emit connectionStateChanged( ConClosed, tr( "Connection to %1 closed" ).arg( url()->host() ) );
    } else {
	if ( state == Connecting )
	    emit connectionStateChanged( ConHostFound, tr( "Host found" ) );
	else if ( state == Connected )
	    emit connectionStateChanged( ConConnected, tr( "Connected to host" ) );
	else if ( state == Unconnected )
	    emit connectionStateChanged( ConClosed, tr( "Connection closed" ) );
    }
}

void QFtp::npDataTransferProgress( int bDone, int bTotal )
{
    emit QNetworkProtocol::dataTransferProgress( bDone, bTotal, operationInProgress() );
}

void QFtp::npReadyRead()
{
    emit data( readAll(), operationInProgress() );
}

// ### unused -- delete in Qt 4.0
/*!  \internal
*/
void QFtp::hostFound()
{
}
/*!  \internal
*/
void QFtp::connected()
{
}
/*!  \internal
*/
void QFtp::closed()
{
}
/*!  \internal
*/
void QFtp::dataHostFound()
{
}
/*!  \internal
*/
void QFtp::dataConnected()
{
}
/*!  \internal
*/
void QFtp::dataClosed()
{
}
/*!  \internal
*/
void QFtp::dataReadyRead()
{
}
/*!  \internal
*/
void QFtp::dataBytesWritten( int )
{
}
/*!  \internal
*/
void QFtp::error( int )
{
}

#include "qftp.moc"

#endif // QT_NO_NETWORKPROTOCOL_FTP
