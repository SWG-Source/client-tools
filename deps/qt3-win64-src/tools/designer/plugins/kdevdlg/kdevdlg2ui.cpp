/**********************************************************************
**
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

#include "kdevdlg2ui.h"
#include <qdir.h>
#include <qstring.h>
#include <qptrstack.h>

/// some little helpers ///

void KDEVDLG2UI::wi()
{
    for ( int i = 0; i < indentation; i++ )
	*out << "    ";
}

void KDEVDLG2UI::indent()
{
    indentation++;
}

void KDEVDLG2UI::undent()
{
    indentation--;
}

void KDEVDLG2UI::writeClass( const QString& name )
{
    wi(); *out << "<class>" << name << "</class>" << endl;
}

void KDEVDLG2UI::writeWidgetStart( const QString& qclass )
{
    wi(); *out << "<widget class=\"" << qclass << "\">" << endl;
}

void KDEVDLG2UI::writeWidgetEnd()
{
    wi(); *out << "</widget>" << endl;
}

void KDEVDLG2UI::writeCString( const QString& name, const QString& value )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<cstring>" << value << "</cstring>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeString( const QString& name, const QString& value )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<string>" << value << "</string>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeRect( const QString& name, int x, int y, int w, int h )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<rect>" << endl; indent();
    wi(); *out << "<x>" << x << "</x>" << endl;
    wi(); *out << "<y>" << y << "</y>" << endl;
    wi(); *out << "<width>" << w << "</width>" << endl;
    wi(); *out << "<height>" << h << "</height>" << endl; undent();
    wi(); *out << "</rect>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeFont( const QString& family, int pointsize )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>font</name>" << endl;
    wi(); *out << "<font>" << endl; indent();
    wi(); *out << "<family>" << family << "</family>" << endl;
    wi(); *out << "<pointsize>" << pointsize << "</pointsize>" << endl; undent();
    wi(); *out << "</font>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeBool( const QString& name, bool value )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<bool>" << (value ? "true" : "false") << "</bool>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeNumber( const QString& name, int value )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<number>" << value << "</number>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeEnum( const QString& name, const QString& value )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<enum>" << value << "</enum>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeSet( const QString& name, const QString& value )
{
    wi(); *out << "<property>" << endl; indent();
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<set>" << value << "</set>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeItem( const QString& name, const QString& value )
{
    wi(); *out << "<item>" << endl; indent();
    writeString( name, value ); undent();
    wi(); *out << "</item>" << endl;
}

void KDEVDLG2UI::writeColumn( const QString& name, const QString& value )
{
    wi(); *out << "<column>" << endl; indent();
    writeString( name, value ); undent();
    wi(); *out << "</column>" << endl;
}

void KDEVDLG2UI::writeColor( const QString& name, const QString& value )
{
    int color = value.toInt();

    int r = color & 0x00ff0000 >> 16;
    int g = color & 0x0000ff00 >> 8;
    int b = color & 0x000000ff;
    
    wi(); *out << "<property>" << endl; indent(); //###FIX
    wi(); *out << "<name>" << name << "</name>" << endl;
    wi(); *out << "<color>" << endl; indent();
    wi(); *out << "<red>" << r << "</red>" << endl;
    wi(); *out << "<green>" << g << "</green>" << endl;
    wi(); *out << "<blue>" << b << "</blue>" << endl; undent();
    wi(); *out << "</color>" << endl;
    wi(); *out << "</property>" << endl;
}

void KDEVDLG2UI::writeStyles( const QStringList styles, bool isFrame )
{
    if ( isFrame ) {
	bool defineFrame = FALSE;
	QString shadow = "NoFrame";
	QString shape = "StyledPanel";
	int width = 2;
	if ( styles.contains( "WS_EX_STATICEDGE" ) ) {
	    shadow = "Plain";
	    width = 1;
	    defineFrame = TRUE;
	}
	if ( styles.contains( "WS_EX_CLIENTEDGE" ) ) {
	    shadow = "Sunken";
	    defineFrame = TRUE;
	}
	if ( styles.contains( "WS_EX_DLGMODALFRAME" ) ) {
	    shadow = "Raised";
	    defineFrame = TRUE;
	}
	if ( !styles.contains( "WS_BORDER" ) ) {
	    shape = "NoFrame";
	    defineFrame = TRUE;
	}

	if ( defineFrame ) {
	    writeEnum( "frameShape", "StyledPanel" );
	    writeEnum( "frameShadow", shadow );
	    writeNumber( "lineWidth", width );
	}
    }

    if ( styles.contains("WS_DISABLED") )
	writeBool("enabled", FALSE );
    if ( styles.contains("WS_EX_ACCEPTFILES") )
	writeBool("acceptDrops", TRUE );
    if ( styles.contains("WS_EX_TRANSPARENT") )
	writeBool("autoMask", TRUE );
    if ( !styles.contains("WS_TABSTOP") )
	writeEnum("focusPolicy", "NoFocus");
}

/*!
  Constructs a KDEVDLG2UI object
*/

