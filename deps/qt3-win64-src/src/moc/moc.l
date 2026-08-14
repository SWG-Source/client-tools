/****************************************************************************
** $Id: moc.l 2051 2007-02-21 10:04:20Z chehrlic $
**
** Lexical analyzer for meta object compiler
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
*****************************************************************************/

%{
#ifdef MOC_YACC_CODE

#ifdef MOC_MWERKS_PLUGIN
#ifdef Q_OS_MAC9
# define isascii(c)   ((int)( (unsigned int) (c) <= (unsigned char)0x7F ))
#endif
const char *buf_buffer = NULL;
long buf_size_total = 0, buf_index = 0;
#define YY_INPUT(buf, result, max_size) \
     { \
          if(buf_index < buf_size_total ) { \
              while(!isascii(buf_buffer[buf_index])) buf_index++; \
              int ms = ((max_size < buf_size_total) ? max_size : buf_size_total); \
              for(result = 0; result < ms; result++) { \
                 char c = buf_buffer[buf_index + result]; \
                 if(!isascii(c)) { \
                    buf_index++; \
                    break; \
                 } \
                 buf[result] = c == '\r' ? '\n' : c; \
              } \
              buf_index += result; \
          } else result = YY_NULL; \
     }
#endif

#include "qstring.h"


#define input yyinput				// yyinput in C++

#define X if(lexDebug){fprintf(stderr,"LEX (%i) : %s\n",lineNo,yytext);};
#define Y if(lexDebug){fprintf(stderr,"LEX (%i) : %s\n",lineNo,yytext);};
/*
#define Y if(lexDebug){fprintf(stderr,"LEX (%i) : %s updates level to %i\n"\
                                 ,lineNo,yytext,templLevel);};
*/
#define Z if(lexDebug){fprintf(stderr,"LEX (%i) : skipped the string %s\"\n"\
                               ,lineNo,yytext);};
#define BEGIN_INSIDE


#define linput() \
    ( (c = input()) == '\n' ? (lineNo++, c) : (c == EOF) ? 0 : c )

#include <string.h>
#include <stdlib.h>

int classPLevel      = 1;  /* Depth of nested curly braces in IN_CLASS */
int namespacePLevel  = 1;  /* Depth of nested curly braces in IN_NAMESPACE */
int expLevel         = 1;  /* Depth of nested parentheses in IN_EXPR */
int enumLevel         = 1;  /* Depth of nested parentheses in IN_ENUM */
int fctLevel         = 1;  /* Depth of nested parentheses in IN_FCT */
int templLevel       = 1;  /* Depth of levels in IN_TEMPL_ARGS */

int lastState        = 0;  /* Remembers the state when a
                              MOC_SKIP_BEGIN is encountered */
int skipLevel        = 0; /* Depth of MOC_SKIP_BEGINs */

class QString;

extern void addExpressionChar( const char );
extern void addExpressionString( const char * );
extern void moc_warn( const char *msg );
%}

%start OUTSIDE QT_DEF IN_CLASS IN_NAMESPACE IN_ENUM IN_EXPR IN_DEF_ARG IN_FCT IN_TEMPL_ARGS GIMME_SEMICOLON SKIP IN_PROPERTY IN_CLASSINFO

ALMOSTSTRING	\"([^"\n\\]|\\(.|\n))*
STRING		{ALMOSTSTRING}\"

%%

<OUTSIDE>"class"		{ X;
				  BEGIN QT_DEF;
				  return CLASS; }
<OUTSIDE>"namespace"		{ X;
				  BEGIN QT_DEF;
				  return NAMESPACE; }
<OUTSIDE>"using"		{ X;
                                  BEGIN QT_DEF;
				  return USING; }
<OUTSIDE>"template"		{ X;
				  BEGIN QT_DEF;
				  return TEMPLATE; }
<QT_DEF>"Q_OBJECT"		{ X; return Q_OBJECT; }
<QT_DEF>"signals"		{ X; return SIGNALS; }
<QT_DEF>"slots"			{ X; return SLOTS; }
<QT_DEF>"Q_CLASSINFO"		{ X; return Q_CLASSINFO; }
<QT_DEF>"Q_PROPERTY"		{ X; return Q_PROPERTY; }
<QT_DEF>"Q_OVERRIDE"		{ X; return Q_OVERRIDE; }
<QT_DEF>"Q_ENUMS"		{ X; return Q_ENUMS; }
<QT_DEF>"Q_SETS"		{ X; return Q_SETS; }

