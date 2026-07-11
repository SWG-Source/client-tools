/****************************************************************************
** $Id: moc.y 2051 2007-02-21 10:04:20Z chehrlic $
**
** Parser and code generator for meta object compiler
**
** Created : 930417
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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
** --------------------------------------------------------------------------
**
** This compiler reads a C++ header file with class definitions and ouputs
** C++ code to build a meta class. The meta data includes public methods
** (not constructors, destructors or operator functions), signals and slot
** definitions. The output file should be compiled and linked into the
** target application.
**
** C++ header files are assumed to have correct syntax, and we are therefore
** doing less strict checking than C++ compilers.
**
** The C++ grammar has been adopted from the "The Annotated C++ Reference
** Manual" (ARM), by Ellis and Stroustrup (Addison Wesley, 1992).
**
** Notice that this code is not possible to compile with GNU bison, instead
** use standard AT&T yacc or Berkeley yacc.
*****************************************************************************/

%{
#define MOC_YACC_CODE
void yyerror( const char *msg );

#include "qplatformdefs.h"
#include "qasciidict.h"
#include "qdatetime.h"
#include "qdict.h"
#include "qfile.h"
#include "qdir.h"
#include "qptrlist.h"
#include "qregexp.h"
#include "qstrlist.h"
#ifdef MOC_MWERKS_PLUGIN
# ifdef Q_OS_MACX
#  undef OLD_DEBUG
#  ifdef DEBUG
#   define OLD_DEBUG DEBUG
#   undef DEBUG
#  endif
#  define DEBUG 0
#  ifndef __IMAGECAPTURE__
#   define __IMAGECAPTURE__
#  endif
#  include <Carbon/Carbon.h>
# endif
# include "mwerks_mac.h"
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#if defined CONST
#undef CONST
#endif
#if defined VOID
#undef VOID
#endif

bool isEnumType( const char* type );
int enumIndex( const char* type );
bool isVariantType( const char* type );
int qvariant_nameToType( const char* name );
static void init();				// initialize
static void initClass();			// prepare for new class
static void generateClass();			// generate C++ code for class
static void initExpression();			// prepare for new expression
static void enterNameSpace( const char *name = 0 );
static void leaveNameSpace();
static void selectOutsideClassState();
static void registerClassInNamespace();
static bool suppress_func_warn = FALSE;
static void func_warn( const char *msg );
static void moc_warn( const char *msg );
static void moc_err( const char *s );
static void moc_err( const char *s1, const char *s2 );
static void operatorError();
static void checkPropertyName( const char* ident );

static const char* const utype_map[] =
{
    "bool",
    "int",
    "double",
    "QString",
    "QVariant",
    0
};

inline bool isIdentChar( char x )
{						// Avoid bug in isalnum
    return x == '_' || (x >= '0' && x <= '9') ||
	 (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z');
}

bool validUType( QCString ctype )
{
     if ( ctype.left(6) == "const " )
	ctype = ctype.mid( 6, ctype.length() - 6 );
    if ( ctype.right(1) == "&" )
	ctype = ctype.left( ctype.length() - 1 );
    else if ( ctype.right(1) == "*" )
	return TRUE;

    int i = -1;
    while ( utype_map[++i] )
	if ( ctype == utype_map[i] )
	    return TRUE;

    return isEnumType( ctype );
}

QCString castToUType( QCString ctype )
{
     if ( ctype.right(1) == "&" )
	 ctype = ctype.left( ctype.length() - 1 );
     if( ctype.right(1) == "]") {
	 int lb = ctype.findRev('[');
	 if(lb != -1)
	     ctype = ctype.left(lb) + "*";
     }
     return ctype;
}

QCString rawUType( QCString ctype )
{
    ctype = castToUType( ctype );
    if ( ctype.left(6) == "const " )
	ctype = ctype.mid( 6, ctype.length() - 6 );
    return ctype;
}

QCString uType( QCString ctype )
{
    if ( !validUType( ctype ) ) {
	if ( isVariantType( rawUType(ctype) ) )
	    return "varptr";
	else
	    return "ptr";
    }
    if ( ctype.left(6) == "const " )
	ctype = ctype.mid( 6, ctype.length() - 6 );
    if ( ctype.right(1) == "&" ) {
	ctype = ctype.left( ctype.length() - 1 );
    } else if ( ctype.right(1) == "*" ) {
	QCString raw = ctype.left( ctype.length() - 1 );
	ctype = "ptr";
	if ( raw == "char" )
	    ctype = "charstar";
	else if ( raw == "QUnknownInterface" )
	    ctype = "iface";
	else if ( raw == "QDispatchInterface" )
	    ctype = "idisp";
	else if ( isVariantType( raw ) )
	    ctype = "varptr";
    }
    if ( isEnumType( ctype ) )
	ctype = "enum";
    return ctype;
}

bool isInOut( QCString ctype )
{
    if ( ctype.left(6) == "const " )
	return FALSE;
    if ( ctype.right(1) == "&" )
	return TRUE;
    if ( ctype.right(2) == "**" )
	return TRUE;
    return FALSE;
}

QCString uTypeExtra( QCString ctype )
{
    QCString typeExtra = "0";
    if ( !validUType( ctype ) ) {
	if ( isVariantType( rawUType(ctype) ) )
	    typeExtra.sprintf("\"\\x%02x\"", qvariant_nameToType( rawUType(ctype) ) );
	else
	    typeExtra.sprintf( "\"%s\"", rawUType(ctype).data() );
	return typeExtra;
    }
    if ( ctype.left(6) == "const " )
	ctype = ctype.mid( 6, ctype.length() - 6 );
    if ( ctype.right(1) == "&" )
	ctype = ctype.left( ctype.length() - 1 );
    if ( ctype.right(1) == "*" ) {
	QCString raw = ctype.left( ctype.length() - 1 );
	ctype = "ptr";
	if ( raw == "char" )
	    ;
	else if ( isVariantType( raw ) )
	    typeExtra.sprintf("\"\\x%02x\"", qvariant_nameToType( raw ) );
	else
	    typeExtra.sprintf( "\"%s\"", raw.stripWhiteSpace().data() );

    } else if ( isEnumType( ctype ) ) {
	int idx = enumIndex( ctype );
	if ( idx >= 0 ) {
	    typeExtra.sprintf( "&enum_tbl[%d]", enumIndex( ctype ) );
	} else {
	    typeExtra.sprintf( "parentObject->enumerator(\"%s\", TRUE )", ctype.data() );
	}
	typeExtra =
	    "\n#ifndef QT_NO_PROPERTIES\n\t  " + typeExtra +
	    "\n#else"
	    "\n\t  0"
	    "\n#endif // QT_NO_PROPERTIES\n\t  ";
    }
    return typeExtra;
}

/*
  Attention!
  This table is copied from qvariant.cpp. If you change
  one, change both.
*/
static const int ntypes = 35;
static const char* const type_map[ntypes] =
{
    0,
    "QMap<QString,QVariant>",
    "QValueList<QVariant>",
    "QString",
    "QStringList",
    "QFont",
    "QPixmap",
    "QBrush",
    "QRect",
    "QSize",
    "QColor",
    "QPalette",
    "QColorGroup",
    "QIconSet",
    "QPoint",
    "QImage",
    "int",
    "uint",
    "bool",
    "double",
    "QCString",
    "QPointArray",
    "QRegion",
    "QBitmap",
    "QCursor",
    "QSizePolicy",
    "QDate",
    "QTime",
    "QDateTime",
    "QByteArray",
    "QBitArray",
    "QKeySequence",
    "QPen",
    "Q_LLONG",
    "Q_ULLONG"
};

int qvariant_nameToType( const char* name )
{
    for ( int i = 0; i < ntypes; i++ ) {
	if ( !qstrcmp( type_map[i], name ) )
	    return i;
    }
    return 0;
}

/*
  Returns TRUE if the type is a QVariant types.
*/
bool isVariantType( const char* type )
{
    return qvariant_nameToType( type ) != 0;
}

/*
  Replaces '>>' with '> >' (as in 'QValueList<QValueList<double> >').
  This function must be called to produce valid C++ code. However,
  the string representation still uses '>>'.
*/
void fixRightAngles( QCString *str )
{
    str->replace( QRegExp(">>"), "> >" );
}

static QCString rmWS( const char * );

enum Access { Private, Protected, Public };


class Argument					// single arg meta data
{
public:
    Argument( const char *left, const char *right, const char* argName = 0, bool isDefaultArgument = FALSE )
    {
	leftType = rmWS( left );
	rightType = rmWS( right );
	if ( leftType == "void" && rightType.isEmpty() )
	    leftType = "";

	int len = leftType.length();

	/*
	  Convert 'char const *' into 'const char *'. Start at index 1,
	  not 0, because 'const char *' is already OK.
	*/
	for ( int i = 1; i < len; i++ ) {
	    if ( leftType[i] == 'c' &&
		 strncmp(leftType.data() + i + 1, "onst", 4) == 0
                 && (i + 5 >= len || !isIdentChar(leftType[i + 5]))
                 && !isIdentChar(i-1)
                ) {
		leftType.remove( i, 5 );
		if ( leftType[i - 1] == ' ' )
		    leftType.remove( i - 1, 1 );
		leftType.prepend( "const " );
		break;
	    }

	    /*
	      We musn't convert 'char * const *' into 'const char **'
	      and we must beware of 'Bar<const Bla>'.
	    */
	    if ( leftType[i] == '&' || leftType[i] == '*' ||
		 leftType[i] == '<' )
		break;
	}

	name = argName;
	isDefault = isDefaultArgument;
    }

    QCString leftType;
    QCString rightType;
    QCString name;
    bool isDefault;
};

class ArgList : public QPtrList<Argument> {	// member function arg list
public:
    ArgList() { setAutoDelete( TRUE ); }
    ~ArgList() { clear(); }

    /* the clone has one default argument less, the orignal has all default arguments removed */
    ArgList* magicClone() {
	ArgList* l = new ArgList;
	bool firstDefault = FALSE;
	for ( first(); current(); next() ) {
	    bool isDefault = current()->isDefault;
	    if ( !firstDefault && isDefault ) {
		isDefault = FALSE;
		firstDefault = TRUE;
	    }
	    l->append( new Argument( current()->leftType, current()->rightType, current()->name, isDefault ) );
	}
	for ( first(); current(); ) {
	    if ( current()->isDefault )
		remove();
	    else
		next();
	}
	return l;
    }

    bool hasDefaultArguments() {
	for ( Argument* a = first(); a; a = next() ) {
	    if ( a->isDefault )
		return TRUE;
	}
	return FALSE;
    }

};


struct Function					// member function meta data
{
    Access access;
    QCString    qualifier;			// const or volatile
    QCString    name;
    QCString    type;
    QCString    signature;
    int	       lineNo;
    ArgList   *args;
    Function() { args=0;}
   ~Function() { delete args; }
    const char* accessAsString() {
	switch ( access ) {
	case Private: return "Private";
	case Protected: return "Protected";
	default: return "Public";
	}
    }
};

class FuncList : public QPtrList<Function> {	// list of member functions
public:
    FuncList( bool autoDelete = FALSE ) { setAutoDelete( autoDelete ); }

    FuncList find( const char* name )
    {
	FuncList result;
	for ( QPtrListIterator<Function> it(*this); it.current(); ++it ) {
	    if ( it.current()->name == name )
		result.append( it.current() );
	}
	return result;
    }
};

class Enum : public QStrList
{
public:
    QCString name;
    bool set;
};

class EnumList : public QPtrList<Enum> {		// list of property enums
public:
    EnumList() { setAutoDelete(TRUE); }
};


struct Property
{
    Property( int l, const char* t, const char* n, const char* s, const char* g, const char* r,
	      const QCString& st, const QCString& d, const QCString& sc, bool ov )
	: lineNo(l), type(t), name(n), set(s), get(g), reset(r), setfunc(0), getfunc(0),
	  sspec(Unspecified), gspec(Unspecified), stored( st ),
	  designable( d ), scriptable( sc ), override( ov ), oredEnum( -1 )
    {
	/*
	  The Q_PROPERTY construct cannot contain any commas, since
	  commas separate macro arguments. We therefore expect users
	  to type "QMap" instead of "QMap<QString, QVariant>". For
	  coherence, we also expect the same for
	  QValueList<QVariant>, the other template class supported by
	  QVariant.
	*/
	if ( type == "QMap" ) {
	    type = "QMap<QString,QVariant>";
	} else if ( type == "QValueList" ) {
	    type = "QValueList<QVariant>";
	} else if ( type == "LongLong" ) {
	    type = "Q_LLONG";
	} else if ( type == "ULongLong" ) {
	    type = "Q_ULLONG";
	}
    }

    int lineNo;
    QCString type;
    QCString name;
    QCString set;
    QCString get;
    QCString reset;
    QCString stored;
    QCString designable;
    QCString scriptable;
    bool override;

    Function* setfunc;
    Function* getfunc;

    int oredEnum; // If the enums item may be ored. That means the data type is int.
		  // Allowed values are 1 (True), 0 (False), and -1 (Unset)
    QCString enumsettype; // contains the set function type in case of oredEnum
    QCString enumgettype; // contains the get function type in case of oredEnum

    enum Specification  { Unspecified, Class, Reference, Pointer, ConstCharStar };
    Specification sspec;
    Specification gspec;

    bool stdSet() {
	QCString s = "set";
	s += toupper( name[0] );
	s += name.mid( 1 );
	return s == set;
    }

    static const char* specToString( Specification s )
    {
	switch ( s ) {
	case Class:
	    return "Class";
	case Reference:
	    return "Reference";
	case Pointer:
	    return "Pointer";
	case ConstCharStar:
	    return "ConstCharStar";
	default:
	    return "Unspecified";
	}
    }
};

class PropList : public QPtrList<Property> {	// list of properties
public:
    PropList() { setAutoDelete( TRUE ); }
};


struct ClassInfo
{
    ClassInfo( const char* n, const char* v )
	: name(n), value(v)
    {}
    QCString name;
    QCString value;
};

class ClassInfoList : public QPtrList<ClassInfo> {	// list of class infos
public:
    ClassInfoList() { setAutoDelete( TRUE ); }
};

class parser_reg {
 public:
    parser_reg();
    ~parser_reg();

    // some temporary values
    QCString   tmpExpression;			// Used to store the characters the lexer
						// is currently skipping (see addExpressionChar and friends)
    QCString  fileName;				// file name
    QCString  outputFile;				// output file name
    QCString  pchFile;				// name of PCH file (used on Windows)
    QStrList  includeFiles;			// name of #include files
    QCString  includePath;				// #include file path
    QCString  qtPath;				// #include qt file path
    int           gen_count; //number of classes generated
    bool	  noInclude;		// no #include <filename>
    bool	  generatedCode;		// no code generated
    bool	  mocError;			// moc parsing error occurred
    bool       hasVariantIncluded;	//whether or not qvariant.h was included yet
    QCString  className;				// name of parsed class
    QCString  superClassName;			// name of first super class
    QStrList  multipleSuperClasses;			// other superclasses
    FuncList  signals;				// signal interface
    FuncList  slots;				// slots interface
    FuncList  propfuncs;				// all possible property access functions
    FuncList  funcs;			// all parsed functions, including signals
    EnumList  enums;				// enums used in properties
    PropList  props;				// list of all properties
    ClassInfoList	infos;				// list of all class infos

// Used to store the values in the Q_PROPERTY macro
    QCString propWrite;				// set function
    QCString propRead;				// get function
    QCString propReset;				// reset function
    QCString propStored;				//
    QCString propDesignable;				// "true", "false" or function or empty if not specified
    QCString propScriptable;				// "true", "false" or function or empty if not specified
    bool propOverride;				// Wether OVERRIDE was detected

    QStrList qtEnums;				// Used to store the contents of Q_ENUMS
    QStrList qtSets;				// Used to store the contents of Q_SETS

};

static parser_reg *g = 0;

ArgList *addArg( Argument * );			// add arg to tmpArgList

enum Member { SignalMember,
	      SlotMember,
	      PropertyCandidateMember
	    };

void	 addMember( Member );			// add tmpFunc to current class
void     addEnum();				// add tmpEnum to current class

char	*stradd( const char *, const char * );	// add two strings
char	*stradd( const char *, const char *,	// add three strings
			       const char * );
char 	*stradd( const char *, const char *,	// adds 4 strings
		 const char *, const char * );

char	*straddSpc( const char *, const char * );
char	*straddSpc( const char *, const char *,
			       const char * );
char	*straddSpc( const char *, const char *,
		    const char *, const char * );

extern int yydebug;
bool	   lexDebug	   = FALSE;
int	   lineNo;			// current line number
bool	   errorControl	   = FALSE;	// controled errors
bool	   displayWarnings = TRUE;
bool	   skipClass;			// don't generate for class
bool	   skipFunc;			// don't generate for func
bool	   templateClass;		// class is a template
bool	   templateClassOld;		// previous class is a template

ArgList	  *tmpArgList;			// current argument list
Function  *tmpFunc;			// current member function
Enum      *tmpEnum;			// current enum
Access tmpAccess;			// current access permission
Access subClassPerm;			// current access permission

bool	   Q_OBJECTdetected;		// TRUE if current class
					//  contains the Q_OBJECT macro
bool	   Q_PROPERTYdetected;		// TRUE if current class
					//  contains at least one Q_PROPERTY,
					//  Q_OVERRIDE, Q_SETS or Q_ENUMS macro
bool	   tmpPropOverride;		// current property override setting

int	   tmpYYStart;			// Used to store the lexers current mode
int	   tmpYYStart2;			// Used to store the lexers current mode
					//  (if tmpYYStart is already used)

// if the format revision changes, you MUST change it in qmetaobject.h too
const int formatRevision = 26;		// moc output format revision

// if the flags change, you HAVE to change it in qmetaobject.h too
enum Flags  {
    Invalid		= 0x00000000,
    Readable		= 0x00000001,
    Writable		= 0x00000002,
    EnumOrSet		= 0x00000004,
    UnresolvedEnum	= 0x00000008,
    StdSet		= 0x00000100,
    Override		= 0x00000200,
    NotDesignable	= 0x00001000,
    DesignableOverride  = 0x00002000,
    NotScriptable	= 0x00004000,
    ScriptableOverride  = 0x00008000,
    NotStored 		= 0x00010000,
    StoredOverride 	= 0x00020000
};


#ifdef YYBISON
# if defined(Q_OS_WIN32)
# include <io.h>
# undef isatty
extern "C" int hack_isatty( int )
 {
     return 0;
 }
# define isatty hack_isatty
# else
# include <unistd.h>
# endif

# define YYDEBUG 1
# include "moc_yacc.h"
# include "moc_lex.cpp"
#endif //YYBISON
%}




%union {
    char	char_val;
    int		int_val;
    double	double_val;
    char       *string;
    Access	access;
    Function   *function;
    ArgList    *arg_list;
    Argument   *arg;
}

%start declaration_seq

%token <char_val>	CHAR_VAL		/* value types */
%token <int_val>	INT_VAL
%token <double_val>	DOUBLE_VAL
%token <string>		STRING
%token <string>		IDENTIFIER		/* identifier string */

%token			FRIEND			/* declaration keywords */
%token			TYPEDEF
%token			AUTO
%token			REGISTER
%token			STATIC
%token			EXTERN
%token			INLINE
%token			VIRTUAL
%token			CONST
%token			VOLATILE
%token			CHAR
%token			SHORT
%token			INT
%token			LONG
%token			SIGNED
%token			UNSIGNED
%token			FLOAT
%token			DOUBLE
%token			VOID
%token			ENUM
%token			CLASS
%token			STRUCT
%token			UNION
%token			ASM
%token			PRIVATE
%token			PROTECTED
%token			PUBLIC
%token			OPERATOR
%token			DBL_COLON
%token			TRIPLE_DOT
%token			TEMPLATE
%token			NAMESPACE
%token			USING
%token			MUTABLE
%token			THROW

%token			SIGNALS
%token			SLOTS
%token			Q_OBJECT
%token			Q_PROPERTY
%token			Q_OVERRIDE
%token			Q_CLASSINFO
%token			Q_ENUMS
%token			Q_SETS

%token			READ
%token			WRITE
%token			STORED
%token			DESIGNABLE
%token			SCRIPTABLE
%token			RESET

%type  <string>		class_name
%type  <string>		template_class_name
%type  <string>		template_spec
%type  <string>		opt_base_spec

%type  <string>		base_spec
%type  <string>		base_list
%type  <string>		qt_macro_name
%type  <string>		base_specifier
%type  <access>		access_specifier
%type  <string>		fct_name
%type  <string>		type_name
%type  <string>		simple_type_names
%type  <string>		simple_type_name
%type  <string>		class_key
%type  <string>		complete_class_name
%type  <string>		qualified_class_name
%type  <string>		elaborated_type_specifier
%type  <string>		whatever

%type  <arg_list>	argument_declaration_list
%type  <arg_list>	arg_declaration_list
%type  <arg_list>	arg_declaration_list_opt
%type  <string>		abstract_decl_opt
%type  <string>		abstract_decl
%type  <arg>		argument_declaration
%type  <arg>		opt_exception_argument
%type  <string>		cv_qualifier_list_opt
%type  <string>		cv_qualifier_list
%type  <string>		cv_qualifier
%type  <string>		decl_specifiers
%type  <string>		decl_specifier
%type  <string>		decl_specs_opt
%type  <string>		decl_specs
%type  <string>		type_specifier
%type  <string>		fct_specifier
%type  <string>		declarator
%type  <string>		ptr_operator
%type  <string>		ptr_operators
%type  <string>		ptr_operators_opt

%type  <string>		dname

%%
declaration_seq:	  /* empty */
			| declaration_seq declaration
			;

declaration:		  class_def
/* | template_declaration */
			| namespace_def
			| namespace_alias_def
			| using_declaration
			| using_directive
			;

namespace_def:            named_namespace_def
			| unnamed_namespace_def
			;

named_namespace_def:      NAMESPACE
			  IDENTIFIER         { enterNameSpace($2); }
			  '{'                { BEGIN IN_NAMESPACE; }
			  namespace_body
			  '}'                { leaveNameSpace();
					       selectOutsideClassState();
					     }
			;

unnamed_namespace_def:    NAMESPACE          { enterNameSpace(); }
			  '{'                { BEGIN IN_NAMESPACE; }
			  namespace_body
			  '}'                { leaveNameSpace();
					       selectOutsideClassState();
					     }
			;

namespace_body:           declaration_seq
			;

namespace_alias_def:      NAMESPACE IDENTIFIER '=' complete_class_name ';'
					    { selectOutsideClassState(); }
			;


using_directive:          USING NAMESPACE   { selectOutsideClassState(); } /* Skip namespace */
			;

using_declaration:        USING IDENTIFIER  { selectOutsideClassState(); }
			| USING DBL_COLON   { selectOutsideClassState(); }
			;

class_def:				      { initClass(); }
			  class_specifier ';' { generateClass();
						registerClassInNamespace();
						selectOutsideClassState(); }
			;


/***** r.17.1 (ARM p.387 ): Keywords	*****/

class_name:		  IDENTIFIER	      { $$ = $1; }
			| template_class_name { $$ = $1; }
			;

template_class_name:	  IDENTIFIER '<' template_args '>'
				   { g->tmpExpression = rmWS( g->tmpExpression );
				     $$ = stradd( $1, "<",
						  g->tmpExpression, ">" ); }
			;

/*
   template_args skips all characters until it encounters a ">" (it
   handles and discards sublevels of parentheses).  Since the rule is
   empty it must be used with care!
*/

template_args:		  /* empty */		  { initExpression();
						    templLevel = 1;
						    BEGIN IN_TEMPL_ARGS; }
			;

/***** r.17.2 (ARM p.388): Expressions	*****/


/* const_expression skips all characters until it encounters either one
   of "]", ")" or "," (it handles and discards sublevels of parentheses).
   Since the rule is empty it must be used with care!
*/

const_expression:	   /* empty */		  { initExpression();
						    BEGIN IN_EXPR; }
			;

/* def_argument is just like const expression but handles the ","
   slightly differently. It was added for 3.0 as quick solution. TODO:
   merge with const_expression.
 */

def_argument:	   /* empty */		  { BEGIN IN_DEF_ARG; }
			;

enumerator_expression:	   /* empty */		  { initExpression();
						    BEGIN IN_ENUM; }
			;

/***** r.17.3 (ARM p.391): Declarations *****/

decl_specifier:		  storage_class_specifier { $$ = ""; }
			| type_specifier	  { $$ = $1; }
			| fct_specifier		  { $$ = ""; }
			| FRIEND		  { skipFunc = TRUE; $$ = ""; }
			| TYPEDEF		  { skipFunc = TRUE; $$ = ""; }
			;

decl_specifiers:	  decl_specs_opt type_name decl_specs_opt
						  { $$ = straddSpc($1,$2,$3); }
			;
decl_specs_opt:			/* empty */	  { $$ = ""; }
			| decl_specs		  { $$ = $1; }
			;

decl_specs:		  decl_specifier	    { $$ = $1; }
			| decl_specs decl_specifier { $$ = straddSpc($1,$2); }
			;

storage_class_specifier:  AUTO
			| REGISTER
			| STATIC		{ skipFunc = TRUE; }
			| EXTERN
			;

fct_specifier:		  INLINE                { }
			| VIRTUAL               { }
			;

type_specifier:		  CONST			{ $$ = "const"; }
			| VOLATILE		{ $$ = "volatile"; }
			;

type_name:		  elaborated_type_specifier { $$ = $1; }
			| complete_class_name	    { $$ = $1; }
			| simple_type_names	    { $$ = $1; }
			;

simple_type_names:	  simple_type_names simple_type_name
						    { $$ = straddSpc($1,$2); }
			| simple_type_name	    { $$ = $1; }
                        ;

simple_type_name:	  CHAR			    { $$ = "char"; }
			| SHORT			    { $$ = "short"; }
			| INT			    { $$ = "int"; }
			| LONG			    { $$ = "long"; }
			| SIGNED		    { $$ = "signed"; }
			| UNSIGNED		    { $$ = "unsigned"; }
			| FLOAT			    { $$ = "float"; }
			| DOUBLE		    { $$ = "double"; }
			| VOID			    { $$ = "void"; }
			;

template_spec:		  TEMPLATE '<' template_args '>'
				   { g->tmpExpression = rmWS( g->tmpExpression );
				     $$ = stradd( "template<",
						  g->tmpExpression, ">" ); }
			;

opt_template_spec:	  /* empty */
			| template_spec		{ templateClassOld = templateClass;
						  templateClass = TRUE;
						}
			;


class_key:		  opt_template_spec CLASS	{ $$ = "class"; }
			| opt_template_spec STRUCT	{ $$ = "struct"; }
			;

complete_class_name:	  qualified_class_name	{ $$ = $1; }
			| DBL_COLON  qualified_class_name
						{ $$ = stradd( "::", $2 ); }
			;

qualified_class_name:	  qualified_class_name DBL_COLON class_name
						{ $$ = stradd( $1, "::", $3 );}
			| class_name		{ $$ = $1; }
			;

elaborated_type_specifier:
			  class_key IDENTIFIER	{ $$ = straddSpc($1,$2); }
			| ENUM IDENTIFIER	{ $$ = stradd("enum ",$2); }
			| UNION IDENTIFIER	{ $$ = stradd("union ",$2); }
			;

/***** r.17.4 (ARM p.393): Declarators	*****/

argument_declaration_list:  arg_declaration_list_opt triple_dot_opt { $$ = $1;}
			|   arg_declaration_list ',' TRIPLE_DOT	    { $$ = $1;
				       func_warn("Ellipsis not supported"
						 " in signals and slots.\n"
						 "Ellipsis argument ignored."); }
			;

arg_declaration_list_opt:	/* empty */	{ $$ = tmpArgList; }
			| arg_declaration_list	{ $$ = $1; }
			;

opt_exception_argument:		/* empty */     { $$ = 0; }
			| argument_declaration
			;

triple_dot_opt:			/* empty */
			| TRIPLE_DOT { func_warn("Ellipsis not supported"
						 " in signals and slots.\n"
						 "Ellipsis argument ignored."); }

			;

arg_declaration_list:	  arg_declaration_list
			  ','
			  argument_declaration	{ $$ = addArg($3); }
			| argument_declaration	{ $$ = addArg($1); }
			;

argument_declaration:	  decl_specifiers abstract_decl_opt
				{ $$ = new Argument(straddSpc($1,$2),""); }
			| decl_specifiers abstract_decl_opt
			  '=' { expLevel = 1; }
			  def_argument
				{ $$ = new Argument(straddSpc($1,$2),"", 0, TRUE ); }
			| decl_specifiers abstract_decl_opt dname
				abstract_decl_opt
				{ $$ = new Argument(straddSpc($1,$2),$4, $3); }
			| decl_specifiers abstract_decl_opt dname
				abstract_decl_opt
			  '='	{ expLevel = 1; }
			  def_argument
				{ $$ = new Argument(straddSpc($1,$2),$4, $3, TRUE); }
			;


abstract_decl_opt:	  /* empty */		{ $$ = ""; }
			| abstract_decl		{ $$ = $1; }
			;

abstract_decl:		  abstract_decl ptr_operator
						{ $$ = straddSpc($1,$2); }
			| '['			{ expLevel = 1; }
			  const_expression ']'
				   { $$ = stradd( "[",
				     g->tmpExpression =
				     g->tmpExpression.stripWhiteSpace(), "]" ); }
			| abstract_decl '['	{ expLevel = 1; }
			  const_expression ']'
				   { $$ = stradd( $1,"[",
				     g->tmpExpression =
				     g->tmpExpression.stripWhiteSpace(),"]" ); }
			| ptr_operator		{ $$ = $1; }
			| '(' abstract_decl ')' { $$ = $2; }
			;

declarator:		  dname			{ $$ = ""; }
			| declarator ptr_operator
						{ $$ = straddSpc($1,$2);}
			| declarator '['	{ expLevel = 1; }
			  const_expression ']'
				   { $$ = stradd( $1,"[",
				     g->tmpExpression =
				     g->tmpExpression.stripWhiteSpace(),"]" ); }
			| '(' declarator ')'	{ $$ = $2; }
			;

dname:			  IDENTIFIER
			;

fct_decl:		  '('
			  argument_declaration_list
			  ')'
			  cv_qualifier_list_opt
			  ctor_initializer_opt
			  exception_spec_opt
                          opt_identifier
			  fct_body_or_semicolon
						{ tmpFunc->args	     = $2;
						  tmpFunc->qualifier = $4; }
			;

fct_name:		  IDENTIFIER		/* NOTE: simplified! */
			| IDENTIFIER array_decls
				{ func_warn("Variable as signal or slot."); }
			| IDENTIFIER '=' { expLevel=0; }
			  const_expression     /* probably const member */
						{ skipFunc = TRUE; }
			| IDENTIFIER array_decls '=' { expLevel=0; }
			  const_expression     /* probably const member */
						{ skipFunc = TRUE; }
			;


array_decls:		 '['			{ expLevel = 1; }
			  const_expression ']'
			| array_decls '['	 { expLevel = 1; }
			  const_expression ']'

			;

ptr_operators_opt:	   /* empty */		{ $$ = ""; }
			| ptr_operators		 { $$ = $1; }
			;

ptr_operators:		  ptr_operator		{ $$ = $1; }
			| ptr_operators ptr_operator { $$ = straddSpc($1,$2);}
			;

ptr_operator:		  '*' cv_qualifier_list_opt { $$ = straddSpc("*",$2);}
			| '&' cv_qualifier_list_opt { $$ = stradd("&",$2);}
/*!			| complete_class_name
			  DBL_COLON
			  '*'
			  cv_qualifier_list_opt { $$ = stradd($1,"::*",$4); }*/
			;

cv_qualifier_list_opt:		/* empty */	{ $$ = ""; }
			| cv_qualifier_list	{ $$ = $1; }
			;

cv_qualifier_list:	  cv_qualifier		{ $$ = $1; }
			| cv_qualifier_list cv_qualifier
						{ $$ = straddSpc($1,$2); }
			;

cv_qualifier:		  CONST			{ $$ = "const"; }
			| VOLATILE		{ $$ = "volatile"; }
			;

fct_body_or_semicolon:	  ';'
			| fct_body
			| '=' INT_VAL ';'   /* abstract func, INT_VAL = 0 */
			;

fct_body:		  '{' { BEGIN IN_FCT; fctLevel = 1;}
			  '}' { BEGIN QT_DEF; }
			;


/***** r.17.5 (ARM p.395): Class Declarations *****/

class_specifier:	  full_class_head
			  '{'			{ BEGIN IN_CLASS;
						  classPLevel = 1;
						}
			  opt_obj_member_list
			  '}'			{ BEGIN QT_DEF; } /*catch ';'*/
			| class_head		{ BEGIN QT_DEF;	  /* -- " -- */
						  skipClass = TRUE; }
 			| class_head '*' IDENTIFIER	{ BEGIN QT_DEF;	  /* -- " -- */
 						  skipClass = TRUE; }
 			| class_head '&' IDENTIFIER	{ BEGIN QT_DEF;	  /* -- " -- */
 						  skipClass = TRUE; }
			| class_head
			  '(' IDENTIFIER ')' /* Qt macro name */
						{ BEGIN QT_DEF; /* catch ';' */
						  skipClass = TRUE; }
			| template_spec whatever { skipClass = TRUE;
						  BEGIN GIMME_SEMICOLON; }
			;

whatever:		  IDENTIFIER
			| simple_type_name
			| type_specifier
			| storage_class_specifier { $$ = ""; }
			| fct_specifier
			;


class_head:		  class_key
			  qualified_class_name	{ g->className = $2;
						  if ( g->className == "QObject" )
						     Q_OBJECTdetected = TRUE;
						}
			| class_key
			  IDENTIFIER		/* possible DLL EXPORT macro */
			  class_name		{ g->className = $3;
						  if ( g->className == "QObject" )
						     Q_OBJECTdetected = TRUE;
						}
			;

full_class_head:	  class_head
			  opt_base_spec		{ g->superClassName = $2; }
			;

nested_class_head:	  class_key
			  qualified_class_name
			  opt_base_spec		{ templateClass = templateClassOld; }
                        ;

exception_spec_opt:		/* empty */
			| exception_spec
			;

/* looser than the real thing */
exception_spec:		THROW '(' opt_exception_argument ')'
			;

ctor_initializer_opt:		/* empty */
			| ctor_initializer
			;

ctor_initializer:	':' mem_initializer_list
			;

mem_initializer_list:	mem_initializer
			| mem_initializer ',' mem_initializer_list
			;

/* complete_class_name also represents IDENTIFIER */
mem_initializer:	complete_class_name '('	{ expLevel = 1; }
			const_expression ')'
			;


opt_base_spec:			/* empty */	{ $$ = 0; }
			| base_spec		{ $$ = $1; }
			;

opt_obj_member_list:		/* empty */
			| obj_member_list
			;

obj_member_list:	  obj_member_list obj_member_area
			| obj_member_area
			;


qt_access_specifier:	  access_specifier	{ tmpAccess = $1; }
			| SLOTS	      { moc_err( "Missing access specifier"
						   " before \"slots:\"." ); }
			;

obj_member_area:	  qt_access_specifier	{ BEGIN QT_DEF; }
			  slot_area
			| SIGNALS		{ BEGIN QT_DEF; }
			  ':'  opt_signal_declarations
			| Q_OBJECT		{
			      if ( tmpAccess )
				  moc_warn("Q_OBJECT is not in the private"
					   " section of the class.\n"
					   "Q_OBJECT is a macro that resets"
					   " access permission to \"private\".");
			      Q_OBJECTdetected = TRUE;
			  }
			| Q_PROPERTY { tmpYYStart = YY_START;
				       tmpPropOverride = FALSE;
				       BEGIN IN_PROPERTY; }
			  '(' property ')' {
						BEGIN tmpYYStart;
					   }
			  opt_property_candidates
			| Q_OVERRIDE { tmpYYStart = YY_START;
				       tmpPropOverride = TRUE;
				       BEGIN IN_PROPERTY; }
			  '(' property ')' {
						BEGIN tmpYYStart;
					   }
			  opt_property_candidates
			| Q_CLASSINFO { tmpYYStart = YY_START; BEGIN IN_CLASSINFO; }
			  '(' STRING ',' STRING ')'
				  {
				      g->infos.append( new ClassInfo( $4, $6 ) );
				      BEGIN tmpYYStart;
				  }
			  opt_property_candidates
			| Q_ENUMS { tmpYYStart = YY_START; BEGIN IN_PROPERTY; }
			  '(' qt_enums ')' {
						Q_PROPERTYdetected = TRUE;
						BEGIN tmpYYStart;
					   }
			  opt_property_candidates
			| Q_SETS { tmpYYStart = YY_START; BEGIN IN_PROPERTY; }
			  '(' qt_sets ')' {
						Q_PROPERTYdetected = TRUE;
						BEGIN tmpYYStart;
					   }
			  opt_property_candidates
			;

slot_area:		  SIGNALS ':'	{ moc_err( "Signals cannot "
						 "have access specifiers" ); }
			| SLOTS	  ':' opt_slot_declarations
			| ':'		{ if ( tmpAccess == Public && Q_PROPERTYdetected )
						  BEGIN QT_DEF;
					      else
						  BEGIN IN_CLASS;
					  suppress_func_warn = TRUE;
					}
			  opt_property_candidates
					{
					  suppress_func_warn = FALSE;
					}
			| IDENTIFIER	{ BEGIN IN_CLASS;
					   if ( classPLevel != 1 )
					       moc_warn( "unexpected access"
							 "specifier" );
					}
			;

opt_property_candidates:	  /*empty*/
				| property_candidate_declarations
			;

property_candidate_declarations:	  property_candidate_declarations property_candidate_declaration
					| property_candidate_declaration
				;

property_candidate_declaration:	signal_or_slot { addMember( PropertyCandidateMember ); }
				;

opt_signal_declarations:	/* empty */
			| signal_declarations
			;

signal_declarations:	  signal_declarations signal_declaration
			| signal_declaration
			;


signal_declaration:	  signal_or_slot	{ addMember( SignalMember ); }
			;

opt_slot_declarations:		/* empty */
			| slot_declarations
			;

slot_declarations:	  slot_declarations slot_declaration
			| slot_declaration
			;

slot_declaration:	  signal_or_slot	{ addMember( SlotMember ); }
			;

opt_semicolons:			/* empty */
			| opt_semicolons ';'
			;

base_spec:		  ':' base_list		{ $$=$2; }
			;

base_list		: base_list ',' base_specifier  { g->multipleSuperClasses.append( $3 ); }
			| base_specifier
			;

qt_macro_name:		  IDENTIFIER '(' IDENTIFIER ')'
					   { $$ = stradd( $1, "(", $3, ")" ); }
			| IDENTIFIER '(' simple_type_name ')'
					   { $$ = stradd( $1, "(", $3, ")" ); }
			;

base_specifier:		  complete_class_name			       {$$=$1;}
			| VIRTUAL access_specifier complete_class_name {$$=$3;}
			| VIRTUAL complete_class_name		       {$$=$2;}
			| access_specifier VIRTUAL complete_class_name {$$=$3;}
			| access_specifier complete_class_name	       {$$=$2;}
			| qt_macro_name				       {$$=$1;}
			| VIRTUAL access_specifier qt_macro_name       {$$=$3;}
			| VIRTUAL qt_macro_name			       {$$=$2;}
			| access_specifier VIRTUAL qt_macro_name       {$$=$3;}
			| access_specifier qt_macro_name	       {$$=$2;}
			;

access_specifier:	  PRIVATE		{ $$=Private; }
			| PROTECTED		{ $$=Protected; }
			| PUBLIC		{ $$=Public; }
			;

operator_name:		  decl_specs_opt IDENTIFIER ptr_operators_opt { }
			| decl_specs_opt simple_type_name ptr_operators_opt { }
			| '+'
			| '-'
			| '*'
			| '/'
			| '%'
			| '^'
			| '&'
			| '|'
			| '~'
			| '!'
			| '='
			| '<'
			| '>'
			| '+' '='
			| '-' '='
			| '*' '='
			| '/' '='
			| '%' '='
			| '^' '='
			| '&' '='
			| '|' '='
			| '~' '='
			| '!' '='
			| '=' '='
			| '<' '='
			| '>' '='
			| '<' '<'
			| '>' '>'
			| '<' '<' '='
			| '>' '>' '='
			| '&' '&'
			| '|' '|'
			| '+' '+'
			| '-' '-'
			| ','
			| '-' '>' '*'
			| '-' '>'
			| '(' ')'
			| '[' ']'
			;


opt_virtual:		  /* empty */
			| VIRTUAL
			;

type_and_name:		  type_name fct_name
						{ tmpFunc->type = $1;
						  tmpFunc->name = $2; }
			| fct_name
						{ tmpFunc->type = "int";
						  tmpFunc->name = $1;
				  if ( tmpFunc->name == g->className )
				      func_warn( "Constructors cannot be"
						 " signals or slots.");
						}
			| opt_virtual '~' fct_name
						{ tmpFunc->type = "void";
						  tmpFunc->name = "~";
						  tmpFunc->name += $3;
				       func_warn( "Destructors cannot be"
						  " signals or slots.");
						}
			| decl_specs type_name decl_specs_opt
			  ptr_operators_opt fct_name
						{
						    char *tmp =
							straddSpc($1,$2,$3,$4);
						    tmpFunc->type = rmWS(tmp);
						    delete [] tmp;
						    tmpFunc->name = $5; }
			| decl_specs type_name /* probably friend decl */
						{ skipFunc = TRUE; }
			| type_name ptr_operators fct_name
						{ tmpFunc->type =
						      straddSpc($1,$2);
						  tmpFunc->name = $3; }
			| type_name decl_specs ptr_operators_opt
			  fct_name
						{ tmpFunc->type =
						      straddSpc($1,$2,$3);
						  tmpFunc->name = $4; }
			| type_name OPERATOR operator_name
						{ operatorError();    }
			| OPERATOR operator_name
						{ operatorError();    }
			| decl_specs type_name decl_specs_opt
			  ptr_operators_opt OPERATOR operator_name
						{ operatorError();    }
			| type_name ptr_operators OPERATOR  operator_name
						{ operatorError();    }
			| type_name decl_specs ptr_operators_opt
			  OPERATOR  operator_name
						{ operatorError();    }
			;


signal_or_slot:		type_and_name fct_decl opt_semicolons
			| type_and_name opt_bitfield ';' opt_semicolons
				{ func_warn("Unexpected variable declaration."); }
			| type_and_name opt_bitfield ','member_declarator_list
			  ';' opt_semicolons
				{ func_warn("Unexpected variable declaration."); }
			| enum_specifier opt_identifier ';' opt_semicolons
				{ func_warn("Unexpected enum declaration."); }
			| USING complete_class_name ';' opt_semicolons
				{ func_warn("Unexpected using declaration."); }
			| USING NAMESPACE complete_class_name ';' opt_semicolons
				{ func_warn("Unexpected using declaration."); }
			| NAMESPACE IDENTIFIER '{'
				{ classPLevel++;
				  moc_err("Unexpected namespace declaration."); }
			| nested_class_head ';' opt_semicolons
				{ func_warn("Unexpected class declaration.");}
			| nested_class_head
			  '{'   { func_warn("Unexpected class declaration.");
				  BEGIN IN_FCT; fctLevel=1;
				}
			  '}'  { BEGIN QT_DEF; }
			  ';' opt_semicolons
			;


member_declarator_list:	  member_declarator
			| member_declarator_list ',' member_declarator
			;

member_declarator:	  declarator             { }
			| IDENTIFIER ':'	 { expLevel = 0; }
			  const_expression
			| ':'			 { expLevel = 0; }
			  const_expression
			;

opt_bitfield:		  /* empty */
			| ':'			 { expLevel = 0; }
			  const_expression
			;


enum_specifier:		  ENUM enum_tail
			;

/* optional final comma at the end of an enum list. Not really C++ but
some people still use it */
opt_komma:		  /*empty*/
			| ','
			;

enum_tail:		  IDENTIFIER '{'   enum_list opt_komma
			  '}'   { BEGIN QT_DEF;
				  if ( tmpAccess == Public) {
				      tmpEnum->name = $1;
				      addEnum();
				  }
				}
			| '{'   enum_list opt_komma
			  '}'   { tmpEnum->clear();}
			;

opt_identifier:		  /* empty */
			| IDENTIFIER 		 { }
			;

enum_list:		  /* empty */
			| enumerator
			| enum_list ',' enumerator
			;

enumerator:		  IDENTIFIER { if ( tmpAccess == Public) tmpEnum->append( $1 ); }
			| IDENTIFIER '=' { enumLevel=0; }
			  enumerator_expression {  if ( tmpAccess == Public) tmpEnum->append( $1 );  }
			;

property:		IDENTIFIER IDENTIFIER
				{
				     g->propWrite = "";
				     g->propRead = "";
				     g->propOverride = tmpPropOverride;
				     g->propReset = "";
				     if ( g->propOverride ) {
					 g->propStored = "";
					 g->propDesignable = "";
					 g->propScriptable = "";
				     } else {
					 g->propStored = "true";
					 g->propDesignable = "true";
					 g->propScriptable = "true";
				     }
				}
			prop_statements
				{
				    if ( g->propRead.isEmpty() && !g->propOverride )
					moc_err( "A property must at least feature a read method." );
				    checkPropertyName( $2 );
				    Q_PROPERTYdetected = TRUE;
				    // Avoid duplicates
				    for( QPtrListIterator<Property> lit( g->props ); lit.current(); ++lit ) {
					if ( lit.current()->name == $2 ) {
					    if ( displayWarnings )
						moc_err( "Property '%s' defined twice.",
							 (const char*)lit.current()->name );
					}
				    }
				    g->props.append( new Property( lineNo, $1, $2,
								g->propWrite, g->propRead, g->propReset,
								   g->propStored, g->propDesignable,
								   g->propScriptable, g->propOverride ) );
				}
			;

prop_statements:	  /* empty */
			| READ IDENTIFIER prop_statements { g->propRead = $2; }
			| WRITE IDENTIFIER prop_statements { g->propWrite = $2; }
			| RESET IDENTIFIER prop_statements { g->propReset = $2; }
			| STORED IDENTIFIER prop_statements { g->propStored = $2; }
			| DESIGNABLE IDENTIFIER prop_statements { g->propDesignable = $2; }
			| SCRIPTABLE IDENTIFIER prop_statements { g->propScriptable = $2; }
			;

qt_enums:		  /* empty */ { }
			| IDENTIFIER qt_enums { g->qtEnums.append( $1 ); }
			;

qt_sets:		  /* empty */ { }
			| IDENTIFIER qt_sets { g->qtSets.append( $1 ); }
			;

%%

#ifndef YYBISON
# if defined(Q_OS_WIN32)
# include <io.h>
# undef isatty
extern "C" int hack_isatty( int )
{
    return 0;
}
# define isatty hack_isatty
# else
# include <unistd.h>
# endif
# include "moc_lex.cpp"
#endif //YYBISON

void      cleanup();
QCString  combinePath( const char *, const char * );

FILE  *out;					// output file

parser_reg::parser_reg() : funcs(TRUE)
{
    gen_count = 0;
    noInclude     = FALSE;		// no #include <filename>
    generatedCode = FALSE;		// no code generated
    mocError = FALSE;			// moc parsing error occurred
    hasVariantIncluded = FALSE;
}


parser_reg::~parser_reg()
{
    slots.clear();
    signals.clear();
    propfuncs.clear();
    funcs.clear();
    infos.clear();
    props.clear();
    infos.clear();
}

int yyparse();

void replace( char *s, char c1, char c2 );

void setDefaultIncludeFile()
{
    if ( g->includeFiles.isEmpty() ) {
	if ( g->includePath.isEmpty() ) {
	    if ( !g->fileName.isEmpty() && !g->outputFile.isEmpty() ) {
		g->includeFiles.append( combinePath(g->fileName, g->outputFile) );
	    } else {
		g->includeFiles.append( g->fileName );
	    }
	} else {
	    g->includeFiles.append( combinePath(g->fileName, g->fileName) );
	}
    }
}

#ifdef Q_CC_MSVC
#define ErrorFormatString "%s(%d):"
#else
#define ErrorFormatString "%s:%d:"
#endif

#ifndef MOC_MWERKS_PLUGIN
int main( int argc, char **argv )
{
    init();

    bool autoInclude = TRUE;
    const char *error	     = 0;
    g->qtPath = "";
    for ( int n=1; n<argc && error==0; n++ ) {
	QCString arg = argv[n];
	if ( arg[0] == '-' ) {			// option
	    QCString opt = &arg[1];
	    if ( opt[0] == 'o' ) {		// output redirection
		if ( opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing output file name";
			break;
		    }
		    g->outputFile = argv[++n];
		} else
		    g->outputFile = &opt[1];
	    } else if ( opt == "i" ) {		// no #include statement
		g->noInclude   = TRUE;
		autoInclude = FALSE;
	    } else if ( opt[0] == 'f' ) {	// produce #include statement
		g->noInclude   = FALSE;
		autoInclude = FALSE;
		if ( opt[1] )			// -fsomething.h
		    g->includeFiles.append( &opt[1] );
	    } else if ( opt == "pch" ) {	// produce #include statement for PCH
		if ( !(n < argc-1) ) {
		    error = "Missing name of PCH file";
		    break;
		}
		g->pchFile = argv[++n];
	    } else if ( opt[0] == 'p' ) {	// include file path
		if ( opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing path name for the -p option.";
			break;
		    }
		    g->includePath = argv[++n];
		} else {
		    g->includePath = &opt[1];
		}
	    } else if ( opt[0] == 'q' ) {	// qt include file path
		if ( opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing path name for the -q option.";
			break;
		    }
		    g->qtPath = argv[++n];
		} else {
		    g->qtPath = &opt[1];
		}
		replace(g->qtPath.data(),'\\','/');
		if ( g->qtPath.right(1) != "/" )
		    g->qtPath += '/';
	    } else if ( opt == "v" ) {		// version number
		fprintf( stderr, "Qt Meta Object Compiler version %d"
				 " (Qt %s)\n", formatRevision,
				 QT_VERSION_STR );
		cleanup();
		return 1;
	    } else if ( opt == "k" ) {		// stop on errors
		errorControl = TRUE;
	    } else if ( opt == "nw" ) {		// don't display warnings
		displayWarnings = FALSE;
	    } else if ( opt == "ldbg" ) {	// lex debug output
		lexDebug = TRUE;
	    } else if ( opt == "ydbg" ) {	// yacc debug output
		yydebug = TRUE;
	    } else {
		error = "Invalid argument";
	    }
	} else {
	    if ( !g->fileName.isNull() )		// can handle only one file
		error = "Too many input files specified";
	    else
		g->fileName = arg.copy();
	}
    }

    if ( autoInclude ) {
	int ppos = g->fileName.findRev('.');
	if ( ppos != -1 && tolower( g->fileName[ppos + 1] ) == 'h' )
	    g->noInclude = FALSE;
	else
	    g->noInclude = TRUE;
    }
    setDefaultIncludeFile();

    if ( g->fileName.isNull() && !error ) {
	g->fileName = "standard input";
	yyin	 = stdin;
    } else if ( argc < 2 || error ) {		// incomplete/wrong args
	fprintf( stderr, "Qt meta object compiler\n" );
	if ( error )
	    fprintf( stderr, "moc: %s\n", error );
	fprintf( stderr, "Usage:  moc [options] <header-file>\n"
		 "\t-o file    Write output to file rather than stdout\n"
		 "\t-f[file]   Force #include, optional file name\n"
		 "\t-p path    Path prefix for included file\n"
		 "\t-i         Do not generate an #include statement\n"
		 "\t-k         Do not stop on errors\n"
		 "\t-nw        Do not display warnings\n"
		 "\t-v         Display version of moc\n" );
	cleanup();
	return 1;
    } else {
	yyin = fopen( (const char *)g->fileName, "r" );
	if ( !yyin ) {
	    fprintf( stderr, "moc: %s: No such file\n", (const char*)g->fileName);
	    cleanup();
	    return 1;
	}
    }
    if ( !g->outputFile.isEmpty() ) {		// output file specified
	out = fopen( (const char *)g->outputFile, "w" );	// create output file
	if ( !out ) {
	    fprintf( stderr, "moc: Cannot create %s\n",
		     (const char*)g->outputFile );
	    cleanup();
	    return 1;
	}
    } else {					// use stdout
	out = stdout;
    }
    yyparse();
    fclose( yyin );
    if ( !g->outputFile.isNull() )
	fclose( out );

    if ( !g->generatedCode && displayWarnings && !g->mocError ) {
	fprintf( stderr, ErrorFormatString" Warning: %s\n", g->fileName.data(), 0,
		 "No relevant classes found. No output generated." );
    }

    int ret = g->mocError ? 1 : 0;
    cleanup();
    return ret;
}
#else
bool qt_is_gui_used = FALSE;
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#ifdef Q_OS_MAC9
# include <Files.h>
# include <Strings.h>
# include <Errors.h>
# include "Aliases.h"
#endif
#include "CWPluginErrors.h"
#include <CWPlugins.h>
#include "DropInCompilerLinker.h"
#include <stat.h>

