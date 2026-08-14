/**********************************************************************
**
** Converts a Glade .glade file into a .ui file.
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

#include "glade2ui.h"

#include <qapplication.h>
#include <qfile.h>
#include <qimage.h>
#include <qprogressdialog.h>
#include <qmessagebox.h>
#include <qrect.h>
#include <qregexp.h>
#include <qsizepolicy.h>

#include <ctype.h>

static const struct {
    const char *gtkName;
    const char *qtName;
} classNames[] = {
    { "Custom", "Custom" },
    { "GnomeAbout", "QDialog" },
    { "GnomeApp", "QMainWindow" },
    { "GnomeCanvas", "QLabel" },
    { "GnomeColorPicker", "QComboBox" },
    { "GnomeDateEdit", "QDateTimeEdit" },
    { "GnomeDialog", "QDialog" },
    { "GnomeFontPicker", "QComboBox" },
    { "GnomeIconSelection", "QIconView" },
    { "GnomePixmap", "QLabel" },
    { "GnomePropertyBox", "QDialog" },
    { "GtkAccelLabel", "QLabel" },
    { "GtkAspectFrame", "QFrame" },
    { "GtkButton", "QPushButton" },
    { "GtkCList", "QListView" },
    { "GtkCTree", "QListView" },
    { "GtkCheckButton", "QCheckBox" },
    { "GtkCombo", "QComboBox" },
    { "GtkDial", "QDial" },
    { "GtkDialog", "QDialog" },
    { "GtkEntry", "QLineEdit" },
    { "GtkFixed", "QLayoutWidget" },
    { "GtkFrame", "QFrame" },
    { "GtkHPaned", "QSplitter" },
    { "GtkHScale", "QSlider" },
    { "GtkHScrollbar", "QScrollBar" },
    { "GtkHSeparator", "Line" },
    { "GtkHandleBox", "QFrame" },
    { "GtkImage", "QLabel" },
    { "GtkLabel", "QLabel" },
    { "GtkList", "QListBox" },
    { "GtkNotebook", "QTabWidget" },
    { "GtkOptionMenu", "QComboBox" },
    { "GtkPixmap", "QLabel" },
    { "GtkPreview", "QLabel" },
    { "GtkProgressBar", "QProgressBar" },
    { "GtkRadioButton", "QRadioButton" },
    { "GtkSpinButton", "QSpinBox" },
    { "GtkStatusbar", "QStatusBar" },
    { "GtkText", "QTextEdit" },
    { "GtkToggleButton", "QPushButton" },
    { "GtkTree", "QListView" },
    { "GtkVPaned", "QSplitter" },
    { "GtkVScale", "QSlider" },
    { "GtkVScrollbar", "QScrollBar" },
    { "GtkVSeparator", "Line" },
    { "Placeholder", "QLabel" },
    { 0, 0 }
};

static const struct {
    const char *name;
    const char *menuText;
} stockMenuItems[] = {
    { "ABOUT", "&About" },
    { "CLEAR", "C&lear" },
    { "CLOSE", "&Close" },
    { "CLOSE_WINDOW", "&Close This Window" },
    { "COPY", "&Copy" },
    { "CUT", "C&ut" },
    { "END_GAME", "&End Game" },
    { "EXIT", "E&xit" },
    { "FIND", "&Find..." },
    { "FIND_AGAIN", "Find &Again" },
    { "HINT", "&Hint" },
    { "NEW", "&New" },
    { "NEW_GAME", "&New Game" },
    { "NEW_WINDOW", "Create New &Window" },
    { "OPEN", "&Open..." },
    { "PASTE", "&Paste" },
    { "PAUSE_GAME", "&Pause Game" },
    { "PREFERENCES", "&Preferences..." },
    { "PRINT", "&Print" },
    { "PRINT_SETUP", "Print S&etup..." },
    { "PROPERTIES", "&Properties..." },
    { "REDO", "&Redo" },
    { "REDO_MOVE", "&Redo Move" },
    { "REPLACE", "&Replace..." },
    { "RESTART_GAME", "&Restart Game" },
    { "REVERT", "&Revert" },
    { "SAVE", "&Save" },
    { "SAVE_AS", "Save &As..." },
    { "SCORES", "&Scores..." },
    { "SELECT_ALL", "&Select All" },
    { "UNDO", "&Undo" },
    { "UNDO_MOVE", "&Undo Move" },
    { 0, 0 }
};

static const struct {
    const char *gtkName;
    int qtValue;
} keys[] = {
    { "BackSpace", Qt::Key_BackSpace },
    { "Delete", Qt::Key_Delete },
    { "Down", Qt::Key_Down },
    { "End", Qt::Key_End },
    { "Escape", Qt::Key_Escape },
    { "F1", Qt::Key_F1 },
    { "F10", Qt::Key_F10 },
    { "F11", Qt::Key_F11 },
    { "F12", Qt::Key_F12 },
    { "F2", Qt::Key_F2 },
    { "F3", Qt::Key_F3 },
    { "F4", Qt::Key_F4 },
    { "F5", Qt::Key_F5 },
    { "F6", Qt::Key_F6 },
    { "F7", Qt::Key_F7 },
    { "F8", Qt::Key_F8 },
    { "F9", Qt::Key_F9 },
    { "Home", Qt::Key_Home },
    { "Insert", Qt::Key_Insert },
    { "KP_Enter", Qt::Key_Enter },
    { "Left", Qt::Key_Left },
    { "Page_Down", Qt::Key_PageDown },
    { "Page_Up", Qt::Key_PageUp },
    { "Return", Qt::Key_Return },
    { "Right", Qt::Key_Right },
    { "Tab", Qt::Key_Tab },
    { "Up", Qt::Key_Up },
    { "space", Qt::Key_Space },
    { 0, 0 }
};

static QString nonMenuText( const QString& menuText )
{
    QString t;
    int len = (int) menuText.length();
    if ( menuText.endsWith(QString("...")) )
	len -= 3;
    for ( int i = 0; i < len; i++ ) {
	if ( menuText[i] != QChar('&') )
	    t += menuText[i];
    }
    return t;
}

/*
  Some GTK dialog use hyphens in variable names. Let's take no chance.
*/
static QString fixedName( const QString& name )
{
    const char *latin1 = name.latin1();
    QString fixed;

    int i = 0;
    while ( latin1 != 0 && latin1[i] != '\0' ) {
	if ( isalnum(latin1[i]) )
	    fixed += name[i];
	else
	    fixed += QChar( '_' );
	i++;
    }
    return fixed;
}

/*
  Returns an hexadecimal rendering of a block of memory.
*/
static QString hexed( const char *data, int length )
{
    QString t;
    for ( int i = 0; i < length; i++ ) {
	QString x;
	x.sprintf( "%.2x", (uchar) data[i] );
	t += x;
    }
    return t;
}

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

Glade2Ui::Glade2Ui()
{
    int i = 0;
    while ( classNames[i].gtkName != 0 ) {
	yyClassNameMap.insert( QString(classNames[i].gtkName),
			       QString(classNames[i].qtName) );
	i++;
    }

    i = 0;
    while ( stockMenuItems[i].name != 0 ) {
	yyStockMap.insert( QString(stockMenuItems[i].name),
			   QString(stockMenuItems[i].menuText) );
	i++;
    }

    i = 0;
    while ( keys[i].gtkName != 0 ) {
	yyKeyMap.insert( QString(keys[i].gtkName), keys[i].qtValue );
	i++;
    }
}

QString Glade2Ui::imageName( const QString& fileName )
{
    return *yyImages.insert( fileName, QString("image%1").arg(yyImages.count()),
			     FALSE );
}

QString Glade2Ui::opening( const QString& tag, const AttributeMap& attr )
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

QString Glade2Ui::closing( const QString& tag )
{
    return opening( QChar('/') + tag );
}

QString Glade2Ui::atom( const QString& tag, const AttributeMap& attr )
{
    QString t = opening( tag, attr );
    t.insert( t.length() - 1, QChar('/') );
    return t;
}

void Glade2Ui::error( const QString& message )
{
    if ( numErrors++ == 0 )
	QMessageBox::warning( 0, yyFileName, message );
}

void Glade2Ui::syntaxError()
{
    error( QString("Sorry, I met a random syntax error. I did what I could, but"
		   " that was not enough."
		   "<p>You might want to write to"
		   " <tt>qt-bugs@trolltech.com</tt> about this incident.") );
}

QString Glade2Ui::getTextValue( const QDomNode& node )
{
    if ( node.childNodes().count() > 1 ) {
	syntaxError();
	return QString::null;
    }

    if ( node.childNodes().count() == 0 )
	return QString::null;

    QDomText t = node.firstChild().toText();
    if ( t.isNull() ) {
	syntaxError();
	return QString::null;
    }
    return t.data().stripWhiteSpace();
}

void Glade2Ui::emitHeader()
{
    yyOut += QString( "<!DOCTYPE UI><UI version=\"3.0\" stdsetdef=\"1\">\n" );
}

void Glade2Ui::emitFooter()
{
    yyOut += QString( "</UI>\n" );
}

