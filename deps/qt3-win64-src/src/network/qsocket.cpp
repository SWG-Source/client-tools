/****************************************************************************
** $Id: qsocket.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QSocket class.
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

#include "qsocket.h"
#ifndef QT_NO_NETWORK
#include "qptrlist.h"
#include "qtimer.h"
#include "qsocketdevice.h"
#include "qdns.h"
#include "private/qinternal_p.h"

#include <string.h>
#ifndef NO_ERRNO_H
#include <errno.h>
#endif

//#define QSOCKET_DEBUG

/*
  Perhaps this private functionality needs to be refactored.

  Comment from Robert D Gatlin (Intel):

    It would be nice to have the functionality inherent in QSocket available
    as a separate class as a standard part of the Qt library, something along
    the line of:

      class QByteBuffer : public QIODevice { ... }

    The same class could/would be used within QSocket for the Read/Write
    buffers.

    The above class could be used in the following way(s):

	buffer.open( IO_WriteOnly | IO_Append );
	buffer.writeBlock( a ); // a = QByteArray
	buffer.close();

	QByteArray b;
	b.resize( buffer.size() );
	buffer.open( IO_ReadOnly );
	buffer.readBlock( b.data(), b.size() );
	buffer.close();

    But would also be useable with QDataStream (via QIODevice) with:

	buffer.open( IO_WriteOnly | IO_Append );
	QDataStream is( &buffer );
	is << 100;
	buffer.close();

	buffer.open( IO_ReadOnly );
	QDataStream os( &buffer );
	Q_UINT32 x;
	os >> x;
	buffer.close();

    The real usefulness is with any situations where data (QByteArray) arrives
    incrementally (as in QSocket and filter case above).

    I tried using QBuffer, but QBuffer does not trim bytes from the front of
    the buffer in cases like:

	QBuffer buf;
	buf.open( IO_ReadOnly );
	QDataStream ds( &buf );
	Q_INT32 x;
	ds >> x;
	buf.close();

    In the above case, buf.size() will be identical before and after the
    operation with QDataStream. Based on the implementation of QBuffer, it
    does not appear well suited for this kind of operation.
*/

// Private class for QSocket

class QSocketPrivate {
public:
    QSocketPrivate();
   ~QSocketPrivate();
    void closeSocket();
    void close();
    void connectionClosed();
    void setSocketDevice( QSocket *q, QSocketDevice *device );

    QSocket::State	state;			// connection state
    QString		host;			// host name
    Q_UINT16		port;			// host port
    QSocketDevice      *socket;			// connection socket
    QSocketNotifier    *rsn, *wsn;		// socket notifiers
    QMembuf		rba;			// read buffer
    Q_ULONG		readBufferSize;		// limit for the read buffer size
    QPtrList<QByteArray> wba;			// list of write bufs
    QHostAddress	addr;			// connection address
    QValueList<QHostAddress> addresses;		// alternatives looked up
    QIODevice::Offset	wsize;			// write total buf size
    QIODevice::Offset	windex;			// write index
#ifndef QT_NO_DNS
    QDns	       *dns4;
    QDns	       *dns6;
#endif
    static QPtrList<QSocket> sn_read_alreadyCalled; // used to avoid unwanted recursion
    QValueList<QHostAddress> l4;
    QValueList<QHostAddress> l6;
};

QPtrList<QSocket> QSocketPrivate::sn_read_alreadyCalled;

QSocketPrivate::QSocketPrivate()
    : state(QSocket::Idle), host(QString::fromLatin1("")), port(0),
      socket(0), rsn(0), wsn(0), readBufferSize(0), wsize(0), windex(0)
{
#ifndef QT_NO_DNS
    dns4 = 0;
    dns6 = 0;
#endif
    wba.setAutoDelete( TRUE );
}

QSocketPrivate::~QSocketPrivate()
{
    close();
    delete socket;
#ifndef QT_NO_DNS
    delete dns4;
    delete dns6;
#endif
}

void QSocketPrivate::closeSocket()
{
    // Order is important here - the socket notifiers must go away
    // before the socket does, otherwise libc or the kernel will
    // become unhappy.
    delete rsn;
    rsn = 0;
    delete wsn;
    wsn = 0;
    if ( socket )
	socket->close();
}

void QSocketPrivate::close()
{
    closeSocket();
    wsize = 0;
    rba.clear(); wba.clear();
    windex = 0;
}

void QSocketPrivate::connectionClosed()
{
    // We keep the open state in case there's unread incoming data
    state = QSocket::Idle;
    closeSocket();
    wba.clear();
    windex = wsize = 0;
}

void QSocketPrivate::setSocketDevice( QSocket *q, QSocketDevice *device )
{
    delete socket;
    delete rsn;
    delete wsn;

    if ( device ) {
	socket = device;
    } else {
	socket = new QSocketDevice( QSocketDevice::Stream,
				    ( addr.isIPv4Address() ?
				      QSocketDevice::IPv4 :
				      QSocketDevice::IPv6 ), 0 );
	socket->setBlocking( FALSE );
	socket->setAddressReusable( TRUE );
    }

    rsn = new QSocketNotifier( socket->socket(),
			       QSocketNotifier::Read, q, "read" );
    wsn = new QSocketNotifier( socket->socket(),
			       QSocketNotifier::Write, q, "write" );

    QObject::connect( rsn, SIGNAL(activated(int)), q, SLOT(sn_read()) );
    rsn->setEnabled( FALSE );
    QObject::connect( wsn, SIGNAL(activated(int)), q, SLOT(sn_write()) );
    wsn->setEnabled( FALSE );
}

