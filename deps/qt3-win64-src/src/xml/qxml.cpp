/****************************************************************************
** $Id: qxml.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QXmlSimpleReader and related classes.
**
** Created : 000518
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the xml module of the Qt GUI Toolkit.
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

#include "qxml.h"
#include "qtextcodec.h"
#include "qbuffer.h"
#include "qregexp.h"
#include "qptrstack.h"
#include "qmap.h"
#include "qvaluestack.h"

// needed for QT_TRANSLATE_NOOP:
#include "qobject.h"

#ifndef QT_NO_XML

//#define QT_QXML_DEBUG

// Error strings for the XML reader
#define XMLERR_OK                         QT_TRANSLATE_NOOP( "QXml", "no error occurred" )
#define XMLERR_ERRORBYCONSUMER            QT_TRANSLATE_NOOP( "QXml", "error triggered by consumer" )
#define XMLERR_UNEXPECTEDEOF              QT_TRANSLATE_NOOP( "QXml", "unexpected end of file" )
#define XMLERR_MORETHANONEDOCTYPE         QT_TRANSLATE_NOOP( "QXml", "more than one document type definition" )
#define XMLERR_ERRORPARSINGELEMENT        QT_TRANSLATE_NOOP( "QXml", "error occurred while parsing element" )
#define XMLERR_TAGMISMATCH                QT_TRANSLATE_NOOP( "QXml", "tag mismatch" )
#define XMLERR_ERRORPARSINGCONTENT        QT_TRANSLATE_NOOP( "QXml", "error occurred while parsing content" )
#define XMLERR_UNEXPECTEDCHARACTER        QT_TRANSLATE_NOOP( "QXml", "unexpected character" )
#define XMLERR_INVALIDNAMEFORPI           QT_TRANSLATE_NOOP( "QXml", "invalid name for processing instruction" )
#define XMLERR_VERSIONEXPECTED            QT_TRANSLATE_NOOP( "QXml", "version expected while reading the XML declaration" )
#define XMLERR_WRONGVALUEFORSDECL         QT_TRANSLATE_NOOP( "QXml", "wrong value for standalone declaration" )
#define XMLERR_EDECLORSDDECLEXPECTED      QT_TRANSLATE_NOOP( "QXml", "encoding declaration or standalone declaration expected while reading the XML declaration" )
#define XMLERR_SDDECLEXPECTED             QT_TRANSLATE_NOOP( "QXml", "standalone declaration expected while reading the XML declaration" )
#define XMLERR_ERRORPARSINGDOCTYPE        QT_TRANSLATE_NOOP( "QXml", "error occurred while parsing document type definition" )
#define XMLERR_LETTEREXPECTED             QT_TRANSLATE_NOOP( "QXml", "letter is expected" )
#define XMLERR_ERRORPARSINGCOMMENT        QT_TRANSLATE_NOOP( "QXml", "error occurred while parsing comment" )
#define XMLERR_ERRORPARSINGREFERENCE      QT_TRANSLATE_NOOP( "QXml", "error occurred while parsing reference" )
#define XMLERR_INTERNALGENERALENTITYINDTD QT_TRANSLATE_NOOP( "QXml", "internal general entity reference not allowed in DTD" )
#define XMLERR_EXTERNALGENERALENTITYINAV  QT_TRANSLATE_NOOP( "QXml", "external parsed general entity reference not allowed in attribute value" )
#define XMLERR_EXTERNALGENERALENTITYINDTD QT_TRANSLATE_NOOP( "QXml", "external parsed general entity reference not allowed in DTD" )
#define XMLERR_UNPARSEDENTITYREFERENCE    QT_TRANSLATE_NOOP( "QXml", "unparsed entity reference in wrong context" )
#define XMLERR_RECURSIVEENTITIES          QT_TRANSLATE_NOOP( "QXml", "recursive entities" )
#define XMLERR_ERRORINTEXTDECL            QT_TRANSLATE_NOOP( "QXml", "error in the text declaration of an external entity" )

// the constants for the lookup table
static const signed char cltWS      =  0; // white space
static const signed char cltPer     =  1; // %
static const signed char cltAmp     =  2; // &
static const signed char cltGt      =  3; // >
static const signed char cltLt      =  4; // <
static const signed char cltSlash   =  5; // /
static const signed char cltQm      =  6; // ?
static const signed char cltEm      =  7; // !
static const signed char cltDash    =  8; // -
static const signed char cltCB      =  9; // ]
static const signed char cltOB      = 10; // [
static const signed char cltEq      = 11; // =
static const signed char cltDq      = 12; // "
static const signed char cltSq      = 13; // '
static const signed char cltUnknown = 14;

// character lookup table
static const signed char charLookupTable[256]={
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x00 - 0x07
    cltUnknown, // 0x08
    cltWS,      // 0x09 \t
    cltWS,      // 0x0A \n
    cltUnknown, // 0x0B
    cltUnknown, // 0x0C
    cltWS,      // 0x0D \r
    cltUnknown, // 0x0E
    cltUnknown, // 0x0F
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x17 - 0x16
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x18 - 0x1F
    cltWS,      // 0x20 Space
    cltEm,      // 0x21 !
    cltDq,      // 0x22 "
    cltUnknown, // 0x23
    cltUnknown, // 0x24
    cltPer,     // 0x25 %
    cltAmp,     // 0x26 &
    cltSq,      // 0x27 '
    cltUnknown, // 0x28
    cltUnknown, // 0x29
    cltUnknown, // 0x2A
    cltUnknown, // 0x2B
    cltUnknown, // 0x2C
    cltDash,    // 0x2D -
    cltUnknown, // 0x2E
    cltSlash,   // 0x2F /
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x30 - 0x37
    cltUnknown, // 0x38
    cltUnknown, // 0x39
    cltUnknown, // 0x3A
    cltUnknown, // 0x3B
    cltLt,      // 0x3C <
    cltEq,      // 0x3D =
    cltGt,      // 0x3E >
    cltQm,      // 0x3F ?
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x40 - 0x47
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x48 - 0x4F
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x50 - 0x57
    cltUnknown, // 0x58
    cltUnknown, // 0x59
    cltUnknown, // 0x5A
    cltOB,      // 0x5B [
    cltUnknown, // 0x5C
    cltCB,      // 0x5D ]
    cltUnknown, // 0x5E
    cltUnknown, // 0x5F
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x60 - 0x67
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x68 - 0x6F
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x70 - 0x77
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x78 - 0x7F
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x80 - 0x87
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x88 - 0x8F
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x90 - 0x97
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x98 - 0x9F
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xA0 - 0xA7
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xA8 - 0xAF
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xB0 - 0xB7
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xB8 - 0xBF
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xC0 - 0xC7
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xC8 - 0xCF
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xD0 - 0xD7
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xD8 - 0xDF
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xE0 - 0xE7
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xE8 - 0xEF
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xF0 - 0xF7
    cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown  // 0xF8 - 0xFF
};


//
// local helper functions
//

/*
  This function strips the TextDecl [77] ("<?xml ...?>") from the string \a
  str. The stripped version is stored in \a str. If this function finds an
  invalid TextDecl, it returns FALSE, otherwise TRUE.

  This function is used for external entities since those can include an
  TextDecl that must be stripped before inserting the entity.
*/
static bool stripTextDecl( QString& str )
{
    QString textDeclStart( "<?xml" );
    if ( str.startsWith( textDeclStart ) ) {
	QRegExp textDecl(QString::fromLatin1(
	    "^<\\?xml\\s+"
	    "(version\\s*=\\s*((['\"])[-a-zA-Z0-9_.:]+\\3))?"
	    "\\s*"
	    "(encoding\\s*=\\s*((['\"])[A-Za-z][-a-zA-Z0-9_.]*\\6))?"
	    "\\s*\\?>"
	));
	QString strTmp = str.replace( textDecl, "" );
	if ( strTmp.length() != str.length() )
	    return FALSE; // external entity has wrong TextDecl
	str = strTmp;
    }
    return TRUE;
}


class QXmlAttributesPrivate
{
};
class QXmlInputSourcePrivate
{
};
class QXmlParseExceptionPrivate
{
};
class QXmlLocatorPrivate
{
};
class QXmlDefaultHandlerPrivate
{
};

/*!
    \class QXmlParseException qxml.h
    \reentrant
    \brief The QXmlParseException class is used to report errors with
    the QXmlErrorHandler interface.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module XML
    \ingroup xml-tools

    The XML subsystem constructs an instance of this class when it
    detects an error. You can retrieve the place where the error
    occurred using systemId(), publicId(), lineNumber() and
    columnNumber(), along with the error message().

    \sa QXmlErrorHandler QXmlReader
*/

/*!
    \fn QXmlParseException::QXmlParseException( const QString& name, int c, int l, const QString& p, const QString& s )

    Constructs a parse exception with the error string \a name for
    column \a c and line \a l for the public identifier \a p and the
    system identifier \a s.
*/

/*!
    Returns the error message.
*/
QString QXmlParseException::message() const
{
    return msg;
}
/*!
    Returns the column number where the error occurred.
*/
int QXmlParseException::columnNumber() const
{
    return column;
}
/*!
    Returns the line number where the error occurred.
*/
int QXmlParseException::lineNumber() const
{
    return line;
}
/*!
    Returns the public identifier where the error occurred.
*/
QString QXmlParseException::publicId() const
{
    return pub;
}
/*!
    Returns the system identifier where the error occurred.
*/
QString QXmlParseException::systemId() const
{
    return sys;
}


/*!
    \class QXmlLocator qxml.h
    \reentrant
    \brief The QXmlLocator class provides the XML handler classes with
    information about the parsing position within a file.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module XML
    \ingroup xml-tools

    The reader reports a QXmlLocator to the content handler before it
    starts to parse the document. This is done with the
    QXmlContentHandler::setDocumentLocator() function. The handler
    classes can now use this locator to get the position (lineNumber()
    and columnNumber()) that the reader has reached.
*/

/*!
    Constructor.
*/
QXmlLocator::QXmlLocator()
{
}

/*!
    Destructor.
*/
QXmlLocator::~QXmlLocator()
{
}

/*!
    \fn int QXmlLocator::columnNumber()

    Returns the column number (starting at 1) or -1 if there is no
    column number available.
*/

/*!
    \fn int QXmlLocator::lineNumber()

    Returns the line number (starting at 1) or -1 if there is no line
    number available.
*/

class QXmlSimpleReaderLocator : public QXmlLocator
{
public:
    QXmlSimpleReaderLocator( QXmlSimpleReader* parent )
    {
	reader = parent;
    }
    ~QXmlSimpleReaderLocator()
    {
    }

    int columnNumber()
    {
	return ( reader->columnNr == -1 ? -1 : reader->columnNr + 1 );
    }
    int lineNumber()
    {
	return ( reader->lineNr == -1 ? -1 : reader->lineNr + 1 );
    }
//    QString getPublicId()
//    QString getSystemId()

private:
    QXmlSimpleReader *reader;
};

/*********************************************
 *
 * QXmlNamespaceSupport
 *
 *********************************************/

typedef QMap<QString, QString> NamespaceMap;

class QXmlNamespaceSupportPrivate
{
public:
    QXmlNamespaceSupportPrivate()
    {
	ns.insert( "xml", "http://www.w3.org/XML/1998/namespace" ); // the XML namespace
    }

    ~QXmlNamespaceSupportPrivate()
    {
    }

    QValueStack<NamespaceMap> nsStack;
    NamespaceMap ns;
};

/*!
    \class QXmlNamespaceSupport qxml.h
    \reentrant
    \brief The QXmlNamespaceSupport class is a helper class for XML
    readers which want to include namespace support.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module XML
    \ingroup xml-tools

    You can set the prefix for the current namespace with setPrefix(),
    and get the list of current prefixes (or those for a given URI)
    with prefixes(). The namespace URI is available from uri(). Use
    pushContext() to start a new namespace context, and popContext()
    to return to the previous namespace context. Use splitName() or
    processName() to split a name into its prefix and local name.

    See also the \link xml.html#sax2Namespaces namespace description\endlink.
*/

/*!
    Constructs a QXmlNamespaceSupport.
*/
QXmlNamespaceSupport::QXmlNamespaceSupport()
{
    d = new QXmlNamespaceSupportPrivate;
}

/*!
    Destroys a QXmlNamespaceSupport.
*/
QXmlNamespaceSupport::~QXmlNamespaceSupport()
{
    delete d;
}

/*!
    This function declares a prefix \a pre in the current namespace
    context to be the namespace URI \a uri. The prefix remains in
    force until this context is popped, unless it is shadowed in a
    descendant context.

    Note that there is an asymmetry in this library. prefix() does not
    return the default "" prefix, even if you have declared one; to
    check for a default prefix, you must look it up explicitly using
    uri(). This asymmetry exists to make it easier to look up prefixes
    for attribute names, where the default prefix is not allowed.
*/
void QXmlNamespaceSupport::setPrefix( const QString& pre, const QString& uri )
{
    if( pre.isNull() ) {
	d->ns.insert( "", uri );
    } else {
	d->ns.insert( pre, uri );
    }
}

/*!
    Returns one of the prefixes mapped to the namespace URI \a uri.

    If more than one prefix is currently mapped to the same URI, this
    function makes an arbitrary selection; if you want all of the
    prefixes, use prefixes() instead.

    Note: to check for a default prefix, use the uri() function with
    an argument of "".
*/
QString QXmlNamespaceSupport::prefix( const QString& uri ) const
{
    NamespaceMap::const_iterator itc, it = d->ns.constBegin();
    while ( (itc=it) != d->ns.constEnd() ) {
	++it;
	if ( itc.data() == uri && !itc.key().isEmpty() )
	    return itc.key();
    }
    return "";
}

/*!
    Looks up the prefix \a prefix in the current context and returns
    the currently-mapped namespace URI. Use the empty string ("") for
    the default namespace.
*/
QString QXmlNamespaceSupport::uri( const QString& prefix ) const
{
    return d->ns[prefix];
}

/*!
    Splits the name \a qname at the ':' and returns the prefix in \a
    prefix and the local name in \a localname.

    \sa processName()
*/
void QXmlNamespaceSupport::splitName( const QString& qname,
	QString& prefix, QString& localname ) const
{
    int pos = qname.find(':');
    if (pos == -1)
        pos = qname.length();

    prefix = qname.left( pos );
    localname = qname.mid( pos+1 );
}

/*!
    Processes a raw XML 1.0 name in the current context by removing
    the prefix and looking it up among the prefixes currently
    declared.

    \a qname is the raw XML 1.0 name to be processed. \a isAttribute
    is TRUE if the name is an attribute name.

    This function stores the namespace URI in \a nsuri (which will be
    set to QString::null if the raw name has an undeclared prefix),
    and stores the local name (without prefix) in \a localname (which
    will be set to QString::null if no namespace is in use).

    Note that attribute names are processed differently than element
    names: an unprefixed element name gets the default namespace (if
    any), while an unprefixed element name does not.
*/
void QXmlNamespaceSupport::processName( const QString& qname,
	bool isAttribute,
	QString& nsuri, QString& localname ) const
{
    int len = qname.length();
    const QChar *data = qname.unicode();
    for (int pos = 0; pos < len; ++pos) {
        if (data[pos].unicode() == ':') {
            nsuri = uri(qname.left(pos));
            localname = qname.mid(pos + 1);
            return;
	}
    }
    // there was no ':'
    nsuri = QString::null;
    // attributes don't take default namespace
    if (!isAttribute && !d->ns.isEmpty()) {
        /*
            We want to access d->ns.value(""), but as an optimization
            we use the fact that "" compares less than any other
            string, so it's either first in the map or not there.
        */
        NamespaceMap::const_iterator first = d->ns.constBegin();
        if (first.key().isEmpty())
            nsuri = first.data(); // get default namespace
    }
    localname = qname;
}

/*!
    Returns a list of all the prefixes currently declared.

    If there is a default prefix, this function does not return it in
    the list; check for the default prefix using uri() with an
    argument of "".

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QStringList list = myXmlNamespaceSupport.prefixes();
    QStringList::iterator it = list.begin();
    while ( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode
*/
QStringList QXmlNamespaceSupport::prefixes() const
{
    QStringList list;

    NamespaceMap::const_iterator itc, it = d->ns.constBegin();
    while ( (itc=it) != d->ns.constEnd() ) {
	++it;
	if ( !itc.key().isEmpty() )
	    list.append( itc.key() );
    }
    return list;
}

/*!
    \overload

    Returns a list of all prefixes currently declared for the
    namespace URI \a uri.

    The "xml:" prefix is included. If you only want one prefix that is
    mapped to the namespace URI, and you don't care which one you get,
    use the prefix() function instead.

    Note: the empty (default) prefix is never included in this list;
    to check for the presence of a default namespace, use uri() with
    an argument of "".

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QStringList list = myXmlNamespaceSupport.prefixes( "" );
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode
*/
QStringList QXmlNamespaceSupport::prefixes( const QString& uri ) const
{
    QStringList list;

    NamespaceMap::const_iterator itc, it = d->ns.constBegin();
    while ( (itc=it) != d->ns.constEnd() ) {
	++it;
	if ( itc.data() == uri && !itc.key().isEmpty() )
	    list.append( itc.key() );
    }
    return list;
}

/*!
    Starts a new namespace context.

    Normally, you should push a new context at the beginning of each
    XML element: the new context automatically inherits the
    declarations of its parent context, and it also keeps track of
    which declarations were made within this context.

    \sa popContext()
*/
void QXmlNamespaceSupport::pushContext()
{
    d->nsStack.push(d->ns);
}

/*!
    Reverts to the previous namespace context.

    Normally, you should pop the context at the end of each XML
    element. After popping the context, all namespace prefix mappings
    that were previously in force are restored.

    \sa pushContext()
*/
void QXmlNamespaceSupport::popContext()
{
    d->ns.clear();
    if( !d->nsStack.isEmpty() )
	d->ns = d->nsStack.pop();
}

/*!
    Resets this namespace support object ready for reuse.
*/
void QXmlNamespaceSupport::reset()
{
    delete d;
    d = new QXmlNamespaceSupportPrivate;
}



/*********************************************
 *
 * QXmlAttributes
 *
 *********************************************/

/*!
    \class QXmlAttributes qxml.h
    \reentrant
    \brief The QXmlAttributes class provides XML attributes.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module XML
    \ingroup xml-tools

    If attributes are reported by QXmlContentHandler::startElement()
    this class is used to pass the attribute values.

    Use index() to locate the position of an attribute in the list,
    count() to retrieve the number of attributes, and clear() to
    remove the attributes. New attributes can be added with append().
    Use type() to get an attribute's type and value() to get its
    value. The attribute's name is available from localName() or
    qName(), and its namespace URI from uri().

*/

/*!
    \fn QXmlAttributes::QXmlAttributes()

    Constructs an empty attribute list.
*/

/*!
    \fn QXmlAttributes::~QXmlAttributes()

    Destroys the attributes object.
*/

/*!
    Looks up the index of an attribute by the qualified name \a qName.

    Returns the index of the attribute or -1 if it wasn't found.

    See also the \link xml.html#sax2Namespaces namespace description\endlink.
*/
int QXmlAttributes::index( const QString& qName ) const
{
    return qnameList.findIndex( qName );
}

/*!
    \overload

    Looks up the index of an attribute by a namespace name.

    \a uri specifies the namespace URI, or an empty string if the name
    has no namespace URI. \a localPart specifies the attribute's local
    name.

    Returns the index of the attribute, or -1 if it wasn't found.

    See also the \link xml.html#sax2Namespaces namespace description\endlink.
*/
int QXmlAttributes::index( const QString& uri, const QString& localPart ) const
{
    QString uriTmp;
    if ( uri.isEmpty() )
       uriTmp = QString::null;
    else
       uriTmp = uri;
    uint count = (uint)uriList.count(); // ### size_t/int cast
    for ( uint i=0; i<count; i++ ) {
       if ( uriList[i] == uriTmp && localnameList[i] == localPart )
           return i;
    }
    return -1;
}

/*!
    Returns the number of attributes in the list.

    \sa count()
*/
int QXmlAttributes::length() const
{
    return (int)valueList.count();
}

/*!
    \fn int QXmlAttributes::count() const

    Returns the number of attributes in the list. This function is
    equivalent to length().
*/

/*!
    Looks up an attribute's local name for the attribute at position
    \a index. If no namespace processing is done, the local name is
    QString::null.

    See also the \link xml.html#sax2Namespaces namespace description\endlink.
*/
QString QXmlAttributes::localName( int index ) const
{
    return localnameList[index];
}

/*!
    Looks up an attribute's XML 1.0 qualified name for the attribute
    at position \a index.

    See also the \link xml.html#sax2Namespaces namespace description\endlink.
*/
QString QXmlAttributes::qName( int index ) const
{
    return qnameList[index];
}

/*!
    Looks up an attribute's namespace URI for the attribute at
    position \a index. If no namespace processing is done or if the
    attribute has no namespace, the namespace URI is QString::null.

    See also the \link xml.html#sax2Namespaces namespace description\endlink.
*/
QString QXmlAttributes::uri( int index ) const
{
    return uriList[index];
}

/*!
    Looks up an attribute's type for the attribute at position \a
    index.

    Currently only "CDATA" is returned.
*/
QString QXmlAttributes::type( int ) const
{
    return "CDATA";
}

/*!
    \overload

    Looks up an attribute's type for the qualified name \a qName.

    Currently only "CDATA" is returned.
*/
QString QXmlAttributes::type( const QString& ) const
{
    return "CDATA";
}

/*!
    \overload

    Looks up an attribute's type by namespace name.

    \a uri specifies the namespace URI and \a localName specifies the
    local name. If the name has no namespace URI, use an empty string
    for \a uri.

    Currently only "CDATA" is returned.
*/
QString QXmlAttributes::type( const QString&, const QString& ) const
{
    return "CDATA";
}

/*!
    Looks up an attribute's value for the attribute at position \a
    index.
*/
QString QXmlAttributes::value( int index ) const
{
    return valueList[index];
}

/*!
    \overload

    Looks up an attribute's value for the qualified name \a qName.

    See also the \link xml.html#sax2Namespaces namespace description\endlink.
*/
QString QXmlAttributes::value( const QString& qName ) const
{
    int i = index( qName );
    if ( i == -1 )
	return QString::null;
    return valueList[ i ];
}

/*!
    \overload

    Looks up an attribute's value by namespace name.

    \a uri specifies the namespace URI, or an empty string if the name
    has no namespace URI. \a localName specifies the attribute's local
    name.

    See also the \link xml.html#sax2Namespaces namespace description\endlink.
*/
QString QXmlAttributes::value( const QString& uri, const QString& localName ) const
{
    int i = index( uri, localName );
    if ( i == -1 )
	return QString::null;
    return valueList[ i ];
}

/*!
    Clears the list of attributes.

    \sa append()
*/
void QXmlAttributes::clear()
{
    qnameList.clear();
    uriList.clear();
    localnameList.clear();
    valueList.clear();
}

/*!
    Appends a new attribute entry to the list of attributes. The
    qualified name of the attribute is \a qName, the namespace URI is
    \a uri and the local name is \a localPart. The value of the
    attribute is \a value.

    \sa qName() uri() localName() value()
*/
void QXmlAttributes::append( const QString &qName, const QString &uri, const QString &localPart, const QString &value )
{
    qnameList.append( qName );
    uriList.append( uri );
    localnameList.append( localPart);
    valueList.append( value );
}


