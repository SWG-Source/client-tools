/****************************************************************************
** $Id: qurloperator.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QUrlOperator class
**
** Created : 950429
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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

#include "qurloperator.h"

#ifndef QT_NO_NETWORKPROTOCOL

#include "qurlinfo.h"
#include "qnetworkprotocol.h"
#include "qmap.h"
#include "qdir.h"
#include "qptrdict.h"
#include "qguardedptr.h"

//#define QURLOPERATOR_DEBUG

class QUrlOperatorPrivate
{
public:
    QUrlOperatorPrivate()
    {
	oldOps.setAutoDelete( FALSE );
	networkProtocol = 0;
	nameFilter = "*";
	currPut = 0;
    }

    ~QUrlOperatorPrivate()
    {
	delete networkProtocol;
	while ( oldOps.first() ) {
	    oldOps.first()->free();
	    oldOps.removeFirst();
	}
    }

    QMap<QString, QUrlInfo> entryMap;
    QNetworkProtocol *networkProtocol;
    QString nameFilter;
    QDir dir;

    // maps needed for copy/move operations
    QPtrDict<QNetworkOperation> getOpPutOpMap;
    QPtrDict<QNetworkProtocol> getOpPutProtMap;
    QPtrDict<QNetworkProtocol> getOpGetProtMap;
    QPtrDict<QNetworkOperation> getOpRemoveOpMap;
    QGuardedPtr<QNetworkProtocol> currPut;
    QStringList waitingCopies;
    QString waitingCopiesDest;
    bool waitingCopiesMove;
    QPtrList< QNetworkOperation > oldOps;
};

/*!
    \class QUrlOperator qurloperator.h

    \brief The QUrlOperator class provides common operations on URLs.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \ingroup misc
    \mainclass

    \module network

    This class operates on hierarchical structures (such as
    filesystems) using URLs. Its API facilitates all the common
    operations:
    \table
    \header \i Operation	\i Function
    \row \i List files		\i \l listChildren()
    \row \i Make a directory	\i \l mkdir()
    \row \i Remove a file	\i \l remove()
    \row \i Rename a file	\i \l rename()
    \row \i Get a file		\i \l get()
    \row \i Put a file		\i \l put()
    \row \i Copy a file		\i \l copy()
    \endtable

    You can obtain additional information about the URL with isDir()
    and info(). If a directory is to be traversed using
    listChildren(), a name filter can be set with setNameFilter().

    A QUrlOperator can be used like this, for example to download a
    file (and assuming that the FTP protocol is \link
    qInitNetworkProtocols() registered\endlink):
    \code
    QUrlOperator *op = new QUrlOperator();
    op->copy( QString("ftp://ftp.trolltech.com/qt/source/qt-2.1.0.tar.gz"),
	     "file:/tmp" );
    \endcode

    If you want to be notified about success/failure, progress, etc.,
    you can connect to QUrlOperator's signals, e.g. to start(),
    newChildren(), createdDirectory(), removed(), data(),
    dataTransferProgress(), startedNextCopy(),
    connectionStateChanged(), finished(), etc. A network operation can
    be stopped with stop().

    The class uses the functionality of registered network protocols
    to perform these operations. Depending of the protocol of the URL,
    it uses an appropriate network protocol class for the operations.
    Each of the operation functions of QUrlOperator creates a
    QNetworkOperation object that describes the operation and puts it
    into the operation queue for the network protocol used. If no
    suitable protocol could be found (because no implementation of the
    necessary network protocol is registered), the URL operator emits
    errors. Not every protocol supports every operation, but error
    handling deals with this problem.

    To register the available network protocols, use the
    qInitNetworkProtocols() function. The protocols currently
    supported are:
    \list
    \i \link QFtp FTP\endlink,
    \i \link QHttp HTTP\endlink,
    \i \link QLocalFs local file system\endlink.
    \endlist

    For more information about the Qt Network Architecture see the
    \link network.html Qt Network Documentation\endlink.

    \sa QNetworkProtocol, QNetworkOperation
*/

/*!
    \fn void QUrlOperator::newChildren( const QValueList<QUrlInfo> &i, QNetworkOperation *op )

    This signal is emitted after listChildren() was called and new
    children (i.e. files) have been read from a list of files. \a i
    holds the information about the new files. \a op is a pointer
    to the operation object which contains all the information about
    the operation, including the state.

    \sa QNetworkOperation, QNetworkProtocol
*/


/*!
    \fn void QUrlOperator::finished( QNetworkOperation *op )

    This signal is emitted when an operation of some sort finishes,
    whether with success or failure. \a op is a pointer to the
    operation object, which contains all the information, including
    the state, of the operation which has been finished. Check the
    state and error code of the operation object to see whether or not
    the operation was successful.

    \sa QNetworkOperation, QNetworkProtocol
*/

