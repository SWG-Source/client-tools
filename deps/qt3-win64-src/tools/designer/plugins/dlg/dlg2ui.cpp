/**********************************************************************
**
** Converts a Qt Architect 2.1+ .dlg file into a .ui file.
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

#include "dlg2ui.h"
#include <qfile.h>
#include <qframe.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qtextstream.h>

/*
  Possible improvements:

  1.  Convert layout stretch factors to size policy stretches, now
      that Qt Designer supports the latter.
*/

/*
  These big tables could be more or less eliminated by using Qt's
  QMetaObject and QMetaProperty classes. However, the interface of
  these classes is unwieldy for an otherwise non-GUI program like this
  one, as we would have to create one dummy object for most QObject
  subclasses in Qt. Let's take the safe road.
*/

static const char *widgetTypes[] = {
    "Button", "ButtonGroup", "CheckBox", "ComboBox", "Dial", "DlgWidget",
    "Frame", "Grid", "GroupBox", "HBox", "HButtonGroup", "HGroupBox",
    "IconView", "LCDNumber", "Label", "LineEdit", "ListBox", "ListView",
    "MenuBar", "MultiLineEdit", "ProgressBar", "PushButton", "RadioButton",
    "ScrollBar", "ScrollView", "Slider", "SpinBox", "Splitter", "TabBar",
    "TextBrowser", "TextView", "User", "VBox", "VButtonGroup", "VGroupBox", 0
};

/*
  This table maps Qt Architect properties to Qt Designer properties.
  If there is no corresponding Qt Designer property, qtName is 0 and
  the property can be handled explicitly.
*/
static const struct {
    const char *widgetName;
    const char *architectName;
    const char *qtName;
    const char *type;
} propertyDefs[] = {
    { "Button", "AutoRepeat", "autoRepeat", "boolean" },
    { "Button", "AutoResize", 0, 0 },
    { "Button", "Text", "text", "qstring" },
    { "ButtonGroup", "Exclusive", "exclusive", "boolean" },
    { "ButtonGroup", "RadioButtonExclusive", "radioButtonExclusive",
      "boolean" },
    { "CheckBox", "Checked", "checked", "boolean" },
    { "ComboBox", "AutoCompletion", "autoCompletion", "boolean" },
    { "ComboBox", "AutoResize", 0, 0 },
    { "ComboBox", "DuplicatesEnabled", "duplicatesEnabled", "boolean" },
    { "ComboBox", "MaxCount", "maxCount", "integer" },
    { "ComboBox", "Policy", "insertionPolicy", "enum" },
    { "ComboBox", "SizeLimit", "sizeLimit", "integer" },
    { "ComboBox", "Style", 0, 0 },
    { "Dial", "Initial", "value", "integer" },
    { "Dial", "LineStep", "lineStep", "integer" },
    { "Dial", "MaxValue", "maxValue", "integer" },
    { "Dial", "MinValue", "minValue", "integer" },
    { "Dial", "NotchTarget", "notchTarget", "double" },
    { "Dial", "PageStep", "pageStep", "integer" },
    { "Dial", "ShowNotches", "notchesVisible", "boolean" },
    { "Dial", "Tracking", "tracking", "boolean" },
    { "Dial", "Wrapping", "wrapping", "boolean" },
    { "DlgWidget", "AdjustSize", 0, 0 },
    { "DlgWidget", "BackgroundMode", "backgroundMode", "enum" },
    { "DlgWidget", "BackgroundOrigin", "backgroundOrigin", "enum" },
    { "DlgWidget", "BackgroundPixmap", "backgroundPixmap", "qpixmap" },
    { "DlgWidget", "DataLenName", 0, 0 },
    { "DlgWidget", "DataVarName", 0, 0 },
    { "DlgWidget", "Enabled", "enabled", "boolean" },
    { "DlgWidget", "FocusPolicy", "focusPolicy", "enum" },
    { "DlgWidget", "Font", "font", "qfont" },
    { "DlgWidget", "FontPropagation", 0, 0 },
    { "DlgWidget", "MaximumSize", "maximumSize", "qsize" },
    { "DlgWidget", "MinimumSize", "minimumSize", "qsize" },
    { "DlgWidget", "Name", 0, 0 },
    { "DlgWidget", "Palette", "palette", "qpalette" },
    { "DlgWidget", "PalettePropagation", 0, 0 },
    { "DlgWidget", "ReadPixmapFromData", 0, 0 },
    { "DlgWidget", "Rect", 0, 0 },
    { "DlgWidget", "SignalConnection", 0, 0 },
    { "DlgWidget", "UseBackgroudPixmap", 0, 0 },
    { "DlgWidget", "Variable", 0, 0 },
    { "Frame", "FrameMargin", "margin", "integer" },
    { "Frame", "LineWidth", "lineWidth", "integer" },
    { "Frame", "MidLineWidth", "midLineWidth", "integer" },
    { "Frame", "Style", 0, 0 },
    { "GroupBox", "Title", "title", "qstring" },
    { "IconView", "AlignMode", 0, 0 },
    { "IconView", "Aligning", 0, 0 },
    { "IconView", "Arrangement", "arrangement", "enum" },
    { "IconView", "AutoArrange", "autoArrange", "boolean" },
    { "IconView", "EnableMoveItems", "itemsMovable", "boolean" },
    { "IconView", "GridX", "gridX", "integer" },
    { "IconView", "GridY", "gridY", "integer" },
    { "IconView", "ItemTextPos", "itemTextPos", "enum" },
    { "IconView", "ItemsMovable", "itemsMovable", "boolean" },
    { "IconView", "MaxItemTextLength", "maxItemTextLength", "integer" },
    { "IconView", "MaxItemWidth", "maxItemWidth", "integer" },
    { "IconView", "ResizeMode", "resizeMode", "enum" },
    { "IconView", "SelectionMode", "selectionMode", "enum" },
    { "IconView", "ShowToolTips", "showToolTips", "boolean" },
    { "IconView", "SortAscending", "sortDirection", "bool" },
    { "IconView", "Spacing", "spacing", "integer" },
    { "IconView", "WordWrapIconText", "wordWrapIconText", "boolean" },
    { "LCDNumber", "Digits", "numDigits", "integer" },
    { "LCDNumber", "Mode", "mode", "enum" },
    { "LCDNumber", "SegmentStyle", "segmentStyle", "enum" },
    { "LCDNumber", "SmallDecimalPoint", "smallDecimalPoint", "boolean" },
    { "LCDNumber", "Value", 0, 0 },
    { "Label", "AutoResize", 0, 0 },
    { "Label", "Indent", "indent", "integer" },
    { "Label", "Text", "text", "qstring" },
    { "Label", "TextFormat", "textFormat", "enum" },
    { "LineEdit", "EchoMode", "echoMode", "enum" },
    { "LineEdit", "FrameShown", "frame", "boolean"  },
    { "LineEdit", "MaxLength", "maxLength", "integer" },
    { "LineEdit", "Text", "text", "qstring" },
    { "ListBox", "AutoScroll", 0, 0 },
    { "ListBox", "AutoUpdate", 0, 0 },
    { "ListBox", "ColumnMode", "columnMode", "enum" },
    { "ListBox", "DragSelect", 0, 0 },
    { "ListBox", "RowMode", "rowMode", "enum" },
    { "ListBox", "SelectionMode", "selectionMode", "enum" },
    { "ListBox", "SmoothScrolling", 0, 0 },
    { "ListView", "AllColumnsShowFocus", "allColumnsShowFocus", "boolean" },
    { "ListView", "HeaderInformation", 0, 0 },
    { "ListView", "ItemMargin", "itemMargin", "integer" },
    { "ListView", "MultiSelection", "multiSelection", "boolean" },
    { "ListView", "RootIsDecorated", "rootIsDecorated", "boolean" },
    { "ListView", "TreeStepSize", "treeStepSize", "boolean" },
    { "MultiLineEdit", "AutoUpdate", 0, 0 },
    { "MultiLineEdit", "EchoMode", 0, 0 },
    { "MultiLineEdit", "HorizontalMargin", 0, 0 },
    { "MultiLineEdit", "MaxLength", 0, 0 },
    { "MultiLineEdit", "MaxLineLength", 0, 0 },
    { "MultiLineEdit", "MaxLines", 0, 0 },
    { "MultiLineEdit", "OverwriteMode", 0, 0 },
    { "MultiLineEdit", "ReadOnly", 0, 0 },
    { "MultiLineEdit", "Text", 0, 0 },
    { "MultiLineEdit", "UndoDepth", "undoDepth", "integer" },
    { "MultiLineEdit", "UndoEnabled", 0, 0 },
    { "MultiLineEdit", "WordWrap", 0, 0 },
    { "MultiLineEdit", "WrapColumnOrWidth", 0, 0 },
    { "MultiLineEdit", "WrapPolicy", 0, 0 },
    { "ProgressBar", "CenterIndicator", "centerIndicator", "boolean" },
    { "ProgressBar", "IndicatorFollowsStyle", "indicatorFollowsStyle",
      "boolean" },
    { "ProgressBar", "Progress", "progress", "integer" },
    { "ProgressBar", "TotalSteps", "totalSteps", "integer" },
    { "PushButton", "AutoDefault", "autoDefault", "boolean" },
    { "PushButton", "Default", "default", "boolean" },
    { "PushButton", "IsMenuButton", 0, 0 },
    { "PushButton", "ToggleButton", "toggleButton", "boolean" },
    { "RadioButton", "Checked", "checked", "boolean" },
    { "ScrollBar", "Initial", "value", "integer" },
    { "ScrollBar", "LineStep", "lineStep", "integer" },
    { "ScrollBar", "MaxValue", "maxValue", "integer" },
    { "ScrollBar", "MinValue", "minValue", "integer" },
    { "ScrollBar", "Orientation", "orientation", "enum" },
    { "ScrollBar", "PageStep", "pageStep", "integer" },
    { "ScrollBar", "Tracking", "tracking", "boolean" },
    { "ScrollView", "DragAutoScroll", "dragAutoScroll", "boolean" },
    { "ScrollView", "HScrollBarMode", "hScrollBarMode", "enum" },
    { "ScrollView", "ResizePolicy", "resizePolicy", "enum" },
    { "ScrollView", "VScrollBarMode", "vScrollBarMode", "enum" },
    { "Slider", "Initial", "value", "integer" },
    { "Slider", "LineStep", "lineStep", "integer" },
    { "Slider", "MaxValue", "maxValue", "integer" },
    { "Slider", "MinValue", "minValue", "integer" },
    { "Slider", "Orientation", "orientation", "enum" },
    { "Slider", "PageStep", "pageStep", "integer" },
    { "Slider", "TickInterval", "tickInterval", "integer" },
    { "Slider", "Tickmarks", "tickmarks", "enum" },
    { "Slider", "Tracking", "tracking", "boolean" },
    { "SpinBox", "ButtonSymbols", "buttonSymbols", "enum" },
    { "SpinBox", "MaxValue", "maxValue", "integer" },
    { "SpinBox", "MinValue", "minValue", "integer" },
    { "SpinBox", "Prefix", "prefix", "qstring" },
    { "SpinBox", "SpecialValue", "specialValueText", "qstring" },
    { "SpinBox", "Step", "lineStep", "integer" },
    { "SpinBox", "Suffix", "suffix", "qstring" },
    { "SpinBox", "Wrapping", "wrapping", "boolean" },
    { "Splitter", "OpaqueResize", 0, 0 },
    { "Splitter", "Orientation", "orientation", "enum" },
    { "TabBar", "Shape", "shape", "enum" },
    { "TabBar", "TabNames", 0, 0 },
    { "TextView", "Context", 0, 0 },
    { "TextView", "LinkUnderline", "linkUnderline", "boolean" },
    { "TextView", "Text", "text", "qstring" },
    { "TextView", "TextFormat", "textFormat", "enum" },
    { "User", "UserClassHeader", 0, 0 },
    { "User", "UserClassName", 0, 0 },
    { 0, 0, 0, 0 }
};