<IN_FCT>"{"			{ fctLevel++;Y; }
<IN_FCT>"}"			{ fctLevel--;Y;if (fctLevel==0){X;return '}';}}
<IN_CLASS>"{"			{ classPLevel++;Y; }
<IN_CLASS>"}"			{ classPLevel--;Y;if (classPLevel == 0)
                                                      {X;return '}';} }
<IN_CLASS>"public"		{ X;if( classPLevel == 1 ) return PUBLIC; }
<IN_CLASS>"protected"		{ X;if( classPLevel == 1 ) return PROTECTED; }
<IN_CLASS>"private"		{ X;if( classPLevel == 1 ) return PRIVATE; }
<IN_CLASS>"signals"		{ X;if( classPLevel == 1 ) return SIGNALS; }
<IN_CLASS>"slots"		{ X;if( classPLevel == 1 ) return SLOTS; }
<IN_CLASS>"Q_CLASSINFO"		{ X;if( classPLevel == 1 ) return Q_CLASSINFO; }
<IN_CLASS>"Q_OBJECT"		{ X;
				  if ( classPLevel == 1 )
				      return Q_OBJECT;
				  else if ( classPLevel > 1 )
				      moc_warn( "Cannot use Q_OBJECT in nested class." );
				}
<IN_CLASS>"Q_PROPERTY"		{ X;if( classPLevel == 1 ) return Q_PROPERTY; }
<IN_CLASS>"Q_OVERRIDE"		{ X;if( classPLevel == 1 ) return Q_OVERRIDE; }
<IN_CLASS>"Q_ENUMS"		{ X;if( classPLevel == 1 ) return Q_ENUMS; }
<IN_CLASS>"Q_SETS"		{ X;if( classPLevel == 1 ) return Q_SETS; }

<IN_NAMESPACE>"{"		{ namespacePLevel++;Y; }
<IN_NAMESPACE>"}"		{ namespacePLevel--;Y;if (namespacePLevel == 0)
                                                          {X;return '}';}}
<IN_NAMESPACE>"class" 		{ X;
                                  BEGIN QT_DEF;
				  return CLASS; }
<IN_NAMESPACE>"template"	{ X;
                                  BEGIN QT_DEF;
                                  return TEMPLATE; }
<IN_NAMESPACE>"namespace"	{ X;
				  BEGIN QT_DEF;
				  return NAMESPACE; }
<IN_NAMESPACE>"using"		{ X;
                                  BEGIN QT_DEF;
				  return USING; }

<IN_PROPERTY>"("		{ X; return '('; }
<IN_PROPERTY>")"		{ X; return ')'; }
<IN_PROPERTY>"READ"		{ X; return READ; }
<IN_PROPERTY>"WRITE"		{ X; return WRITE; }
<IN_PROPERTY>"STORED"		{ X; return STORED; }
<IN_PROPERTY>"RESET"		{ X; return RESET; }
<IN_PROPERTY>"DESIGNABLE"	{ X; return DESIGNABLE; }
<IN_PROPERTY>"SCRIPTABLE"	{ X; return SCRIPTABLE; }


<IN_EXPR>"("			{ expLevel++;X; }
<IN_EXPR>")"			{ expLevel--;Y;if (expLevel == 0)
					{ X; BEGIN QT_DEF; return ')';} }
<IN_EXPR>"["			{ expLevel++;X; }
<IN_EXPR>"]"			{ expLevel--;X;if (expLevel == 0)
					{ X; BEGIN QT_DEF; return ']';} }
<IN_EXPR>","			{ if (expLevel == 0)
					{ X; BEGIN QT_DEF; return ',' ;} }
<IN_EXPR>";"			{ if (expLevel == 0)
					{ X; BEGIN QT_DEF; return ';' ;} }
<IN_DEF_ARG>"("			{ expLevel++;X; }
<IN_DEF_ARG>")"			{ expLevel--;Y;if (expLevel == 0)
					{ X; BEGIN QT_DEF; return ')';} }
<IN_DEF_ARG>"["			{ expLevel++;X; }
<IN_DEF_ARG>"]"			{ expLevel--;X;if (expLevel == 0)
					{ X; BEGIN QT_DEF; return ']';} }
<IN_DEF_ARG>","			{ if (expLevel <= 1)
					{ X; BEGIN QT_DEF; return ',' ;} }
<IN_DEF_ARG>";"			{ if (expLevel == 0)
					{ X; BEGIN QT_DEF; return ';' ;} }
<IN_ENUM>"("			{ enumLevel++;X; }
<IN_ENUM>")"			{ enumLevel--;X; }
<IN_ENUM>"["			{ enumLevel++;X; }
<IN_ENUM>"]"			{ enumLevel--;X }
<IN_ENUM>","			{ if (enumLevel == 0)
					{ X; BEGIN QT_DEF; return ',' ;} }