/*!
    \fn void QUrlOperator::start( QNetworkOperation *op )

    Some operations (such as listChildren()) emit this signal when
    they start processing the operation. \a op is a pointer to the
    operation object which contains all the information about the
    operation, including the state.

    \sa QNetworkOperation, QNetworkProtocol
*/

/*!
    \fn void QUrlOperator::createdDirectory( const QUrlInfo &i, QNetworkOperation *op )

    This signal is emitted when mkdir() succeeds and the directory has
    been created. \a i holds the information about the new directory.

    \a op is a pointer to the operation object, which contains all the
    information about the operation, including the state.
    \c op->arg(0) holds the new directory's name.

    \sa QNetworkOperation, QNetworkProtocol
*/

/*!
    \fn void QUrlOperator::removed( QNetworkOperation *op )

    This signal is emitted when remove() has been succesful and the
    file has been removed.

    \a op is a pointer to the operation object which contains all the
    information about the operation, including the state.
    \c op->arg(0) holds the name of the file that was removed.

    \sa QNetworkOperation, QNetworkProtocol
*/

/*!
    \fn void QUrlOperator::itemChanged( QNetworkOperation *op )

    This signal is emitted whenever a file which is a child of the URL
    has been changed, for example by successfully calling rename().
    \a op is a pointer to the operation object which contains all the
    information about the operation, including the state.
    \c op->arg(0) holds the original file name and \c op->arg(1) holds
    the new file name (if it was changed).

    \sa QNetworkOperation, QNetworkProtocol
*/

/*!
    \fn void QUrlOperator::data( const QByteArray &data, QNetworkOperation *op )

    This signal is emitted when new \a data has been received after calling
    get() or put().
    \a op is a pointer to the operation object which contains all
    the information about the operation, including the state.
    \c op->arg(0) holds the name of the file whose data is retrieved
    and op->rawArg(1) holds the (raw) data.

    \sa QNetworkOperation, QNetworkProtocol
*/

/*!
    \fn void QUrlOperator::dataTransferProgress( int bytesDone, int bytesTotal, QNetworkOperation *op )

    This signal is emitted during data transfer (using put() or
    get()). \a bytesDone specifies how many bytes of \a bytesTotal have
    been transferred. More information about the operation is stored in
    \a op, a pointer to the network operation that is processed.
    \a bytesTotal may be -1, which means that the total number of bytes
    is not known.

    \sa QNetworkOperation, QNetworkProtocol
*/

/*!
    \fn void QUrlOperator::startedNextCopy( const QPtrList<QNetworkOperation> &lst )

    This signal is emitted if copy() starts a new copy operation. \a
    lst contains all QNetworkOperations related to this copy
    operation.

    \sa copy()
*/

/*!
    \fn void QUrlOperator::connectionStateChanged( int state, const QString &data )

    This signal is emitted whenever the URL operator's connection
    state changes. \a state describes the new state, which is a
    \l{QNetworkProtocol::ConnectionState} value.

    \a data is a string that describes the change of the connection.
    This can be used to display a message to the user.
*/

/*!
    Constructs a QUrlOperator with an empty (i.e. invalid) URL.
*/

QUrlOperator::QUrlOperator()
    : QUrl()
{
#ifdef QURLOPERATOR_DEBUG
    qDebug( "QUrlOperator: cstr 1" );
#endif
    d = new QUrlOperatorPrivate;
}

/*!
    Constructs a QUrlOperator using \a url and parses this string.

    If you pass strings like "/home/qt" the "file" protocol is
    assumed.
*/

QUrlOperator::QUrlOperator( const QString &url )
    : QUrl( url )
{
#ifdef QURLOPERATOR_DEBUG
    qDebug( "QUrlOperator: cstr 2" );
#endif
    d = new QUrlOperatorPrivate;
    getNetworkProtocol();
}

/*!
    Constructs a copy of \a url.
*/

QUrlOperator::QUrlOperator( const QUrlOperator& url )
    : QObject(), QUrl( url )
{
#ifdef QURLOPERATOR_DEBUG
    qDebug( "QUrlOperator: cstr 3" );
#endif
    d = new QUrlOperatorPrivate;
    *d = *url.d;

    d->networkProtocol = 0;
    getNetworkProtocol();
    d->nameFilter = "*";
    d->currPut = 0;
}

/*!
    Constructs a QUrlOperator. The URL on which this QUrlOperator
    operates is constructed out of the arguments \a url, \a relUrl and
    \a checkSlash: see the corresponding QUrl constructor for an
    explanation of these arguments.
*/