/*
  This is the number of supported color groups in a palette, and
  supported color roles in a color group. Changing these constants is
  dangerous.
*/
static const int NumColorRoles = 14;

static bool isTrue( const QString& val )
{
    return val.lower() == QString( "true" );
}

static AttributeMap attribute( const QString& name, const QString& val )
{
    AttributeMap attr;
    attr.insert( name, val );
    return attr;
}

static QString entitize( const QString& str )
{
    QString t = str;
    t.replace( '&', QString("&amp;") );
    t.replace( '>', QString("&gt;") );
    t.replace( '<', QString("&lt;") );
    t.replace( '"', QString("&quot;") );
    t.replace( '\'', QString("&apos;") );
    return t;
}

QString Dlg2Ui::alias( const QString& name ) const
{
    if ( yyAliasMap.contains(name) )
	return yyAliasMap[name];
    else
	return name;
}

QString Dlg2Ui::opening( const QString& tag, const AttributeMap& attr )
{
    QString t = QChar( '<' ) + tag;
    AttributeMap::ConstIterator a = attr.begin();
    while ( a != attr.end() ) {
	t += QChar( ' ' ) + a.key() + QString( "=\"" ) + entitize( *a ) +
	     QChar( '"' );
	++a;
    }
    t += QChar( '>' );
    return t;
}

QString Dlg2Ui::closing( const QString& tag )
{
    return opening( QChar('/') + tag );
}

void Dlg2Ui::error( const QString& message )
{
    if ( numErrors++ == 0 )
	QMessageBox::warning( 0, yyFileName, message );
}

void Dlg2Ui::syntaxError()
{
    error( QString("Sorry, I met a random syntax error. I did what I could, but"
		   " that was not enough."
		   "<p>You might want to write to"
		   " <tt>qt-bugs@trolltech.com</tt> about this incident.") );
}

QString Dlg2Ui::getTextValue( const QDomNode& node )
{
    if ( node.childNodes().count() > 1 ) {
	syntaxError();
	return QString::null;
    }

    if ( node.childNodes().count() == 0 )
	return QString::null;

    QDomText child = node.firstChild().toText();
    if ( child.isNull() ) {
	syntaxError();
	return QString::null;
    }
    QString t = child.data().stripWhiteSpace();
    t.replace( "\\\\", "\\" );
    t.replace( "\\n", "\n" );
    return t;
}

QVariant Dlg2Ui::getValue( const QDomNodeList& children, const QString& tagName,
			   const QString& type )
{
    for ( int i = 0; i < (int) children.count(); i++ ) {
	QDomNode n = children.item( i );
	if ( n.toElement().tagName() == tagName )
	    return getValue( n.toElement(), tagName, type );
    }
    return QVariant();
}

