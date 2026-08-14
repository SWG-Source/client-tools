#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#define YYPREFIX "yy"
#line 55 "moc.y"
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
static void init();				/* initialize*/
static void initClass();			/* prepare for new class*/
static void generateClass();			/* generate C++ code for class*/
static void initExpression();			/* prepare for new expression*/
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
{						/* Avoid bug in isalnum*/
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


class Argument					/* single arg meta data*/
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

class ArgList : public QPtrList<Argument> {	/* member function arg list*/
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


struct Function					/* member function meta data*/
{
    Access access;
    QCString    qualifier;			/* const or volatile*/
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

class FuncList : public QPtrList<Function> {	/* list of member functions*/
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

class EnumList : public QPtrList<Enum> {		/* list of property enums*/
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

    int oredEnum; /* If the enums item may be ored. That means the data type is int.*/
		  /* Allowed values are 1 (True), 0 (False), and -1 (Unset)*/
    QCString enumsettype; /* contains the set function type in case of oredEnum*/
    QCString enumgettype; /* contains the get function type in case of oredEnum*/

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

class PropList : public QPtrList<Property> {	/* list of properties*/
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

class ClassInfoList : public QPtrList<ClassInfo> {	/* list of class infos*/
public:
    ClassInfoList() { setAutoDelete( TRUE ); }
};

class parser_reg {
 public:
    parser_reg();
    ~parser_reg();

    /* some temporary values*/
    QCString   tmpExpression;			/* Used to store the characters the lexer*/
						/* is currently skipping (see addExpressionChar and friends)*/
    QCString  fileName;				/* file name*/
    QCString  outputFile;				/* output file name*/
    QCString  pchFile;				/* name of PCH file (used on Windows)*/
    QStrList  includeFiles;			/* name of #include files*/
    QCString  includePath;				/* #include file path*/
    QCString  qtPath;				/* #include qt file path*/
    int           gen_count; /*number of classes generated*/
    bool	  noInclude;		/* no #include <filename>*/
    bool	  generatedCode;		/* no code generated*/
    bool	  mocError;			/* moc parsing error occurred*/
    bool       hasVariantIncluded;	/*whether or not qvariant.h was included yet*/
    QCString  className;				/* name of parsed class*/
    QCString  superClassName;			/* name of first super class*/
    QStrList  multipleSuperClasses;			/* other superclasses*/
    FuncList  signals;				/* signal interface*/
    FuncList  slots;				/* slots interface*/
    FuncList  propfuncs;				/* all possible property access functions*/
    FuncList  funcs;			/* all parsed functions, including signals*/
    EnumList  enums;				/* enums used in properties*/
    PropList  props;				/* list of all properties*/
    ClassInfoList	infos;				/* list of all class infos*/

/* Used to store the values in the Q_PROPERTY macro*/
    QCString propWrite;				/* set function*/
    QCString propRead;				/* get function*/
    QCString propReset;				/* reset function*/
    QCString propStored;				/**/
    QCString propDesignable;				/* "true", "false" or function or empty if not specified*/
    QCString propScriptable;				/* "true", "false" or function or empty if not specified*/
    bool propOverride;				/* Wether OVERRIDE was detected*/

    QStrList qtEnums;				/* Used to store the contents of Q_ENUMS*/
    QStrList qtSets;				/* Used to store the contents of Q_SETS*/

};

static parser_reg *g = 0;

ArgList *addArg( Argument * );			/* add arg to tmpArgList*/

enum Member { SignalMember,
	      SlotMember,
	      PropertyCandidateMember
	    };

void	 addMember( Member );			/* add tmpFunc to current class*/
void     addEnum();				/* add tmpEnum to current class*/

char	*stradd( const char *, const char * );	/* add two strings*/
char	*stradd( const char *, const char *,	/* add three strings*/
			       const char * );
char 	*stradd( const char *, const char *,	/* adds 4 strings*/
		 const char *, const char * );

char	*straddSpc( const char *, const char * );
char	*straddSpc( const char *, const char *,
			       const char * );
char	*straddSpc( const char *, const char *,
		    const char *, const char * );

extern int yydebug;
bool	   lexDebug	   = FALSE;
int	   lineNo;			/* current line number*/
bool	   errorControl	   = FALSE;	/* controled errors*/
bool	   displayWarnings = TRUE;
bool	   skipClass;			/* don't generate for class*/
bool	   skipFunc;			/* don't generate for func*/
bool	   templateClass;		/* class is a template*/
bool	   templateClassOld;		/* previous class is a template*/

ArgList	  *tmpArgList;			/* current argument list*/
Function  *tmpFunc;			/* current member function*/
Enum      *tmpEnum;			/* current enum*/
Access tmpAccess;			/* current access permission*/
Access subClassPerm;			/* current access permission*/

bool	   Q_OBJECTdetected;		/* TRUE if current class*/
					/*  contains the Q_OBJECT macro*/
bool	   Q_PROPERTYdetected;		/* TRUE if current class*/
					/*  contains at least one Q_PROPERTY,*/
					/*  Q_OVERRIDE, Q_SETS or Q_ENUMS macro*/
bool	   tmpPropOverride;		/* current property override setting*/

int	   tmpYYStart;			/* Used to store the lexers current mode*/
int	   tmpYYStart2;			/* Used to store the lexers current mode*/
					/*  (if tmpYYStart is already used)*/

/* if the format revision changes, you MUST change it in qmetaobject.h too*/
const int formatRevision = 26;		/* moc output format revision*/

/* if the flags change, you HAVE to change it in qmetaobject.h too*/
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
#endif /*YYBISON*/
#line 689 "moc.y"
typedef union {
    char	char_val;
    int		int_val;
    double	double_val;
    char       *string;
    Access	access;
    Function   *function;
    ArgList    *arg_list;
    Argument   *arg;
} YYSTYPE;
#line 653 "y.tab.c"
#define CHAR_VAL 257
#define INT_VAL 258
#define DOUBLE_VAL 259
#define STRING 260
#define IDENTIFIER 261
#define FRIEND 262
#define TYPEDEF 263
#define AUTO 264
#define REGISTER 265
#define STATIC 266
#define EXTERN 267
#define INLINE 268
#define VIRTUAL 269
#define CONST 270
#define VOLATILE 271
#define CHAR 272
#define SHORT 273
#define INT 274
#define LONG 275
#define SIGNED 276
#define UNSIGNED 277
#define FLOAT 278
#define DOUBLE 279
#define VOID 280
#define ENUM 281
#define CLASS 282
#define STRUCT 283
#define UNION 284
#define ASM 285
#define PRIVATE 286
#define PROTECTED 287
#define PUBLIC 288
#define OPERATOR 289
#define DBL_COLON 290
#define TRIPLE_DOT 291
#define TEMPLATE 292
#define NAMESPACE 293
#define USING 294
#define MUTABLE 295
#define THROW 296
#define SIGNALS 297
#define SLOTS 298
#define Q_OBJECT 299
#define Q_PROPERTY 300
#define Q_OVERRIDE 301
#define Q_CLASSINFO 302
#define Q_ENUMS 303
#define Q_SETS 304
#define READ 305
#define WRITE 306
#define STORED 307
#define DESIGNABLE 308
#define SCRIPTABLE 309
#define RESET 310
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,   40,   40,   40,   40,   40,   42,   42,   48,
   50,   46,   51,   52,   47,   49,   43,   45,   44,   44,
   54,   41,    1,    1,    2,   55,   56,   57,   58,   30,
   30,   30,   30,   30,   29,   31,   31,   32,   32,   59,
   59,   59,   59,   34,   34,   33,   33,   11,   11,   11,
   12,   12,   13,   13,   13,   13,   13,   13,   13,   13,
   13,    3,   60,   60,   14,   14,   15,   15,   16,   16,
   17,   17,   17,   19,   19,   21,   21,   25,   25,   61,
   61,   20,   20,   24,   62,   24,   24,   63,   24,   22,
   22,   23,   64,   23,   65,   23,   23,   23,   35,   35,
   66,   35,   35,   39,   67,   10,   10,   73,   10,   74,
   10,   75,   72,   76,   72,   38,   38,   37,   37,   36,
   36,   26,   26,   27,   27,   28,   28,   71,   71,   71,
   78,   77,   81,   53,   53,   53,   53,   53,   53,   18,
   18,   18,   18,   18,   82,   82,   79,   83,   69,   69,
   84,   68,   68,   85,   86,   86,   88,   87,    4,    4,
   80,   80,   89,   89,   91,   91,   93,   90,   94,   90,
   90,   96,   99,   90,  100,  101,   90,  102,  103,   90,
  104,  106,   90,  107,  109,   90,   92,   92,  111,   92,
   92,   98,   98,  112,  112,  113,   95,   95,  115,  115,
  116,  110,  110,  117,  117,  118,  119,  119,    5,    6,
    6,    7,    7,    8,    8,    8,    8,    8,    8,    8,
    8,    8,    8,    9,    9,    9,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  120,  120,  120,
  120,  120,  120,  120,  120,  120,  120,  121,  121,  122,
  122,  122,  122,  122,  122,  122,  122,  122,  122,  122,
  122,  114,  114,  114,  114,  114,  114,  114,  114,  126,
  127,  114,  124,  124,  128,  129,  128,  130,  128,  123,
  131,  123,  125,  133,  133,  132,  132,   70,   70,  134,
  134,  134,  135,  136,  135,  138,   97,  137,  137,  137,
  137,  137,  137,  137,  105,  105,  108,  108,
};
short yylen[] = {                                         2,
    0,    2,    1,    1,    1,    1,    1,    1,    1,    0,
    0,    7,    0,    0,    6,    1,    5,    2,    2,    2,
    0,    3,    1,    1,    4,    0,    0,    0,    0,    1,
    1,    1,    1,    1,    3,    0,    1,    1,    2,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    2,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    4,    0,    1,    2,    2,    1,    2,    3,    1,
    2,    2,    2,    2,    3,    0,    1,    0,    1,    0,
    1,    3,    1,    2,    0,    5,    4,    0,    7,    0,
    1,    2,    0,    4,    0,    5,    1,    3,    1,    2,
    0,    5,    3,    1,    8,    1,    2,    0,    4,    0,
    5,    0,    4,    0,    5,    0,    1,    1,    2,    2,
    2,    0,    1,    1,    2,    1,    1,    1,    1,    3,
    0,    3,    0,    5,    1,    3,    3,    4,    2,    1,
    1,    1,    1,    1,    2,    3,    2,    3,    0,    1,
    4,    0,    1,    2,    1,    3,    0,    5,    0,    1,
    0,    1,    2,    1,    1,    1,    0,    3,    0,    4,
    1,    0,    0,    7,    0,    0,    7,    0,    0,    9,
    0,    0,    7,    0,    0,    7,    2,    3,    0,    3,
    1,    0,    1,    2,    1,    1,    0,    1,    2,    1,
    1,    0,    1,    2,    1,    1,    0,    2,    2,    3,
    1,    4,    4,    1,    3,    2,    3,    2,    1,    3,
    2,    3,    2,    1,    1,    1,    3,    3,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    3,    3,    2,    2,
    2,    2,    1,    3,    2,    2,    2,    0,    1,    2,
    1,    3,    5,    2,    3,    4,    3,    2,    6,    4,
    5,    3,    4,    6,    4,    4,    5,    3,    3,    0,
    0,    7,    1,    3,    1,    0,    4,    0,    3,    0,
    0,    3,    2,    0,    1,    5,    4,    0,    1,    0,
    1,    3,    1,    0,    4,    0,    4,    0,    3,    3,
    3,    3,    3,    3,    0,    2,    0,    2,
};
short yydefred[] = {                                      1,
    0,    0,    0,    2,    3,    4,    5,    6,    7,    8,
    9,    0,    0,    0,   19,   20,   18,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   14,   26,  140,   40,
   41,   42,   43,   44,   45,   46,   47,   53,   54,   55,
   56,   57,   58,   59,   60,   61,  141,  139,  142,  144,
  143,    0,   70,   24,    0,   22,   65,   66,  133,    0,
    0,    0,    0,  147,  160,    0,    0,    0,    0,   11,
    1,    0,   26,  146,    0,    0,    0,  136,  137,    0,
    0,  224,  225,  226,    0,  219,  211,    0,  214,    0,
   17,    1,    0,    0,   62,    0,   69,  169,  166,  171,
  172,  175,  178,  181,  184,  165,    0,    0,  164,  167,
  138,    0,  221,    0,  216,    0,    0,  223,  218,    0,
   15,   25,    0,    0,    0,    0,    0,    0,  134,  163,
    0,    0,    0,  220,  215,  210,  222,  217,   12,    0,
    0,    0,    0,    0,    0,  191,    0,    0,  189,  168,
  212,  213,    0,   33,   34,    0,    0,    0,    0,    0,
    0,   64,  271,    0,    0,   52,    0,   49,   48,   38,
    0,   31,   32,   30,    0,  170,  201,    0,  200,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  187,    0,    0,  108,  112,    0,    0,    0,  303,   73,
    0,    0,    0,  263,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  278,    0,
    0,    0,    0,    0,    0,    0,  270,    0,  118,    0,
   51,    0,    0,    0,    0,    0,   39,  290,  207,  199,
    0,    0,  301,  207,    0,  309,    0,  316,  173,  176,
    0,  326,  182,  328,  185,  188,  206,    0,  205,  190,
    0,  195,  196,   27,   27,  110,  114,    0,    0,    0,
  311,  252,  253,    0,  254,    0,  267,  266,  244,  248,
  259,  242,  261,  243,    0,  262,  245,  246,  247,  249,
  260,  250,  251,    0,    0,  288,    0,  207,  277,  126,
  127,  120,    0,  124,  121,    0,    0,    0,  275,  119,
  148,   72,    0,   71,    0,    0,  272,    0,    0,    0,
   83,    0,    0,   27,    0,  207,    0,  207,    0,    0,
    0,    0,    0,    0,  204,  194,  109,    0,   27,   27,
    0,  314,    0,    0,  257,  258,  264,  227,  228,  207,
    0,  125,    0,  276,  280,    0,  291,  208,    0,    0,
   81,   74,   93,    0,    0,    0,   97,    0,  302,    0,
    0,    0,  298,    0,   99,    0,  293,    0,    0,    0,
    0,    0,    0,    0,  317,  174,  177,  179,  183,  186,
  113,  111,    0,    0,   29,  312,  307,    0,  281,    0,
  273,    0,    0,   75,   82,   27,    0,  104,   85,    0,
   95,   92,   35,  296,    0,   27,  101,  100,  207,    0,
    0,    0,    0,    0,    0,    0,    0,  115,  306,  315,
  279,  207,    0,    0,  153,    0,   98,   28,    0,   27,
   27,  103,  299,   27,    0,  294,  319,  320,  322,  323,
  324,  321,  180,    0,    0,  154,    0,    0,    0,  150,
   94,   86,   88,    0,  297,    0,  157,    0,    0,    0,
   28,   96,  102,   27,  156,   79,    0,  131,    0,  128,
  105,  129,   89,    0,  151,    0,    0,  158,  132,  130,
};
short yydgoto[] = {                                      93,
   53,   54,  162,   64,   65,   85,   86,   87,   88,  163,
  164,  165,  166,  167,  168,   69,  169,   48,  318,  319,
  320,  365,  366,  321,  477,  302,  303,  304,  322,  170,
  217,  171,  172,  173,  374,  229,  306,  307,  375,    4,
    5,    6,    7,    8,    9,   10,   11,   26,   94,   92,
   14,   71,   21,   12,   72,  337,  462,  430,  174,   22,
  362,  438,  471,  406,  440,  444,  244,  434,  459,  247,
  481,  196,  264,  339,  265,  340,  482,  486,   23,  107,
   76,   24,  175,  460,  435,  456,  457,  474,  108,  109,
  110,  150,  131,  123,  176,  124,  184,  260,  330,  125,
  331,  126,  427,  127,  188,  333,  128,  190,  334,  256,
  193,  261,  262,  263,  178,  179,  258,  259,  316,  219,
  180,  181,  245,  376,  182,  315,  402,  377,  441,  416,
  324,  199,  344,  270,  271,  395,  385,  329,
};
short yysindex[] = {                                      0,
   70, -206, -111,    0,    0,    0,    0,    0,    0,    0,
    0, -220,   41,   10,    0,    0,    0,   67,  912,  -96,
  152,  124,   91,  559,  -81,   98,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   11,    0,    0,  -54,    0,    0,    0,    0,  -14,
   -3,   59,  354,    0,    0,  201,   61,  233,  -54,    0,
    0,  266,    0,    0,   61, 1186,  329,    0,    0,  206,
  245,    0,    0,    0,  328,    0,    0,  -83,    0,  -54,
    0,    0,   70,  287,    0,  348,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  295, 1186,    0,    0,
    0,  438,    0,  -66,    0,  354,  -66,    0,    0,  303,
    0,    0,  384,  406,  411,  457,  468,  480,    0,    0,
  -42,  434,  481,    0,    0,    0,    0,    0,    0, 1064,
  265,  265,  291,  292,  306,    0,  506,  515,    0,    0,
    0,    0,  353,    0,    0,    0,  -71,  315,  467,  337,
  -48,    0,    0,  276, 1230,    0,  374,    0,    0,    0,
 1098,    0,    0,    0,  -26,    0,    0, 1064,    0,  436,
  119,  375,  377,  568,  575,  545,  292,  604,  306,  607,
    0, 1064, 1064,    0,    0,  -30,  520,  389,    0,    0,
  596,  393,  394,    0,  570,  624,  600,  177,  355,  379,
  605,  606,  609,   22,  612,  613,  986, 1229,    0,  572,
  -81,  635,   13,  467,  349,  349,    0,  363,    0,  -36,
    0,  201,  -53,  440, 1229,  441,    0,    0,    0,    0,
  443, 1229,    0,    0,  204,    0,  649,    0,    0,    0,
  463,    0,    0,    0,    0,    0,    0, 1064,    0,    0,
 1064,    0,    0,    0,    0,    0,    0,  389,  664,  665,
    0,    0,    0,  702,    0,  708,    0,    0,    0,    0,
    0,    0,    0,    0,  626,    0,    0,    0,    0,    0,
    0,    0,    0,  285,  285,    0,  667,    0,    0,    0,
    0,    0,  349,    0,    0,  285, -201,  467,    0,    0,
    0,    0,  285,    0,  599,  724,    0,  743,  742,  494,
    0,  277, 1119,    0,  724,    0,  -21,    0,  764, 1064,
 1064,  746, 1064, 1064,    0,    0,    0,  719,    0,    0,
  665,    0,  389,  693,    0,    0,    0,    0,    0,    0,
  724,    0,  467,    0,    0,  -38,    0,    0,  349,  784,
    0,    0,    0,  277,  -22,  178,    0, 1229,    0,  724,
  798,   29,    0,  407,    0,  458,    0,  724,  601,  617,
  622,  625,  646,  647,    0,    0,    0,    0,    0,    0,
    0,    0,  786,  762,    0,    0,    0,  724,    0,  467,
    0,  874,  881,    0,    0,    0,   65,    0,    0,  277,
    0,    0,    0,    0,  385,    0,    0,    0,    0,  -21,
  764,  764,  764,  764,  764,  764, 1064,    0,    0,    0,
    0,    0,  -81,  657,    0,  861,    0,    0,  894,    0,
    0,    0,    0,    0,  724,    0,    0,    0,    0,    0,
    0,    0,    0,  724,  916,    0,  913,  948,  375,    0,
    0,    0,    0,  889,    0,  933,    0,  -81, 1229,   26,
    0,    0,    0,    0,    0,    0,  991,    0,  799,    0,
    0,    0,    0, 1015,    0,  936, 1004,    0,    0,    0,
};
short yyrindex[] = {                                      0,
  330,  941,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  199,  943,    0,    0,    0,    0,    0,  373,    0,
    0,    0,    0,    5,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   76,    0,    0,  327,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   40,    0,    0,   71,    0,
    0,    0,    0,    0,    0,  940,    0,    0,    0,   31,
    0,    0,    0,    0,   78,    0,    0,    0,    0,   82,
    0,    0,  -90,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  953,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  594,
    0,    0,    0, 1062, 1068,    0,    0,    0,    0,    0,
    0,    0,  113,    0,    0, 1032,    0,    0, 1140,    0,
    0,    0,    0,    0,  126,    0,    0,    0,    0,    0,
  199,    0,    0,    0,    0,    0,    0,  602,    0,    0,
  504, 1065,    0,    0,    0,    0, 1062,    0, 1068,    0,
    0,  911,  934,    0,    0,  516,  316,   33,    0,    0,
  548,  618,  620,    0,    0,    0,  663, 1018, 1160, 1209,
 1364, 1366, 1386, 1388, 1391, 1393,    0,    4,    0,    0,
    0,    0, 1396, 1140,  -18,  -18,    0,   37,    0,    0,
    0,  168,   80,    0,  191,    0,    0,    0,    0,    0,
    0,  187,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  942,    0,    0,
  988,    0,    0,    0,    0,    0,    0,   33,   49,  998,
    0,    0,    0, 1398,    0, 1418,    0,    0,    0,    0,
    0,    0,    0,    0, 1420,    0,    0,    0,    0,    0,
    0,    0,    0, 1423, 1423,    0,    0,    0,    0,    0,
    0,    0,  -29,    0,    0,  360,    0, 1140,    0,    0,
    0,    0,   37,    0,    0,  478,    0,    0,  -37, 1084,
    0,  -27,  199,    0,  527,    0,    0,    0, 1085,  934,
  934,    0,  934,  934,    0,    0,    0,    0,    0,    0,
  998,    0, 1002,    0,    0,    0,    0,    0,    0,    0,
  571,    0, 1140,    0,    0,    0,    0,    0,  298,  408,
    0,    0,    0,    0,  -23,   64,    0,    9,    0,  648,
  477,    0,    0,  510,    0,    0,    0,  697,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  741,    0, 1140,
    0,    0,  238,    0,    0,    0,    0,    0,    0,  -34,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
 1085, 1085, 1085, 1085, 1085, 1085,  934,    0,    0,    0,
    0,    0,    0,   -2,    0,    0,    0,    0,  -16,    0,
    0,    0,    0,    0,  818,    0,    0,    0,    0,    0,
    0,    0,    0,  867,    0,    0,  350,    0,   58,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  211,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
short yygindex[] = {                                   1152,
  347,    0, 1150,  930,    0,    0,  435, 1056,  449,  283,
 -135,    0,   57,  -11,  -25,  122,    0,    0,    0,    0,
    0,  765,  810, -294,    0, -159,    0,  892,    0,  -10,
  -44, -156, 1177, 1182,  830,  244, 1039,  301,  840,    0,
    0,    0,    0,    0,    0,    0,    0,    0, 1114,    0,
    0,    0,    0,    0, 1134,  231,  752,    0, 1207,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  768,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  780,    0,    0,    0, 1142,
    0,    0,    0,    0,    0,    0, 1109, -186,    0,    0,
    0,    0,    0,    0, 1067,    0,    0, 1063,    0,    0,
    0,    0,  994,  -82,    0, 1078,    0,  999,  274, -170,
    0,    0,    0,    0,    0,    0,    0,  849,    0,    0,
    0,    0,  931, 1005,  974,    0,  856,    0,
};
#define YYTABLESIZE 1510
short yytable[] = {                                      68,
   20,  226,  218,   77,   63,  225,   90,  228,  123,   90,
  123,  123,  123,   90,  123,  149,   90,   84,  372,  122,
   84,  122,  122,  122,   87,  122,   90,   87,  123,  123,
  266,  123,  239,   90,   16,  235,  373,   89,  409,  122,
  122,   37,  122,   37,   37,   37,   36,   37,   36,   36,
   36,  198,   36,  299,   13,  115,  149,  177,  149,  223,
  267,  123,  119,  135,   37,  405,  305,  218,  372,   36,
   73,   18,  122,  194,   23,   47,  310,   23,  218,   23,
   23,   23,  290,   23,  480,  218,  479,  353,  135,   23,
   89,  138,  313,  123,   37,  177,  238,   23,   23,   36,
   23,   25,  226,  195,   91,  437,  225,   91,   67,  257,
   67,   67,   67,   23,   67,   23,  308,   23,  308,   68,
  149,   68,   68,   68,   91,   68,   28,  159,   67,   67,
   23,   67,   27,   23,   23,  222,  209,  355,  159,   68,
   68,   55,   68,  386,  387,  291,  389,  390,  478,   15,
   23,  218,  106,   23,   23,  411,  106,  310,  242,  236,
  237,   67,   23,   50,   52,   50,   50,   50,  133,   50,
  106,  106,   68,  313,  476,  257,  243,   80,   16,   66,
  308,   17,  399,   50,   50,  117,   50,  368,   90,  197,
  313,   21,   21,   67,   80,  297,  218,  323,   23,  403,
  209,   21,  159,  218,   68,   71,   67,  237,   67,   71,
   56,  218,   66,   59,  281,  226,   50,  237,  146,  225,
   70,  231,  223,   67,  223,   23,   23,   76,   36,  431,
  274,  123,   36,   90,  274,   75,   75,  280,  408,  371,
  453,   67,  122,  218,  221,  112,   77,  327,  274,  274,
  400,   78,  308,   77,  147,  148,   90,   78,  149,  123,
   73,  123,  326,   90,   37,   73,  123,   84,  411,   36,
  122,   66,  122,  295,   87,   37,   37,   37,   37,   37,
   37,   37,   37,   37,   37,   37,   37,   37,  233,  408,
   23,   91,   37,   37,   37,   37,  152,  116,  152,   36,
   23,   23,   23,   23,   23,   23,   23,   23,   23,   23,
   23,  236,  218,  226,  226,  323,  364,  225,  225,   79,
   23,   66,  226,  413,   91,  116,  225,   95,   23,   23,
   23,   67,   67,   67,   67,   67,   67,   67,   67,   67,
   67,   67,   68,   68,   68,   68,   68,   68,   68,   68,
   68,   68,   68,   72,   91,  122,  122,   72,  122,   67,
  152,   67,    2,    3,  145,   23,  145,  363,  145,  111,
   68,  116,   68,   23,   23,   23,   23,   23,   23,   23,
   23,   23,   23,   23,  145,  145,   50,   50,   50,   50,
   50,   50,   50,   50,   50,   50,   50,  283,   74,  117,
  226,   23,   23,  117,  225,   57,   58,  455,  155,  122,
  155,  121,   73,  194,   50,  282,   50,  117,  117,  129,
  122,   97,  226,  286,  323,  442,  225,  139,   71,   71,
   71,   71,   71,   71,   71,   71,   71,   71,   71,  284,
  285,  140,  455,  195,  226,  141,  227,   36,  225,  145,
  142,   36,  274,  273,  275,  276,   71,   23,   36,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,  155,  310,  151,  417,   36,   76,   36,   36,
   63,   63,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,   36,   36,   36,  338,  143,  417,  152,  216,
   36,  420,   36,  212,  208,   80,  206,  144,  207,  209,
  204,  210,  309,  211,  104,  113,  419,  325,  104,  145,
  104,  152,  118,  317,  106,  183,  202,  201,  203,  114,
   82,   83,   84,  152,   67,  104,  223,  154,  155,   30,
   31,   32,   33,   34,   35,   36,   37,  300,  134,  310,
  186,  137,  187,  295,  369,  107,  106,  205,  122,  107,
  213,  241,  300,  191,  224,  367,  189,  104,  295,  392,
  393,  351,  192,  107,  107,  200,   72,   72,   72,   72,
   72,   72,   72,   72,   72,   72,   72,  239,  251,  354,
  214,  239,  215,  122,  348,  349,   62,  220,   60,  370,
   61,  378,  289,  289,   72,  239,  239,  367,  249,  412,
  155,   21,   21,  356,   80,  250,   63,  418,  300,  301,
  117,   21,   81,  398,  154,  155,   30,   31,   32,   33,
   34,   35,   36,   37,  232,  246,  436,  248,  401,   82,
   83,   84,  268,   67,  253,  155,  443,  255,  117,  269,
  412,  282,  282,  367,   64,   64,  272,  240,  418,  241,
  279,  240,  277,  241,  278,  287,  288,  347,   36,  289,
  464,  465,  292,  293,  466,  240,  240,  241,  241,   36,
   36,   36,   36,   36,   36,   36,   36,   36,   36,   36,
   36,   36,  445,  298,  296,  286,  286,   36,  132,   36,
  312,  314,  231,  223,  484,  454,  231,  328,  343,   38,
   39,   40,   41,   42,   43,   44,   45,   46,  197,  268,
  231,  231,  332,  357,  342,  350,  198,  268,  154,  155,
   30,   31,   32,   33,   34,   35,   36,   37,  289,  289,
  289,  289,  289,  289,  289,  289,  289,  289,  289,  289,
  289,  289,  289,  289,  289,  289,  289,  289,  289,  289,
  289,  289,  345,  289,  289,  289,  289,  289,  346,  289,
  289,  289,  283,  283,  289,  289,  289,  289,  289,  289,
  289,  289,  358,  359,  361,  360,  388,  282,  282,  282,
  282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
  282,  282,  282,  282,  282,  282,  282,  282,  282,  282,
  282,  391,  282,  282,  282,  282,  282,  397,  282,  282,
  282,  285,  285,  282,  282,  282,  282,  282,  282,  282,
  282,  286,  286,  286,  286,  286,  286,  286,  286,  286,
  286,  286,  286,  286,  286,  286,  286,  286,  286,  286,
  286,  286,  286,  286,  286,  414,  286,  286,  286,  286,
  286,  421,  286,  286,  286,  287,  287,  286,  286,  286,
  286,  286,  286,  286,  286,   63,   63,  422,  428,  197,
  197,  197,  423,   63,   63,  424,  429,  198,  198,  198,
  197,  197,  197,  197,  197,  197,  197,  197,  198,  198,
  198,  198,  198,  198,  198,  198,  425,  426,  283,  283,
  283,  283,  283,  283,  283,  283,  283,  283,  283,  283,
  283,  283,  283,  283,  283,  283,  283,  283,  283,  283,
  283,  283,  432,  283,  283,  283,  283,  283,  433,  283,
  283,  283,  284,  284,  283,  283,  283,  283,  283,  283,
  283,  283,  458,  461,  463,  467,  468,  285,  285,  285,
  285,  285,  285,  285,  285,  285,  285,  285,  285,  285,
  285,  285,  285,  285,  285,  285,  285,  285,  285,  285,
  285,  472,  285,  285,  285,  285,  285,  469,  285,  285,
  285,  292,  292,  285,  285,  285,  285,  285,  285,  285,
  285,  287,  287,  287,  287,  287,  287,  287,  287,  287,
  287,  287,  287,  287,  287,  287,  287,  287,  287,  287,
  287,  287,  287,  287,  287,  473,  287,  287,  287,  287,
  287,  485,  287,  287,  287,  202,  268,  287,  287,  287,
  287,  287,  287,  287,  287,  154,  155,   30,   31,   32,
   33,   34,   35,   36,   37,  488,  487,  235,  192,  268,
  489,  235,  490,   13,  161,   10,  203,  268,  379,  380,
  381,  382,  383,  384,  404,  235,  235,  162,  284,  284,
  284,  284,  284,  284,  284,  284,  284,  284,  284,  284,
  284,  284,  284,  284,  284,  284,  284,  284,  284,  284,
  284,  284,  325,  284,  284,  284,  284,  284,  327,  284,
  284,  284,  193,  268,  284,  284,  284,  284,  284,  284,
  284,  284,  304,  308,   80,  318,  305,  292,  292,  292,
  292,  292,  292,  292,  292,  292,  292,  292,  292,  292,
  292,  292,  292,  292,  292,  292,  292,  292,  292,  292,
  292,    1,  292,  292,  292,  292,  292,  269,  292,  292,
  292,   19,  311,  292,  292,  292,  292,  292,  292,  292,
  292,  136,   29,  407,  439,   30,   31,   32,   33,   34,
   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,
   45,   46,   63,   63,  352,   49,  202,  202,  202,  229,
   50,  415,  230,  229,  410,  120,   96,  202,  202,  202,
  202,  202,  202,  202,  202,   63,   63,  229,  229,  192,
  192,  192,  483,   63,   63,   51,  470,  203,  203,  203,
  192,  192,  192,  192,  192,  192,  192,  192,  203,  203,
  203,  203,  203,  203,  203,  203,  294,  475,  230,  130,
  185,  254,  230,  252,  336,  240,  335,   38,   39,   40,
   41,   42,   43,   44,   45,   46,  230,  230,  446,   63,
   63,  394,  341,  193,  193,  193,  447,  448,  449,  450,
  451,  452,    0,    0,  193,  193,  193,  193,  193,  193,
  193,  193,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,   45,   45,   45,  396,    0,    0,    0,
    0,   45,    0,   45,  153,  154,  155,   30,   31,   32,
   33,   34,  156,   36,   37,   38,   39,   40,   41,   42,
   43,   44,   45,   46,  157,    0,    0,  158,    0,    0,
    0,    0,  159,   67,    0,   18,  160,  161,   66,  154,
  155,   30,   31,   32,   33,   34,   35,   36,   37,   38,
   39,   40,   41,   42,   43,   44,   45,   46,  234,   66,
    0,  158,    0,    0,    0,    0,    0,   67,    0,   18,
   38,   39,   40,   41,   42,   43,   44,   45,   46,  234,
   36,    0,  158,  232,    0,  233,    0,  232,   67,  233,
   18,   36,   36,   36,   36,   36,   36,   36,   36,   36,
    0,  232,  232,  233,  233,  234,    0,  236,    0,  234,
  237,  236,  238,    0,  237,  106,  238,  255,    0,  106,
    0,  255,    0,  234,  234,  236,  236,    0,  237,  237,
  238,  238,    0,  106,  106,  255,  255,  256,    0,  265,
    0,  256,  116,  265,    0,    0,  116,    0,    0,    0,
    0,   82,   83,   84,    0,  256,  256,  265,  265,    0,
  116,  116,   98,   99,  100,  101,  102,  103,  104,  105,
  154,  155,   30,   31,   32,   33,   34,   35,   36,   37,
    0,   38,   39,   40,   41,   42,   43,   44,   45,   46,
};
short yycheck[] = {                                      25,
   12,   38,  159,   41,   58,   42,   41,  164,   38,   44,
   40,   41,   42,   41,   44,   58,   44,   41,   40,   38,
   44,   40,   41,   42,   41,   44,   61,   44,   58,   59,
   61,   61,   59,   61,  125,  171,   58,   63,   61,   58,
   59,   38,   61,   40,   41,   42,   38,   44,   40,   41,
   42,  123,   44,  224,  261,   81,   59,  140,   61,  261,
   91,   91,   88,   59,   61,  360,  226,  224,   40,   61,
   60,  292,   91,   61,   44,   19,   44,   38,  235,   40,
   41,   42,   61,   44,   59,  242,   61,  289,  114,   59,
  116,  117,   44,  123,   91,  178,  123,   58,   59,   91,
   61,   61,   38,   91,   41,   41,   42,   44,   38,  192,
   40,   41,   42,   38,   44,   40,   59,   42,   61,   38,
  123,   40,   41,   42,   61,   44,   60,  123,   58,   59,
   91,   61,  123,   58,   59,  161,   59,  308,   59,   58,
   59,   20,   61,  330,  331,  124,  333,  334,  123,  261,
   38,  308,   40,  123,   42,   91,   44,  125,   40,  171,
  171,   91,  123,   38,  261,   40,   41,   42,  112,   44,
   58,   59,   91,  125,  469,  258,   58,  261,  290,  261,
  123,  293,  353,   58,   59,  269,   61,  323,   67,  261,
  235,  282,  283,  123,  261,  221,  353,  242,  123,  359,
  123,  292,  123,  360,  123,   38,  290,  218,  290,   42,
   59,  368,  261,  123,   38,   38,   91,  228,  261,   42,
  123,  165,  261,  290,  261,   58,   59,   41,   38,  400,
   40,  261,   42,  261,   44,  290,  290,   61,  261,  261,
  427,  290,  261,  400,  293,   40,  261,   44,   58,   59,
  289,   41,  289,  291,  297,  298,  291,  261,  261,  289,
   60,  291,   59,  291,  261,   60,  296,  291,   91,  261,
  289,  261,  291,  217,  291,  272,  273,  274,  275,  276,
  277,  278,  279,  280,  281,  282,  283,  284,  167,  261,
  123,   59,  289,  290,  291,  292,   59,  261,   61,  291,
  261,  262,  263,  264,  265,  266,  267,  268,  269,  270,
  271,  323,  469,   38,   38,  360,   40,   42,   42,  261,
  290,  261,   38,  368,  261,  289,   42,   62,  289,  290,
  291,  261,  262,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  261,  262,  263,  264,  265,  266,  267,  268,
  269,  270,  271,   38,  291,   58,   59,   42,   61,  289,
  123,  291,  293,  294,   38,  290,   40,   91,   42,   41,
  289,   44,  291,  261,  262,  263,  264,  265,  266,  267,
  268,  269,  270,  271,   58,   59,  261,  262,  263,  264,
  265,  266,  267,  268,  269,  270,  271,   43,   52,   40,
   38,  289,  290,   44,   42,  282,  283,  433,   59,   62,
   61,  125,   60,   61,  289,   61,  291,   58,   59,  125,
  123,   75,   38,   45,  469,   41,   42,  125,  261,  262,
  263,  264,  265,  266,  267,  268,  269,  270,  271,   61,
   62,   58,  468,   91,   38,   40,  164,  261,   42,  123,
   40,  261,   60,   61,   61,   62,  289,  290,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,  261,  123,  230,   41,   91,  290,  291,  292,  289,
  282,  283,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,  265,   40,   91,  261,   33,
  290,   44,  292,   37,   38,  261,   40,   40,   42,   43,
   44,   45,  230,   47,   38,   81,   59,  244,   42,   40,
   44,   41,   88,  241,   76,  261,   60,   61,   62,   81,
  286,  287,  288,  296,  290,   59,  261,  262,  263,  264,
  265,  266,  267,  268,  269,  270,  271,   44,  114,  306,
  260,  117,  261,   44,  324,   40,  108,   91,  261,   44,
   94,  126,   59,   58,  289,  322,  261,   91,   59,  339,
  340,  298,   58,   58,   59,  261,  261,  262,  263,  264,
  265,  266,  267,  268,  269,  270,  271,   40,   44,  307,
  124,   44,  126,  296,  294,  295,   38,  261,   40,  326,
   42,  328,  125,  126,  289,   58,   59,  364,   41,  366,
  261,  282,  283,  313,  261,   41,   58,  374,  270,  271,
  261,  292,  269,  350,  262,  263,  264,  265,  266,  267,
  268,  269,  270,  271,  261,  261,  406,  261,  356,  286,
  287,  288,  123,  290,   41,  296,  416,   41,  289,  261,
  407,  125,  126,  410,  282,  283,   61,   40,  415,   40,
   61,   44,   93,   44,   41,   61,   61,   42,  261,   61,
  440,  441,   61,   61,  444,   58,   59,   58,   59,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,  419,   59,  123,  125,  126,  290,  261,  292,
  261,  261,   40,  261,  474,  432,   44,   59,   44,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  125,  126,
   58,   59,  260,  125,   61,   59,  125,  126,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  261,  262,
  263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   61,  286,  287,  288,  289,  290,   61,  292,
  293,  294,  125,  126,  297,  298,  299,  300,  301,  302,
  303,  304,   59,   41,  291,   44,   41,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   93,  286,  287,  288,  289,  290,  125,  292,  293,
  294,  125,  126,  297,  298,  299,  300,  301,  302,  303,
  304,  261,  262,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   58,  286,  287,  288,  289,
  290,  261,  292,  293,  294,  125,  126,  297,  298,  299,
  300,  301,  302,  303,  304,  282,  283,  261,   93,  286,
  287,  288,  261,  282,  283,  261,  125,  286,  287,  288,
  297,  298,  299,  300,  301,  302,  303,  304,  297,  298,
  299,  300,  301,  302,  303,  304,  261,  261,  261,  262,
  263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   59,  286,  287,  288,  289,  290,   58,  292,
  293,  294,  125,  126,  297,  298,  299,  300,  301,  302,
  303,  304,  296,   93,   61,   40,   44,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,   93,  286,  287,  288,  289,  290,   40,  292,  293,
  294,  125,  126,  297,  298,  299,  300,  301,  302,  303,
  304,  261,  262,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,  284,   93,  286,  287,  288,  289,
  290,   41,  292,  293,  294,  125,  126,  297,  298,  299,
  300,  301,  302,  303,  304,  262,  263,  264,  265,  266,
  267,  268,  269,  270,  271,   41,  258,   40,  125,  126,
  125,   44,   59,  123,  125,  123,  125,  126,  305,  306,
  307,  308,  309,  310,  291,   58,   59,  125,  261,  262,
  263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  284,   41,  286,  287,  288,  289,  290,   41,  292,
  293,  294,  125,  126,  297,  298,  299,  300,  301,  302,
  303,  304,  125,   59,   41,   41,  125,  261,  262,  263,
  264,  265,  266,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  276,  277,  278,  279,  280,  281,  282,  283,
  284,    0,  286,  287,  288,  289,  290,  126,  292,  293,
  294,   12,  233,  297,  298,  299,  300,  301,  302,  303,
  304,  116,  261,  364,  410,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  282,  283,  303,   19,  286,  287,  288,   40,
   19,  372,  164,   44,  365,   92,   73,  297,  298,  299,
  300,  301,  302,  303,  304,  282,  283,   58,   59,  286,
  287,  288,  471,  282,  283,   19,  459,  286,  287,  288,
  297,  298,  299,  300,  301,  302,  303,  304,  297,  298,
  299,  300,  301,  302,  303,  304,  261,  468,   40,  108,
  142,  189,   44,  187,  261,  178,  258,  272,  273,  274,
  275,  276,  277,  278,  279,  280,   58,   59,  420,  282,
  283,  341,  268,  286,  287,  288,  421,  422,  423,  424,
  425,  426,   -1,   -1,  297,  298,  299,  300,  301,  302,
  303,  304,  261,  262,  263,  264,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  284,  343,   -1,   -1,   -1,
   -1,  290,   -1,  292,  261,  262,  263,  264,  265,  266,
  267,  268,  269,  270,  271,  272,  273,  274,  275,  276,
  277,  278,  279,  280,  281,   -1,   -1,  284,   -1,   -1,
   -1,   -1,  289,  290,   -1,  292,  293,  294,  261,  262,
  263,  264,  265,  266,  267,  268,  269,  270,  271,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  261,
   -1,  284,   -1,   -1,   -1,   -1,   -1,  290,   -1,  292,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  261,   -1,  284,   40,   -1,   40,   -1,   44,  290,   44,
  292,  272,  273,  274,  275,  276,  277,  278,  279,  280,
   -1,   58,   59,   58,   59,   40,   -1,   40,   -1,   44,
   40,   44,   40,   -1,   44,   40,   44,   40,   -1,   44,
   -1,   44,   -1,   58,   59,   58,   59,   -1,   58,   59,
   58,   59,   -1,   58,   59,   58,   59,   40,   -1,   40,
   -1,   44,   40,   44,   -1,   -1,   44,   -1,   -1,   -1,
   -1,  286,  287,  288,   -1,   58,   59,   58,   59,   -1,
   58,   59,  297,  298,  299,  300,  301,  302,  303,  304,
  262,  263,  264,  265,  266,  267,  268,  269,  270,  271,
   -1,  272,  273,  274,  275,  276,  277,  278,  279,  280,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 310
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'!'",0,0,0,"'%'","'&'",0,"'('","')'","'*'","'+'","','","'-'",0,"'/'",0,0,0,0,0,
0,0,0,0,0,"':'","';'","'<'","'='","'>'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,"'['",0,"']'","'^'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'{'","'|'","'}'","'~'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"CHAR_VAL","INT_VAL",
"DOUBLE_VAL","STRING","IDENTIFIER","FRIEND","TYPEDEF","AUTO","REGISTER",
"STATIC","EXTERN","INLINE","VIRTUAL","CONST","VOLATILE","CHAR","SHORT","INT",
"LONG","SIGNED","UNSIGNED","FLOAT","DOUBLE","VOID","ENUM","CLASS","STRUCT",
"UNION","ASM","PRIVATE","PROTECTED","PUBLIC","OPERATOR","DBL_COLON",
"TRIPLE_DOT","TEMPLATE","NAMESPACE","USING","MUTABLE","THROW","SIGNALS","SLOTS",
"Q_OBJECT","Q_PROPERTY","Q_OVERRIDE","Q_CLASSINFO","Q_ENUMS","Q_SETS","READ",
"WRITE","STORED","DESIGNABLE","SCRIPTABLE","RESET",
};
char *yyrule[] = {
"$accept : declaration_seq",
"declaration_seq :",
"declaration_seq : declaration_seq declaration",
"declaration : class_def",
"declaration : namespace_def",
"declaration : namespace_alias_def",
"declaration : using_declaration",
"declaration : using_directive",
"namespace_def : named_namespace_def",
"namespace_def : unnamed_namespace_def",
"$$1 :",
"$$2 :",
"named_namespace_def : NAMESPACE IDENTIFIER $$1 '{' $$2 namespace_body '}'",
"$$3 :",
"$$4 :",
"unnamed_namespace_def : NAMESPACE $$3 '{' $$4 namespace_body '}'",
"namespace_body : declaration_seq",
"namespace_alias_def : NAMESPACE IDENTIFIER '=' complete_class_name ';'",
"using_directive : USING NAMESPACE",
"using_declaration : USING IDENTIFIER",
"using_declaration : USING DBL_COLON",
"$$5 :",
"class_def : $$5 class_specifier ';'",
"class_name : IDENTIFIER",
"class_name : template_class_name",
"template_class_name : IDENTIFIER '<' template_args '>'",
"template_args :",
"const_expression :",
"def_argument :",
"enumerator_expression :",
"decl_specifier : storage_class_specifier",
"decl_specifier : type_specifier",
"decl_specifier : fct_specifier",
"decl_specifier : FRIEND",
"decl_specifier : TYPEDEF",
"decl_specifiers : decl_specs_opt type_name decl_specs_opt",
"decl_specs_opt :",
"decl_specs_opt : decl_specs",
"decl_specs : decl_specifier",
"decl_specs : decl_specs decl_specifier",
"storage_class_specifier : AUTO",
"storage_class_specifier : REGISTER",
"storage_class_specifier : STATIC",
"storage_class_specifier : EXTERN",
"fct_specifier : INLINE",
"fct_specifier : VIRTUAL",
"type_specifier : CONST",
"type_specifier : VOLATILE",
"type_name : elaborated_type_specifier",
"type_name : complete_class_name",
"type_name : simple_type_names",
"simple_type_names : simple_type_names simple_type_name",
"simple_type_names : simple_type_name",
"simple_type_name : CHAR",
"simple_type_name : SHORT",
"simple_type_name : INT",
"simple_type_name : LONG",
"simple_type_name : SIGNED",
"simple_type_name : UNSIGNED",
"simple_type_name : FLOAT",
"simple_type_name : DOUBLE",
"simple_type_name : VOID",
"template_spec : TEMPLATE '<' template_args '>'",
"opt_template_spec :",
"opt_template_spec : template_spec",
"class_key : opt_template_spec CLASS",
"class_key : opt_template_spec STRUCT",
"complete_class_name : qualified_class_name",
"complete_class_name : DBL_COLON qualified_class_name",
"qualified_class_name : qualified_class_name DBL_COLON class_name",
"qualified_class_name : class_name",
"elaborated_type_specifier : class_key IDENTIFIER",
"elaborated_type_specifier : ENUM IDENTIFIER",
"elaborated_type_specifier : UNION IDENTIFIER",
"argument_declaration_list : arg_declaration_list_opt triple_dot_opt",
"argument_declaration_list : arg_declaration_list ',' TRIPLE_DOT",
"arg_declaration_list_opt :",
"arg_declaration_list_opt : arg_declaration_list",
"opt_exception_argument :",
"opt_exception_argument : argument_declaration",
"triple_dot_opt :",
"triple_dot_opt : TRIPLE_DOT",
"arg_declaration_list : arg_declaration_list ',' argument_declaration",
"arg_declaration_list : argument_declaration",
"argument_declaration : decl_specifiers abstract_decl_opt",
"$$6 :",
"argument_declaration : decl_specifiers abstract_decl_opt '=' $$6 def_argument",
"argument_declaration : decl_specifiers abstract_decl_opt dname abstract_decl_opt",
"$$7 :",
"argument_declaration : decl_specifiers abstract_decl_opt dname abstract_decl_opt '=' $$7 def_argument",
"abstract_decl_opt :",
"abstract_decl_opt : abstract_decl",
"abstract_decl : abstract_decl ptr_operator",
"$$8 :",
"abstract_decl : '[' $$8 const_expression ']'",
"$$9 :",
"abstract_decl : abstract_decl '[' $$9 const_expression ']'",
"abstract_decl : ptr_operator",
"abstract_decl : '(' abstract_decl ')'",
"declarator : dname",
"declarator : declarator ptr_operator",
"$$10 :",
"declarator : declarator '[' $$10 const_expression ']'",
"declarator : '(' declarator ')'",
"dname : IDENTIFIER",
"fct_decl : '(' argument_declaration_list ')' cv_qualifier_list_opt ctor_initializer_opt exception_spec_opt opt_identifier fct_body_or_semicolon",
"fct_name : IDENTIFIER",
"fct_name : IDENTIFIER array_decls",
"$$11 :",
"fct_name : IDENTIFIER '=' $$11 const_expression",
"$$12 :",
"fct_name : IDENTIFIER array_decls '=' $$12 const_expression",
"$$13 :",
"array_decls : '[' $$13 const_expression ']'",
"$$14 :",
"array_decls : array_decls '[' $$14 const_expression ']'",
"ptr_operators_opt :",
"ptr_operators_opt : ptr_operators",
"ptr_operators : ptr_operator",
"ptr_operators : ptr_operators ptr_operator",
"ptr_operator : '*' cv_qualifier_list_opt",
"ptr_operator : '&' cv_qualifier_list_opt",
"cv_qualifier_list_opt :",
"cv_qualifier_list_opt : cv_qualifier_list",
"cv_qualifier_list : cv_qualifier",
"cv_qualifier_list : cv_qualifier_list cv_qualifier",
"cv_qualifier : CONST",
"cv_qualifier : VOLATILE",
"fct_body_or_semicolon : ';'",
"fct_body_or_semicolon : fct_body",
"fct_body_or_semicolon : '=' INT_VAL ';'",
"$$15 :",
"fct_body : '{' $$15 '}'",
"$$16 :",
"class_specifier : full_class_head '{' $$16 opt_obj_member_list '}'",
"class_specifier : class_head",
"class_specifier : class_head '*' IDENTIFIER",
"class_specifier : class_head '&' IDENTIFIER",
"class_specifier : class_head '(' IDENTIFIER ')'",
"class_specifier : template_spec whatever",
"whatever : IDENTIFIER",
"whatever : simple_type_name",
"whatever : type_specifier",
"whatever : storage_class_specifier",
"whatever : fct_specifier",
"class_head : class_key qualified_class_name",
"class_head : class_key IDENTIFIER class_name",
"full_class_head : class_head opt_base_spec",
"nested_class_head : class_key qualified_class_name opt_base_spec",
"exception_spec_opt :",
"exception_spec_opt : exception_spec",
"exception_spec : THROW '(' opt_exception_argument ')'",
"ctor_initializer_opt :",
"ctor_initializer_opt : ctor_initializer",
"ctor_initializer : ':' mem_initializer_list",
"mem_initializer_list : mem_initializer",
"mem_initializer_list : mem_initializer ',' mem_initializer_list",
"$$17 :",
"mem_initializer : complete_class_name '(' $$17 const_expression ')'",
"opt_base_spec :",
"opt_base_spec : base_spec",
"opt_obj_member_list :",
"opt_obj_member_list : obj_member_list",
"obj_member_list : obj_member_list obj_member_area",
"obj_member_list : obj_member_area",
"qt_access_specifier : access_specifier",
"qt_access_specifier : SLOTS",
"$$18 :",
"obj_member_area : qt_access_specifier $$18 slot_area",
"$$19 :",
"obj_member_area : SIGNALS $$19 ':' opt_signal_declarations",
"obj_member_area : Q_OBJECT",
"$$20 :",
"$$21 :",
"obj_member_area : Q_PROPERTY $$20 '(' property ')' $$21 opt_property_candidates",
"$$22 :",
"$$23 :",
"obj_member_area : Q_OVERRIDE $$22 '(' property ')' $$23 opt_property_candidates",
"$$24 :",
"$$25 :",
"obj_member_area : Q_CLASSINFO $$24 '(' STRING ',' STRING ')' $$25 opt_property_candidates",
"$$26 :",
"$$27 :",
"obj_member_area : Q_ENUMS $$26 '(' qt_enums ')' $$27 opt_property_candidates",
"$$28 :",
"$$29 :",
"obj_member_area : Q_SETS $$28 '(' qt_sets ')' $$29 opt_property_candidates",
"slot_area : SIGNALS ':'",
"slot_area : SLOTS ':' opt_slot_declarations",
"$$30 :",
"slot_area : ':' $$30 opt_property_candidates",
"slot_area : IDENTIFIER",
"opt_property_candidates :",
"opt_property_candidates : property_candidate_declarations",
"property_candidate_declarations : property_candidate_declarations property_candidate_declaration",
"property_candidate_declarations : property_candidate_declaration",
"property_candidate_declaration : signal_or_slot",
"opt_signal_declarations :",
"opt_signal_declarations : signal_declarations",
"signal_declarations : signal_declarations signal_declaration",
"signal_declarations : signal_declaration",
"signal_declaration : signal_or_slot",
"opt_slot_declarations :",
"opt_slot_declarations : slot_declarations",
"slot_declarations : slot_declarations slot_declaration",
"slot_declarations : slot_declaration",
"slot_declaration : signal_or_slot",
"opt_semicolons :",
"opt_semicolons : opt_semicolons ';'",
"base_spec : ':' base_list",
"base_list : base_list ',' base_specifier",
"base_list : base_specifier",
"qt_macro_name : IDENTIFIER '(' IDENTIFIER ')'",
"qt_macro_name : IDENTIFIER '(' simple_type_name ')'",
"base_specifier : complete_class_name",
"base_specifier : VIRTUAL access_specifier complete_class_name",
"base_specifier : VIRTUAL complete_class_name",
"base_specifier : access_specifier VIRTUAL complete_class_name",
"base_specifier : access_specifier complete_class_name",
"base_specifier : qt_macro_name",
"base_specifier : VIRTUAL access_specifier qt_macro_name",
"base_specifier : VIRTUAL qt_macro_name",
"base_specifier : access_specifier VIRTUAL qt_macro_name",
"base_specifier : access_specifier qt_macro_name",
"access_specifier : PRIVATE",
"access_specifier : PROTECTED",
"access_specifier : PUBLIC",
"operator_name : decl_specs_opt IDENTIFIER ptr_operators_opt",
"operator_name : decl_specs_opt simple_type_name ptr_operators_opt",
"operator_name : '+'",
"operator_name : '-'",
"operator_name : '*'",
"operator_name : '/'",
"operator_name : '%'",
"operator_name : '^'",
"operator_name : '&'",
"operator_name : '|'",
"operator_name : '~'",
"operator_name : '!'",
"operator_name : '='",
"operator_name : '<'",
"operator_name : '>'",
"operator_name : '+' '='",
"operator_name : '-' '='",
"operator_name : '*' '='",
"operator_name : '/' '='",
"operator_name : '%' '='",
"operator_name : '^' '='",
"operator_name : '&' '='",
"operator_name : '|' '='",
"operator_name : '~' '='",
"operator_name : '!' '='",
"operator_name : '=' '='",
"operator_name : '<' '='",
"operator_name : '>' '='",
"operator_name : '<' '<'",
"operator_name : '>' '>'",
"operator_name : '<' '<' '='",
"operator_name : '>' '>' '='",
"operator_name : '&' '&'",
"operator_name : '|' '|'",
"operator_name : '+' '+'",
"operator_name : '-' '-'",
"operator_name : ','",
"operator_name : '-' '>' '*'",
"operator_name : '-' '>'",
"operator_name : '(' ')'",
"operator_name : '[' ']'",
"opt_virtual :",
"opt_virtual : VIRTUAL",
"type_and_name : type_name fct_name",
"type_and_name : fct_name",
"type_and_name : opt_virtual '~' fct_name",
"type_and_name : decl_specs type_name decl_specs_opt ptr_operators_opt fct_name",
"type_and_name : decl_specs type_name",
"type_and_name : type_name ptr_operators fct_name",
"type_and_name : type_name decl_specs ptr_operators_opt fct_name",
"type_and_name : type_name OPERATOR operator_name",
"type_and_name : OPERATOR operator_name",
"type_and_name : decl_specs type_name decl_specs_opt ptr_operators_opt OPERATOR operator_name",
"type_and_name : type_name ptr_operators OPERATOR operator_name",
"type_and_name : type_name decl_specs ptr_operators_opt OPERATOR operator_name",
"signal_or_slot : type_and_name fct_decl opt_semicolons",
"signal_or_slot : type_and_name opt_bitfield ';' opt_semicolons",
"signal_or_slot : type_and_name opt_bitfield ',' member_declarator_list ';' opt_semicolons",
"signal_or_slot : enum_specifier opt_identifier ';' opt_semicolons",
"signal_or_slot : USING complete_class_name ';' opt_semicolons",
"signal_or_slot : USING NAMESPACE complete_class_name ';' opt_semicolons",
"signal_or_slot : NAMESPACE IDENTIFIER '{'",
"signal_or_slot : nested_class_head ';' opt_semicolons",
"$$31 :",
"$$32 :",
"signal_or_slot : nested_class_head '{' $$31 '}' $$32 ';' opt_semicolons",
"member_declarator_list : member_declarator",
"member_declarator_list : member_declarator_list ',' member_declarator",
"member_declarator : declarator",
"$$33 :",
"member_declarator : IDENTIFIER ':' $$33 const_expression",
"$$34 :",
"member_declarator : ':' $$34 const_expression",
"opt_bitfield :",
"$$35 :",
"opt_bitfield : ':' $$35 const_expression",
"enum_specifier : ENUM enum_tail",
"opt_komma :",
"opt_komma : ','",
"enum_tail : IDENTIFIER '{' enum_list opt_komma '}'",
"enum_tail : '{' enum_list opt_komma '}'",
"opt_identifier :",
"opt_identifier : IDENTIFIER",
"enum_list :",
"enum_list : enumerator",
"enum_list : enum_list ',' enumerator",
"enumerator : IDENTIFIER",
"$$36 :",
"enumerator : IDENTIFIER '=' $$36 enumerator_expression",
"$$37 :",
"property : IDENTIFIER IDENTIFIER $$37 prop_statements",
"prop_statements :",
"prop_statements : READ IDENTIFIER prop_statements",
"prop_statements : WRITE IDENTIFIER prop_statements",
"prop_statements : RESET IDENTIFIER prop_statements",
"prop_statements : STORED IDENTIFIER prop_statements",
"prop_statements : DESIGNABLE IDENTIFIER prop_statements",
"prop_statements : SCRIPTABLE IDENTIFIER prop_statements",
"qt_enums :",
"qt_enums : IDENTIFIER qt_enums",
"qt_sets :",
"qt_sets : IDENTIFIER qt_sets",
};
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#define yystacksize YYSTACKSIZE
#line 1606 "moc.y"

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
		 "**      by: The Qt MOC ($Id: moc_yacc.cpp 2051 2007-02-21 10:04:20Z chehrlic $)\n**\n";
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
#line 3649 "y.tab.c"
#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 10:
#line 821 "moc.y"
{ enterNameSpace(yyvsp[0].string); }
break;
case 11:
#line 822 "moc.y"
{ BEGIN IN_NAMESPACE; }
break;
case 12:
#line 824 "moc.y"
{ leaveNameSpace();
					       selectOutsideClassState();
					     }
break;
case 13:
#line 829 "moc.y"
{ enterNameSpace(); }
break;
case 14:
#line 830 "moc.y"
{ BEGIN IN_NAMESPACE; }
break;
case 15:
#line 832 "moc.y"
{ leaveNameSpace();
					       selectOutsideClassState();
					     }
break;
case 17:
#line 841 "moc.y"
{ selectOutsideClassState(); }
break;
case 18:
#line 845 "moc.y"
{ selectOutsideClassState(); }
break;
case 19:
#line 848 "moc.y"
{ selectOutsideClassState(); }
break;
case 20:
#line 849 "moc.y"
{ selectOutsideClassState(); }
break;
case 21:
#line 852 "moc.y"
{ initClass(); }
break;
case 22:
#line 853 "moc.y"
{ generateClass();
						registerClassInNamespace();
						selectOutsideClassState(); }
break;
case 23:
#line 861 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 24:
#line 862 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 25:
#line 866 "moc.y"
{ g->tmpExpression = rmWS( g->tmpExpression );
				     yyval.string = stradd( yyvsp[-3].string, "<",
						  g->tmpExpression, ">" ); }
break;
case 26:
#line 877 "moc.y"
{ initExpression();
						    templLevel = 1;
						    BEGIN IN_TEMPL_ARGS; }
break;
case 27:
#line 890 "moc.y"
{ initExpression();
						    BEGIN IN_EXPR; }
break;
case 28:
#line 899 "moc.y"
{ BEGIN IN_DEF_ARG; }
break;
case 29:
#line 902 "moc.y"
{ initExpression();
						    BEGIN IN_ENUM; }
break;
case 30:
#line 908 "moc.y"
{ yyval.string = ""; }
break;
case 31:
#line 909 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 32:
#line 910 "moc.y"
{ yyval.string = ""; }
break;
case 33:
#line 911 "moc.y"
{ skipFunc = TRUE; yyval.string = ""; }
break;
case 34:
#line 912 "moc.y"
{ skipFunc = TRUE; yyval.string = ""; }
break;
case 35:
#line 916 "moc.y"
{ yyval.string = straddSpc(yyvsp[-2].string,yyvsp[-1].string,yyvsp[0].string); }
break;
case 36:
#line 918 "moc.y"
{ yyval.string = ""; }
break;
case 37:
#line 919 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 38:
#line 922 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 39:
#line 923 "moc.y"
{ yyval.string = straddSpc(yyvsp[-1].string,yyvsp[0].string); }
break;
case 42:
#line 928 "moc.y"
{ skipFunc = TRUE; }
break;
case 44:
#line 932 "moc.y"
{ }
break;
case 45:
#line 933 "moc.y"
{ }
break;
case 46:
#line 936 "moc.y"
{ yyval.string = "const"; }
break;
case 47:
#line 937 "moc.y"
{ yyval.string = "volatile"; }
break;
case 48:
#line 940 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 49:
#line 941 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 50:
#line 942 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 51:
#line 946 "moc.y"
{ yyval.string = straddSpc(yyvsp[-1].string,yyvsp[0].string); }
break;
case 52:
#line 947 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 53:
#line 950 "moc.y"
{ yyval.string = "char"; }
break;
case 54:
#line 951 "moc.y"
{ yyval.string = "short"; }
break;
case 55:
#line 952 "moc.y"
{ yyval.string = "int"; }
break;
case 56:
#line 953 "moc.y"
{ yyval.string = "long"; }
break;
case 57:
#line 954 "moc.y"
{ yyval.string = "signed"; }
break;
case 58:
#line 955 "moc.y"
{ yyval.string = "unsigned"; }
break;
case 59:
#line 956 "moc.y"
{ yyval.string = "float"; }
break;
case 60:
#line 957 "moc.y"
{ yyval.string = "double"; }
break;
case 61:
#line 958 "moc.y"
{ yyval.string = "void"; }
break;
case 62:
#line 962 "moc.y"
{ g->tmpExpression = rmWS( g->tmpExpression );
				     yyval.string = stradd( "template<",
						  g->tmpExpression, ">" ); }
break;
case 64:
#line 968 "moc.y"
{ templateClassOld = templateClass;
						  templateClass = TRUE;
						}
break;
case 65:
#line 974 "moc.y"
{ yyval.string = "class"; }
break;
case 66:
#line 975 "moc.y"
{ yyval.string = "struct"; }
break;
case 67:
#line 978 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 68:
#line 980 "moc.y"
{ yyval.string = stradd( "::", yyvsp[0].string ); }
break;
case 69:
#line 984 "moc.y"
{ yyval.string = stradd( yyvsp[-2].string, "::", yyvsp[0].string );}
break;
case 70:
#line 985 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 71:
#line 989 "moc.y"
{ yyval.string = straddSpc(yyvsp[-1].string,yyvsp[0].string); }
break;
case 72:
#line 990 "moc.y"
{ yyval.string = stradd("enum ",yyvsp[0].string); }
break;
case 73:
#line 991 "moc.y"
{ yyval.string = stradd("union ",yyvsp[0].string); }
break;
case 74:
#line 996 "moc.y"
{ yyval.arg_list = yyvsp[-1].arg_list;}
break;
case 75:
#line 997 "moc.y"
{ yyval.arg_list = yyvsp[-2].arg_list;
				       func_warn("Ellipsis not supported"
						 " in signals and slots.\n"
						 "Ellipsis argument ignored."); }
break;
case 76:
#line 1003 "moc.y"
{ yyval.arg_list = tmpArgList; }
break;
case 77:
#line 1004 "moc.y"
{ yyval.arg_list = yyvsp[0].arg_list; }
break;
case 78:
#line 1007 "moc.y"
{ yyval.arg = 0; }
break;
case 81:
#line 1012 "moc.y"
{ func_warn("Ellipsis not supported"
						 " in signals and slots.\n"
						 "Ellipsis argument ignored."); }
break;
case 82:
#line 1020 "moc.y"
{ yyval.arg_list = addArg(yyvsp[0].arg); }
break;
case 83:
#line 1021 "moc.y"
{ yyval.arg_list = addArg(yyvsp[0].arg); }
break;
case 84:
#line 1025 "moc.y"
{ yyval.arg = new Argument(straddSpc(yyvsp[-1].string,yyvsp[0].string),""); }
break;
case 85:
#line 1027 "moc.y"
{ expLevel = 1; }
break;
case 86:
#line 1029 "moc.y"
{ yyval.arg = new Argument(straddSpc(yyvsp[-4].string,yyvsp[-3].string),"", 0, TRUE ); }
break;
case 87:
#line 1032 "moc.y"
{ yyval.arg = new Argument(straddSpc(yyvsp[-3].string,yyvsp[-2].string),yyvsp[0].string, yyvsp[-1].string); }
break;
case 88:
#line 1035 "moc.y"
{ expLevel = 1; }
break;
case 89:
#line 1037 "moc.y"
{ yyval.arg = new Argument(straddSpc(yyvsp[-6].string,yyvsp[-5].string),yyvsp[-3].string, yyvsp[-4].string, TRUE); }
break;
case 90:
#line 1041 "moc.y"
{ yyval.string = ""; }
break;
case 91:
#line 1042 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 92:
#line 1046 "moc.y"
{ yyval.string = straddSpc(yyvsp[-1].string,yyvsp[0].string); }
break;
case 93:
#line 1047 "moc.y"
{ expLevel = 1; }
break;
case 94:
#line 1049 "moc.y"
{ yyval.string = stradd( "[",
				     g->tmpExpression =
				     g->tmpExpression.stripWhiteSpace(), "]" ); }
break;
case 95:
#line 1052 "moc.y"
{ expLevel = 1; }
break;
case 96:
#line 1054 "moc.y"
{ yyval.string = stradd( yyvsp[-4].string,"[",
				     g->tmpExpression =
				     g->tmpExpression.stripWhiteSpace(),"]" ); }
break;
case 97:
#line 1057 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 98:
#line 1058 "moc.y"
{ yyval.string = yyvsp[-1].string; }
break;
case 99:
#line 1061 "moc.y"
{ yyval.string = ""; }
break;
case 100:
#line 1063 "moc.y"
{ yyval.string = straddSpc(yyvsp[-1].string,yyvsp[0].string);}
break;
case 101:
#line 1064 "moc.y"
{ expLevel = 1; }
break;
case 102:
#line 1066 "moc.y"
{ yyval.string = stradd( yyvsp[-4].string,"[",
				     g->tmpExpression =
				     g->tmpExpression.stripWhiteSpace(),"]" ); }
break;
case 103:
#line 1069 "moc.y"
{ yyval.string = yyvsp[-1].string; }
break;
case 105:
#line 1083 "moc.y"
{ tmpFunc->args	     = yyvsp[-6].arg_list;
						  tmpFunc->qualifier = yyvsp[-4].string; }
break;
case 107:
#line 1089 "moc.y"
{ func_warn("Variable as signal or slot."); }
break;
case 108:
#line 1090 "moc.y"
{ expLevel=0; }
break;
case 109:
#line 1092 "moc.y"
{ skipFunc = TRUE; }
break;
case 110:
#line 1093 "moc.y"
{ expLevel=0; }
break;
case 111:
#line 1095 "moc.y"
{ skipFunc = TRUE; }
break;
case 112:
#line 1099 "moc.y"
{ expLevel = 1; }
break;
case 114:
#line 1101 "moc.y"
{ expLevel = 1; }
break;
case 116:
#line 1106 "moc.y"
{ yyval.string = ""; }
break;
case 117:
#line 1107 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 118:
#line 1110 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 119:
#line 1111 "moc.y"
{ yyval.string = straddSpc(yyvsp[-1].string,yyvsp[0].string);}
break;
case 120:
#line 1114 "moc.y"
{ yyval.string = straddSpc("*",yyvsp[0].string);}
break;
case 121:
#line 1115 "moc.y"
{ yyval.string = stradd("&",yyvsp[0].string);}
break;
case 122:
#line 1122 "moc.y"
{ yyval.string = ""; }
break;
case 123:
#line 1123 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 124:
#line 1126 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 125:
#line 1128 "moc.y"
{ yyval.string = straddSpc(yyvsp[-1].string,yyvsp[0].string); }
break;
case 126:
#line 1131 "moc.y"
{ yyval.string = "const"; }
break;
case 127:
#line 1132 "moc.y"
{ yyval.string = "volatile"; }
break;
case 131:
#line 1140 "moc.y"
{ BEGIN IN_FCT; fctLevel = 1;}
break;
case 132:
#line 1141 "moc.y"
{ BEGIN QT_DEF; }
break;
case 133:
#line 1148 "moc.y"
{ BEGIN IN_CLASS;
						  classPLevel = 1;
						}
break;
case 134:
#line 1152 "moc.y"
{ BEGIN QT_DEF; }
break;
case 135:
#line 1153 "moc.y"
{ BEGIN QT_DEF;	  /* -- " -- */
						  skipClass = TRUE; }
break;
case 136:
#line 1155 "moc.y"
{ BEGIN QT_DEF;	  /* -- " -- */
 						  skipClass = TRUE; }
break;
case 137:
#line 1157 "moc.y"
{ BEGIN QT_DEF;	  /* -- " -- */
 						  skipClass = TRUE; }
break;
case 138:
#line 1161 "moc.y"
{ BEGIN QT_DEF; /* catch ';' */
						  skipClass = TRUE; }
break;
case 139:
#line 1163 "moc.y"
{ skipClass = TRUE;
						  BEGIN GIMME_SEMICOLON; }
break;
case 143:
#line 1170 "moc.y"
{ yyval.string = ""; }
break;
case 145:
#line 1176 "moc.y"
{ g->className = yyvsp[0].string;
						  if ( g->className == "QObject" )
						     Q_OBJECTdetected = TRUE;
						}
break;
case 146:
#line 1182 "moc.y"
{ g->className = yyvsp[0].string;
						  if ( g->className == "QObject" )
						     Q_OBJECTdetected = TRUE;
						}
break;
case 147:
#line 1189 "moc.y"
{ g->superClassName = yyvsp[0].string; }
break;
case 148:
#line 1194 "moc.y"
{ templateClass = templateClassOld; }
break;
case 157:
#line 1217 "moc.y"
{ expLevel = 1; }
break;
case 159:
#line 1222 "moc.y"
{ yyval.string = 0; }
break;
case 160:
#line 1223 "moc.y"
{ yyval.string = yyvsp[0].string; }
break;
case 165:
#line 1235 "moc.y"
{ tmpAccess = yyvsp[0].access; }
break;
case 166:
#line 1236 "moc.y"
{ moc_err( "Missing access specifier"
						   " before \"slots:\"." ); }
break;
case 167:
#line 1240 "moc.y"
{ BEGIN QT_DEF; }
break;
case 169:
#line 1242 "moc.y"
{ BEGIN QT_DEF; }
break;
case 171:
#line 1244 "moc.y"
{
			      if ( tmpAccess )
				  moc_warn("Q_OBJECT is not in the private"
					   " section of the class.\n"
					   "Q_OBJECT is a macro that resets"
					   " access permission to \"private\".");
			      Q_OBJECTdetected = TRUE;
			  }
break;
case 172:
#line 1252 "moc.y"
{ tmpYYStart = YY_START;
				       tmpPropOverride = FALSE;
				       BEGIN IN_PROPERTY; }
break;
case 173:
#line 1255 "moc.y"
{
						BEGIN tmpYYStart;
					   }
break;
case 175:
#line 1259 "moc.y"
{ tmpYYStart = YY_START;
				       tmpPropOverride = TRUE;
				       BEGIN IN_PROPERTY; }
break;
case 176:
#line 1262 "moc.y"
{
						BEGIN tmpYYStart;
					   }
break;
case 178:
#line 1266 "moc.y"
{ tmpYYStart = YY_START; BEGIN IN_CLASSINFO; }
break;
case 179:
#line 1268 "moc.y"
{
				      g->infos.append( new ClassInfo( yyvsp[-3].string, yyvsp[-1].string ) );
				      BEGIN tmpYYStart;
				  }
break;
case 181:
#line 1273 "moc.y"
{ tmpYYStart = YY_START; BEGIN IN_PROPERTY; }
break;
case 182:
#line 1274 "moc.y"
{
						Q_PROPERTYdetected = TRUE;
						BEGIN tmpYYStart;
					   }
break;
case 184:
#line 1279 "moc.y"
{ tmpYYStart = YY_START; BEGIN IN_PROPERTY; }
break;
case 185:
#line 1280 "moc.y"
{
						Q_PROPERTYdetected = TRUE;
						BEGIN tmpYYStart;
					   }
break;
case 187:
#line 1287 "moc.y"
{ moc_err( "Signals cannot "
						 "have access specifiers" ); }
break;
case 189:
#line 1290 "moc.y"
{ if ( tmpAccess == Public && Q_PROPERTYdetected )
						  BEGIN QT_DEF;
					      else
						  BEGIN IN_CLASS;
					  suppress_func_warn = TRUE;
					}
break;
case 190:
#line 1297 "moc.y"
{
					  suppress_func_warn = FALSE;
					}
break;
case 191:
#line 1300 "moc.y"
{ BEGIN IN_CLASS;
					   if ( classPLevel != 1 )
					       moc_warn( "unexpected access"
							 "specifier" );
					}
break;
case 196:
#line 1315 "moc.y"
{ addMember( PropertyCandidateMember ); }
break;
case 201:
#line 1327 "moc.y"
{ addMember( SignalMember ); }
break;
case 206:
#line 1338 "moc.y"
{ addMember( SlotMember ); }
break;
case 209:
#line 1345 "moc.y"
{ yyval.string=yyvsp[0].string; }
break;
case 210:
#line 1348 "moc.y"
{ g->multipleSuperClasses.append( yyvsp[0].string ); }
break;
case 212:
#line 1353 "moc.y"
{ yyval.string = stradd( yyvsp[-3].string, "(", yyvsp[-1].string, ")" ); }
break;
case 213:
#line 1355 "moc.y"
{ yyval.string = stradd( yyvsp[-3].string, "(", yyvsp[-1].string, ")" ); }
break;
case 214:
#line 1358 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 215:
#line 1359 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 216:
#line 1360 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 217:
#line 1361 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 218:
#line 1362 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 219:
#line 1363 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 220:
#line 1364 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 221:
#line 1365 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 222:
#line 1366 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 223:
#line 1367 "moc.y"
{yyval.string=yyvsp[0].string;}
break;
case 224:
#line 1370 "moc.y"
{ yyval.access=Private; }
break;
case 225:
#line 1371 "moc.y"
{ yyval.access=Protected; }
break;
case 226:
#line 1372 "moc.y"
{ yyval.access=Public; }
break;
case 227:
#line 1375 "moc.y"
{ }
break;
case 228:
#line 1376 "moc.y"
{ }
break;
case 270:
#line 1424 "moc.y"
{ tmpFunc->type = yyvsp[-1].string;
						  tmpFunc->name = yyvsp[0].string; }
break;
case 271:
#line 1427 "moc.y"
{ tmpFunc->type = "int";
						  tmpFunc->name = yyvsp[0].string;
				  if ( tmpFunc->name == g->className )
				      func_warn( "Constructors cannot be"
						 " signals or slots.");
						}
break;
case 272:
#line 1434 "moc.y"
{ tmpFunc->type = "void";
						  tmpFunc->name = "~";
						  tmpFunc->name += yyvsp[0].string;
				       func_warn( "Destructors cannot be"
						  " signals or slots.");
						}
break;
case 273:
#line 1442 "moc.y"
{
						    char *tmp =
							straddSpc(yyvsp[-4].string,yyvsp[-3].string,yyvsp[-2].string,yyvsp[-1].string);
						    tmpFunc->type = rmWS(tmp);
						    delete [] tmp;
						    tmpFunc->name = yyvsp[0].string; }
break;
case 274:
#line 1449 "moc.y"
{ skipFunc = TRUE; }
break;
case 275:
#line 1451 "moc.y"
{ tmpFunc->type =
						      straddSpc(yyvsp[-2].string,yyvsp[-1].string);
						  tmpFunc->name = yyvsp[0].string; }
break;
case 276:
#line 1456 "moc.y"
{ tmpFunc->type =
						      straddSpc(yyvsp[-3].string,yyvsp[-2].string,yyvsp[-1].string);
						  tmpFunc->name = yyvsp[0].string; }
break;
case 277:
#line 1460 "moc.y"
{ operatorError();    }
break;
case 278:
#line 1462 "moc.y"
{ operatorError();    }
break;
case 279:
#line 1465 "moc.y"
{ operatorError();    }
break;
case 280:
#line 1467 "moc.y"
{ operatorError();    }
break;
case 281:
#line 1470 "moc.y"
{ operatorError();    }
break;
case 283:
#line 1476 "moc.y"
{ func_warn("Unexpected variable declaration."); }
break;
case 284:
#line 1479 "moc.y"
{ func_warn("Unexpected variable declaration."); }
break;
case 285:
#line 1481 "moc.y"
{ func_warn("Unexpected enum declaration."); }
break;
case 286:
#line 1483 "moc.y"
{ func_warn("Unexpected using declaration."); }
break;
case 287:
#line 1485 "moc.y"
{ func_warn("Unexpected using declaration."); }
break;
case 288:
#line 1487 "moc.y"
{ classPLevel++;
				  moc_err("Unexpected namespace declaration."); }
break;
case 289:
#line 1490 "moc.y"
{ func_warn("Unexpected class declaration.");}
break;
case 290:
#line 1492 "moc.y"
{ func_warn("Unexpected class declaration.");
				  BEGIN IN_FCT; fctLevel=1;
				}
break;
case 291:
#line 1495 "moc.y"
{ BEGIN QT_DEF; }
break;
case 295:
#line 1504 "moc.y"
{ }
break;
case 296:
#line 1505 "moc.y"
{ expLevel = 0; }
break;
case 298:
#line 1507 "moc.y"
{ expLevel = 0; }
break;
case 301:
#line 1512 "moc.y"
{ expLevel = 0; }
break;
case 306:
#line 1527 "moc.y"
{ BEGIN QT_DEF;
				  if ( tmpAccess == Public) {
				      tmpEnum->name = yyvsp[-4].string;
				      addEnum();
				  }
				}
break;
case 307:
#line 1534 "moc.y"
{ tmpEnum->clear();}
break;
case 309:
#line 1538 "moc.y"
{ }
break;
case 313:
#line 1546 "moc.y"
{ if ( tmpAccess == Public) tmpEnum->append( yyvsp[0].string ); }
break;
case 314:
#line 1547 "moc.y"
{ enumLevel=0; }
break;
case 315:
#line 1548 "moc.y"
{  if ( tmpAccess == Public) tmpEnum->append( yyvsp[-3].string );  }
break;
case 316:
#line 1552 "moc.y"
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
break;
case 317:
#line 1568 "moc.y"
{
				    if ( g->propRead.isEmpty() && !g->propOverride )
					moc_err( "A property must at least feature a read method." );
				    checkPropertyName( yyvsp[-2].string );
				    Q_PROPERTYdetected = TRUE;
				    /* Avoid duplicates*/
				    for( QPtrListIterator<Property> lit( g->props ); lit.current(); ++lit ) {
					if ( lit.current()->name == yyvsp[-2].string ) {
					    if ( displayWarnings )
						moc_err( "Property '%s' defined twice.",
							 (const char*)lit.current()->name );
					}
				    }
				    g->props.append( new Property( lineNo, yyvsp[-3].string, yyvsp[-2].string,
								g->propWrite, g->propRead, g->propReset,
								   g->propStored, g->propDesignable,
								   g->propScriptable, g->propOverride ) );
				}
break;
case 319:
#line 1589 "moc.y"
{ g->propRead = yyvsp[-1].string; }
break;
case 320:
#line 1590 "moc.y"
{ g->propWrite = yyvsp[-1].string; }
break;
case 321:
#line 1591 "moc.y"
{ g->propReset = yyvsp[-1].string; }
break;
case 322:
#line 1592 "moc.y"
{ g->propStored = yyvsp[-1].string; }
break;
case 323:
#line 1593 "moc.y"
{ g->propDesignable = yyvsp[-1].string; }
break;
case 324:
#line 1594 "moc.y"
{ g->propScriptable = yyvsp[-1].string; }
break;
case 325:
#line 1597 "moc.y"
{ }
break;
case 326:
#line 1598 "moc.y"
{ g->qtEnums.append( yyvsp[-1].string ); }
break;
case 327:
#line 1601 "moc.y"
{ }
break;
case 328:
#line 1602 "moc.y"
{ g->qtSets.append( yyvsp[-1].string ); }
break;
#line 4759 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
