/****************************************************************************
** $Id: qhttp.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QHttp and related classes.
**
** Created : 970521
**
** Copyright (C) 1997-2007 Trolltech ASA.  All rights reserved.
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

#include "qhttp.h"

#ifndef QT_NO_NETWORKPROTOCOL_HTTP

#include "qsocket.h"
#include "qtextstream.h"
#include "qmap.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qcstring.h"
#include "qbuffer.h"
#include "qurloperator.h"
#include "qtimer.h"
#include "private/qinternal_p.h"

//#define QHTTP_DEBUG

class QHttpPrivate
{
public:
    QHttpPrivate() :
	state( QHttp::Unconnected ),
	error( QHttp::NoError ),
	hostname( QString::null ),
	port( 0 ),
	toDevice( 0 ),
	postDevice( 0 ),
	bytesDone( 0 ),
	chunkedSize( -1 ),
	idleTimer( 0 )
    {
	pending.setAutoDelete( TRUE );
    }

    QSocket socket;
    QPtrList<QHttpRequest> pending;

    QHttp::State state;
    QHttp::Error error;
    QString errorString;

    QString hostname;
    Q_UINT16 port;

    QByteArray buffer;
    QIODevice* toDevice;
    QIODevice* postDevice;

    uint bytesDone;
    uint bytesTotal;
    Q_LONG chunkedSize;

    QHttpRequestHeader header;

    bool readHeader;
    QString headerStr;
    QHttpResponseHeader response;

    int idleTimer;

    QMembuf rba;
};

class QHttpRequest
{
public:
    QHttpRequest()
    {
	id = ++idCounter;
    }
    virtual ~QHttpRequest()
    { }

    virtual void start( QHttp * ) = 0;
    virtual bool hasRequestHeader();
    virtual QHttpRequestHeader requestHeader();

    virtual QIODevice* sourceDevice() = 0;
    virtual QIODevice* destinationDevice() = 0;

    int id;

private:
    static int idCounter;
};

int QHttpRequest::idCounter = 0;

bool QHttpRequest::hasRequestHeader()
{
    return FALSE;
}

QHttpRequestHeader QHttpRequest::requestHeader()
{
    return QHttpRequestHeader();
}

/****************************************************
 *
 * QHttpNormalRequest
 *
 ****************************************************/

class QHttpNormalRequest : public QHttpRequest
{
public:
    QHttpNormalRequest( const QHttpRequestHeader &h, QIODevice *d, QIODevice *t ) :
	header(h), to(t)
    {
	is_ba = FALSE;
	data.dev = d;
    }

    QHttpNormalRequest( const QHttpRequestHeader &h, QByteArray *d, QIODevice *t ) :
	header(h), to(t)
    {
	is_ba = TRUE;
	data.ba = d;
    }

    ~QHttpNormalRequest()
    {
	if ( is_ba )
	    delete data.ba;
    }

    void start( QHttp * );
    bool hasRequestHeader();
    QHttpRequestHeader requestHeader();

    QIODevice* sourceDevice();
    QIODevice* destinationDevice();

protected:
    QHttpRequestHeader header;

private:
    union {
	QByteArray *ba;
	QIODevice *dev;
    } data;
    bool is_ba;
    QIODevice *to;
};

void QHttpNormalRequest::start( QHttp *http )
{
    http->d->header = header;

    if ( is_ba ) {
	http->d->buffer = *data.ba;
	if ( http->d->buffer.size() > 0 )
	    http->d->header.setContentLength( http->d->buffer.size() );

	http->d->postDevice = 0;
    } else {
	http->d->buffer = QByteArray();

	if ( data.dev && ( data.dev->isOpen() || data.dev->open(IO_ReadOnly) ) ) {
	    http->d->postDevice = data.dev;
	    if ( http->d->postDevice->size() > 0 )
		http->d->header.setContentLength( http->d->postDevice->size() );
	} else {
	    http->d->postDevice = 0;
	}
    }

    if ( to && ( to->isOpen() || to->open(IO_WriteOnly) ) )
	http->d->toDevice = to;
    else
	http->d->toDevice = 0;

    http->sendRequest();
}

bool QHttpNormalRequest::hasRequestHeader()
{
    return TRUE;
}

QHttpRequestHeader QHttpNormalRequest::requestHeader()
{
    return header;
}

QIODevice* QHttpNormalRequest::sourceDevice()
{
    if ( is_ba )
	return 0;
    return data.dev;
}

QIODevice* QHttpNormalRequest::destinationDevice()
{
    return to;
}

/****************************************************
 *
 * QHttpPGHRequest
 * (like a QHttpNormalRequest, but for the convenience
 * functions put(), get() and head() -- i.e. set the
 * host header field correctly before sending the
 * request)
 *
 ****************************************************/

class QHttpPGHRequest : public QHttpNormalRequest
{
public:
    QHttpPGHRequest( const QHttpRequestHeader &h, QIODevice *d, QIODevice *t ) :
	QHttpNormalRequest( h, d, t )
    { }

    QHttpPGHRequest( const QHttpRequestHeader &h, QByteArray *d, QIODevice *t ) :
	QHttpNormalRequest( h, d, t )
    { }

    ~QHttpPGHRequest()
    { }

    void start( QHttp * );
};

void QHttpPGHRequest::start( QHttp *http )
{
    header.setValue( "Host", http->d->hostname );
    QHttpNormalRequest::start( http );
}

/****************************************************
 *
 * QHttpSetHostRequest
 *
 ****************************************************/

class QHttpSetHostRequest : public QHttpRequest
{
public:
    QHttpSetHostRequest( const QString &h, Q_UINT16 p ) :
	hostname(h), port(p)
    { }

    void start( QHttp * );

    QIODevice* sourceDevice()
    { return 0; }
    QIODevice* destinationDevice()
    { return 0; }

private:
    QString hostname;
    Q_UINT16 port;
};

void QHttpSetHostRequest::start( QHttp *http )
{
    http->d->hostname = hostname;
    http->d->port = port;
    http->finishedWithSuccess();
}

/****************************************************
 *
 * QHttpCloseRequest
 *
 ****************************************************/

class QHttpCloseRequest : public QHttpRequest
{
public:
    QHttpCloseRequest()
    { }
    void start( QHttp * );

    QIODevice* sourceDevice()
    { return 0; }
    QIODevice* destinationDevice()
    { return 0; }
};

void QHttpCloseRequest::start( QHttp *http )
{
    http->close();
}

/****************************************************
 *
 * QHttpHeader
 *
 ****************************************************/

/*!
    \class QHttpHeader qhttp.h
    \brief The QHttpHeader class contains header information for HTTP.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    In most cases you should use the more specialized derivatives of
    this class, QHttpResponseHeader and QHttpRequestHeader, rather
    than directly using QHttpHeader.

    QHttpHeader provides the HTTP header fields. A HTTP header field
    consists of a name followed by a colon, a single space, and the
    field value. (See RFC 1945.) Field names are case-insensitive. A
    typical header field looks like this:
    \code
    content-type: text/html
    \endcode

    In the API the header field name is called the "key" and the
    content is called the "value". You can get and set a header
    field's value by using its key with value() and setValue(), e.g.
    \code
    header.setValue( "content-type", "text/html" );
    QString contentType = header.value( "content-type" );
    \endcode

    Some fields are so common that getters and setters are provided
    for them as a convenient alternative to using \l value() and
    \l setValue(), e.g. contentLength() and contentType(),
    setContentLength() and setContentType().

    Each header key has a \e single value associated with it. If you
    set the value for a key which already exists the previous value
    will be discarded.

    \sa QHttpRequestHeader QHttpResponseHeader
*/

/*!
    \fn int QHttpHeader::majorVersion() const

    Returns the major protocol-version of the HTTP header.
*/

/*!
    \fn int QHttpHeader::minorVersion() const

    Returns the minor protocol-version of the HTTP header.
*/

/*!
	Constructs an empty HTTP header.
*/
QHttpHeader::QHttpHeader()
    : valid( TRUE )
{
}