const unsigned char *p_str(const char *, int =-1);

CWPluginContext g_ctx;

moc_status do_moc( CWPluginContext ctx, const QCString &fin, const QCString &fout, CWFileSpec *dspec, bool i)
{
    init();

    g_ctx = ctx;
    g->noInclude = i;
    g->fileName = fin;
    g->outputFile = fout;

    setDefaultIncludeFile();

    CWFileInfo fi;
    memset(&fi, 0, sizeof(fi));
	fi.fullsearch = TRUE;
	fi.dependencyType = cwNormalDependency;
	fi.isdependentoffile = kCurrentCompiledFile;
    if(CWFindAndLoadFile( ctx, fin.data(), &fi) != cwNoErr) {
	cleanup();
	return moc_no_source;
    }

    if(dspec) {
	memcpy(dspec, &fi.filespec, sizeof(fi.filespec));
	const unsigned char *f = p_str(fout.data());
	memcpy(dspec->name, f, f[0]+1);
	free(f);
    }
    buf_size_total = fi.filedatalength;
    buf_buffer = fi.filedata;

    QCString path("");
    AliasHandle alias;
    Str63 str;
    AliasInfoType x = 1;
    char tmp[sizeof(Str63)+2];
    if(NewAlias( NULL, &fi.filespec, &alias) != noErr) {
	cleanup();
	return moc_general_error;
    }
    for(;;) {
	 GetAliasInfo(alias, x++, str);
	 if(!str[0])
	    break;
	 strncpy((char *)tmp, (const char *)str+1, str[0]);
	 tmp[str[0]] = '\0';
	 path.prepend(":");
	 path.prepend((char *)tmp);
    }
    path.prepend("MacOS 9:"); //FIXME

    QString inpath = path + fin, outpath = path + fout;
    struct stat istat, ostat;
    if(stat(inpath, &istat) == -1) {
	cleanup();
	return moc_no_source;
    }
    if(stat(outpath, &ostat) == 0 && istat.st_mtime < ostat.st_mtime) {
	cleanup();
	return moc_not_time;
    }

    unlink(outpath.data());
    out = fopen(outpath.data(), "w+");
    if(!out) {
	cleanup();
	return moc_general_error;
    }

    yyparse();
    if(out != stdout)
      fclose(out);

   if(g->mocError || !g->generatedCode) {
	unlink(outpath.data());
	moc_status ret = !g->generatedCode ? moc_no_qobject : moc_parse_error;
	cleanup();
	return ret;
    }

    cleanup();
    return moc_success;
}
#endif
void replace( char *s, char c1, char c2 )
{
    if ( !s )
	return;
    while ( *s ) {
	if ( *s == c1 )
	    *s = c2;
	s++;
    }
}

