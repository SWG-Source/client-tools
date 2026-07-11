/****************************************************************************
** $Id: qmime.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of MIME support
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

#include "qmime.h"

#ifndef QT_NO_MIME

#include "qmap.h"
#include "qstringlist.h"
#include "qfileinfo.h"
#include "qdir.h"
#include "qdragobject.h"
#include "qcleanuphandler.h"
#include "qapplication.h" // ### for now
#include "qclipboard.h" // ### for now

/*!
    \class QMimeSource qmime.h
    \brief The QMimeSource class is an abstraction of objects which provide formatted data of a certain MIME type.

    \ingroup io
    \ingroup draganddrop
    \ingroup misc

    \link dnd.html Drag-and-drop\endlink and
    \link QClipboard clipboard\endlink use this abstraction.

    \sa \link http://www.isi.edu/in-notes/iana/assignments/media-types/
	    IANA list of MIME media types\endlink
*/

static int qt_mime_serial_number = 0;
static QMimeSourceFactory* defaultfactory = 0;
static QSingleCleanupHandler<QMimeSourceFactory> qmime_cleanup_factory;

/*!
    Constructs a mime source and assigns a globally unique serial
    number to it.

    \sa serialNumber()
*/

QMimeSource::QMimeSource()
{
    ser_no = qt_mime_serial_number++;
    cacheType = NoCache;
}

/*!
    \fn int QMimeSource::serialNumber() const

    Returns the mime source's globally unique serial number.
*/


void QMimeSource::clearCache()
{
    if ( cacheType == Text ) {
	delete cache.txt.str;
	delete cache.txt.subtype;
	cache.txt.str = 0;
	cache.txt.subtype = 0;
    } else if ( cacheType == Graphics ) {
	delete cache.gfx.img;
	delete cache.gfx.pix;
	cache.gfx.img = 0;
	cache.gfx.pix = 0;
    }
    cacheType = NoCache;
}

/*!
    Provided to ensure that subclasses destroy themselves correctly.
*/
QMimeSource::~QMimeSource()
{
#ifndef QT_NO_CLIPBOARD
    extern void qt_clipboard_cleanup_mime_source(QMimeSource *);
    qt_clipboard_cleanup_mime_source(this);
#endif
    clearCache();
}

/*!
    \fn QByteArray QMimeSource::encodedData(const char*) const

    Returns the encoded data of this object in the specified MIME
    format.

    Subclasses must reimplement this function.
*/



/*!
    Returns TRUE if the object can provide the data in format \a
    mimeType; otherwise returns FALSE.

    If you inherit from QMimeSource, for consistency reasons it is
    better to implement the more abstract canDecode() functions such
    as QTextDrag::canDecode() and QImageDrag::canDecode().
*/
bool QMimeSource::provides(const char* mimeType) const
{
    const char* fmt;
    for (int i=0; (fmt = format(i)); i++) {
	if ( !qstricmp(mimeType,fmt) )
	    return TRUE;
    }
    return FALSE;
}


/*!
    \fn const char * QMimeSource::format(int i) const

    Returns the \a{i}-th supported MIME format, or 0.
*/



class QMimeSourceFactoryData {
public:
    QMimeSourceFactoryData() :
	last(0)
    {
    }

    ~QMimeSourceFactoryData()
    {
	QMap<QString, QMimeSource*>::Iterator it = stored.begin();
	while ( it != stored.end() ) {
	    delete *it;
	    ++it;
	}
	delete last;
    }

    QMap<QString, QMimeSource*> stored;
    QMap<QString, QString> extensions;
    QStringList path;
    QMimeSource* last;
    QPtrList<QMimeSourceFactory> factories;
};