/*!
	Constructs a copy of \a header.
*/
QHttpHeader::QHttpHeader( const QHttpHeader& header )
    : valid( header.valid )
{
    values = header.values;
}

/*!
    Constructs a HTTP header for \a str.

    This constructor parses the string \a str for header fields and
    adds this information. The \a str should consist of one or more
    "\r\n" delimited lines; each of these lines should have the format
    key, colon, space, value.
*/
QHttpHeader::QHttpHeader( const QString& str )
    : valid( TRUE )
{
    parse( str );
}

/*!
    Destructor.
*/
QHttpHeader::~QHttpHeader()
{
}

/*!
    Assigns \a h and returns a reference to this http header.
*/
QHttpHeader& QHttpHeader::operator=( const QHttpHeader& h )
{
    values = h.values;
    valid = h.valid;
    return *this;
}

/*!
    Returns TRUE if the HTTP header is valid; otherwise returns FALSE.

    A QHttpHeader is invalid if it was created by parsing a malformed string.
*/
bool QHttpHeader::isValid() const
{
    return valid;
}

/*! \internal
    Parses the HTTP header string \a str for header fields and adds
    the keys/values it finds. If the string is not parsed successfully
    the QHttpHeader becomes \link isValid() invalid\endlink.

    Returns TRUE if \a str was successfully parsed; otherwise returns FALSE.

    \sa toString()
*/
bool QHttpHeader::parse( const QString& str )
{
    QStringList lst;
    int pos = str.find( '\n' );
    if ( pos > 0 && str.at( pos - 1 ) == '\r' )
	lst = QStringList::split( "\r\n", str.stripWhiteSpace(), FALSE );
    else
	lst = QStringList::split( "\n", str.stripWhiteSpace(), FALSE );

    if ( lst.isEmpty() )
	return TRUE;

    QStringList lines;
    QStringList::Iterator it = lst.begin();
    for( ; it != lst.end(); ++it ) {
	if ( !(*it).isEmpty() ) {
	    if ( (*it)[0].isSpace() ) {
		if ( !lines.isEmpty() ) {
		    lines.last() += " ";
		    lines.last() += (*it).stripWhiteSpace();
		}
	    } else {
		lines.append( (*it) );
	    }
	}
    }

    int number = 0;
    it = lines.begin();
    for( ; it != lines.end(); ++it ) {
	if ( !parseLine( *it, number++ ) ) {
	    valid = FALSE;
	    return FALSE;
	}
    }
    return TRUE;
}

/*! \internal
*/
void QHttpHeader::setValid( bool v )
{
    valid = v;
}

/*!
    Returns the value for the entry with the given \a key. If no entry
    has this \a key, an empty string is returned.

    \sa setValue() removeValue() hasKey() keys()
*/
QString QHttpHeader::value( const QString& key ) const
{
    return values[ key.lower() ];
}

/*!
    Returns a list of the keys in the HTTP header.

    \sa hasKey()
*/
QStringList QHttpHeader::keys() const
{
    return values.keys();
}

/*!
    Returns TRUE if the HTTP header has an entry with the given \a
    key; otherwise returns FALSE.

    \sa value() setValue() keys()
*/
bool QHttpHeader::hasKey( const QString& key ) const
{
    return values.contains( key.lower() );
}

/*!
    Sets the value of the entry with the \a key to \a value.

    If no entry with \a key exists, a new entry with the given \a key
    and \a value is created. If an entry with the \a key already
    exists, its value is discarded and replaced with the given \a
    value.

    \sa value() hasKey() removeValue()
*/
void QHttpHeader::setValue( const QString& key, const QString& value )
{
    values[ key.lower() ] = value;
}

/*!
    Removes the entry with the key \a key from the HTTP header.

    \sa value() setValue()
*/
void QHttpHeader::removeValue( const QString& key )
{
    values.remove( key.lower() );
}

/*! \internal
    Parses the single HTTP header line \a line which has the format
    key, colon, space, value, and adds key/value to the headers. The
    linenumber is \a number. Returns TRUE if the line was successfully
    parsed and the key/value added; otherwise returns FALSE.

    \sa parse()
*/
bool QHttpHeader::parseLine( const QString& line, int )
{
    int i = line.find( ":" );
    if ( i == -1 )
	return FALSE;

    values.insert( line.left( i ).stripWhiteSpace().lower(), line.mid( i + 1 ).stripWhiteSpace() );

    return TRUE;
}

/*!
    Returns a string representation of the HTTP header.

    The string is suitable for use by the constructor that takes a
    QString. It consists of lines with the format: key, colon, space,
    value, "\r\n".
*/
QString QHttpHeader::toString() const
{
    if ( !isValid() )
	return "";

    QString ret = "";

    QMap<QString,QString>::ConstIterator it = values.begin();
    for( ; it != values.end(); ++it )
	ret += it.key() + ": " + it.data() + "\r\n";

    return ret;
}

/*!
    Returns TRUE if the header has an entry for the special HTTP
    header field \c content-length; otherwise returns FALSE.

    \sa contentLength() setContentLength()
*/
bool QHttpHeader::hasContentLength() const
{
    return hasKey( "content-length" );
}

/*!
    Returns the value of the special HTTP header field \c
    content-length.

    \sa setContentLength() hasContentLength()
*/
uint QHttpHeader::contentLength() const
{
    return values[ "content-length" ].toUInt();
}

/*!
    Sets the value of the special HTTP header field \c content-length
    to \a len.

    \sa contentLength() hasContentLength()
*/
void QHttpHeader::setContentLength( int len )
{
    values[ "content-length" ] = QString::number( len );
}

/*!
    Returns TRUE if the header has an entry for the the special HTTP
    header field \c content-type; otherwise returns FALSE.

    \sa contentType() setContentType()
*/
bool QHttpHeader::hasContentType() const
{
    return hasKey( "content-type" );
}

/*!
    Returns the value of the special HTTP header field \c content-type.

    \sa setContentType() hasContentType()
*/
QString QHttpHeader::contentType() const
{
    QString type = values[ "content-type" ];
    if ( type.isEmpty() )
	return QString::null;

    int pos = type.find( ";" );
    if ( pos == -1 )
	return type;

    return type.left( pos ).stripWhiteSpace();
}

/*!
    Sets the value of the special HTTP header field \c content-type to
    \a type.

    \sa contentType() hasContentType()
*/
void QHttpHeader::setContentType( const QString& type )
{
    values[ "content-type" ] = type;
}

/****************************************************
 *
 * QHttpResponseHeader
 *
 ****************************************************/

/*!
    \class QHttpResponseHeader qhttp.h
    \brief The QHttpResponseHeader class contains response header information for HTTP.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    This class is used by the QHttp class to report the header
    information that the client received from the server.

    HTTP responses have a status code that indicates the status of the
    response. This code is a 3-digit integer result code (for details
    see to RFC 1945). In addition to the status code, you can also
    specify a human-readable text that describes the reason for the
    code ("reason phrase"). This class allows you to get the status
    code and the reason phrase.

    \sa QHttpRequestHeader QHttp
*/

/*!
    Constructs an empty HTTP response header.
*/
QHttpResponseHeader::QHttpResponseHeader()
{
    setValid( FALSE );
}

/*!
    Constructs a HTTP response header with the status code \a code,
    the reason phrase \a text and the protocol-version \a majorVer and
    \a minorVer.
*/
QHttpResponseHeader::QHttpResponseHeader( int code, const QString& text, int majorVer, int minorVer )
    : QHttpHeader(), statCode( code ), reasonPhr( text ), majVer( majorVer ), minVer( minorVer )
{
}

/*!
    Constructs a copy of \a header.
*/
QHttpResponseHeader::QHttpResponseHeader( const QHttpResponseHeader& header )
    : QHttpHeader( header ), statCode( header.statCode ), reasonPhr( header.reasonPhr ), majVer( header.majVer ), minVer( header.minVer )
{
}

