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

#include "syntaxhighliter_cpp.h"
#include "paragdata.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qmap.h"
#include "qapplication.h"
#include "qregexp.h"

const char * const SyntaxHighlighter_CPP::keywords[] = {
    // C++ keywords
    "and",
    "and_eq",
    "asm",
    "auto",
    "bitand",
    "bitor",
    "bool",
    "break",
    "case",
    "catch",
    "char",
    "class",
    "compl",
    "const",
    "const_cast",
    "continue",
    "default",
    "delete",
    "do",
    "double",
    "dynamic_cast",
    "else",
    "enum",
    "explicit",
    "export",
    "extern",
    "false",
    "FALSE",
    "float",
    "for",
    "friend",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "mutable",
    "namespace",
    "new",
    "not",
    "not_eq",
    "operator",
    "or",
    "or_eq",
    "private",
    "protected",
    "public",
    "register",
    "reinterpret_cast",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "static_cast",
    "struct",
    "switch",
    "template",
    "this",
    "throw",
    "true",
    "TRUE",
    "try",
    "typedef",
    "typeid",
    "typename",
    "union",
    "unsigned",
    "using",
    "virtual",
    "void",
    "volatile",
    "wchar_t",
    "while",
    "xor",
    "xor_eq",
    // additional "keywords" intoduced by Qt
    "slots",
    "signals",
    "uint",
    "ushort",
    "ulong",
    "emit",
    // end of array
    0
};

static QMap<int, QMap<QString, int > > *wordMap = 0;

SyntaxHighlighter_CPP::SyntaxHighlighter_CPP()
    : QTextPreProcessor(), lastFormat( 0 ), lastFormatId( -1 )
{
    QFont f( qApp->font() );

    addFormat( Standard, new QTextFormat( f, Qt::black ) );
    addFormat( Number, new QTextFormat( f, Qt::darkBlue ) );
    addFormat( String, new QTextFormat( f, Qt::darkGreen ) );
    addFormat( Type, new QTextFormat( f, Qt::darkMagenta ) );
    addFormat( Keyword, new QTextFormat( f, Qt::darkYellow ) );
    addFormat( PreProcessor, new QTextFormat( f, Qt::darkBlue ) );
    addFormat( Label, new QTextFormat( f, Qt::darkRed ) );
    f.setFamily( "times" );
    addFormat( Comment, new QTextFormat( f, Qt::red ) );

    if ( wordMap )
	return;

    wordMap = new QMap<int, QMap<QString, int> >;
    int len;
    for ( int i = 0; keywords[ i ]; ++i ) {
	len = (int)strlen( keywords[ i ] );
	if ( !wordMap->contains( len ) )
	    wordMap->insert( len, QMap<QString, int >() );
	QMap<QString, int> &map = wordMap->operator[]( len );
	map[ keywords[ i ] ] = Keyword;
    }
}

SyntaxHighlighter_CPP::~SyntaxHighlighter_CPP()
{
}

static int string2Id( const QString &s )
{
    if ( s == "Standard" )
	return SyntaxHighlighter_CPP::Standard;
    if ( s == "Comment" )
	return SyntaxHighlighter_CPP::Comment;
    if ( s == "Number" )
	return SyntaxHighlighter_CPP::Number;
    if ( s == "String" )
	return SyntaxHighlighter_CPP::String;
    if ( s == "Type" )
	return SyntaxHighlighter_CPP::Type;
    if ( s == "Preprocessor" )
	return SyntaxHighlighter_CPP::PreProcessor;
    if ( s == "Label" )
	return SyntaxHighlighter_CPP::Label;
    if ( s == "Keyword" )
	return SyntaxHighlighter_CPP::Keyword;
    return SyntaxHighlighter_CPP::Standard;
}