/*
    This function looks at two file names and returns the name of the
    infile with a path relative to outfile.

    Examples:

	/tmp/abc, /tmp/bcd -> abc
	xyz/a/bc, xyz/b/ac -> ../a/bc
	/tmp/abc, xyz/klm -> /tmp/abc
 */

QCString combinePath( const char *infile, const char *outfile )
{
    QFileInfo inFileInfo( QDir::current(), QFile::decodeName(infile) );
    QFileInfo outFileInfo( QDir::current(), QFile::decodeName(outfile) );
    int numCommonComponents = 0;

    QStringList inSplitted =
	QStringList::split( '/', inFileInfo.dir().canonicalPath(), TRUE );
    QStringList outSplitted =
	QStringList::split( '/', outFileInfo.dir().canonicalPath(), TRUE );

    while ( !inSplitted.isEmpty() && !outSplitted.isEmpty() &&
	    inSplitted.first() == outSplitted.first() ) {
	inSplitted.remove( inSplitted.begin() );
	outSplitted.remove( outSplitted.begin() );
	numCommonComponents++;
    }

    if ( numCommonComponents < 2 ) {
	/*
	  The paths don't have the same drive, or they don't have the
	  same root directory. Use an absolute path.
	*/
	return QFile::encodeName( inFileInfo.absFilePath() );
    } else {
	/*
	  The paths have something in common. Use a path relative to
	  the output file.
	*/
	while ( !outSplitted.isEmpty() ) {
	    outSplitted.remove( outSplitted.begin() );
	    inSplitted.prepend( ".." );
	}
	inSplitted.append( inFileInfo.fileName() );
	return QFile::encodeName( inSplitted.join("/") );
    }
}


