/**********************************************************************
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Designer.
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
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef UIC_H
#define UIC_H
#include <qdom.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qtextstream.h>
#include <qpalette.h>
#include <qvariant.h>

class Uic : public Qt
{
public:
    Uic( const QString &fn, const char *outputFn, QTextStream& out,
	 QDomDocument doc, bool decl, bool subcl, const QString &trm,
	 const QString& subclname, bool omitForwardDecls );

    void createFormDecl( const QDomElement &e );
    void createFormImpl( const QDomElement &e );

    void createSubDecl( const QDomElement &e, const QString& subclname );
    void createSubImpl( const QDomElement &e, const QString& subclname );

    void createObjectDecl( const QDomElement &e );
    void createSpacerDecl( const QDomElement &e );
    void createActionDecl( const QDomElement &e );
    void createToolbarDecl( const QDomElement &e );
    void createMenuBarDecl( const QDomElement &e );
    void createPopupMenuDecl( const QDomElement &e );  
    void createActionImpl( const QDomElement &e, const QString &parent );
    void createToolbarImpl( const QDomElement &e, const QString &parentClass, const QString &parent );
    void createMenuBarImpl( const QDomElement &e, const QString &parentClass, const QString &parent );
    void createPopupMenuImpl( const QDomElement &e, const QString &parentClass, const QString &parent );
    QString createObjectImpl( const QDomElement &e, const QString& parentClass, const QString& parent, const QString& layout = QString::null );
    QString createLayoutImpl( const QDomElement &e, const QString& parentClass, const QString& parent, const QString& layout = QString::null );
    QString createObjectInstance( const QString& objClass, const QString& parent, const QString& objName );
    QString createSpacerImpl( const QDomElement &e, const QString& parentClass, const QString& parent, const QString& layout = QString::null );
    void createExclusiveProperty( const QDomElement & e, const QString& exclusiveProp );
    QString createListBoxItemImpl( const QDomElement &e, const QString &parent, QString *value = 0 );
    QString createIconViewItemImpl( const QDomElement &e, const QString &parent );
    QString createListViewColumnImpl( const QDomElement &e, const QString &parent, QString *value = 0 );
    QString createTableRowColumnImpl( const QDomElement &e, const QString &parent, QString *value = 0 );
    QString createListViewItemImpl( const QDomElement &e, const QString &parent,
				    const QString &parentItem );
    void createColorGroupImpl( const QString& cg, const QDomElement& e );
    QColorGroup loadColorGroup( const QDomElement &e );

    QDomElement getObjectProperty( const QDomElement& e, const QString& name );
    QString getPixmapLoaderFunction( const QDomElement& e );
    QString getFormClassName( const QDomElement& e );
    QString getClassName( const QDomElement& e );
    QString getObjectName( const QDomElement& e );
    QString getLayoutName( const QDomElement& e );
    QString getInclude( const QString& className );

    QString setObjectProperty( const QString& objClass, const QString& obj, const QString &prop, const QDomElement &e, bool stdset );

    QString registerObject( const QString& name );
    QString registeredName( const QString& name );
    bool isObjectRegistered( const QString& name );
    QStringList unique( const QStringList& );

    QString trcall( const QString& sourceText, const QString& comment = "" );

    static void embed( QTextStream& out, const char* project, const QStringList& images );

private:
    void registerLayouts ( const QDomElement& e );

    QTextStream& out;
    QTextOStream trout;
    QString languageChangeBody;
    QCString outputFileName;
    QStringList objectNames;
    QMap<QString,QString> objectMapper;
    QString indent;
    QStringList tags;
    QStringList layouts;
    QString formName;
    QString lastItem;
    QString trmacro;
    bool nofwd;

    struct Buddy
    {
	Buddy( const QString& k, const QString& b )
	    : key( k ), buddy( b ) {}
	Buddy(){} // for valuelist
	QString key;
	QString buddy;
	bool operator==( const Buddy& other ) const
	    { return (key == other.key); }
    };
    struct CustomInclude
    {
	QString header;
	QString location;
	Q_DUMMY_COMPARISON_OPERATOR(CustomInclude)
    };
    QValueList<Buddy> buddies;

    QStringList layoutObjects;
    bool isLayout( const QString& name ) const;

    uint item_used : 1;
    uint cg_used : 1;
    uint pal_used : 1;
    uint stdsetdef : 1;
    uint externPixmaps : 1;

    QString uiFileVersion;
    QString nameOfClass;
    QStringList namespaces;
    QString bareNameOfClass;
    QString pixmapLoaderFunction;

    void registerDatabases( const QDomElement& e );
    bool isWidgetInTable( const QDomElement& e, const QString& connection, const QString& table );
    bool isFrameworkCodeGenerated( const QDomElement& e );
    QString getDatabaseInfo( const QDomElement& e, const QString& tag );
    void createFormImpl( const QDomElement& e, const QString& form, const QString& connection, const QString& table );
    void writeFunctionsDecl( const QStringList &fuLst, const QStringList &typLst, const QStringList &specLst );
    void writeFunctionsSubDecl( const QStringList &fuLst, const QStringList &typLst, const QStringList &specLst );
    void writeFunctionsSubImpl( const QStringList &fuLst, const QStringList &typLst, const QStringList &specLst,
				const QString &subClass, const QString &descr );
    QStringList dbConnections;
    QMap< QString, QStringList > dbCursors;
    QMap< QString, QStringList > dbForms;

    static bool isMainWindow;
    static QString mkBool( bool b );
    static QString mkBool( const QString& s );
    bool toBool( const QString& s );
    static QString fixString( const QString &str, bool encode = FALSE );
    static bool onlyAscii;
    static QString mkStdSet( const QString& prop );
    static QString getComment( const QDomNode& n );
    QVariant defSpacing, defMargin;
    QString fileName;
    bool writeFunctImpl;
};

#endif
