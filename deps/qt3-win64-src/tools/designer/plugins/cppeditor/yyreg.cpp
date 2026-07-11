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

#include <qregexp.h>

#include <ctype.h>
#include <stdio.h>

#include "yyreg.h"

/*
  First comes the tokenizer. We don't need something that knows much
  about C++. However, we need something that gives tokens from the
  end of the file to the start, which is tricky.

  If you are not familiar with hand-written tokenizers and parsers,
  you might want to read other simpler parsers written in the same
  style:

	$(QTDIR)/src/tools/qregexp.cpp
	$(QTDIR)/tools/inspector/cppparser.cpp
  
  You might also want to read Section 2 in the Dragon Book.
*/

/*
  Those are the tokens we are interested in. Tok_Something represents
  any C++ token that does not interest us, but it's dangerous to
  ignore tokens completely.
*/
enum { Tok_Boi, Tok_Ampersand, Tok_Aster, Tok_LeftParen, Tok_RightParen,
       Tok_Equal, Tok_LeftBrace, Tok_RightBrace, Tok_Semicolon, Tok_Colon,
       Tok_LeftAngle, Tok_RightAngle, Tok_Comma, Tok_Ellipsis, Tok_Gulbrandsen,
       Tok_LeftBracket, Tok_RightBracket, Tok_Tilde, Tok_Something, Tok_Comment,
       Tok_Ident,

       Tok_char, Tok_const, Tok_double, Tok_int, Tok_long, Tok_operator,
       Tok_short, Tok_signed, Tok_unsigned };

/*
  The following variables store the lexical analyzer state. The best way
  to understand them is to implement a function myGetToken() that calls
  getToken(), to add some qDebug() statements in there and then to
  #define getToken() myGetToken().
*/
static QString *yyIn; // the input stream
static int yyPos; // the position of the current token in yyIn
static int yyCurPos; // the position of the next lookahead character
static char *yyLexBuf; // the lexeme buffer
static const int YYLexBufSize = 65536; // big enough for long comments
static char *yyLex; // the lexeme itself (a pointer into yyLexBuf)
static int yyCh; // the lookbehind character

/*
  Moves back to the previous character in the input stream and
  updates the tokenizer state. This function is to be used only by
  getToken(), which provides the right abstraction.
*/
static inline void readChar()
{
    if ( yyCh == EOF )
	return;

    if ( yyLex > yyLexBuf )
	*--yyLex = (char) yyCh;

    if ( yyCurPos < 0 )
	yyCh = EOF;
    else
	yyCh = (*yyIn)[yyCurPos].unicode();
    yyCurPos--;
}

/*
  Sets up the tokenizer.
*/
static void startTokenizer( const QString& in )
{
    yyIn = new QString;
    *yyIn = in;
    yyPos = yyIn->length() - 1;
    yyCurPos = yyPos;
    yyLexBuf = new char[YYLexBufSize];
    yyLex = yyLexBuf + YYLexBufSize - 1;
    *yyLex = '\0';
    yyCh = '\0';
    readChar();
}

/*
  Frees resources allocated by the tokenizer.
*/
static void stopTokenizer()
{
    delete yyIn;
    delete[] yyLexBuf;
    yyLexBuf = 0;
}

/*
  These two macros implement quick-and-dirty hashing for telling
  apart keywords fast.
*/
#define HASH( ch, len ) ( (ch) | ((len) << 8) )
#define CHECK( target ) \
    if ( strcmp((target), yyLex) != 0 ) \
	break;