#define getenv hack_getenv			// workaround for byacc
char *getenv()		     { return 0; }
char *getenv( const char * ) { return 0; }

void init()					// initialize
{
    BEGIN OUTSIDE;
    if(g)
	delete g;
    g = new parser_reg;
    lineNo	 = 1;
    skipClass	 = FALSE;
    skipFunc	 = FALSE;
    tmpArgList	 = new ArgList;
    tmpFunc	 = new Function;
    tmpEnum	 = new Enum;

#ifdef MOC_MWERKS_PLUGIN
    buf_buffer = NULL;
    buf_index = 0;
    buf_size_total = 0;
#endif
}

void cleanup()
{
    delete g;
    g = NULL;

#ifdef MOC_MWERKS_PLUGIN
    if(buf_buffer && g_ctx)
	CWReleaseFileText(g_ctx, buf_buffer);
#endif
}

void initClass()				 // prepare for new class
{
    tmpAccess      = Private;
    subClassPerm       = Private;
    Q_OBJECTdetected   = FALSE;
    Q_PROPERTYdetected = FALSE;
    skipClass	       = FALSE;
    templateClass      = FALSE;
    g->slots.clear();
    g->signals.clear();
    g->propfuncs.clear();
    g->enums.clear();
    g->funcs.clear();
    g->props.clear();
    g->infos.clear();
    g->qtSets.clear();
    g->qtEnums.clear();
    g->multipleSuperClasses.clear();
}

