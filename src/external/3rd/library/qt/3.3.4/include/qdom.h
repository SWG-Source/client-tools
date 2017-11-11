/****************************************************************************
** $Id: qt/qdom.h   3.3.4   edited May 27 2003 $
**
** Definition of QDomDocument and related classes.
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

#ifndef QDOM_H
#define QDOM_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

#if !defined(QT_MODULE_XML) || defined( QT_LICENSE_PROFESSIONAL ) || defined( QT_INTERNAL_XML )
#define QM_EXPORT_DOM
#else
#define QM_EXPORT_DOM Q_EXPORT
#endif

#ifndef QT_NO_DOM

class QIODevice;
class QTextStream;

class QXmlInputSource;
class QXmlReader;

class QDomDocumentPrivate;
class QDomDocumentTypePrivate;
class QDomDocumentFragmentPrivate;
class QDomNodePrivate;
class QDomNodeListPrivate;
class QDomImplementationPrivate;
class QDomElementPrivate;
class QDomNotationPrivate;
class QDomEntityPrivate;
class QDomEntityReferencePrivate;
class QDomProcessingInstructionPrivate;
class QDomAttrPrivate;
class QDomCharacterDataPrivate;
class QDomTextPrivate;
class QDomCommentPrivate;
class QDomCDATASectionPrivate;
class QDomNamedNodeMapPrivate;
class QDomImplementationPrivate;

class QDomNodeList;
class QDomElement;
class QDomText;
class QDomComment;
class QDomCDATASection;
class QDomProcessingInstruction;
class QDomAttr;
class QDomEntityReference;
class QDomDocument;
class QDomNamedNodeMap;
class QDomDocument;
class QDomDocumentFragment;
class QDomDocumentType;
class QDomImplementation;
class QDomNode;
class QDomEntity;
class QDomNotation;
class QDomCharacterData;

class QM_EXPORT_DOM QDomImplementation
{
public:
    QDomImplementation();
    QDomImplementation( const QDomImplementation& );
    virtual ~QDomImplementation();
    QDomImplementation& operator= ( const QDomImplementation& );
    bool operator== ( const QDomImplementation& ) const;
    bool operator!= ( const QDomImplementation& ) const;

    // functions
    virtual bool hasFeature( const QString& feature, const QString& version );
    virtual QDomDocumentType createDocumentType( const QString& qName, const QString& publicId, const QString& systemId );
    virtual QDomDocument createDocument( const QString& nsURI, const QString& qName, const QDomDocumentType& doctype );

    // Qt extension
    bool isNull();

private:
    QDomImplementationPrivate* impl;
    QDomImplementation( QDomImplementationPrivate* );

    friend class QDomDocument;
};

class QM_EXPORT_DOM QDomNode
{
public:
    enum NodeType {
	ElementNode               = 1,
	AttributeNode             = 2,
	TextNode                  = 3,
	CDATASectionNode          = 4,
	EntityReferenceNode       = 5,
	EntityNode                = 6,
	ProcessingInstructionNode = 7,
	CommentNode               = 8,
	DocumentNode              = 9,
	DocumentTypeNode          = 10,
	DocumentFragmentNode      = 11,
	NotationNode              = 12,
	BaseNode                  = 21,// this is not in the standard
	CharacterDataNode         = 22 // this is not in the standard
    };

    QDomNode();
    QDomNode( const QDomNode& );
    QDomNode& operator= ( const QDomNode& );
    bool operator== ( const QDomNode& ) const;
    bool operator!= ( const QDomNode& ) const;
    virtual ~QDomNode();

    // DOM functions
    virtual QDomNode insertBefore( const QDomNode& newChild, const QDomNode& refChild );
    virtual QDomNode insertAfter( const QDomNode& newChild, const QDomNode& refChild );
    virtual QDomNode replaceChild( const QDomNode& newChild, const QDomNode& oldChild );
    virtual QDomNode removeChild( const QDomNode& oldChild );
    virtual QDomNode appendChild( const QDomNode& newChild );
    virtual bool hasChildNodes() const;
    virtual QDomNode cloneNode( bool deep = TRUE ) const;
    virtual void normalize();
    virtual bool isSupported( const QString& feature, const QString& version ) const;

    // DOM read only attributes
    virtual QString nodeName() const;
    virtual QDomNode::NodeType nodeType() const;
    virtual QDomNode         parentNode() const;
    virtual QDomNodeList     childNodes() const;
    virtual QDomNode         firstChild() const;
    virtual QDomNode         lastChild() const;
    virtual QDomNode         previousSibling() const;
    virtual QDomNode         nextSibling() const;
    virtual QDomNamedNodeMap attributes() const;
    virtual QDomDocument     ownerDocument() const;
    virtual QString namespaceURI() const;
    virtual QString localName() const;
    virtual bool hasAttributes() const;

    // DOM attributes
    virtual QString nodeValue() const;
    virtual void setNodeValue( const QString& );
    virtual QString prefix() const;
    virtual void setPrefix( const QString& pre );

    // Qt extensions
    virtual bool isAttr() const;
    virtual bool isCDATASection() const;
    virtual bool isDocumentFragment() const;
    virtual bool isDocument() const;
    virtual bool isDocumentType() const;
    virtual bool isElement() const;
    virtual bool isEntityReference() const;
    virtual bool isText() const;
    virtual bool isEntity() const;
    virtual bool isNotation() const;
    virtual bool isProcessingInstruction() const;
    virtual bool isCharacterData() const;
    virtual bool isComment() const;

    /**
     * Shortcut to avoid dealing with QDomNodeList
     * all the time.
     */
    QDomNode namedItem( const QString& name ) const;

    bool isNull() const;
    void clear();

    QDomAttr toAttr();
    QDomCDATASection toCDATASection();
    QDomDocumentFragment toDocumentFragment();
    QDomDocument toDocument();
    QDomDocumentType toDocumentType();
    QDomElement toElement();
    QDomEntityReference toEntityReference();
    QDomText toText();
    QDomEntity toEntity();
    QDomNotation toNotation();
    QDomProcessingInstruction toProcessingInstruction();
    QDomCharacterData toCharacterData();
    QDomComment toComment();

    void save( QTextStream&, int ) const;

