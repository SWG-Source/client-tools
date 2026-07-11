/**********************************************************************
**
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
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

#ifndef DLG2UI_H
#define DLG2UI_H

#include <qdom.h>
#include <qmap.h>
#include <qstring.h>
#include <qvariant.h>

typedef QMap<QString, QString> AttributeMap;

struct DlgConnection
{
    QString sender;
    QString signal;
    QString slot;

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
    bool operator==( const DlgConnection& ) const {
        return sender == sender && signal == signal && slot == slot;
    }
#endif
};

class Dlg2Ui
{
public:
    QStringList convertQtArchitectDlgFile( const QString& fileName );

private:
    QString alias( const QString& name ) const;
    QString opening( const QString& tag,
		     const AttributeMap& attr = AttributeMap() );
    QString closing( const QString& tag );
    void error( const QString& message );
    void syntaxError();
    QString getTextValue( const QDomNode& node );
    QVariant getValue( const QDomNodeList& children, const QString& tagName,
		       const QString& type = "qstring" );
    void emitHeader();
    void emitFooter();
    void emitSimpleValue( const QString& tag, const QString& value,
			  const AttributeMap& attr = AttributeMap() );
    void emitOpening( const QString& tag,
		      const AttributeMap& attr = AttributeMap() );
    void emitClosing( const QString& tag );
    void emitOpeningWidget( const QString& className );
    QString widgetClassName( const QDomElement& e );
    void emitColor( const QColor& color );
    void emitColorGroup( const QString& name, const QColorGroup& group );
    void emitVariant( const QVariant& val,
		      const QString& stringType = "string" );
    void emitProperty( const QString& prop, const QVariant& val,
		       const QString& stringType = "string" );
    void emitAttribute( const QString& attr, const QVariant& val,
			   const QString& stringType = "string" );
    void emitOpeningLayout( bool needsWidget, const QString& layoutKind,
			    const QString& name, int border, int autoBorder );
    void flushWidgets();
    void emitClosingLayout( bool needsWidget, const QString& layoutKind );
    bool isWidgetType( const QDomElement& e );
    void emitSpacer( int spacing, int stretch );
    QString filteredFlags( const QString& flags, const QRegExp& filter );
    void emitFrameStyleProperty( int style );
    void emitWidgetBody( const QDomElement& e, bool layouted );
    bool checkTagName( const QDomElement& e, const QString& tagName );
    QString normalizeType( const QString& type );
    QVariant getValue( const QDomElement& e, const QString& tagName,
		       const QString& type = "qstring" );
    void matchDialogCommon( const QDomElement& dialogCommon );
    bool needsQLayoutWidget( const QDomElement& e );
    void matchBoxLayout( const QDomElement& boxLayout );
    void matchBoxSpacing( const QDomElement& boxSpacing );
    void matchBoxStretch( const QDomElement& boxStretch );
    void matchGridLayout( const QDomElement& gridLayout );
    void matchGridRow( const QDomElement& gridRow );
    void matchGridSpacer( const QDomElement& gridSpacer );
    void matchLayoutWidget( const QDomElement& layoutWidget );
    void matchBox( const QDomElement& box );
    void matchLayout( const QDomElement& layout );
    void matchWidgetLayoutCommon( const QDomElement& widgetLayoutCommon );
    void matchWidget( const QDomElement& widget );
    void matchWidgets( const QDomElement& widgets );
    void matchTabOrder( const QDomElement& tabOrder );
    void matchWidgetLayout( const QDomElement& widgetLayout );
    void matchDialog( const QDomElement& dialog );

    QString yyOut;
    QString yyIndentStr;
    QString yyFileName;
    QString yyClassName;
    QMap<QString, int> yyWidgetTypeSet;
    QMap<QString, QMap<QString, int> > yyPropertyMap;
    QMap<QString, QDomElement> yyWidgetMap;
    QMap<QString, QString> yyCustomWidgets;
    QValueList<DlgConnection> yyConnections;
    QMap<QString, QString> yySlots;
    QMap<QString, QString> yyAliasMap;
    QStringList yyTabStops;
    QString yyBoxKind;
    int yyLayoutDepth;
    int yyGridRow;
    int yyGridColumn;

    int numErrors;
    int uniqueLayout;
    int uniqueSpacer;
    int uniqueWidget;
};

#endif