struct NamespaceInfo
{
    QCString name;
    int pLevelOnEntering; // Parenthesis level on entering the namespace
    QDict<char> definedClasses; // Classes defined in the namespace
};

QPtrList<NamespaceInfo> namespaces;

void enterNameSpace( const char *name )	 // prepare for new class
{
    static bool first = TRUE;
    if ( first ) {
	namespaces.setAutoDelete( TRUE );
	first = FALSE;
    }

    NamespaceInfo *tmp = new NamespaceInfo;
    if ( name )
	tmp->name = name;
    tmp->pLevelOnEntering = namespacePLevel;
    namespaces.append( tmp );
}

void leaveNameSpace()				 // prepare for new class
{
    NamespaceInfo *tmp = namespaces.last();
    namespacePLevel = tmp->pLevelOnEntering;
    namespaces.remove();
}

QCString nameQualifier()
{
    QPtrListIterator<NamespaceInfo> iter( namespaces );
    NamespaceInfo *tmp;
    QCString qualifier = "";
    for( ; (tmp = iter.current()) ; ++iter ) {
	if ( !tmp->name.isNull() ) {  // If not unnamed namespace
	    qualifier += tmp->name;
	    qualifier += "::";
	}
    }
    return qualifier;
}

int openNameSpaceForMetaObject( FILE *out )
{
    int levels = 0;
    QPtrListIterator<NamespaceInfo> iter( namespaces );
    NamespaceInfo *tmp;
    QCString indent = "";
    for( ; (tmp = iter.current()) ; ++iter ) {
	if ( !tmp->name.isNull() ) {  // If not unnamed namespace
	    fprintf( out, "%snamespace %s {\n", (const char *)indent,
		     (const char *) tmp->name );
	    indent += "    ";
	    levels++;
	}
    }
    QCString nm = g->className;
    int pos;
    while( (pos = nm.find( "::" )) != -1 ) {
	QCString spaceName = nm.left( pos );
	nm = nm.right( nm.length() - pos - 2 );
	if ( !spaceName.isEmpty() ) {
	    fprintf( out, "%snamespace %s {\n", (const char *)indent,
		     (const char *) spaceName );
	    indent += "    ";
	    levels++;
	}
    }
    return levels;
}

void closeNameSpaceForMetaObject( FILE *out, int levels )
{
    int i;
    for( i = 0 ; i < levels ; i++ )
	    fprintf( out, "}" );
    if ( levels )
	fprintf( out, "\n" );

}

void selectOutsideClassState()
{
    if ( namespaces.count() == 0 )
	BEGIN OUTSIDE;
    else
	BEGIN IN_NAMESPACE;
}

void registerClassInNamespace()
{
    if ( namespaces.count() == 0 )
	return;
    namespaces.last()->definedClasses.insert((const char *)g->className,(char*)1);
}

//
// Remove white space from SIGNAL and SLOT names.
// This function has been copied from qobject.cpp.
//

inline bool isSpace( char x )
{
#if defined(Q_CC_BOR)
  /*
    Borland C++ 4.5 has a weird isspace() bug.
    isspace() usually works, but not here.
    This implementation is sufficient for our internal use: rmWS()
  */
    return (uchar) x <= 32;
#else
    return isspace( (uchar) x );
#endif
}

static QCString rmWS( const char *src )
{
    QCString result( qstrlen(src)+1 );
    char *d = result.data();
    char *s = (char *)src;
    char last = 0;
    while( *s && isSpace(*s) )			// skip leading space
	s++;
    while ( *s ) {
	while ( *s && !isSpace(*s) )
	    last = *d++ = *s++;
	while ( *s && isSpace(*s) )
	    s++;
	if ( *s && isIdentChar(*s) && isIdentChar(last) )
	    last = *d++ = ' ';
    }
    result.truncate( (int)(d - result.data()) );
    return result;
}


void initExpression()
{
    g->tmpExpression = "";
}

void addExpressionString( const char *s )
{
    g->tmpExpression += s;
}

void addExpressionChar( const char c )
{
    g->tmpExpression += c;
}

void yyerror( const char *msg )			// print yacc error message
{
    g->mocError = TRUE;
#ifndef MOC_MWERKS_PLUGIN
    fprintf( stderr, ErrorFormatString" Error: %s\n", g->fileName.data(), lineNo, msg );
#else
    char	msg2[200];
    sprintf(msg2, ErrorFormatString" Error: %s", g->fileName.data(), lineNo, msg);
    CWReportMessage(g_ctx, NULL, msg2, NULL, messagetypeError, 0);
#endif
    if ( errorControl ) {
	if ( !g->outputFile.isEmpty() && yyin && fclose(yyin) == 0 )
	    remove( g->outputFile );
	exit( -1 );
    }
}

void moc_err( const char *s )
{
    yyerror( s );
}

void moc_err( const char *s1, const char *s2 )
{
    static char tmp[1024];
    sprintf( tmp, s1, s2 );
    yyerror( tmp );
}

void moc_warn( const char *msg )
{
    if ( displayWarnings )
	fprintf( stderr, ErrorFormatString" Warning: %s\n", g->fileName.data(), lineNo, msg);
}

void moc_warn( char *s1, char *s2 )
{
    static char tmp[1024];
    sprintf( tmp, s1, s2 );
    if ( displayWarnings )
	fprintf( stderr, ErrorFormatString" Warning: %s\n", g->fileName.data(), lineNo, tmp);
}

void func_warn( const char *msg )
{
    if ( !suppress_func_warn )
	moc_warn( msg );
    skipFunc = TRUE;
}

void operatorError()
{
    if ( !suppress_func_warn )
	moc_warn("Operator functions cannot be signals or slots.");
    skipFunc = TRUE;
}

#ifndef yywrap
int yywrap()					// more files?
{
    return 1;					// end of file
}
#endif

char *stradd( const char *s1, const char *s2 )	// adds two strings
{
    char *n = new char[qstrlen(s1)+qstrlen(s2)+1];
    qstrcpy( n, s1 );
    strcat( n, s2 );
    return n;
}

char *stradd( const char *s1, const char *s2, const char *s3 )// adds 3 strings
{
    char *n = new char[qstrlen(s1)+qstrlen(s2)+qstrlen(s3)+1];
    qstrcpy( n, s1 );
    strcat( n, s2 );
    strcat( n, s3 );
    return n;
}

char *stradd( const char *s1, const char *s2,
	      const char *s3, const char *s4 )// adds 4 strings
{
    char *n = new char[qstrlen(s1)+qstrlen(s2)+qstrlen(s3)+qstrlen(s4)+1];
    qstrcpy( n, s1 );
    strcat( n, s2 );
    strcat( n, s3 );
    strcat( n, s4 );
    return n;
}


char *straddSpc( const char *s1, const char *s2 )
{
    char *n = new char[qstrlen(s1)+qstrlen(s2)+2];
    qstrcpy( n, s1 );
    strcat( n, " " );
    strcat( n, s2 );
    return n;
}

char *straddSpc( const char *s1, const char *s2, const char *s3 )
{
    char *n = new char[qstrlen(s1)+qstrlen(s2)+qstrlen(s3)+3];
    qstrcpy( n, s1 );
    strcat( n, " " );
    strcat( n, s2 );
    strcat( n, " " );
    strcat( n, s3 );
    return n;
}

char *straddSpc( const char *s1, const char *s2,
	      const char *s3, const char *s4 )
{
    char *n = new char[qstrlen(s1)+qstrlen(s2)+qstrlen(s3)+qstrlen(s4)+4];
    qstrcpy( n, s1 );
    strcat( n, " " );
    strcat( n, s2 );
    strcat( n, " " );
    strcat( n, s3 );
    strcat( n, " " );
    strcat( n, s4 );
    return n;
}

// Generate C++ code for building member function table


/*
  We call B::qt_invoke() rather than A::B::qt_invoke() to
  work around a bug in MSVC 6. The bug occurs if the
  super-class is in a namespace and the sub-class isn't.

  Exception: If the superclass has the same name as the subclass, we
  want non-MSVC users to have a working generated files.
*/
QCString purestSuperClassName()
{
    QCString sc = g->superClassName;
    QCString c = g->className;
    /*
      Make sure qualified template arguments (e.g., foo<bar::baz>)
      don't interfere.
    */
    int pos = sc.findRev( "::", sc.find( '<' ) );
    if ( pos != -1 ) {
	sc = sc.right( sc.length() - pos - 2 );
	pos = c.findRev( "::" );
	if ( pos != -1 )
	    c = c.right( c.length() - pos - 2 );
	if ( sc == c )
	    sc = g->superClassName;
    }
    return sc;
}

QCString qualifiedClassName()
{
    return nameQualifier() + g->className;
}

const int Slot_Num = 1;
const int Signal_Num = 2;
const int Prop_Num = 3;

void generateFuncs( FuncList *list, const char *functype, int num )
{
    Function *f;
    for ( f=list->first(); f; f=list->next() ) {
	bool hasReturnValue = f->type != "void" && (validUType( f->type ) || isVariantType( f->type) );

	if ( hasReturnValue || !f->args->isEmpty() ) {
	    fprintf( out, "    static const QUParameter param_%s_%d[] = {\n", functype, list->at() );
	    if ( hasReturnValue ) {
		if ( validUType( f->type ) )
		    fprintf( out, "\t{ 0, &static_QUType_%s, %s, QUParameter::Out }", uType(f->type).data(), uTypeExtra(f->type).data() );
		else
		    fprintf( out, "\t{ 0, &static_QUType_QVariant, %s, QUParameter::Out }", uTypeExtra(f->type).data() );
		if ( !f->args->isEmpty() )
		    fprintf( out, ",\n" );
	    }
	    Argument* a = f->args->first();
	    while ( a ) {
		QCString type = a->leftType + ' ' + a->rightType;
		type = type.simplifyWhiteSpace();
		if( a->name.isEmpty() )
		    fprintf( out, "\t{ 0, &static_QUType_%s, %s, QUParameter::%s }",
			     uType( type ).data(), uTypeExtra( type ).data(),
			     isInOut( type ) ? "InOut" : "In" );
		else
		    fprintf( out, "\t{ \"%s\", &static_QUType_%s, %s, QUParameter::%s }",
			     a->name.data(), uType( type ).data(), uTypeExtra( type ).data(),
			     isInOut( type ) ? "InOut" : "In" );
		a = f->args->next();
		if ( a )
		    fprintf( out, ",\n" );
	    }
	    fprintf( out, "\n    };\n");
	}

	fprintf( out, "    static const QUMethod %s_%d = {", functype, list->at() );
	int n = f->args->count();
	if ( hasReturnValue )
	    n++;
	fprintf( out, "\"%s\", %d,", f->name.data(), n );
	if ( n )
	    fprintf( out, " param_%s_%d };\n", functype, list->at() );
	else
	    fprintf( out, " 0 };\n" );

	QCString typstr = "";
	int count = 0;
	Argument *a = f->args->first();
	while ( a ) {
	    if ( !a->leftType.isEmpty() || ! a->rightType.isEmpty() ) {
		if ( count++ )
		    typstr += ",";
		typstr += a->leftType;
		typstr += a->rightType;
	    }
	    a = f->args->next();
	}
	f->signature = f->name;
	f->signature += "(";
	f->signature += typstr;
	f->signature += ")";
    }
    if ( list->count() ) {
	fprintf(out,"    static const QMetaData %s_tbl[] = {\n", functype );
	f = list->first();
	while ( f ) {
	    fprintf( out, "\t{ \"%s\",", f->signature.data() );
	    fprintf( out, " &%s_%d,", functype, list->at() );
	    fprintf( out, " QMetaData::%s }", f->accessAsString() );
	    f = list->next();
	    if ( f )
		fprintf( out, ",\n");
	}
	fprintf( out, "\n    };\n" );
    }
}


int enumIndex( const char* type )
{
    int index = 0;
    for( QPtrListIterator<Enum> lit( g->enums ); lit.current(); ++lit ) {
	if ( lit.current()->name == type )
	    return index;
	index++;
    }
    return -1;
}

bool isEnumType( const char* type )
{
    return enumIndex( type ) >= 0 ||  ( g->qtEnums.contains( type ) || g->qtSets.contains( type ) );
}

bool isPropertyType( const char* type )
{
    if ( isVariantType( type ) )
	return TRUE;

    return isEnumType( type );
}

int generateEnums()
{
    if ( g->enums.count() == 0 )
	return 0;

    fprintf( out, "#ifndef QT_NO_PROPERTIES\n" );
    int i = 0;
    for ( QPtrListIterator<Enum> it( g->enums ); it.current(); ++it, ++i ) {
	fprintf( out, "    static const QMetaEnum::Item enum_%i[] = {\n", i );
	int k = 0;
	for( QStrListIterator eit( *it.current() ); eit.current(); ++eit, ++k ) {
	    if ( k )
		fprintf( out, ",\n" );
	    fprintf( out, "\t{ \"%s\",  (int) %s::%s }", eit.current(), (const char*) g->className, eit.current() );
	}
	fprintf( out, "\n    };\n" );
    }
    fprintf( out, "    static const QMetaEnum enum_tbl[] = {\n" );
    i = 0;
    for ( QPtrListIterator<Enum> it2( g->enums ); it2.current(); ++it2, ++i ) {
	if ( i )
	    fprintf( out, ",\n" );
	fprintf( out, "\t{ \"%s\", %u, enum_%i, %s }",
		 (const char*)it2.current()->name,
		 it2.current()->count(),
		 i,
		 it2.current()->set ? "TRUE" : "FALSE" );
    }
    fprintf( out, "\n    };\n" );
    fprintf( out, "#endif // QT_NO_PROPERTIES\n" );

    return g->enums.count();
}