protected:
    QDomNodePrivate* impl;
    QDomNode( QDomNodePrivate* );

private:
    friend class QDomDocument;
    friend class QDomDocumentType;
    friend class QDomNodeList;
    friend class QDomNamedNodeMap;
};

class QM_EXPORT_DOM QDomNodeList
{
public:
    QDomNodeList();
    QDomNodeList( const QDomNodeList& );
    QDomNodeList& operator= ( const QDomNodeList& );
    bool operator== ( const QDomNodeList& ) const;
    bool operator!= ( const QDomNodeList& ) const;
    virtual ~QDomNodeList();

    // DOM functions
    virtual QDomNode item( int index ) const;

    // DOM read only attributes
    virtual uint length() const;
    uint count() const { return length(); } // Qt API consitancy

private:
    QDomNodeListPrivate* impl;
    QDomNodeList( QDomNodeListPrivate* );

    friend class QDomNode;
    friend class QDomElement;
    friend class QDomDocument;
};

class QM_EXPORT_DOM QDomDocumentType : public QDomNode
{
public:
    QDomDocumentType();
    QDomDocumentType( const QDomDocumentType& x );
    QDomDocumentType& operator= ( const QDomDocumentType& );
    ~QDomDocumentType();

    // DOM read only attributes
    virtual QString name() const;
    virtual QDomNamedNodeMap entities() const;
    virtual QDomNamedNodeMap notations() const;
    virtual QString publicId() const;
    virtual QString systemId() const;
    virtual QString internalSubset() const;

    // Reimplemented from QDomNode
    QDomNode::NodeType nodeType() const;
    bool isDocumentType() const;

private:
    QDomDocumentType( QDomDocumentTypePrivate* );

    friend class QDomImplementation;
    friend class QDomDocument;
    friend class QDomNode;
};

class QM_EXPORT_DOM QDomDocument : public QDomNode
{
public:
    QDomDocument();
    Q_EXPLICIT QDomDocument( const QString& name );
    Q_EXPLICIT QDomDocument( const QDomDocumentType& doctype );
    QDomDocument( const QDomDocument& x );
    QDomDocument& operator= ( const QDomDocument& );
    ~QDomDocument();