/*!
    \class QSocket qsocket.h
    \brief The QSocket class provides a buffered TCP connection.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    It provides a totally non-blocking QIODevice, and modifies and
    extends the API of QIODevice with socket-specific code.

    Note that a QApplication must have been constructed before this
    class can be used.

    The functions you're likely to call most are connectToHost(),
    bytesAvailable(), canReadLine() and the ones it inherits from
    QIODevice.

    connectToHost() is the most-used function. As its name implies,
    it opens a connection to a named host.

    Most network protocols are either packet-oriented or
    line-oriented. canReadLine() indicates whether a connection
    contains an entire unread line or not, and bytesAvailable()
    returns the number of bytes available for reading.

    The signals error(), connected(), readyRead() and
    connectionClosed() inform you of the progress of the connection.
    There are also some less commonly used signals. hostFound() is
    emitted when connectToHost() has finished its DNS lookup and is
    starting its TCP connection. delayedCloseFinished() is emitted
    when close() succeeds. bytesWritten() is emitted when QSocket
    moves data from its "to be written" queue into the TCP
    implementation.

    There are several access functions for the socket: state() returns
    whether the object is idle, is doing a DNS lookup, is connecting,
    has an operational connection, etc. address() and port() return
    the IP address and port used for the connection. The peerAddress()
    and peerPort() functions return the IP address and port used by
    the peer, and peerName() returns the name of the peer (normally
    the name that was passed to connectToHost()). socketDevice()
    returns a pointer to the QSocketDevice used for this socket.

    QSocket inherits QIODevice, and reimplements some functions. In
    general, you can treat it as a QIODevice for writing, and mostly
    also for reading. The match isn't perfect, since the QIODevice
    API is designed for devices that are controlled by the same
    machine, and an asynchronous peer-to-peer network connection isn't
    quite like that. For example, there is nothing that matches
    QIODevice::size() exactly. The documentation for open(), close(),
    flush(), size(), at(), atEnd(), readBlock(), writeBlock(),
    getch(), putch(), ungetch() and readLine() describes the
    differences in detail.

    \warning QSocket is not suitable for use in threads. If you need
    to uses sockets in threads use the lower-level QSocketDevice class.

    \warning Because Qt doesn't use the native socketstream
    implementation on Mac OS X, QSocket has an implicit transfer
    latency of 100ms. You can achieve lower latency on Mac OS X by
    using QSocketDevice instead.

    \sa QSocketDevice, QHostAddress, QSocketNotifier
*/


/*!
    Creates a QSocket object in \c QSocket::Idle state.

    The \a parent and \a name arguments are passed on to the QObject
    constructor.

    Note that a QApplication must have been constructed before sockets
    can be used.
*/

QSocket::QSocket( QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QSocketPrivate;
    setSocketDevice( 0 );
    setFlags( IO_Direct );
    resetStatus();
}


/*!
    Destroys the socket. Closes the connection if necessary.

    \sa close()
*/

QSocket::~QSocket()
{
#if defined(QSOCKET_DEBUG)
    qDebug( "QSocket (%s): Destroy", name() );
#endif
    if ( state() != Idle )
	close();
    Q_ASSERT( d != 0 );
    delete d;
}


/*!
    Returns a pointer to the internal socket device.

    There is normally no need to manipulate the socket device directly
    since this class does the necessary setup for most applications.
*/

QSocketDevice *QSocket::socketDevice()
{
    return d->socket;
}

/*!
    Sets the internal socket device to \a device. Passing a \a device
    of 0 will cause the internal socket device to be used. Any
    existing connection will be disconnected before using the new \a
    device.

    The new device should not be connected before being associated
    with a QSocket; after setting the socket call connectToHost() to
    make the connection.

    This function is useful if you need to subclass QSocketDevice and
    want to use the QSocket API, for example, to implement Unix domain
    sockets.
*/

void QSocket::setSocketDevice( QSocketDevice *device )
{
    if ( state() != Idle )
	close();
    d->setSocketDevice( this, device );
}

/*!
    \enum QSocket::State

    This enum defines the connection states:

    \value Idle if there is no connection
    \value HostLookup during a DNS lookup
    \value Connecting during TCP connection establishment
    \value Connected when there is an operational connection
    \value Closing if the socket is closing down, but is not yet closed.
*/

/*!
    Returns the current state of the socket connection.

    \sa QSocket::State
*/

QSocket::State QSocket::state() const
{
    return d->state;
}


#ifndef QT_NO_DNS

/*!
    Attempts to make a connection to \a host on the specified \a port
    and return immediately.

    Any connection or pending connection is closed immediately, and
    QSocket goes into the \c HostLookup state. When the lookup
    succeeds, it emits hostFound(), starts a TCP connection and goes
    into the \c Connecting state. Finally, when the connection
    succeeds, it emits connected() and goes into the \c Connected
    state. If there is an error at any point, it emits error().

    \a host may be an IP address in string form, or it may be a DNS
    name. QSocket will do a normal DNS lookup if required. Note that
    \a port is in native byte order, unlike some other libraries.

    \sa state()
*/