<IN_ENUM>";"			{ if (enumLevel == 0)
					{ X; BEGIN QT_DEF; return ';' ;} }
<IN_ENUM>"}"			{ if (enumLevel == 0)
					{ X; BEGIN QT_DEF; return '}' ;} }
<IN_TEMPL_ARGS>[[(<]		{ templLevel++;
				  Y;
				  addExpressionChar( yytext[0] );
				}
<IN_TEMPL_ARGS>[])>]		{ templLevel--;
				  Y;
				  if ( templLevel == 0 ) {
				      X;
				      BEGIN QT_DEF;
				      return yytext[0];
				  } else {
				      addExpressionChar( yytext[0] );
				  }
				}
<QT_DEF>"friend"		{ X;return FRIEND; }
<QT_DEF>"typedef"		{ X;return TYPEDEF; }
<QT_DEF>"auto"			{ X;return AUTO; }
<QT_DEF>"register"		{ X;return REGISTER; }
<QT_DEF>"static"		{ X;return STATIC; }
<QT_DEF>"extern"		{ X;return EXTERN; }
<QT_DEF>"inline"		{ X;return INLINE; }
<QT_DEF>"__inline__"		{ X;return INLINE; }
<QT_DEF>"virtual"		{ X;return VIRTUAL; }
<QT_DEF>"const"			{ X;return CONST; }
<QT_DEF>"volatile"		{ X;return VOLATILE; }
<QT_DEF>"char"			{ X;return CHAR; }
<QT_DEF>"short"			{ X;return SHORT; }
<QT_DEF>"int"			{ X;return INT; }
<QT_DEF>"long"			{ X;return LONG; }
<QT_DEF>"signed"		{ X;return SIGNED; }
<QT_DEF>"unsigned"		{ X;return UNSIGNED; }
<QT_DEF>"float"			{ X;return FLOAT; }
<QT_DEF>"double"		{ X;return DOUBLE; }
<QT_DEF>"void"			{ X;return VOID; }
<QT_DEF>"enum"			{ X;return ENUM; }
<QT_DEF>"class"			{ X;return CLASS; }
<QT_DEF>"struct"		{ X;return STRUCT; }
<QT_DEF>"union"			{ X;return UNION; }
<QT_DEF>"asm"			{ X;return ASM; }
<QT_DEF>"private"		{ X;return PRIVATE; }
<QT_DEF>"protected"		{ X;return PROTECTED; }
<QT_DEF>"public"		{ X;return PUBLIC; }
<QT_DEF>"operator"		{ X;return OPERATOR; }
<QT_DEF>"::"			{ X;return DBL_COLON; }
<QT_DEF>"..."			{ X;return TRIPLE_DOT; }
<QT_DEF>"template"		{ X;return TEMPLATE; }
<QT_DEF>"mutable"		{ X;return MUTABLE; }
<QT_DEF>"throw"			{ X;return THROW; }
<QT_DEF>"using"		        { X;return USING; }
<QT_DEF>"namespace"		{ X;return NAMESPACE; }

<QT_DEF>[_a-zA-Z][_a-zA-Z0-9]* {
			  X;
			  yylval.string = qstrdup(yytext);
			  return IDENTIFIER;
			}

<IN_PROPERTY>[_a-zA-Z][_a-zA-Z0-9]* {
			  X;
			  yylval.string = qstrdup(yytext);
			  return IDENTIFIER;
			}

<IN_CLASSINFO>"(" { X; return '('; }
<IN_CLASSINFO>")" { X; return ')'; }
<IN_CLASSINFO>"," { X; return ','; }

<IN_CLASSINFO>{ALMOSTSTRING} {
			  X;
			  yylval.string = qstrdup( yytext + 1 );
			  input();		/* discard the '"' */
			  return STRING;
			}

<OUTSIDE>[_a-zA-Z][_a-zA-Z0-9]* ;
<IN_CLASS>[_a-zA-Z][_a-zA-Z0-9]* ;
<IN_NAMESPACE>[_a-zA-Z][_a-zA-Z0-9]* ;

<OUTSIDE>{STRING} {				/* discard strings */
			  Z;
			}

<IN_CLASS>{STRING} {				/* discard strings */
			  Z;
			}

<IN_NAMESPACE>{STRING}	{			/* discard strings */
			  Z;
			}

<IN_FCT>{ALMOSTSTRING} {			/* discard strings */
			  Z;
			  addExpressionString( yytext );
			  input();		/* discard the '"' */
			}


<IN_TEMPL_ARGS>{ALMOSTSTRING} {
			  X;
			  addExpressionString( yytext );
			  input();		/* discard the '"' */
			  return STRING;
			}