    // DOM functions
    QDomElement createElement( const QString& tagName );
    QDomDocumentFragment createDocumentFragment();
    QDomText createTextNode( const QString& data );
    QDomComment createComment( const QString& data );
    QDomCDATASection createCDATASection( const QString& data );
    QDomProcessingInstruction createProcessingInstruction( const QString& target, const QString& data );
    QDomAttr createAttribute( const QString& name );
    QDomEntityReference createEntityReference( const QString& name );
    QDomNodeList elementsByTagName( const QString& tagname ) const;
    QDomNode importNode( const QDomNode& importedNode, bool deep );
    QDomElement createElementNS( const QString& nsURI, const QString& qName );
    QDomAttr createAttributeNS( const QString& nsURI, const QString& qName );
    QDomNodeList elementsByTagNameNS( const QString& nsURI, const QString& localName );
    QDomElement elementById( const QString& elementId );

    // DOM read only attributes
    QDomDocumentType doctype() const;
    QDomImplementation implementation() const;
    QDomElement documentElement() const;

    // Qt extensions
    bool setContent( const QCString& text, bool namespaceProcessing, QString *errorMsg=0, int *errorLine=0, int *errorColumn=0  );
    bool setContent( const QByteArray& text, bool namespaceProcessing, QString *errorMsg=0, int *errorLine=0, int *errorColumn=0  );
    bool setContent( const QString& text, bool namespaceProcessing, QString *errorMsg=0, int *errorLine=0, int *errorColumn=0  );
    bool setContent( QIODevice* dev, bool namespaceProcessing, QString *errorMsg=0, int *errorLine=0, int *errorColumn=0  );
    bool setContent( const QCString& text, QString *errorMsg=0, int *errorLine=0, int *errorColumn=0 );
    bool setContent( const QByteArray& text, QString *errorMsg=0, int *errorLine=0, int *errorColumn=0  );
    bool setContent( const QString& text, QString *errorMsg=0, int *errorLine=0, int *errorColumn=0  );
    bool setContent( QIODevice* dev, QString *errorMsg=0, int *errorLine=0, int *errorColumn=0  );

    bool setContent( QXmlInputSource *source, QXmlReader *reader, QString *errorMsg=0, int *errorLine=0, int *errorColumn=0  );

    // Reimplemented from QDomNode
    QDomNode::NodeType nodeType() const;
    bool isDocument() const;

    // Qt extensions
    QString toString() const; // ### Qt 4: merge the two overloads
    QString toString( int ) const;
    QCString toCString() const; // ### Qt 4: merge the two overloads
    QCString toCString( int ) const;

private:
    QDomDocument( QDomDocumentPrivate* );

    friend class QDomNode;
};

class QM_EXPORT_DOM QDomNamedNodeMap
{
public:
    QDomNamedNodeMap();
    QDomNamedNodeMap( const QDomNamedNodeMap& );
    QDomNamedNodeMap& operator= ( const QDomNamedNodeMap& );
    bool operator== ( const QDomNamedNodeMap& ) const;
    bool operator!= ( const QDomNamedNodeMap& ) const;
    ~QDomNamedNodeMap();

    // DOM functions
    QDomNode namedItem( const QString& name ) const;
    QDomNode setNamedItem( const QDomNode& newNode );
    QDomNode removeNamedItem( const QString& name );
    QDomNode item( int index ) const;
    QDomNode namedItemNS( const QString& nsURI, const QString& localName ) const;
    QDomNode setNamedItemNS( const QDomNode& newNode );
    QDomNode removeNamedItemNS( const QString& nsURI, const QString& localName );

    // DOM read only attributes
    uint length() const;
    uint count() const { return length(); } // Qt API consitancy

    // Qt extension
    bool contains( const QString& name ) const;

private:
    QDomNamedNodeMapPrivate* impl;
    QDomNamedNodeMap( QDomNamedNodeMapPrivate* );

    friend class QDomNode;
    friend class QDomDocumentType;
    friend class QDomElement;
};

class QM_EXPORT_DOM QDomDocumentFragment : public QDomNode
{
public:
    QDomDocumentFragment();
    QDomDocumentFragment( const QDomDocumentFragment& x );
    QDomDocumentFragment& operator= ( const QDomDocumentFragment& );
    ~QDomDocumentFragment();