/*
  Returns the previous token in the abstract token stream. The parser
  deals only with tokens, not with characters.
*/
static int getToken()
{
    // why "+ 2"? try putting some qDebug()'s and see
    yyPos = yyCurPos + 2;

    for ( ;; ) {
	/*
	  See if the previous token is interesting. If it isn't, we
	  will loop anyway an go to the token before the previous
	  token, and so on.
	*/

	yyLex = yyLexBuf + YYLexBufSize - 1;
	*yyLex = '\0';

	if ( yyCh == EOF ) {
	    break;
	} else if ( isspace(yyCh) ) {
	    bool metNL = FALSE;
	    do {
		metNL = ( metNL || yyCh == '\n' );
		readChar();
	    } while ( isspace(yyCh) );

	    if ( metNL ) {
		/*
		  C++ style comments are tricky. In left-to-right
		  thinking, C++ comments start with "//" and end with
		  '\n'. In right-to-left thinking, they start with a
		  '\n'; but of course not every '\n' starts a comment.

		  When we meet the '\n', we look behind, on the same
		  line, for a "//", and if there is one we mess
		  around with the tokenizer state to effectively
		  ignore the comment. Beware of off-by-one and
		  off-by-two bugs when you modify this code by adding
		  qDebug()'s here and there.
		*/
		if ( yyCurPos >= 0 ) {
		    int lineStart = yyIn->findRev( QChar('\n'), yyCurPos ) + 1;
		    QString line = yyIn->mid( lineStart,
					      yyCurPos - lineStart + 2 );
		    int commentStart = line.find( QString("//") );
		    if ( commentStart != -1 ) {
			yyCurPos = lineStart + commentStart - 1;
			yyPos = yyCurPos + 2;
			readChar();
		    }
		}
	    }
	} else if ( isalnum(yyCh) || yyCh == '_' ) {
	    do {
		readChar();
	    } while ( isalnum(yyCh) || yyCh == '_' );

	    switch ( HASH(yyLex[0], strlen(yyLex)) ) {
	    case HASH( 'c', 4 ):
		CHECK( "char" );
		return Tok_char;
	    case HASH( 'c', 5 ):
		CHECK( "const" );
		return Tok_const;
	    case HASH( 'd', 6 ):
		CHECK( "double" );
		return Tok_double;
	    case HASH( 'i', 3 ):
		CHECK( "int" );
		return Tok_int;
	    case HASH( 'l', 4 ):
		CHECK( "long" );
		return Tok_long;
	    case HASH( 'o', 8 ):
		CHECK( "operator" );
		return Tok_operator;
	    case HASH( 's', 5 ):
		CHECK( "short" );
		return Tok_short;
	    case HASH( 's', 6 ):
		CHECK( "signed" );
		return Tok_signed;
	    case HASH( 'u', 8 ):
		CHECK( "unsigned" );
		return Tok_unsigned;
	    }
	    if ( isdigit(*yyLex) )
		return Tok_Something;
	    else
		return Tok_Ident;
	} else {
	    int quote;

	    switch ( yyCh ) {
	    case '!':
	    case '%':
	    case '^':
	    case '+':
	    case '-':
	    case '?':
	    case '|':
		readChar();
		return Tok_Something;
	    case '"':
	    case '\'':
		quote = yyCh;
		readChar();

		while ( yyCh != EOF && yyCh != '\n' ) {
		    if ( yyCh == quote ) {
			readChar();
			if ( yyCh != '\\' )
			    break;
		    } else {
			readChar();
		    }
		}
		return Tok_Something;
	    case '&':
		readChar();
		if ( yyCh == '&' ) {
		    readChar();
		    return Tok_Something;
		} else {
		    return Tok_Ampersand;
		}
	    case '(':
		readChar();
		return Tok_LeftParen;
	    case ')':
		readChar();
		return Tok_RightParen;
	    case '*':
		readChar();
		return Tok_Aster;
	    case ',':
		readChar();
		return Tok_Comma;
	    case '.':
		readChar();
		if ( yyCh == '.' ) {
		    do {
			readChar();
		    } while ( yyCh == '.' );
		    return Tok_Ellipsis;
		} else {
		    return Tok_Something;
		}
	    case '/':
		/*
		  C-style comments are symmetric. C++-style comments
		  are handled elsewhere.
		*/
		readChar();
		if ( yyCh == '*' ) {
		    bool metAster = FALSE;
		    bool metAsterSlash = FALSE;

		    readChar();

		    while ( !metAsterSlash ) {
			if ( yyCh == EOF )
			    break;

			if ( yyCh == '*' )
			    metAster = TRUE;
			else if ( metAster && yyCh == '/' )
			    metAsterSlash = TRUE;
			else
			    metAster = FALSE;
			readChar();
		    }
		    break;
		    // return Tok_Comment;
		} else {
		    return Tok_Something;
		}
	    case ':':
		readChar();
		if ( yyCh == ':' ) {
		    readChar();
		    return Tok_Gulbrandsen;
		} else {
		    return Tok_Colon;
		}
	    case ';':
		readChar();
		return Tok_Semicolon;
	    case '<':
		readChar();
		return Tok_LeftAngle;
	    case '=':
		readChar();
		return Tok_Equal;
	    case '>':
		readChar();
		return Tok_RightAngle;
	    case '[':
		readChar();
		return Tok_LeftBracket;
	    case ']':
		readChar();
		return Tok_RightBracket;
	    case '{':
		readChar();
		return Tok_LeftBrace;
	    case '}':
		readChar();
		return Tok_RightBrace;
	    case '~':
		readChar();
		return Tok_Tilde;
	    default:
		readChar();
	    }
	}
    }
    return Tok_Boi;
}