void Glade2Ui::emitSimpleValue( const QString& tag, const QString& value,
				const AttributeMap& attr )
{
    yyOut += yyIndentStr + opening( tag, attr ) + entitize( value ) +
	     closing( tag ) + QChar( '\n' );
}

void Glade2Ui::emitOpening( const QString& tag, const AttributeMap& attr )
{
    yyOut += yyIndentStr + opening( tag, attr ) + QChar( '\n' );
    yyIndentStr += QString( "    " );
}

void Glade2Ui::emitClosing( const QString& tag )
{
    yyIndentStr.truncate( yyIndentStr.length() - 4 );
    yyOut += yyIndentStr + closing( tag ) + QChar( '\n' );
}

void Glade2Ui::emitAtom( const QString& tag, const AttributeMap& attr )
{
    yyOut += yyIndentStr + atom( tag, attr ) + QChar( '\n' );
}

void Glade2Ui::emitVariant( const QVariant& val, const QString& stringType )
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
	case QVariant::Size:
	    emitOpening( QString("size") );
	    emitSimpleValue( QString("width"),
			     QString::number(val.toSize().width()) );
	    emitSimpleValue( QString("height"),
			     QString::number(val.toSize().height()) );
	    emitClosing( QString("size") );
	    break;
	case QVariant::SizePolicy:
	    emitOpening( QString("sizepolicy") );
	    emitSimpleValue( QString("hsizetype"),
			     QString::number((int) val.toSizePolicy()
						      .horData()) );
	    emitSimpleValue( QString("vsizetype"),
			     QString::number((int) val.toSizePolicy()
						      .verData()) );
	    emitClosing( QString("sizepolicy") );
	    break;
	default:
	    emitSimpleValue( QString("fnord"), QString::null );
	}
    }
}

void Glade2Ui::emitProperty( const QString& prop, const QVariant& val,
			     const QString& stringType )
{
    emitOpening( QString("property"), attribute(QString("name"), prop) );
    emitVariant( val, stringType );
    emitClosing( QString("property") );
}

void Glade2Ui::emitFontProperty( const QString& prop, int pointSize, bool bold )
{
    emitOpening( QString("property"), attribute(QString("name"), prop) );
    emitOpening( QString("font") );
    emitSimpleValue( QString("pointsize"), QString::number(pointSize) );
    if ( bold )
	emitSimpleValue( QString("bold"), QString("1") );
    emitClosing( QString("font") );
    emitClosing( QString("property") );
}

void Glade2Ui::emitAttribute( const QString& prop, const QVariant& val,
			      const QString& stringType )
{
    emitOpening( QString("attribute"), attribute(QString("name"), prop) );
    emitVariant( val, stringType );
    emitClosing( QString("attribute") );
}

static QString accelerate( const QString& gtkLabel )
{
    QString qtLabel = gtkLabel;
    qtLabel.replace( '&', QString("&&") );
    // close but not quite right
    qtLabel.replace( QChar('_'), QChar('&') );
    return qtLabel;
}

static QString decelerate( const QString& gtkLabel )
{
    QString qtLabel = gtkLabel;
    // ditto
    qtLabel.replace( '_', QString::null );
    return qtLabel;
}

/*
  Converting a GTK widget to a corresponding Qt widget is sometimes
  hard. For example, a GtkScrolledWindow should sometimes be converted
  into a QTextView, a QTextEdit, a QListView or whatever. What we do
  is pretty hackish, but it mostly works.
*/
QString Glade2Ui::gtk2qtClass( const QString& gtkClass,
			       const QValueList<QDomElement>& childWidgets )
{
    QRegExp gnomeXEntry( QString("Gnome(File|Number|Pixmap)?Entry") );

    QString qtClass;

    if ( gtkClass == QString("GtkScrolledWindow") ) {
	if ( childWidgets.count() == 1 ) {
	    QString g;
	    bool editable = FALSE;
	    bool showTitles = TRUE;

	    QDomNode n = childWidgets.first().firstChild();
	    while ( !n.isNull() ) {
		QString tagName = n.toElement().tagName();
		if ( tagName == QString("class") ) {
		    g = getTextValue( n );
		} else if ( tagName == QString("editable") ) {
		    editable = isTrue( getTextValue(n) );
		} else if ( tagName.startsWith(QString("show_tit")) ) {
		    showTitles = isTrue( getTextValue(n) );
		}
		n = n.nextSibling();
	    }

	    if ( g == QString("GnomeCanvas") ||
		 g == QString("GtkDrawingArea") ) {
		qtClass = QString( "QLabel" );
	    } else if ( g == QString("GnomeIconList") ) {
		qtClass = QString( "QIconView" );
	    } else if ( g == QString("GtkCList") ) {
		if ( showTitles )
		    qtClass = QString( "QListView" );
		else
		    qtClass = QString( "QListBox" );
	    } else if ( g == QString("GtkCTree") ) {
		qtClass = QString( "QListView" );
	    } else if ( g == QString("GtkList") ) {
		qtClass = QString( "QListBox" );
	    } else if ( g == QString("GtkText") ) {
		if ( editable )
		    qtClass = QString( "QTextEdit" );
		else
		    qtClass = QString( "QTextView" );
	    } else if ( g == QString("GtkTree") ) {
		qtClass = QString( "QListView" );
	    }
	    // else too bad (qtClass is empty)
	}
    } else if ( gtkClass == QString("GtkWindow") ) {
	qtClass = QString( "QDialog" );
	if ( childWidgets.count() == 1 ) {
	    QString g;

	    QDomNode n = childWidgets.first().firstChild();
	    while ( !n.isNull() ) {
		QString tagName = n.toElement().tagName();
		if ( tagName == QString("class") )
		    g = getTextValue( n );
		n = n.nextSibling();
	    }
	    if ( g == QString("GnomeDruid") )
		qtClass = QString( "QWizard" );
	}
    /*
      GnomeEntry and friends are a wrapper around a GtkEntry. We only
      want the GtkEntry child.
    */
    } else if ( !gnomeXEntry.exactMatch(gtkClass) &&
		gtkClass != QString("GtkAlignment") &&
		gtkClass != QString("GtkEventBox") ) {
	qtClass = yyClassNameMap[gtkClass];
	if ( qtClass.isEmpty() )
	    qtClass = QString( "Unknown" );
    }
    return qtClass;
}

static QString gtk2qtScrollBarMode( const QString& scrollbarPolicy )
{
    if ( scrollbarPolicy.endsWith(QString("_NEVER")) ) {
	return QString( "AlwaysOff" );
    } else if ( scrollbarPolicy.endsWith(QString("_ALWAYS")) ) {
	return QString( "AlwaysOn" );
    } else {
	return QString( "Auto" );
    }
}

static QString gtk2qtSelectionMode( const QString& selectionMode )
{
    if ( selectionMode.endsWith(QString("_MULTIPLE")) )
	return QString( "Multi" );
    else if ( selectionMode.endsWith(QString("_EXTENDED")) )
	return QString( "Extended" );
    else
	return QString( "Single" );
}

int Glade2Ui::matchAccelOnActivate( const QDomElement& accel )
{
    QString key;
    QString modifiers;

    QDomNode n = accel.firstChild();
    while ( !n.isNull() ) {
	QString tagName = n.toElement().tagName();
	if ( tagName == QString("key") ) {
	    key = getTextValue( n );
	    if ( !key.startsWith(QString("GDK_")) )
		return 0;
	} else if ( tagName == QString("modifiers") ) {
	    modifiers = getTextValue( n );
	} else if ( tagName == QString("signal") ) {
	    if ( getTextValue(n) != QString("activate") )
		return 0;
	}
	n = n.nextSibling();
    }

    int flags = 0;

    if ( key.length() == 5 ) {
	flags = key[4].upper().latin1();
    } else if ( yyKeyMap.contains(key.mid(4)) ) {
	flags = yyKeyMap[key.mid(4)];
    } else {
	return 0;
    }

    if ( modifiers.contains(QString("_CONTROL_")) )
	flags |= Qt::CTRL;
    if ( modifiers.contains(QString("_SHIFT_")) )
	flags |= Qt::SHIFT;
    if ( modifiers.contains(QString("_MOD1_")) )
	flags |= Qt::ALT;
    return flags;
}