    // Reimplemented from QDomNode
    QDomNode::NodeType nodeType() const;
    bool isDocumentFragment() const;

private:
    QDomDocumentFragment( QDomDocumentFragmentPrivate* );

    friend class QDomDocument;
    friend class QDomNode;
};

class QM_EXPORT_DOM QDomCharacterData : public QDomNode
{
public:
    QDomCharacterData();
    QDomCharacterData( const QDomCharacterData& x );
    QDomCharacterData& operator= ( const QDomCharacterData& );
    ~QDomCharacterData();

    // DOM functions
    virtual QString substringData( unsigned long offset, unsigned long count );
    virtual void appendData( const QString& arg );
    virtual void insertData( unsigned long offset, const QString& arg );
    virtual void deleteData( unsigned long offset, unsigned long count );
    virtual void replaceData( unsigned long offset, unsigned long count, const QString& arg );

    // DOM read only attributes
    virtual uint length() const;

    // DOM attributes
    virtual QString data() const;
    virtual void setData( const QString& );

    // Reimplemented from QDomNode
    QDomNode::NodeType nodeType() const;
    bool isCharacterData() const;

private:
    QDomCharacterData( QDomCharacterDataPrivate* );

    friend class QDomDocument;
    friend class QDomText;
    friend class QDomComment;
    friend class QDomNode;
};

class QM_EXPORT_DOM QDomAttr : public QDomNode
{
public:
    QDomAttr();
    QDomAttr( const QDomAttr& x );
    QDomAttr& operator= ( const QDomAttr& );
    ~QDomAttr();

    // DOM read only attributes
    virtual QString name() const;
    virtual bool specified() const;
    virtual QDomElement ownerElement() const;

    // DOM attributes
    virtual QString value() const;
    virtual void setValue( const QString& );

    // Reimplemented from QDomNode
    QDomNode::NodeType nodeType() const;
    bool isAttr() const;

private:
    QDomAttr( QDomAttrPrivate* );

    friend class QDomDocument;
    friend class QDomElement;
    friend class QDomNode;
};

class QM_EXPORT_DOM QDomElement : public QDomNode
{
public:
    QDomElement();
    QDomElement( const QDomElement& x );
    QDomElement& operator= ( const QDomElement& );
    ~QDomElement();

    // DOM functions
    QString attribute( const QString& name, const QString& defValue = QString::null ) const;
    void setAttribute( const QString& name, const QString& value );
    void setAttribute( const QString& name, int value );
    void setAttribute( const QString& name, uint value );
    void setAttribute( const QString& name, long value );
    void setAttribute( const QString& name, ulong value );
    void setAttribute( const QString& name, double value );
    void removeAttribute( const QString& name );
    QDomAttr attributeNode( const QString& name);
    QDomAttr setAttributeNode( const QDomAttr& newAttr );
    QDomAttr removeAttributeNode( const QDomAttr& oldAttr );
    virtual QDomNodeList elementsByTagName( const QString& tagname ) const;
    bool hasAttribute( const QString& name ) const;

    QString attributeNS( const QString nsURI, const QString& localName, const QString& defValue ) const;
    void setAttributeNS( const QString nsURI, const QString& qName, const QString& value );
    void setAttributeNS( const QString nsURI, const QString& qName, int value );
    void setAttributeNS( const QString nsURI, const QString& qName, uint value );
    void setAttributeNS( const QString nsURI, const QString& qName, long value );
    void setAttributeNS( const QString nsURI, const QString& qName, ulong value );
    void setAttributeNS( const QString nsURI, const QString& qName, double value );
    void removeAttributeNS( const QString& nsURI, const QString& localName );
    QDomAttr attributeNodeNS( const QString& nsURI, const QString& localName );
    QDomAttr setAttributeNodeNS( const QDomAttr& newAttr );
    virtual QDomNodeList elementsByTagNameNS( const QString& nsURI, const QString& localName ) const;
    bool hasAttributeNS( const QString& nsURI, const QString& localName ) const;

    // DOM read only attributes
    QString tagName() const;
    void setTagName( const QString& name ); // Qt extension

    // Reimplemented from QDomNode
    QDomNamedNodeMap attributes() const;
    QDomNode::NodeType nodeType() const;
    bool isElement() const;