/*
  Follow the member function(s) of CppFunction.
*/

/*
  Returns the prototype for the C++ function, without the semicolon.
*/
QString CppFunction::prototype() const
{
    QString proto;

    if ( !returnType().isEmpty() )
	proto = returnType() + QChar( ' ' );
    proto += scopedName();
    proto += QChar( '(' );
    if ( !parameterList().isEmpty() ) {
	QStringList::ConstIterator p = parameterList().begin();
	proto += *p;
	++p;
	while ( p != parameterList().end() ) {
	    proto += QString( ", " );
	    proto += *p;
	    ++p;
	}
    }
    proto += QChar( ')' );
    if ( isConst() )
	proto += QString( " const" );
    return proto;
}

/*
  The parser follows. We are not really parsing C++, just trying to
  find the start and end of function definitions.

  One important pitfall is that the parsed code needs not be valid.
  Parsing from right to left helps cope with that, as explained in
  comments below.

  In the examples, we will use the symbol @ to stand for the position
  in the token stream. In "int @ x ;", the lookahead token (yyTok) is
  'int'.
*/

static int yyTok; // the current token

/*
  Returns TRUE if thingy is a constructor or a destructor; otherwise
  returns FALSE.
*/
static bool isCtorOrDtor( const QString& thingy )
{
    // e.g., Alpha<a>::Beta<Bar<b, c> >::~Beta
    QRegExp xtor( QString(
	    "(?:([A-Z_a-z][0-9A-Z_a-z]*)" // class name
	       "(?:<(?:[^>]|<[^>]*>)*>)*" // template arguments
	       "::)+"                     // many in a row
	    "~?"                          // ctor or dtor?
	    "\\1") );                     // function has same name as class
    return xtor.exactMatch( thingy );
}

/*
  Skips over any template arguments with balanced angle brackets, and
  returns the skipped material as a string.

  Before: QMap < QString , QValueList < QString > > @ m ;
  After: QMap @ < QString , QValueList < QString > > m ;
*/
static QString matchTemplateAngles()
{
    QString t;

    if ( yyTok == Tok_RightAngle ) {
	int depth = 0;
	do {
	    if ( yyTok == Tok_RightAngle )
		depth++;
	    else if ( yyTok == Tok_LeftAngle )
		depth--;
	    t.prepend( yyLex );
	    yyTok = getToken();
	} while ( depth > 0 && yyTok != Tok_Boi && yyTok != Tok_LeftBrace );
    }
    return t;
}

/*
  Similar to matchTemplateAngles(), but for array brackets in parameter
  data types (as in "int *argv[]").
*/
static QString matchArrayBrackets()
{
    QString t;

    while ( yyTok == Tok_RightBracket ) {
	t.prepend( yyLex );
	yyTok = getToken();
	if ( yyTok == Tok_Something ) {
	    t.prepend( yyLex );
	    yyTok = getToken();
	}
	if ( yyTok != Tok_LeftBracket )
	    return QString::null;
	t.prepend( yyLex );
	yyTok = getToken();
    }
    return t;
}

/*
  Prepends prefix to *type. This operation is in theory trivial, but
  for the spacing to look good, we have to do something. The original
  spacing is lost as the input is tokenized.
*/
static void prependToType( QString *type, const QString& prefix )
{
    if ( !type->isEmpty() && !prefix.isEmpty() ) {
	QChar left = prefix[(int) prefix.length() - 1];
	QChar right = (*type)[0];

	if ( left.isLetter() &&
	     (right.isLetter() || right == QChar('*') || right == QChar('&')) )
	    type->prepend( QChar(' ') );
    }
    type->prepend( prefix );
}