/*********************************************
 *
 * QXmlInputSource
 *
 *********************************************/

/*!
    \class QXmlInputSource qxml.h
    \reentrant
    \brief The QXmlInputSource class provides the input data for the
    QXmlReader subclasses.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module XML
    \ingroup xml-tools

    All subclasses of QXmlReader read the input XML document from this
    class.

    This class recognizes the encoding of the data by reading the
    encoding declaration in the XML file if it finds one, and reading
    the data using the corresponding encoding. If it does not find an
    encoding declaration, then it assumes that the data is either in
    UTF-8 or UTF-16, depending on whether it can find a byte-order
    mark.

    There are two ways to populate the input source with data: you can
    construct it with a QIODevice* so that the input source reads the
    data from that device. Or you can set the data explicitly with one
    of the setData() functions.

    Usually you either construct a QXmlInputSource that works on a
    QIODevice* or you construct an empty QXmlInputSource and set the
    data with setData(). There are only rare occasions where you would
    want to mix both methods.

    The QXmlReader subclasses use the next() function to read the
    input character by character. If you want to start from the
    beginning again, use reset().

    The functions data() and fetchData() are useful if you want to do
    something with the data other than parsing, e.g. displaying the
    raw XML file. The benefit of using the QXmlInputClass in such
    cases is that it tries to use the correct encoding.

    \sa QXmlReader QXmlSimpleReader
*/

// the following two are guaranteed not to be a character
const QChar QXmlInputSource::EndOfData = QChar((ushort)0xfffe);
const QChar QXmlInputSource::EndOfDocument = QChar((ushort)0xffff);

/*
    Common part of the constructors.
*/
void QXmlInputSource::init()
{
    inputDevice = 0;
    inputStream = 0;

    setData( QString::null );
    encMapper = 0;
}

/*!
    Constructs an input source which contains no data.

    \sa setData()
*/
QXmlInputSource::QXmlInputSource()
{
    init();
}

/*!
    Constructs an input source and gets the data from device \a dev.
    If \a dev is not open, it is opened in read-only mode. If \a dev
    is 0 or it is not possible to read from the device, the input
    source will contain no data.

    \sa setData() fetchData() QIODevice
*/
QXmlInputSource::QXmlInputSource( QIODevice *dev )
{
    init();
    inputDevice = dev;
    fetchData();
}

/*! \obsolete
  Constructs an input source and gets the data from the text stream \a stream.
*/
QXmlInputSource::QXmlInputSource( QTextStream& stream )
{
    init();
    inputStream = &stream;
    fetchData();
}

/*! \obsolete
  Constructs an input source and gets the data from the file \a file. If the
  file cannot be read the input source is empty.
*/
QXmlInputSource::QXmlInputSource( QFile& file )
{
    init();
    inputDevice = &file;
    fetchData();
}

/*!
    Destructor.
*/
QXmlInputSource::~QXmlInputSource()
{
    delete encMapper;
}

/*!
    Returns the next character of the input source. If this function
    reaches the end of available data, it returns
    QXmlInputSource::EndOfData. If you call next() after that, it
    tries to fetch more data by calling fetchData(). If the
    fetchData() call results in new data, this function returns the
    first character of that data; otherwise it returns
    QXmlInputSource::EndOfDocument.

    \sa reset() fetchData() QXmlSimpleReader::parse() QXmlSimpleReader::parseContinue()
*/
QChar QXmlInputSource::next()
{
    if ( pos >= length ) {
	if ( nextReturnedEndOfData ) {
	    nextReturnedEndOfData = FALSE;
	    fetchData();
	    if ( pos >= length ) {
		return EndOfDocument;
	    }
	    return next();
	}
	nextReturnedEndOfData = TRUE;
	return EndOfData;
    }
    return unicode[pos++];
}

/*!
    This function sets the position used by next() to the beginning of
    the data returned by data(). This is useful if you want to use the
    input source for more than one parse.

    \sa next()
*/
void QXmlInputSource::reset()
{
    nextReturnedEndOfData = FALSE;
    pos = 0;
}

/*!
    Returns the data the input source contains or QString::null if the
    input source does not contain any data.

    \sa setData() QXmlInputSource() fetchData()
*/
QString QXmlInputSource::data()
{
    return str;
}

/*!
    Sets the data of the input source to \a dat.

    If the input source already contains data, this function deletes
    that data first.

    \sa data()
*/
void QXmlInputSource::setData( const QString& dat )
{
    str = dat;
    unicode = str.unicode();
    pos = 0;
    length = str.length();
    nextReturnedEndOfData = FALSE;
}

/*!
    \overload

    The data \a dat is passed through the correct text-codec, before
    it is set.
*/
void QXmlInputSource::setData( const QByteArray& dat )
{
    setData( fromRawData( dat ) );
}

/*!
    This function reads more data from the device that was set during
    construction. If the input source already contained data, this
    function deletes that data first.

    This object contains no data after a call to this function if the
    object was constructed without a device to read data from or if
    this function was not able to get more data from the device.

    There are two occasions where a fetch is done implicitly by
    another function call: during construction (so that the object
    starts out with some initial data where available), and during a
    call to next() (if the data had run out).

    You don't normally need to use this function if you use next().

    \sa data() next() QXmlInputSource()
*/
void QXmlInputSource::fetchData()
{
    QByteArray rawData;

    if ( inputDevice != 0 ) {
	if ( inputDevice->isOpen() || inputDevice->open( IO_ReadOnly )  )
	    rawData = inputDevice->readAll();
    } else if ( inputStream != 0 ) {
	if ( inputStream->device()->isDirectAccess() ) {
	    rawData = inputStream->device()->readAll();
	} else {
	    int nread = 0;
	    const int bufsize = 512;
	    while ( !inputStream->device()->atEnd() ) {
		rawData.resize( nread + bufsize );
		nread += inputStream->device()->readBlock( rawData.data()+nread, bufsize );
	    }
	    rawData.resize( nread );
	}
    }
    setData( fromRawData( rawData ) );
}

/*!
    This function reads the XML file from \a data and tries to
    recognize the encoding. It converts the raw data \a data into a
    QString and returns it. It tries its best to get the correct
    encoding for the XML file.

    If \a beginning is TRUE, this function assumes that the data
    starts at the beginning of a new XML document and looks for an
    encoding declaration. If \a beginning is FALSE, it converts the
    raw data using the encoding determined from prior calls.
*/
QString QXmlInputSource::fromRawData( const QByteArray &data, bool beginning )
{
    if ( data.size() == 0 )
	return QString::null;
    if ( beginning ) {
	delete encMapper;
	encMapper = 0;
    }
    if ( encMapper == 0 ) {
	QTextCodec *codec = 0;
	// look for byte order mark and read the first 5 characters
	if ( data.size() >= 2 &&
		( ((uchar)data.at(0)==(uchar)0xfe &&
		   (uchar)data.at(1)==(uchar)0xff ) ||
		  ((uchar)data.at(0)==(uchar)0xff &&
		   (uchar)data.at(1)==(uchar)0xfe ) )) {
	    codec = QTextCodec::codecForMib( 1000 ); // UTF-16
	} else {
	    codec = QTextCodec::codecForMib( 106 ); // UTF-8
	}
	if ( !codec )
	    return QString::null;

	encMapper = codec->makeDecoder();
	QString input = encMapper->toUnicode( data.data(), data.size() );
	// ### unexpected EOF? (for incremental parsing)
	// starts the document with an XML declaration?
	if ( input.find("<?xml") == 0 ) {
	    // try to find out if there is an encoding
	    int endPos = input.find( ">" );
	    int pos = input.find( "encoding" );
	    if ( pos < endPos && pos != -1 ) {
		QString encoding;
		do {
		    pos++;
		    if ( pos > endPos ) {
			return input;
		    }
		} while( input[pos] != '"' && input[pos] != '\'' );
		pos++;
		while( input[pos] != '"' && input[pos] != '\'' ) {
		    encoding += input[pos];
		    pos++;
		    if ( pos > endPos ) {
			return input;
		    }
		}

		codec = QTextCodec::codecForName( encoding );
		if ( codec == 0 ) {
		    return input;
		}
		delete encMapper;
		encMapper = codec->makeDecoder();
		return encMapper->toUnicode( data.data(), data.size() );
	    }
	}
	return input;
    }
    return encMapper->toUnicode( data.data(), data.size() );
}


/*********************************************
 *
 * QXmlDefaultHandler
 *
 *********************************************/

/*!
    \class QXmlContentHandler qxml.h
    \reentrant
    \brief The QXmlContentHandler class provides an interface to
    report the logical content of XML data.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module XML
    \ingroup xml-tools

    If the application needs to be informed of basic parsing events,
    it can implement this interface and activate it using
    QXmlReader::setContentHandler(). The reader can then report basic
    document-related events like the start and end of elements and
    character data through this interface.

    The order of events in this interface is very important, and
    mirrors the order of information in the document itself. For
    example, all of an element's content (character data, processing
    instructions, and sub-elements) appears, in order, between the
    startElement() event and the corresponding endElement() event.

    The class QXmlDefaultHandler provides a default implementation for
    this interface; subclassing from the QXmlDefaultHandler class is
    very convenient if you only want to be informed of some parsing
    events.

    The startDocument() function is called at the start of the
    document, and endDocument() is called at the end. Before parsing
    begins setDocumentLocator() is called. For each element
    startElement() is called, with endElement() being called at the
    end of each element. The characters() function is called with
    chunks of character data; ignorableWhitespace() is called with
    chunks of whitespace and processingInstruction() is called with
    processing instructions. If an entity is skipped skippedEntity()
    is called. At the beginning of prefix-URI scopes
    startPrefixMapping() is called.

    See also the \link xml.html#sax2Intro Introduction to SAX2\endlink.

    \sa QXmlDTDHandler QXmlDeclHandler QXmlEntityResolver QXmlErrorHandler
    QXmlLexicalHandler
*/

/*!
    \fn void QXmlContentHandler::setDocumentLocator( QXmlLocator* locator )

    The reader calls this function before it starts parsing the
    document. The argument \a locator is a pointer to a QXmlLocator
    which allows the application to get the parsing position within
    the document.

    Do not destroy the \a locator; it is destroyed when the reader is
    destroyed. (Do not use the \a locator after the reader is
    destroyed).
*/

/*!
    \fn bool QXmlContentHandler::startDocument()

    The reader calls this function when it starts parsing the
    document. The reader calls this function just once, after the call
    to setDocumentLocator(), and before any other functions in this
    class or in the QXmlDTDHandler class are called.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.

    \sa endDocument()
*/

/*!
    \fn bool QXmlContentHandler::endDocument()

    The reader calls this function after it has finished parsing. It
    is called just once, and is the last handler function called. It
    is called after the reader has read all input or has abandoned
    parsing because of a fatal error.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.

    \sa startDocument()
*/

/*!
    \fn bool QXmlContentHandler::startPrefixMapping( const QString& prefix, const QString& uri )

    The reader calls this function to signal the begin of a prefix-URI
    namespace mapping scope. This information is not necessary for
    normal namespace processing since the reader automatically
    replaces prefixes for element and attribute names.

    Note that startPrefixMapping() and endPrefixMapping() calls are
    not guaranteed to be properly nested relative to each other: all
    startPrefixMapping() events occur before the corresponding
    startElement() event, and all endPrefixMapping() events occur
    after the corresponding endElement() event, but their order is not
    otherwise guaranteed.

    The argument \a prefix is the namespace prefix being declared and
    the argument \a uri is the namespace URI the prefix is mapped to.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.

    See also the \link xml.html#sax2Namespaces namespace description\endlink.

    \sa endPrefixMapping()
*/

/*!
    \fn bool QXmlContentHandler::endPrefixMapping( const QString& prefix )

    The reader calls this function to signal the end of a prefix
    mapping for the prefix \a prefix.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.

    See also the \link xml.html#sax2Namespaces namespace description\endlink.

    \sa startPrefixMapping()
*/

/*!
    \fn bool QXmlContentHandler::startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts )

    The reader calls this function when it has parsed a start element
    tag.

    There is a corresponding endElement() call when the corresponding
    end element tag is read. The startElement() and endElement() calls
    are always nested correctly. Empty element tags (e.g. \c{<x/>})
    cause a startElement() call to be immediately followed by an
    endElement() call.

    The attribute list provided only contains attributes with explicit
    values. The attribute list contains attributes used for namespace
    declaration (i.e. attributes starting with xmlns) only if the
    namespace-prefix property of the reader is TRUE.

    The argument \a namespaceURI is the namespace URI, or
    QString::null if the element has no namespace URI or if no
    namespace processing is done. \a localName is the local name
    (without prefix), or QString::null if no namespace processing is
    done, \a qName is the qualified name (with prefix) and \a atts are
    the attributes attached to the element. If there are no
    attributes, \a atts is an empty attributes object.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.

    See also the \link xml.html#sax2Namespaces namespace description\endlink.

    \sa endElement()
*/

/*!
    \fn bool QXmlContentHandler::endElement( const QString& namespaceURI, const QString& localName, const QString& qName )

    The reader calls this function when it has parsed an end element
    tag with the qualified name \a qName, the local name \a localName
    and the namespace URI \a namespaceURI.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.

    See also the \link xml.html#sax2Namespaces namespace description\endlink.

    \sa startElement()
*/

/*!
    \fn bool QXmlContentHandler::characters( const QString& ch )

    The reader calls this function when it has parsed a chunk of
    character data (either normal character data or character data
    inside a CDATA section; if you need to distinguish between those
    two types you must use QXmlLexicalHandler::startCDATA() and
    QXmlLexicalHandler::endCDATA()). The character data is reported in
    \a ch.

    Some readers report whitespace in element content using the
    ignorableWhitespace() function rather than using this one.

    A reader may report the character data of an element in more than
    one chunk; e.g. a reader might want to report "a\<b" in three
    characters() events ("a ", "\<" and " b").

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.
*/

/*!
    \fn bool QXmlContentHandler::ignorableWhitespace( const QString& ch )

    Some readers may use this function to report each chunk of
    whitespace in element content. The whitespace is reported in \a ch.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.
*/

/*!
    \fn bool QXmlContentHandler::processingInstruction( const QString& target, const QString& data )

    The reader calls this function when it has parsed a processing
    instruction.

    \a target is the target name of the processing instruction and \a
    data is the data in the processing instruction.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.
*/

/*!
    \fn bool QXmlContentHandler::skippedEntity( const QString& name )

    Some readers may skip entities if they have not seen the
    declarations (e.g. because they are in an external DTD). If they
    do so they report that they skipped the entity called \a name by
    calling this function.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.
*/

/*!
    \fn QString QXmlContentHandler::errorString()

    The reader calls this function to get an error string, e.g. if any
    of the handler functions returns FALSE.
*/


/*!
    \class QXmlErrorHandler qxml.h
    \reentrant
    \brief The QXmlErrorHandler class provides an interface to report
    errors in XML data.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module XML
    \ingroup xml-tools

    If you want your application to report errors to the user or to
    perform customized error handling, you should subclass this class.

    You can set the error handler with QXmlReader::setErrorHandler().

    Errors can be reported using warning(), error() and fatalError(),
    with the error text being reported with errorString().

    See also the \link xml.html#sax2Intro Introduction to SAX2\endlink.

    \sa QXmlDTDHandler QXmlDeclHandler QXmlContentHandler QXmlEntityResolver
    QXmlLexicalHandler
*/

/*!
    \fn bool QXmlErrorHandler::warning( const QXmlParseException& exception )

    A reader might use this function to report a warning. Warnings are
    conditions that are not errors or fatal errors as defined by the
    XML 1.0 specification. Details of the warning are stored in \a
    exception.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.
*/

/*!
    \fn bool QXmlErrorHandler::error( const QXmlParseException& exception )

    A reader might use this function to report a recoverable error. A
    recoverable error corresponds to the definiton of "error" in
    section 1.2 of the XML 1.0 specification. Details of the error are
    stored in \a exception.

    The reader must continue to provide normal parsing events after
    invoking this function.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.
*/

/*!
    \fn bool QXmlErrorHandler::fatalError( const QXmlParseException& exception )

    A reader must use this function to report a non-recoverable error.
    Details of the error are stored in \a exception.

    If this function returns TRUE the reader might try to go on
    parsing and reporting further errors; but no regular parsing
    events are reported.
*/

/*!
    \fn QString QXmlErrorHandler::errorString()

    The reader calls this function to get an error string if any of
    the handler functions returns FALSE.
*/


/*!
    \class QXmlDTDHandler qxml.h
    \reentrant
    \brief The QXmlDTDHandler class provides an interface to report
    DTD content of XML data.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module XML
    \ingroup xml-tools

    If an application needs information about notations and unparsed
    entities, it can implement this interface and register an instance
    with QXmlReader::setDTDHandler().

    Note that this interface includes only those DTD events that the
    XML recommendation requires processors to report, i.e. notation
    and unparsed entity declarations using notationDecl() and
    unparsedEntityDecl() respectively.

    See also the \link xml.html#sax2Intro Introduction to SAX2\endlink.

    \sa QXmlDeclHandler QXmlContentHandler QXmlEntityResolver QXmlErrorHandler
    QXmlLexicalHandler
*/

/*!
    \fn bool QXmlDTDHandler::notationDecl( const QString& name, const QString& publicId, const QString& systemId )

    The reader calls this function when it has parsed a notation
    declaration.

    The argument \a name is the notation name, \a publicId is the
    notation's public identifier and \a systemId is the notation's
    system identifier.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.
*/

/*!
    \fn bool QXmlDTDHandler::unparsedEntityDecl( const QString& name, const QString& publicId, const QString& systemId, const QString& notationName )

    The reader calls this function when it finds an unparsed entity
    declaration.

    The argument \a name is the unparsed entity's name, \a publicId is
    the entity's public identifier, \a systemId is the entity's system
    identifier and \a notationName is the name of the associated
    notation.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.
*/

/*!
    \fn QString QXmlDTDHandler::errorString()

    The reader calls this function to get an error string if any of
    the handler functions returns FALSE.
*/


/*!
    \class QXmlEntityResolver qxml.h
    \reentrant
    \brief The QXmlEntityResolver class provides an interface to
    resolve external entities contained in XML data.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module XML
    \ingroup xml-tools

    If an application needs to implement customized handling for
    external entities, it must implement this interface, i.e.
    resolveEntity(), and register it with
    QXmlReader::setEntityResolver().

    See also the \link xml.html#sax2Intro Introduction to SAX2\endlink.

    \sa QXmlDTDHandler QXmlDeclHandler QXmlContentHandler QXmlErrorHandler
    QXmlLexicalHandler
*/

/*!
    \fn bool QXmlEntityResolver::resolveEntity( const QString& publicId, const QString& systemId, QXmlInputSource*& ret )

    The reader calls this function before it opens any external
    entity, except the top-level document entity. The application may
    request the reader to resolve the entity itself (\a ret is 0) or
    to use an entirely different input source (\a ret points to the
    input source).

    The reader deletes the input source \a ret when it no longer needs
    it, so you should allocate it on the heap with \c new.

    The argument \a publicId is the public identifier of the external
    entity, \a systemId is the system identifier of the external
    entity and \a ret is the return value of this function. If \a ret
    is 0 the reader should resolve the entity itself, if it is
    non-zero it must point to an input source which the reader uses
    instead.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.
*/

/*!
    \fn QString QXmlEntityResolver::errorString()

    The reader calls this function to get an error string if any of
    the handler functions returns FALSE.
*/


/*!
    \class QXmlLexicalHandler qxml.h
    \reentrant
    \brief The QXmlLexicalHandler class provides an interface to
    report the lexical content of XML data.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module XML
    \ingroup xml-tools

    The events in the lexical handler apply to the entire document,
    not just to the document element, and all lexical handler events
    appear between the content handler's startDocument and endDocument
    events.

    You can set the lexical handler with
    QXmlReader::setLexicalHandler().

    This interface's design is based on the the SAX2 extension
    LexicalHandler.

    The interface provides the startDTD(), endDTD(), startEntity(),
    endEntity(), startCDATA(), endCDATA() and comment() functions.

    See also the \link xml.html#sax2Intro Introduction to SAX2\endlink.

    \sa QXmlDTDHandler QXmlDeclHandler QXmlContentHandler QXmlEntityResolver
    QXmlErrorHandler
*/

/*!
    \fn bool QXmlLexicalHandler::startDTD( const QString& name, const QString& publicId, const QString& systemId )

    The reader calls this function to report the start of a DTD
    declaration, if any. It reports the name of the document type in
    \a name, the public identifier in \a publicId and the system
    identifier in \a systemId.

    If the public identifier is missing, \a publicId is set to
    QString::null. If the system identifier is missing, \a systemId is
    set to QString::null. Note that it is not valid XML to have a
    public identifier but no system identifier; in such cases a parse
    error will occur.

    All declarations reported through QXmlDTDHandler or
    QXmlDeclHandler appear between the startDTD() and endDTD() calls.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.

    \sa endDTD()
*/

/*!
    \fn bool QXmlLexicalHandler::endDTD()

    The reader calls this function to report the end of a DTD
    declaration, if any.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.

    \sa startDTD()
*/

/*!
    \fn bool QXmlLexicalHandler::startEntity( const QString& name )

    The reader calls this function to report the start of an entity
    called \a name.

    Note that if the entity is unknown, the reader reports it through
    QXmlContentHandler::skippedEntity() and not through this
    function.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.

    \sa endEntity() QXmlSimpleReader::setFeature()
*/

/*!
    \fn bool QXmlLexicalHandler::endEntity( const QString& name )

    The reader calls this function to report the end of an entity
    called \a name.

    For every startEntity() call, there is a corresponding endEntity()
    call. The calls to startEntity() and endEntity() are properly
    nested.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.

    \sa startEntity() QXmlContentHandler::skippedEntity() QXmlSimpleReader::setFeature()
*/

/*!
    \fn bool QXmlLexicalHandler::startCDATA()

    The reader calls this function to report the start of a CDATA
    section. The content of the CDATA section is reported through the
    QXmlContentHandler::characters() function. This function is
    intended only to report the boundary.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.

    \sa endCDATA()
*/

/*!
    \fn bool QXmlLexicalHandler::endCDATA()

    The reader calls this function to report the end of a CDATA
    section.

    If this function returns FALSE the reader stops parsing and reports
    an error. The reader uses the function errorString() to get the error
    message.

    \sa startCDATA() QXmlContentHandler::characters()
*/

/*!
    \fn bool QXmlLexicalHandler::comment( const QString& ch )

    The reader calls this function to report an XML comment anywhere
    in the document. It reports the text of the comment in \a ch.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.
*/

/*!
    \fn QString QXmlLexicalHandler::errorString()

    The reader calls this function to get an error string if any of
    the handler functions returns FALSE.
*/


/*!
    \class QXmlDeclHandler qxml.h
    \reentrant
    \brief The QXmlDeclHandler class provides an interface to report declaration
    content of XML data.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module XML
    \ingroup xml-tools

    You can set the declaration handler with
    QXmlReader::setDeclHandler().

    This interface is based on the SAX2 extension DeclHandler.

    The interface provides attributeDecl(), internalEntityDecl() and
    externalEntityDecl() functions.

    See also the \link xml.html#sax2Intro Introduction to SAX2\endlink.

    \sa QXmlDTDHandler QXmlContentHandler QXmlEntityResolver QXmlErrorHandler
    QXmlLexicalHandler
*/