void Glade2Ui::emitGtkMenu( const QDomElement& menu )
{
    QRegExp gnomeuiinfoMenuXItem( QString("GNOMEUIINFO_MENU_(.+)_ITEM") );

    QDomNode n = menu.firstChild();
    while ( !n.isNull() ) {
	QString tagName = n.toElement().tagName();
	if ( tagName == QString("widget") ) {
	    QString activateHandler;
	    QString gtkClass;
	    QString icon;
	    QString label;
	    QString name;
	    QString stockItem;
	    QString tooltip;
	    int qtAccel = 0;

	    QDomNode child = n.firstChild();
	    while ( !child.isNull() ) {
		QString childTagName = child.toElement().tagName();
		if ( childTagName == QString("accelerator") ) {
		    qtAccel = matchAccelOnActivate( child.toElement() );
		} else if ( childTagName == QString("class") ) {
		    gtkClass = getTextValue( child );
		} else if ( childTagName == QString("icon") ) {
		    icon = getTextValue( child );
		} else if ( childTagName == QString("label") ) {
		    label = getTextValue( child );
		} else if ( childTagName == QString("name") ) {
		    name = getTextValue( child );
		} else if ( childTagName == QString("signal") ) {
		    QString signalName;
		    QString signalHandler;

		    QDomNode grandchild = child.firstChild();
		    while ( !grandchild.isNull() ) {
			QString grandchildTagName =
				grandchild.toElement().tagName();
			if ( grandchildTagName == QString("handler") ) {
			    signalHandler = getTextValue( grandchild );
			} else if ( grandchildTagName == QString("name") ) {
			    signalName = getTextValue( grandchild );
			}
			grandchild = grandchild.nextSibling();
		    }
		    if ( signalName == QString("activate") )
			activateHandler = signalHandler;
		} else if ( childTagName == QString("stock_item") ) {
		    stockItem = getTextValue( child );
		} else if ( childTagName == QString("tooltip") ) {
		    tooltip = getTextValue( child );
		}
		child = child.nextSibling();
	    }

	    if ( label.length() + stockItem.length() == 0 ) {
		emitAtom( QString("separator") );
	    } else {
		if ( name.isEmpty() )
		    name = QString( "action%1" ).arg( uniqueAction++ );
		emitAtom( QString("action"), attribute(QString("name"), name) );

		if ( !activateHandler.isEmpty() ) {
		    QString slot = activateHandler + QString( "()" );
		    GladeConnection c;
		    c.sender = name;
		    c.signal = QString( "activated()" );
		    c.slot = slot;
		    yyConnections.push_back( c );
		    yySlots.insert( slot, QString("public") );
		}

		QString x;
		GladeAction a;

		if ( gnomeuiinfoMenuXItem.exactMatch(stockItem) ) {
		    x = gnomeuiinfoMenuXItem.cap( 1 );
		    a.menuText = yyStockMap[x];
		    if ( x == QString("EXIT") && qtAccel == 0 )
			qtAccel = Qt::CTRL + Qt::Key_Q;
		} else {
		    a.menuText = accelerate( label );
		}
		a.text = nonMenuText( a.menuText );
		a.toolTip = tooltip;
		a.accel = qtAccel;
		a.iconSet = icon;
		yyActions.insert( name, a );
		if ( !x.isEmpty() )
		    yyStockItemActions.insert( x, name );
	    }
	}
	n = n.nextSibling();
    }
}

void Glade2Ui::emitGtkMenuBarChildWidgets(
	const QValueList<QDomElement>& childWidgets )
{
    QRegExp gnomeuiinfoMenuXTree( QString("GNOMEUIINFO_MENU_(.+)_TREE") );

    emitOpening( QString("menubar") );
    emitProperty( QString("name"),
		  QString("MenuBar%1").arg(uniqueMenuBar++).latin1() );

    QValueList<QDomElement>::ConstIterator c = childWidgets.begin();
    while ( c != childWidgets.end() ) {
	QValueList<QDomElement> grandchildWidgets;
	QString gtkClass;
	QString label;
	QString name;
	QString stockItem;

	QDomNode n = (*c).firstChild();
	while ( !n.isNull() ) {
	    QString tagName = n.toElement().tagName();
	    if ( tagName == QString("class") ) {
		gtkClass = getTextValue( n );
	    } else if ( tagName == QString("label") ) {
		label = getTextValue( n );
	    } else if ( tagName == QString("name") ) {
		name = getTextValue( n );
	    } else if ( tagName == QString("stock_item") ) {
		stockItem = getTextValue( n );
	    } else if ( tagName == QString("widget") ) {
		grandchildWidgets.push_back( n.toElement() );
	    }
	    n = n.nextSibling();
	}

	if ( gtkClass == QString("GtkMenuItem") &&
	     grandchildWidgets.count() == 1 ) {
	    QString text;
	    if ( gnomeuiinfoMenuXTree.exactMatch(stockItem) ) {
		text = gnomeuiinfoMenuXTree.cap( 1 );
		if ( text == QString("Files") )
		    text = QString( "Fi&les" );
		else
		    text = QChar( '&' ) + text.left( 1 ) +
			   text.mid( 1 ).lower();
	    } else {
		text = accelerate( label );
	    }

	    AttributeMap attr;
	    attr.insert( QString("name"), name );
	    attr.insert( QString("text"), text );

	    emitOpening( QString("item"), attr );
	    emitGtkMenu( grandchildWidgets.first() );
	    emitClosing( QString("item") );
	}
	++c;
    }
    emitClosing( QString("menubar") );
}

void Glade2Ui::emitGtkToolbarChildWidgets(
	const QValueList<QDomElement>& childWidgets )
{
    QRegExp gnomeStockPixmapX( QString("GNOME_STOCK_PIXMAP_(.+)") );

    emitOpening( QString("toolbar"), attribute(QString("dock"), QString("2")) );
    emitProperty( QString("name"),
		  QString("ToolBar%1").arg(uniqueToolBar++).latin1() );

    QValueList<QDomElement>::ConstIterator c = childWidgets.begin();
    while ( c != childWidgets.end() ) {
	QString childName;
	QString icon;
	QString label;
	QString name;
	QString stockPixmap;
	QString tooltip;

	QDomNode n = (*c).firstChild();
	while ( !n.isNull() ) {
	    QString tagName = n.toElement().tagName();
	    if ( tagName == QString("child_name") ) {
		childName = getTextValue( n );
	    } else if ( tagName == QString("icon") ) {
		icon = getTextValue( n );
	    } else if ( tagName == QString("label") ) {
		label = getTextValue( n );
	    } else if ( tagName == QString("name") ) {
		name = getTextValue( n );
	    } else if ( tagName == QString("stock_pixmap") ) {
		stockPixmap = getTextValue( n );
	    } else if ( tagName == QString("tooltip") ) {
		tooltip = getTextValue( n );
	    }
	    n = n.nextSibling();
	}

	if ( childName == QString("Toolbar:button") ) {
	    QString actionName;
	    GladeAction a;
	    a.menuText = label;
	    a.text = label;
	    a.accel = 0;
	    a.iconSet = icon;

	    if ( gnomeStockPixmapX.exactMatch(stockPixmap) ) {
		QString x = gnomeStockPixmapX.cap( 1 );
		actionName = yyStockItemActions[x];
	    }
	    if ( actionName.isEmpty() ) {
		if ( name.isEmpty() )
		    actionName = QString( "action%1" ).arg( uniqueAction++ );
		else
		    actionName = QString( "action_%1" ).arg( name );
		yyActions.insert( actionName, a );
	    }
	    if ( !tooltip.isEmpty() )
		yyActions[actionName].toolTip = tooltip;

	    emitAtom( QString("action"), attribute(QString("name"),
						   actionName) );
	} else {
	    emitAtom( QString("separator") );
	}
	++c;
    }
    emitClosing( QString("toolbar") );
}

void Glade2Ui::emitPushButton( const QString& text, const QString& name )
{
    emitOpening( QString("widget"),
		 attribute(QString("class"), QString("QPushButton")) );
    emitProperty( QString("name"), name.latin1() );
    emitProperty( QString("text"), text );
    if ( name.contains(QString("ok")) > 0 ) {
	emitProperty( QString("default"), QVariant(TRUE, 0) );
    } else if ( name.contains(QString("help")) > 0 ) {
	emitProperty( QString("accel"), (int) Qt::Key_F1 );
    }
    emitClosing( QString("widget") );
}

void Glade2Ui::attach( AttributeMap *attr, int leftAttach, int rightAttach,
		       int topAttach, int bottomAttach )
{
    if ( leftAttach >= 0 ) {
	attr->insert( QString("row"), QString::number(topAttach) );
	attr->insert( QString("column"), QString::number(leftAttach) );
	if ( bottomAttach - topAttach != 1 )
	    attr->insert( QString("rowspan"),
			 QString::number(bottomAttach - topAttach) );
	if ( rightAttach - leftAttach != 1 )
	    attr->insert( QString("colspan"),
			 QString::number(rightAttach - leftAttach) );
    }
}

void Glade2Ui::emitSpacer( const QString& orientation, int leftAttach,
			   int rightAttach, int topAttach, int bottomAttach )
{
    AttributeMap attr;
    attach( &attr, leftAttach, rightAttach, topAttach, bottomAttach );
    emitOpening( QString("spacer"), attr );
    emitProperty( QString("name"),
		  QString("Spacer%1").arg(uniqueSpacer++).latin1() );
    emitProperty( QString("orientation"), orientation, QString("enum") );
    emitProperty( QString("sizeType"), QString("Expanding"),
		  QString("enum") );
    emitClosing( QString("spacer") );
}

void Glade2Ui::emitPixmap( const QString& imageName, int leftAttach,
			   int rightAttach, int topAttach, int bottomAttach )
{
    emitOpeningWidget( QString("QLabel"), leftAttach, rightAttach, topAttach,
		       bottomAttach );
    emitProperty( QString("sizePolicy"),
		  QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );
    emitProperty( QString("pixmap"), imageName, QString("pixmap") );
    emitClosing( QString("widget") );
}