int generateProps()
{
    //
    // Resolve and verify property access functions
    //
    for( QPtrListIterator<Property> it( g->props ); it.current(); ) {
	Property* p = it.current();
	++it;

	// verify get function
	if ( !p->get.isEmpty() ) {
	    FuncList candidates = g->propfuncs.find( p->get );
	    for ( Function* f = candidates.first(); f; f = candidates.next() ) {
		if ( f->qualifier != "const" ) // get functions must be const
		    continue;
		if ( f->args && !f->args->isEmpty() ) // and must not take any arguments
		    continue;
		QCString tmp = f->type;
		Property::Specification spec = Property::Unspecified;
		if ( p->type == "QCString" && (tmp == "const char*" || tmp == "const char *" ) ) {
		    tmp = "QCString";
		    spec = Property::ConstCharStar;
		} else if ( tmp.right(1) == "&" ) {
		    tmp = tmp.left( tmp.length() - 1 );
		    spec = Property::Reference;
		} else if ( tmp.right(1) == "*" ) {
		    tmp = tmp.left( tmp.length() - 1 );
		    spec = Property::Pointer;
		} else {
		    spec = Property::Class;
		}
		if ( tmp.left(6) == "const " )
		    tmp = tmp.mid( 6, tmp.length() - 6 );
		tmp = tmp.simplifyWhiteSpace();
		if ( p->type == tmp ) {
		    // If it is an enum then it may not be a set
		    bool ok = TRUE;
		    for( QPtrListIterator<Enum> lit( g->enums ); lit.current(); ++lit )
			if ( lit.current()->name == p->type && lit.current()->set )
			    ok = FALSE;
		    if ( !ok ) continue;
		    p->gspec = spec;
		    p->getfunc = f;
		    p->oredEnum = 0;
		    break;
		}
		else if ( !isVariantType( p->type ) ) {
		    if ( tmp == "int" || tmp == "uint" || tmp == "unsigned int" ) {
			// Test whether the enum is really a set (unfortunately we don't know enums of super classes)
			bool ok = TRUE;
			for( QPtrListIterator<Enum> lit( g->enums ); lit.current(); ++lit )
			    if ( lit.current()->name == p->type && !lit.current()->set )
				ok = FALSE;
			if ( !ok ) continue;
			p->gspec = spec;
			p->getfunc = f;
			p->oredEnum = 1;
			p->enumgettype = tmp;
		    }
		}
	    }
	    if ( p->getfunc == 0 ) {
		if ( displayWarnings ) {

		    // Is the type a set, that means, mentioned in Q_SETS?
		    bool set = FALSE;
		    for( QPtrListIterator<Enum> lit( g->enums ); lit.current(); ++lit )
			if ( lit.current()->name == p->type && lit.current()->set )
			    set = TRUE;

		    fprintf( stderr, ErrorFormatString" Warning: Property '%s' not available.\n",
			     g->fileName.data(), p->lineNo, (const char*) p->name );
		    fprintf( stderr, "   Have been looking for public get functions \n");
		    if ( !set ) {
			fprintf( stderr,
			     "      %s %s() const\n"
			     "      %s& %s() const\n"
			     "      const %s& %s() const\n"
			     "      %s* %s() const\n",
			     (const char*) p->type, (const char*) p->get,
			     (const char*) p->type, (const char*) p->get,
			     (const char*) p->type, (const char*) p->get,
			     (const char*) p->type, (const char*) p->get );
		    }
		    if ( set || !isPropertyType( p->type ) ) {
			fprintf( stderr,
			     "      int %s() const\n"
			     "      uint %s() const\n"
			     "      unsigned int %s() const\n",
			     (const char*) p->get,
			     (const char*) p->get,
			     (const char*) p->get );
		    }
		    if ( p->type == "QCString" )
			fprintf( stderr, "      const char* %s() const\n",
				 (const char*)p->get );

		    if ( candidates.isEmpty() ) {
			fprintf( stderr, "   but found nothing.\n");
		    } else {
			fprintf( stderr, "   but only found the mismatching candidate(s)\n");
			for ( Function* f = candidates.first(); f; f = candidates.next() ) {
			    QCString typstr = "";
			    Argument *a = f->args->first();
			    int count = 0;
			    while ( a ) {
				if ( !a->leftType.isEmpty() || ! a->rightType.isEmpty() ) {
				    if ( count++ )
					typstr += ",";
				    typstr += a->leftType;
				    typstr += a->rightType;
				}
				a = f->args->next();
			    }
			    fprintf( stderr, "      %s:%d: %s %s(%s) %s\n", g->fileName.data(), f->lineNo,
				     (const char*) f->type,(const char*) f->name, (const char*) typstr,
				     f->qualifier.isNull()?"":(const char*) f->qualifier );
			}
		    }
		}
	    }
	}

	// verify set function
	if ( !p->set.isEmpty() ) {
	    FuncList candidates = g->propfuncs.find( p->set );
	    for ( Function* f = candidates.first(); f; f = candidates.next() ) {
		if ( !f->args || f->args->isEmpty() )
		    continue;
		QCString tmp = f->args->first()->leftType;
		tmp = tmp.simplifyWhiteSpace();
		Property::Specification spec = Property::Unspecified;
		if ( tmp.right(1) == "&" ) {
		    tmp = tmp.left( tmp.length() - 1 );
		    spec = Property::Reference;
		}
		else {
		    spec = Property::Class;
		}
		if ( p->type == "QCString" && (tmp == "const char*" || tmp == "const char *" ) ) {
		    tmp = "QCString";
		    spec = Property::ConstCharStar;
		}
		if ( tmp.left(6) == "const " )
		    tmp = tmp.mid( 6, tmp.length() - 6 );
		tmp = tmp.simplifyWhiteSpace();

		if ( p->type == tmp && f->args->count() == 1 ) {
		    // If it is an enum then it may not be a set
		    if ( p->oredEnum == 1 )
			continue;
		    bool ok = TRUE;
		    for( QPtrListIterator<Enum> lit( g->enums ); lit.current(); ++lit )
			if ( lit.current()->name == p->type && lit.current()->set )
			    ok = FALSE;
		    if ( !ok ) continue;
		    p->sspec = spec;
		    p->setfunc = f;
		    p->oredEnum = 0;
		    break;
		} else if ( !isVariantType( p->type ) && f->args->count() == 1 ) {
		    if ( tmp == "int" || tmp == "uint" || tmp == "unsigned int" ) {
			if ( p->oredEnum == 0 )
			    continue;
			// Test wether the enum is really a set (unfortunately we don't know enums of super classes)
			bool ok = TRUE;
			for( QPtrListIterator<Enum> lit( g->enums ); lit.current(); ++lit )
			    if ( lit.current()->name == p->type && !lit.current()->set )
				ok = FALSE;
			if ( !ok ) continue;
			p->sspec = spec;
			p->setfunc = f;
			p->oredEnum = 1;
			p->enumsettype = tmp;
		    }
		}
	    }
	    if ( p->setfunc == 0 ) {
		if ( displayWarnings ) {

		    // Is the type a set, that means, mentioned in Q_SETS ?
		    bool set = FALSE;
		    for( QPtrListIterator<Enum> lit( g->enums ); lit.current(); ++lit )
			if ( lit.current()->name == p->type && lit.current()->set )
			    set = TRUE;

		    fprintf( stderr, ErrorFormatString" Warning: Property '%s' not writable.\n",
			     g->fileName.data(), p->lineNo, (const char*) p->name );
		    fprintf( stderr, "   Have been looking for public set functions \n");
		    if ( !set && p->oredEnum != 1 ) {
			fprintf( stderr,
			     "      void %s( %s )\n"
			     "      void %s( %s& )\n"
			     "      void %s( const %s& )\n",
			     (const char*) p->set, (const char*) p->type,
			     (const char*) p->set, (const char*) p->type,
			     (const char*) p->set, (const char*) p->type );
		    }
		    if ( set || ( !isPropertyType( p->type ) && p->oredEnum != 0 ) ) {
			fprintf( stderr,
			     "      void %s( int )\n"
			     "      void %s( uint )\n"
			     "      void %s( unsigned int )\n",
			     (const char*) p->set,
			     (const char*) p->set,
			     (const char*) p->set );
		    }

		    if ( p->type == "QCString" )
			fprintf( stderr, "      void %s( const char* ) const\n",
				 (const char*) p->set );

		    if ( !candidates.isEmpty() ) {
			fprintf( stderr, "   but only found the mismatching candidate(s)\n");
			for ( Function* f = candidates.first(); f; f = candidates.next() ) {
			    QCString typstr = "";
			    Argument *a = f->args->first();
			    int count = 0;
			    while ( a ) {
				if ( !a->leftType.isEmpty() || ! a->rightType.isEmpty() ) {
				    if ( count++ )
					typstr += ",";
				    typstr += a->leftType;
				    typstr += a->rightType;
				}
				a = f->args->next();
			    }
			    fprintf( stderr, "      %s:%d: %s %s(%s)\n", g->fileName.data(), f->lineNo,
				     (const char*) f->type,(const char*) f->name, (const char*) typstr );
			}
		    }
		}
	    }
	}
    }

    //
    // Create meta data
    //
    if ( g->props.count() )   {
	if ( displayWarnings && !Q_OBJECTdetected )
		moc_err("The declaration of the class \"%s\" contains properties"
		" but no Q_OBJECT macro.", g->className.data());

	fprintf( out, "#ifndef QT_NO_PROPERTIES\n" );

	fprintf( out, "    static const QMetaProperty props_tbl[%d] = {\n ", g->props.count() );
	for( QPtrListIterator<Property> it( g->props ); it.current(); ++it ) {

	    fprintf( out, "\t{ \"%s\",\"%s\", ", it.current()->type.data(), it.current()->name.data() );
	    int flags = Invalid;
	    if ( !isVariantType( it.current()->type ) ) {
		flags |= EnumOrSet;
		if ( !isEnumType( it.current()->type ) )
		    flags |= UnresolvedEnum;
	    } else {
		flags |= qvariant_nameToType( it.current()->type ) << 24;
	    }
	    if ( it.current()->getfunc )
		flags |= Readable;
	    if ( it.current()->setfunc ) {
		flags |= Writable;
		if ( it.current()->stdSet() )
		    flags |= StdSet;
	    }
	    if ( it.current()->override )
		flags |= Override;

	    if ( it.current()->designable.isEmpty() )
		flags |= DesignableOverride;
	    else if ( it.current()->designable == "false" )
		flags |= NotDesignable;

	    if ( it.current()->scriptable.isEmpty() )
		flags |= ScriptableOverride;
	    else if ( it.current()->scriptable == "false" )
		flags |= NotScriptable;

	    if ( it.current()->stored.isEmpty() )
		flags |= StoredOverride;
	    else if ( it.current()->stored == "false" )
		flags |= NotStored;


	    fprintf( out, "0x%.4x, ", flags );
	    fprintf( out, "&%s::metaObj, ", (const char*) qualifiedClassName() );
	    if ( !isVariantType( it.current()->type ) ) {
		int enumpos = -1;
		int k = 0;
		for( QPtrListIterator<Enum> eit( g->enums ); eit.current(); ++eit, ++k ) {
		    if ( eit.current()->name == it.current()->type )
			enumpos = k;
		}

		// Is it an enum of this class ?
		if ( enumpos != -1 )
		    fprintf( out, "&enum_tbl[%i], ", enumpos );
		else
		    fprintf( out, "0, ");
	    } else {
		fprintf( out, "0, ");
	    }
	    fprintf( out, "-1 }" );
	    if ( !it.atLast() )
		fprintf( out, ",\n" );
	    else
		fprintf( out, "\n" );
	}
	fprintf( out, "    };\n" );
	fprintf( out, "#endif // QT_NO_PROPERTIES\n" );
    }

    return g->props.count();
}



int generateClassInfos()
{
    if ( g->infos.isEmpty() )
	return 0;

    if ( displayWarnings && !Q_OBJECTdetected )
	moc_err("The declaration of the class \"%s\" contains class infos"
		" but no Q_OBJECT macro.", g->className.data());

    fprintf( out, "    static const QClassInfo classinfo_tbl[] = {\n" );
    int i = 0;
    for( QPtrListIterator<ClassInfo> it( g->infos ); it.current(); ++it, ++i ) {
	if ( i )
	    fprintf( out, ",\n" );
	fprintf( out, "\t{ \"%s\", \"%s\" }", it.current()->name.data(),it.current()->value.data() );
    }
    fprintf( out, "\n    };\n" );
    return i;
}