KDEVDLG2UI::KDEVDLG2UI( QTextStream* input, const QString& name )
{
    className = name;
    writeToFile = TRUE;
    in = input;
    indentation = 0;
    out = 0;
}

/*!
  Destructs the KDEVDLG2UI object
*/

KDEVDLG2UI::~KDEVDLG2UI()
{
}

/*!
  Parses the input stream and writes the output to files.
*/

bool KDEVDLG2UI::parse()
{
    QFile fileOut;
    QString buffer;
	
    if ( writeToFile ) {

	QString outputFile = QString( className ) + ".ui";
	fileOut.setName( outputFile );
	if (!fileOut.open( IO_WriteOnly ) )
	    qFatal( "kdevdlg2ui: Could not open output file '%s'", outputFile.latin1() );
	out = new QTextStream( &fileOut );
	targetFiles.append( outputFile );
    } else {
	out = new QTextStream( &buffer, IO_WriteOnly );
    }
    
    writeDialog( className );

    delete out;
    out = 0;

    return TRUE;
}

/*!
  Parses the input stream and writes the output in \a get.
*/
bool KDEVDLG2UI::parse( QStringList& get )
{
    writeToFile = FALSE;
    bool result = parse();
    get = target;
    return result;
}

/*!
  Replaces characters like '&', '<' and '>' with the proper encoding.
*/
void KDEVDLG2UI::cleanString( QString * text )
{
    if ( !text ) return;
    text->replace( "\\n", "\n" );
    text->replace( "\\t", "\t" );
    text->replace( "&", "&amp;" );
    text->replace( "<", "&lt;" );
    text->replace( ">", "&gt;" );
}