void Dlg2Ui::emitHeader()
{
    yyOut += QString( "<!DOCTYPE UI><UI version=\"3.0\" stdsetdef=\"1\">\n" );
}

void Dlg2Ui::emitFooter()
{
    yyOut += QString( "</UI>\n" );
}

void Dlg2Ui::emitSimpleValue( const QString& tag, const QString& value,
			      const AttributeMap& attr )
{
    yyOut += yyIndentStr + opening( tag, attr ) + entitize( value ) +
	     closing( tag ) + QChar( '\n' );
}

void Dlg2Ui::emitOpening( const QString& tag, const AttributeMap& attr )
{
    yyOut += yyIndentStr + opening( tag, attr ) + QChar( '\n' );
    yyIndentStr += QString( "    " );
}

void Dlg2Ui::emitClosing( const QString& tag )
{
    yyIndentStr.truncate( yyIndentStr.length() - 4 );
    yyOut += yyIndentStr + closing( tag ) + QChar( '\n' );
}

void Dlg2Ui::emitOpeningWidget( const QString& className )
{
    AttributeMap attr = attribute( QString("class"), className );
    if ( yyGridColumn >= 0 ) {
	attr.insert( QString("row"), QString::number(yyGridRow) );
	attr.insert( QString("column"), QString::number(yyGridColumn) );
	yyGridColumn = -1;
    }
    emitOpening( QString("widget"), attr );
}

QString Dlg2Ui::widgetClassName( const QDomElement& e )
{
    if ( e.tagName() == QString("User") ) {
	return getValue( e.childNodes(), QString("UserClassName") )
			 .toString();
    } else if ( e.tagName() == QString("DlgWidget") ) {
	return QString( "QWidget" );
    } else {
	return QChar( 'Q' ) + e.tagName();
    }
}

void Dlg2Ui::emitColor( const QColor& color )
{
    emitOpening( QString("color") );
    emitSimpleValue( QString("red"), QString::number(color.red()) );
    emitSimpleValue( QString("green"), QString::number(color.green()) );
    emitSimpleValue( QString("blue"), QString::number(color.blue()) );
    emitClosing( QString("color") );
}

void Dlg2Ui::emitColorGroup( const QString& name, const QColorGroup& group )
{
    emitOpening( name );
    for ( int i = 0; i < NumColorRoles; i++ )
	emitColor( group.color((QColorGroup::ColorRole) i) );
    emitClosing( name );
}

void Dlg2Ui::emitVariant( const QVariant& val, const QString& stringType )
{
    if ( val.isValid() ) {
	switch ( val.type() ) {
	case QVariant::String:
	    emitSimpleValue( stringType, val.toString() );
	    break;
	case QVariant::CString:
	    emitSimpleValue( QString("cstring"), val.toString() );
	    break;
	case QVariant::Bool:
	    emitSimpleValue( QString("bool"),
			     QString(val.toBool() ? "true" : "false") );
	    break;
	case QVariant::Int:
	case QVariant::UInt:
	    emitSimpleValue( QString("number"), val.toString() );
	    break;
	case QVariant::Rect:
	    emitOpening( QString("rect") );
	    emitSimpleValue( QString("x"), QString::number(val.toRect().x()) );
	    emitSimpleValue( QString("y"), QString::number(val.toRect().y()) );
	    emitSimpleValue( QString("width"),
			     QString::number(val.toRect().width()) );
	    emitSimpleValue( QString("height"),
			     QString::number(val.toRect().height()) );
	    emitClosing( QString("rect") );
	    break;
	case QVariant::Point:
	    emitOpening( QString("point") );
	    emitSimpleValue( QString("x"), QString::number(val.toPoint().x()) );
	    emitSimpleValue( QString("y"), QString::number(val.toPoint().y()) );
	    emitClosing( QString("point") );
	    break;
	case QVariant::Size:
	    emitOpening( QString("size") );
	    emitSimpleValue( QString("width"),
			     QString::number(val.toSize().width()) );
	    emitSimpleValue( QString("height"),
			     QString::number(val.toSize().height()) );
	    emitClosing( QString("size") );
	    break;
	case QVariant::Color:
	    emitColor( val.toColor() );
	    break;
	case QVariant::Font:
	    emitOpening( QString("font") );
	    emitSimpleValue( QString("family"), val.toFont().family() );
	    emitSimpleValue( QString("pointsize"),
			     QString::number(val.toFont().pointSize()) );
	    if ( val.toFont().bold() )
		emitSimpleValue( QString("bold"), QChar('1') );
	    if ( val.toFont().italic() )
		emitSimpleValue( QString("italic"), QChar('1') );
	    if ( val.toFont().underline() )
		emitSimpleValue( QString("underline"), QChar('1') );
	    if ( val.toFont().strikeOut() )
		emitSimpleValue( QString("strikeout"), QChar('1') );
	    emitClosing( QString("font") );
	    break;
	case QVariant::Palette:
	    emitOpening( QString("palette") );
	    emitColorGroup( QString("active"), val.toPalette().active() );
	    emitColorGroup( QString("disabled"), val.toPalette().disabled() );
	    emitColorGroup( QString("inactive"), val.toPalette().inactive() );
	    emitClosing( QString("palette") );
	    break;
	default:
	    emitSimpleValue( QString("fnord"), QString::null );
	}
    }
}

void Dlg2Ui::emitProperty( const QString& prop, const QVariant& val,
			   const QString& stringType )
{
    emitOpening( QString("property"), attribute(QString("name"), prop) );
    emitVariant( val, stringType );
    emitClosing( QString("property") );
}

void Dlg2Ui::emitAttribute( const QString& attr, const QVariant& val,
			    const QString& stringType )
{
    emitOpening( QString("attribute"), attribute(QString("name"), attr) );
    emitVariant( val, stringType );
    emitClosing( QString("attribute") );
}

void Dlg2Ui::emitOpeningLayout( bool needsWidget, const QString& layoutKind,
				const QString& name, int border,
				int autoBorder )
{
    QString namex = name;

    if ( namex.isEmpty() )
	namex = QString( "Layout%1" ).arg( uniqueLayout++ );

    if ( needsWidget ) {
	emitOpeningWidget( QString("QLayoutWidget") );
	emitProperty( QString("name"), namex.latin1() );
    }
    emitOpening( layoutKind );
    if ( !needsWidget )
	emitProperty( QString("name"), namex.latin1() );
    if ( border != 5 )
	emitProperty( QString("margin"), border );
    if ( autoBorder != 5 )
	emitProperty( QString("spacing"), autoBorder );
    yyLayoutDepth++;
}

void Dlg2Ui::flushWidgets()
{
    QRegExp widgetForLayout( QString("Q(?:[HV]Box|Grid)") );

    while ( !yyWidgetMap.isEmpty() ) {
	QString className = widgetClassName( *yyWidgetMap.begin() );
	if ( !widgetForLayout.exactMatch(className) ) {
	    emitOpeningWidget( className );
	    emitWidgetBody( *yyWidgetMap.begin(), FALSE );
	    emitClosing( QString("widget") );
	}
	yyWidgetMap.remove( yyWidgetMap.begin() );
    }
}

void Dlg2Ui::emitClosingLayout( bool needsWidget, const QString& layoutKind )
{
    yyLayoutDepth--;
    /*
      Qt Designer can deal with layouted widgets and with
      fixed-position widgets, but not both at the same time. If such a
      thing happens, we arbitrarily put the fixed-position widgets in
      the layout so that they at least show up in Qt Designer.
    */
    if ( yyLayoutDepth == 0 )
	flushWidgets();

    emitClosing( layoutKind );
    if ( needsWidget )
	emitClosing( QString("widget") );
}