void QSocket::connectToHost( const QString &host, Q_UINT16 port )
{
#if defined(QSOCKET_DEBUG)
    qDebug( "QSocket (%s)::connectToHost: host %s, port %d",
	    name(), host.ascii(), port );
#endif
    setSocketIntern( -1 );
    d->state = HostLookup;
    d->host = host;
    d->port = port;
    d->dns4 = new QDns( host, QDns::A );
    d->dns6 = new QDns( host, QDns::Aaaa );

    // try if the address is already available (for faster connecting...)
    tryConnecting();
    if ( d->state == HostLookup ) {
	connect( d->dns4, SIGNAL(resultsReady()),
		 this, SLOT(tryConnecting()) );
	connect( d->dns6, SIGNAL(resultsReady()),
		 this, SLOT(tryConnecting()) );
    }
}

#endif


/*!
    This private slots continues the connection process where
    connectToHost() leaves off.
*/

void QSocket::tryConnecting()
{
#if defined(QSOCKET_DEBUG)
    qDebug( "QSocket (%s)::tryConnecting()", name() );
#endif
    // ### this ifdef isn't correct - addresses() also does /etc/hosts and
    // numeric-address-as-string handling.
#ifndef QT_NO_DNS

    if ( d->dns4 ) {
	d->l4 = d->dns4->addresses();
	if ( !d->l4.isEmpty() || !d->dns4->isWorking() ) {
#if defined(QSOCKET_DEBUG)
	    qDebug( "QSocket (%s)::tryConnecting: host %s, port %d: "
		    "%d IPv4 addresses",
		    name(), d->host.ascii(), d->port, d->l4.count() );
#endif
	    delete d->dns4;
	    d->dns4 = 0;
	}
    }

    if ( d->dns6 ) {
	d->l6 = d->dns6->addresses();
	if ( !d->l6.isEmpty() || !d->dns6->isWorking() ) {
#if defined(QSOCKET_DEBUG)
	    qDebug( "QSocket (%s)::tryConnecting: host %s, port %d: "
		    "%d IPv6 addresses",
		    name(), d->host.ascii(), d->port, d->l6.count() );
#endif
	    delete d->dns6;
	    d->dns6 = 0;
	}
    }

    if ( d->state == HostLookup ) {
	if ( d->l4.isEmpty() && d->l6.isEmpty() &&
	     !d->dns4 && !d->dns6 ) {
	    // no results and we're not still looking: give up
	    d->state = Idle;
	    emit error( ErrHostNotFound );
	    return;
	}
	if ( d->l4.isEmpty() && d->l6.isEmpty() ) {
	    // no results (yet): try again later
	    return;
	}

	// we've found something. press on with that. if we later find
	// more, fine.
	emit hostFound();
	d->state = Connecting;
    }

    if ( d->state == Connecting ) {
	d->addresses += d->l4;
	d->addresses += d->l6;
	d->l4.clear();
	d->l6.clear();

	// try one address at a time, falling back to the next one if
	// there is a connection failure. (should also support a timeout,
	// or do multiple TCP-level connects at a time, with staggered
	// starts to avoid bandwidth waste and cause fewer
	// "connect-and-abort" errors. but that later.)
	bool stuck = TRUE;
	while( stuck ) {
	    stuck = FALSE;
	    if ( d->socket &&
		 d->socket->connect( d->addr, d->port ) == FALSE ) {
		if ( d->socket->error() == QSocketDevice::NoError ) {
		    if ( d->wsn )
			d->wsn->setEnabled( TRUE );
		    return; // not serious, try again later
		}

#if defined(QSOCKET_DEBUG)
		qDebug( "QSocket (%s)::tryConnecting: "
			"Gave up on IP address %s",
			name(), d->socket->peerAddress().toString().ascii() );
#endif
		delete d->wsn;
		d->wsn = 0;
		delete d->rsn;
		d->rsn = 0;
		delete d->socket;
		d->socket = 0;

                if(d->addresses.isEmpty()) {
                    emit error( ErrConnectionRefused );
                    return;
                }
            }
	    // if the host has more addresses, try another some.
	    if ( d->socket == 0 && !d->addresses.isEmpty() ) {
		d->addr = *d->addresses.begin();
		d->addresses.remove( d->addresses.begin() );
		d->setSocketDevice( this, 0 );
		stuck = TRUE;
#if defined(QSOCKET_DEBUG)
		qDebug( "QSocket (%s)::tryConnecting: Trying IP address %s",
			name(), d->addr.toString().ascii() );
#endif
	    }
	};

	// The socket write notifier will fire when the connection succeeds
	if ( d->wsn )
	    d->wsn->setEnabled( TRUE );
    }
#endif
}

/*!
    \enum QSocket::Error

    This enum specifies the possible errors:
    \value ErrConnectionRefused if the connection was refused
    \value ErrHostNotFound if the host was not found
    \value ErrSocketRead if a read from the socket failed
*/

/*!
    \fn void QSocket::error( int )

    This signal is emitted after an error occurred. The parameter is
    the \l Error value.
*/

/*!
    \fn void QSocket::hostFound()

    This signal is emitted after connectToHost() has been called and
    the host lookup has succeeded.

    \sa connected()
*/


/*!
    \fn void QSocket::connected()

    This signal is emitted after connectToHost() has been called and a
    connection has been successfully established.

    \sa connectToHost(), connectionClosed()
*/