QUrlOperator::QUrlOperator( const QUrlOperator& url, const QString& relUrl, bool checkSlash )
    : QUrl( url, relUrl, checkSlash )
{
#ifdef QURLOPERATOR_DEBUG
    qDebug( "QUrlOperator: cstr 4" );
#endif
    d = new QUrlOperatorPrivate;
    if ( relUrl == "." )
	*d = *url.d;

    d->networkProtocol = 0;
    getNetworkProtocol();
    d->currPut = 0;
}

/*!
    Destructor.
*/

QUrlOperator::~QUrlOperator()
{
#ifdef QURLOPERATOR_DEBUG
    qDebug( "QUrlOperator: dstr" );
#endif
    delete d;
}

/*!
    This private function is used by the simple operation functions,
    i.e. listChildren(), mkdir(), remove(), rename(), get() and put(),
    to really start the operation. \a op is a pointer to the network
    operation that should be started. Returns \a op on success;
    otherwise returns 0.
*/
const QNetworkOperation *QUrlOperator::startOperation( QNetworkOperation *op )
{
    if ( !d->networkProtocol )
        getNetworkProtocol();
    
    if ( d->networkProtocol && (d->networkProtocol->supportedOperations()&op->operation()) ) {
	d->networkProtocol->addOperation( op );
	if ( op->operation() == QNetworkProtocol::OpListChildren )
	    clearEntries();
	return op;
    }

    // error
    QString msg;
    if ( !d->networkProtocol ) {
	msg = tr( "The protocol `%1' is not supported" ).arg( protocol() );
    } else {
	switch ( op->operation() ) {
	case QNetworkProtocol::OpListChildren:
	    msg = tr( "The protocol `%1' does not support listing directories" ).arg( protocol() );
	    break;
	case QNetworkProtocol::OpMkDir:
	    msg = tr( "The protocol `%1' does not support creating new directories" ).arg( protocol() );
	    break;
	case QNetworkProtocol::OpRemove:
	    msg = tr( "The protocol `%1' does not support removing files or directories" ).arg( protocol() );
	    break;
	case QNetworkProtocol::OpRename:
	    msg = tr( "The protocol `%1' does not support renaming files or directories" ).arg( protocol() );
	    break;
	case QNetworkProtocol::OpGet:
	    msg = tr( "The protocol `%1' does not support getting files" ).arg( protocol() );
	    break;
	case QNetworkProtocol::OpPut:
	    msg = tr( "The protocol `%1' does not support putting files" ).arg( protocol() );
	    break;
	default:
	    // this should never happen
	    break;
	}
    }
    op->setState( QNetworkProtocol::StFailed );
    op->setProtocolDetail( msg );
    op->setErrorCode( (int)QNetworkProtocol::ErrUnsupported );
    emit finished( op );
    deleteOperation( op );
    return 0;
}

/*!
    Starts listing the children of this URL (e.g. the files in the
    directory). The start() signal is emitted before the first entry
    is listed and finished() is emitted after the last one. The
    newChildren() signal is emitted for each list of new entries. If
    an error occurs, the signal finished() is emitted, so be sure to
    check the state of the network operation pointer.

    Because the operation may not be executed immediately, a pointer
    to the QNetworkOperation object created by this function is
    returned. This object contains all the data about the operation
    and is used to refer to this operation later (e.g. in the signals
    that are emitted by the QUrlOperator). The return value can also
    be 0 if the operation object couldn't be created.

    The path of this QUrlOperator must to point to a directory
    (because the children of this directory will be listed), not to a
    file.
*/

const QNetworkOperation *QUrlOperator::listChildren()
{
    if ( !checkValid() )
	return 0;

    QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpListChildren, QString::null, QString::null, QString::null );
    return startOperation( res );
}

/*!
    Tries to create a directory (child) with the name \a dirname. If
    it is successful, a newChildren() signal with the new child is
    emitted, and the createdDirectory() signal with the information
    about the new child is also emitted. The finished() signal (with
    success or failure) is emitted after the operation has been
    processed, so check the state of the network operation object to
    see whether or not the operation was successful.

    Because the operation will not be executed immediately, a pointer
    to the QNetworkOperation object created by this function is
    returned. This object contains all the data about the operation
    and is used to refer to this operation later (e.g. in the signals
    that are emitted by the QUrlOperator). The return value can also
    be 0 if the operation object couldn't be created.

    The path of this QUrlOperator must to point to a directory (not a
    file) because the new directory will be created in this path.
*/

const QNetworkOperation *QUrlOperator::mkdir( const QString &dirname )
{
    if ( !checkValid() )
	return 0;

    QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpMkDir, dirname, QString::null, QString::null );
    return startOperation( res );
}