bool Dlg2Ui::isWidgetType( const QDomElement& e )
{
    return yyWidgetTypeSet.contains( e.tagName() );
}

void Dlg2Ui::emitSpacer( int spacing, int stretch )
{
    QString orientationStr;
    QSize sizeHint;
    QString sizeType = QString( "Fixed" );

    if ( yyBoxKind == QString("hbox") ) {
	orientationStr = QString( "Horizontal" );
	sizeHint = QSize( spacing, 20 );
    } else {
	orientationStr = QString( "Vertical" );
	sizeHint = QSize( 20, spacing );
    }
    if ( stretch > 0 )
	sizeType = QString( "Expanding" );

    emitOpening( QString("spacer") );
    emitProperty( QString("name"),
		  QString("Spacer%1").arg(uniqueSpacer++).latin1() );
    emitProperty( QString("orientation"), orientationStr, QString("enum") );
    if ( spacing > 0 )
	emitProperty( QString("sizeHint"), sizeHint, QString("qsize") );
    emitProperty( QString("sizeType"), sizeType, QString("enum") );
    emitClosing( QString("spacer") );
}

QString Dlg2Ui::filteredFlags( const QString& flags, const QRegExp& filter )
{
    QRegExp evil( QString("[^0-9A-Z_a-z|]") );

    QString f = flags;
    f.replace( evil, QString::null );
    QStringList splitted = QStringList::split( QChar('|'), f );
    return splitted.grep( filter ).join( QChar('|') );
}

void Dlg2Ui::emitFrameStyleProperty( int style )
{
    QString shape;
    QString shadow;

    switch ( style & QFrame::MShape ) {
    case QFrame::Box:
	shape = QString( "Box" );
	break;
    case QFrame::Panel:
	shape = QString( "Panel" );
	break;
    case QFrame::WinPanel:
	shape = QString( "WinPanel" );
	break;
    case QFrame::HLine:
	shape = QString( "HLine" );
	break;
    case QFrame::VLine:
	shape = QString( "VLine" );
	break;
    case QFrame::StyledPanel:
	shape = QString( "StyledPanel" );
	break;
    case QFrame::PopupPanel:
	shape = QString( "PopupPanel" );
	break;
    case QFrame::MenuBarPanel:
	shape = QString( "MenuBarPanel" );
	break;
    case QFrame::ToolBarPanel:
	shape = QString( "ToolBarPanel" );
	break;
    case QFrame::LineEditPanel:
	shape = QString( "LineEditPanel" );
	break;
    case QFrame::TabWidgetPanel:
	shape = QString( "TabWidgetPanel" );
	break;
    case QFrame::GroupBoxPanel:
	shape = QString( "GroupBoxPanel" );
	break;
    default:
	shape = QString( "NoFrame" );
    }

    switch ( style & QFrame::MShadow ) {
    case QFrame::Raised:
	shadow = QString( "Raised" );
	break;
    case QFrame::Sunken:
	shadow = QString( "Sunken" );
	break;
    default:
	shadow = QString( "Plain" );
    }

    emitProperty( QString("frameShape"), shape, QString("enum") );
    emitProperty( QString("frameShadow"), shadow, QString("enum") );
}

void Dlg2Ui::emitWidgetBody( const QDomElement& e, bool layouted )
{
    QRegExp align( QString("^(?:Align|WordBreak$)") );
    QRegExp frameShape( QString(
	    "^(?:NoFrame|Box|(?:Win|Styled|Popup|(?:Menu|Tool)Bar)?Panel|"
	    "[HV]Line)$") );
    QRegExp frameShadow( QString( "^(?:Plain|Raised|Sunken)$") );
    QRegExp numeric( QString("[0-9]+(?:\\.[0-9]*)?|\\.[0-9]+") );
    QRegExp connex( QString(
	    "\\s*\\[(BaseClass|P(?:ublic|rotected))\\]\\s*([0-9A-Z_a-z]+)\\s*"
	    "-->\\s*([0-9A-Z_a-z]+)\\s*(\\([^()]*\\))\\s*") );
    QRegExp qdialogSlots( QString(
	    "done\\(\\s*int\\s*\\)|(?:accept|reject)\\(\\s*\\)") );

    QString userClassHeader;
    QString userClassName;
    QString parentTagName;
    QString name;
    QString variableName;
    QMap<QString, int> pp;

    QDomNode n = e;
    while ( !n.isNull() ) {
	if ( isWidgetType(n.toElement()) ) {
	    parentTagName = n.toElement().tagName();
	    pp = yyPropertyMap[parentTagName];
	    n = n.firstChild();
	} else {
	    QString tagName = n.toElement().tagName();

	    QMap<QString, int>::ConstIterator p = pp.find( tagName );
	    if ( p == pp.end() ) {
		/*
		  These properties are not in the propertyDefs table,
		  since they are found in many classes anyway and need
		  to be treated the same in each case.
		*/
		if ( tagName == QString("Alignement") ||
		     tagName == QString("Alignment") ) {
		    QString flags = getValue( n.toElement(), tagName )
				    .toString();
		    flags = filteredFlags( flags, align );
		    if ( !flags.isEmpty() )
			emitProperty( QString("alignment"), flags,
				      QString("set") );
		} else if ( tagName == QString("ItemList") ) {
		    QDomNode child = n.firstChild();
		    while ( !child.isNull() ) {
			if ( child.toElement().tagName() == QString("Item") ) {
			    QString text = getTextValue( child );
			    emitOpening( QString("item") );
			    emitProperty( QString("text"), text );
			    emitClosing( QString("item") );
			}
			child = child.nextSibling();
		    }
		}
	    } else {
		QString propertyName( propertyDefs[*p].qtName );

		if ( propertyName.isEmpty() ) {
		    /*
		      These properties are in the propertyDefs table,
		      but they have no direct Qt equivalent.
		    */
		    if ( parentTagName == QString("ComboBox") ) {
			if ( tagName == QString("Style") ) {
			    if ( getTextValue(n) == QString("ReadWrite") )
				emitProperty( QString("editable"),
					      QVariant(TRUE, 0) );
			}
		    } else if ( parentTagName == QString("DlgWidget") ) {
			if ( tagName == QString("Name") ) {
			    name = getTextValue( n );
			} else if ( tagName == QString("Rect") ) {
			    QRect rect = getValue( n.toElement(), tagName,
						   QString("qrect") )
					 .toRect();
			    if ( !layouted )
				emitProperty( QString("geometry"), rect,
					      QString("qrect") );
			} else if ( tagName == QString("SignalConnection") ) {
			    QDomNode child = n.firstChild();
			    while ( !child.isNull() ) {
				if ( child.toElement().tagName() ==
				     QString("Signal") ) {
				    QString text = getTextValue( child );
				    if ( connex.exactMatch(text) ) {
					DlgConnection c;
					c.sender = getValue(
						n.parentNode().childNodes(),
						QString("Name") ).toString();
					c.signal = connex.cap( 2 ) +
						   connex.cap( 4 );
					c.slot = connex.cap( 3 ) +
						 connex.cap( 4 );
					yyConnections.append( c );

					if ( connex.cap(1) !=
					     QString("BaseClass") &&
					     !qdialogSlots.exactMatch(c.slot) )
					    yySlots.insert( c.slot,
							    connex.cap(1) );
				    }
				}
				child = child.nextSibling();
			    }
			} else if ( tagName == QString("Variable") ) {
			    variableName = getTextValue( n );
			}
		    } else if ( parentTagName == QString("Frame") ) {
			if ( tagName == QString("Style") ) {
			    int style = getValue( n.toElement(), tagName,
						  QString("integer") ).toInt();
			    emitFrameStyleProperty( style );
			}
		    } else if ( parentTagName == QString("LCDNumber") ) {
			if ( tagName == QString("Value") ) {
			    QString text = getValue( n.toElement(), tagName )
					   .toString();
			    if ( numeric.exactMatch(text) )
				emitProperty( QString("value"),
					      text.toDouble() );
			}
		    } else if ( parentTagName == QString("ListView") ) {
			if ( tagName == QString("HeaderInformation") ) {
			    int columnNo = 1;
			    QDomNode child = n.firstChild();
			    while ( !child.isNull() ) {
				if ( child.toElement().tagName() ==
				     QString("Header") ) {
				    QString text = getValue( child.childNodes(),
							     QString("Text") )
						   .toString();
				    if ( text.isEmpty() )
					text = QString( "Column %1" )
					       .arg( columnNo );
				    emitOpening( QString("column") );
				    emitProperty( QString("text"), text );
				    emitClosing( QString("column") );
				}
				child = child.nextSibling();
				columnNo++;
			    }
			}
		    } else if ( parentTagName == QString("TabBar") ) {
			if ( tagName == QString("TabNames") ) {
			    QDomNode child = n.firstChild();
			    while ( !child.isNull() ) {
				if ( child.toElement().tagName() ==
				     QString("Tab") ) {
				    QString text = getTextValue( child );
				    emitOpeningWidget( QString("QWidget") );
				    emitProperty( QString("name"), "tab" );
				    emitAttribute( QString("title"), text );
				    emitClosing( QString("widget") );
				}
				child = child.nextSibling();
			    }
			}
		    } else if ( parentTagName == QString("User") ) {
			if ( tagName == QString("UserClassHeader") ) {
			    userClassHeader = getTextValue( n );
			} else if ( tagName == QString("UserClassName") ) {
			    userClassName = getTextValue( n );
			}
		    }
		} else {
		    /*
		      These properties are in the propertyDefs table;
		      they have a direct Qt equivalent.
		    */
		    QString type( propertyDefs[*p].type );
		    QVariant val = getValue( n.toElement(), tagName, type );

		    if ( type == QString("qstring") )
			type = QString( "string" );

		    bool omit = FALSE;
		    if ( propertyName == QString("backgroundOrigin") &&
			 val.toString() == QString("WidgetOrigin") )
			omit = TRUE;
		    if ( propertyName == QString("enabled") && val.toBool() )
			omit = TRUE;
		    if ( propertyName == QString("minimumSize") &&
			 val.toSize() == QSize(-1, -1) )
			omit = TRUE;
		    if ( propertyName == QString("maximumSize") &&
			 val.toSize() == QSize(32767, 32767) )
			omit = TRUE;

		    if ( !omit )
			emitProperty( propertyName, val, type );
		}
	    }
	    n = n.nextSibling();
	}
    }

    if ( !variableName.isEmpty() ) {
	yyAliasMap.insert( name, variableName );
	name = variableName;
    }
    if ( !name.isEmpty() )
	emitProperty( QString("name"), name.latin1() );

    if ( !userClassName.isEmpty() )
	yyCustomWidgets.insert( userClassName, userClassHeader );
}