/*!
    \fn void QSocket::connectionClosed()

    This signal is emitted when the other end has closed the
    connection. The read buffers may contain buffered input data which
    you can read after the connection was closed.

    \sa connectToHost(), close()
*/


/*!
    \fn void QSocket::delayedCloseFinished()

    This signal is emitted when a delayed close is finished.

    If you call close() and there is buffered output data to be
    written, QSocket goes into the \c QSocket::Closing state and
    returns immediately. It will then keep writing to the socket until
    all the data has been written. Then, the delayedCloseFinished()
    signal is emitted.

    \sa close()
*/


/*!
    \fn void QSocket::readyRead()

    This signal is emitted every time there is new incoming data.

    Bear in mind that new incoming data is only reported once; if you do not
    read all the data, this class buffers the data and you can read it later,
    but no signal is emitted unless new data arrives. A good practice is to
    read all data in the slot connected to this signal unless you are sure that
    you need to receive more data to be able to process it.

    \sa readBlock(), readLine(), bytesAvailable()
*/


/*!
    \fn void QSocket::bytesWritten( int nbytes )

    This signal is emitted when data has been written to the network.
    The \a nbytes parameter specifies how many bytes were written.

    The bytesToWrite() function is often used in the same context; it
    indicates how many buffered bytes there are left to write.

    \sa writeBlock(), bytesToWrite()
*/


/*!
    Opens the socket using the specified QIODevice file mode \a m.
    This function is called automatically when needed and you should
    not call it yourself.

    \sa close()
*/

bool QSocket::open( int m )
{
    if ( isOpen() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QSocket::open: Already open" );
#endif
	return FALSE;
    }
    QIODevice::setMode( m & IO_ReadWrite );
    setState( IO_Open );
    return TRUE;
}


/*!
    Closes the socket.

    The read buffer is cleared.

    If the output buffer is empty, the state is set to \c
    QSocket::Idle and the connection is terminated immediately. If the
    output buffer still contains data to be written, QSocket goes into
    the \c QSocket::Closing state and the rest of the data will be
    written. When all of the outgoing data have been written, the
    state is set to \c QSocket::Idle and the connection is terminated.
    At this point, the delayedCloseFinished() signal is emitted.

    If you don't want that the data of the output buffer is written, call
    clearPendingData() before you call close().

    \sa state(), bytesToWrite() clearPendingData()
*/

void QSocket::close()
{
    if ( !isOpen() || d->state == Idle )	// already closed
	return;
    if ( d->state == Closing )
	return;
    if ( !d->rsn || !d->wsn )
	return;
#if defined(QSOCKET_DEBUG)
    qDebug( "QSocket (%s): close socket", name() );
#endif
    if ( d->socket && d->wsize ) {		// there's data to be written
	d->state = Closing;
	if ( d->rsn )
	    d->rsn->setEnabled( FALSE );
	if ( d->wsn )
	    d->wsn->setEnabled( TRUE );
	d->rba.clear();				// clear incoming data
	return;
    }
    setFlags( IO_Sequential );
    resetStatus();
    setState( 0 );
    d->close();
    d->state = Idle;
}


/*!
    This function consumes \a nbytes bytes of data from the write
    buffer.
*/

bool QSocket::consumeWriteBuf( Q_ULONG nbytes )
{
    if ( nbytes <= 0 || nbytes > d->wsize )
	return FALSE;
#if defined(QSOCKET_DEBUG)
    qDebug( "QSocket (%s): skipWriteBuf %d bytes", name(), (int)nbytes );
#endif
    d->wsize -= nbytes;
    for ( ;; ) {
	QByteArray *a = d->wba.first();
	if ( d->windex + nbytes >= a->size() ) {
	    nbytes -= a->size() - d->windex;
	    d->wba.remove();
	    d->windex = 0;
	    if ( nbytes == 0 )
		break;
	} else {
	    d->windex += nbytes;
	    break;
	}
    }
    return TRUE;
}



/*!
    Implementation of the abstract virtual QIODevice::flush() function.
*/

void QSocket::flush()
{
    if ( !d->socket )
        return;
    bool osBufferFull = FALSE;
    int consumed = 0;
    while ( !osBufferFull && d->state >= Connecting && d->wsize > 0 ) {
#if defined(QSOCKET_DEBUG)
	qDebug( "QSocket (%s): flush: Write data to the socket", name() );
#endif
	QByteArray *a = d->wba.first();
	int nwritten;
	int i = 0;
	if ( (int)a->size() - d->windex < 1460 ) {
	    // Concatenate many smaller blocks.  the first may be
	    // partial, but each subsequent block is copied entirely
	    // or not at all.  the sizes here are picked so that we
	    // generally won't trigger nagle's algorithm in the tcp
	    // implementation: we concatenate if we'd otherwise send
	    // less than PMTU bytes (we assume PMTU is 1460 bytes),
	    // and concatenate up to the largest payload TCP/IP can
	    // carry.  with these precautions, nagle's algorithm
	    // should apply only when really appropriate.
	    QByteArray out( 65536 );
	    int j = d->windex;
	    int s = a->size() - j;
	    while ( a && i+s < (int)out.size() ) {
		memcpy( out.data()+i, a->data()+j, s );
		j = 0;
		i += s;
		a = d->wba.next();
		s = a ? a->size() : 0;
	    }
	    nwritten = d->socket->writeBlock( out.data(), i );
	    if ( d->wsn )
		d->wsn->setEnabled( FALSE ); // the QSocketNotifier documentation says so
	} else {
	    // Big block, write it immediately
	    i = a->size() - d->windex;
	    nwritten = d->socket->writeBlock( a->data() + d->windex, i );
	    if ( d->wsn )
		d->wsn->setEnabled( FALSE ); // the QSocketNotifier documentation says so
	}
	if ( nwritten > 0 ) {
	    if ( consumeWriteBuf( nwritten ) )
		consumed += nwritten;
	}
	if ( nwritten < i )
	    osBufferFull = TRUE;
    }
    if ( consumed > 0 ) {
#if defined(QSOCKET_DEBUG)
	qDebug( "QSocket (%s): flush: wrote %d bytes, %d left",
		name(), consumed, (int)d->wsize );
#endif
	emit bytesWritten( consumed );
    }
    if ( d->state == Closing && d->wsize == 0 ) {
#if defined(QSOCKET_DEBUG)
	qDebug( "QSocket (%s): flush: Delayed close done. Terminating.",
		name() );
#endif
	setFlags( IO_Sequential );
	resetStatus();
	setState( 0 );
	d->close();
	d->state = Idle;
	emit delayedCloseFinished();
	return;
    }
    if ( !d->socket->isOpen() ) {
	d->connectionClosed();
	emit connectionClosed();
	return;
    }
    if ( d->wsn )
	d->wsn->setEnabled( d->wsize > 0 ); // write if there's data
}