/*!
    \class QMimeSourceFactory qmime.h
    \brief The QMimeSourceFactory class is an extensible provider of mime-typed data.

    \ingroup io
    \ingroup environment

    A QMimeSourceFactory provides an abstract interface to a
    collection of information. Each piece of information is
    represented by a QMimeSource object which can be examined and
    converted to concrete data types by functions such as
    QImageDrag::canDecode() and QImageDrag::decode().

    The base QMimeSourceFactory can be used in two ways: as an
    abstraction of a collection of files or as specifically stored
    data. For it to access files, call setFilePath() before accessing
    data. For stored data, call setData() for each item (there are
    also convenience functions, e.g. setText(), setImage() and
    setPixmap(), that simply call setData() with appropriate
    parameters).

    The rich text widgets, QTextEdit and QTextBrowser, use
    QMimeSourceFactory to resolve references such as images or links
    within rich text documents. They either access the default factory
    (see \l{defaultFactory()}) or their own (see
    \l{QTextEdit::setMimeSourceFactory()}). Other classes that are
    capable of displaying rich text (such as QLabel, QWhatsThis or
    QMessageBox) always use the default factory.

    A factory can also be used as a container to store data associated
    with a name. This technique is useful whenever rich text contains
    images that are stored in the program itself, not loaded from the
    hard disk. Your program may, for example, define some image data
    as:
    \code
    static const char* myimage_data[]={
    "...",
    ...
    "..."};
    \endcode

    To be able to use this image within some rich text, for example
    inside a QLabel, you must create a QImage from the raw data and
    insert it into the factory with a unique name:
    \code
    QMimeSourceFactory::defaultFactory()->setImage( "myimage", QImage(myimage_data) );
    \endcode

    Now you can create a rich text QLabel with

    \code
    QLabel* label = new QLabel(
	"Rich text with embedded image:<img source=\"myimage\">"
	"Isn't that <em>cute</em>?" );
    \endcode

    When no longer needed, you can clear the data from the factory:

    \code
    delete label;
    QMimeSourceFactory::defaultFactory()->setData( "myimage", 0 );
    \endcode
*/


/*!
    Constructs a QMimeSourceFactory that has no file path and no
    stored content.
*/
QMimeSourceFactory::QMimeSourceFactory() :
    d(new QMimeSourceFactoryData)
{
    // add some reasonable defaults
    setExtensionType("htm", "text/html;charset=iso8859-1");
    setExtensionType("html", "text/html;charset=iso8859-1");
    setExtensionType("txt", "text/plain");
    setExtensionType("xml", "text/xml;charset=UTF-8");
    setExtensionType("jpg", "image/jpeg"); // support misspelled jpeg files
}

/*!
    Destroys the QMimeSourceFactory, deleting all stored content.
*/
QMimeSourceFactory::~QMimeSourceFactory()
{
    if ( defaultFactory() == this )
	defaultfactory = 0;
    delete d;
}

QMimeSource* QMimeSourceFactory::dataInternal(const QString& abs_name, const QMap<QString, QString> &extensions ) const
{
    QMimeSource* r = 0;
    QFileInfo fi(abs_name);
    if ( fi.isReadable() ) {

	// get the right mimetype
	QString e = fi.extension(FALSE);
	QCString mimetype = "application/octet-stream";
	const char* imgfmt;
	if ( extensions.contains(e) )
	    mimetype = extensions[e].latin1();
	else if ( ( imgfmt = QImage::imageFormat( abs_name ) ) )
	    mimetype = QCString("image/")+QCString(imgfmt).lower();

	QFile f(abs_name);
	if ( f.open(IO_ReadOnly) && f.size() ) {
	    QByteArray ba(f.size());
	    f.readBlock(ba.data(), ba.size());
	    QStoredDrag* sr = new QStoredDrag( mimetype );
	    sr->setEncodedData( ba );
	    delete d->last;
	    d->last = r = sr;
	}
    }

    // we didn't find the mime-source, so ask the default factory for
    // the mime-source (this one will iterate over all installed ones)
    //
    // this looks dangerous, as this dataInternal() function will be
    // called again when the default factory loops over all installed
    // factories (including this), but the static bool looping in
    // data() avoids endless recursions
    if ( !r && this != defaultFactory() )
	r = (QMimeSource*)defaultFactory()->data( abs_name );

    return r;
}