/*!
    Tries to remove the file (child) \a filename. If it succeeds the
    removed() signal is emitted. finished() (with success or failure)
    is also emitted after the operation has been processed, so check
    the state of the network operation object to see whether or not
    the operation was successful.

    Because the operation will not be executed immediately, a pointer
    to the QNetworkOperation object created by this function is
    returned. This object contains all the data about the operation
    and is used to refer to this operation later (e.g. in the signals
    that are emitted by the QUrlOperator). The return value can also
    be 0 if the operation object couldn't be created.

    The path of this QUrlOperator must point to a directory; because
    if \a filename is relative, it will try to remove it in this
    directory.
*/

const QNetworkOperation *QUrlOperator::remove( const QString &filename )
{
    if ( !checkValid() )
	return 0;

    QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpRemove, filename, QString::null, QString::null );
    return startOperation( res );
}

/*!
    Tries to rename the file (child) called \a oldname to \a newname.
    If it succeeds, the itemChanged() signal is emitted. finished()
    (with success or failure) is also emitted after the operation has
    been processed, so check the state of the network operation object
    to see whether or not the operation was successful.

    Because the operation may not be executed immediately, a pointer
    to the QNetworkOperation object created by this function is
    returned. This object contains all the data about the operation
    and is used to refer to this operation later (e.g. in the signals
    that are emitted by the QUrlOperator). The return value can also
    be 0 if the operation object couldn't be created.

    This path of this QUrlOperator must to point to a directory
    because \a oldname and \a newname are handled relative to this
    directory.
*/

const QNetworkOperation *QUrlOperator::rename( const QString &oldname, const QString &newname )
{
    if ( !checkValid() )
	return 0;

    QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpRename, oldname, newname, QString::null );
    return startOperation( res );
}

/*!
    Copies the file \a from to \a to. If \a move is TRUE, the file is
    moved (copied and removed). \a from must point to a file and \a to
    must point to a directory (into which \a from is copied) unless \a
    toPath is set to FALSE. If \a toPath is set to FALSE then the \a
    to variable is assumed to be the absolute file path (destination
    file path + file name). The copying is done using the get() and
    put() operations. If you want to be notified about the progress of
    the operation, connect to the dataTransferProgress() signal. Bear
    in mind that the get() and put() operations emit this signal
    through the QUrlOperator. The number of transferred bytes and the
    total bytes that you receive as arguments in this signal do not
    relate to the the whole copy operation; they relate first to the
    get() and then to the put() operation. Always check what type of
    operation the signal comes from; this is given in the signal's
    last argument.

    At the end, finished() (with success or failure) is emitted, so
    check the state of the network operation object to see whether or
    not the operation was successful.

    Because a move or copy operation consists of multiple operations
    (get(), put() and maybe remove()), this function doesn't return a
    single QNetworkOperation, but rather a list of them. They are in
    the order: get(), put() and (if applicable) remove().

    \sa get(), put()
*/