/*!
    Returns the number of incoming bytes that can be read right now
    (like bytesAvailable()).
*/

QIODevice::Offset QSocket::size() const
{
    return (Offset)bytesAvailable();
}


/*!
    Returns the current read index. Since QSocket is a sequential
    device, the current read index is always zero.
*/

QIODevice::Offset QSocket::at() const
{
    return 0;
}


/*!
    \overload

    Moves the read index forward to \a index and returns TRUE if the
    operation was successful; otherwise returns FALSE. Moving the
    index forward means skipping incoming data.
*/

bool QSocket::at( Offset index )
{
    if ( index > d->rba.size() )
	return FALSE;
    d->rba.consumeBytes( (Q_ULONG)index, 0 );			// throw away data 0..index-1
    // After we read data from our internal buffer, if we use the
    // setReadBufferSize() to limit our buffer, we might now be able to
    // read more data in our buffer. So enable the read socket notifier,
    // but do this only if we are not in a slot connected to the
    // readyRead() signal since this might cause a bad recursive behavior.
    // We can test for this condition by looking at the
    // sn_read_alreadyCalled flag.
    if ( d->rsn && QSocketPrivate::sn_read_alreadyCalled.findRef(this) == -1 )
	d->rsn->setEnabled( TRUE );
    return TRUE;
}


/*!
    Returns TRUE if there is no more data to read; otherwise returns FALSE.
*/

bool QSocket::atEnd() const
{
    if ( d->socket == 0 )
	return TRUE;
    QSocket * that = (QSocket *)this;
    if ( that->d->socket->bytesAvailable() )	// a little slow, perhaps...
	that->sn_read();
    return that->d->rba.size() == 0;
}


/*!
    Returns the number of incoming bytes that can be read, i.e. the
    size of the input buffer. Equivalent to size().

    This function can trigger the readyRead() signal, if more data has
    arrived on the socket.

    \sa bytesToWrite()
*/

Q_ULONG QSocket::bytesAvailable() const
{
    if ( d->socket == 0 )
	return 0;
    QSocket * that = (QSocket *)this;
    if ( that->d->socket->bytesAvailable() ) // a little slow, perhaps...
	(void)that->sn_read();
    return that->d->rba.size();
}


/*!
    Wait up to \a msecs milliseconds for more data to be available.

    If \a msecs is -1 the call will block indefinitely.

    Returns the number of bytes available.

    If \a timeout is non-null and no error occurred (i.e. it does not
    return -1): this function sets \a *timeout to TRUE, if the reason
    for returning was that the timeout was reached; otherwise it sets
    \a *timeout to FALSE. This is useful to find out if the peer
    closed the connection.

    \warning This is a blocking call and should be avoided in event
    driven applications.

    \sa bytesAvailable()
*/

Q_ULONG QSocket::waitForMore( int msecs, bool *timeout ) const
{
    if ( d->socket == 0 )
	return 0;
    QSocket * that = (QSocket *)this;
    if ( that->d->socket->waitForMore( msecs, timeout ) > 0 )
	(void)that->sn_read( TRUE );
    return that->d->rba.size();
}

/*! \overload
*/

Q_ULONG QSocket::waitForMore( int msecs ) const
{
    return waitForMore( msecs, 0 );
}

/*!
    Returns the number of bytes that are waiting to be written, i.e.
    the size of the output buffer.

    \sa bytesAvailable() clearPendingData()
*/

Q_ULONG QSocket::bytesToWrite() const
{
    return d->wsize;
}

/*!
    Deletes the data that is waiting to be written. This is useful if you want
    to close the socket without waiting for all the data to be written.

    \sa bytesToWrite() close() delayedCloseFinished()
*/

void QSocket::clearPendingData()
{
    d->wba.clear();
    d->windex = d->wsize = 0;
}

/*!
    Reads \a maxlen bytes from the socket into \a data and returns the
    number of bytes read. Returns -1 if an error occurred.
*/

