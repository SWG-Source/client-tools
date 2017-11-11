/****************************************************************************
** $Id: qt/qhostaddress.h   3.3.4   edited Oct 28 2003 $
**
** Definition of QHostAddress class.
**
** Created : 979899
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

#ifndef QHOSTADDRESS_H
#define QHOSTADDRESS_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

#if !defined( QT_MODULE_NETWORK ) || defined( QT_LICENSE_PROFESSIONAL ) || defined( QT_INTERNAL_NETWORK )
#define QM_EXPORT_NETWORK
#else
#define QM_EXPORT_NETWORK Q_EXPORT
#endif

#ifndef QT_NO_NETWORK
class QHostAddressPrivate;

typedef struct {
    Q_UINT8 c[16];
} Q_IPV6ADDR;

class QM_EXPORT_NETWORK QHostAddress
{
public:
    QHostAddress();
    QHostAddress( Q_UINT32 ip4Addr );
    QHostAddress( Q_UINT8 *ip6Addr );
    QHostAddress(const Q_IPV6ADDR &ip6Addr);
#ifndef QT_NO_STRINGLIST
    QHostAddress(const QString &address);
#endif
    QHostAddress( const QHostAddress & );
    virtual ~QHostAddress();

    QHostAddress & operator=( const QHostAddress & );

    void setAddress( Q_UINT32 ip4Addr );
    void setAddress( Q_UINT8 *ip6Addr );
#ifndef QT_NO_STRINGLIST
    bool setAddress( const QString& address );
#endif
    bool	 isIp4Addr()	 const; // obsolete
    Q_UINT32	 ip4Addr()	 const; // obsolete

    bool	 isIPv4Address() const;
    Q_UINT32	 toIPv4Address() const;
    bool	 isIPv6Address() const;
    Q_IPV6ADDR	 toIPv6Address() const;

#ifndef QT_NO_SPRINTF
    QString	 toString() const;
#endif

    bool	 operator==( const QHostAddress & ) const;
    bool	 isNull() const;

private:
    QHostAddressPrivate* d;
};

#endif //QT_NO_NETWORK
#endif