/*!
    Constructs a HTTP response header from the string \a str. The
    string is parsed and the information is set. The \a str should
    consist of one or more "\r\n" delimited lines; the first line should be the
    status-line (format: HTTP-version, space, status-code, space,
    reason-phrase); each of remaining lines should have the format key, colon,
    space, value.
*/
QHttpResponseHeader::QHttpResponseHeader( const QString& str )
    : QHttpHeader()
{
    parse( str );
}

/*!
    Sets the status code to \a code, the reason phrase to \a text and
    the protocol-version to \a majorVer and \a minorVer.

    \sa statusCode() reasonPhrase() majorVersion() minorVersion()
*/
void QHttpResponseHeader::setStatusLine( int code, const QString& text, int majorVer, int minorVer )
{
    setValid( TRUE );
    statCode = code;
    reasonPhr = text;
    majVer = majorVer;
    minVer = minorVer;
}

/*!
    Returns the status code of the HTTP response header.

    \sa reasonPhrase() majorVersion() minorVersion()
*/
int QHttpResponseHeader::statusCode() const
{
    return statCode;
}

/*!
    Returns the reason phrase of the HTTP response header.

    \sa statusCode() majorVersion() minorVersion()
*/
QString QHttpResponseHeader::reasonPhrase() const
{
    return reasonPhr;
}

/*!
    Returns the major protocol-version of the HTTP response header.

    \sa minorVersion() statusCode() reasonPhrase()
*/
int QHttpResponseHeader::majorVersion() const
{
    return majVer;
}

/*!
    Returns the minor protocol-version of the HTTP response header.

    \sa majorVersion() statusCode() reasonPhrase()
*/
int QHttpResponseHeader::minorVersion() const
{
    return minVer;
}

/*! \reimp
*/
bool QHttpResponseHeader::parseLine( const QString& line, int number )
{
    if ( number != 0 )
	return QHttpHeader::parseLine( line, number );

    QString l = line.simplifyWhiteSpace();
    if ( l.length() < 10 )
	return FALSE;

    if ( l.left( 5 ) == "HTTP/" && l[5].isDigit() && l[6] == '.' &&
	    l[7].isDigit() && l[8] == ' ' && l[9].isDigit() ) {
	majVer = l[5].latin1() - '0';
	minVer = l[7].latin1() - '0';

	int pos = l.find( ' ', 9 );
	if ( pos != -1 ) {
	    reasonPhr = l.mid( pos + 1 );
	    statCode = l.mid( 9, pos - 9 ).toInt();
	} else {
	    statCode = l.mid( 9 ).toInt();
	    reasonPhr = QString::null;
	}
    } else {
	return FALSE;
    }

    return TRUE;
}

/*! \reimp
*/
QString QHttpResponseHeader::toString() const
{
    QString ret( "HTTP/%1.%2 %3 %4\r\n%5\r\n" );
    return ret.arg( majVer ).arg ( minVer ).arg( statCode ).arg( reasonPhr ).arg( QHttpHeader::toString() );
}

/****************************************************
 *
 * QHttpRequestHeader
 *
 ****************************************************/

/*!
    \class QHttpRequestHeader qhttp.h
    \brief The QHttpRequestHeader class contains request header information for
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif
    HTTP.
\if defined(commercial_edition)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    This class is used in the QHttp class to report the header
    information if the client requests something from the server.

    HTTP requests have a method which describes the request's action.
    The most common requests are "GET" and "POST". In addition to the
    request method the header also includes a request-URI to specify
    the location for the method to use.

    The method, request-URI and protocol-version can be set using a
    constructor or later using setRequest(). The values can be
    obtained using method(), path(), majorVersion() and
    minorVersion().

    This class is a QHttpHeader subclass so that class's functions,
    e.g. \link QHttpHeader::setValue() setValue()\endlink, \link
    QHttpHeader::value() value()\endlink, etc. are also available.

    \sa QHttpResponseHeader QHttp

    \important value() setValue()
*/

/*!
    Constructs an empty HTTP request header.
*/
QHttpRequestHeader::QHttpRequestHeader()
    : QHttpHeader()
{
    setValid( FALSE );
}

/*!
    Constructs a HTTP request header for the method \a method, the
    request-URI \a path and the protocol-version \a majorVer and \a minorVer.
*/
QHttpRequestHeader::QHttpRequestHeader( const QString& method, const QString& path, int majorVer, int minorVer )
    : QHttpHeader(), m( method ), p( path ), majVer( majorVer ), minVer( minorVer )
{
}

/*!
    Constructs a copy of \a header.
*/
QHttpRequestHeader::QHttpRequestHeader( const QHttpRequestHeader& header )
    : QHttpHeader( header ), m( header.m ), p( header.p ), majVer( header.majVer ), minVer( header.minVer )
{
}

/*!
    Constructs a HTTP request header from the string \a str. The \a
    str should consist of one or more "\r\n" delimited lines; the first line
    should be the request-line (format: method, space, request-URI, space
    HTTP-version); each of the remaining lines should have the format key,
    colon, space, value.
*/
QHttpRequestHeader::QHttpRequestHeader( const QString& str )
    : QHttpHeader()
{
    parse( str );
}

/*!
    This function sets the request method to \a method, the
    request-URI to \a path and the protocol-version to \a majorVer and
    \a minorVer.

    \sa method() path() majorVersion() minorVersion()
*/
void QHttpRequestHeader::setRequest( const QString& method, const QString& path, int majorVer, int minorVer )
{
    setValid( TRUE );
    m = method;
    p = path;
    majVer = majorVer;
    minVer = minorVer;
}

/*!
    Returns the method of the HTTP request header.

    \sa path() majorVersion() minorVersion() setRequest()
*/
QString QHttpRequestHeader::method() const
{
    return m;
}

/*!
    Returns the request-URI of the HTTP request header.

    \sa method() majorVersion() minorVersion() setRequest()
*/
QString QHttpRequestHeader::path() const
{
    return p;
}

/*!
    Returns the major protocol-version of the HTTP request header.

    \sa minorVersion() method() path() setRequest()
*/
int QHttpRequestHeader::majorVersion() const
{
    return majVer;
}

/*!
    Returns the minor protocol-version of the HTTP request header.

    \sa majorVersion() method() path() setRequest()
*/
int QHttpRequestHeader::minorVersion() const
{
    return minVer;
}

/*! \reimp
*/
bool QHttpRequestHeader::parseLine( const QString& line, int number )
{
    if ( number != 0 )
	return QHttpHeader::parseLine( line, number );

    QStringList lst = QStringList::split( " ", line.simplifyWhiteSpace() );
    if ( lst.count() > 0 ) {
	m = lst[0];
	if ( lst.count() > 1 ) {
	    p = lst[1];
	    if ( lst.count() > 2 ) {
		QString v = lst[2];
		if ( v.length() >= 8 && v.left( 5 ) == "HTTP/" &&
			v[5].isDigit() && v[6] == '.' && v[7].isDigit() ) {
		    majVer = v[5].latin1() - '0';
		    minVer = v[7].latin1() - '0';
		    return TRUE;
		}
	    }
	}
    }

    return FALSE;
}

/*! \reimp
*/
QString QHttpRequestHeader::toString() const
{
    QString first( "%1 %2");
    QString last(" HTTP/%3.%4\r\n%5\r\n" );
    return first.arg( m ).arg( p ) +
	last.arg( majVer ).arg( minVer ).arg( QHttpHeader::toString());
}


/****************************************************
 *
 * QHttp
 *
 ****************************************************/
