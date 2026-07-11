/****************************************************************************
** $Id: client.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef CLIENT_H
#define CLIENT_H

#include "clientbase.h"

class QSocket;
class QTextEdit;
class QLineEdit;
class QListBox;
class QLabel;

static const Q_UINT16 infoPort = 42417;

class ClientInfo : public ClientInfoBase
{
    Q_OBJECT
public:
    ClientInfo( QWidget *parent = 0, const char *name = 0 );

private:
    enum Operation { List, Get };

private slots:
    void connectToServer();
    void selectItem( const QString& item );
    void stepBack();
    void sendToServer( Operation op, const QString& location );
    void socketConnected();
    void socketReadyRead();
    void socketConnectionClosed();
    void socketError( int code );

private:
    QSocket *socket;
};

#endif // CLIENT_H