QPtrList<QNetworkOperation> QUrlOperator::copy( const QString &from, const QString &to, bool move, bool toPath )
{
#ifdef QURLOPERATOR_DEBUG
    qDebug( "QUrlOperator: copy %s %s %d", from.latin1(), to.latin1(), move );
#endif

    QPtrList<QNetworkOperation> ops;
    ops.setAutoDelete( FALSE );

    QUrlOperator *uFrom = new QUrlOperator( *this, from );
    QUrlOperator *uTo = new QUrlOperator( to );

    // prepare some string for later usage
    QString frm = *uFrom;
    QString file = uFrom->fileName();

     if (frm == to + file)
         return ops;
    
    file.prepend( "/" );

    // uFrom and uTo are deleted when the QNetworkProtocol deletes itself via
    // autodelete
    uFrom->getNetworkProtocol();
    uTo->getNetworkProtocol();
    QNetworkProtocol *gProt = uFrom->d->networkProtocol;
    QNetworkProtocol *pProt = uTo->d->networkProtocol;

    uFrom->setPath( uFrom->dirPath() );

    if ( gProt && (gProt->supportedOperations()&QNetworkProtocol::OpGet) &&
	 pProt && (pProt->supportedOperations()&QNetworkProtocol::OpPut) ) {

	connect( gProt, SIGNAL( data(const QByteArray&,QNetworkOperation*) ),
		 this, SLOT( copyGotData(const QByteArray&,QNetworkOperation*) ) );
	connect( gProt, SIGNAL( dataTransferProgress(int,int,QNetworkOperation*) ),
		 this, SIGNAL( dataTransferProgress(int,int,QNetworkOperation*) ) );
	connect( gProt, SIGNAL( finished(QNetworkOperation*) ),
		 this, SLOT( continueCopy(QNetworkOperation*) ) );
	connect( gProt, SIGNAL( finished(QNetworkOperation*) ),
		 this, SIGNAL( finished(QNetworkOperation*) ) );
	connect( gProt, SIGNAL( connectionStateChanged(int,const QString&) ),
		 this, SIGNAL( connectionStateChanged(int,const QString&) ) );

	connect( pProt, SIGNAL( dataTransferProgress(int,int,QNetworkOperation*) ),
		 this, SIGNAL( dataTransferProgress(int,int,QNetworkOperation*) ) );
	connect( pProt, SIGNAL( finished(QNetworkOperation*) ),
		 this, SIGNAL( finished(QNetworkOperation*) ) );
	connect( pProt, SIGNAL( finished(QNetworkOperation*) ),
		 this, SLOT( finishedCopy() ) );

	QNetworkOperation *opGet = new QNetworkOperation( QNetworkProtocol::OpGet, frm, QString::null, QString::null );
	ops.append( opGet );
	gProt->addOperation( opGet );


	QString toFile = to + file;
	if (!toPath)
	    toFile = to;

	QNetworkOperation *opPut = new QNetworkOperation( QNetworkProtocol::OpPut, toFile, QString::null, QString::null );
	ops.append( opPut );

	d->getOpPutProtMap.insert( (void*)opGet, pProt );
	d->getOpGetProtMap.insert( (void*)opGet, gProt );
	d->getOpPutOpMap.insert( (void*)opGet, opPut );

	if ( move && (gProt->supportedOperations()&QNetworkProtocol::OpRemove) ) {
	    gProt->setAutoDelete( FALSE );

	    QNetworkOperation *opRm = new QNetworkOperation( QNetworkProtocol::OpRemove, frm, QString::null, QString::null );
	    ops.append( opRm );
	    d->getOpRemoveOpMap.insert( (void*)opGet, opRm );
	} else {
	    gProt->setAutoDelete( TRUE );
	}
#ifdef QURLOPERATOR_DEBUG
	qDebug( "QUrlOperator: copy operation should start now..." );
#endif
	return ops;
    } else {
	QString msg;
	if ( !gProt ) {
	    msg = tr( "The protocol `%1' is not supported" ).arg( uFrom->protocol() );
	} else if ( gProt->supportedOperations() & QNetworkProtocol::OpGet ) {
	    msg = tr( "The protocol `%1' does not support copying or moving files or directories" ).arg( uFrom->protocol() );
	} else if ( !pProt ) {
	    msg = tr( "The protocol `%1' is not supported" ).arg( uTo->protocol() );
	} else {
	    msg = tr( "The protocol `%1' does not support copying or moving files or directories" ).arg( uTo->protocol() );
	}
	delete uFrom;
	delete uTo;
	QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpGet, frm, to, QString::null );
	res->setState( QNetworkProtocol::StFailed );
	res->setProtocolDetail( msg );
	res->setErrorCode( (int)QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	deleteOperation( res );
    }

    return ops;
}

/*!
    \overload

    Copies the \a files to the directory \a dest. If \a move is TRUE
    the files are moved, not copied. \a dest must point to a
    directory.

    This function calls copy() for each entry in \a files in turn. You
    don't get a result from this function; each time a new copy
    begins, startedNextCopy() is emitted, with a list of
    QNetworkOperations that describe the new copy operation.
*/

void QUrlOperator::copy( const QStringList &files, const QString &dest,
			 bool move )
{
    d->waitingCopies = files;
    d->waitingCopiesDest = dest;
    d->waitingCopiesMove = move;

    finishedCopy();
}

/*!
    Returns TRUE if the URL is a directory; otherwise returns FALSE.
    This may not always work correctly, if the protocol of the URL is
    something other than file (local filesystem). If you pass a bool
    pointer as the \a ok argument, \a *ok is set to TRUE if the result
    of this function is known to be correct, and to FALSE otherwise.
*/

bool QUrlOperator::isDir( bool *ok )
{
    if ( ok )
	*ok = TRUE;
    if ( isLocalFile() ) {
	if ( QFileInfo( path() ).isDir() )
	    return TRUE;
	else
	    return FALSE;
    }

    if ( d->entryMap.contains( "." ) ) {
	return d->entryMap[ "." ].isDir();
    }
    // #### can assume that we are a directory?
    if ( ok )
	*ok = FALSE;
    return TRUE;
}