/*!
    \class QHttp qhttp.h
    \brief The QHttp class provides an implementation of the HTTP protocol.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    This class provides two different interfaces: one is the
    QNetworkProtocol interface that allows you to use HTTP through the
    QUrlOperator abstraction. The other is a direct interface to HTTP
    that allows you to have more control over the requests and that
    allows you to access the response header fields.

    Don't mix the two interfaces, since the behavior is not
    well-defined.

    If you want to use QHttp with the QNetworkProtocol interface, you
    do not use it directly, but rather through a QUrlOperator, for
    example:

    \code
    QUrlOperator op( "http://www.trolltech.com" );
    op.get( "index.html" );
    \endcode

    This code will only work if the QHttp class is registered; to
    register the class, you must call qInitNetworkProtocols() before
    using a QUrlOperator with HTTP.

    The QNetworkProtocol interface for HTTP only supports the
    operations operationGet() and operationPut(), i.e.
    QUrlOperator::get() and QUrlOperator::put(), if you use it with a
    QUrlOperator.

    The rest of this descrption describes the direct interface to
    HTTP.

    The class works asynchronously, so there are no blocking
    functions. If an operation cannot be executed immediately, the
    function will still return straight away and the operation will be
    scheduled for later execution. The results of scheduled operations
    are reported via signals. This approach depends on the event loop
    being in operation.

    The operations that can be scheduled (they are called "requests"
    in the rest of the documentation) are the following: setHost(),
    get(), post(), head() and request().

    All of these requests return a unique identifier that allows you
    to keep track of the request that is currently executed. When the
    execution of a request starts, the requestStarted() signal with
    the identifier is emitted and when the request is finished, the
    requestFinished() signal is emitted with the identifier and a bool
    that indicates if the request finished with an error.

    To make an HTTP request you must set up suitable HTTP headers. The
    following example demonstrates, how to request the main HTML page
    from the Trolltech home page (i.e. the URL
    http://www.trolltech.com/index.html):

    \code
    QHttpRequestHeader header( "GET", "/index.html" );
    header.setValue( "Host", "www.trolltech.com" );
    http->setHost( "www.trolltech.com" );
    http->request( header );
    \endcode

    For the common HTTP requests \c GET, \c POST and \c HEAD, QHttp
    provides the convenience functions get(), post() and head(). They
    already use a reasonable header and if you don't have to set
    special header fields, they are easier to use. The above example
    can also be written as:

    \code
    http->setHost( "www.trolltech.com" ); // id == 1
    http->get( "/index.html" );           // id == 2
    \endcode

    For this example the following sequence of signals is emitted
    (with small variations, depending on network traffic, etc.):

    \code
    requestStarted( 1 )
    requestFinished( 1, FALSE )

    requestStarted( 2 )
    stateChanged( Connecting )
    stateChanged( Sending )
    dataSendProgress( 77, 77 )
    stateChanged( Reading )
    responseHeaderReceived( responseheader )
    dataReadProgress( 5388, 0 )
    readyRead( responseheader )
    dataReadProgress( 18300, 0 )
    readyRead( responseheader )
    stateChanged( Connected )
    requestFinished( 2, FALSE )

    done( FALSE )

    stateChanged( Closing )
    stateChanged( Unconnected )
    \endcode

    The dataSendProgress() and dataReadProgress() signals in the above
    example are useful if you want to show a \link QProgressBar
    progressbar\endlink to inform the user about the progress of the
    download. The second argument is the total size of data. In
    certain cases it is not possible to know the total amount in
    advance, in which case the second argument is 0. (If you connect
    to a QProgressBar a total of 0 results in a busy indicator.)

    When the response header is read, it is reported with the
    responseHeaderReceived() signal.

    The readyRead() signal tells you that there is data ready to be
    read. The amount of data can then be queried with the
    bytesAvailable() function and it can be read with the readBlock()
    or readAll() functions.

    If an error occurs during the execution of one of the commands in
    a sequence of commands, all the pending commands (i.e. scheduled,
    but not yet executed commands) are cleared and no signals are
    emitted for them.

    For example, if you have the following sequence of reqeusts

    \code
    http->setHost( "www.foo.bar" );       // id == 1
    http->get( "/index.html" );           // id == 2
    http->post( "register.html", data );  // id == 3
    \endcode

    and the get() request fails because the host lookup fails, then
    the post() request is never executed and the signals would look
    like this:

    \code
    requestStarted( 1 )
    requestFinished( 1, FALSE )

    requestStarted( 2 )
    stateChanged( HostLookup )
    requestFinished( 2, TRUE )

    done( TRUE )

    stateChanged( Unconnected )
    \endcode

    You can then get details about the error with the error() and
    errorString() functions. Note that only unexpected behaviour, like
    network failure is considered as an error. If the server response
    contains an error status, like a 404 response, this is reported as
    a normal response case. So you should always check the \link
    QHttpResponseHeader::statusCode() status code \endlink of the
    response header.

    The functions currentId() and currentRequest() provide more
    information about the currently executing request.

    The functions hasPendingRequests() and clearPendingRequests()
    allow you to query and clear the list of pending requests.

    \sa \link network.html Qt Network Documentation \endlink QNetworkProtocol, QUrlOperator QFtp
*/

/*!
    Constructs a QHttp object.
*/
QHttp::QHttp()
{
    init();
}

/*!
    Constructs a QHttp object. The parameters \a parent and \a name
    are passed on to the QObject constructor.
*/
QHttp::QHttp( QObject* parent, const char* name )
{
    if ( parent )
	parent->insertChild( this );
    setName( name );
    init();
}

/*!
    Constructs a QHttp object. Subsequent requests are done by
    connecting to the server \a hostname on port \a port. The
    parameters \a parent and \a name are passed on to the QObject
    constructor.

    \sa setHost()
*/
QHttp::QHttp( const QString &hostname, Q_UINT16 port, QObject* parent, const char* name )
{
    if ( parent )
	parent->insertChild( this );
    setName( name );
    init();

    d->hostname = hostname;
    d->port = port;
}

void QHttp::init()
{
    bytesRead = 0;
    d = new QHttpPrivate;
    d->errorString = tr( "Unknown error" );

    connect( &d->socket, SIGNAL( connected() ),
	    this, SLOT( slotConnected() ) );
    connect( &d->socket, SIGNAL( connectionClosed() ),
	    this, SLOT( slotClosed() ) );
    connect( &d->socket, SIGNAL( delayedCloseFinished() ),
	    this, SLOT( slotClosed() ) );
    connect( &d->socket, SIGNAL( readyRead() ),
	    this, SLOT( slotReadyRead() ) );
    connect( &d->socket, SIGNAL( error(int) ),
	    this, SLOT( slotError(int) ) );
    connect( &d->socket, SIGNAL( bytesWritten(int) ),
	    this, SLOT( slotBytesWritten(int) ) );

    d->idleTimer = startTimer( 0 );
}

/*!
    Destroys the QHttp object. If there is an open connection, it is
    closed.
*/
QHttp::~QHttp()
{
    abort();
    delete d;
}

/*!
    \enum QHttp::State

    This enum is used to specify the state the client is in:

    \value Unconnected There is no connection to the host.
    \value HostLookup A host name lookup is in progress.
    \value Connecting An attempt to connect to the host is in progress.
    \value Sending The client is sending its request to the server.
    \value Reading The client's request has been sent and the client
    is reading the server's response.
    \value Connected The connection to the host is open, but the client is
    neither sending a request, nor waiting for a response.
    \value Closing The connection is closing down, but is not yet
    closed. (The state will be \c Unconnected when the connection is
    closed.)

    \sa stateChanged() state()
*/

/*!  \enum QHttp::Error

    This enum identifies the error that occurred.

    \value NoError No error occurred.
    \value HostNotFound The host name lookup failed.
    \value ConnectionRefused The server refused the connection.
    \value UnexpectedClose The server closed the connection unexpectedly.
    \value InvalidResponseHeader The server sent an invalid response header.
    \value WrongContentLength The client could not read the content correctly
    because an error with respect to the content length occurred.
    \value Aborted The request was aborted with abort().
    \value UnknownError An error other than those specified above
    occurred.

    \sa error()
*/

/*!
    \fn void QHttp::stateChanged( int state )

    This signal is emitted when the state of the QHttp object changes.
    The argument \a state is the new state of the connection; it is
    one of the \l State values.

    This usually happens when a request is started, but it can also
    happen when the server closes the connection or when a call to
    closeConnection() succeeded.

    \sa get() post() head() request() closeConnection() state() State
*/