    QString text() const;

private:
    QDomElement( QDomElementPrivate* );

    friend class QDomDocument;
    friend class QDomNode;
    friend class QDomAttr;
};

class QM_EXPORT_DOM QDomText : public QDomCharacterData
{
public:
    QDomText();
    QDomText( const QDomText& x );
    QDomText& operator= ( const QDomText& );
    ~QDomText();

    // DOM functions
    QDomText splitText( int offset );

    // Reimplemented from QDomNode
    QDomNode::NodeType nodeType() const;
    bool isText() const;

private:
    QDomText( QDomTextPrivate* );

    friend class QDomCDATASection;
    friend class QDomDocument;
    friend class QDomNode;
};

class QM_EXPORT_DOM QDomComment : public QDomCharacterData
{
public:
    QDomComment();
    QDomComment( const QDomComment& x );
    QDomComment& operator= ( const QDomComment& );
    ~QDomComment();

    // Reimplemented from QDomNode
    QDomNode::NodeType nodeType() const;
    bool isComment() const;

private:
    QDomComment( QDomCommentPrivate* );

    friend class QDomDocument;
    friend class QDomNode;
};

class QM_EXPORT_DOM QDomCDATASection : public QDomText
{
public:
    QDomCDATASection();
    QDomCDATASection( const QDomCDATASection& x );
    QDomCDATASection& operator= ( const QDomCDATASection& );
    ~QDomCDATASection();

    // Reimplemented from QDomNode
    QDomNode::NodeType nodeType() const;
    bool isCDATASection() const;

private:
    QDomCDATASection( QDomCDATASectionPrivate* );

    friend class QDomDocument;
    friend class QDomNode;
};

class QM_EXPORT_DOM QDomNotation : public QDomNode
{
public:
    QDomNotation();
    QDomNotation( const QDomNotation& x );
    QDomNotation& operator= ( const QDomNotation& );
    ~QDomNotation();

    // DOM read only attributes
    QString publicId() const;
    QString systemId() const;

    // Reimplemented from QDomNode
    QDomNode::NodeType nodeType() const;
    bool isNotation() const;

private:
    QDomNotation( QDomNotationPrivate* );

    friend class QDomDocument;
    friend class QDomNode;
};

class QM_EXPORT_DOM QDomEntity : public QDomNode
{
public:
    QDomEntity();
    QDomEntity( const QDomEntity& x );
    QDomEntity& operator= ( const QDomEntity& );
    ~QDomEntity();

    // DOM read only attributes
    virtual QString publicId() const;
    virtual QString systemId() const;
    virtual QString notationName() const;

    // Reimplemented from QDomNode
    QDomNode::NodeType nodeType() const;
    bool isEntity() const;

private:
    QDomEntity( QDomEntityPrivate* );

    friend class QDomNode;
};

class QM_EXPORT_DOM QDomEntityReference : public QDomNode
{
public:
    QDomEntityReference();
    QDomEntityReference( const QDomEntityReference& x );
    QDomEntityReference& operator= ( const QDomEntityReference& );
    ~QDomEntityReference();

    // Reimplemented from QDomNode
    QDomNode::NodeType nodeType() const;
    bool isEntityReference() const;

private:
    QDomEntityReference( QDomEntityReferencePrivate* );

    friend class QDomDocument;
    friend class QDomNode;
};

class QM_EXPORT_DOM QDomProcessingInstruction : public QDomNode
{
public:
    QDomProcessingInstruction();
    QDomProcessingInstruction( const QDomProcessingInstruction& x );
    QDomProcessingInstruction& operator= ( const QDomProcessingInstruction& );
    ~QDomProcessingInstruction();

    // DOM read only attributes
    virtual QString target() const;

    // DOM attributes
    virtual QString data() const;
    virtual void setData( const QString& d );

    // Reimplemented from QDomNode
    QDomNode::NodeType nodeType() const;
    bool isProcessingInstruction() const;

private:
    QDomProcessingInstruction( QDomProcessingInstructionPrivate* );

    friend class QDomDocument;
    friend class QDomNode;
};


QM_EXPORT_DOM QTextStream& operator<<( QTextStream&, const QDomNode& );

#endif //QT_NO_DOM
#endif // QDOM_H