/*!
    Tells the network protocol to get data from \a location or, if
    this is QString::null, to get data from the location to which this
    URL points (see QUrl::fileName() and QUrl::encodedPathAndQuery()).
    What happens then depends on the network protocol. The data()
    signal is emitted when data comes in. Because it's unlikely that
    all data will come in at once, it is common for multiple data()
    signals to be emitted. The dataTransferProgress() signal is
    emitted while processing the operation. At the end, finished()
    (with success or failure) is emitted, so check the state of the
    network operation object to see whether or not the operation was
    successful.

    If \a location is QString::null, the path of this QUrlOperator
    should point to a file when you use this operation. If \a location
    is not empty, it can be a relative URL (a child of the path to
    which the QUrlOperator points) or an absolute URL.

    For example, to get a web page you might do something like this:

    \code
    QUrlOperator op( "http://www.whatever.org/cgi-bin/search.pl?cmd=Hello" );
    op.get();
    \endcode

    For most other operations, the path of the QUrlOperator must point
    to a directory. If you want to download a file you could do the
    following:

    \code
    QUrlOperator op( "ftp://ftp.whatever.org/pub" );
    // do some other stuff like op.listChildren() or op.mkdir( "new_dir" )
    op.get( "a_file.txt" );
    \endcode

    This will get the data of ftp://ftp.whatever.org/pub/a_file.txt.

    \e Never do anything like this:
    \code
    QUrlOperator op( "http://www.whatever.org/cgi-bin" );
    op.get( "search.pl?cmd=Hello" ); // WRONG!
    \endcode

    If \a location is not empty and relative it must not contain any
    queries or references, just the name of a child. So if you need to
    specify a query or reference, do it as shown in the first example
    or specify the full URL (such as
    http://www.whatever.org/cgi-bin/search.pl?cmd=Hello) as \a location.

    \sa copy()
*/

const QNetworkOperation *QUrlOperator::get( const QString &location )
{
    QUrl u( *this );
    if ( !location.isEmpty() )
	u = QUrl( *this, location );

    if ( !u.isValid() )
	return 0;

    if ( !d->networkProtocol ) {
	setProtocol( u.protocol() );
	getNetworkProtocol();
    }

    QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpGet, u, QString::null, QString::null );
    return startOperation( res );
}

/*!
    This function tells the network protocol to put \a data in \a
    location. If \a location is empty (QString::null), it puts the \a
    data in the location to which the URL points. What happens depends
    on the network protocol. Depending on the network protocol, some
    data might come back after putting data, in which case the data()
    signal is emitted. The dataTransferProgress() signal is emitted
    during processing of the operation. At the end, finished() (with
    success or failure) is emitted, so check the state of the network
    operation object to see whether or not the operation was
    successful.

    If \a location is QString::null, the path of this QUrlOperator
    should point to a file when you use this operation. If \a location
    is not empty, it can be a relative (a child of the path to which
    the QUrlOperator points) or an absolute URL.

    For putting some data to a file you can do the following:

    \code
    QUrlOperator op( "ftp://ftp.whatever.com/home/me/filename.dat" );
    op.put( data );
    \endcode

    For most other operations, the path of the QUrlOperator must point
    to a directory. If you want to upload data to a file you could do
    the following:

    \code
    QUrlOperator op( "ftp://ftp.whatever.com/home/me" );
    // do some other stuff like op.listChildren() or op.mkdir( "new_dir" )
    op.put( data, "filename.dat" );
    \endcode

    This will upload the data to ftp://ftp.whatever.com/home/me/filename.dat.

    \sa copy()
*/

const QNetworkOperation *QUrlOperator::put( const QByteArray &data, const QString &location )
{
    QUrl u( *this );
    if ( !location.isEmpty() )
	u = QUrl( *this, location );

    if ( !u.isValid() )
	return 0;

    if ( !d->networkProtocol ) {
	setProtocol( u.protocol() );
	getNetworkProtocol();
    }

    QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpPut, u, QString::null, QString::null );
    res->setRawArg( 1, data );
    return startOperation( res );
}

/*!
    Sets the name filter of the URL to \a nameFilter.

    \sa QDir::setNameFilter()
*/

void QUrlOperator::setNameFilter( const QString &nameFilter )
{
    d->nameFilter = nameFilter;
}

/*!
    Returns the name filter of the URL.

    \sa QUrlOperator::setNameFilter() QDir::nameFilter()
*/

QString QUrlOperator::nameFilter() const
{
    return d->nameFilter;
}

/*!
    Clears the cache of children.
*/

void QUrlOperator::clearEntries()
{
    d->entryMap.clear();
}

/*!
    Adds an entry to the cache of children.
*/

void QUrlOperator::addEntry( const QValueList<QUrlInfo> &i )
{
    QValueList<QUrlInfo>::ConstIterator it = i.begin();
    for ( ; it != i.end(); ++it )
	d->entryMap[ ( *it ).name().stripWhiteSpace() ] = *it;
}

/*!
    Returns the URL information for the child \a entry, or returns an
    empty QUrlInfo object if there is no information available about
    \a entry. Information about \a entry is only available after a successfully
    finished listChildren() operation.
*/