static bool isModifier( int tok )
{
    return ( tok == Tok_signed || tok == Tok_unsigned ||
	     tok == Tok_short || tok == Tok_long );
}

/*
  Parses a data type (backwards as usual) and returns a textual
  representation of it.
*/
static QString matchDataType()
{
    QString type;

    while ( yyTok == Tok_Ampersand || yyTok == Tok_Aster ||
	    yyTok == Tok_const ) {
	prependToType( &type, yyLex );
	yyTok = getToken();
    }

    /*
      This code is really hard to follow... sorry. The loop matches
      Alpha::Beta::Gamma::...::Omega.
    */
    for ( ;; ) {
	bool modifierMet = FALSE;

	prependToType( &type, matchTemplateAngles() );

	if ( yyTok != Tok_Ident ) {
	    /*
	      People may write 'const unsigned short' or
	      'short unsigned const' or any other permutation.
	    */
	    while ( yyTok == Tok_const || isModifier(yyTok) ) {
		prependToType( &type, yyLex );
		yyTok = getToken();
		if ( yyTok != Tok_const )
		    modifierMet = TRUE;
	    }

	    if ( yyTok == Tok_Tilde ) {
		prependToType( &type, yyLex );
		yyTok = getToken();
	    }
	}

	if ( !modifierMet ) {
	    if ( yyTok == Tok_Ellipsis || yyTok == Tok_Ident ||
		 yyTok == Tok_char || yyTok == Tok_int ||
		 yyTok == Tok_double ) {
		prependToType( &type, yyLex );
		yyTok = getToken();
	    } else {
		return QString::null;
	    }
	} else if ( yyTok == Tok_int || yyTok == Tok_char ||
		    yyTok == Tok_double ) {
	    prependToType( &type, yyLex );
	    yyTok = getToken();
	}

	while ( yyTok == Tok_const || isModifier(yyTok) ) {
	    prependToType( &type, yyLex );
	    yyTok = getToken();
	}

	if ( yyTok == Tok_Gulbrandsen ) {
	    prependToType( &type, yyLex );
	    yyTok = getToken();
	} else {
	    break;
	}
    }
    return type;
}

/*
  Parses a function prototype (without the semicolon) and returns an
  object that stores information about this function.
*/
static CppFunction matchFunctionPrototype( bool stripParamNames )
{
    CppFunction func;
#if 0
    QString documentation;
#endif
    QString returnType;
    QString scopedName;
    QStringList params;
    QString qualifier;
    bool cnst = FALSE;

    if ( yyTok == Tok_const ) {
	cnst = TRUE;
	yyTok = getToken();
    }

    if ( yyTok != Tok_RightParen )
	return func;
    yyTok = getToken();

    if ( yyTok != Tok_LeftParen ) {
	for ( ;; ) {
	    QString brackets = matchArrayBrackets();
	    QString name;
	    if ( yyTok == Tok_Ident ) {
		name = yyLex;
		yyTok = getToken();
	    }
	    QString type = matchDataType();

	    if ( type.isEmpty() ) {
		if ( name.isEmpty() )
		    return func;
		type = name;
		name = QString::null;
	    }
	    if ( stripParamNames )
		name = QString::null;

	    QString param = type + QChar( ' ' ) + name + brackets;
	    params.prepend( param.stripWhiteSpace() );

	    if ( yyTok != Tok_Comma )
		break;
	    yyTok = getToken();
	}
	if ( yyTok != Tok_LeftParen )
	    return func;
    }
    yyTok = getToken();

    for ( ;; ) {
	scopedName.prepend( matchTemplateAngles() );

	if ( yyTok != Tok_Ident ) {
	    // the operator keyword should be close
	    int i = 0;
	    while ( i < 4 && yyTok != Tok_operator ) {
		scopedName.prepend( yyLex );
		i++;
	    }
	    if ( yyTok != Tok_operator )
		return func;
	}
	scopedName.prepend( yyLex );
	yyTok = getToken();

	if ( yyTok != Tok_Gulbrandsen )
	    break;
	scopedName.prepend( yyLex );
	yyTok = getToken();
    }

    if ( !isCtorOrDtor(scopedName) ) {
	returnType = matchDataType();
	if ( returnType.isEmpty() )
	    return func;
    }

    /*
      The documentation feature is unused so far, since we cannot
      really distinguist between a normal comment between two
      functions and one that relates to the following function. One
      good heuristic is to assume that a comment immediately followed
      by a function with no blank line in between relates to the
      function, but there's no easy way to find that out with a
      tokenizer.
    */
#if 0
    if ( yyTok == Tok_Comment ) {
	documentation = yyLex;
	yyTok = getToken();
    }

    func.setDocumentation( documentation );
#endif
    func.setReturnType( returnType );
    func.setScopedName( scopedName );
    func.setParameterList( params );
    func.setConst( cnst );
    return func;
}