bool Dlg2Ui::checkTagName( const QDomElement& e, const QString& tagName )
{
    bool ok = ( e.tagName() == tagName );
    if ( !ok )
	syntaxError();
    return ok;
}

QString Dlg2Ui::normalizeType( const QString& type )
{
    QString t = type;
    if ( t.isEmpty() || t == QString("enum") || t == QString( "qcstring" ) ||
	 t == QString("set") )
	t = QString( "qstring" );
    return t;
}

QVariant Dlg2Ui::getValue( const QDomElement& e, const QString& tagName,
			   const QString& type )
{
    QVariant val;

    if ( e.tagName() != tagName )
	return val;

    QString t = e.attributes().namedItem( "type" ).toAttr().value();
    if ( normalizeType(t) != normalizeType(type) )
	return val;

    if ( type == QString("integer") ) {
	return getTextValue( e ).toInt();
    } else if ( type == QString("boolean") ) {
	return QVariant( isTrue(getTextValue(e)), 0 );
    } else if ( type == QString("double") ) {
	return getTextValue( e ).toDouble();
    } else if ( type == QString("qcstring") ) {
	return getTextValue( e ).latin1();
    } else if ( type == QString("enum") || type == QString("qstring") ||
		type == QString("set") ) {
	return getTextValue( e );
    } else {
	QDomNodeList children = e.childNodes();

	if ( type == QString("qsize") ) {
	    int width = getValue( children, QString("Width"),
				  QString("integer") ).toInt();
	    int height = getValue( children, QString("Height"),
				   QString("integer") ).toInt();
	    return QSize( width, height );
	} else if ( type == QString("qrect") ) {
	    int x = getValue( children, QString("X"), QString("integer") )
		    .toInt();
	    int y = getValue( children, QString("Y"), QString("integer") )
		    .toInt();
	    int width = getValue( children, QString("Width"),
				  QString("integer") ).toInt();
	    int height = getValue( children, QString("Height"),
				   QString("integer") ).toInt();
	    return QRect( x, y, width, height );
	} else if ( type == QString("qpoint") ) {
	    int x = getValue( children, QString("X"), QString("integer") )
		    .toInt();
	    int y = getValue( children, QString("Y"), QString("integer") )
		    .toInt();
	    return QPoint( x, y );
	} else if ( type == QString("qpalette") ) {
	    QColorGroup active = getValue( children, QString("Active"),
					   QString("qcolorgroup") )
					   .toColorGroup();
	    QColorGroup disabled = getValue( children, QString("Disabled"),
					   QString("qcolorgroup") )
					   .toColorGroup();
	    QColorGroup inactive = getValue( children, QString("Inactive"),
					   QString("qcolorgroup") )
					   .toColorGroup();
	    return QPalette( active, disabled, inactive );
	} else if ( type == QString("qfont") ) {
	    QString family = getValue( children, QString("Family"),
				       QString("qstring") ).toString();
	    int pointSize = getValue( children, QString("PointSize"),
				      QString("integer") ).toInt();
	    int weight = getValue( children, QString("weight"),
				   QString("integer") ).toInt();
	    bool italic = getValue( children, QString("Italic"),
				    QString("boolean") ).toBool();
	    bool underline = getValue( children, QString("Underline"),
				       QString("boolean") ).toBool();
	    bool strikeOut = getValue( children, QString("StrikeOut"),
				       QString("boolean") ).toBool();
	    int styleHint = getValue( children, QString("StyleHint"),
				      QString("integer") ).toInt();

	    QFont f;
	    if ( !family.isEmpty() )
		f.setFamily( family );
	    if ( pointSize != 0 )
		f.setPointSize( pointSize );
	    if ( weight != 0 )
		f.setWeight( weight );
	    f.setItalic( italic );
	    f.setUnderline( underline );
	    f.setStrikeOut( strikeOut );
	    if ( styleHint != 0 )
		f.setStyleHint( (QFont::StyleHint) styleHint );
	    return f;
	} else if ( type == QString("qcolor") ) {
	    // if any component missing, zero is to be assumed
	    int red = getValue( children, QString("Red"), QString("integer") )
		      .toInt();
	    int green = getValue( children, QString("Green"),
				  QString("integer") ).toInt();
	    int blue = getValue( children, QString("Blue"), QString("integer") )
		       .toInt();
	    return QColor( red, green, blue );
	} else if ( type == QString("qcolorgroup") ) {
	    static const QColorGroup::ColorRole roles[NumColorRoles] = {
		QColorGroup::Foreground, QColorGroup::Button,
		QColorGroup::Light, QColorGroup::Midlight, QColorGroup::Dark,
		QColorGroup::Mid, QColorGroup::Text, QColorGroup::BrightText,
		QColorGroup::ButtonText, QColorGroup::Base,
		QColorGroup::Background, QColorGroup::Shadow,
		QColorGroup::Highlight, QColorGroup::HighlightedText
	    };
	    static const char * const roleNames[NumColorRoles] = {
		"Foreground", "Button", "Light", "MidLight", "Dark", "Mid",
		"Text", "BrightText", "ButtonText", "Base", "Background",
		"Shadow", "HighLighted", "HighLightedText"
	    };
	    QColorGroup group;

	    for ( int i = 0; i < NumColorRoles; i++ )
		group.setColor( roles[i],
				getValue(children, QString(roleNames[i]),
					 QString("qcolor")).toColor() );
	    return group;
	} else {
	    syntaxError();
	}
    }
    return val;
}