/*!
    \fn void QHttp::responseHeaderReceived( const QHttpResponseHeader& resp )

    This signal is emitted when the HTTP header of a server response
    is available. The header is passed in \a resp.

    \sa get() post() head() request() readyRead()
*/

/*!
    \fn void QHttp::readyRead( const QHttpResponseHeader& resp )

    This signal is emitted when there is new response data to read.

    If you specified a device in the request where the data should be
    written to, then this signal is \e not emitted; instead the data
    is written directly to the device.

    The response header is passed in \a resp.

    You can read the data with the readAll() or readBlock() functions

    This signal is useful if you want to process the data in chunks as
    soon as it becomes available. If you are only interested in the
    complete data, just connect to the requestFinished() signal and
    read the data then instead.

    \sa get() post() request() readAll() readBlock() bytesAvailable()
*/

/*!
    \fn void QHttp::dataSendProgress( int done, int total )

    This signal is emitted when this object sends data to a HTTP
    server to inform it about the progress of the upload.

    \a done is the amount of data that has already arrived and \a
    total is the total amount of data. It is possible that the total
    amount of data that should be transferred cannot be determined, in
    which case \a total is 0.(If you connect to a QProgressBar, the
    progress bar shows a busy indicator if the total is 0).

    \warning \a done and \a total are not necessarily the size in
    bytes, since for large files these values might need to be
    "scaled" to avoid overflow.

    \sa dataReadProgress() post() request() QProgressBar::setProgress()
*/

/*!
    \fn void QHttp::dataReadProgress( int done, int total )

    This signal is emitted when this object reads data from a HTTP
    server to indicate the current progress of the download.

    \a done is the amount of data that has already arrived and \a
    total is the total amount of data. It is possible that the total
    amount of data that should be transferred cannot be determined, in
    which case \a total is 0.(If you connect to a QProgressBar, the
    progress bar shows a busy indicator if the total is 0).

    \warning \a done and \a total are not necessarily the size in
    bytes, since for large files these values might need to be
    "scaled" to avoid overflow.

    \sa dataSendProgress() get() post() request() QProgressBar::setProgress()
*/

/*!
    \fn void QHttp::requestStarted( int id )

    This signal is emitted when processing the request identified by
    \a id starts.

    \sa requestFinished() done()
*/

/*!
    \fn void QHttp::requestFinished( int id, bool error )

    This signal is emitted when processing the request identified by
    \a id has finished. \a error is TRUE if an error occurred during
    the processing; otherwise \a error is FALSE.

    \sa requestStarted() done() error() errorString()
*/

/*!
    \fn void QHttp::done( bool error )

    This signal is emitted when the last pending request has finished;
    (it is emitted after the last request's requestFinished() signal).
    \a error is TRUE if an error occurred during the processing;
    otherwise \a error is FALSE.

    \sa requestFinished() error() errorString()
*/

/*!
    Aborts the current request and deletes all scheduled requests.

    For the current request, the requestFinished() signal with the \c
    error argument \c TRUE is emitted. For all other requests that are
    affected by the abort(), no signals are emitted.

    Since this slot also deletes the scheduled requests, there are no
    requests left and the done() signal is emitted (with the \c error
    argument \c TRUE).

    \sa clearPendingRequests()
*/
void QHttp::abort()
{
    QHttpRequest *r = d->pending.getFirst();
    if ( r == 0 )
	return;

    finishedWithError( tr("Request aborted"), Aborted );
    clearPendingRequests();
    d->socket.clearPendingData();
    close();
}

/*!
    Returns the number of bytes that can be read from the response
    content at the moment.

    \sa get() post() request() readyRead() readBlock() readAll()
*/
Q_ULONG QHttp::bytesAvailable() const
{
#if defined(QHTTP_DEBUG)
    qDebug( "QHttp::bytesAvailable(): %d bytes", (int)d->rba.size() );
#endif
    return d->rba.size();
}

/*!
    Reads \a maxlen bytes from the response content into \a data and
    returns the number of bytes read. Returns -1 if an error occurred.

    \sa get() post() request() readyRead() bytesAvailable() readAll()
*/
Q_LONG QHttp::readBlock( char *data, Q_ULONG maxlen )
{
    if ( data == 0 && maxlen != 0 ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QHttp::readBlock: Null pointer error" );
#endif
	return -1;
    }
    if ( maxlen >= d->rba.size() )
	maxlen = d->rba.size();
    d->rba.consumeBytes( maxlen, data );

    d->bytesDone += maxlen;
#if defined(QHTTP_DEBUG)
    qDebug( "QHttp::readBlock(): read %d bytes (%d bytes done)", (int)maxlen, d->bytesDone );
#endif
    return maxlen;
}

/*!
    Reads all the bytes from the response content and returns them.

    \sa get() post() request() readyRead() bytesAvailable() readBlock()
*/
QByteArray QHttp::readAll()
{
    Q_ULONG avail = bytesAvailable();
    QByteArray tmp( avail );
    Q_LONG read = readBlock( tmp.data(), avail );
    tmp.resize( read );
    return tmp;
}

/*!
    Returns the identifier of the HTTP request being executed or 0 if
    there is no request being executed (i.e. they've all finished).

    \sa currentRequest()
*/
int QHttp::currentId() const
{
    QHttpRequest *r = d->pending.getFirst();
    if ( r == 0 )
	return 0;
    return r->id;
}

/*!
    Returns the request header of the HTTP request being executed. If
    the request is one issued by setHost() or closeConnection(), it
    returns an invalid request header, i.e.
    QHttpRequestHeader::isValid() returns FALSE.

    \sa currentId()
*/
QHttpRequestHeader QHttp::currentRequest() const
{
    QHttpRequest *r = d->pending.getFirst();
    if ( r != 0 && r->hasRequestHeader() )
	return r->requestHeader();
    return QHttpRequestHeader();
}

/*!
    Returns the QIODevice pointer that is used as the data source of the HTTP
    request being executed. If there is no current request or if the request
    does not use an IO device as the data source, this function returns 0.

    This function can be used to delete the QIODevice in the slot connected to
    the requestFinished() signal.

    \sa currentDestinationDevice() post() request()
*/
QIODevice* QHttp::currentSourceDevice() const
{
    QHttpRequest *r = d->pending.getFirst();
    if ( !r )
	return 0;
    return r->sourceDevice();
}

/*!
    Returns the QIODevice pointer that is used as to store the data of the HTTP
    request being executed. If there is no current request or if the request
    does not store the data to an IO device, this function returns 0.

    This function can be used to delete the QIODevice in the slot connected to
    the requestFinished() signal.

    \sa currentDestinationDevice() get() post() request()
*/
QIODevice* QHttp::currentDestinationDevice() const
{
    QHttpRequest *r = d->pending.getFirst();
    if ( !r )
	return 0;
    return r->destinationDevice();
}

/*!
    Returns TRUE if there are any requests scheduled that have not yet
    been executed; otherwise returns FALSE.

    The request that is being executed is \e not considered as a
    scheduled request.

    \sa clearPendingRequests() currentId() currentRequest()
*/
bool QHttp::hasPendingRequests() const
{
    return d->pending.count() > 1;
}

/*!
    Deletes all pending requests from the list of scheduled requests.
    This does not affect the request that is being executed. If
    you want to stop this this as well, use abort().

    \sa hasPendingRequests() abort()
*/
void QHttp::clearPendingRequests()
{
    QHttpRequest *r = 0;
    if ( d->pending.count() > 0 )
	r = d->pending.take( 0 );
    d->pending.clear();
    if ( r )
	d->pending.append( r );
}

