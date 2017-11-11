/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
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

#ifndef QASSISTANTCLIENT_H
#define QASSISTANTCLIENT_H

#include <qobject.h>

class QSocket;
class QProcess;

class QAssistantClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool open READ isOpen )

public:
    QAssistantClient( const QString &path, QObject *parent = 0, const char *name = 0 );
    ~QAssistantClient();

    bool isOpen() const;

    void setArguments( const QStringList &args );

public slots:
    virtual void openAssistant();
    virtual void closeAssistant();
    virtual void showPage( const QString &page );

signals:
    void assistantOpened();
    void assistantClosed();
    void error( const QString &msg );

private slots:
    void socketConnected();
    void socketConnectionClosed();
    void readPort();
    void socketError( int );
    void readStdError();

private:
    QSocket *socket;
    QProcess *proc;
    Q_UINT16 port;
    QString host, assistantCommand, pageBuffer;
    bool opened;
};

#endif