void Dlg2Ui::matchDialogCommon( const QDomElement& dialogCommon )
{
    if ( !checkTagName(dialogCommon, QString("DialogCommon")) )
	return;

    QString sourceDir;
    QString classHeader;
    QString classSource;
    QString dataHeader;
    QString dataSource;
    QString dataName;
    QString windowBaseClass( "QDialog" );
    bool isCustom = FALSE;
    QString customBaseHeader;
    QString windowCaption;

    yyClassName = "Form1";

    QDomNode n = dialogCommon.firstChild();
    while ( !n.isNull() ) {
	QString tagName = n.toElement().tagName();
	QString val = getTextValue( n );

	if ( tagName == QString("SourceDir") ) {
	    sourceDir = val;
	} else if ( tagName == QString("ClassHeader") ) {
	    classHeader = val;
	} else if ( tagName == QString("ClassSource") ) {
	    classSource = val;
	} else if ( tagName == QString("ClassName") ) {
	    yyClassName = val;
	} else if ( tagName == QString("DataHeader") ) {
	    dataHeader = val;
	} else if ( tagName == QString("DataSource") ) {
	    dataSource = val;
	} else if ( tagName == QString("DataName") ) {
	    dataName = val;
	} else if ( tagName == QString("WindowBaseClass") ) {
	    if ( val == QString("Custom") )
		isCustom = TRUE;
	    else
		windowBaseClass = val;
	} else if ( tagName == QString("IsModal") ) {
	} else if ( tagName == QString("CustomBase") ) {
	    windowBaseClass = val;
	} else if ( tagName == QString("CustomBaseHeader") ) {
	    customBaseHeader = val;
	} else if ( tagName == QString("WindowCaption") ) {
	    windowCaption = val;
	}
	n = n.nextSibling();
    }

    emitSimpleValue( QString("class"), yyClassName );
    emitOpeningWidget( windowBaseClass );

    if ( windowCaption.isEmpty() )
	windowCaption = yyClassName;
    emitProperty( QString("name"), yyClassName.latin1() );
    emitProperty( QString("caption"), windowCaption );

    if ( isCustom )
	yyCustomWidgets.insert( windowBaseClass, customBaseHeader );
}

bool Dlg2Ui::needsQLayoutWidget( const QDomElement& e )
{
    QRegExp widgetExists( QString("WidgetLayout|Layout_Widget") );

    // we should also check that the widget is not a QHBox, QVBox, or QGrid
    QString grandpa = e.parentNode().parentNode().toElement().tagName();
    return !widgetExists.exactMatch( grandpa );
}

void Dlg2Ui::matchBoxLayout( const QDomElement& boxLayout )
{
    QString directionStr;
    QString prevBoxKind = yyBoxKind;
    int border = 5;
    int autoBorder = 5;
    QString name;
    bool needsWidget = needsQLayoutWidget( boxLayout );
    bool opened = FALSE;

    QDomNode n = boxLayout.firstChild();
    while ( !n.isNull() ) {
	QString tagName = n.toElement().tagName();

	if ( tagName == QString("Children") ) {
	    if ( !opened ) {
		emitOpeningLayout( needsWidget, yyBoxKind, name, border,
				   autoBorder );
		if ( !directionStr.isEmpty() )
		    emitProperty( QString("direction"), directionStr,
				  QString("enum") );
		opened = TRUE;
	    }
	    matchLayout( n.toElement() );
	} else {
	    QString val = getTextValue( n );

	    if ( tagName == QString("Direction") ) {
		if ( val == QString("LeftToRight") ) {
		    yyBoxKind = QString( "hbox" );
		} else if ( val == QString("RightToLeft") ) {
		    directionStr = val;
		    yyBoxKind = QString( "hbox" );
		} else if ( val == QString("TopToBottom") ) {
		    yyBoxKind = QString( "vbox" );
		} else if ( val == QString("BottomToTop") ) {
		    directionStr = val;
		    yyBoxKind = QString( "vbox" );
		} else {
		    syntaxError();
		}
	    } else if ( tagName == QString("Border") ) {
		border = val.toInt();
	    } else if ( tagName == QString("AutoBorder") ) {
		autoBorder = val.toInt();
	    } else if ( tagName == QString("Name") ) {
		name = val;
	    }
	}

	n = n.nextSibling();
    }
    if ( opened ) {
	emitClosingLayout( needsWidget, yyBoxKind );
	yyBoxKind = prevBoxKind;
    }
}

void Dlg2Ui::matchBoxSpacing( const QDomElement& boxSpacing )
{
    int spacing = 7;

    QDomNode n = boxSpacing.firstChild();
    while ( !n.isNull() ) {
	QString val = getTextValue( n );

	if ( n.toElement().tagName() == QString("Spacing") )
	    spacing = val.toInt();
	n = n.nextSibling();
    }
    emitSpacer( spacing, 0 );
}

void Dlg2Ui::matchBoxStretch( const QDomElement& boxStretch )
{
    int stretch = 1;

    QDomNode n = boxStretch.firstChild();
    while ( !n.isNull() ) {
	QString val = getTextValue( n );

	if ( n.toElement().tagName() == QString("Stretch") )
	    stretch = val.toInt();
	n = n.nextSibling();
    }
    emitSpacer( 0, stretch );
}

void Dlg2Ui::matchGridLayout( const QDomElement& gridLayout )
{
    int oldGridRow = yyGridRow;
    int oldGridColumn = yyGridColumn;
    int border = 5;
    int autoBorder = 5;
    QString name;
    QString menu;
    bool needsWidget = needsQLayoutWidget( gridLayout );
    bool opened = FALSE;

    QDomNode n = gridLayout.firstChild();
    while ( !n.isNull() ) {
	QString tagName = n.toElement().tagName();

	if ( tagName == QString("Children") ) {
	    if ( !opened ) {
		emitOpeningLayout( needsWidget, QString("grid"), name, border,
				   autoBorder );
		yyGridRow = -1;
		yyGridColumn = -1;
		opened = TRUE;
	    }
	    matchLayout( n.toElement() );
	} else {
	    if ( tagName == QString("Border") ) {
		border = getTextValue( n ).toInt();
	    } else if ( tagName == QString("AutoBorder") ) {
		autoBorder = getTextValue( n ).toInt();
	    } else if ( tagName == QString("Name") ) {
		name = getTextValue( n );
	    } else if ( tagName == QString("Menu") ) {
		menu = getTextValue( n );
	    }
	}
	n = n.nextSibling();
    }
    if ( opened )
	emitClosingLayout( needsWidget, QString("grid") );
    yyGridRow = oldGridRow;
    yyGridColumn = oldGridColumn;
}