<QT_DEF>{ALMOSTSTRING} {
			  X;
			  yylval.string = qstrdup( yytext + 1 );
			  input();		/* discard the '"' */
			  return STRING;
			}

<QT_DEF>'.'	     { X;
			  yylval.char_val = yytext[1];
			  return CHAR_VAL;
			}

<QT_DEF>'\\a'	    { X;
			 yylval.char_val = '\a';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\b'	    { X;
			 yylval.char_val = '\b';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\f'	    { X;
			 yylval.char_val = '\f';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\n'	    { X;
			 yylval.char_val = '\n';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\r'	    { X;
			 yylval.char_val = '\r';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\t'	    { X;
			 yylval.char_val = '\t';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\v'	    { X;
			 yylval.char_val = '\v';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\\\'	    { X;
			 yylval.char_val = '\\';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\?'	    { X;
			 yylval.char_val = '\?';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\''	    { X;
			 yylval.char_val = '\'';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\\"'	    { X;
			 yylval.char_val = '\"';	/* " */
			 return CHAR_VAL;
		       }

<QT_DEF>'\\0'	    { X;
			 yylval.char_val = '\0';
			 return CHAR_VAL;
		       }

<QT_DEF>'\\[0-7]+'  { X;
			 yylval.char_val =
			   (char)strtol( &yytext[1], 0, 8 );
			 return CHAR_VAL;
		       }

<QT_DEF>'\\x[0-9a-fA-F]+' { X;
			 yylval.char_val =
			   (char)strtol( &yytext[2], 0, 16 );
			 return CHAR_VAL;
		       }

<QT_DEF>'\\.'	    { X;
			 yylval.char_val = ' ';
			 return CHAR_VAL;
		       }

<QT_DEF>[0-9]+	     { X;
			  yylval.int_val = atoi(yytext);
			  return INT_VAL;
			}

<QT_DEF>[0-9]+\.[0-9]* { X;
			  yylval.double_val = atof(yytext);
			  return DOUBLE_VAL;
			}

<QT_DEF>\.[0-9]+     { X;
			  yylval.double_val = atof(yytext);
			  return DOUBLE_VAL;
			}


^#define.*\\$		{ /* skip multi-line macro-definitions */
			  int c, c1;
			  input();   /* Discard the '\n'. */
			  do {
			      c1=' ';
			      while((c = linput()) != '\n' && c != 0) c1=c;
			      if (c == 0) break;
			  } while(c1=='\\');
			  unput(c);  /* put back the '\n' or the EOF */
			}

^[ \t]*#.*		{ /* preprocessor commands are skipped */}
"//"[^\n]*		{ /* C++ comment */
			  QCString s = yytext;
                          if ( s.contains( "MOC_SKIP_BEGIN" ) ) {
                            skipLevel++;
                            if ( skipLevel == 1 ) {
                              lastState = YYSTATE;
                              BEGIN SKIP;
                            }
                          } else
                          if ( s.contains( "MOC_SKIP_END" ) ) {
                            if ( skipLevel == 0 ) {
                              moc_warn(" MOC_SKIP_END without MOC_SKIP_BEGIN");
                            } else {
                              skipLevel--;
                              if ( skipLevel == 0 ) {
                                BEGIN lastState;
                              }
                            }
                          }
                        }
"/*"			{ /* C comment */
			  int c = ' ';
			  do {
			      if ( c!= '*' ) {
				  while((c = linput()) != '*' && c != 0)
				      ;
			      }
			      if (c == 0) break;
			  } while(((c = linput())) != '/' && c != 0);
			  if (c == 0)
			      unput(c);
			}

<IN_TEMPL_ARGS>.	  { addExpressionChar( yytext[0] ); }

<IN_TEMPL_ARGS>[ \t\r\b\f]+	  {
				    /* spaces are important in template args,
				       e.g. Foo<const int> */
				    addExpressionChar( yytext[0] ); }
[ \t\r\b\f]+		;
<IN_CLASS>.		;
<IN_NAMESPACE>.		;
<IN_FCT>.		;
<IN_EXPR>.		  { addExpressionChar( yytext[0] ); }
<IN_DEF_ARG>.		;
<IN_ENUM>.		  { addExpressionChar( yytext[0] ); }
<OUTSIDE>.		;
<SKIP>.		        ;
<QT_DEF>.		{
			  X;
			  return yytext[0];
			}
<GIMME_SEMICOLON>.	{
			  X;
			  return ';';
			}
\n			{
			  lineNo++;
			}


%%

#endif // MOC_YACC_CODE