Q_LONG QSocket::readBlock( char *data, Q_ULONG maxlen )
{
    if ( data == 0 && maxlen != 0 ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QSocket::readBlock: Null pointer error" );
#endif
	return -1;
    }
    if ( !isOpen() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QSocket::readBlock: Socket is not open" );
#endif
	return -1;
    }
    if ( maxlen >= d->rba.size() )
	maxlen = d->rba.size();
#if defined(QSOCKET_DEBUG)
    qDebug( "QSocket (%s): readBlock %d bytes", name(), (int)maxlen );
#endif
    d->rba.consumeBytes( maxlen, data );
    // After we read data from our internal buffer, if we use the
    // setReadBufferSize() to limit our buffer, we might now be able to
    // read more data in our buffer. So enable the read socket notifier,
    // but do this only if we are not in a slot connected to the
    // readyRead() signal since this might cause a bad recursive behavior.
    // We can test for this condition by looking at the
    // sn_read_alreadyCalled flag.
    if ( d->rsn && QSocketPrivate::sn_read_alreadyCalled.findRef(this) == -1 )
	d->rsn->setEnabled( TRUE );
    return maxlen;
}


/*!
    Writes \a len bytes to the socket from \a data and returns the
    number of bytes written. Returns -1 if an error occurred.
*/

Q_LONG QSocket::writeBlock( const char *data, Q_ULONG len )
{
#if defined(QT_CHECK_NULL)
    if ( data == 0 && len != 0 ) {
	qWarning( "QSocket::writeBlock: Null pointer error" );
    }
#endif
#if defined(QT_CHECK_STATE)
    if ( !isOpen() ) {
	qWarning( "QSocket::writeBlock: Socket is not open" );
	return -1;
    }
#endif
#if defined(QT_CHECK_STATE)
    if ( d->state == Closing ) {
	qWarning( "QSocket::writeBlock: Cannot write, socket is closing" );
    }
#endif
    if ( len == 0 || d->state == Closing || d->state == Idle )
	return 0;
    QByteArray *a = d->wba.last();

    // next bit is sensitive.  if we're writing really small chunks,
    // try to buffer up since system calls are expensive, and nagle's
    // algorithm is even more expensive.  but if anything even
    // remotely large is being written, try to issue a write at once.

    bool writeNow = ( d->wsize + len >= 1400 || len > 512 );

    if ( a && a->size() + len < 128 ) {
	// small buffer, resize
	int i = a->size();
	a->resize( i+len );
	memcpy( a->data()+i, data, len );
    } else {
	// append new buffer
	a = new QByteArray( len );
	memcpy( a->data(), data, len );
	d->wba.append( a );
    }
    d->wsize += len;
    if ( writeNow )
	flush();
    else if ( d->wsn )
	d->wsn->setEnabled( TRUE );
#if defined(QSOCKET_DEBUG)
    qDebug( "QSocket (%s): writeBlock %d bytes", name(), (int)len );
#endif
    return len;
}


/*!
    Reads a single byte/character from the internal read buffer.
    Returns the byte/character read, or -1 if there is nothing to be
    read.

    \sa bytesAvailable(), putch()
*/

int QSocket::getch()
{
    if ( isOpen() && d->rba.size() > 0 ) {
	uchar c;
	d->rba.consumeBytes( 1, (char*)&c );
	// After we read data from our internal buffer, if we use the
	// setReadBufferSize() to limit our buffer, we might now be able to
	// read more data in our buffer. So enable the read socket notifier,
	// but do this only if we are not in a slot connected to the
	// readyRead() signal since this might cause a bad recursive behavior.
	// We can test for this condition by looking at the
	// sn_read_alreadyCalled flag.
	if ( d->rsn && QSocketPrivate::sn_read_alreadyCalled.findRef(this) == -1 )
	    d->rsn->setEnabled( TRUE );
	return c;
    }
    return -1;
}


/*!
    Writes the character \a ch to the output buffer.

    Returns \a ch, or -1 if an error occurred.

    \sa getch()
*/

int QSocket::putch( int ch )
{
    char buf[2];
    buf[0] = ch;
    return writeBlock(buf, 1) == 1 ? ch : -1;
}


/*!
    This implementation of the virtual function QIODevice::ungetch()
    prepends the character \a ch to the read buffer so that the next
    read returns this character as the first character of the output.
*/

int QSocket::ungetch( int ch )
{
#if defined(QT_CHECK_STATE)
    if ( !isOpen() ) {
	qWarning( "QSocket::ungetch: Socket not open" );
	return -1;
    }
#endif
    return d->rba.ungetch( ch );
}


/*!
    Returns TRUE if it's possible to read an entire line of text from
    this socket at this time; otherwise returns FALSE.

    Note that if the peer closes the connection unexpectedly, this
    function returns FALSE. This means that loops such as this won't
    work:

    \code
	while( !socket->canReadLine() ) // WRONG
	    ;
    \endcode

    \sa readLine()
*/

bool QSocket::canReadLine() const
{
    if ( ((QSocket*)this)->d->rba.scanNewline( 0 ) )
	return TRUE;
    return ( bytesAvailable() > 0 &&
	     ((QSocket*)this)->d->rba.scanNewline( 0 ) );
}

/*!
  \reimp
  \internal
    So that it's not hidden by our other readLine().
*/
Q_LONG QSocket::readLine( char *data, Q_ULONG maxlen )
{
    return QIODevice::readLine(data,maxlen);
}

