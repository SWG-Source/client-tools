/****************************************************************************
** $Id: qsocketdevice.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QSocketDevice class.
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

#include "qsocketdevice.h"
#ifndef QT_NO_NETWORK

#include "qwindowdefs.h"
#include <string.h>


//#define QSOCKETDEVICE_DEBUG


class QSocketDevicePrivate
{
public:
    QSocketDevicePrivate( QSocketDevice::Protocol p )
	: protocol(p)
    { }

    QSocketDevice::Protocol protocol;
};


/*!
    \class QSocketDevice qsocketdevice.h
    \brief The QSocketDevice class provides a platform-independent low-level socket API.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    This class provides a low level API for working with sockets.  Users of
    this class are assumed to have networking experience. For most users the
    QSocket class provides a much easier and high level alternative, but
    certain things (like UDP) can't be done with QSocket and if you need a
    platform-independent API for those, QSocketDevice is the right choice.

    The essential purpose of the class is to provide a QIODevice that
    works on sockets, wrapped in a platform-independent API.

    When calling connect() or bind(), QSocketDevice detects the
    protocol family (IPv4, IPv6) automatically. Passing the protocol
    family to QSocketDevice's constructor or to setSocket() forces
    creation of a socket device of a specific protocol. If not set, the
    protocol will be detected at the first call to connect() or bind().

    \sa QSocket, QSocketNotifier, QHostAddress
*/


/*!
    \enum QSocketDevice::Protocol

    This enum type describes the protocol family of the socket. Possible values
    are:

    \value IPv4 The socket is an IPv4 socket.
    \value IPv6 The socket is an IPv6 socket.
    \value Unknown The protocol family of the socket is not known. This can
	   happen if you use QSocketDevice with an already existing socket; it
	   tries to determine the protocol family, but this can fail if the
	   protocol family is not known to QSocketDevice.

    \sa protocol() setSocket()
*/

/*!
    \enum QSocketDevice::Error

    This enum type describes the error states of QSocketDevice.

    \value NoError  No error has occurred.

    \value AlreadyBound  The device is already bound, according to bind().

    \value Inaccessible  The operating system or firewall prohibited
			the action.

    \value NoResources  The operating system ran out of a resource.

    \value InternalError  An internal error occurred in QSocketDevice.

    \value Impossible  An attempt was made to do something which makes
    no sense. For example:
    \code
    ::close( sd->socket() );
    sd->writeBlock( someData, 42 );
    \endcode
    The libc ::close() closes the socket, but QSocketDevice is not aware
    of this. So when you call writeBlock(), the impossible happens.

    \value NoFiles  The operating system will not let QSocketDevice open
    another file.

    \value ConnectionRefused  A connection attempt was rejected by the
    peer.

    \value NetworkFailure  There is a network failure.

    \value UnknownError  The operating system did something
    unexpected.
*/

/*!
    \enum QSocketDevice::Type

    This enum type describes the type of the socket:
    \value Stream  a stream socket (TCP, usually)
    \value Datagram  a datagram socket (UDP, usually)
*/


/*!
    Creates a QSocketDevice object for the existing socket \a socket.

    The \a type argument must match the actual socket type; use \c
    QSocketDevice::Stream for a reliable, connection-oriented TCP
    socket, or \c QSocketDevice::Datagram for an unreliable,
    connectionless UDP socket.
*/
QSocketDevice::QSocketDevice( int socket, Type type )
    : fd( socket ), t( type ), p( 0 ), pp( 0 ), e( NoError ),
      d(new QSocketDevicePrivate(Unknown))
{
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice: Created QSocketDevice %p (socket %x, type %d)",
	   this, socket, type );
#endif
    init();
    setSocket( socket, type );
}

/*!
    Creates a QSocketDevice object for a stream or datagram socket.

    The \a type argument must be either \c QSocketDevice::Stream for a
    reliable, connection-oriented TCP socket, or \c
    QSocketDevice::Datagram for an unreliable UDP socket.

    The socket is created as an IPv4 socket.

    \sa blocking() protocol()
*/
QSocketDevice::QSocketDevice( Type type )
    : fd( -1 ), t( type ), p( 0 ), pp( 0 ), e( NoError ),
      d(new QSocketDevicePrivate(IPv4))
{
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice: Created QSocketDevice object %p, type %d",
	    this, type );