/*!
    Sets the HTTP server that is used for requests to \a hostname on
    port \a port.

    The function does not block and returns immediately. The request
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    requestStarted() and requestFinished().

    When the request is started the requestStarted() signal is
    emitted. When it is finished the requestFinished() signal is
    emitted.

    \sa get() post() head() request() requestStarted() requestFinished() done()
*/
int QHttp::setHost(const QString &hostname, Q_UINT16 port )
{
    return addRequest( new QHttpSetHostRequest( hostname, port ) );
}

/*!
    Sends a get request for \a path to the server set by setHost() or
    as specified in the constructor.

    \a path must be an absolute path like \c /index.html or an
    absolute URI like \c http://www.trolltech.com/index.html.

    If the IO device \a to is 0 the readyRead() signal is emitted
    every time new content data is available to read.

    If the IO device \a to is not 0, the content data of the response
    is written directly to the device. Make sure that the \a to
    pointer is valid for the duration of the operation (it is safe to
    delete it when the requestFinished() signal is emitted).

    The function does not block and returns immediately. The request
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    requestStarted() and requestFinished().

    When the request is started the requestStarted() signal is
    emitted. When it is finished the requestFinished() signal is
    emitted.

    \sa setHost() post() head() request() requestStarted() requestFinished() done()
*/
int QHttp::get( const QString& path, QIODevice* to )
{
    QHttpRequestHeader header( "GET", path );
    header.setValue( "Connection", "Keep-Alive" );
    return addRequest( new QHttpPGHRequest( header, (QIODevice*)0, to ) );
}

/*!
    Sends a post request for \a path to the server set by setHost() or
    as specified in the constructor.

    \a path must be an absolute path like \c /index.html or an
    absolute URI like \c http://www.trolltech.com/index.html.

    The incoming data comes via the \a data IO device.

    If the IO device \a to is 0 the readyRead() signal is emitted
    every time new content data is available to read.

    If the IO device \a to is not 0, the content data of the response
    is written directly to the device. Make sure that the \a to
    pointer is valid for the duration of the operation (it is safe to
    delete it when the requestFinished() signal is emitted).

    The function does not block and returns immediately. The request
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    requestStarted() and requestFinished().

    When the request is started the requestStarted() signal is
    emitted. When it is finished the requestFinished() signal is
    emitted.

    \sa setHost() get() head() request() requestStarted() requestFinished() done()
*/
int QHttp::post( const QString& path, QIODevice* data, QIODevice* to  )
{
    QHttpRequestHeader header( "POST", path );
    header.setValue( "Connection", "Keep-Alive" );
    return addRequest( new QHttpPGHRequest( header, data, to ) );
}

/*!
    \overload

    \a data is used as the content data of the HTTP request.
*/
int QHttp::post( const QString& path, const QByteArray& data, QIODevice* to )
{
    QHttpRequestHeader header( "POST", path );
    header.setValue( "Connection", "Keep-Alive" );
    return addRequest( new QHttpPGHRequest( header, new QByteArray(data), to ) );
}

/*!
    Sends a header request for \a path to the server set by setHost()
    or as specified in the constructor.

    \a path must be an absolute path like \c /index.html or an
    absolute URI like \c http://www.trolltech.com/index.html.

    The function does not block and returns immediately. The request
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    requestStarted() and requestFinished().

    When the request is started the requestStarted() signal is
    emitted. When it is finished the requestFinished() signal is
    emitted.

    \sa setHost() get() post() request() requestStarted() requestFinished() done()
*/
int QHttp::head( const QString& path )
{
    QHttpRequestHeader header( "HEAD", path );
    header.setValue( "Connection", "Keep-Alive" );
    return addRequest( new QHttpPGHRequest( header, (QIODevice*)0, 0 ) );
}

/*!
    Sends a request to the server set by setHost() or as specified in
    the constructor. Uses the \a header as the HTTP request header.
    You are responsible for setting up a header that is appropriate
    for your request.

    The incoming data comes via the \a data IO device.

    If the IO device \a to is 0 the readyRead() signal is emitted
    every time new content data is available to read.

    If the IO device \a to is not 0, the content data of the response
    is written directly to the device. Make sure that the \a to
    pointer is valid for the duration of the operation (it is safe to
    delete it when the requestFinished() signal is emitted).

    The function does not block and returns immediately. The request
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    requestStarted() and requestFinished().

    When the request is started the requestStarted() signal is
    emitted. When it is finished the requestFinished() signal is
    emitted.

    \sa setHost() get() post() head() requestStarted() requestFinished() done()
*/
int QHttp::request( const QHttpRequestHeader &header, QIODevice *data, QIODevice *to )
{
    return addRequest( new QHttpNormalRequest( header, data, to ) );
}

/*!
    \overload

    \a data is used as the content data of the HTTP request.
*/
int QHttp::request( const QHttpRequestHeader &header, const QByteArray &data, QIODevice *to  )
{
    return addRequest( new QHttpNormalRequest( header, new QByteArray(data), to ) );
}

/*!
    Closes the connection; this is useful if you have a keep-alive
    connection and want to close it.

    For the requests issued with get(), post() and head(), QHttp sets
    the connection to be keep-alive. You can also do this using the
    header you pass to the request() function. QHttp only closes the
    connection to the HTTP server if the response header requires it
    to do so.

    The function does not block and returns immediately. The request
    is scheduled, and its execution is performed asynchronously. The
    function returns a unique identifier which is passed by
    requestStarted() and requestFinished().

    When the request is started the requestStarted() signal is
    emitted. When it is finished the requestFinished() signal is
    emitted.

    If you want to close the connection immediately, you have to use
    abort() instead.

    \sa stateChanged() abort() requestStarted() requestFinished() done()
*/
int QHttp::closeConnection()
{
    return addRequest( new QHttpCloseRequest() );
}

int QHttp::addRequest( QHttpRequest *req )
{
    d->pending.append( req );

    if ( d->pending.count() == 1 )
	// don't emit the requestStarted() signal before the id is returned
	QTimer::singleShot( 0, this, SLOT(startNextRequest()) );

    return req->id;
}

void QHttp::startNextRequest()
{
    QHttpRequest *r = d->pending.getFirst();
    if ( r == 0 )
	return;

    d->error = NoError;
    d->errorString = tr( "Unknown error" );

    if ( bytesAvailable() )
	readAll(); // clear the data
    emit requestStarted( r->id );
    r->start( this );
}

void QHttp::sendRequest()
{
    if ( d->hostname.isNull() ) {
	finishedWithError( tr("No server set to connect to"), UnknownError );
	return;
    }

    killIdleTimer();

    // Do we need to setup a new connection or can we reuse an
    // existing one ?
    if ( d->socket.peerName() != d->hostname || d->socket.peerPort() != d->port
        || d->socket.state() != QSocket::Connection ) {
	setState( QHttp::Connecting );
	d->socket.connectToHost( d->hostname, d->port );
    } else {
        slotConnected();
    }

}

void QHttp::finishedWithSuccess()
{
    QHttpRequest *r = d->pending.getFirst();
    if ( r == 0 )
	return;

    emit requestFinished( r->id, FALSE );
    d->pending.removeFirst();
    if ( d->pending.isEmpty() ) {
	emit done( FALSE );
    } else {
	startNextRequest();
    }
}

void QHttp::finishedWithError( const QString& detail, int errorCode )
{
    QHttpRequest *r = d->pending.getFirst();
    if ( r == 0 )
	return;

    d->error = (Error)errorCode;
    d->errorString = detail;
    emit requestFinished( r->id, TRUE );

    d->pending.clear();
    emit done( TRUE );
}

void QHttp::slotClosed()
{
    if ( d->state == Closing )
	return;

    if ( d->state == Reading ) {
	if ( d->response.hasKey( "content-length" ) ) {
	    // We got Content-Length, so did we get all bytes?
	    if ( d->bytesDone+bytesAvailable() != d->response.contentLength() ) {
		finishedWithError( tr("Wrong content length"), WrongContentLength );
	    }
	}
    } else if ( d->state == Connecting || d->state == Sending ) {
	finishedWithError( tr("Server closed connection unexpectedly"), UnexpectedClose );
    }

    d->postDevice = 0;
    setState( Closing );
    d->idleTimer = startTimer( 0 );
}

