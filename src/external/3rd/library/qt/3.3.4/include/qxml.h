/****************************************************************************
** $Id: qt/qxml.h   3.3.4   edited Jul 15 2004 $
**
** Definition of QXmlSimpleReader and related classes.
**
** Created : 000518
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the xml module of the Qt GUI Toolkit.
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

#ifndef QXML_H
#define QXML_H

#ifndef QT_H
#include "qtextstream.h"
#include "qfile.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qvaluevector.h"
#endif // QT_H

#if !defined(QT_MODULE_XML) || defined( QT_LICENSE_PROFESSIONAL ) || defined( QT_INTERNAL_XML )
#define QM_EXPORT_XML
#else
#define QM_EXPORT_XML Q_EXPORT
#endif

#ifndef QT_NO_XML

class QXmlNamespaceSupport;
class QXmlAttributes;
class QXmlContentHandler;
class QXmlDefaultHandler;
class QXmlDTDHandler;
class QXmlEntityResolver;
class QXmlErrorHandler;
class QXmlLexicalHandler;
class QXmlDeclHandler;
class QXmlInputSource;
class QXmlLocator;
class QXmlNamespaceSupport;
class QXmlParseException;

class QXmlReader;
class QXmlSimpleReader;

class QXmlSimpleReaderPrivate;
class QXmlNamespaceSupportPrivate;
class QXmlAttributesPrivate;
class QXmlInputSourcePrivate;
class QXmlParseExceptionPrivate;
class QXmlLocatorPrivate;
class QXmlDefaultHandlerPrivate;


//
// SAX Namespace Support
//

class QM_EXPORT_XML QXmlNamespaceSupport
{
public:
    QXmlNamespaceSupport();
    ~QXmlNamespaceSupport();

    void setPrefix( const QString&, const QString& );

    QString prefix( const QString& ) const;
    QString uri( const QString& ) const;
    void splitName( const QString&, QString&, QString& ) const;
    void processName( const QString&, bool, QString&, QString& ) const;
    QStringList prefixes() const;
    QStringList prefixes( const QString& ) const;

    void pushContext();
    void popContext();
    void reset();

private:
    QXmlNamespaceSupportPrivate *d;

    friend class QXmlSimpleReader;
};


//
// SAX Attributes
//

class QM_EXPORT_XML QXmlAttributes
{
public:
    QXmlAttributes() {}
    virtual ~QXmlAttributes() {}

    int index( const QString& qName ) const;
    int index( const QString& uri, const QString& localPart ) const;
    int length() const;
    int count() const;
    QString localName( int index ) const;
    QString qName( int index ) const;
    QString uri( int index ) const;
    QString type( int index ) const;
    QString type( const QString& qName ) const;
    QString type( const QString& uri, const QString& localName ) const;
    QString value( int index ) const;
    QString value( const QString& qName ) const;
    QString value( const QString& uri, const QString& localName ) const;

    void clear();
    void append( const QString &qName, const QString &uri, const QString &localPart, const QString &value );

private:
    QStringList qnameList;
    QStringList uriList;
    QStringList localnameList;
    QStringList valueList;

    QXmlAttributesPrivate *d;
};

//
// SAX Input Source
//

class QM_EXPORT_XML QXmlInputSource
{
public:
    QXmlInputSource();
    QXmlInputSource( QIODevice *dev );
    QXmlInputSource( QFile& file ); // obsolete
    QXmlInputSource( QTextStream& stream ); // obsolete
    virtual ~QXmlInputSource();

    virtual void setData( const QString& dat );
    virtual void setData( const QByteArray& dat );
    virtual void fetchData();
    virtual QString data();
    virtual QChar next();
    virtual void reset();

    static const QChar EndOfData;
    static const QChar EndOfDocument;

protected:
    virtual QString fromRawData( const QByteArray &data, bool beginning = FALSE );

private:
    void init();

    QIODevice *inputDevice;
    QTextStream *inputStream;

    QString str;
    const QChar *unicode;
    int pos;
    int length;
    bool nextReturnedEndOfData;
    QTextDecoder *encMapper;

    QXmlInputSourcePrivate *d;
};

//
// SAX Exception Classes
//

class QM_EXPORT_XML QXmlParseException
{
public:
    QXmlParseException( const QString& name="", int c=-1, int l=-1, const QString& p="", const QString& s="" )
	: msg( name ), column( c ), line( l ), pub( p ), sys( s )
    { }

    int columnNumber() const;
    int lineNumber() const;
    QString publicId() const;
    QString systemId() const;
    QString message() const;

private:
    QString msg;
    int column;
    int line;
    QString pub;
    QString sys;

    QXmlParseExceptionPrivate *d;
};


//
// XML Reader
//

class QM_EXPORT_XML QXmlReader
{
public:
    virtual bool feature( const QString& name, bool *ok = 0 ) const = 0;
    virtual void setFeature( const QString& name, bool value ) = 0;
    virtual bool hasFeature( const QString& name ) const = 0;
    virtual void* property( const QString& name, bool *ok = 0 ) const = 0;
    virtual void setProperty( const QString& name, void* value ) = 0;
    virtual bool hasProperty( const QString& name ) const = 0;
    virtual void setEntityResolver( QXmlEntityResolver* handler ) = 0;
    virtual QXmlEntityResolver* entityResolver() const = 0;
    virtual void setDTDHandler( QXmlDTDHandler* handler ) = 0;
    virtual QXmlDTDHandler* DTDHandler() const = 0;
    virtual void setContentHandler( QXmlContentHandler* handler ) = 0;
    virtual QXmlContentHandler* contentHandler() const = 0;
    virtual void setErrorHandler( QXmlErrorHandler* handler ) = 0;
    virtual QXmlErrorHandler* errorHandler() const = 0;
    virtual void setLexicalHandler( QXmlLexicalHandler* handler ) = 0;
    virtual QXmlLexicalHandler* lexicalHandler() const = 0;
    virtual void setDeclHandler( QXmlDeclHandler* handler ) = 0;
    virtual QXmlDeclHandler* declHandler() const = 0;
    virtual bool parse( const QXmlInputSource& input ) = 0;
    virtual bool parse( const QXmlInputSource* input ) = 0;
};

class QM_EXPORT_XML QXmlSimpleReader : public QXmlReader
{
public:
    QXmlSimpleReader();
    virtual ~QXmlSimpleReader();

    bool feature( const QString& name, bool *ok = 0 ) const;
    void setFeature( const QString& name, bool value );
    bool hasFeature( const QString& name ) const;

    void* property( const QString& name, bool *ok = 0 ) const;
    void setProperty( const QString& name, void* value );
    bool hasProperty( const QString& name ) const;

    void setEntityResolver( QXmlEntityResolver* handler );
    QXmlEntityResolver* entityResolver() const;
    void setDTDHandler( QXmlDTDHandler* handler );
    QXmlDTDHandler* DTDHandler() const;
    void setContentHandler( QXmlContentHandler* handler );
    QXmlContentHandler* contentHandler() const;
    void setErrorHandler( QXmlErrorHandler* handler );
    QXmlErrorHandler* errorHandler() const;
    void setLexicalHandler( QXmlLexicalHandler* handler );
    QXmlLexicalHandler* lexicalHandler() const;
    void setDeclHandler( QXmlDeclHandler* handler );
    QXmlDeclHandler* declHandler() const;

    bool parse( const QXmlInputSource& input );
    bool parse( const QXmlInputSource* input );
    virtual bool parse( const QXmlInputSource* input, bool incremental );
    virtual bool parseContinue();

private:
    // variables
    QXmlContentHandler *contentHnd;
    QXmlErrorHandler   *errorHnd;
    QXmlDTDHandler     *dtdHnd;
    QXmlEntityResolver *entityRes;
    QXmlLexicalHandler *lexicalHnd;
    QXmlDeclHandler    *declHnd;

    QXmlInputSource *inputSource;

    QChar c; // the character at reading position
    int   lineNr; // number of line
    int   columnNr; // position in line

    int     nameArrayPos;
    QChar   nameArray[256]; // only used for names
    QString nameValue; // only used for names
    int     refArrayPos;
    QChar   refArray[256]; // only used for references
    QString refValue; // only used for references
    int     stringArrayPos;
    QChar   stringArray[256]; // used for any other strings that are parsed
    QString stringValue; // used for any other strings that are parsed

    QXmlSimpleReaderPrivate* d;

    const QString &string();
    void stringClear();
    inline void stringAddC() { stringAddC(c); }
    void stringAddC(const QChar&);
    const QString& name();
    void nameClear();
    inline void nameAddC() { nameAddC(c); }
    void nameAddC(const QChar&);
    const QString& ref();
    void refClear();
    inline void refAddC() { refAddC(c); }
    void refAddC(const QChar&);

    // used by parseReference() and parsePEReference()
    enum EntityRecognitionContext { InContent, InAttributeValue, InEntityValue, InDTD };

    // private functions
    bool eat_ws();
    bool next_eat_ws();

    void next();
    bool atEnd();

    void init( const QXmlInputSource* i );
    void initData();

    bool entityExist( const QString& ) const;

    bool parseBeginOrContinue( int state, bool incremental );

    bool parseProlog();
    bool parseElement();
    bool processElementEmptyTag();
    bool processElementETagBegin2();
    bool processElementAttribute();
    bool parseMisc();
    bool parseContent();

    bool parsePI();
    bool parseDoctype();
    bool parseComment();

    bool parseName();
    bool parseNmtoken();
    bool parseAttribute();
    bool parseReference();
    bool processReference();

    bool parseExternalID();
    bool parsePEReference();
    bool parseMarkupdecl();
    bool parseAttlistDecl();
    bool parseAttType();
    bool parseAttValue();
    bool parseElementDecl();
    bool parseNotationDecl();
    bool parseChoiceSeq();
    bool parseEntityDecl();
    bool parseEntityValue();

    bool parseString();

    bool insertXmlRef( const QString&, const QString&, bool );

    bool reportEndEntities();
    void reportParseError( const QString& error );

    typedef bool (QXmlSimpleReader::*ParseFunction) ();
    void unexpectedEof( ParseFunction where, int state );
    void parseFailed( ParseFunction where, int state );
    void pushParseState( ParseFunction function, int state );

    void setUndefEntityInAttrHack(bool b);

    friend class QXmlSimpleReaderPrivate;
    friend class QXmlSimpleReaderLocator;
    friend class QDomDocumentPrivate;
};

//
// SAX Locator
//

class QM_EXPORT_XML QXmlLocator
{
public:
    QXmlLocator();
    virtual ~QXmlLocator();

    virtual int columnNumber() = 0;
    virtual int lineNumber() = 0;
//    QString getPublicId()
//    QString getSystemId()
};

//
// SAX handler classes
//

class QM_EXPORT_XML QXmlContentHandler
{
public:
    virtual void setDocumentLocator( QXmlLocator* locator ) = 0;
    virtual bool startDocument() = 0;
    virtual bool endDocument() = 0;
    virtual bool startPrefixMapping( const QString& prefix, const QString& uri ) = 0;
    virtual bool endPrefixMapping( const QString& prefix ) = 0;
    virtual bool startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts ) = 0;
    virtual bool endElement( const QString& namespaceURI, const QString& localName, const QString& qName ) = 0;
    virtual bool characters( const QString& ch ) = 0;
    virtual bool ignorableWhitespace( const QString& ch ) = 0;
    virtual bool processingInstruction( const QString& target, const QString& data ) = 0;
    virtual bool skippedEntity( const QString& name ) = 0;
    virtual QString errorString() = 0;
};

class QM_EXPORT_XML QXmlErrorHandler
{
public:
    virtual bool warning( const QXmlParseException& exception ) = 0;
    virtual bool error( const QXmlParseException& exception ) = 0;
    virtual bool fatalError( const QXmlParseException& exception ) = 0;
    virtual QString errorString() = 0;
};

class QM_EXPORT_XML QXmlDTDHandler
{
public:
    virtual bool notationDecl( const QString& name, const QString& publicId, const QString& systemId ) = 0;
    virtual bool unparsedEntityDecl( const QString& name, const QString& publicId, const QString& systemId, const QString& notationName ) = 0;
    virtual QString errorString() = 0;
};

class QM_EXPORT_XML QXmlEntityResolver
{
public:
    virtual bool resolveEntity( const QString& publicId, const QString& systemId, QXmlInputSource*& ret ) = 0;
    virtual QString errorString() = 0;
};

class QM_EXPORT_XML QXmlLexicalHandler
{
public:
    virtual bool startDTD( const QString& name, const QString& publicId, const QString& systemId ) = 0;
    virtual bool endDTD() = 0;
    virtual bool startEntity( const QString& name ) = 0;
    virtual bool endEntity( const QString& name ) = 0;
    virtual bool startCDATA() = 0;
    virtual bool endCDATA() = 0;
    virtual bool comment( const QString& ch ) = 0;
    virtual QString errorString() = 0;
};

class QM_EXPORT_XML QXmlDeclHandler
{
public:
    virtual bool attributeDecl( const QString& eName, const QString& aName, const QString& type, const QString& valueDefault, const QString& value ) = 0;
    virtual bool internalEntityDecl( const QString& name, const QString& value ) = 0;
    virtual bool externalEntityDecl( const QString& name, const QString& publicId, const QString& systemId ) = 0;
    virtual QString errorString() = 0;
};


class QM_EXPORT_XML QXmlDefaultHandler : public QXmlContentHandler, public QXmlErrorHandler, public QXmlDTDHandler, public QXmlEntityResolver, public QXmlLexicalHandler, public QXmlDeclHandler
{
public:
    QXmlDefaultHandler() { }
    virtual ~QXmlDefaultHandler() { }

    void setDocumentLocator( QXmlLocator* locator );
    bool startDocument();
    bool endDocument();
    bool startPrefixMapping( const QString& prefix, const QString& uri );
    bool endPrefixMapping( const QString& prefix );
    bool startElement( const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts );
    bool endElement( const QString& namespaceURI, const QString& localName, const QString& qName );
    bool characters( const QString& ch );
    bool ignorableWhitespace( const QString& ch );
    bool processingInstruction( const QString& target, const QString& data );
    bool skippedEntity( const QString& name );

    bool warning( const QXmlParseException& exception );
    bool error( const QXmlParseException& exception );
    bool fatalError( const QXmlParseException& exception );

    bool notationDecl( const QString& name, const QString& publicId, const QString& systemId );
    bool unparsedEntityDecl( const QString& name, const QString& publicId, const QString& systemId, const QString& notationName );

    bool resolveEntity( const QString& publicId, const QString& systemId, QXmlInputSource*& ret );

    bool startDTD( const QString& name, const QString& publicId, const QString& systemId );
    bool endDTD();
    bool startEntity( const QString& name );
    bool endEntity( const QString& name );
    bool startCDATA();
    bool endCDATA();
    bool comment( const QString& ch );

    bool attributeDecl( const QString& eName, const QString& aName, const QString& type, const QString& valueDefault, const QString& value );
    bool internalEntityDecl( const QString& name, const QString& value );
    bool externalEntityDecl( const QString& name, const QString& publicId, const QString& systemId );

    QString errorString();

private:
    QXmlDefaultHandlerPrivate *d;
};


//
// inlines
//

inline bool QXmlSimpleReader::atEnd()
{ return (c.unicode()|0x0001) == 0xffff; }
inline int QXmlAttributes::count() const
{ return length(); }


#endif //QT_NO_XML

#endif