#endif
    init();
    setSocket( createNewSocket(), type );
}

/*!
    Creates a QSocketDevice object for a stream or datagram socket.

    The \a type argument must be either \c QSocketDevice::Stream for a
    reliable, connection-oriented TCP socket, or \c
    QSocketDevice::Datagram for an unreliable UDP socket.

    The \a protocol indicates whether the socket should be of type IPv4
    or IPv6. Passing \c Unknown is not meaningful in this context and you
    should avoid using (it creates an IPv4 socket, but your code is not easily
    readable).

    The argument \a dummy is necessary for compatibility with some
    compilers.

    \sa blocking() protocol()
*/
QSocketDevice::QSocketDevice( Type type, Protocol protocol, int )
    : fd( -1 ), t( type ), p( 0 ), pp( 0 ), e( NoError ),
      d(new QSocketDevicePrivate(protocol))
{
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice: Created QSocketDevice object %p, type %d",
	    this, type );
#endif
    init();
    setSocket( createNewSocket(), type );
}

/*!
    Destroys the socket device and closes the socket if it is open.
*/
QSocketDevice::~QSocketDevice()
{
    close();
    delete d;
    d = 0;
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice: Destroyed QSocketDevice %p", this );
#endif
}


/*!
    Returns TRUE if this is a valid socket; otherwise returns FALSE.

    \sa socket()
*/
bool QSocketDevice::isValid() const
{
    return fd != -1;
}


/*!
    \fn Type QSocketDevice::type() const

    Returns the socket type which is either \c QSocketDevice::Stream
    or \c QSocketDevice::Datagram.

    \sa socket()
*/
QSocketDevice::Type QSocketDevice::type() const
{
    return t;
}

/*!
    Returns the socket's protocol family, which is one of \c Unknown, \c IPv4,
    or \c IPv6.

    QSocketDevice either creates a socket with a well known protocol family or
    it uses an already existing socket. In the first case, this function
    returns the protocol family it was constructed with. In the second case, it
    tries to determine the protocol family of the socket; if this fails, it
    returns \c Unknown.

    \sa Protocol setSocket()
*/
QSocketDevice::Protocol QSocketDevice::protocol() const
{
    if ( d->protocol == Unknown )
	d->protocol = getProtocol();
    return d->protocol;
}

/*!
    Returns the socket number, or -1 if it is an invalid socket.

    \sa isValid(), type()
*/
int QSocketDevice::socket() const
{
    return fd;
}


/*!
    Sets the socket device to operate on the existing socket \a
    socket.

    The \a type argument must match the actual socket type; use \c
    QSocketDevice::Stream for a reliable, connection-oriented TCP
    socket, or \c QSocketDevice::Datagram for an unreliable,
    connectionless UDP socket.

    Any existing socket is closed.

    \sa isValid(), close()
*/
void QSocketDevice::setSocket( int socket, Type type )
{
    if ( fd != -1 )			// close any open socket
	close();
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice::setSocket: socket %x, type %d", socket, type );
#endif
    t = type;
    fd = socket;
    d->protocol = Unknown;
    e = NoError;
    setFlags( IO_Sequential );
    resetStatus();
    open( IO_ReadWrite );
    fetchConnectionParameters();
}


/*!
    \reimp

    Opens the socket using the specified QIODevice file \a mode. This
    function is called from the QSocketDevice constructors and from
    the setSocket() function. You should not call it yourself.

    \sa close().
*/
bool QSocketDevice::open( int mode )
{
    if ( isOpen() || !isValid() )
	return FALSE;
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice::open: mode %x", mode );
#endif
    setMode( mode & IO_ReadWrite );
    setState( IO_Open );
    return TRUE;
}


/*!
    \reimp

    The current QSocketDevice implementation does not buffer at all,
    so this is a no-op.
*/
void QSocketDevice::flush()
{
}


/*!
    \reimp

    The size is meaningless for a socket, therefore this function returns 0.
*/
QIODevice::Offset QSocketDevice::size() const
{
    return 0;
}