void Glade2Ui::emitGnomeAbout( QString copyright, QString authors,
			       QString comments )
{
    QString prog = yyProgramName;
    if ( prog.isEmpty() )
	prog = QString( "Gnomovision 1.69" );
    if ( copyright.isEmpty() )
	copyright = QString( "(C) 2001 Jasmin Blanchette" );
    if ( authors.isEmpty() )
	authors = QString( "Jasmin Blanchette <jasmin@troll.no>" );
    if ( comments.isEmpty() )
	comments = QString( "Gnomovision is the official GNU application." );

    emitOpening( QString("hbox") );
    emitSpacer( QString("Horizontal") );
    emitOpeningWidget( QString("QLayoutWidget") );
    emitOpening( QString("vbox") );
    emitProperty( QString("spacing"), 17 );

    /*
      Emit the application name.
    */
    emitOpeningWidget( QString("QLabel") );
    emitFontProperty( QString("font"), 24, TRUE );
    emitProperty( QString("text"), prog );
    emitProperty( QString("alignment"), QString("AlignAuto|AlignCenter"),
		  QString("set") );
    emitClosing( QString("widget") );

    /*
      Emit the copyright notice.
    */
    emitOpeningWidget( QString("QLabel") );
    emitFontProperty( QString("font"), 12, TRUE );
    emitProperty( QString("text"), copyright );
    emitClosing( QString("widget") );

    /*
      Emit the authors' names.
    */
    emitOpeningWidget( QString("QLayoutWidget") );
    emitOpening( QString("hbox") );

    emitOpeningWidget( QString("QLabel") );
    emitFontProperty( QString("font"), 12, TRUE );
    emitProperty( QString("text"), QString("Authors:") );
    emitProperty( QString("alignment"), QString("AlignAuto|AlignTop"),
		  QString("set") );
    emitClosing( QString("widget") );

    emitOpeningWidget( QString("QLabel") );
    emitFontProperty( QString("font"), 12, FALSE );
    emitProperty( QString("text"), authors );
    emitProperty( QString("alignment"), QString("AlignAuto|AlignTop"),
		  QString("set") );
    emitClosing( QString("widget") );

    emitSpacer( QString("Horizontal") );

    emitClosing( QString("hbox") );
    emitClosing( QString("widget") );

    /*
      Emit the comments.
    */
    emitOpeningWidget( QString("QLabel") );
    emitFontProperty( QString("font"), 10, FALSE );
    emitProperty( QString("text"), comments );
    emitProperty( QString("alignment"), QString("AlignAuto|AlignTop"),
		  QString("set") );
    emitClosing( QString("widget") );

    /*
      Emit the spacer and the OK button.
    */
    emitSpacer( QString("Vertical") );

    emitOpeningWidget( QString("QLayoutWidget") );
    emitOpening( QString("hbox") );
    emitSpacer( QString("Horizontal") );

    emitPushButton( QString("&OK"), QString("okButton") );

    emitSpacer( QString("Horizontal") );
    emitClosing( QString("hbox") );
    emitClosing( QString("widget") );

    emitClosing( QString("vbox") );
    emitClosing( QString("widget") );
    emitSpacer( QString("Horizontal") );
    emitClosing( QString("hbox") );
}

/*
  GnomeApps are emitted in two passes, because some information goes
  into the <widget> section, whereas other information goes into the
  <toolbars> section.
*/
void Glade2Ui::emitGnomeAppChildWidgetsPass1(
	const QValueList<QDomElement>& childWidgets )
{
    QValueList<QDomElement>::ConstIterator c = childWidgets.begin();
    while ( c != childWidgets.end() ) {
	QValueList<QDomElement> grandchildWidgets;
	QString childName;

	QDomNode n = (*c).firstChild();
	while ( !n.isNull() ) {
	    QString tagName = n.toElement().tagName();
	    if ( tagName == QString("child_name") ) {
		childName = getTextValue( n );
	    } else if ( tagName == QString("widget") ) {
		grandchildWidgets.push_back( n.toElement() );
	    }
	    n = n.nextSibling();
	}

	if ( childName == QString("GnomeDock:contents") ) {
	    emitWidget( *c, FALSE );
	} else {
	    emitGnomeAppChildWidgetsPass1( grandchildWidgets );
	}
	++c;
    }
}

void Glade2Ui::doPass2( const QValueList<QDomElement>& childWidgets,
			QValueList<QDomElement> *menuBar,
			QValueList<QValueList<QDomElement> > *toolBars )
{
    QValueList<QDomElement>::ConstIterator c = childWidgets.begin();
    while ( c != childWidgets.end() ) {
	QValueList<QDomElement> grandchildWidgets;
	QString childName;
	QString gtkClass;

	QDomNode n = (*c).firstChild();
	while ( !n.isNull() ) {
	    QString tagName = n.toElement().tagName();
	    if ( tagName == QString("child_name") ) {
		childName = getTextValue( n );
	    } else if ( tagName == QString("class") ) {
		gtkClass = getTextValue( n );
	    } else if ( tagName == QString("widget") ) {
		grandchildWidgets.push_back( n.toElement() );
	    }
	    n = n.nextSibling();
	}

	if ( gtkClass == QString("GtkMenuBar") ) {
	    *menuBar = grandchildWidgets;
	} else if ( gtkClass == QString("GtkToolbar") ) {
	    toolBars->push_back( grandchildWidgets );
	} else if ( childName != QString("GnomeDock:contents") ) {
	    doPass2( grandchildWidgets, menuBar, toolBars );
	}
	++c;
    }
}

void Glade2Ui::emitGnomeAppChildWidgetsPass2(
	const QValueList<QDomElement>& childWidgets )
{
    QValueList<QDomElement> menuBar;
    QValueList<QValueList<QDomElement> > toolBars;

    doPass2( childWidgets, &menuBar, &toolBars );

    emitGtkMenuBarChildWidgets( menuBar );
    if ( !toolBars.isEmpty() ) {
	emitOpening( QString("toolbars") );
	while ( !toolBars.isEmpty() ) {
	    emitGtkToolbarChildWidgets( toolBars.first() );
	    toolBars.remove( toolBars.begin() );
	}
	emitClosing( QString("toolbars") );
    }
}

void Glade2Ui::emitGtkButtonChildWidgets( QValueList<QDomElement> childWidgets )
{
    QValueList<QDomElement>::ConstIterator c = childWidgets.begin();
    while ( c != childWidgets.end() ) {
	QString label;

	QDomNode n = (*c).firstChild();
	while ( !n.isNull() ) {
	    QString tagName = n.toElement().tagName();
	    if ( tagName == QString("label") ) {
		label = getTextValue( n );
	    } else if ( tagName == QString("widget") ) {
		childWidgets.push_back( n.toElement() );
	    }
	    n = n.nextSibling();
	}

	if ( !label.isEmpty() ) {
	    emitProperty( QString("text"), accelerate(label) );
	    break;
	}
	++c;
    }
}

void Glade2Ui::emitGtkComboChildWidgets(
	const QValueList<QDomElement>& childWidgets, const QStringList& items )
{
    QString text;

    // there should be exactly one child, of type GtkEntry
    if ( childWidgets.count() == 1 ) {
	QDomNode n = childWidgets.first().firstChild();
	while ( !n.isNull() ) {
	    QString tagName = n.toElement().tagName();
	    if ( tagName == QString("name") ) {
		// grep 'elsewhere'
		emitProperty( QString("name"),
			      fixedName(getTextValue(n).latin1()) );
	    } else if ( tagName == QString("text") ) {
		text = getTextValue( n );
	    }
	    n = n.nextSibling();
	}
    }

    int n = 0;
    QStringList::ConstIterator s = items.begin();
    while ( s != items.end() ) {
	if ( !text.isEmpty() && *s == text )
	    emitProperty( QString("currentItem"), n );
	n++;
	++s;
    }

}

void Glade2Ui::emitGtkNotebookChildWidgets(
	const QValueList<QDomElement>& childWidgets )
{
    QStringList tabNames;
    QStringList tabLabels;

    for ( int i = 0; i < (int) childWidgets.count(); i++ ) {
	tabNames.push_back( QString("tab%1").arg(i + 1) );
	tabLabels.push_back( QString("Tab %1").arg(i + 1) );
    }

    QValueList<QDomElement>::ConstIterator c;
    c = childWidgets.begin();

    QStringList::Iterator nam = tabNames.begin();
    QStringList::Iterator lab = tabLabels.begin();

    while ( c != childWidgets.end() ) {
	QString childName;
	QString name;
	QString label;

	QDomNode n = (*c).firstChild();
	while ( !n.isNull() ) {
	    QString tagName = n.toElement().tagName();
	    if ( tagName == QString("child_name") ) {
		childName = getTextValue( n );
	    } else if ( tagName == QString("name") ) {
		name = getTextValue( n );
	    } else if ( tagName == QString("label") ) {
		label = getTextValue( n );
	    }
	    n = n.nextSibling();
	}

	if ( childName == QString("Notebook:tab") ) {
	    if ( !name.isEmpty() )
		*nam = name;
	    if ( !label.isEmpty() )
		*lab = label;
	    ++nam;
	    ++lab;
	}
	++c;
    }

    c = childWidgets.begin();
    while ( c != childWidgets.end() ) {
	QString childName;
	QString name;
	QString label;

	QDomNode n = (*c).firstChild();
	while ( !n.isNull() ) {
	    QString tagName = n.toElement().tagName();
	    if ( tagName == QString("child_name") )
		childName = getTextValue( n );
	    n = n.nextSibling();
	}

	if ( childName != QString("Notebook:tab") ) {
	    emitOpeningWidget( QString("QWidget") );
	    emitProperty( QString("name"), tabNames.first().latin1() );
	    tabNames.remove( tabNames.begin() );
	    emitAttribute( QString("title"), accelerate(tabLabels.first()) );
	    tabLabels.remove( tabLabels.begin() );
	    emitWidget( *c, FALSE );
	    emitClosing( QString("widget") );
	}
	++c;
    }
}