/*!
  Builds a number of UI dialog out of the current input stream
*/
bool KDEVDLG2UI::writeDialog( const QString& name )
{  
    *out << "<!DOCTYPE UI><UI>" << endl;
    writeClass( name );
    
    while ( !in->eof() ) {
	
	line = in->readLine().simplifyWhiteSpace();
	
	if ( line.left( 4 ) == "data" ) {
	    // ignore data section
	    while ( line.left( 1 ) != "}" && !in->eof() ) {
		line = in->readLine();
	    }
	} else if ( line.left( 4 ) == "item" ) {
	    writeWidgetStart( line.section( "//", 0, 0 ).section( " ", 1, 1 ) );
	} else if ( line.left( 1 ) == "{" ) {
	    indent();
	} else if ( line.left( 1 ) == "}" ) {
	    undent();
	    writeWidgetEnd();
	} else if ( line.left( 4 ) == "Name" ) {
	    QString name = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    writeString( "name", name );
	} else if ( line.left( 4 ) == "Font" ) {
	    QString font = line.section( "//", 0, 0 ).section( "\"", 1 );
	    QString family = font.section("\"", 1, 1 );
	    int pointSize = font.section("\"", 3, 3 ).toInt();
	    //int weight = font.section("\"", 5, 5 ).toInt();
	    //bool italic = ( font.section("\"", 7, 7 ) == "TRUE" );
	    writeFont( family, pointSize ); // weight, italic ?
	} else if ( line.left( 9 ) == "IsEnabled" ) {
	    bool isEnabled =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "enabled", isEnabled );
	} else if ( line.left( 12 ) == "AcceptsDrops" ) {
	    bool acceptDrops =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "acceptDrops", acceptDrops );
	} else if ( line.left( 12 ) == "isAutoResize" ) {
	    bool isAutoResize =
	    	( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "autoResize", isAutoResize ); //###FIX: obsolete
	} else if ( line.left( 12 ) == "isAutoRepeat" ) {
	    bool isAutoRepeat =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "autoRepeat", isAutoRepeat );
	} else if ( line.left( 9 ) == "isDefault" ) {
	    bool isDefault =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "default", isDefault );
	} else if ( line.left( 13 ) == "isAutoDefault" ) {
	    bool isAutoDefault =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "autoDefault", isAutoDefault );
	} else if ( line.left( 14 ) == "isToggleButton" ) {
	    bool isToggleButton =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "toggleButton", isToggleButton );
	} else if ( line.left( 11 ) == "isToggledOn" ) {
	    bool isToggledOn =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "on", isToggledOn );
	} else if ( line.left( 8 ) == "hasFrame" ) {
	    bool hasFrame =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "frame", hasFrame );
	} else if ( line.left( 10 ) == "isReadOnly" ) {
	    bool isReadOnly =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "readOnly", isReadOnly );
	} else if ( line.left( 9 ) == "isChecked" ) {
	    bool isChecked =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "checked", isChecked );
	} else if ( line.left( 16 ) == "isAutoCompletion" ) {
	    bool isAutoCompl =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "autoCompletion", isAutoCompl );
	} else if ( line.left( 8 ) == "EditText" ) {
	    bool editText =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "editable", editText );
	} else if ( line.left( 10 ) == "isTracking" ) {
	    bool isTracking =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "tracking", isTracking );
	} else if ( line.left( 16 ) == "isMultiSelection" ) {
	    bool isMultiSel =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "multiSelection", isMultiSel );
	} else if ( line.left( 21 ) == "isAllColumnsShowFocus" ) {
	    bool isAllColsShowFocus =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "allColumnsShowFocus", isAllColsShowFocus );
	} else if ( line.left( 16 ) == "isRootDecorated" ) {
	    bool isRootDec =
		( line.section( "//", 0, 0 ).section("\"", 1, 1 ) == "true" );
	    writeBool( "rootIsDecorated", isRootDec );
	} else if ( line.left( 1 ) == "X" ) {
	    int x = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    line = in->readLine().stripWhiteSpace();
	    int y = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    line = in->readLine().stripWhiteSpace();
	    int w = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    line = in->readLine().stripWhiteSpace();
	    int h = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeRect( "geometry", x, y, w, h );
	} else if ( line.left( 8 ) == "MinWidth" ) {
	    int minw = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "minimumWidth", minw );
	} else if ( line.left( 9 ) == "MinHeight" ) {
	    int minh = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "minimumHeight", minh );
	} else if ( line.left( 8 ) == "MaxWidth" ) {
	    int maxw = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "maximumWidth", maxw );
	} else if ( line.left( 9 ) == "MaxHeight" ) {
	    int maxh = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "maximumHeight", maxh );
	} else if ( line.left( 4 ) == "Text" ) {
	    QString text = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & text );
	    writeString( "text", text );
	} else if ( line.left( 5 ) == "Title" ) {
	    QString title = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & title );
	    writeString( "title", title );
	} else if ( line.left( 5 ) == "Buddy" ) {
	    QString buddy = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    writeString( "buddy", buddy );
	} else if ( line.left( 14 ) == "SpecialValText" ) {
	    QString text = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    writeString( "specialValueText", text );
	} else if ( line.left( 6 ) == "Prefix" ) {
	    QString text = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    writeString( "prefix", text );
	}  else if ( line.left( 6 ) == "Suffix" ) {
	    QString text = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    writeString( "suffix", text );
	} else if ( line.left( 5 ) == "Value" ) {
	    int v = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "value", v );
	} else if ( line.left( 8 ) == "MinValue" ) {
	    int minv = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "minValue", minv );
	} else if ( line.left( 8 ) == "MaxValue" ) {
	    int maxv = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "maxValue", maxv );
	} else if ( line.left( 9 ) == "SizeLimit" ) {
	    int limit = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "sizeLimit", limit );
	} else if ( line.left( 9 ) == "MaxLength" ) {
	    int maxl = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "maxLength", maxl );
	} else if ( line.left( 8 ) == "MaxCount" ) {
	    int maxc = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "maxCount", maxc );
	} else if ( line.left( 14 ) == "CursorPosition" ) {
	    int pos = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "cursorPosition", pos );
	} else if ( line.left( 9 ) == "NumDigits" ) {
	    int digits = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "numDigits", digits );
	} else if ( line.left( 10 ) == "TotalSteps" ) {
	    int steps = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "totalSteps", steps );
	} else if ( line.left( 12 ) == "TreeStepSize" ) {
	    int stepSize = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "treeStepSize", stepSize );
	} else if ( line.left( 10 ) == "ItemMargin" ) {
	    int margin = line.section( "//", 0, 0 ).section("\"", 1, 1 ).toInt();
	    writeNumber( "itemMargin", margin );
	} else if ( line.left( 7 ) == "ToolTip" ) {
	    QString text = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    writeString( "toolTip", text );
	} else if ( line.left( 9 ) == "QuickHelp" ) {
	    QString text = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & text );
	    writeString( "whatsThis", text );
	} else if ( line.left( 15 ) == "InsertionPolicy" ) {
	    QString policy = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & policy );
	    writeEnum( "insertionPolicy", policy ); //###FIX: QComboBox::
	} else if ( line.left( 11 ) == "Orientation" ) {
	    QString orientation =
		line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & orientation );
	    if ( orientation == "V" )
		writeEnum( "orientation", "Qt::Vertical"  );
	    else if ( orientation == "H" )
		writeEnum( "orientation", "Qt::Horizontal"  );
	} else if ( line.left( 14 ) == "vScrollBarMode" ) {
	    QString mode = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & mode );
	    writeEnum( "vScrollBarMode", mode );
	} else if ( line.left( 14 ) == "hScrollBarMode" ) {
	    QString mode = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & mode );
	    writeEnum( "hScrollBarMode", mode );
	} else if ( line.left( 7 ) == "Entries" ) {
	    QString entries = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & entries );
	    QStringList l = QStringList::split( '\n', entries );
	    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it )
		writeItem( "text", *it );
	} else if ( line.left( 7 ) == "Columns" ) {
	    QString columns = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	    cleanString( & columns );
	    QStringList l = QStringList::split( '\n', columns );
	    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it )
		writeColumn( "text", *it );
	} else if ( line.left( 6 ) == "BgMode" ) {
	   QString mode = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	   cleanString( & mode );
	   writeString( "backgroundMode", mode ); //###FIX: QWidget:::
	} else if ( line.left( 10 ) == "BgPalColor" ) {
	   QString color = line.section( "//", 0, 0 ).section("\"", 1, 1 );
	   cleanString( & color );
	   writeColor( "paletteBackgroundColor", color );
	} //else {
	    //if ( line.length() )
	    //qDebug( "IGNORED: %s", line.latin1() );
	//}
    }
    *out << "</UI>" << endl;
    return TRUE;
}