/*!
    \fn bool QXmlDeclHandler::attributeDecl( const QString& eName, const QString& aName, const QString& type, const QString& valueDefault, const QString& value )

    The reader calls this function to report an attribute type
    declaration. Only the effective (first) declaration for an
    attribute is reported.

    The reader passes the name of the associated element in \a eName
    and the name of the attribute in \a aName. It passes a string that
    represents the attribute type in \a type and a string that
    represents the attribute default in \a valueDefault. This string
    is one of "#IMPLIED", "#REQUIRED", "#FIXED" or QString::null (if
    none of the others applies). The reader passes the attribute's
    default value in \a value. If no default value is specified in the
    XML file, \a value is QString::null.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.
*/

/*!
    \fn bool QXmlDeclHandler::internalEntityDecl( const QString& name, const QString& value )

    The reader calls this function to report an internal entity
    declaration. Only the effective (first) declaration is reported.

    The reader passes the name of the entity in \a name and the value
    of the entity in \a value.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.
*/

/*!
    \fn bool QXmlDeclHandler::externalEntityDecl( const QString& name, const QString& publicId, const QString& systemId )

    The reader calls this function to report a parsed external entity
    declaration. Only the effective (first) declaration for each
    entity is reported.

    The reader passes the name of the entity in \a name, the public
    identifier in \a publicId and the system identifier in \a
    systemId. If there is no public identifier specified, it passes
    QString::null in \a publicId.

    If this function returns FALSE the reader stops parsing and
    reports an error. The reader uses the function errorString() to
    get the error message.
*/

/*!
    \fn QString QXmlDeclHandler::errorString()

    The reader calls this function to get an error string if any of
    the handler functions returns FALSE.
*/


/*!
    \class QXmlDefaultHandler qxml.h
    \reentrant
    \brief The QXmlDefaultHandler class provides a default implementation of all
    the XML handler classes.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module XML
    \ingroup xml-tools

    Very often we are only interested in parts of the things that the
    reader reports. This class implements a default behaviour for the
    handler classes (i.e. most of the time do nothing). Usually this
    is the class you subclass for implementing your own customized
    handler.

    See also the \link xml.html#sax2Intro Introduction to SAX2\endlink.

    \sa QXmlDTDHandler QXmlDeclHandler QXmlContentHandler QXmlEntityResolver
    QXmlErrorHandler QXmlLexicalHandler
*/

/*!
    \fn QXmlDefaultHandler::QXmlDefaultHandler()

    Constructor.
*/
/*!
    \fn QXmlDefaultHandler::~QXmlDefaultHandler()

    Destructor.
*/