void Glade2Ui::emitQListViewColumns( const QDomElement& qlistview )
{
    QDomNode n = qlistview.firstChild();
    while ( !n.isNull() ) {
	QString tagName = n.toElement().tagName();
	if ( tagName == QString("widget") ) {
	    QDomNode child = n.firstChild();
	    while ( !child.isNull() ) {
		QString tagName = child.toElement().tagName();
		if ( tagName == QString("label") ) {
		    emitOpening( QString("column") );
		    emitProperty( QString("text"),
				  decelerate(getTextValue(child)) );
		    emitClosing( QString("column") );
		}
		child = child.nextSibling();
	    }
	} else if ( tagName == QString("class") ) {
	    QString gtkClass = getTextValue( n );
	    if ( gtkClass.endsWith(QString("Tree")) )
		emitProperty( QString("rootIsDecorated"), QVariant(TRUE, 0) );
	} else if ( tagName == QString("selection_mode") ) {
	    emitProperty( QString("selectionMode"),
			  gtk2qtSelectionMode(getTextValue(n)) );
	}
	n = n.nextSibling();
    }
}

void Glade2Ui::emitGtkScrolledWindowChildWidgets(
	const QValueList<QDomElement>& childWidgets, const QString& qtClass )
{
    if ( childWidgets.count() == 1 ) {
	if ( qtClass == QString("QIconView") ||
	     qtClass == QString("QListBox") ||
	     qtClass == QString("QListView") ) {
	    QDomNode n = childWidgets.first().firstChild();
	    while ( !n.isNull() ) {
		QString tagName = n.toElement().tagName();
		if ( tagName == QString("selection_mode") ) {
		    emitProperty( QString("selectionMode"),
				  gtk2qtSelectionMode(getTextValue(n)) );
		}
		n = n.nextSibling();
	    }
	}

	if ( qtClass == QString("QListView") ) {
	    emitQListViewColumns( childWidgets.first() );
	} else if ( qtClass == QString("QTextEdit") ||
		    qtClass == QString("QTextView") ) {
	    QDomNode n = childWidgets.first().firstChild();
	    while ( !n.isNull() ) {
		QString tagName = n.toElement().tagName();
		if ( tagName == QString("text") )
		    emitProperty( QString("text"), getTextValue(n) );
		n = n.nextSibling();
	    }
	}
    }
}

void Glade2Ui::emitGnomeDruidPage( const QDomElement& druidPage )
{
    QValueList<QDomElement> childWidgets;
    QString gtkClass;
    QString logoImage;
    QString name;
    QString text;
    QString title;
    QString watermarkImage;

    emitOpeningWidget( QString("QWidget") );

    QDomNode n = druidPage.firstChild();
    while ( !n.isNull() ) {
	QString tagName = n.toElement().tagName();
	if ( tagName == QString("class") ) {
	    gtkClass = getTextValue( n );
	} else if ( tagName == QString("logo_image") ) {
	    logoImage = getTextValue( n );
	} else if ( tagName == QString("name") ) {
	    name = getTextValue( n );
	} else if ( tagName == QString("text") ) {
	    text = getTextValue( n );
	} else if ( tagName == QString("title") ) {
	    title = getTextValue( n );
	} else if ( tagName == QString("watermark_image") ) {
	    watermarkImage = getTextValue( n );
	} else if ( tagName == QString("widget") ) {
	    childWidgets.push_back( n.toElement() );
	}
	n = n.nextSibling();
    }

    if ( !name.isEmpty() )
	emitProperty( QString("name"), fixedName(name).latin1() );
    if ( title.isEmpty() )
	title = QString( "Page" );
    emitAttribute( QString("title"), title );

    /*
      We're striving to get the logoImage and/or the watermarkImage at
      the right places with a grid layout.
    */
    int leftAttach = 0;
    int rightAttach = 0;
    int topAttach = 0;
    int bottomAttach = 0;

    int numImages = 0;
    if ( !logoImage.isEmpty() ) {
	topAttach = 1;
	numImages++;
    }
    if ( !watermarkImage.isEmpty() ) {
	leftAttach = 1;
	numImages++;
    }
    rightAttach = leftAttach + numImages;
    bottomAttach = topAttach + numImages;

    bool layouted = ( numImages > 0 );
    if ( layouted ) {
	emitOpening( QString("grid") );
	if ( !logoImage.isEmpty() )
	    emitPixmap( imageName(logoImage), numImages, numImages + 1, 0, 1 );
	if ( !watermarkImage.isEmpty() )
	    emitPixmap( imageName(watermarkImage), 0, 1, numImages,
			numImages + 1 );
    } else {
	leftAttach = -1;
	rightAttach = -1;
	topAttach = -1;
	bottomAttach = -1;
    }

    if ( gtkClass.endsWith(QString("Standard")) ) {
	emitChildWidgets( childWidgets, layouted, leftAttach, rightAttach,
			  topAttach, bottomAttach );
    } else if ( !text.isEmpty() ) {
	if ( layouted )
	    emitOpeningWidget( QString("QLayoutWidget"), leftAttach,
			       rightAttach, topAttach, bottomAttach );
	emitOpening( QString("hbox") );
	emitSpacer( QString("Horizontal") );
	emitOpeningWidget( QString("QLabel") );
	emitProperty( QString("text"), text );
	emitClosing( QString("widget") );
	emitSpacer( QString("Horizontal") );
	emitClosing( QString("hbox") );
	if ( layouted )
	    emitClosing( QString("widget") );
    }

    if ( layouted )
	emitClosing( QString("grid") );
    emitClosing( QString("widget") );
}

void Glade2Ui::emitGtkWindowChildWidgets(
	const QValueList<QDomElement>& childWidgets, const QString& qtClass )
{
    if ( childWidgets.count() == 1 && qtClass == QString("QWizard") ) {
	emitFontProperty( QString("titleFont"), 18, FALSE );

	QDomNode n = childWidgets.first().firstChild();
	while ( !n.isNull() ) {
	    if ( n.toElement().tagName() == QString("widget") )
		emitGnomeDruidPage( n.toElement() );
	    n = n.nextSibling();
	}
    } else {
	emitChildWidgets( childWidgets, FALSE );
    }
}

bool Glade2Ui::packEnd( const QDomElement& widget )
{
    QDomNode n = widget.firstChild();
    while ( !n.isNull() ) {
	if ( n.toElement().tagName() == QString("child") ) {
	    QDomNode child = n.firstChild();
	    while ( !child.isNull() ) {
		if ( child.toElement().tagName() == QString("pack") ) {
		    QString pack = getTextValue( child );
		    return pack.endsWith( QString("_END") );
		}
		child = child.nextSibling();
	    }
	}
	n = n.nextSibling();
    }
    return FALSE;
}

void Glade2Ui::emitChildWidgets( const QValueList<QDomElement>& childWidgets,
				 bool layouted, int leftAttach, int rightAttach,
				 int topAttach, int bottomAttach )
{
    QValueList<QDomElement> start;
    QValueList<QDomElement> end;
    QValueList<QDomElement>::ConstIterator e;

    if ( layouted ) {
	e = childWidgets.begin();
	while ( e != childWidgets.end() ) {
	    if ( packEnd(*e) )
		end.push_front( *e );
	    else
		start.push_back( *e );
	    ++e;
	}
    } else {
	start = childWidgets;
    }

    e = start.begin();
    while ( e != start.end() ) {
	emitWidget( *e, layouted, leftAttach, rightAttach, topAttach,
		    bottomAttach );
	++e;
    }
    e = end.begin();
    while ( e != end.end() ) {
	emitWidget( *e, layouted, leftAttach, rightAttach, topAttach,
		    bottomAttach );
	++e;
    }
}

void Glade2Ui::emitOpeningWidget( const QString& qtClass, int leftAttach,
				  int rightAttach, int topAttach,
				  int bottomAttach )
{
    AttributeMap attr = attribute( QString("class"), qtClass );
    attach( &attr, leftAttach, rightAttach, topAttach, bottomAttach );
    emitOpening( QString("widget"), attr );
}