void Dlg2Ui::matchGridRow( const QDomElement& gridRow )
{
    yyGridRow++;

    QDomNode n = gridRow.firstChild();
    while ( !n.isNull() ) {
	QString tagName = n.toElement().tagName();

	if ( tagName == QString("Children") ) {
	    yyGridColumn = 0;
	    matchLayout( n.toElement() );
	}
	n = n.nextSibling();
    }
}

void Dlg2Ui::matchGridSpacer( const QDomElement& gridSpacer )
{
    if ( !gridSpacer.firstChild().isNull() )
	syntaxError();
}

void Dlg2Ui::matchLayoutWidget( const QDomElement& layoutWidget )
{
    QDomElement children;
    QString widget;

    QDomNode n = layoutWidget.firstChild();
    while ( !n.isNull() ) {
	QString tagName = n.toElement().tagName();

	if ( tagName == QString("Children") )
	    children = n.toElement();
	else if ( tagName == QString("Widget") )
	    widget = getTextValue( n );
	n = n.nextSibling();
    }

    if ( !widget.isEmpty() ) {
	QMap<QString, QDomElement>::Iterator w = yyWidgetMap.find( widget );
	if ( w == yyWidgetMap.end() ) {
	    syntaxError();
	} else {
	    QString className = widgetClassName( *w );
	    if ( className == QString("QHBox") ||
		 className == QString("QVBox") ) {
		bool needsWidget = needsQLayoutWidget( layoutWidget );

		QString prevBoxKind = yyBoxKind;
		yyBoxKind = className.mid( 1 ).lower();

		int spacing = getValue( (*w).childNodes(), QString("Spacing"),
					QString("integer") ).toInt();
		if ( spacing < 1 )
		    spacing = 5;

		emitOpeningLayout( needsWidget, yyBoxKind, widget, 0, spacing );
		if ( !children.isNull() )
		    matchLayout( children );
		emitClosingLayout( needsWidget, yyBoxKind );
		yyBoxKind = prevBoxKind;
	    } else if ( className == QString("QGrid") ) {
		bool needsWidget = needsQLayoutWidget( layoutWidget );
		int n = 0;

		QString direction = getValue( (*w).childNodes(),
					      QString("Direction") ).toString();
		int rowsCols = getValue( (*w).childNodes(), QString("RowCols"),
					 QString("integer") ).toInt();
		if ( rowsCols == 0 )
		    rowsCols = getValue( (*w).childNodes(),
					 QString("RowsCols"),
					 QString("integer") ).toInt();
		if ( rowsCols < 1 )
		    rowsCols = 5;
		int spacing = getValue( (*w).childNodes(), QString("Spacing"),
					QString("integer") ).toInt();
		if ( spacing < 1 )
		    spacing = 5;

		emitOpeningLayout( needsWidget, QString("grid"), widget, 0,
				   spacing );

		QDomNode child = children.firstChild();
		while ( !child.isNull() ) {
		    if ( direction == QString("Vertical") ) {
			yyGridColumn = n / rowsCols;
			yyGridRow = n % rowsCols;
		    } else {
			yyGridColumn = n % rowsCols;
			yyGridRow = n / rowsCols;
		    }
		    matchBox( child.toElement() );
		    n++;
		    child = child.nextSibling();
		}
		yyGridColumn = -1;
		yyGridRow = -1;
		emitClosingLayout( needsWidget, QString("grid") );
	    } else {
		emitOpeningWidget( widgetClassName(*w) );
		emitWidgetBody( *w, TRUE );
		if ( !children.isNull() )
		    matchLayout( children );
		emitClosing( QString("widget") );
	    }
	    yyWidgetMap.remove( w );
	}
    }
}

void Dlg2Ui::matchBox( const QDomElement& box )
{
    /*
      What is this jump table doing in here?
    */
    static const struct {
	const char *tagName;
	void (Dlg2Ui::*matchFunc)( const QDomElement& );
    } jumpTable[] = {
	{ "Box_Layout", &Dlg2Ui::matchBoxLayout },
	{ "Box_Spacing", &Dlg2Ui::matchBoxSpacing },
	{ "Box_Stretch", &Dlg2Ui::matchBoxStretch },
	{ "Grid_Layout", &Dlg2Ui::matchGridLayout },
	{ "Grid_Row", &Dlg2Ui::matchGridRow },
	{ "Grid_Spacer", &Dlg2Ui::matchGridSpacer },
	{ "Layout_Widget", &Dlg2Ui::matchLayoutWidget },
	{ 0, 0 }
    };

    int i = 0;
    while ( jumpTable[i].tagName != 0 ) {
	if ( QString(jumpTable[i].tagName) == box.tagName() ) {
	    (this->*jumpTable[i].matchFunc)( box );
	    break;
	}
	i++;
    }
    if ( jumpTable[i].tagName == 0 )
	syntaxError();
}

void Dlg2Ui::matchLayout( const QDomElement& layout )
{
    int column = yyGridColumn;

    QDomNode n = layout.firstChild();
    while ( !n.isNull() ) {
	if ( column != -1 )
	    yyGridColumn = column++;
	matchBox( n.toElement() );
	n = n.nextSibling();
    }
}

void Dlg2Ui::matchWidgetLayoutCommon( const QDomElement& widgetLayoutCommon )
{
    QDomNodeList children = widgetLayoutCommon.childNodes();

    /*
      Since we do not respect the spacing and margins specified in
      the .dlg file, the specified geometry is slightly wrong (too
      small). It still seems to be better to take it in.
    */
#if 1
    QPoint initialPos = getValue( children, QString("InitialPos"),
				  QString("qpoint") ).toPoint();
    QSize size = getValue( children, QString("Size"), QString("qsize") )
		 .toSize();
#endif
    QSize minSize = getValue( children, QString("MinSize"), QString("qsize") )
		    .toSize();
    QSize maxSize = getValue( children, QString("MaxSize"), QString("qsize") )
		    .toSize();

#if 1
    if ( initialPos == QPoint(-1, -1) )
	initialPos = QPoint( 0, 0 );

    emitProperty( QString("geometry"), QRect(initialPos, size) );
#endif
    if ( minSize != QSize(-1, -1) )
	emitProperty( QString("minimumSize"), minSize );
    if ( maxSize != QSize(32767, 32767) )
	emitProperty( QString("maximumSize"), maxSize );
}

void Dlg2Ui::matchWidget( const QDomElement& widget )
{
    QString name;

    QDomNode n = widget;
    while ( !n.isNull() ) {
	if ( isWidgetType(n.toElement()) ) {
	    n = n.firstChild();
	} else {
	    if ( n.toElement().tagName() == QString("Name") ) {
		name = getTextValue( n );
		break;
	    }
	    n = n.nextSibling();
	}
    }
    if ( name.isEmpty() )
	name = QString( "Widget%1" ).arg( uniqueWidget++ );

    if ( yyWidgetMap.contains(name) )
	syntaxError();
    yyWidgetMap.insert( name, widget );
}

