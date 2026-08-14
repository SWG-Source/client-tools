/****************************************************************************
** $Id: qip.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef QIP_H
#define QIP_H

#include <qnetworkprotocol.h>

class QSocket;

static const Q_UINT16 infoPort = 42417;


class Qip : public QNetworkProtocol
{
    Q_OBJECT

public:
    Qip();
    virtual int supportedOperations() const;

protected:
    virtual void operationListChildren( QNetworkOperation *op ); 
    virtual void operationGet( QNetworkOperation *op );
    virtual bool checkConnection( QNetworkOperation *op );

private slots:
    void socketConnected();
    void socketReadyRead();
    void socketConnectionClosed();
    void socketError( int code );

private:
    QSocket *socket;
    enum State { Start, List, Data } state;
    void error( int code, const QString& msg );
};



#endif // QIP_H