void generateClass()		      // generate C++ source code for a class
{
    const char *hdr1 = "/****************************************************************************\n"
		 "** %s meta object code from reading C++ file '%s'\n**\n";
    const char *hdr2 = "** Created: %s\n"
		 "**      by: The Qt MOC ($Id: moc.y 2051 2007-02-21 10:04:20Z chehrlic $)\n**\n";
    const char *hdr3 = "** WARNING! All changes made in this file will be lost!\n";
    const char *hdr4 = "*****************************************************************************/\n\n";
    int   i;

    if ( skipClass )				// don't generate for class
	return;

    if ( !Q_OBJECTdetected ) {
	if ( g->signals.count() == 0 && g->slots.count() == 0 && g->props.count() == 0 && g->infos.count() == 0 )
	    return;
	if ( displayWarnings && (g->signals.count() + g->slots.count()) != 0 )
	    moc_err("The declaration of the class \"%s\" contains signals "
		    "or slots\n\t but no Q_OBJECT macro.", g->className.data());
    } else {
	if ( g->superClassName.isEmpty() )
	    moc_err("The declaration of the class \"%s\" contains the\n"
		    "\tQ_OBJECT macro but does not inherit from any class!\n"
		    "\tInherit from QObject or one of its descendants"
		    " or remove Q_OBJECT.", g->className.data() );
    }
    if ( templateClass ) {			// don't generate for class
	moc_err( "Sorry, Qt does not support templates that contain\n"
		 "\tsignals, slots or Q_OBJECT." );
	return;
    }
    g->generatedCode = TRUE;
    g->gen_count++;

    if ( g->gen_count == 1 ) {			// first class to be generated
	QDateTime dt = QDateTime::currentDateTime();
	QCString dstr = dt.toString().ascii();
	QCString fn = g->fileName;
	i = g->fileName.length()-1;
	while ( i>0 && g->fileName[i-1] != '/' && g->fileName[i-1] != '\\' )
	    i--;				// skip path
	if ( i >= 0 )
	    fn = &g->fileName[i];
	fprintf( out, hdr1, (const char*)qualifiedClassName(),(const char*)fn);
	fprintf( out, hdr2, (const char*)dstr );
	fprintf( out, hdr3 );
	fprintf( out, hdr4 );

	if ( !g->noInclude ) {
	    /*
	      The header file might be a Qt header file with
	      QT_NO_COMPAT macros around signals, slots or
	      properties. Without the #undef, we cannot compile the
	      Qt library with QT_NO_COMPAT defined.

	      Header files of libraries build around Qt can also use
	      QT_NO_COMPAT, so this #undef might be beneficial to
	      users of Qt, and not only to developers of Qt.
	    */
	    fprintf( out, "#undef QT_NO_COMPAT\n" );

	    if ( !g->pchFile.isEmpty() )
	    	fprintf( out, "#include \"%s\" // PCH include\n", (const char*)g->pchFile );
	    if ( !g->includePath.isEmpty() && g->includePath.right(1) != "/" )
		g->includePath += "/";

	    g->includeFiles.first();
	    while ( g->includeFiles.current() ) {
		QCString inc = g->includeFiles.current();
		if ( inc[0] != '<' && inc[0] != '"' ) {
		    if ( !g->includePath.isEmpty() && g->includePath != "./" )
			inc.prepend( g->includePath );
		    inc = "\"" + inc + "\"";
		}
		fprintf( out, "#include %s\n", (const char *)inc );
		g->includeFiles.next();
	    }
	}
	fprintf( out, "#include <%sqmetaobject.h>\n", (const char*)g->qtPath );
	fprintf( out, "#include <%sqapplication.h>\n\n", (const char*)g->qtPath );
	fprintf( out, "#include <%sprivate/qucomextra_p.h>\n", (const char*)g->qtPath );
	fprintf( out, "#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != %d)\n", formatRevision );
	fprintf( out, "#error \"This file was generated using the moc from %s."
		 " It\"\n#error \"cannot be used with the include files from"
		 " this version of Qt.\"\n#error \"(The moc has changed too"
		 " much.)\"\n", QT_VERSION_STR );
	fprintf( out, "#endif\n\n" );
    } else {
	fprintf( out, "\n\n" );
    }

    if ( !g->hasVariantIncluded ) {
	bool needToIncludeVariant = !g->props.isEmpty();
	for ( Function* f =g->slots.first(); f && !needToIncludeVariant; f=g->slots.next() )
	    needToIncludeVariant = ( f->type != "void" && !validUType( f->type ) && isVariantType( f->type) );

	if ( needToIncludeVariant ) {
	    fprintf( out, "#include <%sqvariant.h>\n", (const char*)g->qtPath );
	    g->hasVariantIncluded = TRUE;
	}
    }

    bool isQObject =  g->className == "QObject" ;


//
// Generate virtual function className()
//
    fprintf( out, "const char *%s::className() const\n{\n    ",
	     (const char*)qualifiedClassName() );
    fprintf( out, "return \"%s\";\n}\n\n", (const char*)qualifiedClassName() );

//
// Generate static metaObj variable
//
    fprintf( out, "QMetaObject *%s::metaObj = 0;\n", (const char*)qualifiedClassName());

//
// Generate static cleanup object variable
//
    QCString cleanup = qualifiedClassName().copy();
    for ( int cnpos = 0; cnpos < cleanup.length(); cnpos++ ) {
	if ( cleanup[cnpos] == ':' )
	    cleanup[cnpos] = '_';
    }

    fprintf( out, "static QMetaObjectCleanUp cleanUp_%s( \"%s\", &%s::staticMetaObject );\n\n", (const char*)cleanup, (const char*)qualifiedClassName(), (const char*)qualifiedClassName() );

//
// Generate tr and trUtf8 member functions
//
    fprintf( out, "#ifndef QT_NO_TRANSLATION\n" );
    fprintf( out, "QString %s::tr( const char *s, const char *c )\n{\n",
	     (const char*)qualifiedClassName() );
    fprintf( out, "    if ( qApp )\n" );
    fprintf( out, "\treturn qApp->translate( \"%s\", s, c,"
		  " QApplication::DefaultCodec );\n",
	     (const char*)qualifiedClassName() );
    fprintf( out, "    else\n" );
    fprintf( out, "\treturn QString::fromLatin1( s );\n");
    fprintf( out, "}\n" );
    fprintf( out, "#ifndef QT_NO_TRANSLATION_UTF8\n" );
    fprintf( out, "QString %s::trUtf8( const char *s, const char *c )\n{\n",
	     (const char*)qualifiedClassName() );
    fprintf( out, "    if ( qApp )\n" );
    fprintf( out, "\treturn qApp->translate( \"%s\", s, c,"
		  " QApplication::UnicodeUTF8 );\n",
	     (const char*)qualifiedClassName() );
    fprintf( out, "    else\n" );
    fprintf( out, "\treturn QString::fromUtf8( s );\n" );
    fprintf( out, "}\n" );
    fprintf( out, "#endif // QT_NO_TRANSLATION_UTF8\n\n" );
    fprintf( out, "#endif // QT_NO_TRANSLATION\n\n" );

//
// Generate staticMetaObject member function
//
    fprintf( out, "QMetaObject* %s::staticMetaObject()\n{\n", (const char*)qualifiedClassName() );
    fprintf( out, "    if ( metaObj )\n\treturn metaObj;\n" );
    if ( isQObject )
	fprintf( out, "    QMetaObject* parentObject = staticQtMetaObject();\n" );
    else if ( !g->superClassName.isEmpty() )
	fprintf( out, "    QMetaObject* parentObject = %s::staticMetaObject();\n", (const char*)g->superClassName );
    else
	fprintf( out, "    QMetaObject* parentObject = 0;\n" );

//
// Build the classinfo array
//
   int n_infos = generateClassInfos();

// Build the enums array
// Enums HAVE to be generated BEFORE the properties and slots
//
    int n_enums = generateEnums();

//
// Build slots array in staticMetaObject()
//
    generateFuncs( &g->slots, "slot", Slot_Num );

//
// Build signals array in staticMetaObject()
//
    generateFuncs( &g->signals, "signal", Signal_Num );

//
// Build property array in staticMetaObject()
//
   int n_props = generateProps();

//
// Finally code to create and return meta object
//
    fprintf( out, "    metaObj = QMetaObject::new_metaobject(\n"
		  "\t\"%s\", parentObject,\n", (const char*)qualifiedClassName() );

    if ( g->slots.count() )
	fprintf( out, "\tslot_tbl, %d,\n", g->slots.count() );
    else
	fprintf( out, "\t0, 0,\n" );

    if ( g->signals.count() )
	fprintf( out, "\tsignal_tbl, %d,\n", g->signals.count() );
    else
	fprintf( out, "\t0, 0,\n" );

    fprintf( out, "#ifndef QT_NO_PROPERTIES\n" );
    if ( n_props )
	fprintf( out, "\tprops_tbl, %d,\n", n_props );
    else
	fprintf( out, "\t0, 0,\n" );
    if ( n_enums )
	fprintf( out, "\tenum_tbl, %d,\n", n_enums );
    else
	fprintf( out, "\t0, 0,\n" );
    fprintf( out, "#endif // QT_NO_PROPERTIES\n" );

    if ( n_infos )
	fprintf( out, "\tclassinfo_tbl, %d );\n", n_infos );
    else
	fprintf( out, "\t0, 0 );\n" );


//
// Setup cleanup handler and return meta object
//
    fprintf( out, "    cleanUp_%s.setMetaObject( metaObj );\n", cleanup.data() );
    fprintf( out, "    return metaObj;\n}\n" );

//
// End of function staticMetaObject()
//

//
// Generate smart cast function
//
    fprintf( out, "\nvoid* %s::qt_cast( const char* clname )\n{\n",
	     (const char*)qualifiedClassName() );
    fprintf( out, "    if ( !qstrcmp( clname, \"%s\" ) )\n"
		  "\treturn this;\n",
	     (const char*)qualifiedClassName() );
    for ( const char* cname = g->multipleSuperClasses.first(); cname; cname = g->multipleSuperClasses.next() ) {
	fprintf( out, "    if ( !qstrcmp( clname, \"%s\" ) )\n", cname);
        QCString fixed(cname);
        while (fixed.find(">>") != -1)
            fixed = fixed.replace(">>", "> >");
        fprintf( out, "\treturn (%s*)this;\n", fixed.data());
    }
    if ( !g->superClassName.isEmpty() && !isQObject )
	fprintf( out, "    return %s::qt_cast( clname );\n",
		 (const char*)purestSuperClassName() );
    else
	fprintf( out, "    return 0;\n" );
    fprintf( out, "}\n" );

//
// Generate internal signal functions
//
    Function *f;
    f = g->signals.first();			// make internal signal methods
    static bool included_list_headers = FALSE;
    int sigindex = 0;
    while ( f ) {
	QCString argstr;
	char buf[12];
	Argument *a = f->args->first();
	int offset = 0;
	const char *predef_call_func = 0;
	bool hasReturnValue = f->type != "void" && (validUType( f->type ) || isVariantType( f->type) );
	if ( hasReturnValue ) {
	    ; // no predefined function available
	} else if ( !a ) {
	    predef_call_func = "activate_signal";
	} else if ( f->args->count() == 1 ) {
	    QCString ctype = (a->leftType + ' ' + a->rightType).simplifyWhiteSpace();
	    if ( !isInOut( ctype ) ) {
		QCString utype = uType( ctype );
		if ( utype == "bool" )
		    predef_call_func = "activate_signal_bool";
		else if ( utype == "QString" || utype == "int" || utype == "double"  )
		    predef_call_func = "activate_signal";
	    }
	}

	if ( !predef_call_func && !included_list_headers ) {
	    // yes we need it, because otherwise QT_VERSION may not be defined
	    fprintf( out, "\n#include <%sqobjectdefs.h>\n", (const char*)g->qtPath );
	    fprintf( out, "#include <%sqsignalslotimp.h>\n", (const char*)g->qtPath );
	    included_list_headers = TRUE;
	}

	while ( a ) { // argument list
	    if ( !a->leftType.isEmpty() || !a->rightType.isEmpty() ) {
		argstr += a->leftType;
		argstr += " ";
		sprintf( buf, "t%d", offset++ );
		argstr += buf;
		argstr += a->rightType;
		a = f->args->next();
		if ( a )
		    argstr += ", ";
	    } else {
		a = f->args->next();
	    }
	}

	fixRightAngles( &argstr );

	fprintf( out, "\n// SIGNAL %s\n", (const char*)f->name );
	fprintf( out, "%s %s::%s(", (const char*) f->type,
		 (const char*)qualifiedClassName(),
		 (const char*)f->name );

	if ( argstr.isEmpty() )
	    fprintf( out, ")\n{\n" );
	else
	    fprintf( out, " %s )\n{\n", (const char*)argstr );

	if ( predef_call_func ) {
	    fprintf( out, "    %s( staticMetaObject()->signalOffset() + %d", predef_call_func, sigindex );
	    if ( !argstr.isEmpty() )
		fprintf( out, ", t0" );
	    fprintf( out, " );\n}\n" );
	} else {
	    if ( hasReturnValue )
		fprintf( out, "    %s something;\n", f->type.data() );
	    int nargs = f->args->count();
	    fprintf( out, "    if ( signalsBlocked() )\n\treturn%s;\n", hasReturnValue ? " something" : "" );
	    fprintf( out, "    QConnectionList *clist = receivers( staticMetaObject()->signalOffset() + %d );\n",
		     sigindex );
	    fprintf( out, "    if ( !clist )\n\treturn%s;\n", hasReturnValue ? " something" : "" );
	    fprintf( out, "    QUObject o[%d];\n", f->args->count() + 1 );

	    // initialize return value to something
	    if ( hasReturnValue ) {
		if ( validUType( f->type ) ) {
		    QCString utype = uType( f->type );
		    fprintf( out, "    static_QUType_%s.set(o,something);\n", utype.data() );
		} else if ( uType( f->type ) == "varptr" ) {
		    fprintf( out, "    static_QUType_varptr.set(o,&something);\n" );
		} else {
		    fprintf( out, "    static_QUType_ptr.set(o,&something);\n" );
		}
	    }

	    // initialize arguments
	    if ( !f->args->isEmpty() ) {
		offset = 0;
		Argument* a = f->args->first();
		while ( a ) {
		    QCString type = a->leftType + ' ' + a->rightType;
		    type = type.simplifyWhiteSpace();
		    if ( validUType( type ) ) {
			QCString utype = uType( type );
			fprintf( out, "    static_QUType_%s.set(o+%d,t%d);\n", utype.data(), offset+1, offset );
		    } else if ( uType( type ) == "varptr" ) {
			fprintf( out, "    static_QUType_varptr.set(o+%d,&t%d);\n", offset+1, offset );
		    } else {
			fprintf( out, "    static_QUType_ptr.set(o+%d,&t%d);\n", offset+1, offset );
		    }
		    a = f->args->next();
		    offset++;
		}
	    }
	    fprintf( out, "    activate_signal( clist, o );\n" );

	    // get return values from inOut parameters
	    if ( !f->args->isEmpty() ) {
		offset = 0;
		Argument* a = f->args->first();
		while ( a ) {
		    QCString type = a->leftType + ' ' + a->rightType;
		    type = type.simplifyWhiteSpace();
		    if ( validUType( type ) && isInOut( type ) ) {
			QCString utype = uType( type );
			if ( utype == "enum" )
			    fprintf( out, "    t%d = (%s)static_QUType_%s.get(o+%d);\n", offset, type.data(), utype.data(), offset+1 );
			else if ( utype == "ptr" && type.right(2) == "**" )
			    fprintf( out, "    if (t%d) *t%d = *(%s)static_QUType_ptr.get(o+%d);\n", offset, offset, type.data(), offset+1 );
			else
			    fprintf( out, "    t%d = static_QUType_%s.get(o+%d);\n", offset, utype.data(), offset+1 );
		    }
		    a = f->args->next();
		    offset++;
		}
	    }

	    // get and return return value
	    if ( hasReturnValue ) {
		QCString utype = uType( f->type );
		if ( utype == "enum" || utype == "ptr" || utype == "varptr" ) // need cast
		    fprintf( out, "    return (%s)static_QUType_%s.get(o);\n", f->type.data(), utype.data() );
		else
		    fprintf( out, "    return static_QUType_%s.get(o);\n", utype.data() );
	    }

	    fprintf( out, "}\n" );
	}

	f = g->signals.next();
	sigindex++;
    }


//
// Generate internal qt_invoke()  function
//
    fprintf( out, "\nbool %s::qt_invoke( int _id, QUObject* _o )\n{\n", qualifiedClassName().data() );

    if( !g->slots.isEmpty() ) {
	fprintf( out, "    switch ( _id - staticMetaObject()->slotOffset() ) {\n" );
	int slotindex = -1;
	for ( f = g->slots.first(); f; f = g->slots.next() ) {
	    slotindex ++;
	    if ( f->type == "void" && f->args->isEmpty() ) {
		fprintf( out, "    case %d: %s(); break;\n", slotindex, f->name.data() );
		continue;
	    }

	    fprintf( out, "    case %d: ", slotindex );
	    bool hasReturnValue = FALSE;
	    bool hasVariantReturn = FALSE;
	    if ( f->type != "void" )  {
		if (  validUType( f->type )) {
		    hasReturnValue = TRUE;
		    fprintf( out, "static_QUType_%s.set(_o,", uType(f->type).data() );
		} else if ( isVariantType( f->type ) ) {
		    hasReturnValue = hasVariantReturn = TRUE;
		    // do not need special handling for bool since this is handled as utype
		    fprintf( out, "static_QUType_QVariant.set(_o,QVariant(" );
		}
	    }
	    int offset = 0;
	    fprintf( out, "%s(", f->name.data() );
	    Argument* a = f->args->first();
	    while ( a ) {
		QCString type = a->leftType + ' ' + a->rightType;
		type = type.simplifyWhiteSpace();
		fixRightAngles( &type );
		if ( validUType( type ) ) {
		    QCString utype = uType( type );
		    if ( utype == "ptr" || utype == "varptr" || utype == "enum" )
			fprintf( out, "(%s)static_QUType_%s.get(_o+%d)", type.data(), utype.data(), offset+1 );
		    else
			fprintf( out, "(%s)static_QUType_%s.get(_o+%d)", type.data(), utype.data(), offset+1 );
		} else {
		    QCString castType = castToUType( type );
		    if(castType == type)
			fprintf( out, "(%s)(*((%s*)static_QUType_ptr.get(_o+%d)))", type.data(),
				 castType.data(), offset+1 );
		    else
			fprintf( out, "(%s)*((%s*)static_QUType_ptr.get(_o+%d))", type.data(),
				 castType.data(), offset+1 );
		}
		a = f->args->next();
		if ( a )
		    fprintf( out, "," );
		offset++;
	    }
	    fprintf( out, ")" );
	    if ( hasReturnValue )
		fprintf( out, ")" );
	    if ( hasVariantReturn )
		fprintf( out, ")" );
	    fprintf( out, "; break;\n" );
	}
	fprintf( out, "    default:\n" );

	if ( !g->superClassName.isEmpty() && !isQObject ) {
	    fprintf( out, "\treturn %s::qt_invoke( _id, _o );\n",
		     (const char *) purestSuperClassName() );
	} else {
	    fprintf( out, "\treturn FALSE;\n" );
	}
	fprintf( out, "    }\n" );
	fprintf( out, "    return TRUE;\n}\n" );
    } else {
	if ( !g->superClassName.isEmpty()  && !isQObject )
	    fprintf( out, "    return %s::qt_invoke(_id,_o);\n}\n",
		     (const char *) purestSuperClassName() );
	else
	    fprintf( out, "    return FALSE;\n}\n" );
    }


//
// Generate internal qt_emit()  function
//
    fprintf( out, "\nbool %s::qt_emit( int _id, QUObject* _o )\n{\n", qualifiedClassName().data() );

    if ( !g->signals.isEmpty() ) {
	fprintf( out, "    switch ( _id - staticMetaObject()->signalOffset() ) {\n" );
	int signalindex = -1;
	for ( f = g->signals.first(); f; f = g->signals.next() ) {
	    signalindex++;
	    if ( f->type == "void" && f->args->isEmpty() ) {
		fprintf( out, "    case %d: %s(); break;\n", signalindex, f->name.data() );
		continue;
	    }

	    fprintf( out, "    case %d: ", signalindex );
	    bool hasReturnValue = FALSE;
	    if ( f->type != "void" && validUType( f->type )) {
		hasReturnValue = TRUE;
		fprintf( out, "static_QUType_%s.set(_o,", uType(f->type).data() );
	    }
	    int offset = 0;
	    fprintf( out, "%s(", f->name.data() );
	    Argument* a = f->args->first();
	    while ( a ) {
		QCString type = a->leftType + ' ' + a->rightType;
		type = type.simplifyWhiteSpace();
		fixRightAngles( &type );
		if ( validUType( type ) ) {
		    QCString utype = uType( type );
		    if ( utype == "ptr" || utype == "varptr" || utype == "enum" )
			fprintf( out, "(%s)static_QUType_%s.get(_o+%d)", type.data(), utype.data(), offset+1 );
		    else
			fprintf( out, "(%s)static_QUType_%s.get(_o+%d)", type.data(), utype.data(), offset+1 );
		} else {
		    QCString castType = castToUType( type );
		    if(castType == type)
			fprintf( out, "(%s)(*((%s*)static_QUType_ptr.get(_o+%d)))", type.data(),
				 castType.data(), offset+1 );
		    else
			fprintf( out, "(%s)*((%s*)static_QUType_ptr.get(_o+%d))", type.data(),
				 castType.data(), offset+1 );
		}
		a = f->args->next();
		if ( a )
		    fprintf( out, "," );
		offset++;
	    }
	    fprintf( out, ")" );
	    if ( hasReturnValue )
		fprintf( out, ")" );
	    fprintf( out, "; break;\n" );
	}
	fprintf( out, "    default:\n" );
	if ( !g->superClassName.isEmpty()  && !isQObject )
	    fprintf( out, "\treturn %s::qt_emit(_id,_o);\n",
		     (const char *) purestSuperClassName() );
	else
	    fprintf( out, "\treturn FALSE;\n" );
	fprintf( out, "    }\n" );
	fprintf( out, "    return TRUE;\n}\n" );
    } else {
	if ( !g->superClassName.isEmpty()  && !isQObject )
	    fprintf( out, "    return %s::qt_emit(_id,_o);\n}\n",
		     (const char *) purestSuperClassName() );
	else
	    fprintf( out, "    return FALSE;\n}\n" );
    }


    fprintf( out, "#ifndef QT_NO_PROPERTIES\n" );
//
// Generate internal qt_property()  functions
//

    fprintf( out, "\nbool %s::qt_property( int id, int f, QVariant* v)\n{\n", qualifiedClassName().data() );

    if ( !g->props.isEmpty() ) {
	fprintf( out, "    switch ( id - staticMetaObject()->propertyOffset() ) {\n" );
	int propindex = -1;
	bool need_resolve = FALSE;

	for( QPtrListIterator<Property> it( g->props ); it.current(); ++it ){
	    propindex ++;
	    fprintf( out, "    case %d: ", propindex );
	    fprintf( out, "switch( f ) {\n" );

	    uint flag_break = 0;
	    uint flag_propagate = 0;

	    if ( it.current()->setfunc ) {
		fprintf( out, "\tcase 0: %s(", it.current()->setfunc->name.data() );
		QCString type = it.current()->type.copy(); // detach on purpose
		if ( it.current()->oredEnum )
		    type = it.current()->enumsettype;
		if ( type == "uint" )
		    fprintf( out, "v->asUInt()" );
		else if ( type == "unsigned int" )
		    fprintf( out, "(uint)v->asUInt()" );
		else if ( type == "QMap<QString,QVariant>" )
		    fprintf( out, "v->asMap()" );
		else if ( type == "QValueList<QVariant>" )
		    fprintf( out, "v->asList()" );
		else if ( type == "Q_LLONG" )
		    fprintf( out, "v->asLongLong()" );
		else if ( type == "Q_ULLONG" )
		    fprintf( out, "v->asULongLong()" );
		else if ( isVariantType( type ) ) {
		    if ( type[0] == 'Q' )
			type = type.mid(1);
		    else
			type[0] = toupper( type[0] );
		    fprintf( out, "v->as%s()", type.data() );
		} else {
		    fprintf( out, "(%s&)v->asInt()", type.data() );
		}
		fprintf( out, "); break;\n" );

	    } else if ( it.current()->override ) {
		flag_propagate |= 1 << (0+1);
	    }
	    if ( it.current()->getfunc ) {
		if ( it.current()->gspec == Property::Pointer )
		    fprintf( out, "\tcase 1: if ( this->%s() ) *v = QVariant( %s*%s()%s ); break;\n",
			     it.current()->getfunc->name.data(),
			     !isVariantType( it.current()->type ) ? "(int)" : "",
			     it.current()->getfunc->name.data(),
			     it.current()->type == "bool" ? ", 0" : "" );
		else
		    fprintf( out, "\tcase 1: *v = QVariant( %sthis->%s()%s ); break;\n",
			     !isVariantType( it.current()->type ) ? "(int)" : "",
			     it.current()->getfunc->name.data(),
			     it.current()->type == "bool" ? ", 0" : "" );
	    } else if ( it.current()->override ) {
		flag_propagate |= 1<< (1+1);
	    }

	    if ( !it.current()->reset.isEmpty() )
		fprintf( out, "\tcase 2: this->%s(); break;\n", it.current()->reset.data() );

	    if ( it.current()->designable.isEmpty() )
		flag_propagate |= 1 << (3+1);
	    else if ( it.current()->designable == "true" )
		flag_break |= 1 << (3+1);
	    else if ( it.current()->designable != "false" )
		fprintf( out, "\tcase 3: return this->%s();\n", it.current()->designable.data() );

	    if ( it.current()->scriptable.isEmpty() )
		flag_propagate |= 1 << (4+1);
	    else if ( it.current()->scriptable == "true" )
		flag_break |= 1 << (4+1);
	    else if ( it.current()->scriptable != "false" )
		fprintf( out, "\tcase 4: return this->%s();\n", it.current()->scriptable.data() );

	    if ( it.current()->stored.isEmpty() )
		flag_propagate |= 1 << (5+1);
	    else if ( it.current()->stored == "true" )
		flag_break |= 1 << (5+1);
	    else if ( it.current()->stored != "false" )
		fprintf( out, "\tcase 5: return this->%s();\n", it.current()->stored.data() );

	    int i = 0;
	    if ( flag_propagate != 0 ) {
		fprintf( out, "\t" );
		for ( i = 0; i <= 5; i++ ) {
		    if ( flag_propagate & (1 << (i+1) ) )
			fprintf( out, "case %d: ", i );
		}
		if (!g->superClassName.isEmpty() &&  !isQObject ) {
		    fprintf( out, "goto resolve;\n" );
		    need_resolve = TRUE;
		} else {
		    fprintf( out, "    return FALSE;\n" );
		}
	    }
	    if ( flag_break != 0 ) {
		fprintf( out, "\t" );
		for ( i = 0; i <= 5; i++ ) {
		    if ( flag_break & (1 << (i+1) ) )
			fprintf( out, "case %d: ", i );
		}
		fprintf( out, "break;\n");
	    }

	    fprintf( out, "\tdefault: return FALSE;\n    } break;\n" );
	}
	fprintf( out, "    default:\n" );
	if ( !g->superClassName.isEmpty()  && !isQObject )
	    fprintf( out, "\treturn %s::qt_property( id, f, v );\n",
		     (const char *) purestSuperClassName() );
	else
	    fprintf( out, "\treturn FALSE;\n" );
	fprintf( out, "    }\n" );
	fprintf( out, "    return TRUE;\n" );

	if ( need_resolve )
	    fprintf( out, "resolve:\n    return %s::qt_property( staticMetaObject()->resolveProperty(id), f, v );\n",
		     (const char *) purestSuperClassName() );
	fprintf( out, "}\n" );
    } else {
	if ( !g->superClassName.isEmpty() &&  !isQObject )
	    fprintf( out, "    return %s::qt_property( id, f, v);\n}\n",
		     (const char *) purestSuperClassName() );
	else
	    fprintf( out, "    return FALSE;\n}\n" );
    }

    fprintf( out, "\nbool %s::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }\n", qualifiedClassName().data() );
    fprintf( out, "#endif // QT_NO_PROPERTIES\n" );
}