/*!
    \reimp

    Does nothing.
*/
void QXmlDefaultHandler::setDocumentLocator( QXmlLocator* )
{
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::startDocument()
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::endDocument()
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::startPrefixMapping( const QString&, const QString& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::endPrefixMapping( const QString& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::startElement( const QString&, const QString&,
	const QString&, const QXmlAttributes& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::endElement( const QString&, const QString&,
	const QString& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::characters( const QString& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::ignorableWhitespace( const QString& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::processingInstruction( const QString&,
	const QString& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::skippedEntity( const QString& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::warning( const QXmlParseException& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::error( const QXmlParseException& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::fatalError( const QXmlParseException& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::notationDecl( const QString&, const QString&,
	const QString& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::unparsedEntityDecl( const QString&, const QString&,
	const QString&, const QString& )
{
    return TRUE;
}

/*!
    \reimp

    Sets \a ret to 0, so that the reader uses the system identifier
    provided in the XML document.
*/
bool QXmlDefaultHandler::resolveEntity( const QString&, const QString&,
	QXmlInputSource*& ret )
{
    ret = 0;
    return TRUE;
}

/*!
    \reimp

    Returns the default error string.
*/
QString QXmlDefaultHandler::errorString()
{
    return QString( XMLERR_ERRORBYCONSUMER );
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::startDTD( const QString&, const QString&, const QString& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::endDTD()
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::startEntity( const QString& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::endEntity( const QString& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::startCDATA()
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::endCDATA()
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::comment( const QString& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::attributeDecl( const QString&, const QString&, const QString&, const QString&, const QString& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::internalEntityDecl( const QString&, const QString& )
{
    return TRUE;
}

/*!
    \reimp

    Does nothing.
*/
bool QXmlDefaultHandler::externalEntityDecl( const QString&, const QString&, const QString& )
{
    return TRUE;
}


/*********************************************
 *
 * QXmlSimpleReaderPrivate
 *
 *********************************************/

class QXmlSimpleReaderPrivate
{
private:
    // functions
    inline QXmlSimpleReaderPrivate()
    {
	parseStack = 0;
        undefEntityInAttrHack = FALSE;
    }

    inline ~QXmlSimpleReaderPrivate()
    {
	delete parseStack;
    }

    inline void initIncrementalParsing()
    {
	delete parseStack;
	parseStack = new QValueStack<ParseState>;
    }

    // used to determine if elements are correctly nested
    QValueStack<QString> tags;

    // used for entity declarations
    struct ExternParameterEntity
    {
	ExternParameterEntity( ) {}
	ExternParameterEntity( const QString &p, const QString &s )
	    : publicId(p), systemId(s) {}
	QString publicId;
	QString systemId;

	Q_DUMMY_COMPARISON_OPERATOR(ExternParameterEntity)
    };
    struct ExternEntity
    {
	ExternEntity( ) {}
	ExternEntity( const QString &p, const QString &s, const QString &n )
	    : publicId(p), systemId(s), notation(n) {}
	QString publicId;
	QString systemId;
	QString notation;
	Q_DUMMY_COMPARISON_OPERATOR(ExternEntity)
    };
    QMap<QString,ExternParameterEntity> externParameterEntities;
    QMap<QString,QString> parameterEntities;
    QMap<QString,ExternEntity> externEntities;
    QMap<QString,QString> entities;

    // used for parsing of entity references
    QValueStack<QString> xmlRef;
    QValueStack<QString> xmlRefName;

    // used for standalone declaration
    enum Standalone { Yes, No, Unknown };

    QString doctype; // only used for the doctype
    QString xmlVersion; // only used to store the version information
    QString encoding; // only used to store the encoding
    Standalone standalone; // used to store the value of the standalone declaration

    QString publicId; // used by parseExternalID() to store the public ID
    QString systemId; // used by parseExternalID() to store the system ID
    QString attDeclEName; // use by parseAttlistDecl()
    QString attDeclAName; // use by parseAttlistDecl()

    // flags for some features support
    bool useNamespaces;
    bool useNamespacePrefixes;
    bool reportWhitespaceCharData;
    bool reportEntities;

    // used to build the attribute list
    QXmlAttributes attList;

    // used in QXmlSimpleReader::parseContent() to decide whether character
    // data was read
    bool contentCharDataRead;

    // helper classes
    QXmlLocator *locator;
    QXmlNamespaceSupport namespaceSupport;

    // error string
    QString error;

    // arguments for parse functions (this is needed to allow incremental
    // parsing)
    bool parsePI_xmldecl;
    bool parseName_useRef;
    bool parseReference_charDataRead;
    QXmlSimpleReader::EntityRecognitionContext parseReference_context;
    bool parseExternalID_allowPublicID;
    QXmlSimpleReader::EntityRecognitionContext parsePEReference_context;
    QString parseString_s;

    // for incremental parsing
    struct ParseState {
	typedef bool (QXmlSimpleReader::*ParseFunction) ();
	ParseFunction function;
	int state;
    };
    QValueStack<ParseState> *parseStack;

    // used in parseProlog()
    bool xmldecl_possible;
    bool doctype_read;

    // used in parseDoctype()
    bool startDTDwasReported;

    // used in parseString()
    signed char Done;

    bool undefEntityInAttrHack;

    int nameValueLen;
    int refValueLen;
    int stringValueLen;

    // friend declarations
    friend class QXmlSimpleReader;
};


/*********************************************
 *
 * QXmlSimpleReader
 *
 *********************************************/

/*!
    \class QXmlReader qxml.h
    \reentrant
    \brief The QXmlReader class provides an interface for XML readers (i.e.
    parsers).
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module XML
    \ingroup xml-tools

    This abstract class provides an interface for all of Qt's XML
    readers. Currently there is only one implementation of a reader
    included in Qt's XML module: QXmlSimpleReader. In future releases
    there might be more readers with different properties available
    (e.g. a validating parser).

    The design of the XML classes follows the \link
    http://www.saxproject.org/ SAX2 Java interface\endlink, with
    the names adapted to fit Qt naming conventions. It should be very
    easy for anybody who has worked with SAX2 to get started with the
    Qt XML classes.

    All readers use the class QXmlInputSource to read the input
    document. Since you are normally interested in particular content
    in the XML document, the reader reports the content through
    special handler classes (QXmlDTDHandler, QXmlDeclHandler,
    QXmlContentHandler, QXmlEntityResolver, QXmlErrorHandler and
    QXmlLexicalHandler), which you must subclass, if you want to
    process the contents.

    Since the handler classes only describe interfaces you must
    implement all the functions. We provide the QXmlDefaultHandler
    class to make this easier: it implements a default behaviour (do
    nothing) for all functions, so you can subclass it and just
    implement the functions you are interested in.

    Features and properties of the reader can be set with setFeature()
    and setProperty() respectively. You can set the reader to use your
    own subclasses with setEntityResolver(), setDTDHandler(),
    setContentHandler(), setErrorHandler(), setLexicalHandler() and
    setDeclHandler(). The parse itself is started with a call to
    parse().

    \sa QXmlSimpleReader
*/

/*!
    \fn bool QXmlReader::feature( const QString& name, bool *ok ) const

    If the reader has the feature called \a name, the feature's value
    is returned. If no such feature exists the return value is
    undefined.

    If \a ok is not 0: \a *ok  is set to TRUE if the reader has the
    feature called \a name; otherwise \a *ok is set to FALSE.

    \sa setFeature() hasFeature()
*/

/*!
    \fn void QXmlReader::setFeature( const QString& name, bool value )

    Sets the feature called \a name to the given \a value. If the
    reader doesn't have the feature nothing happens.

    \sa feature() hasFeature()
*/

/*!
    \fn bool QXmlReader::hasFeature( const QString& name ) const

    Returns \c TRUE if the reader has the feature called \a name;
    otherwise returns FALSE.

    \sa feature() setFeature()
*/

/*!
    \fn void* QXmlReader::property( const QString& name, bool *ok ) const

    If the reader has the property \a name, this function returns the
    value of the property; otherwise the return value is undefined.

    If \a ok is not 0: if the reader has the \a name property \a *ok
    is set to TRUE; otherwise \a *ok is set to FALSE.

    \sa setProperty() hasProperty()
*/

/*!
    \fn void QXmlReader::setProperty( const QString& name, void* value )

    Sets the property \a name to \a value. If the reader doesn't have
    the property nothing happens.

    \sa property() hasProperty()
*/

/*!
    \fn bool QXmlReader::hasProperty( const QString& name ) const

    Returns TRUE if the reader has the property \a name; otherwise
    returns FALSE.

    \sa property() setProperty()
*/

/*!
    \fn void QXmlReader::setEntityResolver( QXmlEntityResolver* handler )

    Sets the entity resolver to \a handler.

    \sa entityResolver()
*/

/*!
    \fn QXmlEntityResolver* QXmlReader::entityResolver() const

    Returns the entity resolver or 0 if none was set.

    \sa setEntityResolver()
*/

/*!
    \fn void QXmlReader::setDTDHandler( QXmlDTDHandler* handler )

    Sets the DTD handler to \a handler.

    \sa DTDHandler()
*/

/*!
    \fn QXmlDTDHandler* QXmlReader::DTDHandler() const

    Returns the DTD handler or 0 if none was set.

    \sa setDTDHandler()
*/

/*!
    \fn void QXmlReader::setContentHandler( QXmlContentHandler* handler )

    Sets the content handler to \a handler.

    \sa contentHandler()
*/

/*!
    \fn QXmlContentHandler* QXmlReader::contentHandler() const

    Returns the content handler or 0 if none was set.

    \sa setContentHandler()
*/

/*!
    \fn void QXmlReader::setErrorHandler( QXmlErrorHandler* handler )

    Sets the error handler to \a handler. Clears the error handler if
    \a handler is 0.

    \sa errorHandler()
*/

/*!
    \fn QXmlErrorHandler* QXmlReader::errorHandler() const

    Returns the error handler or 0 if none is set.

    \sa setErrorHandler()
*/

/*!
    \fn void QXmlReader::setLexicalHandler( QXmlLexicalHandler* handler )

    Sets the lexical handler to \a handler.

    \sa lexicalHandler()
*/

/*!
    \fn QXmlLexicalHandler* QXmlReader::lexicalHandler() const

    Returns the lexical handler or 0 if none was set.

    \sa setLexicalHandler()
*/

/*!
    \fn void QXmlReader::setDeclHandler( QXmlDeclHandler* handler )

    Sets the declaration handler to \a handler.

    \sa declHandler()
*/

/*!
    \fn QXmlDeclHandler* QXmlReader::declHandler() const

    Returns the declaration handler or 0 if none was set.

    \sa setDeclHandler()
*/

/*!
  \fn bool QXmlReader::parse( const QXmlInputSource& input )

  \obsolete
*/

/*!
    \fn bool QXmlReader::parse( const QXmlInputSource* input )

    Reads an XML document from \a input and parses it. Returns TRUE if
    the parsing was successful; otherwise returns FALSE.
*/


/*!
    \class QXmlSimpleReader qxml.h
    \reentrant
    \brief The QXmlSimpleReader class provides an implementation of a
    simple XML reader (parser).
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module XML
    \ingroup xml-tools
    \mainclass

    This XML reader is sufficient for simple parsing tasks. The reader:
    \list
    \i provides a well-formed parser;
    \i does not parse any external entities;
    \i can do namespace processing.
    \endlist

    Documents are parsed with a call to parse().

*/

static inline bool is_S(QChar ch)
{
    ushort uc = ch.unicode();
    return (uc == ' ' || uc == '\t' || uc == '\n' || uc == '\r');
}

enum NameChar { NameBeginning, NameNotBeginning, NotName };

static const char Begi = (char)NameBeginning;
static const char NtBg = (char)NameNotBeginning;
static const char NotN = (char)NotName;

static const char nameCharTable[128] =
{
// 0x00
    NotN, NotN, NotN, NotN, NotN, NotN, NotN, NotN,
    NotN, NotN, NotN, NotN, NotN, NotN, NotN, NotN,
// 0x10
    NotN, NotN, NotN, NotN, NotN, NotN, NotN, NotN,
    NotN, NotN, NotN, NotN, NotN, NotN, NotN, NotN,
// 0x20 (0x2D is '-', 0x2E is '.')
    NotN, NotN, NotN, NotN, NotN, NotN, NotN, NotN,
    NotN, NotN, NotN, NotN, NotN, NtBg, NtBg, NotN,
// 0x30 (0x30..0x39 are '0'..'9', 0x3A is ':')
    NtBg, NtBg, NtBg, NtBg, NtBg, NtBg, NtBg, NtBg,
    NtBg, NtBg, Begi, NotN, NotN, NotN, NotN, NotN,
// 0x40 (0x41..0x5A are 'A'..'Z')
    NotN, Begi, Begi, Begi, Begi, Begi, Begi, Begi,
    Begi, Begi, Begi, Begi, Begi, Begi, Begi, Begi,
// 0x50 (0x5F is '_')
    Begi, Begi, Begi, Begi, Begi, Begi, Begi, Begi,
    Begi, Begi, Begi, NotN, NotN, NotN, NotN, Begi,
// 0x60 (0x61..0x7A are 'a'..'z')
    NotN, Begi, Begi, Begi, Begi, Begi, Begi, Begi,
    Begi, Begi, Begi, Begi, Begi, Begi, Begi, Begi,
// 0x70
    Begi, Begi, Begi, Begi, Begi, Begi, Begi, Begi,
    Begi, Begi, Begi, NotN, NotN, NotN, NotN, NotN
};

static inline NameChar fastDetermineNameChar(QChar ch)
{
    ushort uc = ch.unicode();
    if (!(uc & ~0x7f)) // uc < 128
        return (NameChar)nameCharTable[uc];

    QChar::Category cat = ch.category();
    // ### some these categories might be slightly wrong
    if ((cat >= QChar::Letter_Uppercase && cat <= QChar::Letter_Other)
        || cat == QChar::Number_Letter)
        return NameBeginning;
    if ((cat >= QChar::Number_DecimalDigit && cat <= QChar::Number_Other)
                || (cat >= QChar::Mark_NonSpacing && cat <= QChar::Mark_Enclosing))
        return NameNotBeginning;
    return NotName;
}

static NameChar determineNameChar(QChar ch)
{
    return fastDetermineNameChar(ch);
}

inline void QXmlSimpleReader::nameClear()
{
    d->nameValueLen = 0; nameArrayPos = 0;
}

inline void QXmlSimpleReader::refClear()
{
    d->refValueLen = 0; refArrayPos = 0;
}

/*!
    Constructs a simple XML reader with the following feature settings:
    \table
    \header \i Feature \i Setting
    \row \i \e http://xml.org/sax/features/namespaces \i TRUE
    \row \i \e http://xml.org/sax/features/namespace-prefixes \i FALSE
    \row \i \e http://trolltech.com/xml/features/report-whitespace-only-CharData
	 \i TRUE
    \row \i \e http://trolltech.com/xml/features/report-start-end-entity \i FALSE
    \endtable

    More information about features can be found in the \link
    xml.html#sax2Features Qt SAX2 overview. \endlink

    \sa setFeature()
*/

QXmlSimpleReader::QXmlSimpleReader()
{
    d = new QXmlSimpleReaderPrivate();
    d->locator = new QXmlSimpleReaderLocator( this );

    entityRes  = 0;
    dtdHnd     = 0;
    contentHnd = 0;
    errorHnd   = 0;
    lexicalHnd = 0;
    declHnd    = 0;

    // default feature settings
    d->useNamespaces = TRUE;
    d->useNamespacePrefixes = FALSE;
    d->reportWhitespaceCharData = TRUE;
    d->reportEntities = FALSE;
}

/*!
    Destroys the simple XML reader.
*/
QXmlSimpleReader::~QXmlSimpleReader()
{
    delete d->locator;
    delete d;
}

/*!
    \reimp
*/
bool QXmlSimpleReader::feature( const QString& name, bool *ok ) const
{
    if ( ok != 0 )
	*ok = TRUE;
    if        ( name == "http://xml.org/sax/features/namespaces" ) {
	return d->useNamespaces;
    } else if ( name == "http://xml.org/sax/features/namespace-prefixes" ) {
	return d->useNamespacePrefixes;
    } else if ( name == "http://trolltech.com/xml/features/report-whitespace-only-CharData" ) {
	return d->reportWhitespaceCharData;
    } else if ( name == "http://trolltech.com/xml/features/report-start-end-entity" ) {
	return d->reportEntities;
    } else {
	qWarning( "Unknown feature %s", name.latin1() );
	if ( ok != 0 )
	    *ok = FALSE;
    }
    return FALSE;
}

/*!
    Sets the state of the feature \a name to \a value:

    If the feature is not recognized, it is ignored.

    The following features are supported:
    \table
    \header \i Feature \i Notes
    \row \i \e http://xml.org/sax/features/namespaces
         \i If this feature is TRUE, namespace processing is
     performed.
    \row \i \e http://xml.org/sax/features/namespace-prefixes
         \i If this feature is TRUE, the the original prefixed names
            and attributes used for namespace declarations are
            reported.
    \row \i \e http://trolltech.com/xml/features/report-whitespace-only-CharData
         \i If this feature is TRUE, CharData that only contain
            whitespace are not ignored, but are reported via
            QXmlContentHandler::characters().
    \row \i \e http://trolltech.com/xml/features/report-start-end-entity
         \i If this feature is TRUE, the parser reports
            QXmlContentHandler::startEntity() and
            QXmlContentHandler::endEntity() events. So character data
            might be reported in chunks. If this feature is FALSE, the
            parser does not report those events, but rather silently
            substitutes the entities and reports the character data in
            one chunk.
    \endtable

    \quotefile xml/tagreader-with-features/tagreader.cpp
    \skipto reader
    \printline reader
    \skipto setFeature
    \printline setFeature
    \printline TRUE

    (Code taken from xml/tagreader-with-features/tagreader.cpp)

    \sa feature() hasFeature()
*/
void QXmlSimpleReader::setFeature( const QString& name, bool value )
{
    if        ( name == "http://xml.org/sax/features/namespaces" ) {
	d->useNamespaces = value;
    } else if ( name == "http://xml.org/sax/features/namespace-prefixes" ) {
	d->useNamespacePrefixes = value;
    } else if ( name == "http://trolltech.com/xml/features/report-whitespace-only-CharData" ) {
	d->reportWhitespaceCharData = value;
    } else if ( name == "http://trolltech.com/xml/features/report-start-end-entity" ) {
	d->reportEntities = value;
    } else {
	qWarning( "Unknown feature %s", name.latin1() );
    }
}

/*! \reimp
*/
bool QXmlSimpleReader::hasFeature( const QString& name ) const
{
    if ( name == "http://xml.org/sax/features/namespaces"
	    || name == "http://xml.org/sax/features/namespace-prefixes"
	    || name == "http://trolltech.com/xml/features/report-whitespace-only-CharData"
	    || name == "http://trolltech.com/xml/features/report-start-end-entity" ) {
	return TRUE;
    } else {
	return FALSE;
    }
}

/*! \reimp
*/
void* QXmlSimpleReader::property( const QString&, bool *ok ) const
{
    if ( ok != 0 )
	*ok = FALSE;
    return 0;
}

/*! \reimp
*/
void QXmlSimpleReader::setProperty( const QString&, void* )
{
}

/*!
    \reimp
*/
bool QXmlSimpleReader::hasProperty( const QString& ) const
{
    return FALSE;
}

/*!
    \reimp
*/
void QXmlSimpleReader::setEntityResolver( QXmlEntityResolver* handler )
{ entityRes = handler; }

/*!
    \reimp
*/
QXmlEntityResolver* QXmlSimpleReader::entityResolver() const
{ return entityRes; }

/*!
    \reimp
*/
void QXmlSimpleReader::setDTDHandler( QXmlDTDHandler* handler )
{ dtdHnd = handler; }

/*!
    \reimp
*/
QXmlDTDHandler* QXmlSimpleReader::DTDHandler() const
{ return dtdHnd; }

/*!
    \reimp
*/
void QXmlSimpleReader::setContentHandler( QXmlContentHandler* handler )
{ contentHnd = handler; }

/*!
    \reimp
*/
QXmlContentHandler* QXmlSimpleReader::contentHandler() const
{ return contentHnd; }

/*!
    \reimp
*/
void QXmlSimpleReader::setErrorHandler( QXmlErrorHandler* handler )
{ errorHnd = handler; }

/*!
    \reimp
*/
QXmlErrorHandler* QXmlSimpleReader::errorHandler() const
{ return errorHnd; }

/*!
    \reimp
*/
void QXmlSimpleReader::setLexicalHandler( QXmlLexicalHandler* handler )
{ lexicalHnd = handler; }

/*!
    \reimp
*/
QXmlLexicalHandler* QXmlSimpleReader::lexicalHandler() const
{ return lexicalHnd; }

/*!
    \reimp
*/
void QXmlSimpleReader::setDeclHandler( QXmlDeclHandler* handler )
{ declHnd = handler; }

/*!
    \reimp
*/
QXmlDeclHandler* QXmlSimpleReader::declHandler() const
{ return declHnd; }



/*!
    \reimp
*/
bool QXmlSimpleReader::parse( const QXmlInputSource& input )
{
    return parse( &input, FALSE );
}

/*!
    \reimp
*/
bool QXmlSimpleReader::parse( const QXmlInputSource* input )
{
    return parse( input, FALSE );
}

/*!
    Reads an XML document from \a input and parses it. Returns FALSE
    if the parsing detects an error; otherwise returns TRUE.

    If \a incremental is TRUE, the parser does not return FALSE when
    it reaches the end of the \a input without reaching the end of the
    XML file. Instead it stores the state of the parser so that
    parsing can be continued at a later stage when more data is
    available. You can use the function parseContinue() to continue
    with parsing. This class stores a pointer to the input source \a
    input and the parseContinue() function tries to read from that
    input souce. This means that you should not delete the input
    source \a input until you've finished your calls to
    parseContinue(). If you call this function with \a incremental
    TRUE whilst an incremental parse is in progress a new parsing
    session will be started and the previous session lost.

    If \a incremental is FALSE, this function behaves like the normal
    parse function, i.e. it returns FALSE when the end of input is
    reached without reaching the end of the XML file and the parsing
    cannot be continued.

    \sa parseContinue() QSocket
*/
bool QXmlSimpleReader::parse( const QXmlInputSource *input, bool incremental )
{
    init( input );
    if ( incremental ) {
	d->initIncrementalParsing();
    } else {
	delete d->parseStack;
	d->parseStack = 0;
    }
    // call the handler
    if ( contentHnd ) {
	contentHnd->setDocumentLocator( d->locator );
	if ( !contentHnd->startDocument() ) {
	    reportParseError( contentHnd->errorString() );
	    d->tags.clear();
	    return FALSE;
	}
    }
    return parseBeginOrContinue( 0, incremental );
}

/*!
    Continues incremental parsing; this function reads the input from
    the QXmlInputSource that was specified with the last parse()
    command. To use this function, you \e must have called parse()
    with the incremental argument set to TRUE.

    Returns FALSE if a parsing error occurs; otherwise returns TRUE.

    If the input source returns an empty string for the function
    QXmlInputSource::data(), then this means that the end of the XML
    file has been reached; this is quite important, especially if you
    want to use the reader to parse more than one XML file.

    The case of the end of the XML file being reached without having
    finished parsing is not considered to be an error: you can
    continue parsing at a later stage by calling this function again
    when there is more data available to parse.

    This function assumes that the end of the XML document is reached
    if the QXmlInputSource::next() function returns
    QXmlInputSource::EndOfDocument. If the parser has not finished
    parsing when it encounters this symbol, it is an error and FALSE
    is returned.

    \sa parse() QXmlInputSource::next()
*/
bool QXmlSimpleReader::parseContinue()
{
    if ( d->parseStack == 0 || d->parseStack->isEmpty() )
	return FALSE;
    initData();
    int state = d->parseStack->pop().state;
    return parseBeginOrContinue( state, TRUE );
}

/*
  Common part of parse() and parseContinue()
*/
bool QXmlSimpleReader::parseBeginOrContinue( int state, bool incremental )
{
    bool atEndOrig = atEnd();

    if ( state==0 ) {
	if ( !parseProlog() ) {
	    if ( incremental && d->error.isNull() ) {
		pushParseState( 0, 0 );
		return TRUE;
	    } else {
		d->tags.clear();
		return FALSE;
	    }
	}
	state = 1;
    }
    if ( state==1 ) {
	if ( !parseElement() ) {
	    if ( incremental && d->error.isNull() ) {
		pushParseState( 0, 1 );
		return TRUE;
	    } else {
		d->tags.clear();
		return FALSE;
	    }
	}
	state = 2;
    }
    // parse Misc*
    while ( !atEnd() ) {
	if ( !parseMisc() ) {
	    if ( incremental && d->error.isNull() ) {
		pushParseState( 0, 2 );
		return TRUE;
	    } else {
		d->tags.clear();
		return FALSE;
	    }
	}
    }
    if ( !atEndOrig && incremental ) {
	// we parsed something at all, so be prepared to come back later
	pushParseState( 0, 2 );
	return TRUE;
    }
    // is stack empty?
    if ( !d->tags.isEmpty() && !d->error.isNull() ) {
	reportParseError( XMLERR_UNEXPECTEDEOF );
	d->tags.clear();
	return FALSE;
    }
    // call the handler
    if ( contentHnd ) {
	delete d->parseStack;
	d->parseStack = 0;
	if ( !contentHnd->endDocument() ) {
	    reportParseError( contentHnd->errorString() );
	    return FALSE;
	}
    }
    return TRUE;
}

//
// The following private parse functions have another semantics for the return
// value: They return TRUE iff parsing has finished successfully (i.e. the end
// of the XML file must be reached!). If one of these functions return FALSE,
// there is only an error when d->error.isNULL() is also FALSE.
//

/*
  For the incremental parsing, it is very important that the parse...()
  functions have a certain structure. Since it might be hard to understand how
  they work, here is a description of the layout of these functions:

    bool QXmlSimpleReader::parse...()
    {
(1)	const signed char Init             = 0;
	...

(2)	const signed char Inp...           = 0;
	...

(3)	static signed char table[3][2] = {
	...
	};
	signed char state;
	signed char input;

(4)        if (d->parseStack == 0 || d->parseStack->isEmpty()) {
(4a)	...
	} else {
(4b)	...
	}

	for ( ; ; ) {
(5)	    switch ( state ) {
	    ...
	    }

(6)
(6a)	    if ( atEnd() ) {
		unexpectedEof( &QXmlSimpleReader::parseNmtoken, state );
		return FALSE;
	    }
(6b)        if (determineNameChar(c) != NotName) {
	    ...
	    }
(7)	    state = table[state][input];

(8)	    switch ( state ) {
	    ...
	    }
	}
    }

  Explanation:
  ad 1: constants for the states (used in the transition table)
  ad 2: constants for the input (used in the transition table)
  ad 3: the transition table for the state machine
  ad 4: test if we are in a parseContinue() step
	a) if no, do inititalizations
	b) if yes, restore the state and call parse functions recursively
  ad 5: Do some actions according to the state; from the logical execution
	order, this code belongs after 8 (see there for an explanation)
  ad 6: Check the character that is at the actual "cursor" position:
	a) If we reached the EOF, report either error or push the state (in the
	   case of incremental parsing).
	b) Otherwise, set the input character constant for the transition
	   table.
  ad 7: Get the new state according to the input that was read.
  ad 8: Do some actions according to the state. The last line in every case
	statement reads new data (i.e. it move the cursor). This can also be
	done by calling another parse...() funtion. If you need processing for
	this state after that, you have to put it into the switch statement 5.
	This ensures that you have a well defined re-entry point, when you ran
	out of data.
*/

/*
  Parses the prolog [22].
*/
bool QXmlSimpleReader::parseProlog()
{
    const signed char Init             = 0;
    const signed char EatWS            = 1; // eat white spaces
    const signed char Lt               = 2; // '<' read
    const signed char Em               = 3; // '!' read
    const signed char DocType          = 4; // read doctype
    const signed char Comment          = 5; // read comment
    const signed char CommentR         = 6; // same as Comment, but already reported
    const signed char PInstr           = 7; // read PI
    const signed char PInstrR          = 8; // same as PInstr, but already reported
    const signed char Done             = 9;

    const signed char InpWs            = 0;
    const signed char InpLt            = 1; // <
    const signed char InpQm            = 2; // ?
    const signed char InpEm            = 3; // !
    const signed char InpD             = 4; // D
    const signed char InpDash          = 5; // -
    const signed char InpUnknown       = 6;

    static const signed char table[9][7] = {
     /*  InpWs   InpLt  InpQm  InpEm  InpD      InpDash  InpUnknown */
	{ EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }, // Init
	{ -1,     Lt,    -1,    -1,    -1,       -1,       -1      }, // EatWS
	{ -1,     -1,    PInstr,Em,    Done,     -1,       Done    }, // Lt
	{ -1,     -1,    -1,    -1,    DocType,  Comment,  -1      }, // Em
	{ EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }, // DocType
	{ EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }, // Comment
	{ EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }, // CommentR
	{ EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }, // PInstr
	{ EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }  // PInstrR
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	d->xmldecl_possible = TRUE;
	d->doctype_read = FALSE;
	state = Init;
    } else {
        state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseProlog (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
            ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseProlog, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case DocType:
		if ( d->doctype_read ) {
		    reportParseError( XMLERR_MORETHANONEDOCTYPE );
		    return FALSE;
		} else {
		    d->doctype_read = FALSE;
		}
		break;
	    case Comment:
		if ( lexicalHnd ) {
		    if ( !lexicalHnd->comment( string() ) ) {
			reportParseError( lexicalHnd->errorString() );
			return FALSE;
		    }
		}
		state = CommentR;
		break;
	    case PInstr:
		// call the handler
		if ( contentHnd ) {
		    if ( d->xmldecl_possible && !d->xmlVersion.isEmpty() ) {
			QString value( "version = '" );
			value += d->xmlVersion;
			value += "'";
			if ( !d->encoding.isEmpty() ) {
			    value += " encoding = '";
			    value += d->encoding;
			    value += "'";
			}
			if ( d->standalone == QXmlSimpleReaderPrivate::Yes ) {
			    value += " standalone = 'yes'";
			} else if ( d->standalone == QXmlSimpleReaderPrivate::No ) {
			    value += " standalone = 'no'";
			}
			if ( !contentHnd->processingInstruction( "xml", value ) ) {
			    reportParseError( contentHnd->errorString() );
			    return FALSE;
			}
		    } else {
			if ( !contentHnd->processingInstruction( name(), string() ) ) {
			    reportParseError( contentHnd->errorString() );
			    return FALSE;
			}
		    }
		}
		// XML declaration only on first position possible
		d->xmldecl_possible = FALSE;
		state = PInstrR;
		break;
	    case Done:
		return TRUE;
	    case -1:
		reportParseError( XMLERR_ERRORPARSINGELEMENT );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseProlog, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c.unicode() == '<' ) {
	    input = InpLt;
	} else if ( c.unicode() == '?' ) {
	    input = InpQm;
	} else if ( c.unicode() == '!' ) {
	    input = InpEm;
	} else if ( c.unicode() == 'D' ) {
	    input = InpD;
	} else if ( c.unicode() == '-' ) {
	    input = InpDash;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case EatWS:
		// XML declaration only on first position possible
		d->xmldecl_possible = FALSE;
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseProlog, state );
		    return FALSE;
		}
		break;
	    case Lt:
		next();
		break;
	    case Em:
		// XML declaration only on first position possible
		d->xmldecl_possible = FALSE;
		next();
		break;
	    case DocType:
		if ( !parseDoctype() ) {
		    parseFailed( &QXmlSimpleReader::parseProlog, state );
		    return FALSE;
		}
		break;
	    case Comment:
	    case CommentR:
		if ( !parseComment() ) {
		    parseFailed( &QXmlSimpleReader::parseProlog, state );
		    return FALSE;
		}
		break;
	    case PInstr:
	    case PInstrR:
		d->parsePI_xmldecl = d->xmldecl_possible;
		if ( !parsePI() ) {
		    parseFailed( &QXmlSimpleReader::parseProlog, state );
		    return FALSE;
		}
		break;
	}
    }
}

/*
  Parse an element [39].

  Precondition: the opening '<' is already read.
*/
bool QXmlSimpleReader::parseElement()
{
    const int Init             =  0;
    const int ReadName         =  1;
    const int Ws1              =  2;
    const int STagEnd          =  3;
    const int STagEnd2         =  4;
    const int ETagBegin        =  5;
    const int ETagBegin2       =  6;
    const int Ws2              =  7;
    const int EmptyTag         =  8;
    const int Attrib           =  9;
    const int AttribPro        = 10; // like Attrib, but processAttribute was already called
    const int Ws3              = 11;
    const int Done             = 12;

    const int InpWs            = 0; // whitespace
    const int InpNameBe        = 1; // is_NameBeginning()
    const int InpGt            = 2; // >
    const int InpSlash         = 3; // /
    const int InpUnknown       = 4;

    static const int table[12][5] = {
     /*  InpWs      InpNameBe    InpGt        InpSlash     InpUnknown */
	{ -1,        ReadName,    -1,          -1,          -1        }, // Init
	{ Ws1,       Attrib,      STagEnd,     EmptyTag,    -1        }, // ReadName
	{ -1,        Attrib,      STagEnd,     EmptyTag,    -1        }, // Ws1
	{ STagEnd2,  STagEnd2,    STagEnd2,    STagEnd2,    STagEnd2  }, // STagEnd
	{ -1,        -1,          -1,          ETagBegin,   -1        }, // STagEnd2
	{ -1,        ETagBegin2,  -1,          -1,          -1        }, // ETagBegin
	{ Ws2,       -1,          Done,        -1,          -1        }, // ETagBegin2
	{ -1,        -1,          Done,        -1,          -1        }, // Ws2
	{ -1,        -1,          Done,        -1,          -1        }, // EmptyTag
	{ Ws3,       Attrib,      STagEnd,     EmptyTag,    -1        }, // Attrib
	{ Ws3,       Attrib,      STagEnd,     EmptyTag,    -1        }, // AttribPro
	{ -1,        Attrib,      STagEnd,     EmptyTag,    -1        }  // Ws3
    };
    int state;
    int input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseElement (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseElement, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case ReadName:
		// store it on the stack
		d->tags.push( name() );
		// empty the attributes
		d->attList.clear();
		if ( d->useNamespaces )
		    d->namespaceSupport.pushContext();
		break;
	    case ETagBegin2:
		if ( !processElementETagBegin2() )
		    return FALSE;
		break;
	    case Attrib:
		if ( !processElementAttribute() )
		    return FALSE;
		state = AttribPro;
		break;
	    case Done:
		return TRUE;
	    case -1:
		reportParseError( XMLERR_ERRORPARSINGELEMENT );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseElement, state );
	    return FALSE;
	}

        if (fastDetermineNameChar(c) == NameBeginning) {
	    input = InpNameBe;
	} else if ( c.unicode() == '>' ) {
	    input = InpGt;
	} else if (is_S(c)) {
            input = InpWs;
        } else if (c.unicode() == '/') {
	    input = InpSlash;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case ReadName:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseElement, state );
		    return FALSE;
		}
		break;
	    case Ws1:
	    case Ws2:
	    case Ws3:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElement, state );
		    return FALSE;
		}
		break;
	    case STagEnd:
		// call the handler
		if ( contentHnd ) {
                    const QString &tagsTop = d->tags.top();
		    if ( d->useNamespaces ) {
			QString uri, lname;
                        d->namespaceSupport.processName(tagsTop, FALSE, uri, lname);
                        if (!contentHnd->startElement(uri, lname, tagsTop, d->attList)) {
			    reportParseError( contentHnd->errorString() );
			    return FALSE;
			}
		    } else {
                        if (!contentHnd->startElement(QString::null, QString::null, tagsTop, d->attList)) {
			    reportParseError( contentHnd->errorString() );
			    return FALSE;
			}
		    }
		}
		next();
		break;
	    case STagEnd2:
		if ( !parseContent() ) {
		    parseFailed( &QXmlSimpleReader::parseElement, state );
		    return FALSE;
		}
		break;
	    case ETagBegin:
		next();
		break;
	    case ETagBegin2:
		// get the name of the tag
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseElement, state );
		    return FALSE;
		}
		break;
	    case EmptyTag:
		if  ( d->tags.isEmpty() ) {
		    reportParseError( XMLERR_TAGMISMATCH );
		    return FALSE;
		}
		if ( !processElementEmptyTag() )
		    return FALSE;
		next();
		break;
	    case Attrib:
	    case AttribPro:
		// get name and value of attribute
		if ( !parseAttribute() ) {
		    parseFailed( &QXmlSimpleReader::parseElement, state );
		    return FALSE;
		}
		break;
	    case Done:
		next();
		break;
	}
    }
}
/*
  Helper to break down the size of the code in the case statement.
  Return FALSE on error, otherwise TRUE.
*/
bool QXmlSimpleReader::processElementEmptyTag()
{
    QString uri, lname;
    // pop the stack and call the handler
    if ( contentHnd ) {
	if ( d->useNamespaces ) {
	    // report startElement first...
	    d->namespaceSupport.processName( d->tags.top(), FALSE, uri, lname );
	    if ( !contentHnd->startElement( uri, lname, d->tags.top(), d->attList ) ) {
		reportParseError( contentHnd->errorString() );
		return FALSE;
	    }
	    // ... followed by endElement...
	    if ( !contentHnd->endElement( uri, lname, d->tags.pop() ) ) {
		reportParseError( contentHnd->errorString() );
		return FALSE;
	    }
	    // ... followed by endPrefixMapping
	    QStringList prefixesBefore, prefixesAfter;
	    if ( contentHnd ) {
		prefixesBefore = d->namespaceSupport.prefixes();
	    }
	    d->namespaceSupport.popContext();
	    // call the handler for prefix mapping
	    prefixesAfter = d->namespaceSupport.prefixes();
	    for ( QStringList::Iterator it = prefixesBefore.begin(); it != prefixesBefore.end(); ++it ) {
		if ( prefixesAfter.contains(*it) == 0 ) {
		    if ( !contentHnd->endPrefixMapping( *it ) ) {
			reportParseError( contentHnd->errorString() );
			return FALSE;
		    }
		}
	    }
	} else {
	    // report startElement first...
	    if ( !contentHnd->startElement( QString::null, QString::null, d->tags.top(), d->attList ) ) {
		reportParseError( contentHnd->errorString() );
		return FALSE;
	    }
	    // ... followed by endElement
	    if ( !contentHnd->endElement( QString::null, QString::null, d->tags.pop() ) ) {
		reportParseError( contentHnd->errorString() );
		return FALSE;
	    }
	}
    } else {
	d->tags.pop_back();
	d->namespaceSupport.popContext();
    }
    return TRUE;
}
/*
  Helper to break down the size of the code in the case statement.
  Return FALSE on error, otherwise TRUE.
*/
bool QXmlSimpleReader::processElementETagBegin2()
{
    const QString &name = QXmlSimpleReader::name();

    // pop the stack and compare it with the name
    if ( d->tags.pop() != name ) {
	reportParseError( XMLERR_TAGMISMATCH );
	return FALSE;
    }
    // call the handler
    if ( contentHnd ) {
        QString uri, lname;

        if (d->useNamespaces)
            d->namespaceSupport.processName(name, FALSE, uri, lname);
        if (!contentHnd->endElement(uri, lname, name)) {
            reportParseError(contentHnd->errorString());
            return FALSE;
	}
    }
    if ( d->useNamespaces ) {
        NamespaceMap prefixesBefore, prefixesAfter;
        if (contentHnd)
            prefixesBefore = d->namespaceSupport.d->ns;

	d->namespaceSupport.popContext();
	// call the handler for prefix mapping
	if ( contentHnd ) {
            prefixesAfter = d->namespaceSupport.d->ns;
            if (prefixesBefore.size() != prefixesAfter.size()) {
                for (NamespaceMap::const_iterator it = prefixesBefore.constBegin(); it != prefixesBefore.constEnd(); ++it) {
                    if (!it.key().isEmpty() && !prefixesAfter.contains(it.key())) {
                        if (!contentHnd->endPrefixMapping(it.key())) {
                            reportParseError(contentHnd->errorString());
                            return FALSE;
                        }
		    }
		}
	    }
	}
    }
    return TRUE;
}
/*
  Helper to break down the size of the code in the case statement.
  Return FALSE on error, otherwise TRUE.
*/
bool QXmlSimpleReader::processElementAttribute()
{
    QString uri, lname, prefix;
    const QString &name = QXmlSimpleReader::name();
    const QString &string = QXmlSimpleReader::string();

    // add the attribute to the list
    if ( d->useNamespaces ) {
	// is it a namespace declaration?
        d->namespaceSupport.splitName(name, prefix, lname);
        if (prefix == "xmlns") {
	    // namespace declaration
	    d->namespaceSupport.setPrefix( lname, string );
	    if ( d->useNamespacePrefixes ) {
		// according to http://www.w3.org/2000/xmlns/, the "prefix"
		// xmlns maps to the namespace name
		// http://www.w3.org/2000/xmlns/
		d->attList.append( name, "http://www.w3.org/2000/xmlns/", lname, string );
	    }
	    // call the handler for prefix mapping
	    if ( contentHnd ) {
		if ( !contentHnd->startPrefixMapping( lname, string ) ) {
		    reportParseError( contentHnd->errorString() );
		    return FALSE;
		}
	    }
	} else {
	    // no namespace delcaration
	    d->namespaceSupport.processName( name, TRUE, uri, lname );
	    d->attList.append( name, uri, lname, string );
	}
    } else {
	// no namespace support
	d->attList.append( name, QString::null, QString::null, string );
    }
    return TRUE;
}

/*
  Parse a content [43].

  A content is only used between tags. If a end tag is found the < is already
  read and the head stand on the '/' of the end tag '</name>'.
*/
bool QXmlSimpleReader::parseContent()
{
    const signed char Init             =  0;
    const signed char ChD              =  1; // CharData
    const signed char ChD1             =  2; // CharData help state
    const signed char ChD2             =  3; // CharData help state
    const signed char Ref              =  4; // Reference
    const signed char Lt               =  5; // '<' read
    const signed char PInstr           =  6; // PI
    const signed char PInstrR          =  7; // same as PInstr, but already reported
    const signed char Elem             =  8; // Element
    const signed char Em               =  9; // '!' read
    const signed char Com              = 10; // Comment
    const signed char ComR             = 11; // same as Com, but already reported
    const signed char CDS              = 12; // CDSect
    const signed char CDS1             = 13; // read a CDSect
    const signed char CDS2             = 14; // read a CDSect (help state)
    const signed char CDS3             = 15; // read a CDSect (help state)
    const signed char Done             = 16; // finished reading content

    const signed char InpLt            = 0; // <
    const signed char InpGt            = 1; // >
    const signed char InpSlash         = 2; // /
    const signed char InpQMark         = 3; // ?
    const signed char InpEMark         = 4; // !
    const signed char InpAmp           = 5; // &
    const signed char InpDash          = 6; // -
    const signed char InpOpenB         = 7; // [
    const signed char InpCloseB        = 8; // ]
    const signed char InpUnknown       = 9;

    static const signed char mapCLT2FSMChar[] = {
	InpUnknown, // white space
	InpUnknown, // %
	InpAmp,     // &
	InpGt,      // >
	InpLt,      // <
	InpSlash,   // /
	InpQMark,   // ?
	InpEMark,   // !
	InpDash,    // -
	InpCloseB,  // ]
	InpOpenB,   // [
	InpUnknown, // =
	InpUnknown, // "
	InpUnknown, // '
	InpUnknown  // unknown
    };

    static const signed char table[16][10] = {
     /*  InpLt  InpGt  InpSlash  InpQMark  InpEMark  InpAmp  InpDash  InpOpenB  InpCloseB  InpUnknown */
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD1,      ChD  }, // Init
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD1,      ChD  }, // ChD
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD2,      ChD  }, // ChD1
	{ Lt,    -1,    ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD2,      ChD  }, // ChD2
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // Ref (same as Init)
	{ -1,    -1,    Done,     PInstr,   Em,       -1,     -1,      -1,       -1,        Elem }, // Lt
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // PInstr (same as Init)
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // PInstrR
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // Elem (same as Init)
	{ -1,    -1,    -1,       -1,       -1,       -1,     Com,     CDS,      -1,        -1   }, // Em
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // Com (same as Init)
	{ Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // ComR
	{ CDS1,  CDS1,  CDS1,     CDS1,     CDS1,     CDS1,   CDS1,    CDS1,     CDS2,      CDS1 }, // CDS
	{ CDS1,  CDS1,  CDS1,     CDS1,     CDS1,     CDS1,   CDS1,    CDS1,     CDS2,      CDS1 }, // CDS1
	{ CDS1,  CDS1,  CDS1,     CDS1,     CDS1,     CDS1,   CDS1,    CDS1,     CDS3,      CDS1 }, // CDS2
	{ CDS1,  Init,  CDS1,     CDS1,     CDS1,     CDS1,   CDS1,    CDS1,     CDS3,      CDS1 }  // CDS3
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	d->contentCharDataRead = FALSE;
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseContent (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseContent, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case Ref:
		if ( !d->contentCharDataRead)
		    d->contentCharDataRead = d->parseReference_charDataRead;
		break;
	    case PInstr:
		if ( contentHnd ) {
		    if ( !contentHnd->processingInstruction(name(),string()) ) {
			reportParseError( contentHnd->errorString() );
			return FALSE;
		    }
		}
		state = PInstrR;
		break;
	    case Com:
		if ( lexicalHnd ) {
		    if ( !lexicalHnd->comment( string() ) ) {
			reportParseError( lexicalHnd->errorString() );
			return FALSE;
		    }
		}
		state = ComR;
		break;
	    case CDS:
		stringClear();
		break;
	    case CDS2:
		if ( !atEnd() && c.unicode() != ']' )
		    stringAddC( ']' );
		break;
	    case CDS3:
		// test if this skipping was legal
		if ( !atEnd() ) {
		    if ( c.unicode() == '>' ) {
			// the end of the CDSect
			if ( lexicalHnd ) {
			    if ( !lexicalHnd->startCDATA() ) {
				reportParseError( lexicalHnd->errorString() );
				return FALSE;
			    }
			}
			if ( contentHnd ) {
			    if ( !contentHnd->characters( string() ) ) {
				reportParseError( contentHnd->errorString() );
				return FALSE;
			    }
			}
			if ( lexicalHnd ) {
			    if ( !lexicalHnd->endCDATA() ) {
				reportParseError( lexicalHnd->errorString() );
				return FALSE;
			    }
			}
		    } else if (c.unicode() == ']') {
			// three or more ']'
			stringAddC( ']' );
		    } else {
			// after ']]' comes another character
			stringAddC( ']' );
			stringAddC( ']' );
		    }
		}
		break;
	    case Done:
		// call the handler for CharData
		if ( contentHnd ) {
		    if ( d->contentCharDataRead ) {
			if ( d->reportWhitespaceCharData || !string().simplifyWhiteSpace().isEmpty() ) {
			    if ( !contentHnd->characters( string() ) ) {
				reportParseError( contentHnd->errorString() );
				return FALSE;
			    }
			}
		    }
		}
		// Done
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_ERRORPARSINGCONTENT );
		return FALSE;
	}

	// get input (use lookup-table instead of nested ifs for performance
	// reasons)
	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseContent, state );
	    return FALSE;
	}
	if ( c.row() ) {
	    input = InpUnknown;
	} else {
	    input = mapCLT2FSMChar[ charLookupTable[ c.cell() ] ];
	}
	state = table[state][input];

	switch ( state ) {
	    case Init:
		// skip the ending '>' of a CDATASection
		next();
		break;
	    case ChD:
		// on first call: clear string
		if ( !d->contentCharDataRead ) {
		    d->contentCharDataRead = TRUE;
		    stringClear();
		}
		stringAddC();
		if ( d->reportEntities ) {
		    if ( !reportEndEntities() )
			return FALSE;
		}
		next();
		break;
	    case ChD1:
		// on first call: clear string
		if ( !d->contentCharDataRead ) {
		    d->contentCharDataRead = TRUE;
		    stringClear();
		}
		stringAddC();
		if ( d->reportEntities ) {
		    if ( !reportEndEntities() )
			return FALSE;
		}
		next();
		break;
	    case ChD2:
		stringAddC();
		if ( d->reportEntities ) {
		    if ( !reportEndEntities() )
			return FALSE;
		}
		next();
		break;
	    case Ref:
		if ( !d->contentCharDataRead) {
		    // reference may be CharData; so clear string to be safe
		    stringClear();
		    d->parseReference_context = InContent;
		    if ( !parseReference() ) {
			parseFailed( &QXmlSimpleReader::parseContent, state );
			return FALSE;
		    }
		} else {
		    if ( d->reportEntities ) {
			// report character data in chunks
			if ( contentHnd ) {
			    if ( d->reportWhitespaceCharData || !string().simplifyWhiteSpace().isEmpty() ) {
				if ( !contentHnd->characters( string() ) ) {
				    reportParseError( contentHnd->errorString() );
				    return FALSE;
				}
			    }
			}
			stringClear();
		    }
		    d->parseReference_context = InContent;
		    if ( !parseReference() ) {
			parseFailed( &QXmlSimpleReader::parseContent, state );
			return FALSE;
		    }
		}
		break;
	    case Lt:
		// call the handler for CharData
		if ( contentHnd ) {
		    if ( d->contentCharDataRead ) {
			if ( d->reportWhitespaceCharData || !string().simplifyWhiteSpace().isEmpty() ) {
			    if ( !contentHnd->characters( string() ) ) {
				reportParseError( contentHnd->errorString() );
				return FALSE;
			    }
			}
		    }
		}
		d->contentCharDataRead = FALSE;
		next();
		break;
	    case PInstr:
	    case PInstrR:
		d->parsePI_xmldecl = FALSE;
		if ( !parsePI() ) {
		    parseFailed( &QXmlSimpleReader::parseContent, state );
		    return FALSE;
		}
		break;
	    case Elem:
		if ( !parseElement() ) {
		    parseFailed( &QXmlSimpleReader::parseContent, state );
		    return FALSE;
		}
		break;
	    case Em:
		next();
		break;
	    case Com:
	    case ComR:
		if ( !parseComment() ) {
		    parseFailed( &QXmlSimpleReader::parseContent, state );
		    return FALSE;
		}
		break;
	    case CDS:
		d->parseString_s = "[CDATA[";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseContent, state );
		    return FALSE;
		}
		break;
	    case CDS1:
		stringAddC();
		next();
		break;
	    case CDS2:
		// skip ']'
		next();
		break;
	    case CDS3:
		// skip ']'...
		next();
		break;
	}
    }
}
bool QXmlSimpleReader::reportEndEntities()
{
    int count = (int)d->xmlRef.count();
    while ( count != 0 && d->xmlRef.top().isEmpty() ) {
	if ( contentHnd ) {
	    if ( d->reportWhitespaceCharData || !string().simplifyWhiteSpace().isEmpty() ) {
		if ( !contentHnd->characters( string() ) ) {
		    reportParseError( contentHnd->errorString() );
		    return FALSE;
		}
	    }
	}
	stringClear();
	if ( lexicalHnd ) {
	    if ( !lexicalHnd->endEntity(d->xmlRefName.top()) ) {
		reportParseError( lexicalHnd->errorString() );
		return FALSE;
	    }
	}
	d->xmlRef.pop_back();
	d->xmlRefName.pop_back();
	count--;
    }
    return TRUE;
}

/*
  Parse Misc [27].
*/
bool QXmlSimpleReader::parseMisc()
{
    const signed char Init             = 0;
    const signed char Lt               = 1; // '<' was read
    const signed char Comment          = 2; // read comment
    const signed char eatWS            = 3; // eat whitespaces
    const signed char PInstr           = 4; // read PI
    const signed char Comment2         = 5; // read comment

    const signed char InpWs            = 0; // S
    const signed char InpLt            = 1; // <
    const signed char InpQm            = 2; // ?
    const signed char InpEm            = 3; // !
    const signed char InpUnknown       = 4;

    static const signed char table[3][5] = {
     /*  InpWs   InpLt  InpQm  InpEm     InpUnknown */
	{ eatWS,  Lt,    -1,    -1,       -1        }, // Init
	{ -1,     -1,    PInstr,Comment,  -1        }, // Lt
	{ -1,     -1,    -1,    -1,       Comment2  }  // Comment
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseMisc (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseMisc, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case eatWS:
		return TRUE;
	    case PInstr:
		if ( contentHnd ) {
		    if ( !contentHnd->processingInstruction(name(),string()) ) {
			reportParseError( contentHnd->errorString() );
			return FALSE;
		    }
		}
		return TRUE;
	    case Comment2:
		if ( lexicalHnd ) {
		    if ( !lexicalHnd->comment( string() ) ) {
			reportParseError( lexicalHnd->errorString() );
			return FALSE;
		    }
		}
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseMisc, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c.unicode() == '<' ) {
	    input = InpLt;
	} else if ( c.unicode() == '?' ) {
	    input = InpQm;
	} else if ( c.unicode() == '!' ) {
	    input = InpEm;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case eatWS:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseMisc, state );
		    return FALSE;
		}
		break;
	    case Lt:
		next();
		break;
	    case PInstr:
		d->parsePI_xmldecl = FALSE;
		if ( !parsePI() ) {
		    parseFailed( &QXmlSimpleReader::parseMisc, state );
		    return FALSE;
		}
		break;
	    case Comment:
		next();
		break;
	    case Comment2:
		if ( !parseComment() ) {
		    parseFailed( &QXmlSimpleReader::parseMisc, state );
		    return FALSE;
		}
		break;
	}
    }
}