QUrlInfo QUrlOperator::info( const QString &entry ) const
{
    if ( d->entryMap.contains( entry.stripWhiteSpace() ) ) {
	return d->entryMap[ entry.stripWhiteSpace() ];
    } else if ( entry == "." || entry == ".." ) {
	 // return a faked QUrlInfo
	 QUrlInfo inf;
	 inf.setName( entry );
	 inf.setDir( TRUE );
	 inf.setFile( FALSE );
	 inf.setSymLink( FALSE );
	 inf.setOwner( tr( "(unknown)" ) );
	 inf.setGroup( tr( "(unknown)" ) );
	 inf.setSize( 0 );
	 inf.setWritable( FALSE );
	 inf.setReadable( TRUE );
	 return inf;
    }
    return QUrlInfo();
}

/*!
    Finds a network protocol for the URL and deletes the old network protocol.
*/

void QUrlOperator::getNetworkProtocol()
{
    delete d->networkProtocol;
    QNetworkProtocol *p = QNetworkProtocol::getNetworkProtocol( protocol() );
    if ( !p ) {
	d->networkProtocol = 0;
	return;
    }

    d->networkProtocol = (QNetworkProtocol *)p;
    d->networkProtocol->setUrl( this );
    connect( d->networkProtocol, SIGNAL( itemChanged(QNetworkOperation*) ),
	     this, SLOT( slotItemChanged(QNetworkOperation*) ) );
}

/*!
    Deletes the currently used network protocol.
*/

void QUrlOperator::deleteNetworkProtocol()
{
    if (d->networkProtocol) {
        d->networkProtocol->deleteLater();
        d->networkProtocol = 0;
    }
}

/*!
    \reimp
*/

void QUrlOperator::setPath( const QString& path )
{
    QUrl::setPath( path );
    if ( d->networkProtocol )
	d->networkProtocol->setUrl( this );
}

/*!
    \reimp
*/

void QUrlOperator::reset()
{
    QUrl::reset();
    deleteNetworkProtocol();
    d->nameFilter = "*";
}

/*!
    \reimp
*/

bool QUrlOperator::parse( const QString &url )
{
    bool b = QUrl::parse( url );
    if ( !b ) {
	return b;
    }

    getNetworkProtocol();

    return b;
}

/*!
    \reimp
*/

QUrlOperator& QUrlOperator::operator=( const QUrlOperator &url )
{
    deleteNetworkProtocol();
    QUrl::operator=( url );

    QPtrDict<QNetworkOperation> getOpPutOpMap = d->getOpPutOpMap;
    QPtrDict<QNetworkProtocol> getOpPutProtMap = d->getOpPutProtMap;
    QPtrDict<QNetworkProtocol> getOpGetProtMap = d->getOpGetProtMap;
    QPtrDict<QNetworkOperation> getOpRemoveOpMap = d->getOpRemoveOpMap;

    *d = *url.d;

    d->oldOps.setAutoDelete( FALSE );
    d->getOpPutOpMap = getOpPutOpMap;
    d->getOpPutProtMap = getOpPutProtMap;
    d->getOpGetProtMap = getOpGetProtMap;
    d->getOpRemoveOpMap = getOpRemoveOpMap;

    d->networkProtocol = 0;
    getNetworkProtocol();
    return *this;
}

/*!
    \reimp
*/

QUrlOperator& QUrlOperator::operator=( const QString &url )
{
    deleteNetworkProtocol();
    QUrl::operator=( url );
    d->oldOps.setAutoDelete( FALSE );
    getNetworkProtocol();
    return *this;
}

/*!
    \reimp
*/

bool QUrlOperator::cdUp()
{
    bool b = QUrl::cdUp();
    if ( d->networkProtocol )
	d->networkProtocol->setUrl( this );
    return b;
}

/*!
    \reimp
*/

bool QUrlOperator::checkValid()
{
    // ######
    if ( !isValid() ) {
	//emit error( ErrValid, tr( "The entered URL is not valid!" ) );
	return FALSE;
    } else
	return TRUE;
}


/*!
    \internal
*/

void QUrlOperator::copyGotData( const QByteArray &data_, QNetworkOperation *op )
{
#ifdef QURLOPERATOR_DEBUG
    qDebug( "QUrlOperator: copyGotData: %d new bytes", data_.size() );
#endif
    QNetworkOperation *put = d->getOpPutOpMap[ (void*)op ];
    if ( put ) {
	QByteArray &s = put->raw( 1 );
	int size = s.size();
	s.resize( size + data_.size() );
	memcpy( s.data() + size, data_.data(), data_.size() );
    }
    emit data( data_, op );
}

/*!
    \internal
*/