/*!
    Returns a reference to the data associated with \a abs_name. The
    return value remains valid only until the next data() or setData()
    call, so you should immediately decode the result.

    If there is no data associated with \a abs_name in the factory's
    store, the factory tries to access the local filesystem. If \a
    abs_name isn't an absolute file name, the factory will search for
    it in all defined paths (see \l{setFilePath()}).

    The factory understands all the image formats supported by
    QImageIO. Any other mime types are determined by the file name
    extension. The default settings are
    \code
    setExtensionType("html", "text/html;charset=iso8859-1");
    setExtensionType("htm", "text/html;charset=iso8859-1");
    setExtensionType("txt", "text/plain");
    setExtensionType("xml", "text/xml;charset=UTF-8");
    \endcode
    The effect of these is that file names ending in "txt" will be
    treated as text encoded in the local encoding; those ending in
    "xml" will be treated as text encoded in Unicode UTF-8 encoding.
    The text/html type is treated specially, since the encoding can be
    specified in the html file itself. "html" or "htm" will be treated
    as text encoded in the encoding specified by the html meta tag, if
    none could be found, the charset of the mime type will be used.
    The text subtype ("html", "plain", or "xml") does not affect the
    factory, but users of the factory may behave differently. We
    recommend creating "xml" files where practical. These files can be
    viewed regardless of the runtime encoding and can encode any
    Unicode characters without resorting to encoding definitions
    inside the file.

    Any file data that is not recognized will be retrieved as a
    QMimeSource providing the "application/octet-stream" mime type,
    meaning uninterpreted binary data.

    You can add further extensions or change existing ones with
    subsequent calls to setExtensionType(). If the extension mechanism
    is not sufficient for your problem domain, you can inherit
    QMimeSourceFactory and reimplement this function to perform some
    more specialized mime-type detection. The same applies if you want
    to use the mime source factory to access URL referenced data over
    a network.
*/
const QMimeSource* QMimeSourceFactory::data(const QString& abs_name) const
{
    if ( d->stored.contains(abs_name) )
	return d->stored[abs_name];

    QMimeSource* r = 0;
    QStringList::Iterator it;
    if ( abs_name[0] == '/'
#ifdef Q_WS_WIN
	    || ( abs_name[0] && abs_name[1] == ':' ) || abs_name.startsWith("\\\\")
#endif
    )
    {
	// handle absolute file names directly
	r = dataInternal( abs_name, d->extensions);
    }
    else { // check list of paths
	for ( it = d->path.begin(); !r && it != d->path.end(); ++it ) {
	    QString filename = *it;
	    if ( filename[(int)filename.length()-1] != '/' )
		filename += '/';
	    filename += abs_name;
	    r = dataInternal( filename, d->extensions );
	}
    }

    static bool looping = FALSE;
    if ( !r && this == defaultFactory() ) {
	// we found no mime-source and we are the default factory, so
	// we know all the other installed mime-source factories, so
	// ask them
	if ( !looping ) {
	    // to avoid endless recustions, don't enter the loop below
	    // if data() got called from within the loop below
	    looping = TRUE;
	    QPtrListIterator<QMimeSourceFactory> it( d->factories );
	    QMimeSourceFactory *f;
	    while ( ( f = it.current() ) ) {
		++it;
		if ( f == this )
		    continue;
		r = (QMimeSource*)f->data( abs_name );
		if ( r ) {
		    looping = FALSE;
		    return r;
		}
	    }
	    looping = FALSE;
	}
    } else if ( !r ) {
	// we are not the default mime-source factory, so ask the
	// default one for the mime-source, as this one will loop over
	// all installed mime-source factories and ask these
	r = (QMimeSource*)defaultFactory()->data( abs_name );
    }

    return r;
}

/*!
    Sets the list of directories that will be searched when named data
    is requested to the those given in the string list \a path.

    \sa filePath()
*/
void QMimeSourceFactory::setFilePath( const QStringList& path )
{
    d->path = path;
}

/*!
    Returns the currently set search paths.
*/
QStringList QMimeSourceFactory::filePath() const
{
    return d->path;
}

/*!
    Adds another search path, \a p to the existing search paths.

    \sa setFilePath()
*/
void QMimeSourceFactory::addFilePath( const QString& p )
{
    d->path += p;
}

/*!
    Sets the mime-type to be associated with the file name extension,
    \a ext to \a mimetype. This determines the mime-type for files
    found via the paths set by setFilePath().
*/
void QMimeSourceFactory::setExtensionType( const QString& ext, const char* mimetype )
{
    d->extensions.replace(ext, mimetype);
}