/*
  Returns TRUE if the vbox containing childWidgets should have a
  spacer at the end to prevent it from looking bad, otherwise returns
  FALSE.

  The algorithm is very experimental.
*/
bool Glade2Ui::shouldPullup( const QValueList<QDomElement>& childWidgets )
{
    QRegExp gtkSmallWidget( QString(
	    "G.*(?:Button|Combo|Dial|Entry|Label|OptionMenu|Picker|ProgressBar"
	    "|Separator|Statusbar|Toolbar|VBox)") );

    QValueList<QDomElement>::ConstIterator c = childWidgets.begin();
    while ( c != childWidgets.end() ) {
	QValueList<QDomElement> grandchildWidgets;
	QString gtkClass;

	QDomNode n = (*c).firstChild();
	while ( !n.isNull() ) {
	    QString tagName = n.toElement().tagName();
	    if ( tagName == QString("class") ) {
		gtkClass = getTextValue( n );
	    } else if ( tagName == QString("widget") ) {
		grandchildWidgets.push_back( n.toElement() );
	    }
	    n = n.nextSibling();
	}

	if ( !gtkSmallWidget.exactMatch(gtkClass) ||
	     !shouldPullup(grandchildWidgets) )
	    return FALSE;
	++c;
    }
    return TRUE;
}