/*!
    Returns a line of text including a terminating newline character
    (\n). Returns "" if canReadLine() returns FALSE.

    \sa canReadLine()
*/

QString QSocket::readLine()
{
    QByteArray a(256);
    bool nl = d->rba.scanNewline( &a );
    QString s;
    if ( nl ) {
	at( a.size() );				// skips the data read
	s = QString( a );
    }
    return s;
}

/*!
  \internal
    Internal slot for handling socket read notifications.

    This function has can usually only be entered once (i.e. no
    recursive calls). If the argument \a force is TRUE, the function
    is executed, but no readyRead() signals are emitted. This
    behaviour is useful for the waitForMore() function, so that it is
    possible to call waitForMore() in a slot connected to the
    readyRead() signal.
*/

void QSocket::sn_read( bool force )
{
    Q_LONG maxToRead = 0;
    if ( d->readBufferSize > 0 ) {
	maxToRead = d->readBufferSize - d->rba.size();
	if ( maxToRead <= 0 ) {
	    if ( d->rsn )
		d->rsn->setEnabled( FALSE );
	    return;
	}
    }

    // Use QSocketPrivate::sn_read_alreadyCalled to avoid recursive calls of
    // sn_read() (and as a result avoid emitting the readyRead() signal in a
    // slot for readyRead(), if you use bytesAvailable()).
    if ( !force && QSocketPrivate::sn_read_alreadyCalled.findRef(this) != -1 )
	return;
    QSocketPrivate::sn_read_alreadyCalled.append( this );

    char buf[4096];
    Q_LONG nbytes = d->socket->bytesAvailable();
    Q_LONG nread;
    QByteArray *a = 0;

    if ( state() == Connecting ) {
	if ( nbytes > 0 ) {
	    tryConnection();
	} else {
	    // nothing to do, nothing to care about
	    QSocketPrivate::sn_read_alreadyCalled.removeRef( this );
	    return;
	}
    }
    if ( state() == Idle ) {
	QSocketPrivate::sn_read_alreadyCalled.removeRef( this );
	return;
    }

    if ( nbytes <= 0 ) {			// connection closed?
	// On Windows this may happen when the connection is still open.
	// This happens when the system is heavily loaded and we have
	// read all the data on the socket before a new WSAAsyncSelect
	// event is processed. A new read operation would then block.
	// This code is also useful when QSocket is used without an
	// event loop.
	nread = d->socket->readBlock( buf, maxToRead ? QMIN((Q_LONG)sizeof(buf),maxToRead) : sizeof(buf) );
	if ( nread == 0 ) {			// really closed
	    if ( !d->socket->isOpen() ) {
#if defined(QSOCKET_DEBUG)
		qDebug( "QSocket (%s): sn_read: Connection closed", name() );
#endif
		d->connectionClosed();
		emit connectionClosed();
	    }
	    QSocketPrivate::sn_read_alreadyCalled.removeRef( this );
	    return;
	} else {
	    if ( nread < 0 ) {
		if ( d->socket->error() == QSocketDevice::NoError ) {
		    // all is fine
		    QSocketPrivate::sn_read_alreadyCalled.removeRef( this );
		    return;
		}
#if defined(QSOCKET_DEBUG)
		qWarning( "QSocket::sn_read (%s): Close error", name() );
#endif
		if ( d->rsn )
		    d->rsn->setEnabled( FALSE );
		emit error( ErrSocketRead );
		QSocketPrivate::sn_read_alreadyCalled.removeRef( this );
		return;
	    }
	    a = new QByteArray( nread );
	    memcpy( a->data(), buf, nread );
	}

    } else {					// data to be read
#if defined(QSOCKET_DEBUG)
	qDebug( "QSocket (%s): sn_read: %ld incoming bytes", name(), nbytes );
#endif
	if ( nbytes > (int)sizeof(buf) ) {
	    // big
	    a = new QByteArray( nbytes );
	    nread = d->socket->readBlock( a->data(), maxToRead ? QMIN(nbytes,maxToRead) : nbytes );
	} else {
	    a = 0;
	    nread = d->socket->readBlock( buf, maxToRead ? QMIN((Q_LONG)sizeof(buf),maxToRead) : sizeof(buf) );
	    if ( nread > 0 ) {
		// ##### could setRawData
		a = new QByteArray( nread );
		memcpy( a->data(), buf, nread );
	    }
	}
	if ( nread == 0 ) {
#if defined(QSOCKET_DEBUG)
	    qDebug( "QSocket (%s): sn_read: Connection closed", name() );
#endif
	    // ### we should rather ask the socket device if it is closed
	    d->connectionClosed();
	    emit connectionClosed();
	    QSocketPrivate::sn_read_alreadyCalled.removeRef( this );
            delete a;
	    return;
	} else if ( nread < 0 ) {
            delete a;

	    if ( d->socket->error() == QSocketDevice::NoError ) {
		// all is fine
		QSocketPrivate::sn_read_alreadyCalled.removeRef( this );
		return;
	    }
#if defined(QT_CHECK_RANGE)
	    qWarning( "QSocket::sn_read: Read error" );
#endif
	    if ( d->rsn )
		d->rsn->setEnabled( FALSE );
	    emit error( ErrSocketRead );
	    QSocketPrivate::sn_read_alreadyCalled.removeRef( this );
	    return;
	}
	if ( nread != (int)a->size() ) {		// unexpected
#if defined(CHECK_RANGE) && !defined(Q_OS_WIN32)
	    qWarning( "QSocket::sn_read: Unexpected short read" );
#endif
	    a->resize( nread );
	}
    }
    d->rba.append( a );
    if ( !force ) {
	if ( d->rsn )
	    d->rsn->setEnabled( FALSE );
	emit readyRead();
	if ( d->rsn )
	    d->rsn->setEnabled( TRUE );
    }

    QSocketPrivate::sn_read_alreadyCalled.removeRef( this );
}