void Dlg2Ui::matchWidgets( const QDomElement& widgets )
{
    QDomNode n = widgets.firstChild();
    while ( !n.isNull() ) {
	matchWidget( n.toElement() );
	n = n.nextSibling();
    }
}

void Dlg2Ui::matchTabOrder( const QDomElement& tabOrder )
{
    QDomNode n = tabOrder.firstChild();
    while ( !n.isNull() ) {
	if ( n.toElement().tagName() == QString("Widget") )
	    yyTabStops.append( getTextValue(n.toElement()) );
	n = n.nextSibling();
    }
}

void Dlg2Ui::matchWidgetLayout( const QDomElement& widgetLayout )
{
    if ( !checkTagName(widgetLayout, QString("WidgetLayout")) )
	return;

    QDomNode n = widgetLayout.firstChild();
    while ( !n.isNull() ) {
	QString tagName = n.toElement().tagName();

	if ( tagName == QString("WidgetLayoutCommon") ) {
	    matchWidgetLayoutCommon( n.toElement() );
	} else if ( tagName == QString("Widgets") ) {
	    matchWidgets( n.toElement() );
	} else if ( tagName == QString("TabOrder") ) {
	    matchTabOrder( n.toElement() );
	} else if ( tagName == QString("Layout") ) {
	    matchLayout( n.toElement() );
	}
	n = n.nextSibling();
    }
}

void Dlg2Ui::matchDialog( const QDomElement& dialog )
{
    if ( !checkTagName(dialog, QString("Dialog")) )
	return;

    QDomNodeList nodes = dialog.childNodes();
    if ( nodes.count() == 2 ) {
	matchDialogCommon( nodes.item(0).toElement() );
	matchWidgetLayout( nodes.item(1).toElement() );
	flushWidgets();
	emitClosing( QString("widget") );

	if ( !yyCustomWidgets.isEmpty() ) {
	    emitOpening( QString("customwidgets") );

	    QMap<QString, QString>::Iterator w = yyCustomWidgets.begin();
	    while ( w != yyCustomWidgets.end() ) {
		emitOpening( QString("customwidget") );
		emitSimpleValue( QString("class"), w.key() );
		if ( !(*w).isEmpty() )
		    emitSimpleValue( QString("header"), *w,
				     attribute(QString("location"),
					       QString("local")) );
		emitClosing( QString("customwidget") );
		++w;
	    }
	    emitClosing( QString("customwidgets") );
	}

	if ( yyConnections.count() + yySlots.count() > 0 ) {
	    emitOpening( QString("connections") );

	    QValueList<DlgConnection>::Iterator c = yyConnections.begin();
	    while ( c != yyConnections.end() ) {
		emitOpening( QString("connection") );
		emitSimpleValue( QString("sender"), alias((*c).sender) );
		emitSimpleValue( QString("signal"), (*c).signal );
		emitSimpleValue( QString("receiver"), yyClassName );
		emitSimpleValue( QString("slot"), (*c).slot );
		emitClosing( QString("connection") );
		++c;
	    }

	    QMap<QString, QString>::Iterator s = yySlots.begin();
	    while ( s != yySlots.end() ) {
		AttributeMap attr;
		attr.insert( QString("access"), *s );
		attr.insert( QString("language"), QString("C++") );
		attr.insert( QString("returntype"), QString("void") );
		emitSimpleValue( QString("slot"), s.key(), attr );
		++s;
	    }
	    emitClosing( QString("connections") );
	}

	if ( !yyTabStops.isEmpty() ) {
	    emitOpening( QString("tabstops") );
	    QStringList::ConstIterator t = yyTabStops.begin();
	    while ( t != yyTabStops.end() ) {
		emitSimpleValue( QString("tabstop"), alias(*t) );
		++t;
	    }
	    emitClosing( QString("tabstops") );
	}
    }
}

QStringList Dlg2Ui::convertQtArchitectDlgFile( const QString& fileName )
{
    int i;

    yyFileName = fileName;
    yyLayoutDepth = 0;
    yyGridRow = -1;
    yyGridColumn = -1;

    numErrors = 0;
    uniqueLayout = 1;
    uniqueSpacer = 1;
    uniqueWidget = 1;

    i = 0;
    while ( widgetTypes[i] != 0 ) {
	yyWidgetTypeSet.insert( QString(widgetTypes[i]), 0 );
	i++;
    }

    i = 0;
    while ( propertyDefs[i].widgetName != 0 ) {
	yyPropertyMap[QString(propertyDefs[i].widgetName)]
		.insert( QString(propertyDefs[i].architectName), i );
	i++;
    }

    QDomDocument doc( QString("QtArch") );
    QFile f( fileName );
    if ( !f.open(IO_ReadOnly) ) {
	return QStringList();
    }
    if ( !doc.setContent(&f) ) {
	QString firstLine;
	f.at( 0 );
	f.readLine( firstLine, 128 );
	firstLine = firstLine.stripWhiteSpace();
	if ( firstLine.startsWith(QString("DlgEdit:v1")) ) {
	    error( QString("This file is a Qt Architect 1.x file. Qt Designer"
			   " can only read XML dialog files, as generated by Qt"
			   " Architect 2.1 or above."
			   "<p>To convert this file to the right format,"
			   " first install Qt Architect 2.1 (available at"
			   " <tt>http://qtarch.sourceforge.net/</tt>). Use the"
			   " <i>update20.pl</i> Perl script to update the file"
			   " to the 2.0 format. Load that file in Qt"
			   " Architect and save it. The file should now be in"
			   " XML format and loadable in Qt Designer.") );
	} else if ( firstLine.startsWith(QString("DlgEdit::v2")) ) {
	    error( QString("This file is a Qt Architect 2.0 file. Qt Designer"
			   " can only read XML dialog files, as generated by Qt"
			   " Architect 2.1 or above."
			   "<p>To convert this file to the right format,"
			   " first install Qt Architect 2.1 (available at"
			   " <tt>http://qtarch.sourceforge.net/</tt>). Load the"
			   " 2.0 file in Qt Architect and save it. The file"
			   " should now be in XML format and loadable in Qt"
			   " Designer.") );
	} else {
	    error( QString("The file you gave me is not an XML file, as far as"
			   " I can tell.") );
	}

	f.close();
	return QStringList();
    }
    f.close();

    QDomElement root = doc.documentElement();
    if ( root.tagName() != QString("QtArch") ||
	 root.attributeNode("type").value() != QString("Dialog") ) {
	error( QString("The file you gave me is not a Qt Architect dialog"
		       " file.") );
	return QStringList();
    }

    emitHeader();

    QDomNode n = root.firstChild();
    while ( !n.isNull() ) {
	// there should be only one
	matchDialog( n.toElement() );
	n = n.nextSibling();
    }

    emitFooter();

    QFile outf;
    QString outFileName = yyClassName + QString( ".ui" );

    outf.setName( outFileName );
    if ( !outf.open(IO_WriteOnly) ) {
	qWarning( "dlg2ui: Could not open output file '%s'",
		  outFileName.latin1() );
	return QStringList();
    }

    QTextStream out;
    out.setEncoding( QTextStream::Latin1 );
    out.setDevice( &outf );
    out << yyOut;
    outf.close();

    return QStringList( outFileName );
}