QString Glade2Ui::emitWidget( const QDomElement& widget, bool layouted,
			      int leftAttach, int rightAttach, int topAttach,
			      int bottomAttach )
{
    QRegExp gtkLayoutWidget( QString(
	    "Gtk(?:Packer|Table|Toolbar|[HV](?:(?:Button)?Box))") );
    QRegExp gtkOrientedWidget( QString(
	    "Gtk([HV])(?:Paned|Scale|Scrollbar|Separator)") );

    QValueList<QDomElement> childWidgets;
    QString gtkClass;
    QString name;
    QString title;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    int numRows = 0;
    int numColumns = 0;

    bool active = FALSE;
    QString authors;
    QString childName;
    QString comments;
    QString copyright;
    QString creationFunction;
    bool editable = TRUE;
    QString filename;
    QString focusTarget;
    QString hscrollbarPolicy;
    QString icon;
    int initialChoice = 0;
    QStringList items;
    QString justify;
    QString label;
    QString logoImage;
    int lower = -123456789;
    int page = 10;
    int pageSize = 10;
    QString selectionMode;
    QString shadowType( "GTK_SHADOW_NONE" );
    bool showText = TRUE;
    bool showTitles = TRUE;
    int step = 1;
    QString tabPos;
    QString text;
    int textMaxLength = 0;
    bool textVisible = TRUE;
    QString tooltip;
    QString type;
    int upper = 123456789;
    int value = 123456789;
    bool valueInList = TRUE;
    QString vscrollbarPolicy;
    QString watermarkImage;
    bool wrap = FALSE;

    bool topLevel = yyFormName.isEmpty();
    if ( topLevel )
	name = QString( "Form%1" ).arg( uniqueForm++ );

    QDomNode n = widget.firstChild();
    while ( !n.isNull() ) {
	QString tagName = n.toElement().tagName();
	if ( !tagName.isEmpty() ) {
	    /*
	      Recognize the properties and stores them in variables.
	      This step is a bit silly, and if this function were to
	      be rewritten, almost everything would just be stored, as
	      strings, in a giant map, and looked up for when emitting
	      the properties.
	    */
	    switch ( (uchar) tagName[0].cell() ) {
	    case 'a':
		if ( tagName == QString("active") ) {
		    active = isTrue( getTextValue(n) );
		} else if ( tagName == QString("authors") ) {
		    authors = getTextValue( n );
		}
		break;
	    case 'c':
		if ( tagName == QString("child") ) {
		    QDomNode child = n.firstChild();
		    while ( !child.isNull() ) {
			QString childTagName = child.toElement().tagName();
			if ( childTagName == QString("left_attach") ) {
			    leftAttach = getTextValue( child ).toInt();
			} else if ( childTagName == QString("right_attach") ) {
			    rightAttach = getTextValue( child ).toInt();
			} else if ( childTagName == QString("top_attach") ) {
			    topAttach = getTextValue( child ).toInt();
			} else if ( childTagName == QString("bottom_attach") ) {
			    bottomAttach = getTextValue( child ).toInt();
			}
			child = child.nextSibling();
		    }
		} else if ( tagName == QString("child_name") ) {
		    childName = getTextValue( n );
		} else if ( tagName == QString("class") ) {
		    gtkClass = getTextValue( n );
		} else if ( tagName == QString("climb_rate") ) {
		    step = getTextValue( n ).toInt();
		} else if ( tagName == QString("columns") ) {
		    numColumns = getTextValue( n ).toInt();
		} else if ( tagName == QString("comments") ) {
		    comments = getTextValue( n );
		} else if ( tagName == QString("copyright") ) {
		    copyright = getTextValue( n );
		} else if ( tagName == QString("creation_function") ) {
		    creationFunction = getTextValue( n );
		}
		break;
	    case 'd':
		if ( tagName == QString("default_focus_target") ) {
		    if ( focusTarget.isEmpty() )
			focusTarget = getTextValue( n );
		}
		break;
	    case 'e':
		if ( tagName == QString("editable") )
		    editable = isTrue( getTextValue(n) );
		break;
	    case 'f':
		if ( tagName == QString("filename") ) {
		    filename = getTextValue( n );
		} else if ( tagName == QString("focus_target") ) {
		    focusTarget = getTextValue( n );
		}
		break;
	    case 'h':
		if ( tagName == QString("height") ) {
		    height = getTextValue( n ).toInt();
		} else if ( tagName == QString("hscrollbar_policy") ) {
		    hscrollbarPolicy = getTextValue( n );
		}
		break;
	    case 'i':
		if ( tagName == QString("icon") ) {
		    icon = getTextValue( n );
		} else if ( tagName == QString("initial_choice") ) {
		    initialChoice = getTextValue( n ).toInt();
		} else if ( tagName == QString("items") ) {
		    items = QStringList::split( QChar('\n'), getTextValue(n) );
		}
		break;
	    case 'j':
		if ( tagName == QString("justify") )
		    justify = getTextValue( n );
		break;
	    case 'l':
		if ( tagName == QString("label") ) {
		    label = getTextValue( n );
		} else if ( tagName == QString("logo_image") ) {
		    logoImage = getTextValue( n );
		} else if ( tagName == QString("lower") ) {
		    lower = getTextValue( n ).toInt();
		}
		break;
	    case 'n':
		if ( tagName == QString("name") )
		    name = getTextValue( n );
		break;
	    case 'p':
		if ( tagName == QString("page") ) {
		    page = getTextValue( n ).toInt();
		} else if ( tagName == QString("page_size") ) {
		    pageSize = getTextValue( n ).toInt();
		}
		break;
	    case 'r':
		if ( tagName == QString("rows") )
		    numRows = getTextValue( n ).toInt();
		break;
	    case 's':
		if ( tagName == QString("selection_mode") ) {
		    selectionMode = getTextValue( n );
		} else if ( tagName == QString("shadow_type") ) {
		    shadowType = getTextValue( n );
		} else if ( tagName == QString("show_text") ) {
		    showText = isTrue( getTextValue(n) );
		} else if ( tagName == QString(QString("show_titles")) ) {
		    showTitles = isTrue( getTextValue(n) );
		} else if ( tagName == QString("step") ) {
		    step = getTextValue( n ).toInt();
		} else if ( tagName == QString("stock_button") ) {
		    /*
		      Let's cheat: We convert the symbolic name into a
		      button label.
		    */
		    label = getTextValue( n );
		    int k = label.findRev( QChar('_') );
		    if ( k != -1 )
			label = label.mid( k + 1 );
		    if ( !label.isEmpty() && label != QString("OK") )
			label = label.left( 1 ) + label.mid( 1 ).lower();
		}
		break;
	    case 't':
		if ( tagName == QString("tab_pos") ) {
		    tabPos = getTextValue( n );
		} else if ( tagName == QString("text") ) {
		    text = getTextValue( n );
		} else if ( tagName == QString("textMaxLength") ) {
		    textMaxLength = getTextValue( n ).toInt();
		} else if ( tagName == QString("textVisible") ) {
		    textVisible = isTrue( getTextValue(n) );
		} else if ( tagName == QString("title") ) {
		    title = getTextValue( n );
		} else if ( tagName == QString("tooltip") ) {
		    tooltip = getTextValue( n );
		} else if ( tagName == QString("type") ) {
		    type = getTextValue( n );
		}
		break;
	    case 'u':
		if ( tagName == QString("upper") )
		    upper = getTextValue( n ).toInt();
		break;
	    case 'v':
		if ( tagName == QString("value") ) {
		    value = getTextValue( n ).toInt();
		} else if ( tagName == QString("value_in_list") ) {
		    valueInList = isTrue( getTextValue(n) );
		} else if ( tagName == QString("vscrollbar_policy") ) {
		    vscrollbarPolicy = getTextValue( n );
		}
		break;
	    case 'w':
		if ( tagName == QString("watermark_image") ) {
		    watermarkImage = getTextValue( n );
		} else if ( tagName == QString("widget") )
		    childWidgets.push_back( n.toElement() );
		else if ( tagName == QString("width") )
		    width = getTextValue( n ).toInt();
		else if ( tagName == QString("wrap") )
		    wrap = isTrue( getTextValue(n) );
		break;
	    case 'x':
		if ( tagName == QString("x") )
		    x = getTextValue( n ).toInt();
		break;
	    case 'y':
		if ( tagName == QString("y") )
		    y = getTextValue( n ).toInt();
	    }
	}
	n = n.nextSibling();
    }

    if ( topLevel ) {
	yyFormName = name;
	emitSimpleValue( QString("class"), yyFormName );
    }

    if ( gtkLayoutWidget.exactMatch(gtkClass) ) {
	QString boxKind;
	QString orientation;

	if ( gtkClass.startsWith(QString("GtkH")) ||
	     gtkClass == QString(QString("GtkToolbar")) ) {
	    /*
	      GtkToolbar, right. Toolbars may appear anywhere in a
	      widget, but then they really are just a row of buttons.
	    */
	    boxKind = QString( "hbox" );
	    orientation = QString( "Horizontal" );
	} else if ( gtkClass.startsWith(QString("GtkV")) ||
		    gtkClass == QString("GtkPacker") ) {
	    /*
	      We don't support the GtkPacker, whose trails lead to
	      destruction.
	    */
	    boxKind = QString( "vbox" );
	    orientation = QString( "Vertical" );
	} else {
	    boxKind = QString( "grid" );
	}

	if ( layouted )
	    emitOpeningWidget( QString("QLayoutWidget"), leftAttach,
			       rightAttach, topAttach, bottomAttach );
	emitOpening( boxKind );
	emitProperty( QString("name"), fixedName(name).latin1() );
	if ( gtkClass == QString("GtkHButtonBox") ||
	     childName == QString("Dialog:action_area") )
	    emitSpacer( orientation );
	emitChildWidgets( childWidgets, TRUE );

	if ( gtkClass == QString("GtkVButtonBox") ||
	     childName == QString("Dialog:action_area") ||
	     (boxKind == QString("vbox") && shouldPullup(childWidgets)) ) {
	    emitSpacer( orientation );
	} else if ( boxKind == QString("grid") && shouldPullup(childWidgets) ) {
	    emitSpacer( QString("Vertical"), 0, numColumns, numRows,
			numRows + 1 );
	}

	emitClosing( boxKind );
	if ( layouted )
	    emitClosing( QString("widget") );
    } else if ( gtkClass == QString("GtkFixed") && !layouted ) {
	emitChildWidgets( childWidgets, FALSE );
    /*
      Placeholders in a grid are typically needless.
    */
    } else if ( !(leftAttach >= 0 && gtkClass == QString("Placeholder")) ) {
	bool needFakeLayout = ( !layouted && !topLevel && x == 0 && y == 0 &&
				width == 0 && height == 0 );
	QString qtClass = gtk2qtClass( gtkClass, childWidgets );
	bool unknown = FALSE;

	if ( qtClass == QString("QFrame") && !label.isEmpty() ) {
	    qtClass = QString( "QButtonGroup" );
	} else if ( qtClass == QString("QListView") && !showTitles &&
		    gtkClass.endsWith(QString("List")) ) {
	    qtClass = QString( "QListBox" );
	} else if ( qtClass == QString("Custom") ) {
	    qtClass = creationFunction;
	    yyCustomWidgets.insert( qtClass, QString::null );
	} else if ( qtClass == QString("Unknown") ) {
	    qtClass = QString( "QLabel" );
	    unknown = TRUE;
	}

	if ( qtClass.isEmpty() ) {
	    emitChildWidgets( childWidgets, layouted, leftAttach, rightAttach,
			      topAttach, bottomAttach );
	} else {
	    if ( needFakeLayout ) {
		emitOpening( QString("vbox") );
		emitProperty( QString("margin"), 11 );
	    }
	    emitOpeningWidget( qtClass, leftAttach, rightAttach, topAttach,
			       bottomAttach );

	    // grep 'elsewhere'
	    if ( gtkClass != QString("GtkCombo") )
		emitProperty( QString("name"), fixedName(name).latin1() );
	    if ( !title.isEmpty() )
		emitProperty( QString("caption"), title );
	    if ( !layouted && (x != 0 || y != 0 || width != 0 || height != 0) )
		emitProperty( QString("geometry"), QRect(x, y, width, height) );
	    if ( gtkClass == QString("GtkToggleButton") ) {
		emitProperty( QString("toggleButton"), QVariant(TRUE, 0) );
		if ( active )
		    emitProperty( QString("on"), QVariant(TRUE, 0) );
	    } else {
		if ( active )
		    emitProperty( QString("checked"), QVariant(TRUE, 0) );
	    }

	    if ( !editable )
		emitProperty( QString("readOnly"), QVariant(TRUE, 0) );
	    if ( !focusTarget.isEmpty() )
		emitProperty( QString("buddy"),
			      fixedName(focusTarget).latin1() );
	    if ( !hscrollbarPolicy.isEmpty() )
		emitProperty( QString("hScrollBarMode"),
			      gtk2qtScrollBarMode(hscrollbarPolicy),
			      QString("enum") );
	    if ( !vscrollbarPolicy.isEmpty() )
		emitProperty( QString("vScrollBarMode"),
			      gtk2qtScrollBarMode(vscrollbarPolicy),
			      QString("enum") );
	    if ( !justify.isEmpty() ||
		 (wrap && gtkClass != QString("GtkSpinButton")) ) {
		QStringList flags;
		if ( wrap )
		    flags.push_back( QString("WordBreak") );

		if ( justify.endsWith(QString("_CENTER")) ) {
		    flags.push_back( QString("AlignCenter") );
		} else {
		    if ( justify.endsWith(QString("_RIGHT")) )
			flags.push_back( QString("AlignRight") );
		    flags.push_back( QString("AlignVCenter") );
		}

		if ( !flags.isEmpty() )
		    emitProperty( QString("alignment"), flags.join(QChar('|')),
				  QString("set") );
	    }
	    if ( !label.isEmpty() ) {
		if ( gtkClass.endsWith(QString("Frame")) ) {
		    emitProperty( QString("title"), label );
		} else {
		    emitProperty( QString("text"), accelerate(label) );
		}
	    }
	    if ( lower != -123456789 )
		emitProperty( QString("minValue"), lower );
	    if ( upper != 123456789 )
		emitProperty( QString("maxValue"), upper );
	    if ( value != 123456789 ) {
		if ( gtkClass == QString("GtkProgressBar") ) {
		    emitProperty( QString("progress"), value );
		} else {
		    emitProperty( QString("value"), value );
		}
	    }
	    if ( QMAX(page, pageSize) != 10 &&
		 gtkClass.endsWith(QString("Scrollbar")) )
		emitProperty( QString("pageStep"), QMAX(page, pageSize) );
	    if ( !selectionMode.isEmpty() )
		emitProperty( QString("selectionMode"),
			      gtk2qtSelectionMode(selectionMode),
			      QString("enum") );
	    if ( !shadowType.endsWith(QString("_NONE")) ) {
		QString shape = shadowType.contains( QString("_ETCHED_") ) > 0 ?
				QString( "Box" ) : QString( "WinPanel" );
		QString shadow = shadowType.endsWith( QString("_IN") ) ?
				 QString( "Sunken" ) : QString( "Raised" );
		emitProperty( QString("frameShape"), shape, QString("enum") );
		emitProperty( QString("frameShadow"), shadow, QString("enum") );
	    }
	    if ( !showText )
		emitProperty( QString("percentageVisible"),
			      QVariant(FALSE, 0) );
	    if ( step != 1 )
		emitProperty( QString("lineStep"), step );
	    if ( tabPos.endsWith(QString("_BOTTOM")) ||
		 tabPos.endsWith(QString("_RIGHT")) )
		emitProperty( QString("tabPosition"), QString("Bottom") );
	    if ( !text.isEmpty() )
		emitProperty( QString("text"), text );
	    if ( textMaxLength != 0 )
		emitProperty( QString("maxLength"), textMaxLength );
	    if ( !textVisible )
		emitProperty( QString("echoMode"), QString("Password"),
			      QString("enum") );
	    if ( !tooltip.isEmpty() )
		emitProperty( QString("toolTip"), tooltip );
	    if ( !valueInList )
		emitProperty( QString("editable"), QVariant(TRUE, 0) );
	    if ( wrap && gtkClass == QString("GtkSpinButton") )
		emitProperty( QString("wrapping"), QVariant(TRUE, 0) );

	    if ( gtkClass.endsWith(QString("Tree")) ) {
		emitProperty( QString("rootIsDecorated"), QVariant(TRUE, 0) );
	    } else if ( gtkOrientedWidget.exactMatch(gtkClass) ) {
		QString s = ( gtkOrientedWidget.cap(1) == QChar('H') ) ?
			    QString( "Horizontal" ) : QString( "Vertical" );
		emitProperty( QString("orientation"), s, QString("enum") );
	    }

	    if ( qtClass == QString("QListView") )
		emitProperty( QString("resizeMode"), QString("AllColumns"),
			      QString("enum") );

	    while ( !items.isEmpty() ) {
		emitOpening( QString("item") );
		emitProperty( QString("text"), items.first() );
		emitClosing( QString("item") );
		items.remove( items.begin() );
	    }
	    if ( initialChoice != 0 )
		emitProperty( QString("currentItem"), initialChoice );

	    if ( !icon.isEmpty() )
		emitProperty( QString("pixmap"), imageName(icon),
			      QString("pixmap") );

	    if ( gtkClass == QString("GnomeAbout") ) {
		emitGnomeAbout( copyright, authors, comments );
	    } else if ( gtkClass == QString("GnomeApp") ) {
		emitGnomeAppChildWidgetsPass1( childWidgets );
	    } else if ( gtkClass == QString("GnomePropertyBox") ) {
		emitOpening( QString("vbox") );
		emitChildWidgets( childWidgets, TRUE );
		emitOpeningWidget( QString("QLayoutWidget") );
		emitOpening( QString("hbox") );
		emitPushButton( QString("&Help"), QString("helpButton") );
		emitSpacer( QString("Horizontal") );
		emitPushButton( QString("&OK"), QString("okButton") );
		emitPushButton( QString("&Apply"), QString("applyButton") );
		emitPushButton( QString("&Close"), QString("closeButton") );
		emitClosing( QString("hbox") );
		emitClosing( QString("widget") );
		emitClosing( QString("vbox") );
	    } else if ( gtkClass.endsWith(QString("Button")) ) {
		if ( label.isEmpty() )
		    emitGtkButtonChildWidgets( childWidgets );
	    } else if ( gtkClass == QString("GtkCombo") ) {
		emitGtkComboChildWidgets( childWidgets, items );
	    } else if ( gtkClass == QString("GtkNotebook") ) {
		emitGtkNotebookChildWidgets( childWidgets );
	    } else if ( gtkClass == QString("GtkWindow") ) {
		emitGtkWindowChildWidgets( childWidgets, qtClass );
	    } else if ( gtkClass == QString("GtkScrolledWindow") ) {
		emitGtkScrolledWindowChildWidgets( childWidgets, qtClass );
	    } else if ( qtClass == QString("QListView") ) {
		emitQListViewColumns( widget );
	    } else if ( unknown || gtkClass == QString("Placeholder") ) {
		QString prefix;
		if ( unknown )
		    prefix = gtkClass;
		emitProperty( QString("text"),
			      QString("<font color=\"red\">%1<b>?</b></font>")
			      .arg(prefix) );
		emitProperty( QString("alignment"),
			      QString("AlignAuto|AlignCenter"),
			      QString("set") );
	    } else if ( qtClass != QString("QListBox") ) {
		emitChildWidgets( childWidgets, FALSE );
	    }

	    emitClosing( QString("widget") );
	    if ( needFakeLayout )
		emitClosing( QString("vbox") );
	    if ( gtkClass == QString("GnomeApp") )
		emitGnomeAppChildWidgetsPass2( childWidgets );
	}
    }
    return name;
}