/*!
  \internal
    Internal slot for handling socket write notifications.
*/

void QSocket::sn_write()
{
    if ( d->state == Connecting )		// connection established?
	tryConnection();
    flush();
}

void QSocket::emitErrorConnectionRefused()
{
    emit error( ErrConnectionRefused );
}

void QSocket::tryConnection()
{
    if ( d->socket->connect( d->addr, d->port ) ) {
	d->state = Connected;
#if defined(QSOCKET_DEBUG)
	qDebug( "QSocket (%s): sn_write: Got connection to %s",
		name(), peerName().ascii() );
#endif
	if ( d->rsn )
	    d->rsn->setEnabled( TRUE );
	emit connected();
    } else {
	d->state = Idle;
	QTimer::singleShot( 0, this, SLOT(emitErrorConnectionRefused()) );
	return;
    }
}


/*!
    Returns the socket number, or -1 if there is no socket at the moment.
*/

int QSocket::socket() const
{
    if ( d->socket == 0 )
	return -1;
    return d->socket->socket();
}

/*!
    Sets the socket to use \a socket and the state() to \c Connected.
    The socket must already be connected.

    This allows us to use the QSocket class as a wrapper for other
    socket types (e.g. Unix Domain Sockets).
*/

void QSocket::setSocket( int socket )
{
    setSocketIntern( socket );
    d->state = Connection;
    d->rsn->setEnabled( TRUE );
}


/*!
    Sets the socket to \a socket. This is used by both setSocket() and
    connectToHost() and can also be used on unconnected sockets.
*/

void QSocket::setSocketIntern( int socket )
{
    if ( state() != Idle ) {
	clearPendingData();
        close();
    }
    Q_ULONG oldBufferSize = d ? d->readBufferSize : 0;
    delete d;

    d = new QSocketPrivate;
    if (oldBufferSize)
        d->readBufferSize = oldBufferSize;
    if ( socket >= 0 ) {
	QSocketDevice *sd = new QSocketDevice( socket, QSocketDevice::Stream );
	sd->setBlocking( FALSE );
	sd->setAddressReusable( TRUE );
	d->setSocketDevice( this, sd );
    }
    d->state = Idle;

    // Initialize the IO device flags
    setFlags( IO_Direct );
    resetStatus();
    open( IO_ReadWrite );

    // hm... this is not very nice.
    d->host = QString::null;
    d->port = 0;
#ifndef QT_NO_DNS
    delete d->dns4;
    d->dns4 = 0;
    delete d->dns6;
    d->dns6 = 0;
#endif
}


/*!
    Returns the host port number of this socket, in native byte order.
*/

Q_UINT16 QSocket::port() const
{
    if ( d->socket == 0 )
	return 0;
    return d->socket->port();
}


/*!
    Returns the peer's host port number, normally as specified to the
    connectToHost() function. If none has been set, this function
    returns 0.

    Note that Qt always uses native byte order, i.e. 67 is 67 in Qt;
    there is no need to call htons().
*/

Q_UINT16 QSocket::peerPort() const
{
    if ( d->socket == 0 )
	return 0;
    return d->socket->peerPort();
}


/*!
    Returns the host address of this socket. (This is normally the
    main IP address of the host, but can be e.g. 127.0.0.1 for
    connections to localhost.)
*/

QHostAddress QSocket::address() const
{
    if ( d->socket == 0 ) {
	QHostAddress tmp;
	return tmp;
    }
    return d->socket->address();
}


/*!
    Returns the address of the connected peer if the socket is in
    Connected state; otherwise an empty QHostAddress is returned.
*/

QHostAddress QSocket::peerAddress() const
{
    if ( d->socket == 0 ) {
	QHostAddress tmp;
	return tmp;
    }
    return d->socket->peerAddress();
}


/*!
    Returns the host name as specified to the connectToHost()
    function. An empty string is returned if none has been set.
*/

QString QSocket::peerName() const
{
    return d->host;
}

/*!
    Sets the size of the QSocket's internal read buffer to \a bufSize.

    Usually QSocket reads all data that is available from the operating
    system's socket. If the buffer size is limited to a certain size, this
    means that the QSocket class doesn't buffer more than this size of data.

    If the size of the read buffer is 0, the read buffer is unlimited and all
    incoming data is buffered. This is the default.

    If you read the data in the readyRead() signal, you shouldn't use this
    option since it might slow down your program unnecessary. This option is
    useful if you only need to read the data at certain points in time, like in
    a realtime streaming application.

    \sa readBufferSize()
*/

void QSocket::setReadBufferSize( Q_ULONG bufSize )
{
    d->readBufferSize = bufSize;
}

/*!
    Returns the size of the read buffer.

    \sa setReadBufferSize()
*/

Q_ULONG QSocket::readBufferSize() const
{
    return d->readBufferSize;
}

#endif //QT_NO_NETWORK
