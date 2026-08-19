/****************************************************************************
** $Id: qserversocket.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QServerSocket class.
**
** Created : 970521
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
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

#include "qserversocket.h"

#ifndef QT_NO_NETWORK

#include "qsocketnotifier.h"

class QServerSocketPrivate {
public:
    QServerSocketPrivate(): s(0), n(0) {}
    ~QServerSocketPrivate() { delete n; delete s; }
    QSocketDevice *s;
    QSocketNotifier *n;
};


/*!
    \class QServerSocket qserversocket.h
    \brief The QServerSocket class provides a TCP-based server.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    This class is a convenience class for accepting incoming TCP
    connections. You can specify the port or have QServerSocket pick
    one, and listen on just one address or on all the machine's
    addresses.

    Using the API is very simple: subclass QServerSocket, call the
    constructor of your choice, and implement newConnection() to
    handle new incoming connections. There is nothing more to do.

    (Note that due to lack of support in the underlying APIs,
    QServerSocket cannot accept or reject connections conditionally.)

    \sa QSocket, QSocketDevice, QHostAddress, QSocketNotifier
*/


/*!
    Creates a server socket object, that will serve the given \a port
    on all the addresses of this host. If \a port is 0, QServerSocket
    will pick a suitable port in a system-dependent manner. Use \a
    backlog to specify how many pending connections the server can
    have.

    The \a parent and \a name arguments are passed on to the QObject
    constructor.

    \warning On Tru64 Unix systems a value of 0 for \a backlog means
    that you don't accept any connections at all; you should specify a
    value larger than 0.
*/

QServerSocket::QServerSocket( Q_UINT16 port, int backlog,
			      QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QServerSocketPrivate;
    init( QHostAddress(), port, backlog );
}


/*!
    Creates a server socket object, that will serve the given \a port
    only on the given \a address. Use \a backlog to specify how many
    pending connections the server can have.

    The \a parent and \a name arguments are passed on to the QObject
    constructor.

    \warning On Tru64 Unix systems a value of 0 for \a backlog means
    that you don't accept any connections at all; you should specify a
    value larger than 0.
*/

QServerSocket::QServerSocket( const QHostAddress & address, Q_UINT16 port,
			      int backlog,
			      QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QServerSocketPrivate;
    init( address, port, backlog );
}


/*!
    Construct an empty server socket.

    This constructor, in combination with setSocket(), allows us to
    use the QServerSocket class as a wrapper for other socket types
    (e.g. Unix Domain Sockets under Unix).

    The \a parent and \a name arguments are passed on to the QObject
    constructor.

    \sa setSocket()
*/

QServerSocket::QServerSocket( QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QServerSocketPrivate;
}


/*!
    Returns TRUE if the construction succeeded; otherwise returns FALSE.
*/
bool QServerSocket::ok() const
{
    return !!d->s;
}

/*
  The common bit of the constructors.
 */
void QServerSocket::init( const QHostAddress & address, Q_UINT16 port, int backlog )
{
    d->s = new QSocketDevice( QSocketDevice::Stream, address.isIPv4Address()
			      ? QSocketDevice::IPv4 : QSocketDevice::IPv6, 0 );
#if !defined(Q_OS_WIN32)
    // Under Unix, we want to be able to use the port, even if a socket on the
    // same address-port is in TIME_WAIT. Under Windows this is possible anyway
    // -- furthermore, the meaning of reusable is different: it means that you
    // can use the same address-port for multiple listening sockets.
    d->s->setAddressReusable( TRUE );
#endif
    if ( d->s->bind( address, port )
      && d->s->listen( backlog ) )
    {
	d->n = new QSocketNotifier( d->s->socket(), QSocketNotifier::Read,
				    this, "accepting new connections" );
	connect( d->n, SIGNAL(activated(int)),
		 this, SLOT(incomingConnection(int)) );
    } else {
	qWarning( "QServerSocket: failed to bind or listen to the socket" );
	delete d->s;
	d->s = 0;
    }
}


/*!
    Destroys the socket.

    This causes any backlogged connections (connections that have
    reached the host, but not yet been completely set up by calling
    QSocketDevice::accept()) to be severed.

    Existing connections continue to exist; this only affects the
    acceptance of new connections.
*/
QServerSocket::~QServerSocket()
{
    delete d;
}


/*!
    \fn void QServerSocket::newConnection( int socket )

    This pure virtual function is responsible for setting up a new
    incoming connection. \a socket is the fd (file descriptor) for the
    newly accepted connection.
*/


void QServerSocket::incomingConnection( int )
{
    int fd = d->s->accept();
    if ( fd >= 0 )
	newConnection( fd );
}


/*!
    Returns the port number on which this server socket listens. This
    is always non-zero; if you specify 0 in the constructor,
    QServerSocket will pick a non-zero port itself. ok() must be TRUE
    before calling this function.

    \sa address() QSocketDevice::port()
*/
Q_UINT16 QServerSocket::port() const
{
    if ( !d || !d->s )
	return 0;
    return d->s->port();
}


/*!
    Returns the operating system socket.
*/
int QServerSocket::socket() const
{
    if ( !d || !d->s )
	return -1;

    return d->s->socket();
}

/*!
    Returns the address on which this object listens, or 0.0.0.0 if
    this object listens on more than one address. ok() must be TRUE
    before calling this function.

    \sa port() QSocketDevice::address()
*/
QHostAddress QServerSocket::address() const
{
    if ( !d || !d->s )
	return QHostAddress();

    return d->s->address();
}


/*!
    Returns a pointer to the internal socket device. The returned
    pointer is 0 if there is no connection or pending connection.

    There is normally no need to manipulate the socket device directly
    since this class does all the necessary setup for most client or
    server socket applications.
*/
QSocketDevice *QServerSocket::socketDevice()
{
    if ( !d )
	return 0;

    return d->s;
}


/*!
    Sets the socket to use \a socket. bind() and listen() should
    already have been called for \a socket.

    This allows us to use the QServerSocket class as a wrapper for
    other socket types (e.g. Unix Domain Sockets).
*/
void QServerSocket::setSocket( int socket )
{
    delete d;
    d = new QServerSocketPrivate;
    d->s = new QSocketDevice( socket, QSocketDevice::Stream );
    d->n = new QSocketNotifier( d->s->socket(), QSocketNotifier::Read,
	       this, "accepting new connections" );
    connect( d->n, SIGNAL(activated(int)),
	     this, SLOT(incomingConnection(int)) );
}

#endif //QT_NO_NETWORK