void QHttp::slotConnected()
{
    if ( d->state != Sending ) {
	d->bytesDone = 0;
	setState( Sending );
    }

    QString str = d->header.toString();
    d->bytesTotal = str.length();
    d->socket.writeBlock( str.latin1(), d->bytesTotal );
#if defined(QHTTP_DEBUG)
    qDebug( "QHttp: write request header:\n---{\n%s}---", str.latin1() );
#endif

    if ( d->postDevice ) {
	d->bytesTotal += d->postDevice->size();
    } else {
	d->bytesTotal += d->buffer.size();
	d->socket.writeBlock( d->buffer.data(), d->buffer.size() );
	d->buffer = QByteArray(); // save memory
    }
}

void QHttp::slotError( int err )
{
    d->postDevice = 0;

    if ( d->state == Connecting || d->state == Reading || d->state == Sending ) {
	switch ( err ) {
	    case QSocket::ErrConnectionRefused:
		finishedWithError( tr("Connection refused"), ConnectionRefused );
		break;
	    case QSocket::ErrHostNotFound:
		finishedWithError( tr("Host %1 not found").arg(d->socket.peerName()), HostNotFound );
		break;
	    default:
		finishedWithError( tr("HTTP request failed"), UnknownError );
		break;
	}
    }

    close();
}

void QHttp::slotBytesWritten( int written )
{
    d->bytesDone += written;
    emit dataSendProgress( d->bytesDone, d->bytesTotal );

    if ( !d->postDevice )
	return;

    if ( d->socket.bytesToWrite() == 0 ) {
	int max = QMIN( 4096, d->postDevice->size() - d->postDevice->at() );
	QByteArray arr( max );

	int n = d->postDevice->readBlock( arr.data(), max );
	if ( n != max ) {
	    qWarning("Could not read enough bytes from the device");
	    close();
	    return;
	}
	if ( d->postDevice->atEnd() ) {
	    d->postDevice = 0;
	}

	d->socket.writeBlock( arr.data(), max );
    }
}

void QHttp::slotReadyRead()
{
    if ( d->state != Reading ) {
	setState( Reading );
	d->buffer = QByteArray();
	d->readHeader = TRUE;
	d->headerStr = "";
	d->bytesDone = 0;
	d->chunkedSize = -1;
    }

    while ( d->readHeader ) {
	bool end = FALSE;
	QString tmp;
	while ( !end && d->socket.canReadLine() ) {
	    tmp = d->socket.readLine();
	    if ( tmp == "\r\n" || tmp == "\n" )
		end = TRUE;
	    else
		d->headerStr += tmp;
	}

	if ( !end )
	    return;

#if defined(QHTTP_DEBUG)
	qDebug( "QHttp: read response header:\n---{\n%s}---", d->headerStr.latin1() );
#endif
	d->response = QHttpResponseHeader( d->headerStr );
	d->headerStr = "";
#if defined(QHTTP_DEBUG)
	qDebug( "QHttp: read response header:\n---{\n%s}---", d->response.toString().latin1() );
#endif
	// Check header
	if ( !d->response.isValid() ) {
	    finishedWithError( tr("Invalid HTTP response header"), InvalidResponseHeader );
	    close();
	    return;
	}

	// The 100-continue header is ignored, because when using the
	// POST method, we send both the request header and data in
	// one chunk.
	if (d->response.statusCode() != 100) {
	    d->readHeader = FALSE;
	    if ( d->response.hasKey( "transfer-encoding" ) &&
		 d->response.value( "transfer-encoding" ).lower().contains( "chunked" ) )
		d->chunkedSize = 0;

	    emit responseHeaderReceived( d->response );
	}
    }

    if ( !d->readHeader ) {
	bool everythingRead = FALSE;

	if ( currentRequest().method() == "HEAD" ) {
	    everythingRead = TRUE;
	} else {
	    Q_ULONG n = d->socket.bytesAvailable();
	    QByteArray *arr = 0;
	    if ( d->chunkedSize != -1 ) {
		// transfer-encoding is chunked
		for ( ;; ) {
		    // get chunk size
		    if ( d->chunkedSize == 0 ) {
			if ( !d->socket.canReadLine() )
			    break;
			QString sizeString = d->socket.readLine();
			int tPos = sizeString.find( ';' );
			if ( tPos != -1 )
			    sizeString.truncate( tPos );
			bool ok;
			d->chunkedSize = sizeString.toInt( &ok, 16 );
			if ( !ok ) {
			    finishedWithError( tr("Invalid HTTP chunked body"), WrongContentLength );
			    close();
                            delete arr;
			    return;
			}
			if ( d->chunkedSize == 0 ) // last-chunk
			    d->chunkedSize = -2;
		    }

		    // read trailer
		    while ( d->chunkedSize == -2 && d->socket.canReadLine() ) {
			QString read = d->socket.readLine();
			if ( read == "\r\n" || read == "\n" )
			    d->chunkedSize = -1;
		    }
		    if ( d->chunkedSize == -1 ) {
			everythingRead = TRUE;
			break;
		    }

		    // make sure that you can read the terminating CRLF,
		    // otherwise wait until next time...
		    n = d->socket.bytesAvailable();
		    if ( n == 0 )
			break;
		    if ( (Q_LONG)n == d->chunkedSize || (Q_LONG)n == d->chunkedSize+1 ) {
			n = d->chunkedSize - 1;
			if ( n == 0 )
			    break;
		    }

		    // read data
		    uint toRead = QMIN( (Q_LONG)n, (d->chunkedSize < 0 ? (Q_LONG)n : d->chunkedSize) );
		    if ( !arr )
			arr = new QByteArray( 0 );
		    uint oldArrSize = arr->size();
		    arr->resize( oldArrSize + toRead );
		    Q_LONG read = d->socket.readBlock( arr->data()+oldArrSize, toRead );
		    arr->resize( oldArrSize + read );

		    d->chunkedSize -= read;

		    if ( d->chunkedSize == 0 && n - read >= 2 ) {
			// read terminating CRLF
			char tmp[2];
			d->socket.readBlock( tmp, 2 );
			if ( tmp[0] != '\r' || tmp[1] != '\n' ) {
			    finishedWithError( tr("Invalid HTTP chunked body"), WrongContentLength );
			    close();
			    return;
			}
		    }
		}
	    } else if ( d->response.hasContentLength() ) {
		n = QMIN( d->response.contentLength() - d->bytesDone, n );
		if ( n > 0 ) {
		    arr = new QByteArray( n );
		    Q_LONG read = d->socket.readBlock( arr->data(), n );
		    arr->resize( read );
		}
		if ( d->bytesDone + bytesAvailable() + n == d->response.contentLength() )
		    everythingRead = TRUE;
	    } else if ( n > 0 ) {
		// workaround for VC++ bug
		QByteArray temp = d->socket.readAll();
		arr = new QByteArray( temp );
	    }

	    if ( arr ) {
		n = arr->size();
		if ( d->toDevice ) {
		    d->toDevice->writeBlock( arr->data(), n );
		    delete arr;
		    d->bytesDone += n;
#if defined(QHTTP_DEBUG)
		    qDebug( "QHttp::slotReadyRead(): read %ld bytes (%d bytes done)", n, d->bytesDone );
#endif
		    if ( d->response.hasContentLength() )
			emit dataReadProgress( d->bytesDone, d->response.contentLength() );
		    else
			emit dataReadProgress( d->bytesDone, 0 );
		} else {
		    d->rba.append( arr );
#if defined(QHTTP_DEBUG)
		    qDebug( "QHttp::slotReadyRead(): read %ld bytes (%ld bytes done)", n, d->bytesDone + bytesAvailable() );
#endif
		    if ( d->response.hasContentLength() )
			emit dataReadProgress( d->bytesDone + bytesAvailable(), d->response.contentLength() );
		    else
			emit dataReadProgress( d->bytesDone + bytesAvailable(), 0 );
		    emit readyRead( d->response );
		}
	    }
	}

	if ( everythingRead ) {
	    // Handle "Connection: close"
	    if ( d->response.value("connection").lower() == "close" ) {
		close();
	    } else {
		setState( Connected );
		// Start a timer, so that we emit the keep alive signal
		// "after" this method returned.
		d->idleTimer = startTimer( 0 );
	    }
	}
    }
}