void QUrlOperator::continueCopy( QNetworkOperation *op )
{
    if ( op->operation() != QNetworkProtocol::OpGet )
	return;
    if ( op->state()!=QNetworkProtocol::StDone &&  op->state()!=QNetworkProtocol::StFailed ) {
	return;
    }

#ifdef QURLOPERATOR_DEBUG
    if ( op->state() != QNetworkProtocol::StFailed ) {
	qDebug( "QUrlOperator: continue copy (get finished, put will start)" );
    }
#endif

    QNetworkOperation *put = d->getOpPutOpMap[ (void*)op ];
    QNetworkProtocol *gProt = d->getOpGetProtMap[ (void*)op ];
    QNetworkProtocol *pProt = d->getOpPutProtMap[ (void*)op ];
    QNetworkOperation *rm = d->getOpRemoveOpMap[ (void*)op ];
    d->getOpPutOpMap.take( op );
    d->getOpGetProtMap.take( op );
    d->getOpPutProtMap.take( op );
    d->getOpRemoveOpMap.take( op );
    if ( pProt )
	pProt->setAutoDelete( TRUE );
    if ( put && pProt ) {
	if ( op->state() != QNetworkProtocol::StFailed ) {
	    pProt->addOperation( put );
	    d->currPut = pProt;
	} else {
	    deleteOperation( put );
	}
    }
    if ( gProt ) {
	gProt->setAutoDelete( TRUE );
    }
    if ( rm && gProt ) {
	if ( op->state() != QNetworkProtocol::StFailed ) {
	    gProt->addOperation( rm );
	} else {
	    deleteOperation( rm );
	}
    }
    disconnect( gProt, SIGNAL( data(const QByteArray&,QNetworkOperation*) ),
		this, SLOT( copyGotData(const QByteArray&,QNetworkOperation*) ) );
    disconnect( gProt, SIGNAL( finished(QNetworkOperation*) ),
		this, SLOT( continueCopy(QNetworkOperation*) ) );
}

/*!
    \internal
*/

void QUrlOperator::finishedCopy()
{
#ifdef QURLOPERATOR_DEBUG
    qDebug( "QUrlOperator: finished copy (finished putting)" );
#endif

    if ( d->waitingCopies.isEmpty() )
	return;

    QString cp = d->waitingCopies.first();
    d->waitingCopies.remove( cp );
    QPtrList<QNetworkOperation> lst = copy( cp, d->waitingCopiesDest, d->waitingCopiesMove );
    emit startedNextCopy( lst );
}

/*!
    Stops the current network operation and removes all this
    QUrlOperator's waiting network operations.
*/

void QUrlOperator::stop()
{
    d->getOpPutOpMap.clear();
    d->getOpRemoveOpMap.clear();
    d->getOpGetProtMap.setAutoDelete( TRUE );
    d->getOpPutProtMap.setAutoDelete( TRUE );
    QPtrDictIterator<QNetworkProtocol> it( d->getOpPutProtMap );
    for ( ; it.current(); ++it )
	it.current()->stop();
    d->getOpPutProtMap.clear();
    it = QPtrDictIterator<QNetworkProtocol>( d->getOpGetProtMap );
    for ( ; it.current(); ++it )
	it.current()->stop();
    d->getOpGetProtMap.clear();
    if ( d->currPut ) {
	d->currPut->stop();
	delete (QNetworkProtocol *) d->currPut;
	d->currPut = 0;
    }
    d->waitingCopies.clear();
    if ( d->networkProtocol )
	d->networkProtocol->stop();
    getNetworkProtocol();
}

/*!
    \internal
*/

void QUrlOperator::deleteOperation( QNetworkOperation *op )
{
    if ( op )
	d->oldOps.append( op );
}

/*!
    \internal
    updates the entryMap after a network operation finished
*/

void QUrlOperator::slotItemChanged( QNetworkOperation *op )
{
    if ( !op )
	return;

    switch ( op->operation() ) {
    case QNetworkProtocol::OpRename :
    {
	if ( op->arg( 0 ) == op->arg( 1 ) )
	    return;

	QMap<QString, QUrlInfo>::iterator mi = d->entryMap.find( op->arg( 0 ) );
	if ( mi != d->entryMap.end() ) {
	    mi.data().setName( op->arg( 1 ) );
	    d->entryMap[ op->arg( 1 ) ] = mi.data();
	    d->entryMap.erase( mi );
	}
	break;
    }
    case QNetworkProtocol::OpRemove :
    {
	QMap<QString, QUrlInfo>::iterator mi = d->entryMap.find( op->arg( 0 ) );
	if ( mi != d->entryMap.end() )
	    d->entryMap.erase( mi );
	break;
    }
    default:
	break;
    }
}


#endif // QT_NO_NETWORKPROTOCOL