ArgList *addArg( Argument *a )			// add argument to list
{
    if ( (!a->leftType.isEmpty() || !a->rightType.isEmpty() ) )  //filter out truely void arguments
	tmpArgList->append( a );
    return tmpArgList;
}

void addEnum()
{
    // Avoid duplicates
    for( QPtrListIterator<Enum> lit( g->enums ); lit.current(); ++lit ) {
	if ( lit.current()->name == tmpEnum->name )
	{
	    if ( displayWarnings )
		moc_err( "Enum %s defined twice.", (const char*)tmpEnum->name );
	}
    }

    // Only look at types mentioned  in Q_ENUMS and Q_SETS
    if ( g->qtEnums.contains( tmpEnum->name ) || g->qtSets.contains( tmpEnum->name ) )
    {
	g->enums.append( tmpEnum );
	if ( g->qtSets.contains( tmpEnum->name ) )
	    tmpEnum->set = TRUE;
	else
	    tmpEnum->set = FALSE;
    }
    else
	delete tmpEnum;
    tmpEnum = new Enum;
}

void addMember( Member m )
{
    if ( skipFunc ) {
	tmpFunc->args = tmpArgList; // just to be sure
	delete tmpFunc;
	tmpArgList  = new ArgList;   // ugly but works
	tmpFunc	    = new Function;
	skipFunc    = FALSE;
	return;
    }

    tmpFunc->type = tmpFunc->type.simplifyWhiteSpace();
    tmpFunc->access = tmpAccess;
    tmpFunc->args = tmpArgList;
    tmpFunc->lineNo = lineNo;

    for ( ;; ) {
	g->funcs.append( tmpFunc );

	if ( m == SignalMember ) {
	    g->signals.append( tmpFunc );
	    break;
	} else {
	    if ( m == SlotMember )
		g->slots.append( tmpFunc );
	    // PropertyCandidateMember or SlotMember
	    if ( !tmpFunc->name.isEmpty() && tmpFunc->access == Public )
		g->propfuncs.append( tmpFunc );
	    if ( !tmpFunc->args || !tmpFunc->args->hasDefaultArguments() )
		break;
	    tmpFunc = new Function( *tmpFunc );
	    tmpFunc->args = tmpFunc->args->magicClone();
	}
    }

    skipFunc = FALSE;
    tmpFunc = new Function;
    tmpArgList = new ArgList;
}

void checkPropertyName( const char* ident )
{
    if ( ident[0] == '_' ) {
	moc_err( "Invalid property name '%s'.", ident );
	return;
    }
}
