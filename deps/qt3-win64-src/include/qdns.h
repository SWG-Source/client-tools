/****************************************************************************
** $Id: qdns.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Definition of QDns class.
**
** Created : 991122
**
** Copyright (C) 1999-2007 Trolltech ASA.  All rights reserved.
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

#ifndef QDNS_H
#define QDNS_H

#ifndef QT_H
#include "qobject.h"
#include "qhostaddress.h"
#include "qsocketnotifier.h"
#include "qstringlist.h"
#endif // QT_H

#if !defined( QT_MODULE_NETWORK ) || defined( QT_LICENSE_PROFESSIONAL ) || defined( QT_INTERNAL_NETWORK )
#define QM_EXPORT_DNS
#else
#define QM_EXPORT_DNS Q_EXPORT
#endif

#ifndef QT_NO_DNS

//#define Q_DNS_SYNCHRONOUS

class QDnsPrivate;

class QM_EXPORT_DNS QDns: public QObject {
    Q_OBJECT
public:
    enum RecordType {
	None,
	A, Aaaa,
	Mx, Srv,
	Cname,
	Ptr,
	Txt
    };

    QDns();
    QDns( const QString & label, RecordType rr = A );
    QDns( const QHostAddress & address, RecordType rr = Ptr );
    virtual ~QDns();

    // to set/change the query
    virtual void setLabel( const QString & label );
    virtual void setLabel( const QHostAddress & address );
    QString label() const { return l; }

    virtual void setRecordType( RecordType rr = A );
    RecordType recordType() const { return t; }

    // whether something is happening behind the scenes
    bool isWorking() const;

    // to query for replies
    QValueList<QHostAddress> addresses() const;

    class QM_EXPORT_DNS MailServer {
    public:
	MailServer( const QString & n=QString::null, Q_UINT16 p=0 )
	    :name(n), priority(p) {}
	QString name;
	Q_UINT16 priority;
	Q_DUMMY_COMPARISON_OPERATOR(MailServer)
    };
    QValueList<MailServer> mailServers() const;

    class QM_EXPORT_DNS Server {
    public:
	Server(const QString & n=QString::null, Q_UINT16 p=0, Q_UINT16 w=0, Q_UINT16 po=0 )
	    : name(n), priority(p), weight(w), port(po) {}
	QString name;
	Q_UINT16 priority;
	Q_UINT16 weight;
	Q_UINT16 port;
	Q_DUMMY_COMPARISON_OPERATOR(Server)
    };
    QValueList<Server> servers() const;

    QStringList hostNames() const;

    QStringList texts() const;

    QString canonicalName() const; // ### real-world but uncommon: QStringList

    QStringList qualifiedNames() const { return n; }

#if defined(Q_DNS_SYNCHRONOUS)
protected:
    void connectNotify( const char *signal );
#endif

signals:
    void resultsReady();

private slots:
    void startQuery();

private:
    static void doResInit();
    void setStartQueryTimer();
    static QString toInAddrArpaDomain( const QHostAddress &address );
#if defined(Q_DNS_SYNCHRONOUS)
    void doSynchronousLookup();
#endif

    QString l;
    QStringList n;
    RecordType t;
    QDnsPrivate * d;

    friend class QDnsAnswer;
    friend class QDnsManager;
};


// QDnsSocket are sockets that are used for DNS lookup

class QDnsSocket: public QObject {
    Q_OBJECT
    // note: Private not public.  This class contains NO public API.
protected:
    QDnsSocket( QObject *, const char * );
    virtual ~QDnsSocket();

private slots:
    virtual void cleanCache();
    virtual void retransmit();
    virtual void answer();
};

#endif // QT_NO_DNS

#endif // QDNS_H