/*!
    \reimp

    The read/write index is meaningless for a socket, therefore this
    function returns 0.
*/
QIODevice::Offset QSocketDevice::at() const
{
    return 0;
}


/*!
    \reimp

    The read/write index is meaningless for a socket, therefore this
    function does nothing and returns TRUE.
*/
bool QSocketDevice::at( Offset )
{
    return TRUE;
}


/*!
    \reimp

    Returns TRUE if no data is currently available at the socket;
    otherwise returns FALSE.
*/
bool QSocketDevice::atEnd() const
{
    return bytesAvailable() <= 0;
}


/*!
    \reimp

    \warning getch() is implemented as a one-byte readBlock(), so it
    may be very slow if you call it more than a few times.

    \sa putch() readBlock()
*/
int QSocketDevice::getch()
{
    char buf[2];
    return  readBlock(buf,1) == 1 ? buf[0] : -1;
}


/*!
    \reimp

    \warning putch() is implemented as a one-byte writeBlock(), so it
    may be very slow if you call it more than a few times.

    \sa getch()
*/
int QSocketDevice::putch( int ch )
{
    char buf[2];
    buf[0] = ch;
    return writeBlock(buf, 1) == 1 ? ch : -1;
}


/*!
    \reimp

    This implementation of ungetch returns -1 (error). A socket is a
    sequential device and does not allow any ungetch operation.
*/
int QSocketDevice::ungetch( int )
{
    return -1;
}


/*!
    Returns TRUE if the address of this socket can be used by other
    sockets at the same time, and FALSE if this socket claims
    exclusive ownership.

    \sa setAddressReusable()
*/
bool QSocketDevice::addressReusable() const
{
    return option( ReuseAddress );
}


/*!
    Sets the address of this socket to be usable by other sockets too
    if \a enable is TRUE, and to be used exclusively by this socket if
    \a enable is FALSE.

    When a socket is reusable, other sockets can use the same port
    number (and IP address), which is generally useful. Of course
    other sockets cannot use the same
    (address,port,peer-address,peer-port) 4-tuple as this socket, so
    there is no risk of confusing the two TCP connections.

    \sa addressReusable()
*/
void QSocketDevice::setAddressReusable( bool enable )
{
    setOption( ReuseAddress, enable );
}


/*!
    Returns the size of the operating system receive buffer.

    \sa setReceiveBufferSize()
*/
int QSocketDevice::receiveBufferSize() const
{
    return option( ReceiveBuffer );
}


/*!
    Sets the size of the operating system receive buffer to \a size.

    The operating system receive buffer size effectively limits two
    things: how much data can be in transit at any one moment, and how
    much data can be received in one iteration of the main event loop.

    The default is operating system-dependent. A socket that receives
    large amounts of data is probably best with a buffer size of
    49152.
*/
void QSocketDevice::setReceiveBufferSize( uint size )
{
    setOption( ReceiveBuffer, size );
}


/*!
    Returns the size of the operating system send buffer.

    \sa setSendBufferSize()
*/
int QSocketDevice::sendBufferSize() const
{
    return option( SendBuffer );
}


/*!
    Sets the size of the operating system send buffer to \a size.

    The operating system send buffer size effectively limits how much
    data can be in transit at any one moment.

    The default is operating system-dependent. A socket that sends
    large amounts of data is probably best with a buffer size of
    49152.
*/
void QSocketDevice::setSendBufferSize( uint size )
{
    setOption( SendBuffer, size );
}


/*!
    Returns the port number of this socket device. This may be 0 for a
    while, but is set to something sensible as soon as a sensible
    value is available.

    Note that Qt always uses native byte order, i.e. 67 is 67 in Qt;
    there is no need to call htons().
*/
Q_UINT16 QSocketDevice::port() const
{
    return p;
}


/*!
    Returns the address of this socket device. This may be 0.0.0.0 for
    a while, but is set to something sensible as soon as a sensible
    value is available.
*/
QHostAddress QSocketDevice::address() const
{
    return a;
}


/*!
    Returns the first error seen.
*/
QSocketDevice::Error QSocketDevice::error() const
{
    return e;
}


/*!
    Allows subclasses to set the error state to \a err.
*/
void QSocketDevice::setError( Error err )
{
    e = err;
}
#endif //QT_NO_NETWORK