/*
  Parse a processing instruction [16].

  If xmldec is TRUE, it tries to parse a PI or a XML declaration [23].

  Precondition: the beginning '<' of the PI is already read and the head stand
  on the '?' of '<?'.

  If this funktion was successful, the head-position is on the first
  character after the PI.
*/
bool QXmlSimpleReader::parsePI()
{
    const signed char Init             =  0;
    const signed char QmI              =  1; // ? was read
    const signed char Name             =  2; // read Name
    const signed char XMLDecl          =  3; // read XMLDecl
    const signed char Ws1              =  4; // eat ws after "xml" of XMLDecl
    const signed char PInstr           =  5; // read PI
    const signed char Ws2              =  6; // eat ws after Name of PI
    const signed char Version          =  7; // read versionInfo
    const signed char Ws3              =  8; // eat ws after versionInfo
    const signed char EorSD            =  9; // read EDecl or SDDecl
    const signed char Ws4              = 10; // eat ws after EDecl or SDDecl
    const signed char SD               = 11; // read SDDecl
    const signed char Ws5              = 12; // eat ws after SDDecl
    const signed char ADone            = 13; // almost done
    const signed char Char             = 14; // Char was read
    const signed char Qm               = 15; // Qm was read
    const signed char Done             = 16; // finished reading content

    const signed char InpWs            = 0; // whitespace
    const signed char InpNameBe        = 1; // NameBeginning()
    const signed char InpGt            = 2; // >
    const signed char InpQm            = 3; // ?
    const signed char InpUnknown       = 4;

    static const signed char table[16][5] = {
     /*  InpWs,  InpNameBe  InpGt  InpQm   InpUnknown  */
	{ -1,     -1,        -1,    QmI,    -1     }, // Init
	{ -1,     Name,      -1,    -1,     -1     }, // QmI
	{ -1,     -1,        -1,    -1,     -1     }, // Name (this state is left not through input)
	{ Ws1,    -1,        -1,    -1,     -1     }, // XMLDecl
	{ -1,     Version,   -1,    -1,     -1     }, // Ws1
	{ Ws2,    -1,        -1,    Qm,     -1     }, // PInstr
	{ Char,   Char,      Char,  Qm,     Char   }, // Ws2
	{ Ws3,    -1,        -1,    ADone,  -1     }, // Version
	{ -1,     EorSD,     -1,    ADone,  -1     }, // Ws3
	{ Ws4,    -1,        -1,    ADone,  -1     }, // EorSD
	{ -1,     SD,        -1,    ADone,  -1     }, // Ws4
	{ Ws5,    -1,        -1,    ADone,  -1     }, // SD
	{ -1,     -1,        -1,    ADone,  -1     }, // Ws5
	{ -1,     -1,        Done,  -1,     -1     }, // ADone
	{ Char,   Char,      Char,  Qm,     Char   }, // Char
	{ Char,   Char,      Done,  Qm,     Char   }, // Qm
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parsePI (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parsePI, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case Name:
		// test what name was read and determine the next state
		// (not very beautiful, I admit)
		if ( name().lower() == "xml" ) {
		    if ( d->parsePI_xmldecl && name()=="xml" ) {
			state = XMLDecl;
		    } else {
			reportParseError( XMLERR_INVALIDNAMEFORPI );
			return FALSE;
		    }
		} else {
		    state = PInstr;
		    stringClear();
		}
		break;
	    case Version:
		// get version (syntax like an attribute)
		if ( name() != "version" ) {
		    reportParseError( XMLERR_VERSIONEXPECTED );
		    return FALSE;
		}
		d->xmlVersion = string();
		break;
	    case EorSD:
		// get the EDecl or SDDecl (syntax like an attribute)
		if        ( name() == "standalone" ) {
		    if ( string()=="yes" ) {
			d->standalone = QXmlSimpleReaderPrivate::Yes;
		    } else if ( string()=="no" ) {
			d->standalone = QXmlSimpleReaderPrivate::No;
		    } else {
			reportParseError( XMLERR_WRONGVALUEFORSDECL );
			return FALSE;
		    }
		} else if ( name() == "encoding" ) {
		    d->encoding = string();
		} else {
		    reportParseError( XMLERR_EDECLORSDDECLEXPECTED );
		    return FALSE;
		}
		break;
	    case SD:
		if ( name() != "standalone" ) {
		    reportParseError( XMLERR_SDDECLEXPECTED );
		    return FALSE;
		}
		if ( string()=="yes" ) {
		    d->standalone = QXmlSimpleReaderPrivate::Yes;
		} else if ( string()=="no" ) {
		    d->standalone = QXmlSimpleReaderPrivate::No;
		} else {
		    reportParseError( XMLERR_WRONGVALUEFORSDECL );
		    return FALSE;
		}
		break;
	    case Qm:
		// test if the skipping was legal
		if ( !atEnd() && c.unicode() != '>' )
		    stringAddC( '?' );
		break;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parsePI, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if (determineNameChar(c) == NameBeginning) {
	    input = InpNameBe;
	} else if ( c.unicode() == '>' ) {
	    input = InpGt;
	} else if ( c.unicode() == '?' ) {
	    input = InpQm;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case QmI:
		next();
		break;
	    case Name:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parsePI, state );
		    return FALSE;
		}
		break;
	    case Ws1:
	    case Ws2:
	    case Ws3:
	    case Ws4:
	    case Ws5:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parsePI, state );
		    return FALSE;
		}
		break;
	    case Version:
		if ( !parseAttribute() ) {
		    parseFailed( &QXmlSimpleReader::parsePI, state );
		    return FALSE;
		}
		break;
	    case EorSD:
		if ( !parseAttribute() ) {
		    parseFailed( &QXmlSimpleReader::parsePI, state );
		    return FALSE;
		}
		break;
	    case SD:
		// get the SDDecl (syntax like an attribute)
		if ( d->standalone != QXmlSimpleReaderPrivate::Unknown ) {
		    // already parsed the standalone declaration
		    reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		    return FALSE;
		}
		if ( !parseAttribute() ) {
		    parseFailed( &QXmlSimpleReader::parsePI, state );
		    return FALSE;
		}
		break;
	    case ADone:
		next();
		break;
	    case Char:
		stringAddC();
		next();
		break;
	    case Qm:
		// skip the '?'
		next();
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a document type definition (doctypedecl [28]).

  Precondition: the beginning '<!' of the doctype is already read the head
  stands on the 'D' of '<!DOCTYPE'.

  If this funktion was successful, the head-position is on the first
  character after the document type definition.
*/
bool QXmlSimpleReader::parseDoctype()
{
    const signed char Init             =  0;
    const signed char Doctype          =  1; // read the doctype
    const signed char Ws1              =  2; // eat_ws
    const signed char Doctype2         =  3; // read the doctype, part 2
    const signed char Ws2              =  4; // eat_ws
    const signed char Sys              =  5; // read SYSTEM or PUBLIC
    const signed char Ws3              =  6; // eat_ws
    const signed char MP               =  7; // markupdecl or PEReference
    const signed char MPR              =  8; // same as MP, but already reported
    const signed char PER              =  9; // PERReference
    const signed char Mup              = 10; // markupdecl
    const signed char Ws4              = 11; // eat_ws
    const signed char MPE              = 12; // end of markupdecl or PEReference
    const signed char Done             = 13;

    const signed char InpWs            = 0;
    const signed char InpD             = 1; // 'D'
    const signed char InpS             = 2; // 'S' or 'P'
    const signed char InpOB            = 3; // [
    const signed char InpCB            = 4; // ]
    const signed char InpPer           = 5; // %
    const signed char InpGt            = 6; // >
    const signed char InpUnknown       = 7;

    static const signed char table[13][8] = {
     /*  InpWs,  InpD       InpS       InpOB  InpCB  InpPer InpGt  InpUnknown */
	{ -1,     Doctype,   -1,        -1,    -1,    -1,    -1,    -1        }, // Init
	{ Ws1,    -1,        -1,        -1,    -1,    -1,    -1,    -1        }, // Doctype
	{ -1,     Doctype2,  Doctype2,  -1,    -1,    -1,    -1,    Doctype2  }, // Ws1
	{ Ws2,    -1,        Sys,       MP,    -1,    -1,    Done,  -1        }, // Doctype2
	{ -1,     -1,        Sys,       MP,    -1,    -1,    Done,  -1        }, // Ws2
	{ Ws3,    -1,        -1,        MP,    -1,    -1,    Done,  -1        }, // Sys
	{ -1,     -1,        -1,        MP,    -1,    -1,    Done,  -1        }, // Ws3
	{ -1,     -1,        -1,        -1,    MPE,   PER,   -1,    Mup       }, // MP
	{ -1,     -1,        -1,        -1,    MPE,   PER,   -1,    Mup       }, // MPR
	{ Ws4,    -1,        -1,        -1,    MPE,   PER,   -1,    Mup       }, // PER
	{ Ws4,    -1,        -1,        -1,    MPE,   PER,   -1,    Mup       }, // Mup
	{ -1,     -1,        -1,        -1,    MPE,   PER,   -1,    Mup       }, // Ws4
	{ -1,     -1,        -1,        -1,    -1,    -1,    Done,  -1        }  // MPE
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	d->startDTDwasReported = FALSE;
	d->systemId = QString::null;
	d->publicId = QString::null;
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseDoctype (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseDoctype, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case Doctype2:
		d->doctype = name();
		break;
	    case MP:
		if ( !d->startDTDwasReported && lexicalHnd  ) {
		    d->startDTDwasReported = TRUE;
		    if ( !lexicalHnd->startDTD( d->doctype, d->publicId, d->systemId ) ) {
			reportParseError( lexicalHnd->errorString() );
			return FALSE;
		    }
		}
		state = MPR;
		break;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_ERRORPARSINGDOCTYPE );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseDoctype, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c.unicode() == 'D' ) {
	    input = InpD;
	} else if ( c.unicode() == 'S' ) {
	    input = InpS;
	} else if ( c.unicode() == 'P' ) {
	    input = InpS;
	} else if ( c.unicode() == '[' ) {
	    input = InpOB;
	} else if ( c.unicode() == ']' ) {
	    input = InpCB;
	} else if ( c.unicode() == '%' ) {
	    input = InpPer;
	} else if ( c.unicode() == '>' ) {
	    input = InpGt;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Doctype:
		d->parseString_s = "DOCTYPE";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case Ws1:
	    case Ws2:
	    case Ws3:
	    case Ws4:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case Doctype2:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case Sys:
		d->parseExternalID_allowPublicID = FALSE;
		if ( !parseExternalID() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case MP:
	    case MPR:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case PER:
		d->parsePEReference_context = InDTD;
		if ( !parsePEReference() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case Mup:
		if ( !parseMarkupdecl() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case MPE:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseDoctype, state );
		    return FALSE;
		}
		break;
	    case Done:
		if ( lexicalHnd ) {
		    if ( !d->startDTDwasReported ) {
			d->startDTDwasReported = TRUE;
			if ( !lexicalHnd->startDTD( d->doctype, d->publicId, d->systemId ) ) {
			    reportParseError( lexicalHnd->errorString() );
			    return FALSE;
			}
		    }
		    if ( !lexicalHnd->endDTD() ) {
			reportParseError( lexicalHnd->errorString() );
			return FALSE;
		    }
		}
		next();
		break;
	}
    }
}