void SyntaxHighlighter_CPP::updateStyles( const QMap<QString, ConfigStyle> &styles )
{
    for ( QMap<QString, ConfigStyle>::ConstIterator it = styles.begin(); it != styles.end(); ++it ) {
	int id = string2Id( it.key() );
	QTextFormat *f = format( id );
	if ( !f )
	    continue;
	f->setFont( (*it).font );
	f->setColor( (*it).color );
    }
}

void SyntaxHighlighter_CPP::process( QTextDocument *doc, QTextParagraph *string, int, bool invalidate )
{

    QTextFormat *formatStandard = format( Standard );
    QTextFormat *formatComment = format( Comment );
    QTextFormat *formatNumber = format( Number );
    QTextFormat *formatString = format( String );
    QTextFormat *formatType = format( Type );
    QTextFormat *formatPreProcessor = format( PreProcessor );
    QTextFormat *formatLabel = format( Label );

    // states
    const int StateStandard = 0;
    const int StateCommentStart1 = 1;
    const int StateCCommentStart2 = 2;
    const int StateCppCommentStart2 = 3;
    const int StateCComment = 4;
    const int StateCppComment = 5;
    const int StateCCommentEnd1 = 6;
    const int StateCCommentEnd2 = 7;
    const int StateStringStart = 8;
    const int StateString = 9;
    const int StateStringEnd = 10;
    const int StateString2Start = 11;
    const int StateString2 = 12;
    const int StateString2End = 13;
    const int StateNumber = 14;
    const int StatePreProcessor = 15;

    // tokens
    const int InputAlpha = 0;
    const int InputNumber = 1;
    const int InputAsterix = 2;
    const int InputSlash = 3;
    const int InputParen = 4;
    const int InputSpace = 5;
    const int InputHash = 6;
    const int InputQuotation = 7;
    const int InputApostrophe = 8;
    const int InputSep = 9;

    static uchar table[ 16 ][ 10 ] = {
	{ StateStandard,      StateNumber,     StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateStandard
	{ StateStandard,      StateNumber,   StateCCommentStart2, StateCppCommentStart2, StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateCommentStart1
	{ StateCComment,      StateCComment,   StateCCommentEnd1,   StateCComment,         StateCComment,   StateCComment,   StateCComment,     StateCComment,    StateCComment,     StateCComment }, // StateCCommentStart2
	{ StateCppComment,    StateCppComment, StateCppComment,     StateCppComment,       StateCppComment, StateCppComment, StateCppComment,   StateCppComment,  StateCppComment,   StateCppComment }, // CppCommentStart2
	{ StateCComment,      StateCComment,   StateCCommentEnd1,   StateCComment,         StateCComment,   StateCComment,   StateCComment,     StateCComment,    StateCComment,     StateCComment }, // StateCComment
	{ StateCppComment,    StateCppComment, StateCppComment,     StateCppComment,       StateCppComment, StateCppComment, StateCppComment,   StateCppComment,  StateCppComment,   StateCppComment }, // StateCppComment
	{ StateCComment,      StateCComment,   StateCCommentEnd1,   StateCCommentEnd2,     StateCComment,   StateCComment,   StateCComment,     StateCComment,    StateCComment,     StateCComment }, // StateCCommentEnd1
	{ StateStandard,      StateNumber,     StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateCCommentEnd2
	{ StateString,        StateString,     StateString,         StateString,           StateString,     StateString,     StateString,       StateStringEnd,   StateString,       StateString }, // StateStringStart
	{ StateString,        StateString,     StateString,         StateString,           StateString,     StateString,     StateString,       StateStringEnd,   StateString,       StateString }, // StateString
	{ StateStandard,      StateStandard,   StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateStringEnd
	{ StateString2,       StateString2,    StateString2,        StateString2,          StateString2,    StateString2,    StateString2,      StateString2,     StateString2End,   StateString2 }, // StateString2Start
	{ StateString2,       StateString2,    StateString2,        StateString2,          StateString2,    StateString2,    StateString2,      StateString2,     StateString2End,   StateString2 }, // StateString2
	{ StateStandard,      StateStandard,   StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateString2End
	{ StateNumber,        StateNumber,     StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard }, // StateNumber
	{ StatePreProcessor,  StateStandard,   StateStandard,       StateCommentStart1,    StateStandard,   StateStandard,   StatePreProcessor, StateStringStart, StateString2Start, StateStandard } // StatePreProcessor
    };

    QString buffer;

    int state = StateStandard;
    if ( string->prev() ) {
	if ( string->prev()->endState() == -1 )
	    process( doc, string->prev(), 0, FALSE );
	state = string->prev()->endState();
    }
    int input = -1;
    int i = 0;
    bool lastWasBackSlash = FALSE;
    bool makeLastStandard = FALSE;

    ParagData *paragData = (ParagData*)string->extraData();
    if ( paragData )
	paragData->parenList.clear();
    else
	paragData = new ParagData;

    QString alphabeth = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QString mathChars = "xXeE";
    QString numbers = "0123456789";
    bool questionMark = FALSE;
    QChar lastChar;
    for (;;) {
	QChar c = string->at( i )->c;

	if ( lastWasBackSlash ) {
	    input = InputSep;
	} else {
	    switch ( c ) {
	    case '*':
		input = InputAsterix;
		break;
	    case '/':
		input = InputSlash;
		break;
	    case '(': case '[': case '{':
		input = InputParen;
		if ( state == StateStandard ||
		     state == StateNumber ||
		     state == StatePreProcessor ||
		     state == StateCCommentEnd2 ||
		     state == StateCCommentEnd1 ||
		     state == StateString2End ||
		     state == StateStringEnd )
		    paragData->parenList << Paren( Paren::Open, c, i );
		break;
	    case ')': case ']': case '}':
		input = InputParen;
		if ( state == StateStandard ||
		     state == StateNumber ||
		     state == StatePreProcessor ||
		     state == StateCCommentEnd2 ||
		     state == StateCCommentEnd1 ||
		     state == StateString2End ||
		     state == StateStringEnd )
		    paragData->parenList << Paren( Paren::Closed, c, i );
		break;
	    case '#':
		input = InputHash;
		break;
	    case '"':
		input = InputQuotation;
		break;
	    case '\'':
		input = InputApostrophe;
		break;
	    case ' ':
		input = InputSpace;
		break;
	    case '1': case '2': case '3': case '4': case '5':
	    case '6': case '7': case '8': case '9': case '0':
		if ( alphabeth.find( lastChar ) != -1 &&
		     ( mathChars.find( lastChar ) == -1 || numbers.find( string->at( i - 1 )->c ) == -1 ) ) {
		    input = InputAlpha;
		} else {
		    if ( input == InputAlpha && numbers.find( lastChar ) != -1 )
			input = InputAlpha;
		    else
			input = InputNumber;
		}
		break;
	    case ':': {
		input = InputAlpha;
		QChar nextChar = ' ';
		if ( i < string->length() - 1 )
		    nextChar = string->at( i + 1 )->c;
		if ( state == StateStandard && !questionMark && lastChar != ':' && nextChar != ':' ) {
		    for ( int j = 0; j < i; ++j ) {
			if ( string->at( j )->format() == formatStandard )
			    string->setFormat( j, 1, formatLabel, FALSE );
		    }
		}
	    } break;
	    default: {
		if ( !questionMark && c == '?' )
		    questionMark = TRUE;
		if ( c.isLetter() || c == '_' )
		    input = InputAlpha;
		else
		    input = InputSep;
	    } break;
	    }
	}

	lastWasBackSlash = !lastWasBackSlash && c == '\\';

	if ( input == InputAlpha )
	    buffer += c;

    	state = table[ state ][ input ];

	switch ( state ) {
	case StateStandard: {
	    int len = buffer.length();
	    string->setFormat( i, 1, formatStandard, FALSE );
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    if ( buffer.length() > 0 && input != InputAlpha ) {
		if ( buffer[ 0 ] == 'Q' ) {
		    string->setFormat( i - buffer.length(), buffer.length(), formatType, FALSE );
		} else {
		    QMap<int, QMap<QString, int > >::Iterator it = wordMap->find( len );
		    if ( it != wordMap->end() ) {
			QMap<QString, int >::Iterator it2 = ( *it ).find( buffer );
			if ( it2 != ( *it ).end() )
			    string->setFormat( i - buffer.length(), buffer.length(), format( ( *it2 ) ), FALSE );
		    }
		}
		buffer = QString::null;
	    }
	} break;
	case StateCommentStart1:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = TRUE;
	    buffer = QString::null;
	    break;
	case StateCCommentStart2:
	    string->setFormat( i - 1, 2, formatComment, FALSE );
	    makeLastStandard = FALSE;
	    buffer = QString::null;
	    break;
	case StateCppCommentStart2:
	    string->setFormat( i - 1, 2, formatComment, FALSE );
	    makeLastStandard = FALSE;
	    buffer = QString::null;
	    break;
	case StateCComment:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatComment, FALSE );
	    buffer = QString::null;
	    break;
	case StateCppComment:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatComment, FALSE );
	    buffer = QString::null;
	    break;
	case StateCCommentEnd1:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatComment, FALSE );
	    buffer = QString::null;
	    break;
	case StateCCommentEnd2:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatComment, FALSE );
	    buffer = QString::null;
	    break;
	case StateStringStart:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatStandard, FALSE );
	    buffer = QString::null;
	    break;
	case StateString:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatString, FALSE );
	    buffer = QString::null;
	    break;
	case StateStringEnd:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatStandard, FALSE );
	    buffer = QString::null;
	    break;
	case StateString2Start:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatStandard, FALSE );
	    buffer = QString::null;
	    break;
	case StateString2:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatString, FALSE );
	    buffer = QString::null;
	    break;
	case StateString2End:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatStandard, FALSE );
	    buffer = QString::null;
	    break;
	case StateNumber:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatNumber, FALSE );
	    buffer = QString::null;
	    break;
	case StatePreProcessor:
	    if ( makeLastStandard )
		string->setFormat( i - 1, 1, formatStandard, FALSE );
	    makeLastStandard = FALSE;
	    string->setFormat( i, 1, formatPreProcessor, FALSE );
	    buffer = QString::null;
	    break;
	}

	lastChar = c;
	i++;
	if ( i >= string->length() )
	    break;
    }

    string->setExtraData( paragData );

    int oldEndState = string->endState();
    if ( state == StateCComment ||
	 state == StateCCommentEnd1 ) {
	string->setEndState( StateCComment );
    } else if ( state == StateString ) {
	string->setEndState( StateString );
    } else if ( state == StateString2 ) {
	string->setEndState( StateString2 );
    } else {
	string->setEndState( StateStandard );
    }

    string->setFirstPreProcess( FALSE );

    QTextParagraph *p = string->next();
    if ( (!!oldEndState || !!string->endState()) && oldEndState != string->endState() &&
	 invalidate && p && !p->firstPreProcess() && p->endState() != -1 ) {
	while ( p ) {
	    if ( p->endState() == -1 )
		return;
	    p->setEndState( -1 );
	    p = p->next();
	}
    }
}

QTextFormat *SyntaxHighlighter_CPP::format( int id )
{
    if ( lastFormatId == id  && lastFormat )
	return lastFormat;

    QTextFormat *f = formats[ id ];
    lastFormat = f ? f : formats[ 0 ];
    lastFormatId = id;
    return lastFormat;
}

void SyntaxHighlighter_CPP::addFormat( int id, QTextFormat *f )
{
    formats.insert( id, f );
}

void SyntaxHighlighter_CPP::removeFormat( int id )
{
    formats.remove( id );
}