/*!
    Converts the absolute or relative data item name \a
    abs_or_rel_name to an absolute name, interpreted within the
    context (path) of the data item named \a context (this must be an
    absolute name).
*/
QString QMimeSourceFactory::makeAbsolute(const QString& abs_or_rel_name, const QString& context) const
{
    if ( context.isNull() ||
	 !(context[0] == '/'
#ifdef Q_WS_WIN
	 || ( context[0] && context[1] == ':')
#endif
	   ))
	return abs_or_rel_name;
    if ( abs_or_rel_name.isEmpty() )
	return context;
    QFileInfo c( context );
    if (!c.isDir()) {
	QFileInfo r( c.dir(TRUE), abs_or_rel_name );
	return r.absFilePath();
    } else {
	QDir d(context);
	QFileInfo r(d, abs_or_rel_name);
	return r.absFilePath();
    }
}

/*!
    \overload
    A convenience function. See data(const QString& abs_name). The
    file name is given in \a abs_or_rel_name and the path is in \a
    context.
*/
const QMimeSource* QMimeSourceFactory::data(const QString& abs_or_rel_name, const QString& context) const
{
    const QMimeSource* r = data(makeAbsolute(abs_or_rel_name,context));
    if ( !r && !d->path.isEmpty() )
	r = data(abs_or_rel_name);
    return r;
}


/*!
    Sets \a text to be the data item associated with the absolute name
    \a abs_name.

    Equivalent to setData(abs_name, new QTextDrag(text)).
*/
void QMimeSourceFactory::setText( const QString& abs_name, const QString& text )
{
    setData(abs_name, new QTextDrag(text));
}

/*!
    Sets \a image to be the data item associated with the absolute
    name \a abs_name.

    Equivalent to setData(abs_name, new QImageDrag(image)).
*/
void QMimeSourceFactory::setImage( const QString& abs_name, const QImage& image )
{
    setData(abs_name, new QImageDrag(image));
}

/*!
    Sets \a pixmap to be the data item associated with the absolute
    name \a abs_name.
*/
void QMimeSourceFactory::setPixmap( const QString& abs_name, const QPixmap& pixmap )
{
    setData(abs_name, new QImageDrag(pixmap.convertToImage()));
}

/*!
  Sets \a data to be the data item associated with
  the absolute name \a abs_name. Note that the ownership of \a data is
  transferred to the factory: do not delete or access the pointer after
  passing it to this function.

  Passing 0 for data removes previously stored data.
*/
void QMimeSourceFactory::setData( const QString& abs_name, QMimeSource* data )
{
    if ( d->stored.contains(abs_name) )
	delete d->stored[abs_name];
    d->stored.replace(abs_name,data);
}


/*!
    Returns the application-wide default mime source factory. This
    factory is used by rich text rendering classes such as
    QSimpleRichText, QWhatsThis and QMessageBox to resolve named
    references within rich text documents. It serves also as the
    initial factory for the more complex render widgets, QTextEdit and
    QTextBrowser.

    \sa setDefaultFactory()
*/
QMimeSourceFactory* QMimeSourceFactory::defaultFactory()
{
    if (!defaultfactory)
    {
	defaultfactory = new QMimeSourceFactory();
	qmime_cleanup_factory.set( &defaultfactory );
    }
    return defaultfactory;
}

/*!
    Sets the default \a factory, destroying any previously set mime
    source provider. The ownership of the factory is transferred to
    Qt.

    \sa defaultFactory()
*/
void QMimeSourceFactory::setDefaultFactory( QMimeSourceFactory* factory)
{
    if ( !defaultfactory )
	qmime_cleanup_factory.set( &defaultfactory );
    else if ( defaultfactory != factory )
	delete defaultfactory;
    defaultfactory = factory;
}

/*!
    Sets the defaultFactory() to 0 and returns the previous one.
*/

QMimeSourceFactory* QMimeSourceFactory::takeDefaultFactory()
{
    QMimeSourceFactory *f = defaultfactory;
    defaultfactory = 0;
    return f;
}

/*!
    Adds the QMimeSourceFactory \a f to the list of available
    mimesource factories. If the defaultFactory() can't resolve a
    data() it iterates over the list of installed mimesource factories
    until the data can be resolved.

    \sa removeFactory();
*/

void QMimeSourceFactory::addFactory( QMimeSourceFactory *f )
{
    QMimeSourceFactory::defaultFactory()->d->factories.append( f );
}

/*!
    Removes the mimesource factory \a f from the list of available
    mimesource factories.

    \sa addFactory();
*/

void QMimeSourceFactory::removeFactory( QMimeSourceFactory *f )
{
    QMimeSourceFactory::defaultFactory()->d->factories.removeRef( f );
}

#endif // QT_NO_MIME