/*
  Parse a ExternalID [75].

  If allowPublicID is TRUE parse ExternalID [75] or PublicID [83].
*/
bool QXmlSimpleReader::parseExternalID()
{
    const signed char Init             =  0;
    const signed char Sys              =  1; // parse 'SYSTEM'
    const signed char SysWS            =  2; // parse the whitespace after 'SYSTEM'
    const signed char SysSQ            =  3; // parse SystemLiteral with '
    const signed char SysSQ2           =  4; // parse SystemLiteral with '
    const signed char SysDQ            =  5; // parse SystemLiteral with "
    const signed char SysDQ2           =  6; // parse SystemLiteral with "
    const signed char Pub              =  7; // parse 'PUBLIC'
    const signed char PubWS            =  8; // parse the whitespace after 'PUBLIC'
    const signed char PubSQ            =  9; // parse PubidLiteral with '
    const signed char PubSQ2           = 10; // parse PubidLiteral with '
    const signed char PubDQ            = 11; // parse PubidLiteral with "
    const signed char PubDQ2           = 12; // parse PubidLiteral with "
    const signed char PubE             = 13; // finished parsing the PubidLiteral
    const signed char PubWS2           = 14; // parse the whitespace after the PubidLiteral
    const signed char PDone            = 15; // done if allowPublicID is TRUE
    const signed char Done             = 16;

    const signed char InpSQ            = 0; // '
    const signed char InpDQ            = 1; // "
    const signed char InpS             = 2; // S
    const signed char InpP             = 3; // P
    const signed char InpWs            = 4; // white space
    const signed char InpUnknown       = 5;

    static const signed char table[15][6] = {
     /*  InpSQ    InpDQ    InpS     InpP     InpWs     InpUnknown */
	{ -1,      -1,      Sys,     Pub,     -1,       -1      }, // Init
	{ -1,      -1,      -1,      -1,      SysWS,    -1      }, // Sys
	{ SysSQ,   SysDQ,   -1,      -1,      -1,       -1      }, // SysWS
	{ Done,    SysSQ2,  SysSQ2,  SysSQ2,  SysSQ2,   SysSQ2  }, // SysSQ
	{ Done,    SysSQ2,  SysSQ2,  SysSQ2,  SysSQ2,   SysSQ2  }, // SysSQ2
	{ SysDQ2,  Done,    SysDQ2,  SysDQ2,  SysDQ2,   SysDQ2  }, // SysDQ
	{ SysDQ2,  Done,    SysDQ2,  SysDQ2,  SysDQ2,   SysDQ2  }, // SysDQ2
	{ -1,      -1,      -1,      -1,      PubWS,    -1      }, // Pub
	{ PubSQ,   PubDQ,   -1,      -1,      -1,       -1      }, // PubWS
	{ PubE,    -1,      PubSQ2,  PubSQ2,  PubSQ2,   PubSQ2  }, // PubSQ
	{ PubE,    -1,      PubSQ2,  PubSQ2,  PubSQ2,   PubSQ2  }, // PubSQ2
	{ -1,      PubE,    PubDQ2,  PubDQ2,  PubDQ2,   PubDQ2  }, // PubDQ
	{ -1,      PubE,    PubDQ2,  PubDQ2,  PubDQ2,   PubDQ2  }, // PubDQ2
	{ PDone,   PDone,   PDone,   PDone,   PubWS2,   PDone   }, // PubE
	{ SysSQ,   SysDQ,   PDone,   PDone,   PDone,    PDone   }  // PubWS2
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	d->systemId = QString::null;
	d->publicId = QString::null;
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseExternalID (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseExternalID, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case PDone:
		if ( d->parseExternalID_allowPublicID ) {
		    d->publicId = string();
		    return TRUE;
		} else {
		    reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		    return FALSE;
		}
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseExternalID, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c.unicode() == '\'' ) {
	    input = InpSQ;
	} else if ( c.unicode() == '"' ) {
	    input = InpDQ;
	} else if ( c.unicode() == 'S' ) {
	    input = InpS;
	} else if ( c.unicode() == 'P' ) {
	    input = InpP;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Sys:
		d->parseString_s = "SYSTEM";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseExternalID, state );
		    return FALSE;
		}
		break;
	    case SysWS:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseExternalID, state );
		    return FALSE;
		}
		break;
	    case SysSQ:
	    case SysDQ:
		stringClear();
		next();
		break;
	    case SysSQ2:
	    case SysDQ2:
		stringAddC();
		next();
		break;
	    case Pub:
		d->parseString_s = "PUBLIC";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseExternalID, state );
		    return FALSE;
		}
		break;
	    case PubWS:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseExternalID, state );
		    return FALSE;
		}
		break;
	    case PubSQ:
	    case PubDQ:
		stringClear();
		next();
		break;
	    case PubSQ2:
	    case PubDQ2:
		stringAddC();
		next();
		break;
	    case PubE:
		next();
		break;
	    case PubWS2:
		d->publicId = string();
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseExternalID, state );
		    return FALSE;
		}
		break;
	    case Done:
		d->systemId = string();
		next();
		break;
	}
    }
}

/*
  Parse a markupdecl [29].
*/
bool QXmlSimpleReader::parseMarkupdecl()
{
    const signed char Init             = 0;
    const signed char Lt               = 1; // < was read
    const signed char Em               = 2; // ! was read
    const signed char CE               = 3; // E was read
    const signed char Qm               = 4; // ? was read
    const signed char Dash             = 5; // - was read
    const signed char CA               = 6; // A was read
    const signed char CEL              = 7; // EL was read
    const signed char CEN              = 8; // EN was read
    const signed char CN               = 9; // N was read
    const signed char Done             = 10;

    const signed char InpLt            = 0; // <
    const signed char InpQm            = 1; // ?
    const signed char InpEm            = 2; // !
    const signed char InpDash          = 3; // -
    const signed char InpA             = 4; // A
    const signed char InpE             = 5; // E
    const signed char InpL             = 6; // L
    const signed char InpN             = 7; // N
    const signed char InpUnknown       = 8;

    static const signed char table[4][9] = {
     /*  InpLt  InpQm  InpEm  InpDash  InpA   InpE   InpL   InpN   InpUnknown */
	{ Lt,    -1,    -1,    -1,      -1,    -1,    -1,    -1,    -1     }, // Init
	{ -1,    Qm,    Em,    -1,      -1,    -1,    -1,    -1,    -1     }, // Lt
	{ -1,    -1,    -1,    Dash,    CA,    CE,    -1,    CN,    -1     }, // Em
	{ -1,    -1,    -1,    -1,      -1,    -1,    CEL,   CEN,   -1     }  // CE
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseMarkupdecl (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseMarkupdecl, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case Qm:
		if ( contentHnd ) {
		    if ( !contentHnd->processingInstruction(name(),string()) ) {
			reportParseError( contentHnd->errorString() );
			return FALSE;
		    }
		}
		return TRUE;
	    case Dash:
		if ( lexicalHnd ) {
		    if ( !lexicalHnd->comment( string() ) ) {
			reportParseError( lexicalHnd->errorString() );
			return FALSE;
		    }
		}
		return TRUE;
	    case CA:
		return TRUE;
	    case CEL:
		return TRUE;
	    case CEN:
		return TRUE;
	    case CN:
		return TRUE;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseMarkupdecl, state );
	    return FALSE;
	}
	if        ( c.unicode() == '<' ) {
	    input = InpLt;
	} else if ( c.unicode() == '?' ) {
	    input = InpQm;
	} else if ( c.unicode() == '!' ) {
	    input = InpEm;
	} else if ( c.unicode() == '-' ) {
	    input = InpDash;
	} else if ( c.unicode() == 'A' ) {
	    input = InpA;
	} else if ( c.unicode() == 'E' ) {
	    input = InpE;
	} else if ( c.unicode() == 'L' ) {
	    input = InpL;
	} else if ( c.unicode() == 'N' ) {
	    input = InpN;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Lt:
		next();
		break;
	    case Em:
		next();
		break;
	    case CE:
		next();
		break;
	    case Qm:
		d->parsePI_xmldecl = FALSE;
		if ( !parsePI() ) {
		    parseFailed( &QXmlSimpleReader::parseMarkupdecl, state );
		    return FALSE;
		}
		break;
	    case Dash:
		if ( !parseComment() ) {
		    parseFailed( &QXmlSimpleReader::parseMarkupdecl, state );
		    return FALSE;
		}
		break;
	    case CA:
		if ( !parseAttlistDecl() ) {
		    parseFailed( &QXmlSimpleReader::parseMarkupdecl, state );
		    return FALSE;
		}
		break;
	    case CEL:
		if ( !parseElementDecl() ) {
		    parseFailed( &QXmlSimpleReader::parseMarkupdecl, state );
		    return FALSE;
		}
		break;
	    case CEN:
		if ( !parseEntityDecl() ) {
		    parseFailed( &QXmlSimpleReader::parseMarkupdecl, state );
		    return FALSE;
		}
		break;
	    case CN:
		if ( !parseNotationDecl() ) {
		    parseFailed( &QXmlSimpleReader::parseMarkupdecl, state );
		    return FALSE;
		}
		break;
	}
    }
}

/*
  Parse a PEReference [69]
*/
bool QXmlSimpleReader::parsePEReference()
{
    const signed char Init             = 0;
    const signed char Next             = 1;
    const signed char Name             = 2;
    const signed char NameR            = 3; // same as Name, but already reported
    const signed char Done             = 4;

    const signed char InpSemi          = 0; // ;
    const signed char InpPer           = 1; // %
    const signed char InpUnknown       = 2;

    static const signed char table[4][3] = {
     /*  InpSemi  InpPer  InpUnknown */
	{ -1,      Next,   -1    }, // Init
	{ -1,      -1,     Name  }, // Next
	{ Done,    -1,     -1    }, // Name
	{ Done,    -1,     -1    }  // NameR
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parsePEReference (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parsePEReference, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case Name:
		{
		    bool skipIt = TRUE;
		    QString xmlRefString;

		    QMap<QString,QString>::Iterator it;
		    it = d->parameterEntities.find( ref() );
		    if ( it != d->parameterEntities.end() ) {
			skipIt = FALSE;
			xmlRefString = it.data();
		    } else if ( entityRes ) {
			QMap<QString,QXmlSimpleReaderPrivate::ExternParameterEntity>::Iterator it2;
			it2 = d->externParameterEntities.find( ref() );
			QXmlInputSource *ret = 0;
			if ( it2 != d->externParameterEntities.end() ) {
			    if ( !entityRes->resolveEntity( it2.data().publicId, it2.data().systemId, ret ) ) {
				delete ret;
				reportParseError( entityRes->errorString() );
				return FALSE;
			    }
			    if ( ret ) {
				xmlRefString = ret->data();
				delete ret;
				if ( !stripTextDecl( xmlRefString ) ) {
				    reportParseError( XMLERR_ERRORINTEXTDECL );
				    return FALSE;
				}
				skipIt = FALSE;
			    }
			}
		    }

		    if ( skipIt ) {
			if ( contentHnd ) {
			    if ( !contentHnd->skippedEntity( QString("%") + ref() ) ) {
				reportParseError( contentHnd->errorString() );
				return FALSE;
			    }
			}
		    } else {
			if ( d->parsePEReference_context == InEntityValue ) {
			    // Included in literal
			    if ( !insertXmlRef( xmlRefString, ref(), TRUE ) )
				return FALSE;
			} else if ( d->parsePEReference_context == InDTD ) {
			    // Included as PE
			    if ( !insertXmlRef( QString(" ")+xmlRefString+QString(" "), ref(), FALSE ) )
				return FALSE;
			}
		    }
		}
		state = NameR;
		break;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parsePEReference, state );
	    return FALSE;
	}
	if        ( c.unicode() == ';' ) {
	    input = InpSemi;
	} else if ( c.unicode() == '%' ) {
	    input = InpPer;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Next:
		next();
		break;
	    case Name:
	    case NameR:
		d->parseName_useRef = TRUE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parsePEReference, state );
		    return FALSE;
		}
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a AttlistDecl [52].

  Precondition: the beginning '<!' is already read and the head
  stands on the 'A' of '<!ATTLIST'
*/
bool QXmlSimpleReader::parseAttlistDecl()
{
    const signed char Init             =  0;
    const signed char Attlist          =  1; // parse the string "ATTLIST"
    const signed char Ws               =  2; // whitespace read
    const signed char Name             =  3; // parse name
    const signed char Ws1              =  4; // whitespace read
    const signed char Attdef           =  5; // parse the AttDef
    const signed char Ws2              =  6; // whitespace read
    const signed char Atttype          =  7; // parse the AttType
    const signed char Ws3              =  8; // whitespace read
    const signed char DDecH            =  9; // DefaultDecl with #
    const signed char DefReq           = 10; // parse the string "REQUIRED"
    const signed char DefImp           = 11; // parse the string "IMPLIED"
    const signed char DefFix           = 12; // parse the string "FIXED"
    const signed char Attval           = 13; // parse the AttValue
    const signed char Ws4              = 14; // whitespace read
    const signed char Done             = 15;

    const signed char InpWs            = 0; // white space
    const signed char InpGt            = 1; // >
    const signed char InpHash          = 2; // #
    const signed char InpA             = 3; // A
    const signed char InpI             = 4; // I
    const signed char InpF             = 5; // F
    const signed char InpR             = 6; // R
    const signed char InpUnknown       = 7;

    static const signed char table[15][8] = {
     /*  InpWs    InpGt    InpHash  InpA      InpI     InpF     InpR     InpUnknown */
	{ -1,      -1,      -1,      Attlist,  -1,      -1,      -1,      -1      }, // Init
	{ Ws,      -1,      -1,      -1,       -1,      -1,      -1,      -1      }, // Attlist
	{ -1,      -1,      -1,      Name,     Name,    Name,    Name,    Name    }, // Ws
	{ Ws1,     Done,    Attdef,  Attdef,   Attdef,  Attdef,  Attdef,  Attdef  }, // Name
	{ -1,      Done,    Attdef,  Attdef,   Attdef,  Attdef,  Attdef,  Attdef  }, // Ws1
	{ Ws2,     -1,      -1,      -1,       -1,      -1,      -1,      -1      }, // Attdef
	{ -1,      Atttype, Atttype, Atttype,  Atttype, Atttype, Atttype, Atttype }, // Ws2
	{ Ws3,     -1,      -1,      -1,       -1,      -1,      -1,      -1      }, // Attype
	{ -1,      Attval,  DDecH,   Attval,   Attval,  Attval,  Attval,  Attval  }, // Ws3
	{ -1,      -1,      -1,      -1,       DefImp,  DefFix,  DefReq,  -1      }, // DDecH
	{ Ws4,     Ws4,     -1,      -1,       -1,      -1,      -1,      -1      }, // DefReq
	{ Ws4,     Ws4,     -1,      -1,       -1,      -1,      -1,      -1      }, // DefImp
	{ Ws3,     -1,      -1,      -1,       -1,      -1,      -1,      -1      }, // DefFix
	{ Ws4,     Ws4,     -1,      -1,       -1,      -1,      -1,      -1      }, // Attval
	{ -1,      Done,    Attdef,  Attdef,   Attdef,  Attdef,  Attdef,  Attdef  }  // Ws4
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseAttlistDecl (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case Name:
		d->attDeclEName = name();
		break;
	    case Attdef:
		d->attDeclAName = name();
		break;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseAttlistDecl, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c.unicode() == '>' ) {
	    input = InpGt;
	} else if ( c.unicode() == '#' ) {
	    input = InpHash;
	} else if ( c.unicode() == 'A' ) {
	    input = InpA;
	} else if ( c.unicode() == 'I' ) {
	    input = InpI;
	} else if ( c.unicode() == 'F' ) {
	    input = InpF;
	} else if ( c.unicode() == 'R' ) {
	    input = InpR;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Attlist:
		d->parseString_s = "ATTLIST";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case Ws:
	    case Ws1:
	    case Ws2:
	    case Ws3:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case Name:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case Attdef:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case Atttype:
		if ( !parseAttType() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case DDecH:
		next();
		break;
	    case DefReq:
		d->parseString_s = "REQUIRED";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case DefImp:
		d->parseString_s = "IMPLIED";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case DefFix:
		d->parseString_s = "FIXED";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case Attval:
		if ( !parseAttValue() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case Ws4:
		if ( declHnd ) {
		    // ### not all values are computed yet...
		    if ( !declHnd->attributeDecl( d->attDeclEName, d->attDeclAName, "", "", "" ) ) {
			reportParseError( declHnd->errorString() );
			return FALSE;
		    }
		}
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttlistDecl, state );
		    return FALSE;
		}
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a AttType [54]
*/
bool QXmlSimpleReader::parseAttType()
{
    const signed char Init             =  0;
    const signed char ST               =  1; // StringType
    const signed char TTI              =  2; // TokenizedType starting with 'I'
    const signed char TTI2             =  3; // TokenizedType helpstate
    const signed char TTI3             =  4; // TokenizedType helpstate
    const signed char TTE              =  5; // TokenizedType starting with 'E'
    const signed char TTEY             =  6; // TokenizedType starting with 'ENTITY'
    const signed char TTEI             =  7; // TokenizedType starting with 'ENTITI'
    const signed char N                =  8; // N read (TokenizedType or Notation)
    const signed char TTNM             =  9; // TokenizedType starting with 'NM'
    const signed char TTNM2            = 10; // TokenizedType helpstate
    const signed char NO               = 11; // Notation
    const signed char NO2              = 12; // Notation helpstate
    const signed char NO3              = 13; // Notation helpstate
    const signed char NOName           = 14; // Notation, read name
    const signed char NO4              = 15; // Notation helpstate
    const signed char EN               = 16; // Enumeration
    const signed char ENNmt            = 17; // Enumeration, read Nmtoken
    const signed char EN2              = 18; // Enumeration helpstate
    const signed char ADone            = 19; // almost done (make next and accept)
    const signed char Done             = 20;

    const signed char InpWs            =  0; // whitespace
    const signed char InpOp            =  1; // (
    const signed char InpCp            =  2; // )
    const signed char InpPipe          =  3; // |
    const signed char InpC             =  4; // C
    const signed char InpE             =  5; // E
    const signed char InpI             =  6; // I
    const signed char InpM             =  7; // M
    const signed char InpN             =  8; // N
    const signed char InpO             =  9; // O
    const signed char InpR             = 10; // R
    const signed char InpS             = 11; // S
    const signed char InpY             = 12; // Y
    const signed char InpUnknown       = 13;

    static const signed char table[19][14] = {
     /*  InpWs    InpOp    InpCp    InpPipe  InpC     InpE     InpI     InpM     InpN     InpO     InpR     InpS     InpY     InpUnknown */
	{ -1,      EN,      -1,      -1,      ST,      TTE,     TTI,     -1,      N,       -1,      -1,      -1,      -1,      -1     }, // Init
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done   }, // ST
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    TTI2,    Done,    Done,    Done   }, // TTI
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    TTI3,    Done,    Done   }, // TTI2
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done   }, // TTI3
	{ -1,      -1,      -1,      -1,      -1,      -1,      TTEI,    -1,      -1,      -1,      -1,      -1,      TTEY,    -1     }, // TTE
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done   }, // TTEY
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done   }, // TTEI
	{ -1,      -1,      -1,      -1,      -1,      -1,      -1,      TTNM,    -1,      NO,      -1,      -1,      -1,      -1     }, // N
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    TTNM2,   Done,    Done   }, // TTNM
	{ Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done   }, // TTNM2
	{ NO2,     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }, // NO
	{ -1,      NO3,     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }, // NO2
	{ NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName }, // NO3
	{ NO4,     -1,      ADone,   NO3,     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }, // NOName
	{ -1,      -1,      ADone,   NO3,     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }, // NO4
	{ -1,      -1,      ENNmt,   -1,      ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt  }, // EN
	{ EN2,     -1,      ADone,   EN,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }, // ENNmt
	{ -1,      -1,      ADone,   EN,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }  // EN2
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseAttType (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseAttType, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case ADone:
		return TRUE;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseAttType, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c.unicode() == '(' ) {
	    input = InpOp;
	} else if ( c.unicode() == ')' ) {
	    input = InpCp;
	} else if ( c.unicode() == '|' ) {
	    input = InpPipe;
	} else if ( c.unicode() == 'C' ) {
	    input = InpC;
	} else if ( c.unicode() == 'E' ) {
	    input = InpE;
	} else if ( c.unicode() == 'I' ) {
	    input = InpI;
	} else if ( c.unicode() == 'M' ) {
	    input = InpM;
	} else if ( c.unicode() == 'N' ) {
	    input = InpN;
	} else if ( c.unicode() == 'O' ) {
	    input = InpO;
	} else if ( c.unicode() == 'R' ) {
	    input = InpR;
	} else if ( c.unicode() == 'S' ) {
	    input = InpS;
	} else if ( c.unicode() == 'Y' ) {
	    input = InpY;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case ST:
		d->parseString_s = "CDATA";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case TTI:
		d->parseString_s = "ID";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case TTI2:
		d->parseString_s = "REF";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case TTI3:
		next(); // S
		break;
	    case TTE:
		d->parseString_s = "ENTIT";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case TTEY:
		next(); // Y
		break;
	    case TTEI:
		d->parseString_s = "IES";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case N:
		next(); // N
		break;
	    case TTNM:
		d->parseString_s = "MTOKEN";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case TTNM2:
		next(); // S
		break;
	    case NO:
		d->parseString_s = "OTATION";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case NO2:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case NO3:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case NOName:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case NO4:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case EN:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case ENNmt:
		if ( !parseNmtoken() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case EN2:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttType, state );
		    return FALSE;
		}
		break;
	    case ADone:
		next();
		break;
	}
    }
}

