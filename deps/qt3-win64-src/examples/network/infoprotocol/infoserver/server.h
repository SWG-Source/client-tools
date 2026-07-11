/****************************************************************************
** $Id: server.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef SERVER_H
#define SERVER_H

#include <qsocket.h>
#include <qserversocket.h>

#include "infodata.h"
#include "serverbase.h"

static const Q_UINT16 infoPort = 42417;


/*
  The ServerInfo class provides a small GUI for the server. It also creates the
  SimpleServer and as a result the server.
*/
class ServerInfo : public ServerInfoBase
{
    Q_OBJECT
public:
    ServerInfo( Q_UINT16 port = infoPort, QWidget *parent = 0, const char *name = 0 );

private slots:
    void newConnect();
};


class SimpleServer : public QServerSocket
{
    Q_OBJECT
public:
    SimpleServer( Q_UINT16 port = infoPort, QObject* parent = 0, const char *name = 0 );
    void newConnection( int socket );

signals:
    void newConnect();

private:
    InfoData info;
};


class ClientSocket : public QSocket
{
    Q_OBJECT
public:
    ClientSocket( int sock, InfoData *i, QObject *parent = 0, const char *name = 0 );

private slots:
    void readClient();
    void connectionClosed();

private:
    QString processCommand( const QString& command );
    InfoData *info;
};


#endif //SERVER_H