QStringList Glade2Ui::convertGladeFile( const QString& fileName )
{
    QStringList outFileNames;

    yyFileName = fileName;

    QDomDocument doc( QString("GTK-Interface") );
    QFile f( fileName );
    if ( !f.open(IO_ReadOnly) ) {
	error( QString("Cannot open file for reading.") );
	return QStringList();
    }
    if ( !doc.setContent(&f) ) {
	error( QString("File is not an XML file.") );
	f.close();
	return QStringList();
    }
    f.close();

    QDomElement root = doc.documentElement();
    if ( root.tagName() != QString("GTK-Interface") ) {
	error( QString("File is not a Glade XML file.") );
	return QStringList();
    }

    int widgetNo = 0;
    int numWidgets = 0;
    QDomNode n = root.firstChild();
    while ( !n.isNull() ) {
	if ( n.toElement().tagName() == QString("widget") )
	    numWidgets++;	
	n = n.nextSibling();
    }

    QProgressDialog fremskritt( QString("Converting Glade files..."),
				QString("Abort Conversion"), numWidgets, 0,
				"fremskritt", TRUE );

    n = root.firstChild();
    while ( !n.isNull() ) {
	QString tagName = n.toElement().tagName();

	if ( tagName == QString("project") ) {
	    QDomNode child = n.firstChild();
	    while ( !child.isNull() ) {
		QString childTagName = child.toElement().tagName();
		if ( childTagName == QString("pixmaps_directory") ) {
		    yyPixmapDirectory = getTextValue( child );
		} else if ( childTagName == QString("program_name") ) {
		    yyProgramName = getTextValue( child );
		}
		child = child.nextSibling();
	    }
	} else if ( tagName == QString("widget") ) {
	    yyOut = QString::null;
	    yyCustomWidgets.clear();
	    yyStockItemActions.clear();
	    yyActions.clear();
	    yyConnections.clear();
	    yySlots.clear();
	    yyFormName = QString::null;
	    yyImages.clear();

	    uniqueAction = 1;
	    uniqueForm = 1;
	    uniqueMenuBar = 1;
	    uniqueSpacer = 1;
	    uniqueToolBar = 1;

	    emitHeader();
	    QString name = emitWidget( n.toElement(), FALSE );

	    if ( !yyCustomWidgets.isEmpty() ) {
		emitOpening( QString("customwidgets") );

		QMap<QString, QString>::Iterator c = yyCustomWidgets.begin();
		while ( c != yyCustomWidgets.end() ) {
		    emitOpening( QString("customwidget") );
		    emitSimpleValue( QString("class"), c.key() );
		    if ( !(*c).isEmpty() )
			emitSimpleValue( QString("header"), *c,
					 attribute(QString("location"),
						   QString("local")) );
		    emitClosing( QString("customwidget") );
		    ++c;
		}
		emitClosing( QString("customwidgets") );
	    }

	    if ( !yyActions.isEmpty() ) {
		emitOpening( QString("actions") );

		QMap<QString, GladeAction>::Iterator a = yyActions.begin();
		while ( a != yyActions.end() ) {
		    emitOpening( QString("action") );
		    emitProperty( QString("name"),
				  fixedName(a.key()).latin1() );
		    emitProperty( QString("text"), (*a).text );
		    emitProperty( QString("menuText"), (*a).menuText );
		    if ( (*a).toolTip )
			emitProperty( QString("toolTip"), (*a).toolTip );
		    if ( (*a).accel != 0 )
			emitProperty( QString("accel"), (*a).accel );
		    if ( !(*a).iconSet.isEmpty() )
			emitProperty( QString("iconSet"),
				      imageName((*a).iconSet),
				      QString("iconset") );
		    emitClosing( QString("action") );
		    ++a;
		}
		emitClosing( QString("actions") );
	    }

	    if ( !yyImages.isEmpty() ) {
		uint maxLength = 524288;
		char *data = new char[maxLength];

		QString dir = yyFileName;
		dir.truncate( dir.findRev(QChar('/')) + 1 );
		dir += yyPixmapDirectory;

		emitOpening( QString("images") );

		QMap<QString, QString>::ConstIterator im = yyImages.begin();
		while ( im != yyImages.end() ) {
		    uint length = 0;
		    const char *format = 0;

		    QString fn = dir + QChar( '/' ) + im.key();
		    QFile f( fn );
		    if ( !f.open(IO_ReadOnly) ) {
			error( QString("Cannot open image '%1' for reading.")
			       .arg(fn) );
		    } else {
			length = f.readBlock( data, maxLength );
			f.at( 0 );
			format = QImageIO::imageFormat( &f );
			f.close();
		    }
		    if ( format == 0 )
			format = "XPM";

		    AttributeMap attr;
		    attr.insert( QString("format"), QString(format) );
		    attr.insert( QString("length"), QString::number(length) );

		    emitOpening( QString("image"),
				 attribute(QString("name"), *im) );
		    emitSimpleValue( QString("data"), hexed(data, length),
				     attr );
		    emitClosing( QString("image") );
		    ++im;
		}
		emitClosing( QString("images") );
		delete[] data;
	    }

	    if ( yyConnections.count() + yySlots.count() > 0 ) {
		emitOpening( QString("connections") );

		QValueList<GladeConnection>::Iterator c = yyConnections.begin();
		while ( c != yyConnections.end() ) {
		    emitOpening( QString("connection") );
		    emitSimpleValue( QString("sender"), (*c).sender );
		    emitSimpleValue( QString("signal"), (*c).signal );
		    emitSimpleValue( QString("receiver"), yyFormName );
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
	    emitFooter();

	    QString outFileName = fileName;
	    int k = outFileName.findRev( "." );
	    if ( k != -1 )
		outFileName.truncate( k );
	    if ( widgetNo != 0 )
		outFileName += QString::number( widgetNo + 1 );
	    outFileName += QString( ".ui" );
	    FILE *out = fopen( outFileName.latin1(), "w" );
	    if ( out == 0 ) {
		qWarning( "glade2ui: Could not open file '%s'",
			 outFileName.latin1() );
	    } else {
		if ( !yyOut.isEmpty() )
		    fprintf( out, "%s", yyOut.latin1() );
		fclose( out );
		outFileNames.push_back( outFileName );
	    }
	    yyOut = QString::null;
	    widgetNo++;

	    qApp->processEvents();
	    if ( fremskritt.wasCancelled() )
		break;
	    fremskritt.setProgress( widgetNo );
	}
	n = n.nextSibling();
    }
    return outFileNames;
}