/*
  Try to set the body. It's not sufficient to call
  func->setBody(somewhatBody), as the somewhatBody might be too large.
  Case in point:

    void foo()
    {
	printf( "Hello" );
    }

    int n;

    void bar()
    {
	printf( " world!\n" );
    }

  The parser first finds bar(). Then it finds "void foo() {" and
  naively expects the body to extend up to "void bar()". This
  function's job is to count braces and make sure "int n;" is not
  counted as part of the body.

  Cases where the closing brace of the body is missing require no
  special processing.
*/
static void setBody( CppFunction *func, const QString& somewhatBody )
{
    QString body = somewhatBody;

    int braceDepth = 0;
    int i = 0;
    while ( i < (int) body.length() ) {
	if ( body[i] == QChar('{') ) {
	    braceDepth++;
	} else if ( body[i] == QChar('}') ) {
	    braceDepth--;
	    if ( braceDepth == 0 ) {
		body.truncate( i + 1 );
		break;
	    }
	}
	i++;
    }

    func->setBody( body );
}

/*
  Parses a whole C++ file, looking for function definitions. Case in
  point:

    void foo()
    {
	printf( "Hello" );

    void bar()
    {
	printf( " world!\n" );
    }

  The parser looks for left braces and tries to parse a function
  prototype backwards. First it finds "void bar() {". Then it works
  up and finds "void foo() {".
*/
static void matchTranslationUnit( QValueList<CppFunction> *flist )
{
    int endBody = -1;
    int startBody;

    for ( ;; ) {
	if ( endBody == -1 )
	    endBody = yyPos;

	while ( yyTok != Tok_Boi && yyTok != Tok_LeftBrace )
	    yyTok = getToken();
	if ( yyTok == Tok_Boi )
	    break;

	// found a left brace
	yyTok = getToken();
	startBody = yyPos;
	CppFunction func = matchFunctionPrototype( FALSE );
	if ( !func.scopedName().isEmpty() ) {
	    QString body = yyIn->mid( startBody, endBody - startBody );
	    setBody( &func, body );
	    body = func.body(); // setBody() can change the body

	    /*
	      Compute important line numbers.
	    */
	    int functionStartLineNo = 1 + QConstString( yyIn->unicode(), yyPos )
					  .string().contains( QChar('\n') );
	    int startLineNo = functionStartLineNo +
		    QConstString( yyIn->unicode() + yyPos, startBody - yyPos )
		    .string().contains( QChar('\n') );
	    int endLineNo = startLineNo + body.contains( QChar('\n') );

	    func.setLineNums( functionStartLineNo, startLineNo, endLineNo );
	    flist->prepend( func );
	    endBody = -1;
	}
    }
}

/*
  Extracts C++ function from source code and put them in a list.
*/
void extractCppFunctions( const QString& code, QValueList<CppFunction> *flist )
{
    startTokenizer( code );
    yyTok = getToken();
    matchTranslationUnit( flist );
    stopTokenizer();
}

/*
  Returns the prototype with the parameter names removed.
*/
QString canonicalCppProto( const QString& proto )
{
    startTokenizer( proto );
    yyTok = getToken();
    CppFunction func = matchFunctionPrototype( TRUE );
    stopTokenizer();
    return func.prototype();
}