/*
  Parse a AttValue [10]

  Precondition: the head stands on the beginning " or '

  If this function was successful, the head stands on the first
  character after the closing " or ' and the value of the attribute
  is in string().
*/
bool QXmlSimpleReader::parseAttValue()
{
    const signed char Init             = 0;
    const signed char Dq               = 1; // double quotes were read
    const signed char DqRef            = 2; // read references in double quotes
    const signed char DqC              = 3; // signed character read in double quotes
    const signed char Sq               = 4; // single quotes were read
    const signed char SqRef            = 5; // read references in single quotes
    const signed char SqC              = 6; // signed character read in single quotes
    const signed char Done             = 7;

    const signed char InpDq            = 0; // "
    const signed char InpSq            = 1; // '
    const signed char InpAmp           = 2; // &
    const signed char InpLt            = 3; // <
    const signed char InpUnknown       = 4;

    static const signed char table[7][5] = {
     /*  InpDq  InpSq  InpAmp  InpLt InpUnknown */
	{ Dq,    Sq,    -1,     -1,   -1    }, // Init
	{ Done,  DqC,   DqRef,  -1,   DqC   }, // Dq
	{ Done,  DqC,   DqRef,  -1,   DqC   }, // DqRef
	{ Done,  DqC,   DqRef,  -1,   DqC   }, // DqC
	{ SqC,   Done,  SqRef,  -1,   SqC   }, // Sq
	{ SqC,   Done,  SqRef,  -1,   SqC   }, // SqRef
	{ SqC,   Done,  SqRef,  -1,   SqC   }  // SqRef
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseAttValue (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseAttValue, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseAttValue, state );
	    return FALSE;
	}
	if        ( c.unicode() == '"' ) {
	    input = InpDq;
	} else if ( c.unicode() == '\'' ) {
	    input = InpSq;
	} else if ( c.unicode() == '&' ) {
	    input = InpAmp;
	} else if ( c.unicode() == '<' ) {
	    input = InpLt;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Dq:
	    case Sq:
		stringClear();
		next();
		break;
	    case DqRef:
	    case SqRef:
		d->parseReference_context = InAttributeValue;
		if ( !parseReference() ) {
		    parseFailed( &QXmlSimpleReader::parseAttValue, state );
		    return FALSE;
		}
		break;
	    case DqC:
	    case SqC:
		stringAddC();
		next();
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a elementdecl [45].

  Precondition: the beginning '<!E' is already read and the head
  stands on the 'L' of '<!ELEMENT'
*/
bool QXmlSimpleReader::parseElementDecl()
{
    const signed char Init             =  0;
    const signed char Elem             =  1; // parse the beginning string
    const signed char Ws1              =  2; // whitespace required
    const signed char Nam              =  3; // parse Name
    const signed char Ws2              =  4; // whitespace required
    const signed char Empty            =  5; // read EMPTY
    const signed char Any              =  6; // read ANY
    const signed char Cont             =  7; // read contentspec (except ANY or EMPTY)
    const signed char Mix              =  8; // read Mixed
    const signed char Mix2             =  9; //
    const signed char Mix3             = 10; //
    const signed char MixN1            = 11; //
    const signed char MixN2            = 12; //
    const signed char MixN3            = 13; //
    const signed char MixN4            = 14; //
    const signed char Cp               = 15; // parse cp
    const signed char Cp2              = 16; //
    const signed char WsD              = 17; // eat whitespace before Done
    const signed char Done             = 18;

    const signed char InpWs            =  0;
    const signed char InpGt            =  1; // >
    const signed char InpPipe          =  2; // |
    const signed char InpOp            =  3; // (
    const signed char InpCp            =  4; // )
    const signed char InpHash          =  5; // #
    const signed char InpQm            =  6; // ?
    const signed char InpAst           =  7; // *
    const signed char InpPlus          =  8; // +
    const signed char InpA             =  9; // A
    const signed char InpE             = 10; // E
    const signed char InpL             = 11; // L
    const signed char InpUnknown       = 12;

    static const signed char table[18][13] = {
     /*  InpWs   InpGt  InpPipe  InpOp  InpCp   InpHash  InpQm  InpAst  InpPlus  InpA    InpE    InpL    InpUnknown */
	{ -1,     -1,    -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     Elem,   -1     }, // Init
	{ Ws1,    -1,    -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Elem
	{ -1,     -1,    -1,      -1,    -1,     -1,      -1,    -1,     -1,      Nam,    Nam,    Nam,    Nam    }, // Ws1
	{ Ws2,    -1,    -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Nam
	{ -1,     -1,    -1,      Cont,  -1,     -1,      -1,    -1,     -1,      Any,    Empty,  -1,     -1     }, // Ws2
	{ WsD,    Done,  -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Empty
	{ WsD,    Done,  -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Any
	{ -1,     -1,    -1,      Cp,    Cp,     Mix,     -1,    -1,     -1,      Cp,     Cp,     Cp,     Cp     }, // Cont
	{ Mix2,   -1,    MixN1,   -1,    Mix3,   -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Mix
	{ -1,     -1,    MixN1,   -1,    Mix3,   -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Mix2
	{ WsD,    Done,  -1,      -1,    -1,     -1,      -1,    WsD,    -1,      -1,     -1,     -1,     -1     }, // Mix3
	{ -1,     -1,    -1,      -1,    -1,     -1,      -1,    -1,     -1,      MixN2,  MixN2,  MixN2,  MixN2  }, // MixN1
	{ MixN3,  -1,    MixN1,   -1,    MixN4,  -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // MixN2
	{ -1,     -1,    MixN1,   -1,    MixN4,  -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // MixN3
	{ -1,     -1,    -1,      -1,    -1,     -1,      -1,    WsD,    -1,      -1,     -1,     -1,     -1     }, // MixN4
	{ WsD,    Done,  -1,      -1,    -1,     -1,      Cp2,   Cp2,    Cp2,     -1,     -1,     -1,     -1     }, // Cp
	{ WsD,    Done,  -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Cp2
	{ -1,     Done,  -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }  // WsD
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseElementDecl (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case Done:
		return TRUE;
	    case -1:
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseElementDecl, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c.unicode() == '>' ) {
	    input = InpGt;
	} else if ( c.unicode() == '|' ) {
	    input = InpPipe;
	} else if ( c.unicode() == '(' ) {
	    input = InpOp;
	} else if ( c.unicode() == ')' ) {
	    input = InpCp;
	} else if ( c.unicode() == '#' ) {
	    input = InpHash;
	} else if ( c.unicode() == '?' ) {
	    input = InpQm;
	} else if ( c.unicode() == '*' ) {
	    input = InpAst;
	} else if ( c.unicode() == '+' ) {
	    input = InpPlus;
	} else if ( c.unicode() == 'A' ) {
	    input = InpA;
	} else if ( c.unicode() == 'E' ) {
	    input = InpE;
	} else if ( c.unicode() == 'L' ) {
	    input = InpL;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Elem:
		d->parseString_s = "LEMENT";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Ws1:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Nam:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Ws2:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Empty:
		d->parseString_s = "EMPTY";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Any:
		d->parseString_s = "ANY";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Cont:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Mix:
		d->parseString_s = "#PCDATA";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Mix2:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Mix3:
		next();
		break;
	    case MixN1:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case MixN2:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case MixN3:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case MixN4:
		next();
		break;
	    case Cp:
		if ( !parseChoiceSeq() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Cp2:
		next();
		break;
	    case WsD:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseElementDecl, state );
		    return FALSE;
		}
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a NotationDecl [82].

  Precondition: the beginning '<!' is already read and the head
  stands on the 'N' of '<!NOTATION'
*/
bool QXmlSimpleReader::parseNotationDecl()
{
    const signed char Init             = 0;
    const signed char Not              = 1; // read NOTATION
    const signed char Ws1              = 2; // eat whitespaces
    const signed char Nam              = 3; // read Name
    const signed char Ws2              = 4; // eat whitespaces
    const signed char ExtID            = 5; // parse ExternalID
    const signed char ExtIDR           = 6; // same as ExtID, but already reported
    const signed char Ws3              = 7; // eat whitespaces
    const signed char Done             = 8;

    const signed char InpWs            = 0;
    const signed char InpGt            = 1; // >
    const signed char InpN             = 2; // N
    const signed char InpUnknown       = 3;

    static const signed char table[8][4] = {
     /*  InpWs   InpGt  InpN    InpUnknown */
	{ -1,     -1,    Not,    -1     }, // Init
	{ Ws1,    -1,    -1,     -1     }, // Not
	{ -1,     -1,    Nam,    Nam    }, // Ws1
	{ Ws2,    Done,  -1,     -1     }, // Nam
	{ -1,     Done,  ExtID,  ExtID  }, // Ws2
	{ Ws3,    Done,  -1,     -1     }, // ExtID
	{ Ws3,    Done,  -1,     -1     }, // ExtIDR
	{ -1,     Done,  -1,     -1     }  // Ws3
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseNotationDecl (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseNotationDecl, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case ExtID:
		// call the handler
		if ( dtdHnd ) {
		    if ( !dtdHnd->notationDecl( name(), d->publicId, d->systemId ) ) {
			reportParseError( dtdHnd->errorString() );
			return FALSE;
		    }
		}
		state = ExtIDR;
		break;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseNotationDecl, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c.unicode() == '>' ) {
	    input = InpGt;
	} else if ( c.unicode() == 'N' ) {
	    input = InpN;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Not:
		d->parseString_s = "NOTATION";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseNotationDecl, state );
		    return FALSE;
		}
		break;
	    case Ws1:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseNotationDecl, state );
		    return FALSE;
		}
		break;
	    case Nam:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseNotationDecl, state );
		    return FALSE;
		}
		break;
	    case Ws2:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseNotationDecl, state );
		    return FALSE;
		}
		break;
	    case ExtID:
	    case ExtIDR:
		d->parseExternalID_allowPublicID = TRUE;
		if ( !parseExternalID() ) {
		    parseFailed( &QXmlSimpleReader::parseNotationDecl, state );
		    return FALSE;
		}
		break;
	    case Ws3:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseNotationDecl, state );
		    return FALSE;
		}
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse choice [49] or seq [50].

  Precondition: the beginning '('S? is already read and the head
  stands on the first non-whitespace character after it.
*/
bool QXmlSimpleReader::parseChoiceSeq()
{
    const signed char Init             = 0;
    const signed char Ws1              = 1; // eat whitespace
    const signed char CorS             = 2; // choice or set
    const signed char Ws2              = 3; // eat whitespace
    const signed char More             = 4; // more cp to read
    const signed char Name             = 5; // read name
    const signed char Done             = 6; //

    const signed char InpWs            = 0; // S
    const signed char InpOp            = 1; // (
    const signed char InpCp            = 2; // )
    const signed char InpQm            = 3; // ?
    const signed char InpAst           = 4; // *
    const signed char InpPlus          = 5; // +
    const signed char InpPipe          = 6; // |
    const signed char InpComm          = 7; // ,
    const signed char InpUnknown       = 8;

    static const signed char table[6][9] = {
     /*  InpWs   InpOp  InpCp  InpQm  InpAst  InpPlus  InpPipe  InpComm  InpUnknown */
	{ -1,     Ws1,   -1,    -1,    -1,     -1,      -1,      -1,      Name  }, // Init
	{ -1,     CorS,  -1,    -1,    -1,     -1,      -1,      -1,      CorS  }, // Ws1
	{ Ws2,    -1,    Done,  Ws2,   Ws2,    Ws2,     More,    More,    -1    }, // CorS
	{ -1,     -1,    Done,  -1,    -1,     -1,      More,    More,    -1    }, // Ws2
	{ -1,     Ws1,   -1,    -1,    -1,     -1,      -1,      -1,      Name  }, // More (same as Init)
	{ Ws2,    -1,    Done,  Ws2,   Ws2,    Ws2,     More,    More,    -1    }  // Name (same as CorS)
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseChoiceSeq (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseChoiceSeq, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseChoiceSeq, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c.unicode() == '(' ) {
	    input = InpOp;
	} else if ( c.unicode() == ')' ) {
	    input = InpCp;
	} else if ( c.unicode() == '?' ) {
	    input = InpQm;
	} else if ( c.unicode() == '*' ) {
	    input = InpAst;
	} else if ( c.unicode() == '+' ) {
	    input = InpPlus;
	} else if ( c.unicode() == '|' ) {
	    input = InpPipe;
	} else if ( c.unicode() == ',' ) {
	    input = InpComm;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Ws1:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseChoiceSeq, state );
		    return FALSE;
		}
		break;
	    case CorS:
		if ( !parseChoiceSeq() ) {
		    parseFailed( &QXmlSimpleReader::parseChoiceSeq, state );
		    return FALSE;
		}
		break;
	    case Ws2:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseChoiceSeq, state );
		    return FALSE;
		}
		break;
	    case More:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseChoiceSeq, state );
		    return FALSE;
		}
		break;
	    case Name:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseChoiceSeq, state );
		    return FALSE;
		}
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a EntityDecl [70].

  Precondition: the beginning '<!E' is already read and the head
  stand on the 'N' of '<!ENTITY'
*/
bool QXmlSimpleReader::parseEntityDecl()
{
    const signed char Init             =  0;
    const signed char Ent              =  1; // parse "ENTITY"
    const signed char Ws1              =  2; // white space read
    const signed char Name             =  3; // parse name
    const signed char Ws2              =  4; // white space read
    const signed char EValue           =  5; // parse entity value
    const signed char EValueR          =  6; // same as EValue, but already reported
    const signed char ExtID            =  7; // parse ExternalID
    const signed char Ws3              =  8; // white space read
    const signed char Ndata            =  9; // parse "NDATA"
    const signed char Ws4              = 10; // white space read
    const signed char NNam             = 11; // parse name
    const signed char NNamR            = 12; // same as NNam, but already reported
    const signed char PEDec            = 13; // parse PEDecl
    const signed char Ws6              = 14; // white space read
    const signed char PENam            = 15; // parse name
    const signed char Ws7              = 16; // white space read
    const signed char PEVal            = 17; // parse entity value
    const signed char PEValR           = 18; // same as PEVal, but already reported
    const signed char PEEID            = 19; // parse ExternalID
    const signed char PEEIDR           = 20; // same as PEEID, but already reported
    const signed char WsE              = 21; // white space read
    const signed char Done             = 22;
    const signed char EDDone           = 23; // done, but also report an external, unparsed entity decl

    const signed char InpWs            = 0; // white space
    const signed char InpPer           = 1; // %
    const signed char InpQuot          = 2; // " or '
    const signed char InpGt            = 3; // >
    const signed char InpN             = 4; // N
    const signed char InpUnknown       = 5;

    static const signed char table[22][6] = {
     /*  InpWs  InpPer  InpQuot  InpGt  InpN    InpUnknown */
	{ -1,    -1,     -1,      -1,    Ent,    -1      }, // Init
	{ Ws1,   -1,     -1,      -1,    -1,     -1      }, // Ent
	{ -1,    PEDec,  -1,      -1,    Name,   Name    }, // Ws1
	{ Ws2,   -1,     -1,      -1,    -1,     -1      }, // Name
	{ -1,    -1,     EValue,  -1,    -1,     ExtID   }, // Ws2
	{ WsE,   -1,     -1,      Done,  -1,     -1      }, // EValue
	{ WsE,   -1,     -1,      Done,  -1,     -1      }, // EValueR
	{ Ws3,   -1,     -1,      EDDone,-1,     -1      }, // ExtID
	{ -1,    -1,     -1,      EDDone,Ndata,  -1      }, // Ws3
	{ Ws4,   -1,     -1,      -1,    -1,     -1      }, // Ndata
	{ -1,    -1,     -1,      -1,    NNam,   NNam    }, // Ws4
	{ WsE,   -1,     -1,      Done,  -1,     -1      }, // NNam
	{ WsE,   -1,     -1,      Done,  -1,     -1      }, // NNamR
	{ Ws6,   -1,     -1,      -1,    -1,     -1      }, // PEDec
	{ -1,    -1,     -1,      -1,    PENam,  PENam   }, // Ws6
	{ Ws7,   -1,     -1,      -1,    -1,     -1      }, // PENam
	{ -1,    -1,     PEVal,   -1,    -1,     PEEID   }, // Ws7
	{ WsE,   -1,     -1,      Done,  -1,     -1      }, // PEVal
	{ WsE,   -1,     -1,      Done,  -1,     -1      }, // PEValR
	{ WsE,   -1,     -1,      Done,  -1,     -1      }, // PEEID
	{ WsE,   -1,     -1,      Done,  -1,     -1      }, // PEEIDR
	{ -1,    -1,     -1,      Done,  -1,     -1      }  // WsE
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseEntityDecl (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case EValue:
		if (  !entityExist( name() ) ) {
		    d->entities.insert( name(), string() );
		    if ( declHnd ) {
			if ( !declHnd->internalEntityDecl( name(), string() ) ) {
			    reportParseError( declHnd->errorString() );
			    return FALSE;
			}
		    }
		}
		state = EValueR;
		break;
	    case NNam:
		if (  !entityExist( name() ) ) {
		    d->externEntities.insert( name(), QXmlSimpleReaderPrivate::ExternEntity( d->publicId, d->systemId, ref() ) );
		    if ( dtdHnd ) {
			if ( !dtdHnd->unparsedEntityDecl( name(), d->publicId, d->systemId, ref() ) ) {
			    reportParseError( declHnd->errorString() );
			    return FALSE;
			}
		    }
		}
		state = NNamR;
		break;
	    case PEVal:
		if (  !entityExist( name() ) ) {
		    d->parameterEntities.insert( name(), string() );
		    if ( declHnd ) {
			if ( !declHnd->internalEntityDecl( QString("%")+name(), string() ) ) {
			    reportParseError( declHnd->errorString() );
			    return FALSE;
			}
		    }
		}
		state = PEValR;
		break;
	    case PEEID:
		if (  !entityExist( name() ) ) {
		    d->externParameterEntities.insert( name(), QXmlSimpleReaderPrivate::ExternParameterEntity( d->publicId, d->systemId ) );
		    if ( declHnd ) {
			if ( !declHnd->externalEntityDecl( QString("%")+name(), d->publicId, d->systemId ) ) {
			    reportParseError( declHnd->errorString() );
			    return FALSE;
			}
		    }
		}
		state = PEEIDR;
		break;
	    case EDDone:
		if (  !entityExist( name() ) ) {
		    d->externEntities.insert( name(), QXmlSimpleReaderPrivate::ExternEntity( d->publicId, d->systemId, QString::null ) );
		    if ( declHnd ) {
			if ( !declHnd->externalEntityDecl( name(), d->publicId, d->systemId ) ) {
			    reportParseError( declHnd->errorString() );
			    return FALSE;
			}
		    }
		}
		return TRUE;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseEntityDecl, state );
	    return FALSE;
	}
	if        ( is_S(c) ) {
	    input = InpWs;
	} else if ( c.unicode() == '%' ) {
	    input = InpPer;
	} else if ( c.unicode() == '"' || c.unicode() == '\'' ) {
	    input = InpQuot;
	} else if ( c.unicode() == '>' ) {
	    input = InpGt;
	} else if ( c.unicode() == 'N' ) {
	    input = InpN;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Ent:
		d->parseString_s = "NTITY";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case Ws1:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case Name:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case Ws2:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case EValue:
	    case EValueR:
		if ( !parseEntityValue() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case ExtID:
		d->parseExternalID_allowPublicID = FALSE;
		if ( !parseExternalID() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case Ws3:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case Ndata:
		d->parseString_s = "NDATA";
		if ( !parseString() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case Ws4:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case NNam:
	    case NNamR:
		d->parseName_useRef = TRUE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case PEDec:
		next();
		break;
	    case Ws6:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case PENam:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case Ws7:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case PEVal:
	    case PEValR:
		if ( !parseEntityValue() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case PEEID:
	    case PEEIDR:
		d->parseExternalID_allowPublicID = FALSE;
		if ( !parseExternalID() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case WsE:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityDecl, state );
		    return FALSE;
		}
		break;
	    case EDDone:
		next();
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a EntityValue [9]
*/
bool QXmlSimpleReader::parseEntityValue()
{
    const signed char Init             = 0;
    const signed char Dq               = 1; // EntityValue is double quoted
    const signed char DqC              = 2; // signed character
    const signed char DqPER            = 3; // PERefence
    const signed char DqRef            = 4; // Reference
    const signed char Sq               = 5; // EntityValue is double quoted
    const signed char SqC              = 6; // signed character
    const signed char SqPER            = 7; // PERefence
    const signed char SqRef            = 8; // Reference
    const signed char Done             = 9;

    const signed char InpDq            = 0; // "
    const signed char InpSq            = 1; // '
    const signed char InpAmp           = 2; // &
    const signed char InpPer           = 3; // %
    const signed char InpUnknown       = 4;

    static const signed char table[9][5] = {
     /*  InpDq  InpSq  InpAmp  InpPer  InpUnknown */
	{ Dq,    Sq,    -1,     -1,     -1    }, // Init
	{ Done,  DqC,   DqRef,  DqPER,  DqC   }, // Dq
	{ Done,  DqC,   DqRef,  DqPER,  DqC   }, // DqC
	{ Done,  DqC,   DqRef,  DqPER,  DqC   }, // DqPER
	{ Done,  DqC,   DqRef,  DqPER,  DqC   }, // DqRef
	{ SqC,   Done,  SqRef,  SqPER,  SqC   }, // Sq
	{ SqC,   Done,  SqRef,  SqPER,  SqC   }, // SqC
	{ SqC,   Done,  SqRef,  SqPER,  SqC   }, // SqPER
	{ SqC,   Done,  SqRef,  SqPER,  SqC   }  // SqRef
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseEntityValue (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseEntityValue, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseEntityValue, state );
	    return FALSE;
	}
	if        ( c.unicode() == '"' ) {
	    input = InpDq;
	} else if ( c.unicode() == '\'' ) {
	    input = InpSq;
	} else if ( c.unicode() == '&' ) {
	    input = InpAmp;
	} else if ( c.unicode() == '%' ) {
	    input = InpPer;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Dq:
	    case Sq:
		stringClear();
		next();
		break;
	    case DqC:
	    case SqC:
		stringAddC();
		next();
		break;
	    case DqPER:
	    case SqPER:
		d->parsePEReference_context = InEntityValue;
		if ( !parsePEReference() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityValue, state );
		    return FALSE;
		}
		break;
	    case DqRef:
	    case SqRef:
		d->parseReference_context = InEntityValue;
		if ( !parseReference() ) {
		    parseFailed( &QXmlSimpleReader::parseEntityValue, state );
		    return FALSE;
		}
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
  Parse a comment [15].

  Precondition: the beginning '<!' of the comment is already read and the head
  stands on the first '-' of '<!--'.

  If this funktion was successful, the head-position is on the first
  character after the comment.
*/
bool QXmlSimpleReader::parseComment()
{
    const signed char Init             = 0;
    const signed char Dash1            = 1; // the first dash was read
    const signed char Dash2            = 2; // the second dash was read
    const signed char Com              = 3; // read comment
    const signed char Com2             = 4; // read comment (help state)
    const signed char ComE             = 5; // finished reading comment
    const signed char Done             = 6;

    const signed char InpDash          = 0; // -
    const signed char InpGt            = 1; // >
    const signed char InpUnknown       = 2;

    static const signed char table[6][3] = {
     /*  InpDash  InpGt  InpUnknown */
	{ Dash1,   -1,    -1  }, // Init
	{ Dash2,   -1,    -1  }, // Dash1
	{ Com2,    Com,   Com }, // Dash2
	{ Com2,    Com,   Com }, // Com
	{ ComE,    Com,   Com }, // Com2
	{ -1,      Done,  -1  }  // ComE
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseComment (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseComment, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case Dash2:
		stringClear();
		break;
	    case Com2:
		// if next character is not a dash than don't skip it
		if ( !atEnd() && c.unicode() != '-' )
		    stringAddC( '-' );
		break;
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_ERRORPARSINGCOMMENT );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseComment, state );
	    return FALSE;
	}
	if        ( c.unicode() == '-' ) {
	    input = InpDash;
	} else if ( c.unicode() == '>' ) {
	    input = InpGt;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case Dash1:
		next();
		break;
	    case Dash2:
		next();
		break;
	    case Com:
		stringAddC();
		next();
		break;
	    case Com2:
		next();
		break;
	    case ComE:
		next();
		break;
	    case Done:
		next();
		break;
	}
    }
}

/*
    Parse an Attribute [41].

    Precondition: the head stands on the first character of the name
    of the attribute (i.e. all whitespaces are already parsed).

    The head stand on the next character after the end quotes. The
    variable name contains the name of the attribute and the variable
    string contains the value of the attribute.
*/
bool QXmlSimpleReader::parseAttribute()
{
    const int Init             = 0;
    const int PName            = 1; // parse name
    const int Ws               = 2; // eat ws
    const int Eq               = 3; // the '=' was read
    const int Quotes           = 4; // " or ' were read

    const int InpNameBe        = 0;
    const int InpEq            = 1; // =
    const int InpDq            = 2; // "
    const int InpSq            = 3; // '
    const int InpUnknown       = 4;

    static const int table[4][5] = {
     /*  InpNameBe  InpEq  InpDq    InpSq    InpUnknown */
	{ PName,     -1,    -1,      -1,      -1    }, // Init
	{ -1,        Eq,    -1,      -1,      Ws    }, // PName
	{ -1,        Eq,    -1,      -1,      -1    }, // Ws
	{ -1,        -1,    Quotes,  Quotes,  -1    }  // Eq
    };
    int state;
    int input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseAttribute (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseAttribute, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case Quotes:
		// Done
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_UNEXPECTEDCHARACTER );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseAttribute, state );
	    return FALSE;
	}
	if        ( determineNameChar(c) == NameBeginning ) {
	    input = InpNameBe;
	} else if ( c.unicode() == '=' ) {
	    input = InpEq;
	} else if ( c.unicode() == '"' ) {
	    input = InpDq;
	} else if ( c.unicode() == '\'' ) {
	    input = InpSq;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case PName:
		d->parseName_useRef = FALSE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseAttribute, state );
		    return FALSE;
		}
		break;
	    case Ws:
		if ( !eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttribute, state );
		    return FALSE;
		}
		break;
	    case Eq:
		if ( !next_eat_ws() ) {
		    parseFailed( &QXmlSimpleReader::parseAttribute, state );
		    return FALSE;
		}
		break;
	    case Quotes:
		if ( !parseAttValue() ) {
		    parseFailed( &QXmlSimpleReader::parseAttribute, state );
		    return FALSE;
		}
		break;
	}
    }
}