/*!
    Returns the current state of the object. When the state changes,
    the stateChanged() signal is emitted.

    \sa State stateChanged()
*/
QHttp::State QHttp::state() const
{
    return d->state;
}

/*!
    Returns the last error that occurred. This is useful to find out
    what happened when receiving a requestFinished() or a done()
    signal with the \c error argument \c TRUE.

    If you start a new request, the error status is reset to \c NoError.
*/
QHttp::Error QHttp::error() const
{
    return d->error;
}

/*!
    Returns a human-readable description of the last error that
    occurred. This is useful to present a error message to the user
    when receiving a requestFinished() or a done() signal with the \c
    error argument \c TRUE.
*/
QString QHttp::errorString() const
{
    return d->errorString;
}

/*! \reimp
*/
void QHttp::timerEvent( QTimerEvent *e )
{
    if ( e->timerId() == d->idleTimer ) {
	killTimer( d->idleTimer );
	d->idleTimer = 0;

	if ( d->state == Connected ) {
	    finishedWithSuccess();
	} else if ( d->state != Unconnected ) {
	    setState( Unconnected );
	    finishedWithSuccess();
	}
    } else {
	QObject::timerEvent( e );
    }
}

void QHttp::killIdleTimer()
{
    killTimer( d->idleTimer );
    d->idleTimer = 0;
}

void QHttp::setState( int s )
{
#if defined(QHTTP_DEBUG)
    qDebug( "QHttp state changed %d -> %d", d->state, s );
#endif
    d->state = (State)s;
    emit stateChanged( s );
}

void QHttp::close()
{
    // If no connection is open -> ignore
    if ( d->state == Closing || d->state == Unconnected )
	return;

    d->postDevice = 0;
    setState( Closing );

    // Already closed ?
    if ( !d->socket.isOpen() ) {
	d->idleTimer = startTimer( 0 );
    } else {
	// Close now.
	d->socket.close();

	// Did close succeed immediately ?
	if ( d->socket.state() == QSocket::Idle ) {
	    // Prepare to emit the requestFinished() signal.
	    d->idleTimer = startTimer( 0 );
	}
    }
}

/**********************************************************************
 *
 * QHttp implementation of the QNetworkProtocol interface
 *
 *********************************************************************/
/*! \reimp
*/
int QHttp::supportedOperations() const
{
    return OpGet | OpPut;
}

/*! \reimp
*/
void QHttp::operationGet( QNetworkOperation *op )
{
    connect( this, SIGNAL(readyRead(const QHttpResponseHeader&)),
	    this, SLOT(clientReply(const QHttpResponseHeader&)) );
    connect( this, SIGNAL(done(bool)),
	    this, SLOT(clientDone(bool)) );
    connect( this, SIGNAL(stateChanged(int)),
	    this, SLOT(clientStateChanged(int)) );

    bytesRead = 0;
    op->setState( StInProgress );
    QUrl u( operationInProgress()->arg( 0 ) );
    QHttpRequestHeader header( "GET", u.encodedPathAndQuery(), 1, 0 );
    header.setValue( "Host", u.host() );
    setHost( u.host(), u.port() != -1 ? u.port() : 80 );
    request( header );
}

/*! \reimp
*/
void QHttp::operationPut( QNetworkOperation *op )
{
    connect( this, SIGNAL(readyRead(const QHttpResponseHeader&)),
	    this, SLOT(clientReply(const QHttpResponseHeader&)) );
    connect( this, SIGNAL(done(bool)),
	    this, SLOT(clientDone(bool)) );
    connect( this, SIGNAL(stateChanged(int)),
	    this, SLOT(clientStateChanged(int)) );

    bytesRead = 0;
    op->setState( StInProgress );
    QUrl u( operationInProgress()->arg( 0 ) );
    QHttpRequestHeader header( "POST", u.encodedPathAndQuery(), 1, 0 );
    header.setValue( "Host", u.host() );
    setHost( u.host(), u.port() != -1 ? u.port() : 80 );
    request( header, op->rawArg(1) );
}

void QHttp::clientReply( const QHttpResponseHeader &rep )
{
    QNetworkOperation *op = operationInProgress();
    if ( op ) {
	if ( rep.statusCode() >= 400 && rep.statusCode() < 600 ) {
	    op->setState( StFailed );
	    op->setProtocolDetail(
		    QString("%1 %2").arg(rep.statusCode()).arg(rep.reasonPhrase())
						    );
	    switch ( rep.statusCode() ) {
		case 401:
		case 403:
		case 405:
		    op->setErrorCode( ErrPermissionDenied );
		    break;
		case 404:
		    op->setErrorCode(ErrFileNotExisting );
		    break;
		default:
		    if ( op->operation() == OpGet )
			op->setErrorCode( ErrGet );
		    else
			op->setErrorCode( ErrPut );
		    break;
	    }
	}
	// ### In cases of an error, should we still emit the data() signals?
	if ( op->operation() == OpGet && bytesAvailable() > 0 ) {
	    QByteArray ba = readAll();
	    emit data( ba, op );
	    bytesRead += ba.size();
	    if ( rep.hasContentLength() ) {
		emit dataTransferProgress( bytesRead, rep.contentLength(), op );
	    }
	}
    }
}

void QHttp::clientDone( bool err )
{
    disconnect( this, SIGNAL(readyRead(const QHttpResponseHeader&)),
	    this, SLOT(clientReply(const QHttpResponseHeader&)) );
    disconnect( this, SIGNAL(done(bool)),
	    this, SLOT(clientDone(bool)) );
    disconnect( this, SIGNAL(stateChanged(int)),
	    this, SLOT(clientStateChanged(int)) );

    if ( err ) {
	QNetworkOperation *op = operationInProgress();
	if ( op ) {
	    op->setState( QNetworkProtocol::StFailed );
	    op->setProtocolDetail( errorString() );
	    switch ( error() ) {
		case ConnectionRefused:
		    op->setErrorCode( ErrHostNotFound );
		    break;
		case HostNotFound:
		    op->setErrorCode( ErrHostNotFound );
		    break;
		default:
		    if ( op->operation() == OpGet )
			op->setErrorCode( ErrGet );
		    else
			op->setErrorCode( ErrPut );
		    break;
	    }
	    emit finished( op );
	}
    } else {
	QNetworkOperation *op = operationInProgress();
	if ( op ) {
	    if ( op->state() != StFailed ) {
		op->setState( QNetworkProtocol::StDone );
		op->setErrorCode( QNetworkProtocol::NoError );
	    }
	    emit finished( op );
	}
    }

}

void QHttp::clientStateChanged( int state )
{
    if ( url() ) {
	switch ( (State)state ) {
	    case Connecting:
		emit connectionStateChanged( ConHostFound, tr( "Host %1 found" ).arg( url()->host() ) );
		break;
	    case Sending:
		emit connectionStateChanged( ConConnected, tr( "Connected to host %1" ).arg( url()->host() ) );
		break;
	    case Unconnected:
		emit connectionStateChanged( ConClosed, tr( "Connection to %1 closed" ).arg( url()->host() ) );
		break;
	    default:
		break;
	}
    } else {
	switch ( (State)state ) {
	    case Connecting:
		emit connectionStateChanged( ConHostFound, tr( "Host found" ) );
		break;
	    case Sending:
		emit connectionStateChanged( ConConnected, tr( "Connected to host" ) );
		break;
	    case Unconnected:
		emit connectionStateChanged( ConClosed, tr( "Connection closed" ) );
		break;
	    default:
		break;
	}
    }
}

#endif
