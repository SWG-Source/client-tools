/****************************************************************************
** $Id: qt/qftp.h   3.3.4   edited May 27 2003 $
**
** Definition of QFtp class.
**
** Created : 970521
**
** Copyright (C) 1997-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
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

#ifndef QFTP_H
#define QFTP_H

#ifndef QT_H
#include "qstring.h" // char*->QString conversion
#include "qurlinfo.h"
#include "qnetworkprotocol.h"
#endif // QT_H

#if !defined( QT_MODULE_NETWORK ) || defined( QT_LICENSE_PROFESSIONAL ) || defined( QT_INTERNAL_NETWORK )
#define QM_EXPORT_FTP
#else
#define QM_EXPORT_FTP Q_EXPORT
#endif

#ifndef QT_NO_NETWORKPROTOCOL_FTP


class QSocket;
class QFtpCommand;

class QM_EXPORT_FTP QFtp : public QNetworkProtocol
{
    Q_OBJECT

public:
    QFtp(); // ### Qt 4.0: get rid of this overload
    QFtp( QObject *parent, const char *name=0 );
    virtual ~QFtp();

    int supportedOperations() const;

    // non-QNetworkProtocol functions:
    enum State {
	Unconnected,
	HostLookup,
	Connecting,
	Connected,
	LoggedIn,
	Closing
    };
    enum Error {
	NoError,
	UnknownError,
	HostNotFound,
	ConnectionRefused,
	NotConnected
    };
    enum Command {
	None,
	ConnectToHost,
	Login,
	Close,
	List,
	Cd,
	Get,
	Put,
	Remove,
	Mkdir,
	Rmdir,
	Rename,
	RawCommand
    };

    int connectToHost( const QString &host, Q_UINT16 port=21 );
    int login( const QString &user=QString::null, const QString &password=QString::null );
    int close();
    int list( const QString &dir=QString::null );
    int cd( const QString &dir );
    int get( const QString &file, QIODevice *dev=0 );
    int put( const QByteArray &data, const QString &file );
    int put( QIODevice *dev, const QString &file );
    int remove( const QString &file );
    int mkdir( const QString &dir );
    int _rmdir( const QString &dir );
    int rename( const QString &oldname, const QString &newname );

    int rawCommand( const QString &command );

    Q_ULONG bytesAvailable() const;
    Q_LONG readBlock( char *data, Q_ULONG maxlen );
    QByteArray readAll();

    int currentId() const;
    QIODevice* currentDevice() const;
    Command currentCommand() const;
    bool hasPendingCommands() const;
    void clearPendingCommands();

    State state() const;

    Error error() const;
    QString errorString() const;

public slots:
    void abort();

signals:
    void stateChanged( int );
    void listInfo( const QUrlInfo& );
    void readyRead();
    void dataTransferProgress( int, int );
    void rawCommandReply( int, const QString& );

    void commandStarted( int );
    void commandFinished( int, bool );
    void done( bool );

protected:
    void parseDir( const QString &buffer, QUrlInfo &info ); // ### Qt 4.0: delete this? (not public API)
    void operationListChildren( QNetworkOperation *op );
    void operationMkDir( QNetworkOperation *op );
    void operationRemove( QNetworkOperation *op );
    void operationRename( QNetworkOperation *op );
    void operationGet( QNetworkOperation *op );
    void operationPut( QNetworkOperation *op );

    // ### Qt 4.0: delete these
    // unused variables:
    QSocket *commandSocket, *dataSocket;
    bool connectionReady, passiveMode;
    int getTotalSize, getDoneSize;
    bool startGetOnFail;
    int putToWrite, putWritten;
    bool errorInListChildren;

private:
    void init();
    int addCommand( QFtpCommand * );

    bool checkConnection( QNetworkOperation *op );

private slots:
    void startNextCommand();
    void piFinished( const QString& );
    void piError( int, const QString& );
    void piConnectState( int );
    void piFtpReply( int, const QString& );

private slots:
    void npListInfo( const QUrlInfo & );
    void npDone( bool );
    void npStateChanged( int );
    void npDataTransferProgress( int, int );
    void npReadyRead();

protected slots:
    // ### Qt 4.0: delete these
    void hostFound();
    void connected();
    void closed();
    void dataHostFound();
    void dataConnected();
    void dataClosed();
    void dataReadyRead();
    void dataBytesWritten( int nbytes );
    void error( int );
};

#endif // QT_NO_NETWORKPROTOCOL_FTP

#endif // QFTP_H