/*
  Parse a Name [5] and store the name in name or ref (if useRef is TRUE).
*/
bool QXmlSimpleReader::parseName()
{
    const int Init             = 0;
    const int Name1            = 1; // parse first signed character of the name
    const int Name             = 2; // parse name
    const int Done             = 3;

    const int InpNameBe        = 0; // name beginning signed characters
    const int InpNameCh        = 1; // NameChar without InpNameBe
    const int InpUnknown       = 2;

    Q_ASSERT(InpNameBe == (int)NameBeginning);
    Q_ASSERT(InpNameCh == (int)NameNotBeginning);
    Q_ASSERT(InpUnknown == (int)NotName);

    static const int table[3][3] = {
     /*  InpNameBe  InpNameCh  InpUnknown */
	{ Name1,     -1,        -1    }, // Init
	{ Name,      Name,      Done  }, // Name1
	{ Name,      Name,      Done  }  // Name
    };
    int state;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseName (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseName, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseName, state );
	    return FALSE;
	}

        // we can safely do the (int) cast thanks to the Q_ASSERTs earlier in this function
        state = table[state][(int)fastDetermineNameChar(c)];

	switch ( state ) {
	    case Name1:
		if ( d->parseName_useRef ) {
		    refClear();
		    refAddC();
		} else {
		    nameClear();
		    nameAddC();
		}
		next();
		break;
	    case Name:
		if ( d->parseName_useRef ) {
		    refAddC();
		} else {
		    nameAddC();
		}
		next();
		break;
	}
    }
}

/*
  Parse a Nmtoken [7] and store the name in name.
*/
bool QXmlSimpleReader::parseNmtoken()
{
    const signed char Init             = 0;
    const signed char NameF            = 1;
    const signed char Name             = 2;
    const signed char Done             = 3;

    const signed char InpNameCh        = 0; // NameChar without InpNameBe
    const signed char InpUnknown       = 1;

    static const signed char table[3][2] = {
     /*  InpNameCh  InpUnknown */
	{ NameF,     -1    }, // Init
	{ Name,      Done  }, // NameF
	{ Name,      Done  }  // Name
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseNmtoken (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseNmtoken, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case Done:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_LETTEREXPECTED );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseNmtoken, state );
	    return FALSE;
	}
        if (determineNameChar(c) == NotName) {
            input = InpUnknown;
        } else {
	    input = InpNameCh;
	}
	state = table[state][input];

	switch ( state ) {
	    case NameF:
		nameClear();
		nameAddC();
		next();
		break;
	    case Name:
		nameAddC();
		next();
		break;
	}
    }
}

/*
  Parse a Reference [67].

  parseReference_charDataRead is set to TRUE if the reference must not be
  parsed. The character(s) which the reference mapped to are appended to
  string. The head stands on the first character after the reference.

  parseReference_charDataRead is set to FALSE if the reference must be parsed.
  The charachter(s) which the reference mapped to are inserted at the reference
  position. The head stands on the first character of the replacement).
*/
bool QXmlSimpleReader::parseReference()
{
    // temporary variables (only used in very local context, so they don't
    // interfere with incremental parsing)
    uint tmp;
    bool ok;

    const signed char Init             =  0;
    const signed char SRef             =  1; // start of a reference
    const signed char ChRef            =  2; // parse CharRef
    const signed char ChDec            =  3; // parse CharRef decimal
    const signed char ChHexS           =  4; // start CharRef hexadecimal
    const signed char ChHex            =  5; // parse CharRef hexadecimal
    const signed char Name             =  6; // parse name
    const signed char DoneD            =  7; // done CharRef decimal
    const signed char DoneH            =  8; // done CharRef hexadecimal
    const signed char DoneN            =  9; // done EntityRef

    const signed char InpAmp           = 0; // &
    const signed char InpSemi          = 1; // ;
    const signed char InpHash          = 2; // #
    const signed char InpX             = 3; // x
    const signed char InpNum           = 4; // 0-9
    const signed char InpHex           = 5; // a-f A-F
    const signed char InpUnknown       = 6;

    static const signed char table[8][7] = {
     /*  InpAmp  InpSemi  InpHash  InpX     InpNum  InpHex  InpUnknown */
	{ SRef,   -1,      -1,      -1,      -1,     -1,     -1    }, // Init
	{ -1,     -1,      ChRef,   Name,    Name,   Name,   Name  }, // SRef
	{ -1,     -1,      -1,      ChHexS,  ChDec,  -1,     -1    }, // ChRef
	{ -1,     DoneD,   -1,      -1,      ChDec,  -1,     -1    }, // ChDec
	{ -1,     -1,      -1,      -1,      ChHex,  ChHex,  -1    }, // ChHexS
	{ -1,     DoneH,   -1,      -1,      ChHex,  ChHex,  -1    }, // ChHex
	{ -1,     DoneN,   -1,      -1,      -1,     -1,     -1    }  // Name
    };
    signed char state;
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	d->parseReference_charDataRead = FALSE;
	state = Init;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseReference (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseReference, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	switch ( state ) {
	    case DoneD:
		return TRUE;
	    case DoneH:
		return TRUE;
	    case DoneN:
		return TRUE;
	    case -1:
		// Error
		reportParseError( XMLERR_ERRORPARSINGREFERENCE );
		return FALSE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseReference, state );
	    return FALSE;
	}
	if        ( c.row() ) {
	    input = InpUnknown;
	} else if ( c.cell() == '&' ) {
	    input = InpAmp;
	} else if ( c.cell() == ';' ) {
	    input = InpSemi;
	} else if ( c.cell() == '#' ) {
	    input = InpHash;
	} else if ( c.cell() == 'x' ) {
	    input = InpX;
	} else if ( '0' <= c.cell() && c.cell() <= '9' ) {
	    input = InpNum;
	} else if ( 'a' <= c.cell() && c.cell() <= 'f' ) {
	    input = InpHex;
	} else if ( 'A' <= c.cell() && c.cell() <= 'F' ) {
	    input = InpHex;
	} else {
	    input = InpUnknown;
	}
	state = table[state][input];

	switch ( state ) {
	    case SRef:
		refClear();
		next();
		break;
	    case ChRef:
		next();
		break;
	    case ChDec:
		refAddC();
		next();
		break;
	    case ChHexS:
		next();
		break;
	    case ChHex:
		refAddC();
		next();
		break;
	    case Name:
		// read the name into the ref
		d->parseName_useRef = TRUE;
		if ( !parseName() ) {
		    parseFailed( &QXmlSimpleReader::parseReference, state );
		    return FALSE;
		}
		break;
	    case DoneD:
		tmp = ref().toUInt( &ok, 10 );
		if ( ok ) {
		    stringAddC( QChar(tmp) );
		} else {
		    reportParseError( XMLERR_ERRORPARSINGREFERENCE );
		    return FALSE;
		}
		d->parseReference_charDataRead = TRUE;
		next();
		break;
	    case DoneH:
		tmp = ref().toUInt( &ok, 16 );
		if ( ok ) {
		    stringAddC( QChar(tmp) );
		} else {
		    reportParseError( XMLERR_ERRORPARSINGREFERENCE );
		    return FALSE;
		}
		d->parseReference_charDataRead = TRUE;
		next();
		break;
	    case DoneN:
		if ( !processReference() )
		    return FALSE;
		next();
		break;
	}
    }
}

/* This private function is only called by QDom. It avoids a data corruption bug
   whereby undefined entities in attribute values would be appended after the
   element that contained them.

   The solution is not perfect - the undefined entity reference is replaced by
   an empty string. The propper fix will come in Qt4, when SAX will be extended
   so that attribute values can be made up of multiple children, rather than just
   a single string value.
*/
void QXmlSimpleReader::setUndefEntityInAttrHack(bool b)
{
    d->undefEntityInAttrHack = b;
}

/*
  Helper function for parseReference()
*/
bool QXmlSimpleReader::processReference()
{
    QString reference = ref();
    if ( reference == "amp" ) {
	if ( d->parseReference_context == InEntityValue ) {
	    // Bypassed
	    stringAddC( '&' ); stringAddC( 'a' ); stringAddC( 'm' ); stringAddC( 'p' ); stringAddC( ';' );
	} else {
	    // Included or Included in literal
	    stringAddC( '&' );
	}
	d->parseReference_charDataRead = TRUE;
    } else if ( reference == "lt" ) {
	if ( d->parseReference_context == InEntityValue ) {
	    // Bypassed
	    stringAddC( '&' ); stringAddC( 'l' ); stringAddC( 't' ); stringAddC( ';' );
	} else {
	    // Included or Included in literal
	    stringAddC( '<' );
	}
	d->parseReference_charDataRead = TRUE;
    } else if ( reference == "gt" ) {
	if ( d->parseReference_context == InEntityValue ) {
	    // Bypassed
	    stringAddC( '&' ); stringAddC( 'g' ); stringAddC( 't' ); stringAddC( ';' );
	} else {
	    // Included or Included in literal
	    stringAddC( '>' );
	}
	d->parseReference_charDataRead = TRUE;
    } else if ( reference == "apos" ) {
	if ( d->parseReference_context == InEntityValue ) {
	    // Bypassed
	    stringAddC( '&' ); stringAddC( 'a' ); stringAddC( 'p' ); stringAddC( 'o' ); stringAddC( 's' ); stringAddC( ';' );
	} else {
	    // Included or Included in literal
	    stringAddC( '\'' );
	}
	d->parseReference_charDataRead = TRUE;
    } else if ( reference == "quot" ) {
	if ( d->parseReference_context == InEntityValue ) {
	    // Bypassed
	    stringAddC( '&' ); stringAddC( 'q' ); stringAddC( 'u' ); stringAddC( 'o' ); stringAddC( 't' ); stringAddC( ';' );
	} else {
	    // Included or Included in literal
	    stringAddC( '"' );
	}
	d->parseReference_charDataRead = TRUE;
    } else {
	QMap<QString,QString>::Iterator it;
	it = d->entities.find( reference );
	if ( it != d->entities.end() ) {
	    // "Internal General"
	    switch ( d->parseReference_context ) {
		case InContent:
		    // Included
		    if ( !insertXmlRef( it.data(), reference, FALSE ) )
			return FALSE;
		    d->parseReference_charDataRead = FALSE;
		    break;
		case InAttributeValue:
		    // Included in literal
		    if ( !insertXmlRef( it.data(), reference, TRUE ) )
			return FALSE;
		    d->parseReference_charDataRead = FALSE;
		    break;
		case InEntityValue:
		    {
			// Bypassed
			stringAddC( '&' );
			for ( int i=0; i<(int)reference.length(); i++ ) {
			    stringAddC( reference[i] );
			}
			stringAddC( ';');
			d->parseReference_charDataRead = TRUE;
		    }
		    break;
		case InDTD:
		    // Forbidden
		    d->parseReference_charDataRead = FALSE;
		    reportParseError( XMLERR_INTERNALGENERALENTITYINDTD );
		    return FALSE;
	    }
	} else {
	    QMap<QString,QXmlSimpleReaderPrivate::ExternEntity>::Iterator itExtern;
	    itExtern = d->externEntities.find( reference );
	    if ( itExtern == d->externEntities.end() ) {
		// entity not declared
		// ### check this case for conformance
		if ( d->parseReference_context == InEntityValue ) {
		    // Bypassed
		    stringAddC( '&' );
		    for ( int i=0; i<(int)reference.length(); i++ ) {
			stringAddC( reference[i] );
		    }
		    stringAddC( ';');
		    d->parseReference_charDataRead = TRUE;
		} else {
		    if ( contentHnd && !(d->parseReference_context == InAttributeValue
                             && d->undefEntityInAttrHack)) {
			if ( !contentHnd->skippedEntity( reference ) ) {
			    reportParseError( contentHnd->errorString() );
			    return FALSE; // error
			}
		    }
		}
	    } else if ( (*itExtern).notation.isNull() ) {
		// "External Parsed General"
		switch ( d->parseReference_context ) {
		    case InContent:
			{
			    // Included if validating
			    bool skipIt = TRUE;
			    if ( entityRes ) {
				QXmlInputSource *ret = 0;
				if ( !entityRes->resolveEntity( itExtern.data().publicId, itExtern.data().systemId, ret ) ) {
				    delete ret;
				    reportParseError( entityRes->errorString() );
				    return FALSE;
				}
				if ( ret ) {
				    QString xmlRefString = ret->data();
				    delete ret;
				    if ( !stripTextDecl( xmlRefString ) ) {
					reportParseError( XMLERR_ERRORINTEXTDECL );
					return FALSE;
				    }
				    if ( !insertXmlRef( xmlRefString, reference, FALSE ) )
					return FALSE;
				    skipIt = FALSE;
				}
			    }
			    if ( skipIt && contentHnd ) {
				if ( !contentHnd->skippedEntity( reference ) ) {
				    reportParseError( contentHnd->errorString() );
				    return FALSE; // error
				}
			    }
			    d->parseReference_charDataRead = FALSE;
			} break;
		    case InAttributeValue:
			// Forbidden
			d->parseReference_charDataRead = FALSE;
			reportParseError( XMLERR_EXTERNALGENERALENTITYINAV );
			return FALSE;
		    case InEntityValue:
			{
			    // Bypassed
			    stringAddC( '&' );
			    for ( int i=0; i<(int)reference.length(); i++ ) {
				stringAddC( reference[i] );
			    }
			    stringAddC( ';');
			    d->parseReference_charDataRead = TRUE;
			}
			break;
		    case InDTD:
			// Forbidden
			d->parseReference_charDataRead = FALSE;
			reportParseError( XMLERR_EXTERNALGENERALENTITYINDTD );
			return FALSE;
		}
	    } else {
		// "Unparsed"
		// ### notify for "Occurs as Attribute Value" missing (but this is no refence, anyway)
		// Forbidden
		d->parseReference_charDataRead = FALSE;
		reportParseError( XMLERR_UNPARSEDENTITYREFERENCE );
		return FALSE; // error
	    }
	}
    }
    return TRUE; // no error
}


/*
  Parses over a simple string.

  After the string was successfully parsed, the head is on the first
  character after the string.
*/
bool QXmlSimpleReader::parseString()
{
    const signed char InpCharExpected  = 0; // the character that was expected
    const signed char InpUnknown       = 1;

    signed char state; // state in this function is the position in the string s
    signed char input;

    if ( d->parseStack==0 || d->parseStack->isEmpty() ) {
	d->Done = d->parseString_s.length();
	state = 0;
    } else {
	state = d->parseStack->pop().state;
#if defined(QT_QXML_DEBUG)
	qDebug( "QXmlSimpleReader: parseString (cont) in state %d", state );
#endif
	if ( !d->parseStack->isEmpty() ) {
	    ParseFunction function = d->parseStack->top().function;
	    if ( function == &QXmlSimpleReader::eat_ws ) {
		d->parseStack->pop();
#if defined(QT_QXML_DEBUG)
		qDebug( "QXmlSimpleReader: eat_ws (cont)" );
#endif
	    }
	    if ( !(this->*function)() ) {
		parseFailed( &QXmlSimpleReader::parseString, state );
		return FALSE;
	    }
	}
    }

    for (;;) {
	if ( state == d->Done ) {
	    return TRUE;
	}

	if ( atEnd() ) {
	    unexpectedEof( &QXmlSimpleReader::parseString, state );
	    return FALSE;
	}
	if ( c == d->parseString_s[(int)state] ) {
	    input = InpCharExpected;
	} else {
	    input = InpUnknown;
	}
	if ( input == InpCharExpected ) {
	    state++;
	} else {
	    // Error
	    reportParseError( XMLERR_UNEXPECTEDCHARACTER );
	    return FALSE;
	}

	next();
    }
}

/*
  This private function inserts and reports an entity substitution. The
  substituted string is \a data and the name of the entity reference is \a
  name. If \a inLiteral is TRUE, the entity is IncludedInLiteral (i.e., " and '
  must be quoted. Otherwise they are not quoted.

  This function returns FALSE on error.
*/
bool QXmlSimpleReader::insertXmlRef( const QString &data, const QString &name, bool inLiteral )
{
    if ( inLiteral ) {
	QString tmp = data;
	d->xmlRef.push( tmp.replace( "\"", "&quot;" ).replace( "'", "&apos;" ) );
    } else {
	d->xmlRef.push( data );
    }
    d->xmlRefName.push( name );
    uint n = (uint)QMAX( d->parameterEntities.count(), d->entities.count() );
    if ( d->xmlRefName.count() > n+1 ) {
	// recursive entities
	reportParseError( XMLERR_RECURSIVEENTITIES );
	return FALSE;
    }
    if ( d->reportEntities && lexicalHnd ) {
	if ( !lexicalHnd->startEntity( name ) ) {
	    reportParseError( lexicalHnd->errorString() );
	    return FALSE;
	}
    }
    return TRUE;
}

/*
  This private function moves the cursor to the next character.
*/
void QXmlSimpleReader::next()
{
    int count = (int)d->xmlRef.count();
    while ( count != 0 ) {
	if ( d->xmlRef.top().isEmpty() ) {
	    d->xmlRef.pop_back();
	    d->xmlRefName.pop_back();
	    count--;
	} else {
	    c = d->xmlRef.top().constref( 0 );
	    d->xmlRef.top().remove( (uint)0, 1 );
	    return;
	}
    }
    // the following could be written nicer, but since it is a time-critical
    // function, rather optimize for speed
    ushort uc = c.unicode();
    if (uc == '\n') {
	c = inputSource->next();
	lineNr++;
	columnNr = -1;
    } else if ( uc == '\r' ) {
	c = inputSource->next();
	if ( c.unicode() != '\n' ) {
	    lineNr++;
	    columnNr = -1;
	}
    } else {
	c = inputSource->next();
    }
    ++columnNr;
}

/*
  This private function moves the cursor to the next non-whitespace character.
  This function does not move the cursor if the actual cursor position is a
  non-whitespace charcter.

  Returns FALSE when you use incremental parsing and this function reaches EOF
  with reading only whitespace characters. In this case it also poplulates the
  parseStack with useful information. In all other cases, this function returns
  TRUE.
*/
bool QXmlSimpleReader::eat_ws()
{
    while ( !atEnd() ) {
	if ( !is_S(c) ) {
	    return TRUE;
	}
	next();
    }
    if ( d->parseStack != 0 ) {
	unexpectedEof( &QXmlSimpleReader::eat_ws, 0 );
	return FALSE;
    }
    return TRUE;
}

bool QXmlSimpleReader::next_eat_ws()
{
    next();
    return eat_ws();
}


/*
  This private function initializes the reader. \a i is the input source to
  read the data from.
*/
void QXmlSimpleReader::init( const QXmlInputSource *i )
{
    lineNr = 0;
    columnNr = -1;
    inputSource = (QXmlInputSource *)i;
    initData();

    d->externParameterEntities.clear();
    d->parameterEntities.clear();
    d->externEntities.clear();
    d->entities.clear();

    d->tags.clear();

    d->doctype = "";
    d->xmlVersion = "";
    d->encoding = "";
    d->standalone = QXmlSimpleReaderPrivate::Unknown;
    d->error = QString::null;
}

/*
  This private function initializes the XML data related variables. Especially,
  it reads the data from the input source.
*/
void QXmlSimpleReader::initData()
{
    c = QXmlInputSource::EndOfData;
    d->xmlRef.clear();
    next();
}

/*
  Returns TRUE if a entity with the name \a e exists,
  otherwise returns FALSE.
*/
bool QXmlSimpleReader::entityExist( const QString& e ) const
{
    if (  d->parameterEntities.find(e) == d->parameterEntities.end() &&
	  d->externParameterEntities.find(e) == d->externParameterEntities.end() &&
	  d->externEntities.find(e) == d->externEntities.end() &&
	  d->entities.find(e) == d->entities.end() ) {
	return FALSE;
    } else {
	return TRUE;
    }
}

void QXmlSimpleReader::reportParseError( const QString& error )
{
    d->error = error;
    if ( errorHnd ) {
	if ( d->error.isNull() ) {
	    errorHnd->fatalError( QXmlParseException( XMLERR_OK, columnNr+1, lineNr+1 ) );
	} else {
	    errorHnd->fatalError( QXmlParseException( d->error, columnNr+1, lineNr+1 ) );
	}
    }
}

/*
  This private function is called when a parsing function encounters an
  unexpected EOF. It decides what to do (depending on incremental parsing or
  not). \a where is a pointer to the function where the error occurred and \a
  state is the parsing state in this function.
*/
void QXmlSimpleReader::unexpectedEof( ParseFunction where, int state )
{
    if ( d->parseStack == 0 ) {
	reportParseError( XMLERR_UNEXPECTEDEOF );
    } else {
	if ( c == QXmlInputSource::EndOfDocument ) {
	    reportParseError( XMLERR_UNEXPECTEDEOF );
	} else {
	    pushParseState( where, state );
	}
    }
}

/*
  This private function is called when a parse...() function returned FALSE. It
  determines if there was an error or if incremental parsing simply went out of
  data and does the right thing for the case. \a where is a pointer to the
  function where the error occurred and \a state is the parsing state in this
  function.
*/
void QXmlSimpleReader::parseFailed( ParseFunction where, int state )
{
    if ( d->parseStack!=0 && d->error.isNull() ) {
	pushParseState( where, state );
    }
}

/*
  This private function pushes the function pointer \a function and state \a
  state to the parse stack. This is used when you are doing an incremental
  parsing and reach the end of file too early.

  Only call this function when d->parseStack!=0.
*/
void QXmlSimpleReader::pushParseState( ParseFunction function, int state )
{
    QXmlSimpleReaderPrivate::ParseState ps;
    ps.function = function;
    ps.state = state;
    d->parseStack->push( ps );
}

inline static void updateValue(QString &value, const QChar *array, int &arrayPos, int &valueLen)
{
    value.setLength(valueLen + arrayPos);
    memcpy(const_cast<QChar*>(value.unicode()) + valueLen, array, arrayPos * sizeof(QChar));
    valueLen += arrayPos;
    arrayPos = 0;
}

// use buffers instead of QString::operator+= when single characters are read
const QString& QXmlSimpleReader::string()
{
    updateValue(stringValue, stringArray, stringArrayPos, d->stringValueLen);
    return stringValue;
}

const QString& QXmlSimpleReader::name()
{
    updateValue(nameValue, nameArray, nameArrayPos, d->nameValueLen);
    return nameValue;
}

const QString& QXmlSimpleReader::ref()
{
    updateValue(refValue, refArray, refArrayPos, d->refValueLen);
    return refValue;
}

void QXmlSimpleReader::stringAddC(const QChar &ch)
{
    if (stringArrayPos == 256)
        updateValue(stringValue, stringArray, stringArrayPos, d->stringValueLen);
    stringArray[stringArrayPos++] = ch;
}
void QXmlSimpleReader::nameAddC(const QChar &ch)
{
    if (nameArrayPos == 256)
        updateValue(nameValue, nameArray, nameArrayPos, d->nameValueLen);
    nameArray[nameArrayPos++] = ch;
}

void QXmlSimpleReader::refAddC(const QChar &ch)
{
    if (refArrayPos == 256)
        updateValue(refValue, refArray, refArrayPos, d->refValueLen);
    refArray[refArrayPos++] = ch;
}

void QXmlSimpleReader::stringClear()
{
    d->stringValueLen = 0; stringArrayPos = 0;
}


#endif //QT_NO_XML
