/**********************************************************************
** $Id: qpsprinter.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QPSPrinter class
**
** Created : 941003
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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
**********************************************************************/

#include "qplatformdefs.h"

// POSIX Large File Support redefines open -> open64
#if defined(open)
# undef open
#endif

// POSIX Large File Support redefines truncate -> truncate64
#if defined(truncate)
# undef truncate
#endif

#include "qpsprinter_p.h"

#ifndef QT_NO_PRINTER

#undef Q_PRINTER_USE_TYPE42

#include "qpainter.h"
#include "qapplication.h"
#include "qpaintdevicemetrics.h"
#include "qimage.h"
#include "qdatetime.h"
#include "qstring.h"
#include "qdict.h"
#include "qmemarray.h"
#include "qfile.h"
#include "qbuffer.h"
#include "qintdict.h"
#include "qtextcodec.h"
#include "qsettings.h"
#include "qmap.h"
#include "qfontdatabase.h"
#include "qregexp.h"
#include "qbitmap.h"
#include <private/qunicodetables_p.h>

#if defined(Q_OS_WIN32)
#include <io.h>
#ifdef Q_PRINTER_USE_TYPE42
#include <stdlib.h>
#endif
#else
#include <unistd.h>
#include <stdlib.h>
#endif

#ifdef Q_WS_X11
#include "qt_x11_p.h"
#ifdef None
#undef None
#endif
#ifdef GrayScale
#undef GrayScale
#endif
#endif

#if defined( Q_WS_X11 ) || defined (Q_WS_QWS)
#include "qfontdata_p.h"
#include "qfontengine_p.h"
#include "qtextlayout_p.h"
#include "qtextengine_p.h"
extern bool qt_has_xft;
#endif

static bool qt_gen_epsf = FALSE;
static bool embedFonts = TRUE;

Q_EXPORT void qt_generate_epsf( bool b )
{
    qt_gen_epsf = b;
}

static const char *const ps_header =
"/d/def load def/D{bind d}bind d/d2{dup dup}D/B{0 d2}D/W{255 d2}D/ED{exch d}D\n"
"/D0{0 ED}D/LT{lineto}D/MT{moveto}D/S{stroke}D/F{setfont}D/SW{setlinewidth}D\n"
"/CP{closepath}D/RL{rlineto}D/NP{newpath}D/CM{currentmatrix}D/SM{setmatrix}D\n"
"/TR{translate}D/SD{setdash}D/SC{aload pop setrgbcolor}D/CR{currentfile read\n"
"pop}D/i{index}D/bs{bitshift}D/scs{setcolorspace}D/DB{dict dup begin}D/DE{end\n"
"d}D/ie{ifelse}D/sp{astore pop}D/BSt 0 d/LWi 1 d/PSt 1 d/Cx 0 d/Cy 0 d/WFi\n"
"false d/OMo false d/BCol[1 1 1]d/PCol[0 0 0]d/BkCol[1 1 1]d/BDArr[0.94 0.88\n"
"0.63 0.50 0.37 0.12 0.06]d/defM matrix d/nS 0 d/GPS{PSt 1 ge PSt 5 le and{{\n"
"LArr PSt 1 sub 2 mul get}{LArr PSt 2 mul 1 sub get}ie}{[]}ie}D/QS{PSt 0 ne{\n"
"gsave LWi SW true GPS 0 SD S OMo PSt 1 ne and{BkCol SC false GPS dup 0 get\n"
"SD S}if grestore}if}D/r28{{CR dup 32 gt{exit}if pop}loop 3{CR}repeat 0 4{7\n"
"bs exch dup 128 gt{84 sub}if 42 sub 127 and add}repeat}D/rA 0 d/rL 0 d/rB{rL\n"
"0 eq{/rA r28 d/rL 28 d}if dup rL gt{rA exch rL sub rL exch/rA 0 d/rL 0 d rB\n"
"exch bs add}{dup rA 16#fffffff 3 -1 roll bs not and exch dup rL exch sub/rL\n"
"ED neg rA exch bs/rA ED}ie}D/uc{/rL 0 d 0{dup 2 i length ge{exit}if 1 rB 1\n"
"eq{3 rB dup 3 ge{1 add dup rB 1 i 5 ge{1 i 6 ge{1 i 7 ge{1 i 8 ge{128 add}if\n"
"64 add}if 32 add}if 16 add}if 3 add exch pop}if 3 add exch 10 rB 1 add{dup 3\n"
"i lt{dup}{2 i}ie 4 i 3 i 3 i sub 2 i getinterval 5 i 4 i 3 -1 roll\n"
"putinterval dup 4 -1 roll add 3 1 roll 4 -1 roll exch sub dup 0 eq{exit}if 3\n"
"1 roll}loop pop pop}{3 rB 1 add{2 copy 8 rB put 1 add}repeat}ie}loop pop}D\n"
"/sl D0/QCIgray D0/QCIcolor D0/QCIindex D0/QCI{/colorimage where{pop false 3\n"
"colorimage}{exec/QCIcolor ED/QCIgray QCIcolor length 3 idiv string d 0 1\n"
"QCIcolor length 3 idiv 1 sub{/QCIindex ED/x QCIindex 3 mul d QCIgray\n"
"QCIindex QCIcolor x get 0.30 mul QCIcolor x 1 add get 0.59 mul QCIcolor x 2\n"
"add get 0.11 mul add add cvi put}for QCIgray image}ie}D/di{gsave TR 1 i 1 eq\n"
"{false eq{pop true 3 1 roll 4 i 4 i false 4 i 4 i imagemask BkCol SC\n"
"imagemask}{pop false 3 1 roll imagemask}ie}{dup false ne{/languagelevel\n"
"where{pop languagelevel 3 ge}{false}ie}{false}ie{/ma ED 8 eq{/dc[0 1]d\n"
"/DeviceGray}{/dc[0 1 0 1 0 1]d/DeviceRGB}ie scs/im ED/mt ED/h ED/w ED/id 7\n"
"DB/ImageType 1 d/Width w d/Height h d/ImageMatrix mt d/DataSource im d\n"
"/BitsPerComponent 8 d/Decode dc d DE/md 7 DB/ImageType 1 d/Width w d/Height\n"
"h d/ImageMatrix mt d/DataSource ma d/BitsPerComponent 1 d/Decode[0 1]d DE 4\n"
"DB/ImageType 3 d/DataDict id d/MaskDict md d/InterleaveType 3 d end image}{\n"
"pop 8 4 1 roll 8 eq{image}{QCI}ie}ie}ie grestore}d/BF{gsave BSt 1 eq{BCol SC\n"
"WFi{fill}{eofill}ie}if BSt 2 ge BSt 8 le and{BDArr BSt 2 sub get/sc ED BCol{\n"
"1. exch sub sc mul 1. exch sub}forall 3 array astore SC WFi{fill}{eofill}ie}\n"
"if BSt 9 ge BSt 14 le and{WFi{clip}{eoclip}ie defM SM pathbbox 3 i 3 i TR 4\n"
"2 roll 3 2 roll exch sub/h ED sub/w ED OMo{NP 0 0 MT 0 h RL w 0 RL 0 h neg\n"
"RL CP BkCol SC fill}if BCol SC 0.3 SW NP BSt 9 eq BSt 11 eq or{0 4 h{dup 0\n"
"exch MT w exch LT}for}if BSt 10 eq BSt 11 eq or{0 4 w{dup 0 MT h LT}for}if\n"
"BSt 12 eq BSt 14 eq or{w h gt{0 6 w h add{dup 0 MT h sub h LT}for}{0 6 w h\n"
"add{dup 0 exch MT w sub w exch LT}for}ie}if BSt 13 eq BSt 14 eq or{w h gt{0\n"
"6 w h add{dup h MT h sub 0 LT}for}{0 6 w h add{dup w exch MT w sub 0 exch LT\n"
"}for}ie}if S}if BSt 24 eq{}if grestore}D/mat matrix d/ang1 D0/ang2 D0/w D0/h\n"
"D0/x D0/y D0/ARC{/ang2 ED/ang1 ED/h ED/w ED/y ED/x ED mat CM pop x w 2 div\n"
"add y h 2 div add TR 1 h w div neg scale ang2 0 ge{0 0 w 2 div ang1 ang1\n"
"ang2 add arc}{0 0 w 2 div ang1 ang1 ang2 add arcn}ie mat SM}D/C D0/P{NP MT\n"
"0.5 0.5 rmoveto 0 -1 RL -1 0 RL 0 1 RL CP fill}D/M{/Cy ED/Cx ED}D/L{NP Cx Cy\n"
"MT/Cy ED/Cx ED Cx Cy LT QS}D/DL{NP MT LT QS}D/HL{1 i DL}D/VL{2 i exch DL}D/R\n"
"{/h ED/w ED/y ED/x ED NP x y MT 0 h RL w 0 RL 0 h neg RL CP BF QS}D/ACR{/h\n"
"ED/w ED/y ED/x ED x y MT 0 h RL w 0 RL 0 h neg RL CP}D/xr D0/yr D0/rx D0/ry\n"
"D0/rx2 D0/ry2 D0/RR{/yr ED/xr ED/h ED/w ED/y ED/x ED xr 0 le yr 0 le or{x y\n"
"w h R}{xr 100 ge yr 100 ge or{x y w h E}{/rx xr w mul 200 div d/ry yr h mul\n"
"200 div d/rx2 rx 2 mul d/ry2 ry 2 mul d NP x rx add y MT x y rx2 ry2 180 -90\n"
"x y h add ry2 sub rx2 ry2 270 -90 x w add rx2 sub y h add ry2 sub rx2 ry2 0\n"
"-90 x w add rx2 sub y rx2 ry2 90 -90 ARC ARC ARC ARC CP BF QS}ie}ie}D/E{/h\n"
"ED/w ED/y ED/x ED mat CM pop x w 2 div add y h 2 div add TR 1 h w div scale\n"
"NP 0 0 w 2 div 0 360 arc mat SM BF QS}D/A{16 div exch 16 div exch NP ARC QS}\n"
"D/PIE{/ang2 ED/ang1 ED/h ED/w ED/y ED/x ED NP x w 2 div add y h 2 div add MT\n"
"x y w h ang1 16 div ang2 16 div ARC CP BF QS}D/CH{16 div exch 16 div exch NP\n"
"ARC CP BF QS}D/BZ{curveto QS}D/CRGB{255 div 3 1 roll 255 div 3 1 roll 255\n"
"div 3 1 roll}D/BC{CRGB BkCol sp}D/BR{CRGB BCol sp/BSt ED}D/WB{1 W BR}D/NB{0\n"
"B BR}D/PE{setlinejoin setlinecap CRGB PCol sp/LWi ED/PSt ED LWi 0 eq{0.25\n"
"/LWi ED}if PCol SC}D/P1{1 0 5 2 roll 0 0 PE}D/ST{defM SM concat}D/MF{true\n"
"exch true exch{exch pop exch pop dup 0 get dup findfont dup/FontName get 3\n"
"-1 roll eq{exit}if}forall exch dup 1 get/fxscale ED 2 get/fslant ED exch\n"
"/fencoding ED[fxscale 0 fslant 1 0 0]makefont fencoding false eq{}{dup\n"
"maxlength dict begin{1 i/FID ne{def}{pop pop}ifelse}forall/Encoding\n"
"fencoding d currentdict end}ie definefont pop}D/MFEmb{findfont dup length\n"
"dict begin{1 i/FID ne{d}{pop pop}ifelse}forall/Encoding ED currentdict end\n"
"definefont pop}D/DF{findfont/fs 3 -1 roll d[fs 0 0 fs -1 mul 0 0]makefont d}\n"
"D/ty 0 d/Y{/ty ED}D/Tl{gsave SW NP 1 i exch MT 1 i 0 RL S grestore}D/XYT{ty\n"
"MT/xyshow where{pop pop xyshow}{exch pop 1 i dup length 2 div exch\n"
"stringwidth pop 3 -1 roll exch sub exch div exch 0 exch ashow}ie}D/AT{ty MT\n"
"1 i dup length 2 div exch stringwidth pop 3 -1 roll exch sub exch div exch 0\n"
"exch ashow}D/QI{/C save d pageinit/Cx 0 d/Cy 0 d/OMo false d}D/QP{C restore\n"
"showpage}D/SPD{/setpagedevice where{1 DB 3 1 roll d end setpagedevice}{pop\n"
"pop}ie}D/SV{BSt LWi PSt Cx Cy WFi OMo BCol PCol BkCol/nS nS 1 add d gsave}D\n"
"/RS{nS 0 gt{grestore/BkCol ED/PCol ED/BCol ED/OMo ED/WFi ED/Cy ED/Cx ED/PSt\n"
"ED/LWi ED/BSt ED/nS nS 1 sub d}if}D/CLSTART{/clipTmp matrix CM d defM SM NP}\n"
"D/CLEND{clip NP clipTmp SM}D/CLO{grestore gsave defM SM}D\n";

// the next table is derived from a list provided by Adobe on its web
// server: http://partners.adobe.com/asn/developer/typeforum/glyphlist.txt

// the start of the header comment:
//
// Name:          Adobe Glyph List
// Table version: 1.2
// Date:          22 Oct 1998
//
// Description:
//
//   The Adobe Glyph List (AGL) list relates Unicode values (UVs) to glyph
//   names, and should be used only as described in the document "Unicode and
//   Glyph Names," at
//   http://partners.adobe.com:80/asn/developer/type/unicodegn.html
//
// IMPORTANT NOTE:
// the list contains glyphs in the private use area of unicode. These should get removed when regenerating the glyphlist.
// also 0 shout be mapped to .notdef
static const struct {
    Q_UINT16 u;
    const char * g;
} unicodetoglyph[] = {
    // grep '^[0-9A-F][0-9A-F][0-9A-F][0-9A-F];' < /tmp/glyphlist.txt | sed -e 's/;/, "/' -e 's-;-" },  // -' -e 's/^/    { 0x/' | sort
    { 0x0000, ".notdef" },
    { 0x0020, "space" },  // SPACE
    { 0x0021, "exclam" },  // EXCLAMATION MARK
    { 0x0022, "quotedbl" },  // QUOTATION MARK
    { 0x0023, "numbersign" },  // NUMBER SIGN
    { 0x0024, "dollar" },  // DOLLAR SIGN
    { 0x0025, "percent" },  // PERCENT SIGN
    { 0x0026, "ampersand" },  // AMPERSAND
    { 0x0027, "quotesingle" },  // APOSTROPHE
    { 0x0028, "parenleft" },  // LEFT PARENTHESIS
    { 0x0029, "parenright" },  // RIGHT PARENTHESIS
    { 0x002A, "asterisk" },  // ASTERISK
    { 0x002B, "plus" },  // PLUS SIGN
    { 0x002C, "comma" },  // COMMA
    { 0x002D, "hyphen" },  // HYPHEN-MINUS
    { 0x002E, "period" },  // FULL STOP
    { 0x002F, "slash" },  // SOLIDUS
    { 0x0030, "zero" },  // DIGIT ZERO
    { 0x0031, "one" },  // DIGIT ONE
    { 0x0032, "two" },  // DIGIT TWO
    { 0x0033, "three" },  // DIGIT THREE
    { 0x0034, "four" },  // DIGIT FOUR
    { 0x0035, "five" },  // DIGIT FIVE
    { 0x0036, "six" },  // DIGIT SIX
    { 0x0037, "seven" },  // DIGIT SEVEN
    { 0x0038, "eight" },  // DIGIT EIGHT
    { 0x0039, "nine" },  // DIGIT NINE
    { 0x003A, "colon" },  // COLON
    { 0x003B, "semicolon" },  // SEMICOLON
    { 0x003C, "less" },  // LESS-THAN SIGN
    { 0x003D, "equal" },  // EQUALS SIGN
    { 0x003E, "greater" },  // GREATER-THAN SIGN
    { 0x003F, "question" },  // QUESTION MARK
    { 0x0040, "at" },  // COMMERCIAL AT
    { 0x0041, "A" },  // LATIN CAPITAL LETTER A
    { 0x0042, "B" },  // LATIN CAPITAL LETTER B
    { 0x0043, "C" },  // LATIN CAPITAL LETTER C
    { 0x0044, "D" },  // LATIN CAPITAL LETTER D
    { 0x0045, "E" },  // LATIN CAPITAL LETTER E
    { 0x0046, "F" },  // LATIN CAPITAL LETTER F
    { 0x0047, "G" },  // LATIN CAPITAL LETTER G
    { 0x0048, "H" },  // LATIN CAPITAL LETTER H
    { 0x0049, "I" },  // LATIN CAPITAL LETTER I
    { 0x004A, "J" },  // LATIN CAPITAL LETTER J
    { 0x004B, "K" },  // LATIN CAPITAL LETTER K
    { 0x004C, "L" },  // LATIN CAPITAL LETTER L
    { 0x004D, "M" },  // LATIN CAPITAL LETTER M
    { 0x004E, "N" },  // LATIN CAPITAL LETTER N
    { 0x004F, "O" },  // LATIN CAPITAL LETTER O
    { 0x0050, "P" },  // LATIN CAPITAL LETTER P
    { 0x0051, "Q" },  // LATIN CAPITAL LETTER Q
    { 0x0052, "R" },  // LATIN CAPITAL LETTER R
    { 0x0053, "S" },  // LATIN CAPITAL LETTER S
    { 0x0054, "T" },  // LATIN CAPITAL LETTER T
    { 0x0055, "U" },  // LATIN CAPITAL LETTER U
    { 0x0056, "V" },  // LATIN CAPITAL LETTER V
    { 0x0057, "W" },  // LATIN CAPITAL LETTER W
    { 0x0058, "X" },  // LATIN CAPITAL LETTER X
    { 0x0059, "Y" },  // LATIN CAPITAL LETTER Y
    { 0x005A, "Z" },  // LATIN CAPITAL LETTER Z
    { 0x005B, "bracketleft" },  // LEFT SQUARE BRACKET
    { 0x005C, "backslash" },  // REVERSE SOLIDUS
    { 0x005D, "bracketright" },  // RIGHT SQUARE BRACKET
    { 0x005E, "asciicircum" },  // CIRCUMFLEX ACCENT
    { 0x005F, "underscore" },  // LOW LINE
    { 0x0060, "grave" },  // GRAVE ACCENT
    { 0x0061, "a" },  // LATIN SMALL LETTER A
    { 0x0062, "b" },  // LATIN SMALL LETTER B
    { 0x0063, "c" },  // LATIN SMALL LETTER C
    { 0x0064, "d" },  // LATIN SMALL LETTER D
    { 0x0065, "e" },  // LATIN SMALL LETTER E
    { 0x0066, "f" },  // LATIN SMALL LETTER F
    { 0x0067, "g" },  // LATIN SMALL LETTER G
    { 0x0068, "h" },  // LATIN SMALL LETTER H
    { 0x0069, "i" },  // LATIN SMALL LETTER I
    { 0x006A, "j" },  // LATIN SMALL LETTER J
    { 0x006B, "k" },  // LATIN SMALL LETTER K
    { 0x006C, "l" },  // LATIN SMALL LETTER L
    { 0x006D, "m" },  // LATIN SMALL LETTER M
    { 0x006E, "n" },  // LATIN SMALL LETTER N
    { 0x006F, "o" },  // LATIN SMALL LETTER O
    { 0x0070, "p" },  // LATIN SMALL LETTER P
    { 0x0071, "q" },  // LATIN SMALL LETTER Q
    { 0x0072, "r" },  // LATIN SMALL LETTER R
    { 0x0073, "s" },  // LATIN SMALL LETTER S
    { 0x0074, "t" },  // LATIN SMALL LETTER T
    { 0x0075, "u" },  // LATIN SMALL LETTER U
    { 0x0076, "v" },  // LATIN SMALL LETTER V
    { 0x0077, "w" },  // LATIN SMALL LETTER W
    { 0x0078, "x" },  // LATIN SMALL LETTER X
    { 0x0079, "y" },  // LATIN SMALL LETTER Y
    { 0x007A, "z" },  // LATIN SMALL LETTER Z
    { 0x007B, "braceleft" },  // LEFT CURLY BRACKET
    { 0x007C, "bar" },  // VERTICAL LINE
    { 0x007D, "braceright" },  // RIGHT CURLY BRACKET
    { 0x007E, "asciitilde" },  // TILDE
    { 0x00A0, "space" },  // NO-BREAK SPACE;Duplicate
    { 0x00A1, "exclamdown" },  // INVERTED EXCLAMATION MARK
    { 0x00A2, "cent" },  // CENT SIGN
    { 0x00A3, "sterling" },  // POUND SIGN
    { 0x00A4, "currency" },  // CURRENCY SIGN
    { 0x00A5, "yen" },  // YEN SIGN
    { 0x00A6, "brokenbar" },  // BROKEN BAR
    { 0x00A7, "section" },  // SECTION SIGN
    { 0x00A8, "dieresis" },  // DIAERESIS
    { 0x00A9, "copyright" },  // COPYRIGHT SIGN
    { 0x00AA, "ordfeminine" },  // FEMININE ORDINAL INDICATOR
    { 0x00AB, "guillemotleft" },  // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
    { 0x00AC, "logicalnot" },  // NOT SIGN
    { 0x00AD, "hyphen" },  // SOFT HYPHEN;Duplicate
    { 0x00AE, "registered" },  // REGISTERED SIGN
    { 0x00AF, "macron" },  // MACRON
    { 0x00B0, "degree" },  // DEGREE SIGN
    { 0x00B1, "plusminus" },  // PLUS-MINUS SIGN
    { 0x00B2, "twosuperior" },  // SUPERSCRIPT TWO
    { 0x00B3, "threesuperior" },  // SUPERSCRIPT THREE
    { 0x00B4, "acute" },  // ACUTE ACCENT
    { 0x00B5, "mu" },  // MICRO SIGN
    { 0x00B6, "paragraph" },  // PILCROW SIGN
    { 0x00B7, "periodcentered" },  // MIDDLE DOT
    { 0x00B8, "cedilla" },  // CEDILLA
    { 0x00B9, "onesuperior" },  // SUPERSCRIPT ONE
    { 0x00BA, "ordmasculine" },  // MASCULINE ORDINAL INDICATOR
    { 0x00BB, "guillemotright" }, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
    { 0x00BC, "onequarter" },  // VULGAR FRACTION ONE QUARTER
    { 0x00BD, "onehalf" },  // VULGAR FRACTION ONE HALF
    { 0x00BE, "threequarters" },  // VULGAR FRACTION THREE QUARTERS
    { 0x00BF, "questiondown" },  // INVERTED QUESTION MARK
    { 0x00C0, "Agrave" },  // LATIN CAPITAL LETTER A WITH GRAVE
    { 0x00C1, "Aacute" },  // LATIN CAPITAL LETTER A WITH ACUTE
    { 0x00C2, "Acircumflex" },  // LATIN CAPITAL LETTER A WITH CIRCUMFLEX
    { 0x00C3, "Atilde" },  // LATIN CAPITAL LETTER A WITH TILDE
    { 0x00C4, "Adieresis" },  // LATIN CAPITAL LETTER A WITH DIAERESIS
    { 0x00C5, "Aring" },  // LATIN CAPITAL LETTER A WITH RING ABOVE
    { 0x00C6, "AE" },  // LATIN CAPITAL LETTER AE
    { 0x00C7, "Ccedilla" },  // LATIN CAPITAL LETTER C WITH CEDILLA
    { 0x00C8, "Egrave" },  // LATIN CAPITAL LETTER E WITH GRAVE
    { 0x00C9, "Eacute" },  // LATIN CAPITAL LETTER E WITH ACUTE
    { 0x00CA, "Ecircumflex" },  // LATIN CAPITAL LETTER E WITH CIRCUMFLEX
    { 0x00CB, "Edieresis" },  // LATIN CAPITAL LETTER E WITH DIAERESIS
    { 0x00CC, "Igrave" },  // LATIN CAPITAL LETTER I WITH GRAVE
    { 0x00CD, "Iacute" },  // LATIN CAPITAL LETTER I WITH ACUTE
    { 0x00CE, "Icircumflex" },  // LATIN CAPITAL LETTER I WITH CIRCUMFLEX
    { 0x00CF, "Idieresis" },  // LATIN CAPITAL LETTER I WITH DIAERESIS
    { 0x00D0, "Eth" },  // LATIN CAPITAL LETTER ETH
    { 0x00D1, "Ntilde" },  // LATIN CAPITAL LETTER N WITH TILDE
    { 0x00D2, "Ograve" },  // LATIN CAPITAL LETTER O WITH GRAVE
    { 0x00D3, "Oacute" },  // LATIN CAPITAL LETTER O WITH ACUTE
    { 0x00D4, "Ocircumflex" },  // LATIN CAPITAL LETTER O WITH CIRCUMFLEX
    { 0x00D5, "Otilde" },  // LATIN CAPITAL LETTER O WITH TILDE
    { 0x00D6, "Odieresis" },  // LATIN CAPITAL LETTER O WITH DIAERESIS
    { 0x00D7, "multiply" },  // MULTIPLICATION SIGN
    { 0x00D8, "Oslash" },  // LATIN CAPITAL LETTER O WITH STROKE
    { 0x00D9, "Ugrave" },  // LATIN CAPITAL LETTER U WITH GRAVE
    { 0x00DA, "Uacute" },  // LATIN CAPITAL LETTER U WITH ACUTE
    { 0x00DB, "Ucircumflex" },  // LATIN CAPITAL LETTER U WITH CIRCUMFLEX
    { 0x00DC, "Udieresis" },  // LATIN CAPITAL LETTER U WITH DIAERESIS
    { 0x00DD, "Yacute" },  // LATIN CAPITAL LETTER Y WITH ACUTE
    { 0x00DE, "Thorn" },  // LATIN CAPITAL LETTER THORN
    { 0x00DF, "germandbls" },  // LATIN SMALL LETTER SHARP S
    { 0x00E0, "agrave" },  // LATIN SMALL LETTER A WITH GRAVE
    { 0x00E1, "aacute" },  // LATIN SMALL LETTER A WITH ACUTE
    { 0x00E2, "acircumflex" },  // LATIN SMALL LETTER A WITH CIRCUMFLEX
    { 0x00E3, "atilde" },  // LATIN SMALL LETTER A WITH TILDE
    { 0x00E4, "adieresis" },  // LATIN SMALL LETTER A WITH DIAERESIS
    { 0x00E5, "aring" },  // LATIN SMALL LETTER A WITH RING ABOVE
    { 0x00E6, "ae" },  // LATIN SMALL LETTER AE
    { 0x00E7, "ccedilla" },  // LATIN SMALL LETTER C WITH CEDILLA
    { 0x00E8, "egrave" },  // LATIN SMALL LETTER E WITH GRAVE
    { 0x00E9, "eacute" },  // LATIN SMALL LETTER E WITH ACUTE
    { 0x00EA, "ecircumflex" },  // LATIN SMALL LETTER E WITH CIRCUMFLEX
    { 0x00EB, "edieresis" },  // LATIN SMALL LETTER E WITH DIAERESIS
    { 0x00EC, "igrave" },  // LATIN SMALL LETTER I WITH GRAVE
    { 0x00ED, "iacute" },  // LATIN SMALL LETTER I WITH ACUTE
    { 0x00EE, "icircumflex" },  // LATIN SMALL LETTER I WITH CIRCUMFLEX
    { 0x00EF, "idieresis" },  // LATIN SMALL LETTER I WITH DIAERESIS
    { 0x00F0, "eth" },  // LATIN SMALL LETTER ETH
    { 0x00F1, "ntilde" },  // LATIN SMALL LETTER N WITH TILDE
    { 0x00F2, "ograve" },  // LATIN SMALL LETTER O WITH GRAVE
    { 0x00F3, "oacute" },  // LATIN SMALL LETTER O WITH ACUTE
    { 0x00F4, "ocircumflex" },  // LATIN SMALL LETTER O WITH CIRCUMFLEX
    { 0x00F5, "otilde" },  // LATIN SMALL LETTER O WITH TILDE
    { 0x00F6, "odieresis" },  // LATIN SMALL LETTER O WITH DIAERESIS
    { 0x00F7, "divide" },  // DIVISION SIGN
    { 0x00F8, "oslash" },  // LATIN SMALL LETTER O WITH STROKE
    { 0x00F9, "ugrave" },  // LATIN SMALL LETTER U WITH GRAVE
    { 0x00FA, "uacute" },  // LATIN SMALL LETTER U WITH ACUTE
    { 0x00FB, "ucircumflex" },  // LATIN SMALL LETTER U WITH CIRCUMFLEX
    { 0x00FC, "udieresis" },  // LATIN SMALL LETTER U WITH DIAERESIS
    { 0x00FD, "yacute" },  // LATIN SMALL LETTER Y WITH ACUTE
    { 0x00FE, "thorn" },  // LATIN SMALL LETTER THORN
    { 0x00FF, "ydieresis" },  // LATIN SMALL LETTER Y WITH DIAERESIS
    { 0x0100, "Amacron" },  // LATIN CAPITAL LETTER A WITH MACRON
    { 0x0101, "amacron" },  // LATIN SMALL LETTER A WITH MACRON
    { 0x0102, "Abreve" },  // LATIN CAPITAL LETTER A WITH BREVE
    { 0x0103, "abreve" },  // LATIN SMALL LETTER A WITH BREVE
    { 0x0104, "Aogonek" },  // LATIN CAPITAL LETTER A WITH OGONEK
    { 0x0105, "aogonek" },  // LATIN SMALL LETTER A WITH OGONEK
    { 0x0106, "Cacute" },  // LATIN CAPITAL LETTER C WITH ACUTE
    { 0x0107, "cacute" },  // LATIN SMALL LETTER C WITH ACUTE
    { 0x0108, "Ccircumflex" },  // LATIN CAPITAL LETTER C WITH CIRCUMFLEX
    { 0x0109, "ccircumflex" },  // LATIN SMALL LETTER C WITH CIRCUMFLEX
    { 0x010A, "Cdotaccent" },  // LATIN CAPITAL LETTER C WITH DOT ABOVE
    { 0x010B, "cdotaccent" },  // LATIN SMALL LETTER C WITH DOT ABOVE
    { 0x010C, "Ccaron" },  // LATIN CAPITAL LETTER C WITH CARON
    { 0x010D, "ccaron" },  // LATIN SMALL LETTER C WITH CARON
    { 0x010E, "Dcaron" },  // LATIN CAPITAL LETTER D WITH CARON
    { 0x010F, "dcaron" },  // LATIN SMALL LETTER D WITH CARON
    { 0x0110, "Dcroat" },  // LATIN CAPITAL LETTER D WITH STROKE
    { 0x0111, "dcroat" },  // LATIN SMALL LETTER D WITH STROKE
    { 0x0112, "Emacron" },  // LATIN CAPITAL LETTER E WITH MACRON
    { 0x0113, "emacron" },  // LATIN SMALL LETTER E WITH MACRON
    { 0x0114, "Ebreve" },  // LATIN CAPITAL LETTER E WITH BREVE
    { 0x0115, "ebreve" },  // LATIN SMALL LETTER E WITH BREVE
    { 0x0116, "Edotaccent" },  // LATIN CAPITAL LETTER E WITH DOT ABOVE
    { 0x0117, "edotaccent" },  // LATIN SMALL LETTER E WITH DOT ABOVE
    { 0x0118, "Eogonek" },  // LATIN CAPITAL LETTER E WITH OGONEK
    { 0x0119, "eogonek" },  // LATIN SMALL LETTER E WITH OGONEK
    { 0x011A, "Ecaron" },  // LATIN CAPITAL LETTER E WITH CARON
    { 0x011B, "ecaron" },  // LATIN SMALL LETTER E WITH CARON
    { 0x011C, "Gcircumflex" },  // LATIN CAPITAL LETTER G WITH CIRCUMFLEX
    { 0x011D, "gcircumflex" },  // LATIN SMALL LETTER G WITH CIRCUMFLEX
    { 0x011E, "Gbreve" },  // LATIN CAPITAL LETTER G WITH BREVE
    { 0x011F, "gbreve" },  // LATIN SMALL LETTER G WITH BREVE
    { 0x0120, "Gdotaccent" },  // LATIN CAPITAL LETTER G WITH DOT ABOVE
    { 0x0121, "gdotaccent" },  // LATIN SMALL LETTER G WITH DOT ABOVE
    { 0x0122, "Gcommaaccent" },  // LATIN CAPITAL LETTER G WITH CEDILLA
    { 0x0123, "gcommaaccent" },  // LATIN SMALL LETTER G WITH CEDILLA
    { 0x0124, "Hcircumflex" },  // LATIN CAPITAL LETTER H WITH CIRCUMFLEX
    { 0x0125, "hcircumflex" },  // LATIN SMALL LETTER H WITH CIRCUMFLEX
    { 0x0126, "Hbar" },  // LATIN CAPITAL LETTER H WITH STROKE
    { 0x0127, "hbar" },  // LATIN SMALL LETTER H WITH STROKE
    { 0x0128, "Itilde" },  // LATIN CAPITAL LETTER I WITH TILDE
    { 0x0129, "itilde" },  // LATIN SMALL LETTER I WITH TILDE
    { 0x012A, "Imacron" },  // LATIN CAPITAL LETTER I WITH MACRON
    { 0x012B, "imacron" },  // LATIN SMALL LETTER I WITH MACRON
    { 0x012C, "Ibreve" },  // LATIN CAPITAL LETTER I WITH BREVE
    { 0x012D, "ibreve" },  // LATIN SMALL LETTER I WITH BREVE
    { 0x012E, "Iogonek" },  // LATIN CAPITAL LETTER I WITH OGONEK
    { 0x012F, "iogonek" },  // LATIN SMALL LETTER I WITH OGONEK
    { 0x0130, "Idotaccent" },  // LATIN CAPITAL LETTER I WITH DOT ABOVE
    { 0x0131, "dotlessi" },  // LATIN SMALL LETTER DOTLESS I
    { 0x0132, "IJ" },  // LATIN CAPITAL LIGATURE IJ
    { 0x0133, "ij" },  // LATIN SMALL LIGATURE IJ
    { 0x0134, "Jcircumflex" },  // LATIN CAPITAL LETTER J WITH CIRCUMFLEX
    { 0x0135, "jcircumflex" },  // LATIN SMALL LETTER J WITH CIRCUMFLEX
    { 0x0136, "Kcommaaccent" },  // LATIN CAPITAL LETTER K WITH CEDILLA
    { 0x0137, "kcommaaccent" },  // LATIN SMALL LETTER K WITH CEDILLA
    { 0x0138, "kgreenlandic" },  // LATIN SMALL LETTER KRA
    { 0x0139, "Lacute" },  // LATIN CAPITAL LETTER L WITH ACUTE
    { 0x013A, "lacute" },  // LATIN SMALL LETTER L WITH ACUTE
    { 0x013B, "Lcommaaccent" },  // LATIN CAPITAL LETTER L WITH CEDILLA
    { 0x013C, "lcommaaccent" },  // LATIN SMALL LETTER L WITH CEDILLA
    { 0x013D, "Lcaron" },  // LATIN CAPITAL LETTER L WITH CARON
    { 0x013E, "lcaron" },  // LATIN SMALL LETTER L WITH CARON
    { 0x013F, "Ldot" },  // LATIN CAPITAL LETTER L WITH MIDDLE DOT
    { 0x0140, "ldot" },  // LATIN SMALL LETTER L WITH MIDDLE DOT
    { 0x0141, "Lslash" },  // LATIN CAPITAL LETTER L WITH STROKE
    { 0x0142, "lslash" },  // LATIN SMALL LETTER L WITH STROKE
    { 0x0143, "Nacute" },  // LATIN CAPITAL LETTER N WITH ACUTE
    { 0x0144, "nacute" },  // LATIN SMALL LETTER N WITH ACUTE
    { 0x0145, "Ncommaaccent" },  // LATIN CAPITAL LETTER N WITH CEDILLA
    { 0x0146, "ncommaaccent" },  // LATIN SMALL LETTER N WITH CEDILLA
    { 0x0147, "Ncaron" },  // LATIN CAPITAL LETTER N WITH CARON
    { 0x0148, "ncaron" },  // LATIN SMALL LETTER N WITH CARON
    { 0x0149, "napostrophe" },  // LATIN SMALL LETTER N PRECEDED BY APOSTROPHE
    { 0x014A, "Eng" },  // LATIN CAPITAL LETTER ENG
    { 0x014B, "eng" },  // LATIN SMALL LETTER ENG
    { 0x014C, "Omacron" },  // LATIN CAPITAL LETTER O WITH MACRON
    { 0x014D, "omacron" },  // LATIN SMALL LETTER O WITH MACRON
    { 0x014E, "Obreve" },  // LATIN CAPITAL LETTER O WITH BREVE
    { 0x014F, "obreve" },  // LATIN SMALL LETTER O WITH BREVE
    { 0x0150, "Ohungarumlaut" },  // LATIN CAPITAL LETTER O WITH DOUBLE ACUTE
    { 0x0151, "ohungarumlaut" },  // LATIN SMALL LETTER O WITH DOUBLE ACUTE
    { 0x0152, "OE" },  // LATIN CAPITAL LIGATURE OE
    { 0x0153, "oe" },  // LATIN SMALL LIGATURE OE
    { 0x0154, "Racute" },  // LATIN CAPITAL LETTER R WITH ACUTE
    { 0x0155, "racute" },  // LATIN SMALL LETTER R WITH ACUTE
    { 0x0156, "Rcommaaccent" },  // LATIN CAPITAL LETTER R WITH CEDILLA
    { 0x0157, "rcommaaccent" },  // LATIN SMALL LETTER R WITH CEDILLA
    { 0x0158, "Rcaron" },  // LATIN CAPITAL LETTER R WITH CARON
    { 0x0159, "rcaron" },  // LATIN SMALL LETTER R WITH CARON
    { 0x015A, "Sacute" },  // LATIN CAPITAL LETTER S WITH ACUTE
    { 0x015B, "sacute" },  // LATIN SMALL LETTER S WITH ACUTE
    { 0x015C, "Scircumflex" },  // LATIN CAPITAL LETTER S WITH CIRCUMFLEX
    { 0x015D, "scircumflex" },  // LATIN SMALL LETTER S WITH CIRCUMFLEX
    { 0x015E, "Scedilla" },  // LATIN CAPITAL LETTER S WITH CEDILLA
    { 0x015F, "scedilla" },  // LATIN SMALL LETTER S WITH CEDILLA
    { 0x0160, "Scaron" },  // LATIN CAPITAL LETTER S WITH CARON
    { 0x0161, "scaron" },  // LATIN SMALL LETTER S WITH CARON
    { 0x0162, "Tcommaaccent" },  // LATIN CAPITAL LETTER T WITH CEDILLA
    { 0x0163, "tcommaaccent" },  // LATIN SMALL LETTER T WITH CEDILLA
    { 0x0164, "Tcaron" },  // LATIN CAPITAL LETTER T WITH CARON
    { 0x0165, "tcaron" },  // LATIN SMALL LETTER T WITH CARON
    { 0x0166, "Tbar" },  // LATIN CAPITAL LETTER T WITH STROKE
    { 0x0167, "tbar" },  // LATIN SMALL LETTER T WITH STROKE
    { 0x0168, "Utilde" },  // LATIN CAPITAL LETTER U WITH TILDE
    { 0x0169, "utilde" },  // LATIN SMALL LETTER U WITH TILDE
    { 0x016A, "Umacron" },  // LATIN CAPITAL LETTER U WITH MACRON
    { 0x016B, "umacron" },  // LATIN SMALL LETTER U WITH MACRON
    { 0x016C, "Ubreve" },  // LATIN CAPITAL LETTER U WITH BREVE
    { 0x016D, "ubreve" },  // LATIN SMALL LETTER U WITH BREVE
    { 0x016E, "Uring" },  // LATIN CAPITAL LETTER U WITH RING ABOVE
    { 0x016F, "uring" },  // LATIN SMALL LETTER U WITH RING ABOVE
    { 0x0170, "Uhungarumlaut" },  // LATIN CAPITAL LETTER U WITH DOUBLE ACUTE
    { 0x0171, "uhungarumlaut" },  // LATIN SMALL LETTER U WITH DOUBLE ACUTE
    { 0x0172, "Uogonek" },  // LATIN CAPITAL LETTER U WITH OGONEK
    { 0x0173, "uogonek" },  // LATIN SMALL LETTER U WITH OGONEK
    { 0x0174, "Wcircumflex" },  // LATIN CAPITAL LETTER W WITH CIRCUMFLEX
    { 0x0175, "wcircumflex" },  // LATIN SMALL LETTER W WITH CIRCUMFLEX
    { 0x0176, "Ycircumflex" },  // LATIN CAPITAL LETTER Y WITH CIRCUMFLEX
    { 0x0177, "ycircumflex" },  // LATIN SMALL LETTER Y WITH CIRCUMFLEX
    { 0x0178, "Ydieresis" },  // LATIN CAPITAL LETTER Y WITH DIAERESIS
    { 0x0179, "Zacute" },  // LATIN CAPITAL LETTER Z WITH ACUTE
    { 0x017A, "zacute" },  // LATIN SMALL LETTER Z WITH ACUTE
    { 0x017B, "Zdotaccent" },  // LATIN CAPITAL LETTER Z WITH DOT ABOVE
    { 0x017C, "zdotaccent" },  // LATIN SMALL LETTER Z WITH DOT ABOVE
    { 0x017D, "Zcaron" },  // LATIN CAPITAL LETTER Z WITH CARON
    { 0x017E, "zcaron" },  // LATIN SMALL LETTER Z WITH CARON
    { 0x017F, "longs" },  // LATIN SMALL LETTER LONG S
    { 0x0192, "florin" },  // LATIN SMALL LETTER F WITH HOOK
    { 0x01A0, "Ohorn" },  // LATIN CAPITAL LETTER O WITH HORN
    { 0x01A1, "ohorn" },  // LATIN SMALL LETTER O WITH HORN
    { 0x01AF, "Uhorn" },  // LATIN CAPITAL LETTER U WITH HORN
    { 0x01B0, "uhorn" },  // LATIN SMALL LETTER U WITH HORN
    { 0x01E6, "Gcaron" },  // LATIN CAPITAL LETTER G WITH CARON
    { 0x01E7, "gcaron" },  // LATIN SMALL LETTER G WITH CARON
    { 0x01FA, "Aringacute" },  // LATIN CAPITAL LETTER A WITH RING ABOVE AND ACUTE
    { 0x01FB, "aringacute" },  // LATIN SMALL LETTER A WITH RING ABOVE AND ACUTE
    { 0x01FC, "AEacute" },  // LATIN CAPITAL LETTER AE WITH ACUTE
    { 0x01FD, "aeacute" },  // LATIN SMALL LETTER AE WITH ACUTE
    { 0x01FE, "Oslashacute" },  // LATIN CAPITAL LETTER O WITH STROKE AND ACUTE
    { 0x01FF, "oslashacute" },  // LATIN SMALL LETTER O WITH STROKE AND ACUTE
    { 0x0218, "Scommaaccent" },  // LATIN CAPITAL LETTER S WITH COMMA BELOW
    { 0x0219, "scommaaccent" },  // LATIN SMALL LETTER S WITH COMMA BELOW
    { 0x021A, "Tcommaaccent" },  // LATIN CAPITAL LETTER T WITH COMMA BELOW;Duplicate
    { 0x021B, "tcommaaccent" },  // LATIN SMALL LETTER T WITH COMMA BELOW;Duplicate
    { 0x02BC, "afii57929" },  // MODIFIER LETTER APOSTROPHE
    { 0x02BD, "afii64937" },  // MODIFIER LETTER REVERSED COMMA
    { 0x02C6, "circumflex" },  // MODIFIER LETTER CIRCUMFLEX ACCENT
    { 0x02C7, "caron" },  // CARON
    { 0x02C9, "macron" },  // MODIFIER LETTER MACRON;Duplicate
    { 0x02D8, "breve" },  // BREVE
    { 0x02D9, "dotaccent" },  // DOT ABOVE
    { 0x02DA, "ring" },  // RING ABOVE
    { 0x02DB, "ogonek" },  // OGONEK
    { 0x02DC, "tilde" },  // SMALL TILDE
    { 0x02DD, "hungarumlaut" },  // DOUBLE ACUTE ACCENT
    { 0x0300, "gravecomb" },  // COMBINING GRAVE ACCENT
    { 0x0301, "acutecomb" },  // COMBINING ACUTE ACCENT
    { 0x0303, "tildecomb" },  // COMBINING TILDE
    { 0x0309, "hookabovecomb" },  // COMBINING HOOK ABOVE
    { 0x0323, "dotbelowcomb" },  // COMBINING DOT BELOW
    { 0x0384, "tonos" },  // GREEK TONOS
    { 0x0385, "dieresistonos" },  // GREEK DIALYTIKA TONOS
    { 0x0386, "Alphatonos" },  // GREEK CAPITAL LETTER ALPHA WITH TONOS
    { 0x0387, "anoteleia" },  // GREEK ANO TELEIA
    { 0x0388, "Epsilontonos" },  // GREEK CAPITAL LETTER EPSILON WITH TONOS
    { 0x0389, "Etatonos" },  // GREEK CAPITAL LETTER ETA WITH TONOS
    { 0x038A, "Iotatonos" },  // GREEK CAPITAL LETTER IOTA WITH TONOS
    { 0x038C, "Omicrontonos" },  // GREEK CAPITAL LETTER OMICRON WITH TONOS
    { 0x038E, "Upsilontonos" },  // GREEK CAPITAL LETTER UPSILON WITH TONOS
    { 0x038F, "Omegatonos" },  // GREEK CAPITAL LETTER OMEGA WITH TONOS
    { 0x0390, "iotadieresistonos" },  // GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS
    { 0x0391, "Alpha" },  // GREEK CAPITAL LETTER ALPHA
    { 0x0392, "Beta" },  // GREEK CAPITAL LETTER BETA
    { 0x0393, "Gamma" },  // GREEK CAPITAL LETTER GAMMA
    { 0x0394, "Delta" },  // GREEK CAPITAL LETTER DELTA;Duplicate
    { 0x0395, "Epsilon" },  // GREEK CAPITAL LETTER EPSILON
    { 0x0396, "Zeta" },  // GREEK CAPITAL LETTER ZETA
    { 0x0397, "Eta" },  // GREEK CAPITAL LETTER ETA
    { 0x0398, "Theta" },  // GREEK CAPITAL LETTER THETA
    { 0x0399, "Iota" },  // GREEK CAPITAL LETTER IOTA
    { 0x039A, "Kappa" },  // GREEK CAPITAL LETTER KAPPA
    { 0x039B, "Lambda" },  // GREEK CAPITAL LETTER LAMDA
    { 0x039C, "Mu" },  // GREEK CAPITAL LETTER MU
    { 0x039D, "Nu" },  // GREEK CAPITAL LETTER NU
    { 0x039E, "Xi" },  // GREEK CAPITAL LETTER XI
    { 0x039F, "Omicron" },  // GREEK CAPITAL LETTER OMICRON
    { 0x03A0, "Pi" },  // GREEK CAPITAL LETTER PI
    { 0x03A1, "Rho" },  // GREEK CAPITAL LETTER RHO
    { 0x03A3, "Sigma" },  // GREEK CAPITAL LETTER SIGMA
    { 0x03A4, "Tau" },  // GREEK CAPITAL LETTER TAU
    { 0x03A5, "Upsilon" },  // GREEK CAPITAL LETTER UPSILON
    { 0x03A6, "Phi" },  // GREEK CAPITAL LETTER PHI
    { 0x03A7, "Chi" },  // GREEK CAPITAL LETTER CHI
    { 0x03A8, "Psi" },  // GREEK CAPITAL LETTER PSI
    { 0x03A9, "Omega" },  // GREEK CAPITAL LETTER OMEGA;Duplicate
    { 0x03AA, "Iotadieresis" },  // GREEK CAPITAL LETTER IOTA WITH DIALYTIKA
    { 0x03AB, "Upsilondieresis" },  // GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA
    { 0x03AC, "alphatonos" },  // GREEK SMALL LETTER ALPHA WITH TONOS
    { 0x03AD, "epsilontonos" },  // GREEK SMALL LETTER EPSILON WITH TONOS
    { 0x03AE, "etatonos" },  // GREEK SMALL LETTER ETA WITH TONOS
    { 0x03AF, "iotatonos" },  // GREEK SMALL LETTER IOTA WITH TONOS
    { 0x03B0, "upsilondieresistonos" },  // GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS
    { 0x03B1, "alpha" },  // GREEK SMALL LETTER ALPHA
    { 0x03B2, "beta" },  // GREEK SMALL LETTER BETA
    { 0x03B3, "gamma" },  // GREEK SMALL LETTER GAMMA
    { 0x03B4, "delta" },  // GREEK SMALL LETTER DELTA
    { 0x03B5, "epsilon" },  // GREEK SMALL LETTER EPSILON
    { 0x03B6, "zeta" },  // GREEK SMALL LETTER ZETA
    { 0x03B7, "eta" },  // GREEK SMALL LETTER ETA
    { 0x03B8, "theta" },  // GREEK SMALL LETTER THETA
    { 0x03B9, "iota" },  // GREEK SMALL LETTER IOTA
    { 0x03BA, "kappa" },  // GREEK SMALL LETTER KAPPA
    { 0x03BB, "lambda" },  // GREEK SMALL LETTER LAMDA
    { 0x03BC, "mu" },  // GREEK SMALL LETTER MU;Duplicate
    { 0x03BD, "nu" },  // GREEK SMALL LETTER NU
    { 0x03BE, "xi" },  // GREEK SMALL LETTER XI
    { 0x03BF, "omicron" },  // GREEK SMALL LETTER OMICRON
    { 0x03C0, "pi" },  // GREEK SMALL LETTER PI
    { 0x03C1, "rho" },  // GREEK SMALL LETTER RHO
    { 0x03C2, "sigma1" },  // GREEK SMALL LETTER FINAL SIGMA
    { 0x03C3, "sigma" },  // GREEK SMALL LETTER SIGMA
    { 0x03C4, "tau" },  // GREEK SMALL LETTER TAU
    { 0x03C5, "upsilon" },  // GREEK SMALL LETTER UPSILON
    { 0x03C6, "phi" },  // GREEK SMALL LETTER PHI
    { 0x03C7, "chi" },  // GREEK SMALL LETTER CHI
    { 0x03C8, "psi" },  // GREEK SMALL LETTER PSI
    { 0x03C9, "omega" },  // GREEK SMALL LETTER OMEGA
    { 0x03CA, "iotadieresis" },  // GREEK SMALL LETTER IOTA WITH DIALYTIKA
    { 0x03CB, "upsilondieresis" },  // GREEK SMALL LETTER UPSILON WITH DIALYTIKA
    { 0x03CC, "omicrontonos" },  // GREEK SMALL LETTER OMICRON WITH TONOS
    { 0x03CD, "upsilontonos" },  // GREEK SMALL LETTER UPSILON WITH TONOS
    { 0x03CE, "omegatonos" },  // GREEK SMALL LETTER OMEGA WITH TONOS
    { 0x03D1, "theta1" },  // GREEK THETA SYMBOL
    { 0x03D2, "Upsilon1" },  // GREEK UPSILON WITH HOOK SYMBOL
    { 0x03D5, "phi1" },  // GREEK PHI SYMBOL
    { 0x03D6, "omega1" },  // GREEK PI SYMBOL
    { 0x0401, "afii10023" },  // CYRILLIC CAPITAL LETTER IO
    { 0x0402, "afii10051" },  // CYRILLIC CAPITAL LETTER DJE
    { 0x0403, "afii10052" },  // CYRILLIC CAPITAL LETTER GJE
    { 0x0404, "afii10053" },  // CYRILLIC CAPITAL LETTER UKRAINIAN IE
    { 0x0405, "afii10054" },  // CYRILLIC CAPITAL LETTER DZE
    { 0x0406, "afii10055" },  // CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
    { 0x0407, "afii10056" },  // CYRILLIC CAPITAL LETTER YI
    { 0x0408, "afii10057" },  // CYRILLIC CAPITAL LETTER JE
    { 0x0409, "afii10058" },  // CYRILLIC CAPITAL LETTER LJE
    { 0x040A, "afii10059" },  // CYRILLIC CAPITAL LETTER NJE
    { 0x040B, "afii10060" },  // CYRILLIC CAPITAL LETTER TSHE
    { 0x040C, "afii10061" },  // CYRILLIC CAPITAL LETTER KJE
    { 0x040E, "afii10062" },  // CYRILLIC CAPITAL LETTER SHORT U
    { 0x040F, "afii10145" },  // CYRILLIC CAPITAL LETTER DZHE
    { 0x0410, "afii10017" },  // CYRILLIC CAPITAL LETTER A
    { 0x0411, "afii10018" },  // CYRILLIC CAPITAL LETTER BE
    { 0x0412, "afii10019" },  // CYRILLIC CAPITAL LETTER VE
    { 0x0413, "afii10020" },  // CYRILLIC CAPITAL LETTER GHE
    { 0x0414, "afii10021" },  // CYRILLIC CAPITAL LETTER DE
    { 0x0415, "afii10022" },  // CYRILLIC CAPITAL LETTER IE
    { 0x0416, "afii10024" },  // CYRILLIC CAPITAL LETTER ZHE
    { 0x0417, "afii10025" },  // CYRILLIC CAPITAL LETTER ZE
    { 0x0418, "afii10026" },  // CYRILLIC CAPITAL LETTER I
    { 0x0419, "afii10027" },  // CYRILLIC CAPITAL LETTER SHORT I
    { 0x041A, "afii10028" },  // CYRILLIC CAPITAL LETTER KA
    { 0x041B, "afii10029" },  // CYRILLIC CAPITAL LETTER EL
    { 0x041C, "afii10030" },  // CYRILLIC CAPITAL LETTER EM
    { 0x041D, "afii10031" },  // CYRILLIC CAPITAL LETTER EN
    { 0x041E, "afii10032" },  // CYRILLIC CAPITAL LETTER O
    { 0x041F, "afii10033" },  // CYRILLIC CAPITAL LETTER PE
    { 0x0420, "afii10034" },  // CYRILLIC CAPITAL LETTER ER
    { 0x0421, "afii10035" },  // CYRILLIC CAPITAL LETTER ES
    { 0x0422, "afii10036" },  // CYRILLIC CAPITAL LETTER TE
    { 0x0423, "afii10037" },  // CYRILLIC CAPITAL LETTER U
    { 0x0424, "afii10038" },  // CYRILLIC CAPITAL LETTER EF
    { 0x0425, "afii10039" },  // CYRILLIC CAPITAL LETTER HA
    { 0x0426, "afii10040" },  // CYRILLIC CAPITAL LETTER TSE
    { 0x0427, "afii10041" },  // CYRILLIC CAPITAL LETTER CHE
    { 0x0428, "afii10042" },  // CYRILLIC CAPITAL LETTER SHA
    { 0x0429, "afii10043" },  // CYRILLIC CAPITAL LETTER SHCHA
    { 0x042A, "afii10044" },  // CYRILLIC CAPITAL LETTER HARD SIGN
    { 0x042B, "afii10045" },  // CYRILLIC CAPITAL LETTER YERU
    { 0x042C, "afii10046" },  // CYRILLIC CAPITAL LETTER SOFT SIGN
    { 0x042D, "afii10047" },  // CYRILLIC CAPITAL LETTER E
    { 0x042E, "afii10048" },  // CYRILLIC CAPITAL LETTER YU
    { 0x042F, "afii10049" },  // CYRILLIC CAPITAL LETTER YA
    { 0x0430, "afii10065" },  // CYRILLIC SMALL LETTER A
    { 0x0431, "afii10066" },  // CYRILLIC SMALL LETTER BE
    { 0x0432, "afii10067" },  // CYRILLIC SMALL LETTER VE
    { 0x0433, "afii10068" },  // CYRILLIC SMALL LETTER GHE
    { 0x0434, "afii10069" },  // CYRILLIC SMALL LETTER DE
    { 0x0435, "afii10070" },  // CYRILLIC SMALL LETTER IE
    { 0x0436, "afii10072" },  // CYRILLIC SMALL LETTER ZHE
    { 0x0437, "afii10073" },  // CYRILLIC SMALL LETTER ZE
    { 0x0438, "afii10074" },  // CYRILLIC SMALL LETTER I
    { 0x0439, "afii10075" },  // CYRILLIC SMALL LETTER SHORT I
    { 0x043A, "afii10076" },  // CYRILLIC SMALL LETTER KA
    { 0x043B, "afii10077" },  // CYRILLIC SMALL LETTER EL
    { 0x043C, "afii10078" },  // CYRILLIC SMALL LETTER EM
    { 0x043D, "afii10079" },  // CYRILLIC SMALL LETTER EN
    { 0x043E, "afii10080" },  // CYRILLIC SMALL LETTER O
    { 0x043F, "afii10081" },  // CYRILLIC SMALL LETTER PE
    { 0x0440, "afii10082" },  // CYRILLIC SMALL LETTER ER
    { 0x0441, "afii10083" },  // CYRILLIC SMALL LETTER ES
    { 0x0442, "afii10084" },  // CYRILLIC SMALL LETTER TE
    { 0x0443, "afii10085" },  // CYRILLIC SMALL LETTER U
    { 0x0444, "afii10086" },  // CYRILLIC SMALL LETTER EF
    { 0x0445, "afii10087" },  // CYRILLIC SMALL LETTER HA
    { 0x0446, "afii10088" },  // CYRILLIC SMALL LETTER TSE
    { 0x0447, "afii10089" },  // CYRILLIC SMALL LETTER CHE
    { 0x0448, "afii10090" },  // CYRILLIC SMALL LETTER SHA
    { 0x0449, "afii10091" },  // CYRILLIC SMALL LETTER SHCHA
    { 0x044A, "afii10092" },  // CYRILLIC SMALL LETTER HARD SIGN
    { 0x044B, "afii10093" },  // CYRILLIC SMALL LETTER YERU
    { 0x044C, "afii10094" },  // CYRILLIC SMALL LETTER SOFT SIGN
    { 0x044D, "afii10095" },  // CYRILLIC SMALL LETTER E
    { 0x044E, "afii10096" },  // CYRILLIC SMALL LETTER YU
    { 0x044F, "afii10097" },  // CYRILLIC SMALL LETTER YA
    { 0x0451, "afii10071" },  // CYRILLIC SMALL LETTER IO
    { 0x0452, "afii10099" },  // CYRILLIC SMALL LETTER DJE
    { 0x0453, "afii10100" },  // CYRILLIC SMALL LETTER GJE
    { 0x0454, "afii10101" },  // CYRILLIC SMALL LETTER UKRAINIAN IE
    { 0x0455, "afii10102" },  // CYRILLIC SMALL LETTER DZE
    { 0x0456, "afii10103" },  // CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
    { 0x0457, "afii10104" },  // CYRILLIC SMALL LETTER YI
    { 0x0458, "afii10105" },  // CYRILLIC SMALL LETTER JE
    { 0x0459, "afii10106" },  // CYRILLIC SMALL LETTER LJE
    { 0x045A, "afii10107" },  // CYRILLIC SMALL LETTER NJE
    { 0x045B, "afii10108" },  // CYRILLIC SMALL LETTER TSHE
    { 0x045C, "afii10109" },  // CYRILLIC SMALL LETTER KJE
    { 0x045E, "afii10110" },  // CYRILLIC SMALL LETTER SHORT U
    { 0x045F, "afii10193" },  // CYRILLIC SMALL LETTER DZHE
    { 0x0462, "afii10146" },  // CYRILLIC CAPITAL LETTER YAT
    { 0x0463, "afii10194" },  // CYRILLIC SMALL LETTER YAT
    { 0x0472, "afii10147" },  // CYRILLIC CAPITAL LETTER FITA
    { 0x0473, "afii10195" },  // CYRILLIC SMALL LETTER FITA
    { 0x0474, "afii10148" },  // CYRILLIC CAPITAL LETTER IZHITSA
    { 0x0475, "afii10196" },  // CYRILLIC SMALL LETTER IZHITSA
    { 0x0490, "afii10050" },  // CYRILLIC CAPITAL LETTER GHE WITH UPTURN
    { 0x0491, "afii10098" },  // CYRILLIC SMALL LETTER GHE WITH UPTURN
    { 0x04D9, "afii10846" },  // CYRILLIC SMALL LETTER SCHWA
    { 0x05B0, "afii57799" },  // HEBREW POINT SHEVA
    { 0x05B1, "afii57801" },  // HEBREW POINT HATAF SEGOL
    { 0x05B2, "afii57800" },  // HEBREW POINT HATAF PATAH
    { 0x05B3, "afii57802" },  // HEBREW POINT HATAF QAMATS
    { 0x05B4, "afii57793" },  // HEBREW POINT HIRIQ
    { 0x05B5, "afii57794" },  // HEBREW POINT TSERE
    { 0x05B6, "afii57795" },  // HEBREW POINT SEGOL
    { 0x05B7, "afii57798" },  // HEBREW POINT PATAH
    { 0x05B8, "afii57797" },  // HEBREW POINT QAMATS
    { 0x05B9, "afii57806" },  // HEBREW POINT HOLAM
    { 0x05BB, "afii57796" },  // HEBREW POINT QUBUTS
    { 0x05BC, "afii57807" },  // HEBREW POINT DAGESH OR MAPIQ
    { 0x05BD, "afii57839" },  // HEBREW POINT METEG
    { 0x05BE, "afii57645" },  // HEBREW PUNCTUATION MAQAF
    { 0x05BF, "afii57841" },  // HEBREW POINT RAFE
    { 0x05C0, "afii57842" },  // HEBREW PUNCTUATION PASEQ
    { 0x05C1, "afii57804" },  // HEBREW POINT SHIN DOT
    { 0x05C2, "afii57803" },  // HEBREW POINT SIN DOT
    { 0x05C3, "afii57658" },  // HEBREW PUNCTUATION SOF PASUQ
    { 0x05D0, "afii57664" },  // HEBREW LETTER ALEF
    { 0x05D1, "afii57665" },  // HEBREW LETTER BET
    { 0x05D2, "afii57666" },  // HEBREW LETTER GIMEL
    { 0x05D3, "afii57667" },  // HEBREW LETTER DALET
    { 0x05D4, "afii57668" },  // HEBREW LETTER HE
    { 0x05D5, "afii57669" },  // HEBREW LETTER VAV
    { 0x05D6, "afii57670" },  // HEBREW LETTER ZAYIN
    { 0x05D7, "afii57671" },  // HEBREW LETTER HET
    { 0x05D8, "afii57672" },  // HEBREW LETTER TET
    { 0x05D9, "afii57673" },  // HEBREW LETTER YOD
    { 0x05DA, "afii57674" },  // HEBREW LETTER FINAL KAF
    { 0x05DB, "afii57675" },  // HEBREW LETTER KAF
    { 0x05DC, "afii57676" },  // HEBREW LETTER LAMED
    { 0x05DD, "afii57677" },  // HEBREW LETTER FINAL MEM
    { 0x05DE, "afii57678" },  // HEBREW LETTER MEM
    { 0x05DF, "afii57679" },  // HEBREW LETTER FINAL NUN
    { 0x05E0, "afii57680" },  // HEBREW LETTER NUN
    { 0x05E1, "afii57681" },  // HEBREW LETTER SAMEKH
    { 0x05E2, "afii57682" },  // HEBREW LETTER AYIN
    { 0x05E3, "afii57683" },  // HEBREW LETTER FINAL PE
    { 0x05E4, "afii57684" },  // HEBREW LETTER PE
    { 0x05E5, "afii57685" },  // HEBREW LETTER FINAL TSADI
    { 0x05E6, "afii57686" },  // HEBREW LETTER TSADI
    { 0x05E7, "afii57687" },  // HEBREW LETTER QOF
    { 0x05E8, "afii57688" },  // HEBREW LETTER RESH
    { 0x05E9, "afii57689" },  // HEBREW LETTER SHIN
    { 0x05EA, "afii57690" },  // HEBREW LETTER TAV
    { 0x05F0, "afii57716" },  // HEBREW LIGATURE YIDDISH DOUBLE VAV
    { 0x05F1, "afii57717" },  // HEBREW LIGATURE YIDDISH VAV YOD
    { 0x05F2, "afii57718" },  // HEBREW LIGATURE YIDDISH DOUBLE YOD
    { 0x060C, "afii57388" },  // ARABIC COMMA
    { 0x061B, "afii57403" },  // ARABIC SEMICOLON
    { 0x061F, "afii57407" },  // ARABIC QUESTION MARK
    { 0x0621, "afii57409" },  // ARABIC LETTER HAMZA
    { 0x0622, "afii57410" },  // ARABIC LETTER ALEF WITH MADDA ABOVE
    { 0x0623, "afii57411" },  // ARABIC LETTER ALEF WITH HAMZA ABOVE
    { 0x0624, "afii57412" },  // ARABIC LETTER WAW WITH HAMZA ABOVE
    { 0x0625, "afii57413" },  // ARABIC LETTER ALEF WITH HAMZA BELOW
    { 0x0626, "afii57414" },  // ARABIC LETTER YEH WITH HAMZA ABOVE
    { 0x0627, "afii57415" },  // ARABIC LETTER ALEF
    { 0x0628, "afii57416" },  // ARABIC LETTER BEH
    { 0x0629, "afii57417" },  // ARABIC LETTER TEH MARBUTA
    { 0x062A, "afii57418" },  // ARABIC LETTER TEH
    { 0x062B, "afii57419" },  // ARABIC LETTER THEH
    { 0x062C, "afii57420" },  // ARABIC LETTER JEEM
    { 0x062D, "afii57421" },  // ARABIC LETTER HAH
    { 0x062E, "afii57422" },  // ARABIC LETTER KHAH
    { 0x062F, "afii57423" },  // ARABIC LETTER DAL
    { 0x0630, "afii57424" },  // ARABIC LETTER THAL
    { 0x0631, "afii57425" },  // ARABIC LETTER REH
    { 0x0632, "afii57426" },  // ARABIC LETTER ZAIN
    { 0x0633, "afii57427" },  // ARABIC LETTER SEEN
    { 0x0634, "afii57428" },  // ARABIC LETTER SHEEN
    { 0x0635, "afii57429" },  // ARABIC LETTER SAD
    { 0x0636, "afii57430" },  // ARABIC LETTER DAD
    { 0x0637, "afii57431" },  // ARABIC LETTER TAH
    { 0x0638, "afii57432" },  // ARABIC LETTER ZAH
    { 0x0639, "afii57433" },  // ARABIC LETTER AIN
    { 0x063A, "afii57434" },  // ARABIC LETTER GHAIN
    { 0x0640, "afii57440" },  // ARABIC TATWEEL
    { 0x0641, "afii57441" },  // ARABIC LETTER FEH
    { 0x0642, "afii57442" },  // ARABIC LETTER QAF
    { 0x0643, "afii57443" },  // ARABIC LETTER KAF
    { 0x0644, "afii57444" },  // ARABIC LETTER LAM
    { 0x0645, "afii57445" },  // ARABIC LETTER MEEM
    { 0x0646, "afii57446" },  // ARABIC LETTER NOON
    { 0x0647, "afii57470" },  // ARABIC LETTER HEH
    { 0x0648, "afii57448" },  // ARABIC LETTER WAW
    { 0x0649, "afii57449" },  // ARABIC LETTER ALEF MAKSURA
    { 0x064A, "afii57450" },  // ARABIC LETTER YEH
    { 0x064B, "afii57451" },  // ARABIC FATHATAN
    { 0x064C, "afii57452" },  // ARABIC DAMMATAN
    { 0x064D, "afii57453" },  // ARABIC KASRATAN
    { 0x064E, "afii57454" },  // ARABIC FATHA
    { 0x064F, "afii57455" },  // ARABIC DAMMA
    { 0x0650, "afii57456" },  // ARABIC KASRA
    { 0x0651, "afii57457" },  // ARABIC SHADDA
    { 0x0652, "afii57458" },  // ARABIC SUKUN
    { 0x0660, "afii57392" },  // ARABIC-INDIC DIGIT ZERO
    { 0x0661, "afii57393" },  // ARABIC-INDIC DIGIT ONE
    { 0x0662, "afii57394" },  // ARABIC-INDIC DIGIT TWO
    { 0x0663, "afii57395" },  // ARABIC-INDIC DIGIT THREE
    { 0x0664, "afii57396" },  // ARABIC-INDIC DIGIT FOUR
    { 0x0665, "afii57397" },  // ARABIC-INDIC DIGIT FIVE
    { 0x0666, "afii57398" },  // ARABIC-INDIC DIGIT SIX
    { 0x0667, "afii57399" },  // ARABIC-INDIC DIGIT SEVEN
    { 0x0668, "afii57400" },  // ARABIC-INDIC DIGIT EIGHT
    { 0x0669, "afii57401" },  // ARABIC-INDIC DIGIT NINE
    { 0x066A, "afii57381" },  // ARABIC PERCENT SIGN
    { 0x066D, "afii63167" },  // ARABIC FIVE POINTED STAR
    { 0x0679, "afii57511" },  // ARABIC LETTER TTEH
    { 0x067E, "afii57506" },  // ARABIC LETTER PEH
    { 0x0686, "afii57507" },  // ARABIC LETTER TCHEH
    { 0x0688, "afii57512" },  // ARABIC LETTER DDAL
    { 0x0691, "afii57513" },  // ARABIC LETTER RREH
    { 0x0698, "afii57508" },  // ARABIC LETTER JEH
    { 0x06A4, "afii57505" },  // ARABIC LETTER VEH
    { 0x06AF, "afii57509" },  // ARABIC LETTER GAF
    { 0x06BA, "afii57514" },  // ARABIC LETTER NOON GHUNNA
    { 0x06D2, "afii57519" },  // ARABIC LETTER YEH BARREE
    { 0x06D5, "afii57534" },  // ARABIC LETTER AE
    { 0x1E80, "Wgrave" },  // LATIN CAPITAL LETTER W WITH GRAVE
    { 0x1E81, "wgrave" },  // LATIN SMALL LETTER W WITH GRAVE
    { 0x1E82, "Wacute" },  // LATIN CAPITAL LETTER W WITH ACUTE
    { 0x1E83, "wacute" },  // LATIN SMALL LETTER W WITH ACUTE
    { 0x1E84, "Wdieresis" },  // LATIN CAPITAL LETTER W WITH DIAERESIS
    { 0x1E85, "wdieresis" },  // LATIN SMALL LETTER W WITH DIAERESIS
    { 0x1EF2, "Ygrave" },  // LATIN CAPITAL LETTER Y WITH GRAVE
    { 0x1EF3, "ygrave" },  // LATIN SMALL LETTER Y WITH GRAVE
    { 0x200C, "afii61664" },  // ZERO WIDTH NON-JOINER
    { 0x200D, "afii301" },  // ZERO WIDTH JOINER
    { 0x200E, "afii299" },  // LEFT-TO-RIGHT MARK
    { 0x200F, "afii300" },  // RIGHT-TO-LEFT MARK
    { 0x2012, "figuredash" },  // FIGURE DASH
    { 0x2013, "endash" },  // EN DASH
    { 0x2014, "emdash" },  // EM DASH
    { 0x2015, "afii00208" },  // HORIZONTAL BAR
    { 0x2017, "underscoredbl" },  // DOUBLE LOW LINE
    { 0x2018, "quoteleft" },  // LEFT SINGLE QUOTATION MARK
    { 0x2019, "quoteright" },  // RIGHT SINGLE QUOTATION MARK
    { 0x201A, "quotesinglbase" },  // SINGLE LOW-9 QUOTATION MARK
    { 0x201B, "quotereversed" },  // SINGLE HIGH-REVERSED-9 QUOTATION MARK
    { 0x201C, "quotedblleft" },  // LEFT DOUBLE QUOTATION MARK
    { 0x201D, "quotedblright" },  // RIGHT DOUBLE QUOTATION MARK
    { 0x201E, "quotedblbase" },  // DOUBLE LOW-9 QUOTATION MARK
    { 0x2020, "dagger" },  // DAGGER
    { 0x2021, "daggerdbl" },  // DOUBLE DAGGER
    { 0x2022, "bullet" },  // BULLET
    { 0x2024, "onedotenleader" },  // ONE DOT LEADER
    { 0x2025, "twodotenleader" },  // TWO DOT LEADER
    { 0x2026, "ellipsis" },  // HORIZONTAL ELLIPSIS
    { 0x202C, "afii61573" },  // POP DIRECTIONAL FORMATTING
    { 0x202D, "afii61574" },  // LEFT-TO-RIGHT OVERRIDE
    { 0x202E, "afii61575" },  // RIGHT-TO-LEFT OVERRIDE
    { 0x2030, "perthousand" },  // PER MILLE SIGN
    { 0x2032, "minute" },  // PRIME
    { 0x2033, "second" },  // DOUBLE PRIME
    { 0x2039, "guilsinglleft" },  // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
    { 0x203A, "guilsinglright" },  // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
    { 0x203C, "exclamdbl" },  // DOUBLE EXCLAMATION MARK
    { 0x2044, "fraction" },  // FRACTION SLASH
    { 0x2070, "zerosuperior" },  // SUPERSCRIPT ZERO
    { 0x2074, "foursuperior" },  // SUPERSCRIPT FOUR
    { 0x2075, "fivesuperior" },  // SUPERSCRIPT FIVE
    { 0x2076, "sixsuperior" },  // SUPERSCRIPT SIX
    { 0x2077, "sevensuperior" },  // SUPERSCRIPT SEVEN
    { 0x2078, "eightsuperior" },  // SUPERSCRIPT EIGHT
    { 0x2079, "ninesuperior" },  // SUPERSCRIPT NINE
    { 0x207D, "parenleftsuperior" },  // SUPERSCRIPT LEFT PARENTHESIS
    { 0x207E, "parenrightsuperior" },  // SUPERSCRIPT RIGHT PARENTHESIS
    { 0x207F, "nsuperior" },  // SUPERSCRIPT LATIN SMALL LETTER N
    { 0x2080, "zeroinferior" },  // SUBSCRIPT ZERO
    { 0x2081, "oneinferior" },  // SUBSCRIPT ONE
    { 0x2082, "twoinferior" },  // SUBSCRIPT TWO
    { 0x2083, "threeinferior" },  // SUBSCRIPT THREE
    { 0x2084, "fourinferior" },  // SUBSCRIPT FOUR
    { 0x2085, "fiveinferior" },  // SUBSCRIPT FIVE
    { 0x2086, "sixinferior" },  // SUBSCRIPT SIX
    { 0x2087, "seveninferior" },  // SUBSCRIPT SEVEN
    { 0x2088, "eightinferior" },  // SUBSCRIPT EIGHT
    { 0x2089, "nineinferior" },  // SUBSCRIPT NINE
    { 0x208D, "parenleftinferior" },  // SUBSCRIPT LEFT PARENTHESIS
    { 0x208E, "parenrightinferior" },  // SUBSCRIPT RIGHT PARENTHESIS
    { 0x20A1, "colonmonetary" },  // COLON SIGN
    { 0x20A3, "franc" },  // FRENCH FRANC SIGN
    { 0x20A4, "lira" },  // LIRA SIGN
    { 0x20A7, "peseta" },  // PESETA SIGN
    { 0x20AA, "afii57636" },  // NEW SHEQEL SIGN
    { 0x20AB, "dong" },  // DONG SIGN
    { 0x20AC, "Euro" },  // EURO SIGN
    { 0x2105, "afii61248" },  // CARE OF
    { 0x2111, "Ifraktur" },  // BLACK-LETTER CAPITAL I
    { 0x2113, "afii61289" },  // SCRIPT SMALL L
    { 0x2116, "afii61352" },  // NUMERO SIGN
    { 0x2118, "weierstrass" },  // SCRIPT CAPITAL P
    { 0x211C, "Rfraktur" },  // BLACK-LETTER CAPITAL R
    { 0x211E, "prescription" },  // PRESCRIPTION TAKE
    { 0x2122, "trademark" },  // TRADE MARK SIGN
    { 0x2126, "Omega" },  // OHM SIGN
    { 0x212E, "estimated" },  // ESTIMATED SYMBOL
    { 0x2135, "aleph" },  // ALEF SYMBOL
    { 0x2153, "onethird" },  // VULGAR FRACTION ONE THIRD
    { 0x2154, "twothirds" },  // VULGAR FRACTION TWO THIRDS
    { 0x215B, "oneeighth" },  // VULGAR FRACTION ONE EIGHTH
    { 0x215C, "threeeighths" },  // VULGAR FRACTION THREE EIGHTHS
    { 0x215D, "fiveeighths" },  // VULGAR FRACTION FIVE EIGHTHS
    { 0x215E, "seveneighths" },  // VULGAR FRACTION SEVEN EIGHTHS
    { 0x2190, "arrowleft" },  // LEFTWARDS ARROW
    { 0x2191, "arrowup" },  // UPWARDS ARROW
    { 0x2192, "arrowright" },  // RIGHTWARDS ARROW
    { 0x2193, "arrowdown" },  // DOWNWARDS ARROW
    { 0x2194, "arrowboth" },  // LEFT RIGHT ARROW
    { 0x2195, "arrowupdn" },  // UP DOWN ARROW
    { 0x21A8, "arrowupdnbse" },  // UP DOWN ARROW WITH BASE
    { 0x21B5, "carriagereturn" },  // DOWNWARDS ARROW WITH CORNER LEFTWARDS
    { 0x21D0, "arrowdblleft" },  // LEFTWARDS DOUBLE ARROW
    { 0x21D1, "arrowdblup" },  // UPWARDS DOUBLE ARROW
    { 0x21D2, "arrowdblright" },  // RIGHTWARDS DOUBLE ARROW
    { 0x21D3, "arrowdbldown" },  // DOWNWARDS DOUBLE ARROW
    { 0x21D4, "arrowdblboth" },  // LEFT RIGHT DOUBLE ARROW
    { 0x2200, "universal" },  // FOR ALL
    { 0x2202, "partialdiff" },  // PARTIAL DIFFERENTIAL
    { 0x2203, "existential" },  // THERE EXISTS
    { 0x2205, "emptyset" },  // EMPTY SET
    { 0x2206, "Delta" },  // INCREMENT
    { 0x2207, "gradient" },  // NABLA
    { 0x2208, "element" },  // ELEMENT OF
    { 0x2209, "notelement" },  // NOT AN ELEMENT OF
    { 0x220B, "suchthat" },  // CONTAINS AS MEMBER
    { 0x220F, "product" },  // N-ARY PRODUCT
    { 0x2211, "summation" },  // N-ARY SUMMATION
    { 0x2212, "minus" },  // MINUS SIGN
    { 0x2215, "fraction" },  // DIVISION SLASH;Duplicate
    { 0x2217, "asteriskmath" },  // ASTERISK OPERATOR
    { 0x2219, "periodcentered" },  // BULLET OPERATOR;Duplicate
    { 0x221A, "radical" },  // SQUARE ROOT
    { 0x221D, "proportional" },  // PROPORTIONAL TO
    { 0x221E, "infinity" },  // INFINITY
    { 0x221F, "orthogonal" },  // RIGHT ANGLE
    { 0x2220, "angle" },  // ANGLE
    { 0x2227, "logicaland" },  // LOGICAL AND
    { 0x2228, "logicalor" },  // LOGICAL OR
    { 0x2229, "intersection" },  // INTERSECTION
    { 0x222A, "union" },  // UNION
    { 0x222B, "integral" },  // INTEGRAL
    { 0x2234, "therefore" },  // THEREFORE
    { 0x223C, "similar" },  // TILDE OPERATOR
    { 0x2245, "congruent" },  // APPROXIMATELY EQUAL TO
    { 0x2248, "approxequal" },  // ALMOST EQUAL TO
    { 0x2260, "notequal" },  // NOT EQUAL TO
    { 0x2261, "equivalence" },  // IDENTICAL TO
    { 0x2264, "lessequal" },  // LESS-THAN OR EQUAL TO
    { 0x2265, "greaterequal" },  // GREATER-THAN OR EQUAL TO
    { 0x2282, "propersubset" },  // SUBSET OF
    { 0x2283, "propersuperset" },  // SUPERSET OF
    { 0x2284, "notsubset" },  // NOT A SUBSET OF
    { 0x2286, "reflexsubset" },  // SUBSET OF OR EQUAL TO
    { 0x2287, "reflexsuperset" },  // SUPERSET OF OR EQUAL TO
    { 0x2295, "circleplus" },  // CIRCLED PLUS
    { 0x2297, "circlemultiply" },  // CIRCLED TIMES
    { 0x22A5, "perpendicular" },  // UP TACK
    { 0x22C5, "dotmath" },  // DOT OPERATOR
    { 0x2302, "house" },  // HOUSE
    { 0x2310, "revlogicalnot" },  // REVERSED NOT SIGN
    { 0x2320, "integraltp" },  // TOP HALF INTEGRAL
    { 0x2321, "integralbt" },  // BOTTOM HALF INTEGRAL
    { 0x2329, "angleleft" },  // LEFT-POINTING ANGLE BRACKET
    { 0x232A, "angleright" },  // RIGHT-POINTING ANGLE BRACKET
    { 0x2500, "SF100000" },  // BOX DRAWINGS LIGHT HORIZONTAL
    { 0x2502, "SF110000" },  // BOX DRAWINGS LIGHT VERTICAL
    { 0x250C, "SF010000" },  // BOX DRAWINGS LIGHT DOWN AND RIGHT
    { 0x2510, "SF030000" },  // BOX DRAWINGS LIGHT DOWN AND LEFT
    { 0x2514, "SF020000" },  // BOX DRAWINGS LIGHT UP AND RIGHT
    { 0x2518, "SF040000" },  // BOX DRAWINGS LIGHT UP AND LEFT
    { 0x251C, "SF080000" },  // BOX DRAWINGS LIGHT VERTICAL AND RIGHT
    { 0x2524, "SF090000" },  // BOX DRAWINGS LIGHT VERTICAL AND LEFT
    { 0x252C, "SF060000" },  // BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
    { 0x2534, "SF070000" },  // BOX DRAWINGS LIGHT UP AND HORIZONTAL
    { 0x253C, "SF050000" },  // BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL
    { 0x2550, "SF430000" },  // BOX DRAWINGS DOUBLE HORIZONTAL
    { 0x2551, "SF240000" },  // BOX DRAWINGS DOUBLE VERTICAL
    { 0x2552, "SF510000" },  // BOX DRAWINGS DOWN SINGLE AND RIGHT DOUBLE
    { 0x2553, "SF520000" },  // BOX DRAWINGS DOWN DOUBLE AND RIGHT SINGLE
    { 0x2554, "SF390000" },  // BOX DRAWINGS DOUBLE DOWN AND RIGHT
    { 0x2555, "SF220000" },  // BOX DRAWINGS DOWN SINGLE AND LEFT DOUBLE
    { 0x2556, "SF210000" },  // BOX DRAWINGS DOWN DOUBLE AND LEFT SINGLE
    { 0x2557, "SF250000" },  // BOX DRAWINGS DOUBLE DOWN AND LEFT
    { 0x2558, "SF500000" },  // BOX DRAWINGS UP SINGLE AND RIGHT DOUBLE
    { 0x2559, "SF490000" },  // BOX DRAWINGS UP DOUBLE AND RIGHT SINGLE
    { 0x255A, "SF380000" },  // BOX DRAWINGS DOUBLE UP AND RIGHT
    { 0x255B, "SF280000" },  // BOX DRAWINGS UP SINGLE AND LEFT DOUBLE
    { 0x255C, "SF270000" },  // BOX DRAWINGS UP DOUBLE AND LEFT SINGLE
    { 0x255D, "SF260000" },  // BOX DRAWINGS DOUBLE UP AND LEFT
    { 0x255E, "SF360000" },  // BOX DRAWINGS VERTICAL SINGLE AND RIGHT DOUBLE
    { 0x255F, "SF370000" },  // BOX DRAWINGS VERTICAL DOUBLE AND RIGHT SINGLE
    { 0x2560, "SF420000" },  // BOX DRAWINGS DOUBLE VERTICAL AND RIGHT
    { 0x2561, "SF190000" },  // BOX DRAWINGS VERTICAL SINGLE AND LEFT DOUBLE
    { 0x2562, "SF200000" },  // BOX DRAWINGS VERTICAL DOUBLE AND LEFT SINGLE
    { 0x2563, "SF230000" },  // BOX DRAWINGS DOUBLE VERTICAL AND LEFT
    { 0x2564, "SF470000" },  // BOX DRAWINGS DOWN SINGLE AND HORIZONTAL DOUBLE
    { 0x2565, "SF480000" },  // BOX DRAWINGS DOWN DOUBLE AND HORIZONTAL SINGLE
    { 0x2566, "SF410000" },  // BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL
    { 0x2567, "SF450000" },  // BOX DRAWINGS UP SINGLE AND HORIZONTAL DOUBLE
    { 0x2568, "SF460000" },  // BOX DRAWINGS UP DOUBLE AND HORIZONTAL SINGLE
    { 0x2569, "SF400000" },  // BOX DRAWINGS DOUBLE UP AND HORIZONTAL
    { 0x256A, "SF540000" },  // BOX DRAWINGS VERTICAL SINGLE AND HORIZONTAL DOUBLE
    { 0x256B, "SF530000" },  // BOX DRAWINGS VERTICAL DOUBLE AND HORIZONTAL SINGLE
    { 0x256C, "SF440000" },  // BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL
    { 0x2580, "upblock" },  // UPPER HALF BLOCK
    { 0x2584, "dnblock" },  // LOWER HALF BLOCK
    { 0x2588, "block" },  // FULL BLOCK
    { 0x258C, "lfblock" },  // LEFT HALF BLOCK
    { 0x2590, "rtblock" },  // RIGHT HALF BLOCK
    { 0x2591, "ltshade" },  // LIGHT SHADE
    { 0x2592, "shade" },  // MEDIUM SHADE
    { 0x2593, "dkshade" },  // DARK SHADE
    { 0x25A0, "filledbox" },  // BLACK SQUARE
    { 0x25A1, "H22073" },  // WHITE SQUARE
    { 0x25AA, "H18543" },  // BLACK SMALL SQUARE
    { 0x25AB, "H18551" },  // WHITE SMALL SQUARE
    { 0x25AC, "filledrect" },  // BLACK RECTANGLE
    { 0x25B2, "triagup" },  // BLACK UP-POINTING TRIANGLE
    { 0x25BA, "triagrt" },  // BLACK RIGHT-POINTING POINTER
    { 0x25BC, "triagdn" },  // BLACK DOWN-POINTING TRIANGLE
    { 0x25C4, "triaglf" },  // BLACK LEFT-POINTING POINTER
    { 0x25CA, "lozenge" },  // LOZENGE
    { 0x25CB, "circle" },  // WHITE CIRCLE
    { 0x25CF, "H18533" },  // BLACK CIRCLE
    { 0x25D8, "invbullet" },  // INVERSE BULLET
    { 0x25D9, "invcircle" },  // INVERSE WHITE CIRCLE
    { 0x25E6, "openbullet" },  // WHITE BULLET
    { 0x263A, "smileface" },  // WHITE SMILING FACE
    { 0x263B, "invsmileface" },  // BLACK SMILING FACE
    { 0x263C, "sun" },  // WHITE SUN WITH RAYS
    { 0x2640, "female" },  // FEMALE SIGN
    { 0x2642, "male" },  // MALE SIGN
    { 0x2660, "spade" },  // BLACK SPADE SUIT
    { 0x2663, "club" },  // BLACK CLUB SUIT
    { 0x2665, "heart" },  // BLACK HEART SUIT
    { 0x2666, "diamond" },  // BLACK DIAMOND SUIT
    { 0x266A, "musicalnote" },  // EIGHTH NOTE
    { 0x266B, "musicalnotedbl" },  // BEAMED EIGHTH NOTES
    // The names below are in the PU area of Unicode, but needed to get a correct mapping of the symbol font
    { 0xF6D9, "copyrightserif" },
    { 0xF6DA, "registerserif" },
    { 0xF6DB, "trademarkserif" },
    { 0xF8E5, "radicalex" },
    { 0xF8E6, "arrowvertex" },
    { 0xF8E7, "arrowhorizex" },
    { 0xF8E8, "registersans" },
    { 0xF8E9, "copyrightsans" },
    { 0xF8EA, "trademarksans" },
    { 0xF8EB, "parenlefttp" },
    { 0xF8EC, "parenleftex" },
    { 0xF8ED, "parenleftbt" },
    { 0xF8EE, "bracketlefttp" },
    { 0xF8EF, "bracketleftex" },
    { 0xF8F0, "bracketleftbt" },
    { 0xF8F1, "bracelefttp" },
    { 0xF8F2, "braceleftmid" },
    { 0xF8F3, "braceleftbt" },
    { 0xF8F4, "braceex" },
    { 0xF8F5, "integralex" },
    { 0xF8F6, "parenrighttp" },
    { 0xF8F7, "parenrightex" },
    { 0xF8F8, "parenrightbt" },
    { 0xF8F9, "bracketrighttp" },
    { 0xF8FA, "bracketrightex" },
    { 0xF8FB, "bracketrightbt" },
    { 0xF8FC, "bracerighttp" },
    { 0xF8FD, "bracerightmid" },
    { 0xF8FE, "bracerightbt" },
    // End of extensions needed for symbols
    { 0xFB00, "ff" },  // LATIN SMALL LIGATURE FF
    { 0xFB01, "fi" },  // LATIN SMALL LIGATURE FI
    { 0xFB02, "fl" },  // LATIN SMALL LIGATURE FL
    { 0xFB03, "ffi" },  // LATIN SMALL LIGATURE FFI
    { 0xFB04, "ffl" },  // LATIN SMALL LIGATURE FFL
    { 0xFB1F, "afii57705" },  // HEBREW LIGATURE YIDDISH YOD YOD PATAH
    { 0xFB2A, "afii57694" },  // HEBREW LETTER SHIN WITH SHIN DOT
    { 0xFB2B, "afii57695" },  // HEBREW LETTER SHIN WITH SIN DOT
    { 0xFB35, "afii57723" },  // HEBREW LETTER VAV WITH DAGESH
    { 0xFB4B, "afii57700" },  // HEBREW LETTER VAV WITH HOLAM
    // end of stuff from glyphlist.txt
    { 0xFFFF, 0 }
};

// ---------------------------------------------------------------------
// postscript font substitution dictionary. We assume every postscript printer has at least
// Helvetica, Times, Courier and Symbol

struct psfont {
    const char *psname;
    float slant;
    float xscale;
};

static const psfont Arial[] = {
    {"Arial", 0, 84.04 },
    { "Arial-Italic", 0, 84.04 },
    { "Arial-Bold", 0, 88.65 },
    { "Arial-BoldItalic", 0, 88.65 }
};

static const psfont AvantGarde[] = {
    { "AvantGarde-Book", 0, 87.43 },
    { "AvantGarde-BookOblique", 0, 88.09 },
    { "AvantGarde-Demi", 0, 88.09 },
    { "AvantGarde-DemiOblique", 0, 87.43 },
};

static const psfont Bookman [] = {
    { "Bookman-Light", 0, 93.78 },
    { "Bookman-LightItalic", 0, 91.42 },
    { "Bookman-Demi", 0, 99.86 },
    { "Bookman-DemiItalic", 0, 101.54 }
};

static const psfont Charter [] = {
    { "CharterBT-Roman", 0, 84.04 },
    { "CharterBT-Italic", 0.0, 81.92 },
    { "CharterBT-Bold", 0, 88.99 },
    { "CharterBT-BoldItalic", 0.0, 88.20 }
};

static const psfont Courier [] = {
    { "Courier", 0, 100. },
    { "Courier-Oblique", 0, 100. },
    { "Courier-Bold", 0, 100. },
    { "Courier-BoldOblique", 0, 100. }
};

static const psfont Garamond [] = {
    { "Garamond-Antiqua", 0, 78.13 },
    { "Garamond-Kursiv", 0, 78.13 },
    { "Garamond-Halbfett", 0, 78.13 },
    { "Garamond-KursivHalbfett", 0, 78.13 }
};

static const psfont GillSans [] = { // ### some estimated value for xstretch
    { "GillSans", 0, 82 },
    { "GillSans-Italic", 0, 82 },
    { "GillSans-Bold", 0, 82 },
    { "GillSans-BoldItalic", 0, 82 }
};

static const psfont Helvetica [] = {
    { "Helvetica", 0, 84.04 },
    { "Helvetica-Oblique", 0, 84.04 },
    { "Helvetica-Bold", 0, 88.65 },
    { "Helvetica-BoldOblique", 0, 88.65 }
};

static const psfont Letter [] = {
    { "LetterGothic", 0, 83.32 },
    { "LetterGothic-Italic", 0, 83.32 },
    { "LetterGothic-Bold", 0, 83.32 },
    { "LetterGothic-Bold", 0.2, 83.32 }
};

static const psfont LucidaSans [] = {
    { "LucidaSans", 0, 94.36 },
    { "LucidaSans-Oblique", 0, 94.36 },
    { "LucidaSans-Demi", 0, 98.10 },
    { "LucidaSans-DemiOblique", 0, 98.08 }
};

static const psfont LucidaSansTT [] = {
    { "LucidaSans-Typewriter", 0, 100.50 },
    { "LucidaSans-TypewriterOblique", 0, 100.50 },
    { "LucidaSans-TypewriterBold", 0, 100.50 },
    { "LucidaSans-TypewriterBoldOblique", 0, 100.50 }
};

static const psfont LucidaBright [] = {
    { "LucidaBright", 0, 93.45 },
    { "LucidaBright-Italic", 0, 91.98 },
    { "LucidaBright-Demi", 0, 96.22 },
    { "LucidaBright-DemiItalic", 0, 96.98 }
};

static const psfont Palatino [] = {
    { "Palatino-Roman", 0, 82.45 },
    { "Palatino-Italic", 0, 76.56 },
    { "Palatino-Bold", 0, 83.49 },
    { "Palatino-BoldItalic", 0, 81.51 }
};

static const psfont Symbol [] = {
    { "Symbol", 0, 82.56 },
    { "Symbol", 0.2, 82.56 },
    { "Symbol", 0, 82.56 },
    { "Symbol", 0.2, 82.56 }
};

static const psfont Tahoma [] = {
    { "Tahoma", 0, 83.45 },
    { "Tahoma", 0.2, 83.45 },
    { "Tahoma-Bold", 0, 95.59 },
    { "Tahoma-Bold", 0.2, 95.59 }
};

static const psfont Times [] = {
    { "Times-Roman", 0, 82.45 },
    { "Times-Italic", 0, 82.45 },
    { "Times-Bold", 0, 82.45 },
    { "Times-BoldItalic", 0, 82.45 }
};

static const psfont Verdana [] = {
    { "Verdana", 0, 96.06 },
    { "Verdana-Italic", 0, 96.06 },
    { "Verdana-Bold", 0, 107.12 },
    { "Verdana-BoldItalic", 0, 107.10 }
};

static const psfont Utopia [] = { // ###
    { "Utopia-Regular", 0, 84.70 },
    { "Utopia-Regular", 0.2, 84.70 },
    { "Utopia-Bold", 0, 88.01 },
    { "Utopia-Bold", 0.2, 88.01 }
};

static const psfont * const SansSerifReplacements[] = {
    Helvetica, 0
        };
static const psfont * const SerifReplacements[] = {
    Times, 0
        };
static const psfont * const FixedReplacements[] = {
    Courier, 0
        };
static const psfont * const TahomaReplacements[] = {
    Verdana, AvantGarde, Helvetica, 0
        };
static const psfont * const VerdanaReplacements[] = {
    Tahoma, AvantGarde, Helvetica, 0
        };

static const struct {
    const char * input; // spaces are stripped in here, and everything lowercase
    const psfont * ps;
    const psfont *const * replacements;
} postscriptFonts [] = {
    { "arial", Arial, SansSerifReplacements },
    { "arialmt", Arial, SansSerifReplacements },
    { "arialunicodems", Arial, SansSerifReplacements },
    { "avantgarde", AvantGarde, SansSerifReplacements },
    { "bookman", Bookman, SerifReplacements },
    { "charter", Charter, SansSerifReplacements },
    { "bitstreamcharter", Charter, SansSerifReplacements },
        { "bitstreamcyberbit", Times, SerifReplacements }, // ###
    { "courier", Courier, 0 },
    { "couriernew", Courier, 0 },
    { "fixed", Courier, 0 },
    { "garamond", Garamond, SerifReplacements },
    { "gillsans", GillSans, SansSerifReplacements },
    { "helvetica", Helvetica, 0 },
    { "letter", Letter, FixedReplacements },
    { "lucida", LucidaSans, SansSerifReplacements },
    { "lucidasans", LucidaSans, SansSerifReplacements },
    { "lucidabright", LucidaBright, SerifReplacements },
    { "lucidasanstypewriter", LucidaSansTT, FixedReplacements },
    { "luciduxsans", LucidaSans, SansSerifReplacements },
    { "luciduxserif", LucidaBright, SerifReplacements },
    { "luciduxmono", LucidaSansTT, FixedReplacements },
    { "palatino", Palatino, SerifReplacements },
    { "symbol", Symbol, 0 },
    { "tahoma", Tahoma, TahomaReplacements },
    { "terminal", Courier, 0 },
    { "times", Times, 0 },
    { "timesnewroman", Times, 0 },
    { "verdana", Verdana, VerdanaReplacements },
    { "utopia", Utopia, SerifReplacements },
    { 0, 0, 0 }
};


// ------------------------------End of static data ----------------------------------

// make sure DSC comments are not longer than 255 chars per line.
static QString wrapDSC( const QString &str )
{
    QString dsc = str.simplifyWhiteSpace();
    const uint wrapAt = 254;
    QString wrapped;
    if ( dsc.length() < wrapAt )
	wrapped = dsc;
    else {
	wrapped = dsc.left( wrapAt );
	QString tmp = dsc.mid( wrapAt );
	while ( tmp.length() > wrapAt-3 ) {
	    wrapped += "\n%%+" + tmp.left( wrapAt-3 );
	    tmp = tmp.mid( wrapAt-3 );
	}
	wrapped += "\n%%+" + tmp;
    }
    return wrapped + "\n";
}

static QString toString( const float num )
{
    return QString::number( num, 'f', 3 );
}

// ----------------------------- Internal class declarations -----------------------------

class QPSPrinterFontPrivate;

class QPSPrinterPrivate {
public:
    QPSPrinterPrivate( QPrinter *prt, int filedes );
    ~QPSPrinterPrivate();

    void matrixSetup( QPainter * );
    void clippingSetup( QPainter * );
    void setClippingOff( QPainter * );
    void orientationSetup();
    void resetDrawingTools( QPainter * );
    void emitHeader( bool finished );
    void setFont( const QFont &, int script );
    void drawImage( QPainter *, float x, float y, float w, float h, const QImage &img, const QImage &mask );
    void initPage( QPainter *paint );
    void flushPage( bool last = FALSE );

    QPrinter   *printer;
    int         pageCount;
    bool        dirtyMatrix;
    bool        dirtyNewPage;
    bool        epsf;
    QString     fontsUsed;

    // outstream is the stream the build up pages are copied to. It points to buffer
    // at the start, and is reset to use the outDevice after emitHeader has been called.
    QTextStream outStream;

    // stores the descriptions of the first pages. outStream operates on this buffer
    // until we call emitHeader
    QBuffer *buffer;
    int pagesInBuffer;

    // the device the output is in the end streamed to.
    QIODevice * outDevice;
    int fd;

    // buffer for the current page. Needed becaus we might have page fonts.
    QBuffer *pageBuffer;
    QTextStream pageStream;

    QDict<QString> headerFontNames;
    QDict<QString> pageFontNames;
    QDict<QPSPrinterFontPrivate> fonts;
    QPSPrinterFontPrivate *currentFontFile;
    int headerFontNumber;
    int pageFontNumber;
    QBuffer * fontBuffer;
    QTextStream fontStream;
    bool dirtyClipping;
    bool firstClipOnPage;
    QRect boundingBox;
    QImage * savedImage;
    QPen cpen;
    QBrush cbrush;
    bool dirtypen;
    bool dirtybrush;
    QColor bkColor;
    bool dirtyBkColor;
    Qt::BGMode bkMode;
    bool dirtyBkMode;
#ifndef QT_NO_TEXTCODEC
    QTextCodec * currentFontCodec;
#endif
    QString currentFont;
    QFontMetrics fm;
    int textY;
    QFont currentUsed;
    int scriptUsed;
    QFont currentSet;
    float scale;

    QStringList fontpath;
};


class QPSPrinterFontPrivate {
public:
    QPSPrinterFontPrivate();
    virtual ~QPSPrinterFontPrivate() {}
    virtual QString postScriptFontName() { return psname; }
    virtual QString defineFont( QTextStream &stream, const QString &ps, const QFont &f, const QString &key,
                             QPSPrinterPrivate *d );
    virtual void download(QTextStream& s, bool global);
    virtual void drawText( QTextStream &stream, const QPoint &p, QTextEngine *engine, int item,
                           const QString &text, QPSPrinterPrivate *d, QPainter *paint);
    virtual unsigned short mapUnicode( unsigned short unicode );
    void downloadMapping( QTextStream &s, bool global );
    QString glyphName( unsigned short glyphindex, bool *glyphSet = 0 );
    virtual void restore();

    virtual unsigned short unicode_for_glyph(int glyphindex) { return glyphindex; }
    virtual unsigned short glyph_for_unicode(unsigned short unicode) { return unicode; }
    unsigned short insertIntoSubset( unsigned short unicode );
    virtual bool embedded() { return FALSE; }

    bool operator == ( const QPSPrinterFontPrivate &other ) {
	return other.psname == psname;
    }
    inline void setSymbol() { symbol = TRUE; }

protected:
    QString psname;
    QStringList replacementList;

    QMap<unsigned short, unsigned short> subset;      // unicode subset in the global font
    QMap<unsigned short, unsigned short> page_subset; // subset added in this page
    int subsetCount;
    int pageSubsetCount;
    bool global_dict;
    bool downloaded;
    bool symbol;
};

// ------------------- end of class declarations ---------------------------

// --------------------------------------------------------------
//   beginning of font related methods
// --------------------------------------------------------------


static int getPsFontType( const QFontEngine *fe )
{
    int weight = fe->fontDef.weight;
    bool italic = fe->fontDef.italic;

    int type = 0; // used to look up in the psname array
    // get the right modification, or build something
    if ( weight > QFont::Normal && italic )
	type = 3;
    else if ( weight > QFont::Normal )
	type = 2;
    else if ( italic )
	type = 1;
    return type;
}

static int addPsFontNameExtension( const QFontEngine *fe, QString &ps, const psfont *psf = 0 )
{
    int type = getPsFontType( fe );

    if ( psf ) {
	ps = QString::fromLatin1( psf[type].psname );
    } else {
	switch ( type ) {
	case 1:
	    ps.append( QString::fromLatin1("-Italic") );
	    break;
	case 2:
	    ps.append( QString::fromLatin1("-Bold") );
	    break;
	case 3:
	    ps.append( QString::fromLatin1("-BoldItalic") );
	    break;
	case 0:
	default:
	    break;
	}
    }
    return type;
}

static QString makePSFontName( const QFontEngine *fe, int *listpos = 0, int *ftype = 0 )
{
  QString ps;
  int i;

  QString family = fe->fontDef.family.lower();

  // try to make a "good" postscript name
  ps = family.simplifyWhiteSpace();
  i = 0;
  while( (unsigned int)i < ps.length() ) {
    if ( i != 0 && ps[i] == '[') {
      if ( ps[i-1] == ' ' )
	ps.truncate (i-1);
      else
	ps.truncate (i);
      break;
    }
    if ( i == 0 || ps[i-1] == ' ' ) {
      ps[i] = ps[i].upper();
      if ( i )
        ps.remove( i-1, 1 );
      else
        i++;
    } else {
      i++;
    }
  }

  if ( ps.isEmpty() )
      ps = "Helvetica";

  // see if the table has a better name
  i = 0;
  QString lowerName = ps.lower();
  while( postscriptFonts[i].input &&
         postscriptFonts[i].input != lowerName )
    i++;
  const psfont *psf = postscriptFonts[i].ps;

  int type = addPsFontNameExtension( fe, ps, psf );

  if ( listpos )
      *listpos = i;
  if ( ftype )
      *ftype = type;
  return ps;
}

static void appendReplacements( QStringList &list, const psfont * const * replacements, int type, float xscale = 100. )
{
    // iterate through the replacement fonts
    while ( *replacements ) {
        const psfont *psf = *replacements;
        QString ps = "[ /" + QString::fromLatin1( psf[type].psname ) + " " +
		     toString( xscale / psf[type].xscale ) + " " +
		     toString( psf[type].slant ) + " ]";
        list.append( ps );
        ++replacements;
    }
}

static QStringList makePSFontNameList( const QFontEngine *fe, const QString &psname = QString::null, bool useNameForLookup = FALSE )
{
    int i;
    int type;
    QStringList list;
    QString ps = psname;

    if ( !ps.isEmpty() && !useNameForLookup ) {
        QString best = "[ /" + ps + " 1.0 0.0 ]";
        list.append( best );
    }

    ps = makePSFontName( fe, &i, &type );

    const psfont *psf = postscriptFonts[i].ps;
    const psfont * const * replacements = postscriptFonts[i].replacements;
    float xscale = 100;
    if ( psf ) {
        // xscale for the "right" font is always 1. We scale the replacements...
        xscale = psf->xscale;
        ps = "[ /" + QString::fromLatin1( psf[type].psname ) + " 1.0 " +
             toString( psf[type].slant ) + " ]";
    } else {
        ps = "[ /" + ps + " 1.0 0.0 ]";
        // only add default replacement fonts in case this font was unknown.
        if ( fe->fontDef.fixedPitch ) {
            replacements = FixedReplacements;
        } else {
            replacements = SansSerifReplacements;
            // 100 is courier, but most fonts are not as wide as courier. Using 100
            // here would make letters overlap for some fonts. This value is empirical.
            xscale = 83;
        }
    }
    list.append( ps );

    if ( replacements )
        appendReplacements( list, replacements, type, xscale);
    return list;
}

static void emitPSFontNameList( QTextStream &s, const QString &psname, const QStringList &list )
{
    s << "/" << psname << "List [\n";
    s << list.join("\n  ");
    s << "\n] d\n";
}

static inline float pointSize( const QFont &f, float scale )
{
    float psize;
    if ( f.pointSize() != -1 )
	psize = f.pointSize()/scale;
    else
	psize = f.pixelSize();
    return psize;
}


// ========================== FONT CLASSES  ===============


QPSPrinterFontPrivate::QPSPrinterFontPrivate()
{
    global_dict = FALSE;
    downloaded  = FALSE;
    symbol = FALSE;
    // map 0 to .notdef
    subset.insert( 0, 0 );
    subsetCount = 1;
    pageSubsetCount = 0;
}

unsigned short QPSPrinterFontPrivate::insertIntoSubset( unsigned short u )
{
    unsigned short retval = 0;
    if ( subset.find(u) == subset.end() ) {
        if ( !downloaded ) { // we need to add to the page subset
            subset.insert( u, subsetCount ); // mark it as used
            //printf("GLOBAL SUBSET ADDED %04x = %04x\n",u, subsetCount);
            retval = subsetCount;
            subsetCount++;
        } else if ( page_subset.find(u) == page_subset.end() ) {
            page_subset.insert( u, pageSubsetCount ); // mark it as used
            //printf("PAGE SUBSET ADDED %04x = %04x\n",u, pageSubsetCount);
            retval = pageSubsetCount + (subsetCount/256 + 1) * 256;
            pageSubsetCount++;
        }
    } else {
        qWarning("QPSPrinterFont::internal error");
    }
    return retval;
}

void QPSPrinterFontPrivate::restore()
{
    page_subset.clear();
    pageSubsetCount = 0;
    //qDebug("restore for font %s\n",psname.latin1());
}

static inline const char *toHex( uchar u )
{
    static char hexVal[3];
    int i = 1;
    while ( i >= 0 ) {
	ushort hex = (u & 0x000f);
	if ( hex < 0x0a )
	    hexVal[i] = '0'+hex;
	else
	    hexVal[i] = 'A'+(hex-0x0a);
	u = u >> 4;
	i--;
    }
    hexVal[2] = '\0';
    return hexVal;
}

static inline const char *toHex( ushort u )
{
    static char hexVal[5];
    int i = 3;
    while ( i >= 0 ) {
	ushort hex = (u & 0x000f);
	if ( hex < 0x0a )
	    hexVal[i] = '0'+hex;
	else
	    hexVal[i] = 'A'+(hex-0x0a);
	u = u >> 4;
	i--;
    }
    hexVal[4] = '\0';
    return hexVal;
}

static inline const char * toInt( int i )
{
    static char intVal[20];
    intVal[19] = 0;
    int pos = 19;
    if ( i == 0 ) {
	intVal[--pos] = '0';
    } else {
	bool neg = FALSE;
	if ( i < 0 ) {
	    neg = TRUE;
	    i = -i;
	}
	while ( i ) {
	    int dec = i%10;
	    intVal[--pos] = '0'+dec;
	    i /= 10;
	}
	if ( neg )
	    intVal[--pos] = '-';
    }
    return intVal+pos;
}

void QPSPrinterFontPrivate::drawText( QTextStream &stream, const QPoint &p, QTextEngine *engine, int item,
                                 const QString &text, QPSPrinterPrivate *d, QPainter *paint)
{
    int len = engine->length( item );
    QScriptItem &si = engine->items[item];

    int x = p.x() + si.x;
    int y = p.y() + si.y;
    if ( y != d->textY || d->textY == 0 )
        stream << y << " Y";
    d->textY = y;

    stream << "<";
    if ( si.analysis.bidiLevel % 2 ) {
	for ( int i = len-1; i >=0; i-- )
	    stream << toHex( mapUnicode(text.unicode()[i].unicode()) );
    } else {
	for ( int i = 0; i < len; i++ )
	    stream << toHex( mapUnicode(text.unicode()[i].unicode()) );
    }
    stream << ">";

    stream << si.width << " " << x;

    if ( paint->font().underline() )
        stream << ' ' << y + d->fm.underlinePos() + d->fm.lineWidth()
               << " " << d->fm.lineWidth() << " Tl";
    if ( paint->font().strikeOut() )
        stream << ' ' << y + d->fm.strikeOutPos()
               << " " << d->fm.lineWidth() << " Tl";
    stream << " AT\n";

}


QString QPSPrinterFontPrivate::defineFont( QTextStream &stream, const QString &ps, const QFont &f, const QString &key,
                                   QPSPrinterPrivate *d )
{
    QString fontName;
    fontName.sprintf( "/%s-Uni", ps.latin1());

    if ( d->buffer ) {
        ++d->headerFontNumber;
        d->fontStream << "/F" << d->headerFontNumber << " "
                      << pointSize( f, d->scale ) << fontName << " DF\n";
        fontName.sprintf( "F%d", d->headerFontNumber );
        d->headerFontNames.insert( key, new QString( fontName ) );
    } else {
        ++d->pageFontNumber;
        stream << "/F" << d->pageFontNumber << " "
               << pointSize( f, d->scale ) << fontName << " DF\n";
        fontName.sprintf( "F%d", d->pageFontNumber );
        d->pageFontNames.insert( key, new QString( fontName ) );
    }
    return fontName;
}

unsigned short QPSPrinterFontPrivate::mapUnicode( unsigned short unicode )
{
    QMap<unsigned short, unsigned short>::iterator res;
    res = subset.find( unicode );
    unsigned short offset = 0;
    bool found = FALSE;
    if ( res != subset.end() ) {
        found = TRUE;
    } else {
        if ( downloaded ) {
            res = page_subset.find( unicode );
            offset = (subsetCount/256 + 1) * 256;
            if ( res != page_subset.end() )
                found = TRUE;
        }
    }
    if ( !found ) {
        return insertIntoSubset( unicode );
    }
    //qDebug("mapping unicode %x to %x", unicode, offset+*res);
    return offset + *res;
}

// This map is used for symbol fonts to get the correct glyph names for the latin range
static const unsigned short symbol_map[0x100] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
    0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
    0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
    0x0020, 0x0021, 0x2200, 0x0023, 0x2203, 0x0025, 0x0026, 0x220b,
    0x0028, 0x0029, 0x2217, 0x002b, 0x002c, 0x2212, 0x002e, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,

    0x2245, 0x0391, 0x0392, 0x03a7, 0x0394, 0x0395, 0x03a6, 0x0393,
    0x0397, 0x0399, 0x03d1, 0x039a, 0x039b, 0x039c, 0x039d, 0x039f,
    0x03a0, 0x0398, 0x03a1, 0x03a3, 0x03a4, 0x03a5, 0x03c2, 0x03a9,
    0x039e, 0x03a8, 0x0396, 0x005b, 0x2234, 0x005d, 0x22a5, 0x005f,
    0xf8e5, 0x03b1, 0x03b2, 0x03c7, 0x03b4, 0x03b5, 0x03c6, 0x03b3,
    0x03b7, 0x03b9, 0x03d5, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03bf,
    0x03c0, 0x03b8, 0x03c1, 0x03c3, 0x03c4, 0x03c5, 0x03d6, 0x03c9,
    0x03be, 0x03c8, 0x03b6, 0x007b, 0x007c, 0x007d, 0x223c, 0x007f,

    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
    0x20ac, 0x03d2, 0x2023, 0x2264, 0x2044, 0x221e, 0x0192, 0x2263,
    0x2666, 0x2665, 0x2660, 0x2194, 0x2190, 0x2191, 0x2192, 0x2193,
    0x00b0, 0x00b1, 0x2033, 0x2265, 0x00d7, 0x221d, 0x2202, 0x2022,
    0x00f7, 0x2260, 0x2261, 0x2248, 0x2026, 0xf8e6, 0xf8e7, 0x21b5,

    0x2135, 0x2111, 0x211c, 0x2118, 0x2297, 0x2295, 0x2205, 0x2229,
    0x222a, 0x2283, 0x2287, 0x2284, 0x2282, 0x2286, 0x2208, 0x2209,
    0x2220, 0x2207, 0xf6da, 0xf6d9, 0xf6db, 0x220f, 0x221a, 0x22c5,
    0x00ac, 0x2227, 0x2228, 0x21d4, 0x21d0, 0x21d1, 0x21d2, 0x21d3,
    0x25ca, 0x2329, 0xf8e8, 0xf8e9, 0xf8ea, 0x2211, 0xf8eb, 0xf8ec,
    0xf8ed, 0xf8ee, 0xf8ef, 0xf8f0, 0xf8f1, 0xf8f2, 0xf8f3, 0xf8f4,
    0x0000, 0x232a, 0x222b, 0x2320, 0xf8f5, 0x2321, 0xf8f6, 0xf8f7,
    0xf8f8, 0xf8f9, 0xf8fa, 0xf8fb, 0xf8fc, 0xf8fd, 0xf8fe, 0x0000,
};

QString QPSPrinterFontPrivate::glyphName( unsigned short glyphindex, bool *glyphSet )
{
    QString glyphname;
    int l = 0;
    unsigned short unicode = unicode_for_glyph( glyphindex );
    if (symbol && unicode < 0x100) {
	// map from latin1 to symbol
	unicode = symbol_map[unicode];
    }
    if ( !unicode && glyphindex ) {
	glyphname = "gl";
	glyphname += toHex( glyphindex );
    } else {
	while( unicodetoglyph[l].u < unicode )
	    l++;
	if ( unicodetoglyph[l].u == unicode ) {
	    glyphname = unicodetoglyph[l].g;
	    if ( glyphSet ) {
		int other = 0;
		switch ( unicode ) {
		    // some glyph names are duplicate in postscript. Make sure we give the
		    // duplicate a different name to avoid infinite recursion
		case 0x0394:
		    other = 0x2206;
		    break;
		case 0x03a9:
		    other = 0x2126;
		    break;
		case 0x0162:
		    other = 0x021a;
		    break;
		case 0x2215:
		    other = 0x2044;
		    break;
		case 0x00ad:
		    other = 0x002d;
		    break;
		case 0x02c9:
		    other = 0x00af;
		    break;
		case 0x03bc:
		    other = 0x00b5;
		    break;
		case 0x2219:
		    other = 0x00b7;
		    break;
		case 0x00a0:
		    other = 0x0020;
		    break;
		case 0x0163:
		    other = 0x021b;
		    break;
		default:
		    break;
		}
		if ( other ) {
		    int oglyph = glyph_for_unicode( other );
		    if( oglyph && oglyph != glyphindex && glyphSet[oglyph] ) {
 			glyphname = "uni";
 			glyphname += toHex( unicode );
		    }
		}
	    }
 	} else {
	    glyphname = "uni";
	    glyphname += toHex( unicode );
	}
    }
    return glyphname;
}

void QPSPrinterFontPrivate::download(QTextStream &s, bool global)
{
    //printf("defining mapping for printer font %s\n",psname.latin1());
    downloadMapping( s, global );
}

void QPSPrinterFontPrivate::downloadMapping( QTextStream &s, bool global )
{
    uchar rangeOffset = 0;
    uchar numRanges = (uchar)(subsetCount/256 + 1);
    uchar range;
    QMap<unsigned short, unsigned short> *subsetDict = &subset;
    if ( !global ) {
        rangeOffset = numRanges;
        numRanges = pageSubsetCount/256 + 1;
        subsetDict = &page_subset;
    }
    // build up inverse table
    unsigned short *inverse = new unsigned short[numRanges * 256];
    memset( inverse, 0, numRanges * 256 * sizeof( unsigned short ) );

    QMap<unsigned short, unsigned short>::iterator it;
    for ( it = subsetDict->begin(); it != subsetDict->end(); ++it) {
        const unsigned short &mapped = *it;
        inverse[mapped] = it.key();
    }

    QString vector;
    QString glyphname;

    for (range=0; range < numRanges; range++) {
        //printf("outputing range %04x\n",range*256);
        vector = "%% Font Page ";
	vector += toHex((uchar)(range + rangeOffset));
        vector += "\n/";
        vector += psname;
        vector += "-ENC-";
	vector += toHex((uchar)(range + rangeOffset));
	vector += " [\n";

        QString line;
        for(int k=0; k<256; k++ ) {
            int c = range*256 + k;
            unsigned short glyph = inverse[c];
            glyphname = glyphName( glyph );
            if ( line.length() + glyphname.length() > 76 ) {
                vector += line;
		vector += "\n";
                line = "";
            }
            line += "/" + glyphname;
        }
        vector += line;
	vector += "] def\n";
        s << vector;
    }

    delete [] inverse;

    // DEFINE BASE FONTS

    for (range=0; range < numRanges; range++) {
        s << "/";
        s << psname;
        s << "-Uni-";
        s << toHex((uchar)(range + rangeOffset));
        s << " ";
        s << psname;
        s << "-ENC-";
        s << toHex((uchar)(range + rangeOffset));
        if ( embedded() && embedFonts ) {
            s << " /";
            s << psname;
            s << " MFEmb\n";
        } else {
            s << " " << psname << "List";
            s << " MF\n";
        }
    }

    // === write header ===
    //   int VMMin;
    //   int VMMax;

    s << wrapDSC( "%%BeginFont: " + psname )
      << "%!PS-AdobeFont-1.0 Composite Font\n"
      << wrapDSC( "%%FontName: " + psname + "-Uni" )
      << "%%Creator: Composite font created by Qt\n";

    /* Start the dictionary which will eventually */
    /* become the font. */
    s << "25 dict begin\n"; // need to verify. Sivan

    s << "/FontName /";
    s << psname;
    s << "-Uni";
    s << " def\n";
    s << "/PaintType 0 def\n";

    // This is concatenated with the base fonts, so it should perform
    // no transformation. Sivan
    s << "/FontMatrix[1 0 0 1 0 0]def\n";

    s << "/FontType ";
    s << 0;
    s << " def\n";

    // now come composite font structures
    // FMapTypes:
    // 2: 8/8, 8 bits select the font, 8 the glyph

    s << "/FMapType 2 def\n";

    // The encoding in a composite font is used for indirection.
    // Every char is split into a font-number and a character-selector.
    // PostScript prints glyph number character-selector from the font
    // FDepVector[ Encoding[ font-number ] ].

    s << "/Encoding [";
    for (range=0; range < rangeOffset + numRanges; range++) {
        if (range % 16 == 0)
            s << "\n";
        else
            s << " ";
        s << range;
    }
    s << "]def\n";

  // Descendent fonts

    s << "/FDepVector [\n";
    for (range=0; range < rangeOffset + numRanges; range++) {
        s << "/";
        s << psname;
        s << "-Uni-";
        s << toHex( range );
        s << " findfont\n";
    }
    s << "]def\n";

  // === trailer ===

    s << "FontName currentdict end definefont pop\n";
    s << "%%EndFont\n";
}


// ================== TTF ====================

typedef Q_UINT8  BYTE;
typedef Q_UINT16 USHORT;
typedef Q_UINT16 uFWord;
typedef Q_INT16 SHORT;
typedef Q_INT16 FWord;
typedef Q_UINT32   ULONG;
typedef Q_INT32  FIXED;

typedef struct {
  Q_INT16 whole;
  Q_UINT16 fraction;
} Fixed; // 16.16 bit fixed-point number

static float f2dot14( ushort s )
{
    float f = ((float)( s & 0x3fff ))/ 16384.;
    f += (s & 0x8000) ? ( (s & 0x4000) ? -1 : -2 ) : ( (s & 0x4000) ? 1 : 0 );
    return f;
}

typedef struct {
  int*    epts_ctr;                     /* array of contour endpoints */
  int     num_pts, num_ctr;             /* number of points, number of coutours */
  FWord*  xcoor, *ycoor;                /* arrays of x and y coordinates */
  BYTE*   tt_flags;                     /* array of TrueType flags */
  double* area_ctr;
  char*   check_ctr;
  int*    ctrset;               /* in contour index followed by out contour index */
} charproc_data;


class QPSPrinterFontTTF
    : public QPSPrinterFontPrivate {
public:
    QPSPrinterFontTTF(const QFontEngine *f, QByteArray& data);
    virtual void    download(QTextStream& s, bool global);
    virtual void drawText( QTextStream &stream, const QPoint &p, QTextEngine *engine, int item,
                           const QString &text, QPSPrinterPrivate *d, QPainter *paint);
    //  virtual ~QPSPrinterFontTTF();

    virtual bool embedded() { return TRUE; }
private:
    QByteArray     data;
    QMemArray<ushort> uni2glyph; // to speed up lookups
    QMemArray<ushort> glyph2uni; // to speed up lookups
    bool           defective; // if we can't process this file

    BYTE*   getTable(const char *);
    void uni2glyphSetup();
    unsigned short unicode_for_glyph(int glyphindex);
    unsigned short glyph_for_unicode(unsigned short unicode);
    int   topost(FWord x) { return (int)( ((int)(x) * 1000 + HUPM) / unitsPerEm ); }

#ifdef Q_PRINTER_USE_TYPE42
    void sfnts_pputBYTE(BYTE n,QTextStream& s,
			int& string_len, int& line_len, bool& in_string);
    void sfnts_pputUSHORT(USHORT n,QTextStream& s,
			  int& string_len, int& line_len, bool& in_string);
    void sfnts_pputULONG(ULONG n,QTextStream& s,
			 int& string_len, int& line_len, bool& in_string);
    void sfnts_end_string(QTextStream& s,
			  int& string_len, int& line_len, bool& in_string);
    void sfnts_new_table(ULONG length,QTextStream& s,
			 int& string_len, int& line_len, bool& in_string);
    void sfnts_glyf_table(ULONG oldoffset,
			  ULONG correct_total_length,
			  QTextStream& s,
			  int& string_len, int& line_len, bool& in_string);
    void download_sfnts(QTextStream& s);
#endif

    void subsetGlyph(int charindex,bool* glyphset);

    void charproc(int charindex, QTextStream& s, bool *glyphSet);
    BYTE* charprocFindGlyphData(int charindex);
    void charprocComposite(BYTE *glyph, QTextStream& s, bool *glyphSet);
    void charprocLoad(BYTE *glyph, charproc_data* cd);

    int target_type;                      /* 42 or 3 */

    int numTables;                        /* number of tables present */
    QString PostName;                     /* Font's PostScript name */
    QString FullName;                     /* Font's full name */
    QString FamilyName;                   /* Font's family name */
    QString Style;                        /* Font's style string */
    QString Copyright;                    /* Font's copyright string */
    QString Version;                      /* Font's version string */
    QString Trademark;                    /* Font's trademark string */
    int llx,lly,urx,ury;          /* bounding box */

    Fixed TTVersion;                      /* Truetype version number from offset table */
    Fixed MfrRevision;                    /* Revision number of this font */

    BYTE *offset_table;           /* Offset table in memory */
    BYTE *post_table;                     /* 'post' table in memory */

    BYTE *loca_table;                     /* 'loca' table in memory */
    BYTE *glyf_table;                     /* 'glyf' table in memory */
    BYTE *hmtx_table;                     /* 'hmtx' table in memory */

    USHORT numberOfHMetrics;
    int unitsPerEm;                       /* unitsPerEm converted to int */
    int HUPM;                             /* half of above */

    int numGlyphs;                        /* from 'post' table */

    int indexToLocFormat;         /* short or long offsets */

};


static ULONG getULONG(BYTE *p)
{
  int x;
  ULONG val=0;

  for(x=0; x<4; x++) {
    val *= 0x100;
    val += p[x];
  }

  return val;
}

static USHORT getUSHORT(BYTE *p)
{
  int x;
  USHORT val=0;

  for(x=0; x<2; x++) {
    val *= 0x100;
    val += p[x];
  }

  return val;
}

static Fixed getFixed(BYTE *s)
{
  Fixed val={0,0};

  val.whole = ((s[0] * 256) + s[1]);
  val.fraction = ((s[2] * 256) + s[3]);

  return val;
}

static FWord getFWord(BYTE* s)  { return (FWord)  getUSHORT(s); }
static uFWord getuFWord(BYTE* s) { return (uFWord) getUSHORT(s); }
static SHORT getSHORT(BYTE* s)  { return (SHORT)  getUSHORT(s); }

#if 0
static const char * const Apple_CharStrings[]={
  ".notdef",".null","nonmarkingreturn","space","exclam","quotedbl","numbersign",
  "dollar","percent","ampersand","quotesingle","parenleft","parenright",
  "asterisk","plus", "comma","hyphen","period","slash","zero","one","two",
  "three","four","five","six","seven","eight","nine","colon","semicolon",
  "less","equal","greater","question","at","A","B","C","D","E","F","G","H","I",
  "J","K", "L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z",
  "bracketleft","backslash","bracketright","asciicircum","underscore","grave",
  "a","b","c","d","e","f","g","h","i","j","k", "l","m","n","o","p","q","r","s",
  "t","u","v","w","x","y","z","braceleft","bar","braceright","asciitilde",
  "Adieresis","Aring","Ccedilla","Eacute","Ntilde","Odieresis","Udieresis",
  "aacute","agrave","acircumflex","adieresis","atilde","aring","ccedilla",
  "eacute","egrave","ecircumflex","edieresis","iacute","igrave","icircumflex",
  "idieresis","ntilde","oacute","ograve","ocircumflex","odieresis","otilde",
  "uacute","ugrave","ucircumflex","udieresis","dagger","degree","cent",
  "sterling","section","bullet","paragraph","germandbls","registered",
  "copyright","trademark","acute","dieresis","notequal","AE","Oslash",
  "infinity","plusminus","lessequal","greaterequal","yen","mu","partialdiff",
  "summation","product","pi","integral","ordfeminine","ordmasculine","Omega",
  "ae","oslash","questiondown","exclamdown","logicalnot","radical","florin",
  "approxequal","Delta","guillemotleft","guillemotright","ellipsis",
  "nobreakspace","Agrave","Atilde","Otilde","OE","oe","endash","emdash",
  "quotedblleft","quotedblright","quoteleft","quoteright","divide","lozenge",
  "ydieresis","Ydieresis","fraction","currency","guilsinglleft","guilsinglright",
  "fi","fl","daggerdbl","periodcentered","quotesinglbase","quotedblbase",
  "perthousand","Acircumflex","Ecircumflex","Aacute","Edieresis","Egrave",
  "Iacute","Icircumflex","Idieresis","Igrave","Oacute","Ocircumflex","apple",
  "Ograve","Uacute","Ucircumflex","Ugrave","dotlessi","circumflex","tilde",
  "macron","breve","dotaccent","ring","cedilla","hungarumlaut","ogonek","caron",
  "Lslash","lslash","Scaron","scaron","Zcaron","zcaron","brokenbar","Eth","eth",
  "Yacute","yacute","Thorn","thorn","minus","multiply","onesuperior",
  "twosuperior","threesuperior","onehalf","onequarter","threequarters","franc",
  "Gbreve","gbreve","Idot","Scedilla","scedilla","Cacute","cacute","Ccaron",
  "ccaron","dmacron","markingspace","capslock","shift","propeller","enter",
  "markingtabrtol","markingtabltor","control","markingdeleteltor",
  "markingdeletertol","option","escape","parbreakltor","parbreakrtol",
  "newpage","checkmark","linebreakltor","linebreakrtol","markingnobreakspace",
  "diamond","appleoutline"};
#endif

// #define DEBUG_TRUETYPE

QPSPrinterFontTTF::QPSPrinterFontTTF(const QFontEngine *f, QByteArray& d)
{
  data = d;
  defective = FALSE;

  BYTE *ptr;

  target_type = 3;  // will work on any printer
  //target_type = 42; // works with gs, faster, better quality

#ifdef Q_PRINTER_USE_TYPE42
  char* environment_preference = getenv("QT_TTFTOPS");
  if (environment_preference) {
    if (QString(environment_preference) == "42")
      target_type = 42;
    else if (QString(environment_preference) == "3")
      target_type = 3;
    else
      qWarning("The value of QT_TTFTOPS must be 42 or 3");
  }
#endif
  offset_table = (unsigned char*) data.data(); /* first 12 bytes */

  /* Determine how many directory entries there are. */
  numTables = getUSHORT( offset_table + 4 );

  /* Extract information from the "Offset" table. */
  TTVersion = getFixed( offset_table );

  /* Load the "head" table and extract information from it. */
  ptr = getTable("head");
  if ( !ptr ) {
      defective = TRUE;
      return;
  }
  MfrRevision = getFixed( ptr + 4 );            /* font revision number */
  unitsPerEm = getUSHORT( ptr + 18 );
  HUPM = unitsPerEm / 2;
#ifdef DEBUG_TRUETYPE
  printf("unitsPerEm=%d",(int)unitsPerEm);
#endif
  llx = topost( getFWord( ptr + 36 ) );         /* bounding box info */
  lly = topost( getFWord( ptr + 38 ) );
  urx = topost( getFWord( ptr + 40 ) );
  ury = topost( getFWord( ptr + 42 ) );
  indexToLocFormat = getSHORT( ptr + 50 );      /* size of 'loca' data */
  if(indexToLocFormat != 0 && indexToLocFormat != 1) {
    qWarning("TrueType font is unusable because indexToLocFormat != 0");
    defective = TRUE;
    return;
  }
  if( getSHORT(ptr+52) != 0 ) {
    qWarning("TrueType font is unusable because glyphDataFormat != 0");
    defective = TRUE;
    return;
  }

  // === Load information from the "name" table ===

  /* Set default values to avoid future references to */
  /* undefined pointers. */

  psname = FullName = FamilyName = Version = Style = "unknown";
  Copyright = "No copyright notice";
  Trademark = "No trademark notice";

  BYTE* table_ptr = getTable("name");           /* pointer to table */
  if ( !table_ptr ) {
      defective = TRUE;
      qDebug("couldn't find name table" );
      return;
  }
  int numrecords = getUSHORT( table_ptr + 2 );  /* number of names */
  char* strings = (char *)table_ptr + getUSHORT( table_ptr + 4 ); /* start of string storage */

  BYTE* ptr2 = table_ptr + 6;
  for(int x=0; x < numrecords; x++,ptr2+=12) {
      int platform = getUSHORT(ptr2);
    //int encoding = getUSHORT(ptr2+2);
    //int language = getUSHORT(ptr2+4);
    int nameid   = getUSHORT(ptr2+6);
    int length   = getUSHORT(ptr2+8);
    int offset   = getUSHORT(ptr2+10);

    if( platform == 1 && nameid == 0 )
      Copyright.setLatin1(strings+offset,length);

    if( platform == 1 && nameid == 1 )
      FamilyName.setLatin1(strings+offset,length);

    if( platform == 1 && nameid == 2 )
      Style.setLatin1(strings+offset,length);

    if( platform == 1 && nameid == 4 )
      FullName.setLatin1(strings+offset,length);

    if( platform == 1 && nameid == 5 )
      Version.setLatin1(strings+offset,length);

    if( platform == 1 && nameid == 6 )
      psname.setLatin1(strings+offset,length);

    if( platform == 1 && nameid == 7 )
      Trademark.setLatin1(strings+offset,length);

  }
  psname.replace(' ', '-');
  psname.replace("/", "");
  if (psname.isEmpty())
      psname = makePSFontName(f);

  //read_cmap(font);

  /* We need to have the PostScript table around. */

  post_table = getTable("post");
#if 0
  if ( post_table ) {
      Fixed post_format = getFixed( post_table );

      if( post_format.whole != 2 || post_format.fraction != 0 ) {
          qWarning("TrueType font does not have a format 2.0 'post' table");
          qWarning("post format is %d.%d",post_format.whole,post_format.fraction);
          // Sivan Feb 2001: no longer defective.
          // defective = TRUE;
      }
  }
#endif
  BYTE *maxp = getTable("maxp");
  if ( !maxp ) {
      defective = TRUE;
      qDebug("no maxp table in font");
      return;
  }
  numGlyphs = getUSHORT( maxp + 4 );
//  qDebug("number of glyphs is %d", numGlyphs);
  replacementList = makePSFontNameList( f, psname );
  uni2glyphSetup();
}


void QPSPrinterFontTTF::drawText( QTextStream &stream, const QPoint &p, QTextEngine *engine, int item,
				  const QString &text, QPSPrinterPrivate *d, QPainter *paint)
{
    // we draw glyphs here to get correct shaping of arabic and indic
    QScriptItem &si = engine->items[item];
    engine->shape( item );
    int len = si.num_glyphs;

    int x = p.x() + si.x;
    int y = p.y() + si.y;
    if ( y != d->textY || d->textY == 0 )
        stream << y << " Y";
    d->textY = y;

    QCString xyarray;
    int xo = 0;
    int yo = 0;

    glyph_t *glyphs = engine->glyphs( &si );
    advance_t *advances = engine->advances( &si );
    qoffset_t *offsets = engine->offsets( &si );
#ifdef Q_WS_X11
    int type = si.fontEngine->type();
    bool glyphIndices = (type == QFontEngine::Xft);
    // This helps us get arabic for XLFD fonts working. In that case we have a Unicode
    // cmap (== 0), and the glyphs array contains the shaped string.
    bool useGlyphAsUnicode = (type == QFontEngine::XLFD && si.fontEngine->cmap() == 0);
#else // Q_WS_QWS
    const bool glyphIndices = FALSE;
    const bool useGlyphAsUnicode = TRUE;
#endif
    stream << "<";
    if ( si.analysis.bidiLevel % 2 ) {
	for ( int i = len-1; i >=0; i-- ) {
	    // map unicode is not really the correct name, as we map glyphs, but we also download glyphs, so this works
            unsigned short glyph;
            if (glyphIndices)
                glyph = glyphs[i];
            else
                glyph = glyph_for_unicode(useGlyphAsUnicode ? glyphs[i] : text.unicode()[i].unicode());
	    stream << toHex(mapUnicode(glyph));
	    if ( i != len-1 ) {
		xyarray += toInt( xo + offsets[i].x + advances[i+1] );
		xyarray += " ";
		xyarray += toInt( yo + offsets[i].y );
		xyarray += " ";
		xo = -offsets[i].x;
		yo = -offsets[i].y;
	    }
	}
    } else {
	for ( int i = 0; i < len; i++ ) {
	    // map unicode is not really the correct name, as we map glyphs, but we also download glyphs, so this works
            unsigned short glyph;
            if (glyphIndices)
                glyph = glyphs[i];
            else
                glyph = glyph_for_unicode(useGlyphAsUnicode ? glyphs[i] : text.unicode()[i].unicode());
	    stream << toHex(mapUnicode(glyph));
	    if ( i ) {
		xyarray += toInt( xo + offsets[i].x + advances[i-1] );
		xyarray += " ";
		xyarray += toInt( yo + offsets[i].y );
		xyarray += " ";
		xo = -offsets[i].x;
		yo = -offsets[i].y;
	    }
	}
    }
    stream << ">";

    stream << "[" << xyarray << "0 0]";
    stream << si.width << " " << x;

    if ( paint->font().underline() )
        stream << ' ' << y + d->fm.underlinePos() + d->fm.lineWidth()
               << " " << d->fm.lineWidth() << " Tl";
    if ( paint->font().strikeOut() )
        stream << ' ' << y + d->fm.strikeOutPos()
               << " " << d->fm.lineWidth() << " Tl";
    stream << " XYT\n";

}


void QPSPrinterFontTTF::download(QTextStream& s,bool global)
{
    emitPSFontNameList( s, psname, replacementList);
    if ( !embedFonts ) {
        downloadMapping(s, global);
        return;
    }
    
    //qDebug("downloading ttf font %s", psname.latin1() );
    //qDebug("target type=%d", target_type);
    global_dict = global;
    QMap<unsigned short, unsigned short> *subsetDict = &subset;
    if ( !global )
        subsetDict = &page_subset;

    downloaded  = TRUE;

    if (defective) {
        s << "% Font ";
        s << FullName;
        s << " cannot be downloaded\n";
        return;
    }

    // === write header ===
    int VMMin;
    int VMMax;

    s << wrapDSC( "%%BeginFont: " + FullName );
    if( target_type == 42 ) {
        s << "%!PS-TrueTypeFont-"
          << TTVersion.whole
          << "."
          << TTVersion.fraction
          << "-"
          << MfrRevision.whole
          << "."
          << MfrRevision.fraction
          << "\n";
    } else {
        /* If it is not a Type 42 font, we will use a different format. */
        s << "%!PS-Adobe-3.0 Resource-Font\n";
    }     /* See RBIIp 641 */

    if( Copyright != (char*)NULL ) {
        s << wrapDSC( "%%Copyright: " + Copyright );
    }

    if( target_type == 42 )
        s << "%%Creator: Converted from TrueType to type 42 by Qt\n";
    else
        s << "%%Creator: Converted from TrueType by Qt\n";

    /* If VM usage information is available, print it. */
    if( target_type == 42 && post_table)
    {
        VMMin = (int)getULONG( post_table + 16 );
        VMMax = (int)getULONG( post_table + 20 );
        if( VMMin > 0 && VMMax > 0 )
            s << "%%VMUsage: " << VMMin << " " << VMMax << "\n";
    }

    /* Start the dictionary which will eventually */
    /* become the font. */
    if( target_type != 3 ) {
        s << "15 dict begin\n";
    }  else {
        s << "25 dict begin\n";

        /* Type 3 fonts will need some subroutines here. */
        s << "/_d{bind def}bind def\n";
        s << "/_m{moveto}_d\n";
        s << "/_l{lineto}_d\n";
        s << "/_cl{closepath eofill}_d\n";
        s << "/_c{curveto}_d\n";
        s << "/_sc{7 -1 roll{setcachedevice}{pop pop pop pop pop pop}ifelse}_d\n";
        s << "/_e{exec}_d\n";
    }

    s << "/FontName /";
    s << psname;
    s << " def\n";
    s << "/PaintType 0 def\n";

    if(target_type == 42)
        s << "/FontMatrix[1 0 0 1 0 0]def\n";
    else
        s << "/FontMatrix[.001 0 0 .001 0 0]def\n";

    s << "/FontBBox[";
    s<< llx;
    s << " ";
    s<< lly;
    s << " ";
    s<< urx;
    s << " ";
    s<< ury;
    s << "]def\n";

    s << "/FontType ";
    s<< target_type;
    s << " def\n";

    // === write encoding ===

    s << "/Encoding StandardEncoding def\n";

    // === write fontinfo dict ===

    /* We create a sub dictionary named "FontInfo" where we */
    /* store information which though it is not used by the */
    /* interpreter, is useful to some programs which will */
    /* be printing with the font. */
    s << "/FontInfo 10 dict dup begin\n";

    /* These names come from the TrueType font's "name" table. */
    s << "/FamilyName (";
    s << FamilyName;
    s << ") def\n";

    s << "/FullName (";
    s << FullName;
    s << ") def\n";

    s << "/Notice (";
    s << Copyright;
    s << " ";
    s << Trademark;
    s << ") def\n";

    /* This information is not quite correct. */
    s << "/Weight (";
    s << Style;
    s << ") def\n";

    /* Some fonts have this as "version". */
    s << "/Version (";
    s << Version;
    s << ") def\n";

    /* Some information from the "post" table. */
    if ( post_table ) {
        Fixed ItalicAngle = getFixed( post_table + 4 );
        s << "/ItalicAngle ";
        s << ItalicAngle.whole;
        s << ".";
        s << ItalicAngle.fraction;
        s << " def\n";

        s << "/isFixedPitch ";
        s << (getULONG( post_table + 12 ) ? "true" : "false" );
        s << " def\n";

        s << "/UnderlinePosition ";
        s << (int)getFWord( post_table + 8 );
        s << " def\n";

        s << "/UnderlineThickness ";
        s << (int)getFWord( post_table + 10 );
        s << " def\n";
    }
    s << "end readonly def\n";

#ifdef Q_PRINTER_USE_TYPE42
    /* If we are generating a type 42 font, */
    /* emmit the sfnts array. */
    if( target_type == 42 )
        download_sfnts(s);
#endif
    /* If we are generating a Type 3 font, we will need to */
    /* have the 'loca' and 'glyf' tables arround while */
    /* we are generating the CharStrings. */
    if(target_type == 3)
    {
        BYTE *ptr;                        /* We need only one value */
        ptr = getTable("hhea");
        numberOfHMetrics = getUSHORT(ptr + 34);

        loca_table = getTable("loca");
        glyf_table = getTable("glyf");
        hmtx_table = getTable("hmtx");
    }

    // ===  CharStrings array ===

    // subsetting. We turn a char subset into a glyph subset
    // and we mark as used the base glyphs of used composite glyphs.

    bool glyphset[65536];
    for(int c=0; c < 65536; c++)
        glyphset[c] = FALSE;
    glyphset[0] = TRUE; // always output .notdef

    QMap<unsigned short, unsigned short>::iterator it;
    for( it = subsetDict->begin(); it != subsetDict->end(); ++it ) {
	subsetGlyph( it.key(), glyphset );
    }
    int nGlyphs = numGlyphs;
    if ( target_type == 3 ) {
        nGlyphs = 0;;
        for(int c=0; c < 65536; c++)
            if ( glyphset[c] ) nGlyphs++;
    }

    s << "/CharStrings ";
    s << nGlyphs;
    s << " dict dup begin\n";

    // Emmit one key-value pair for each glyph.
    for(int x=0; x < 65536; x++) {
        if(target_type == 42) {
            s << "/";
            s << glyphName( x );
            s << " ";
            s << x;
            s << " def\n";
        } else { /* type 3 */
            if (!glyphset[x]) continue;

	    //qDebug("emitting charproc for glyph %d, name=%s", x, glyphName(x).latin1() );
            s << "/";
            s << glyphName( x, glyphset );
            s << "{";
            charproc(x,s, glyphset);
            s << "}_d\n";     /* "} bind def" */
        }
    }

    s << "end readonly def\n";

    // === trailer ===

    /* If we are generating a type 3 font, we need to provide */
    /* a BuildGlyph and BuildChar proceedures. */
    if( target_type == 3 ) {
        s << "\n";

        s << "/BuildGlyph\n";
        s << " {exch begin\n";              /* start font dictionary */
        s << " CharStrings exch\n";
        s << " 2 copy known not{pop /.notdef}if\n";
        s << " true 3 1 roll get exec\n";
        s << " end}_d\n";

        s << "\n";

        /* This proceedure is for compatiblity with */
        /* level 1 interpreters. */
        s << "/BuildChar {\n";
        s << " 1 index /Encoding get exch get\n";
        s << " 1 index /BuildGlyph get exec\n";
        s << "}_d\n";

        s << "\n";

    }

    /* If we are generating a type 42 font, we need to check to see */
    /* if this PostScript interpreter understands type 42 fonts.  If */
    /* it doesn't, we will hope that the Apple TrueType rasterizer */
    /* has been loaded and we will adjust the font accordingly. */
    /* I found out how to do this by examining a TrueType font */
    /* generated by a Macintosh.  That is where the TrueType interpreter */
    /* setup instructions and part of BuildGlyph came from. */
    else if( target_type == 42 ) {
        s << "\n";

        /* If we have no "resourcestatus" command, or FontType 42 */
        /* is unknown, leave "true" on the stack. */
        s << "systemdict/resourcestatus known\n";
        s << " {42 /FontType resourcestatus\n";
        s << "   {pop pop false}{true}ifelse}\n";
        s << " {true}ifelse\n";

        /* If true, execute code to produce an error message if */
        /* we can't find Apple's TrueDict in VM. */
        s << "{/TrueDict where{pop}{(%%[ Error: no TrueType rasterizer ]%%)= flush}ifelse\n";

        /* Since we are expected to use Apple's TrueDict TrueType */
        /* reasterizer, change the font type to 3. */
        s << "/FontType 3 def\n";

        /* Define a string to hold the state of the Apple */
        /* TrueType interpreter. */
        s << " /TrueState 271 string def\n";

        /* It looks like we get information about the resolution */
        /* of the printer and store it in the TrueState string. */
        s << " TrueDict begin sfnts save\n";
        s << " 72 0 matrix defaultmatrix dtransform dup\n";
        s << " mul exch dup mul add sqrt cvi 0 72 matrix\n";
        s << " defaultmatrix dtransform dup mul exch dup\n";
        s << " mul add sqrt cvi 3 -1 roll restore\n";
        s << " TrueState initer end\n";

        /* This BuildGlyph procedure will look the name up in the */
        /* CharStrings array, and then check to see if what it gets */
        /* is a procedure.  If it is, it executes it, otherwise, it */
        /* lets the TrueType rasterizer loose on it. */

        /* When this proceedure is executed the stack contains */
        /* the font dictionary and the character name.  We */
        /* exchange arguments and move the dictionary to the */
        /* dictionary stack. */
        s << " /BuildGlyph{exch begin\n";
        /* stack: charname */

        /* Put two copies of CharStrings on the stack and consume */
        /* one testing to see if the charname is defined in it, */
        /* leave the answer on the stack. */
        s << "  CharStrings dup 2 index known\n";
        /* stack: charname CharStrings bool */

        /* Exchange the CharStrings dictionary and the charname, */
        /* but if the answer was false, replace the character name */
        /* with ".notdef". */
        s << "    {exch}{exch pop /.notdef}ifelse\n";
        /* stack: CharStrings charname */

        /* Get the value from the CharStrings dictionary and see */
        /* if it is executable. */
        s << "  get dup xcheck\n";
        /* stack: CharStrings_entry */

        /* If is a proceedure.  Execute according to RBIIp 277-278. */
        s << "    {currentdict systemdict begin begin exec end end}\n";

        /* Is a TrueType character index, let the rasterizer at it. */
        s << "    {TrueDict begin /bander load cvlit exch TrueState render end}\n";

        s << "    ifelse\n";

        /* Pop the font's dictionary off the stack. */
        s << " end}bind def\n";

        /* This is the level 1 compatibility BuildChar procedure. */
        /* See RBIIp 281. */
        s << " /BuildChar{\n";
        s << "  1 index /Encoding get exch get\n";
        s << "  1 index /BuildGlyph get exec\n";
        s << " }bind def\n";

        /* Here we close the condition which is true */
        /* if the printer has no built-in TrueType */
        /* rasterizer. */
        s << "}if\n";
        s << "\n";
    } /* end of if Type 42 not understood. */

    s << "FontName currentdict end definefont pop\n";

    downloadMapping(s, global);
    s << "%%EndFont\n";
}

BYTE* QPSPrinterFontTTF::getTable(const char* name)
{
    BYTE *ptr;
    int x;

    /* We must search the table directory. */
    ptr = offset_table + 12;
    x=0;
    while (x != numTables) {
        if( strncmp((const char *)ptr,name,4) == 0 ) {
            ULONG offset;
            //ULONG length;
            BYTE *table;

            offset = getULONG( ptr + 8 );
            //length = getULONG( ptr + 12 );

            table = offset_table + offset;
            return table;
        }

        x++;
        ptr += 16;
    }

    return 0;
}

void QPSPrinterFontTTF::uni2glyphSetup()
{
  uni2glyph.resize(65536);
  int i;
  for (i=0; i<65536; i++) uni2glyph[i] = 0x0000;
  glyph2uni.resize(65536);
  for (i=0; i<65536; i++) glyph2uni[i] = 0x0000;

  unsigned char* cmap = getTable("cmap");
  int pos = 0;

  //USHORT version = getUSHORT(cmap + pos);
  pos += 2;
  USHORT nmaps   = getUSHORT(cmap + pos); pos += 2;

  //fprintf(stderr,"cmap version %d (should be 0), %d maps\n",version,nmaps);

  ULONG offset = 0;
  int map = -1;
  bool symbol = TRUE;
  for (i=0; i<nmaps; i++) {
    USHORT platform = getUSHORT(cmap+pos); pos+=2;
    USHORT encoding = getUSHORT(cmap+pos); pos+=2;
           offset   = getULONG( cmap+pos); pos+=4;
           //fprintf(stderr,"[%d] plat %d enc %d\n",i,platform,encoding);
	   if (platform == 3 && encoding == 1) {
	       map = i;
	       symbol = FALSE;
	       break; // unicode
	   }
	   if (platform == 3 && encoding == 0) {
	       // symbol, continue looking
	       map = i;
	   }
  }
  if (map==nmaps) {
    qWarning("Font does not have unicode encoding\n");
    return; // no unicode encoding!
  }

  pos = 8*map;
  //fprintf(stderr,"Doing Unicode encoding\n");

  pos = offset;
  USHORT format = getUSHORT(cmap+pos); pos+=2;
  //fprintf(stderr,"Unicode cmap format %d\n",format);

  if (format != 4) {
    //qWarning("Unicode cmap format is not 4");
    return;
  }

  pos += 2; // length
  pos += 2; // version
  USHORT segcount = getUSHORT(cmap+pos) / 2; pos+=2;

  //fprintf(stderr,"Unicode cmap seg count %d\n",segcount);

  // skip search data
  pos += 2;
  pos += 2;
  pos += 2;

  unsigned char* endcode    = cmap + offset + 14;
  unsigned char* startcode  = cmap + offset + 16 + 2*segcount;
  unsigned char* iddelta    = cmap + offset + 16 + 4*segcount;
  unsigned char* idrangeoff = cmap + offset + 16 + 6*segcount;
  //unsigned char* glyphid    = cmap + offset + 16 + 8*segcount;
  for (i=0; i<segcount; i++) {
    USHORT endcode_i    = getUSHORT(endcode   +2*i);
    USHORT startcode_i  = getUSHORT(startcode +2*i);
    SHORT iddelta_i    = getSHORT(iddelta   +2*i);
    USHORT idrangeoff_i = getUSHORT(idrangeoff+2*i);

//     fprintf(stderr,"[%d] %04x-%04x (%x %x)\n",
//         i,startcode_i,endcode_i,iddelta_i,idrangeoff_i);
    if (endcode_i == 0xffff) break; // last dummy segment

    if (idrangeoff_i == 0) {
      for (USHORT c = startcode_i; c <= endcode_i; c++) {
        USHORT g = c + iddelta_i; // glyph index
        if ( g != 0 ) {
            uni2glyph[g] = c;
            glyph2uni[c] = g;
        }
      }
    } else {
      for (USHORT c = startcode_i; c <= endcode_i; c++) {
        USHORT g = getUSHORT(idrangeoff+2*i
                             + 2*(c - startcode_i)
                             + idrangeoff_i);
        if ( g != 0 ) {
            uni2glyph[g] = c;
            glyph2uni[c] = g;
        }
      }
    }
  }
  if (symbol && glyph2uni[0x40] == 0 && glyph2uni[0xf040] != 0) {
      // map 0xf000-0xf0ff into latin1 range.
      for (int i = 0; i < 0x100; ++i) {
	  if (!glyph2uni[i])
	      glyph2uni[i] = glyph2uni[i+0xf000];

      }
  }
}

USHORT QPSPrinterFontTTF::unicode_for_glyph(int glyphindex)
{
  return uni2glyph[glyphindex];
}

USHORT QPSPrinterFontTTF::glyph_for_unicode(unsigned short unicode)
{
  return glyph2uni[unicode];
}

#ifdef Q_PRINTER_USE_TYPE42
// ****************** SNFTS ROUTINES *******

/*-------------------------------------------------------------------
** sfnts routines
** These routines generate the PostScript "sfnts" array which
** contains one or more strings which contain a reduced version
** of the TrueType font.
**
** A number of functions are required to accomplish this rather
** complicated task.
-------------------------------------------------------------------*/

// Write a BYTE as a hexadecimal value as part of the sfnts array.

void QPSPrinterFontTTF::sfnts_pputBYTE(BYTE n,QTextStream& s,
                                  int& string_len, int& line_len, bool& in_string)
{
  static const char hexdigits[]="0123456789ABCDEF";

  if(!in_string) {
    s << "<";
    string_len = 0;
    line_len++;
    in_string = TRUE;
  }

  s << hexdigits[ n / 16 ] ;
  s << hexdigits[ n % 16 ] ;
  string_len++;
  line_len+=2;

  if(line_len > 70) {
    s << "\n";
    line_len=0;
  }
}

// Write a USHORT as a hexadecimal value as part of the sfnts array.

void QPSPrinterFontTTF::sfnts_pputUSHORT(USHORT n,QTextStream& s,
                                  int& string_len, int& line_len, bool& in_string)
{
  sfnts_pputBYTE(n / 256,s, string_len, line_len, in_string);
  sfnts_pputBYTE(n % 256,s, string_len, line_len, in_string);
}


// Write a ULONG as part of the sfnts array.

void QPSPrinterFontTTF::sfnts_pputULONG(ULONG n,QTextStream& s,
                                  int& string_len, int& line_len, bool& in_string)
{
  int x1 = n % 256;   n /= 256;
  int x2 = n % 256;   n /= 256;
  int x3 = n % 256;   n /= 256;

  sfnts_pputBYTE(n,s , string_len, line_len, in_string);
  sfnts_pputBYTE(x3,s, string_len, line_len, in_string);
  sfnts_pputBYTE(x2,s, string_len, line_len, in_string);
  sfnts_pputBYTE(x1,s, string_len, line_len, in_string);
}

/*
** This is called whenever it is
** necessary to end a string in the sfnts array.
**
** (The array must be broken into strings which are
** no longer than 64K characters.)
*/
void QPSPrinterFontTTF::sfnts_end_string(QTextStream& s,
                                    int& string_len, int& line_len, bool& in_string)
{
  if(in_string) {
    string_len=0;               /* fool sfnts_pputBYTE() */

    // s << "\n% dummy byte:\n";

    // extra byte for pre-2013 compatibility
    sfnts_pputBYTE(0, s, string_len, line_len, in_string);

    s << ">";
    line_len++;
  }

  in_string=FALSE;
}

/*
** This is called at the start of each new table.
** The argement is the length in bytes of the table
** which will follow.  If the new table will not fit
** in the current string, a new one is started.
*/
void QPSPrinterFontTTF::sfnts_new_table(ULONG length,QTextStream& s,
                                   int& string_len, int& line_len, bool& in_string)
{
  if( (string_len + length) > 65528 )
    sfnts_end_string(s, string_len, line_len, in_string);
}

/*
** We may have to break up the 'glyf' table.  That is the reason
** why we provide this special routine to copy it into the sfnts
** array.
*/
void QPSPrinterFontTTF::sfnts_glyf_table(ULONG oldoffset,
                                    ULONG correct_total_length,
                                    QTextStream& s,
                                    int& string_len, int& line_len, bool& in_string)

{
  int x;
  ULONG off;
  ULONG length;
  int c;
  ULONG total=0;                /* running total of bytes written to table */

  loca_table = getTable("loca");

  int font_off = oldoffset;

  /* Copy the glyphs one by one */
  for(x=0; x < numGlyphs; x++) {
    /* Read the glyph offset from the index-to-location table. */
    if(indexToLocFormat == 0) {
      off = getUSHORT( loca_table + (x * 2) );
      off *= 2;
      length = getUSHORT( loca_table + ((x+1) * 2) );
      length *= 2;
      length -= off;
    } else {
      off = getULONG( loca_table + (x * 4) );
      length = getULONG( loca_table + ((x+1) * 4) );
      length -= off;
    }

    //  fprintf(stderr,"glyph length=%d",(int)length);

    /* Start new string if necessary. */
    sfnts_new_table( (int)length, s, string_len, line_len, in_string );

    /*
    ** Make sure the glyph is padded out to a
    ** two byte boundary.
    */
    if( length % 2 ) {
      qWarning("TrueType font contains a 'glyf' table without 2 byte padding");
      defective = TRUE;
      return;
    }

    /* Copy the bytes of the glyph. */
    while( length-- ) {
      c = offset_table[ font_off ];
      font_off++;

      sfnts_pputBYTE(c, s, string_len, line_len, in_string);
      total++;          /* add to running total */
    }
  }

  /* Pad out to full length from table directory */
  while( total < correct_total_length ) {
    sfnts_pputBYTE(0, s, string_len, line_len, in_string);
    total++;
  }

  /* Look for unexplainable descrepancies between sizes */
  if( total != correct_total_length ) {
    qWarning("QPSPrinterFontTTF::sfnts_glyf_table: total != correct_total_length");
    defective = TRUE;
    return;
  }
}

/*
** Here is the routine which ties it all together.
**
** Create the array called "sfnts" which
** holds the actual TrueType data.
*/

void QPSPrinterFontTTF::download_sfnts(QTextStream& s)
{
  // tables worth including in a type 42 font
  char *table_names[]= {
    "cvt ",
    "fpgm",
    "glyf",
    "head",
    "hhea",
    "hmtx",
    "loca",
    "maxp",
    "prep"
  };

  struct {                      /* The location of each of */
    ULONG oldoffset;    /* the above tables. */
    ULONG newoffset;
    ULONG length;
    ULONG checksum;
  } tables[9];

  int c;                        /* Input character. */
  int diff;
  int count;                    /* How many `important' tables did we find? */

  BYTE* ptr = offset_table + 12; // original table directory
  ULONG nextoffset=0;
  count=0;

  /*
  ** Find the tables we want and store there vital
  ** statistics in tables[].
  */
  for(int x=0; x < 9; x++ ) {
    do {
      diff = strncmp( (char*)ptr, table_names[x], 4 );

      if( diff > 0 ) {          /* If we are past it. */
        tables[x].length = 0;
        diff = 0;
      }
      else if( diff < 0 ) {             /* If we haven't hit it yet. */
        ptr += 16;
      }
      else if( diff == 0 ) {    /* Here it is! */
        tables[x].newoffset = nextoffset;
        tables[x].checksum = getULONG( ptr + 4 );
        tables[x].oldoffset = getULONG( ptr + 8 );
        tables[x].length = getULONG( ptr + 12 );
        nextoffset += ( ((tables[x].length + 3) / 4) * 4 );
        count++;
        ptr += 16;
      }
    } while(diff != 0);
  } /* end of for loop which passes over the table directory */

  /* Begin the sfnts array. */

  s << "/sfnts[<";

  bool in_string=TRUE;
  int  string_len=0;
  int  line_len=8;

  /* Generate the offset table header */
  /* Start by copying the TrueType version number. */
  ptr = offset_table;
  for(int x=0; x < 4; x++)
    sfnts_pputBYTE( *(ptr++) , s, string_len, line_len, in_string );

  /* Now, generate those silly numTables numbers. */
  sfnts_pputUSHORT(count,s, string_len, line_len, in_string);           /* number of tables */
  if( count == 9 ) {
    sfnts_pputUSHORT(7,s, string_len, line_len, in_string);             /* searchRange */
    sfnts_pputUSHORT(3,s, string_len, line_len, in_string);             /* entrySelector */
    sfnts_pputUSHORT(81,s, string_len, line_len, in_string);            /* rangeShift */
  }
  else {
    qWarning("Fewer than 9 tables selected");
  }

  /* Now, emmit the table directory. */
  for(int x=0; x < 9; x++) {
    if( tables[x].length == 0 ) /* Skip missing tables */
      continue;

    /* Name */
    sfnts_pputBYTE( table_names[x][0], s, string_len, line_len, in_string);
    sfnts_pputBYTE( table_names[x][1], s, string_len, line_len, in_string);
    sfnts_pputBYTE( table_names[x][2], s, string_len, line_len, in_string);
    sfnts_pputBYTE( table_names[x][3], s, string_len, line_len, in_string);

    /* Checksum */
    sfnts_pputULONG( tables[x].checksum, s, string_len, line_len, in_string );

    /* Offset */
    sfnts_pputULONG( tables[x].newoffset + 12 + (count * 16), s,
                     string_len, line_len, in_string );

    /* Length */
    sfnts_pputULONG( tables[x].length, s,
                     string_len, line_len, in_string );
  }

  /* Now, send the tables */
  for(int x=0; x < 9; x++) {
    if( tables[x].length == 0 ) /* skip tables that aren't there */
      continue;

    /* 'glyf' table gets special treatment */
    if( strcmp(table_names[x],"glyf")==0 ) {
      sfnts_glyf_table(tables[x].oldoffset,tables[x].length, s,
                       string_len, line_len, in_string);
    } else { // other tables should not exceed 64K (not always true; Sivan)
      if( tables[x].length > 65535 ) {
        qWarning("TrueType font has a table which is too long");
        defective = TRUE;
        return;
      }

      /* Start new string if necessary. */
      sfnts_new_table(tables[x].length, s,
                      string_len, line_len, in_string);

      int font_off = tables[x].oldoffset;
      /* Copy the bytes of the table. */
      for( int y=0; y < (int)tables[x].length; y++ ) {
        c = offset_table[ font_off ];
        font_off++;

        sfnts_pputBYTE(c, s, string_len, line_len, in_string);
      }
    }

    /* Padd it out to a four byte boundary. */
    int y=tables[x].length;
    while( (y % 4) != 0 ) {
      sfnts_pputBYTE(0, s, string_len, line_len, in_string);
      y++;
    }

  } /* End of loop for all tables */

  /* Close the array. */
  sfnts_end_string(s, string_len, line_len, in_string);
  s << "]def\n";
}
#endif

// ****************** Type 3 CharProcs *******

/*
** This routine is used to break the character
** procedure up into a number of smaller
** procedures.  This is necessary so as not to
** overflow the stack on certain level 1 interpreters.
**
** Prepare to push another item onto the stack,
** starting a new proceedure if necessary.
**
** Not all the stack depth calculations in this routine
** are perfectly accurate, but they do the job.
*/
static int stack_depth = 0;
static void stack(int num_pts, int newnew, QTextStream& s)
{
  if( num_pts > 25 ) {          /* Only do something of we will */
                                /* have a log of points. */
    if(stack_depth == 0) {
      s << "{";
      stack_depth=1;
    }

    stack_depth += newnew;              /* Account for what we propose to add */

    if(stack_depth > 100) {
      s << "}_e{";
      stack_depth = 3 + newnew; /* A rough estimate */
    }
  }
}

static void stack_end(QTextStream& s)                   /* called at end */
{
  if(stack_depth) {
    s << "}_e";
    stack_depth=0;
  }
}

// postscript drawing commands

static void PSMoveto(FWord x, FWord y, QTextStream& ts)
{
  ts << x;
  ts << " ";
  ts << y;
  ts << " _m\n";
}

static void PSLineto(FWord x, FWord y, QTextStream& ts)
{
  ts << x;
  ts << " ";
  ts << y;
  ts << " _l\n";
}

/* Emmit a PostScript "curveto" command. */
static void PSCurveto(FWord* xcoor, FWord* ycoor,
                      FWord x, FWord y, int s, int t, QTextStream& ts)
{
  int N, i;
  double sx[3], sy[3], cx[4], cy[4];

  N = t-s+2;
  for(i=0; i<N-1; i++) {
    sx[0] = i==0?xcoor[s-1]:(xcoor[i+s]+xcoor[i+s-1])/2;
    sy[0] = i==0?ycoor[s-1]:(ycoor[i+s]+ycoor[i+s-1])/2;
    sx[1] = xcoor[s+i];
    sy[1] = ycoor[s+i];
    sx[2] = i==N-2?x:(xcoor[s+i]+xcoor[s+i+1])/2;
    sy[2] = i==N-2?y:(ycoor[s+i]+ycoor[s+i+1])/2;
    cx[3] = sx[2];
    cy[3] = sy[2];
    cx[1] = (2*sx[1]+sx[0])/3;
    cy[1] = (2*sy[1]+sy[0])/3;
    cx[2] = (sx[2]+2*sx[1])/3;
    cy[2] = (sy[2]+2*sy[1])/3;

    ts << (int)cx[1];
    ts << " ";
    ts << (int)cy[1];
    ts << " ";
    ts << (int)cx[2];
    ts << " ";
    ts << (int)cy[2];
    ts << " ";
    ts << (int)cx[3];
    ts << " ";
    ts << (int)cy[3];
    ts << " _c\n";
  }
}

/* The PostScript bounding box. */
/* Variables to hold the character data. */

//void load_char(struct TTFONT *font, BYTE *glyph);
//void clear_data();

//void PSMoveto(FWord x, FWord y, QTextStream& ts);
//void PSLineto(FWord x, FWord y, QTextStream& ts);
//void PSCurveto(FWord x, FWord y, int s, int t, QTextStream& ts);

//double area(FWord *x, FWord *y, int n);
//int nextinctr(int co, int ci);
//int nextoutctr(int co);
//int nearout(int ci);
//double intest(int co, int ci);
#define sqr(x) ((x)*(x))

#define NOMOREINCTR -1
#define NOMOREOUTCTR -1

/*
** Find the area of a contour?
*/
static double area(FWord *x, FWord *y, int n)
{
  int i;
  double sum;

  sum=x[n-1]*y[0]-y[n-1]*x[0];
  for (i=0; i<=n-2; i++) sum += x[i]*y[i+1] - y[i]*x[i+1];
  return sum;
}

static int nextoutctr(int /*co*/, charproc_data* cd)
{
  int j;

  for(j=0; j<cd->num_ctr; j++)
    if (cd->check_ctr[j]==0 && cd->area_ctr[j] < 0) {
      cd->check_ctr[j]=1;
      return j;
    }

  return NOMOREOUTCTR;
} /* end of nextoutctr() */

static int nextinctr(int co, int /*ci*/, charproc_data* cd)
{
  int j;

  for(j=0; j<cd->num_ctr; j++)
    if (cd->ctrset[2*j+1]==co)
      if (cd->check_ctr[ cd->ctrset[2*j] ]==0) {
        cd->check_ctr[ cd->ctrset[2*j] ]=1;
        return cd->ctrset[2*j];
      }

  return NOMOREINCTR;
}

static double intest( int co, int ci, charproc_data *cd )
{
    int i, j, start, end;
    double r1, r2;
    FWord xi[3], yi[3];

    j = start = ( co == 0 ) ? 0 : ( cd->epts_ctr[co - 1] + 1 );
    end = cd->epts_ctr[co];
    i = ( ci == 0 ) ? 0 : ( cd->epts_ctr[ci - 1] + 1 );
    xi[0] = cd->xcoor[i];
    yi[0] = cd->ycoor[i];
    r1 = sqr( cd->xcoor[start] - xi[0] ) + sqr( cd->ycoor[start] - yi[0] );

    for ( i = start; i <= end; i++ ) {
	r2 = sqr( cd->xcoor[i] - xi[0] ) + sqr( cd->ycoor[i] - yi[0] );
	if ( r2 < r1 ) {
	    r1 = r2;
	    j = i;
	}
    }
    if ( j == start ) {
	xi[1] = cd->xcoor[end];
	yi[1] = cd->ycoor[end];
    } else {
	xi[1] = cd->xcoor[j - 1];
	yi[1] = cd->ycoor[j - 1];
    }
    if ( j == end ) {
	xi[2] = cd->xcoor[start];
	yi[2] = cd->ycoor[start];
    } else {
	xi[2] = cd->xcoor[j + 1];
	yi[2] = cd->ycoor[j + 1];
    }
    return area( xi, yi, 3 );
}

/*
** find the nearest out contour to a specified in contour.
*/
static int nearout(int ci, charproc_data* cd)
{
  int k = 0;                    /* !!! is this right? */
  int co;
  double a, a1=0;

  for (co=0; co < cd->num_ctr; co++) {
    if(cd->area_ctr[co] < 0) {
      a=intest(co,ci, cd);
      if (a<0 && a1==0) {
        k=co;
        a1=a;
      }
      if(a<0 && a1!=0 && a>a1) {
        k=co;
        a1=a;
      }
    }
  }

  return k;
} /* end of nearout() */


/*
** We call this routine to emmit the PostScript code
** for the character we have loaded with load_char().
*/
static void PSConvert(QTextStream& s, charproc_data* cd)
{
  int i,j,k,fst,start_offpt;
  int end_offpt=0;

  cd->area_ctr = new double[cd->num_ctr];
  memset(cd->area_ctr, 0, (cd->num_ctr*sizeof(double)));

  cd->check_ctr = new char[cd->num_ctr];
  memset(cd->check_ctr, 0, (cd->num_ctr*sizeof(char)));

  cd->ctrset = new int[2*(cd->num_ctr)];
  memset(cd->ctrset, 0, (cd->num_ctr*2*sizeof(int)));

  cd->check_ctr[0]=1;
  cd->area_ctr[0]=area(cd->xcoor, cd->ycoor, cd->epts_ctr[0]+1);

  for (i=1; i<cd->num_ctr; i++)
    cd->area_ctr[i]=area(cd->xcoor+cd->epts_ctr[i-1]+1,
                         cd->ycoor+cd->epts_ctr[i-1]+1,
                         cd->epts_ctr[i]-cd->epts_ctr[i-1]);

  for (i=0; i<cd->num_ctr; i++) {
    if (cd->area_ctr[i]>0) {
      cd->ctrset[2*i]=i;
      cd->ctrset[2*i+1]=nearout(i,cd);
    } else {
      cd->ctrset[2*i]=-1;
      cd->ctrset[2*i+1]=-1;
    }
  }

  /* Step thru the coutours. */
  /* I believe that a contour is a detatched */
  /* set of curves and lines. */
  i=j=k=0;
  while (i < cd->num_ctr ) {
    fst = j = (k==0) ? 0 : (cd->epts_ctr[k-1]+1);

    /* Move to the first point on the contour. */
    stack(cd->num_pts,3,s);
    PSMoveto(cd->xcoor[j],cd->ycoor[j],s);
    start_offpt = 0;            /* No off curve points yet. */

    /* Step thru the remaining points of this contour. */
    for(j++; j <= cd->epts_ctr[k]; j++) {
      if (!(cd->tt_flags[j]&1)) { /* Off curve */
        if (!start_offpt)
          { start_offpt = end_offpt = j; }
        else
          end_offpt++;
      } else {                  /* On Curve */
        if (start_offpt) {
          stack(cd->num_pts,7,s);
          PSCurveto(cd->xcoor,cd->ycoor,
                    cd->xcoor[j],cd->ycoor[j],
                    start_offpt,end_offpt,s);
          start_offpt = 0;
        } else {
          stack(cd->num_pts,3,s);
          PSLineto(cd->xcoor[j], cd->ycoor[j],s);
        }
      }
    }

    /* Do the final curve or line */
    /* of this coutour. */
    if (start_offpt) {
      stack(cd->num_pts,7,s);
      PSCurveto(cd->xcoor,cd->ycoor,
                cd->xcoor[fst],cd->ycoor[fst],
                start_offpt,end_offpt,s);
    } else {
      stack(cd->num_pts,3,s);
      PSLineto(cd->xcoor[fst],cd->ycoor[fst],s);
    }

    k=nextinctr(i,k,cd);

    if (k==NOMOREINCTR)
      i=k=nextoutctr(i,cd);

    if (i==NOMOREOUTCTR)
      break;
  }

  /* Now, we can fill the whole thing. */
  stack(cd->num_pts,1,s);
  s << "_cl";           /* "closepath eofill" */

  /* Free our work arrays. */
  delete [] cd->area_ctr;
  delete [] cd->check_ctr;
  delete [] cd->ctrset;
}


/*
** Load the simple glyph data pointed to by glyph.
** The pointer "glyph" should point 10 bytes into
** the glyph data.
*/
void QPSPrinterFontTTF::charprocLoad(BYTE *glyph, charproc_data* cd)
{
  int x;
  BYTE c, ct;

  /* Read the contour endpoints list. */
  cd->epts_ctr = new int[cd->num_ctr];
  //cd->epts_ctr = (int *)myalloc(cd->num_ctr,sizeof(int));
  for (x = 0; x < cd->num_ctr; x++) {
    cd->epts_ctr[x] = getUSHORT(glyph);
    glyph += 2;
  }

  /* From the endpoint of the last contour, we can */
  /* determine the number of points. */
  cd->num_pts = cd->epts_ctr[cd->num_ctr-1]+1;
#ifdef DEBUG_TRUETYPE
  fprintf(stderr,"num_pts=%d\n",cd->num_pts);
#endif

  /* Skip the instructions. */
  x = getUSHORT(glyph);
  glyph += 2;
  glyph += x;

  /* Allocate space to hold the data. */
  //cd->tt_flags = (BYTE *)myalloc(num_pts,sizeof(BYTE));
  //cd->xcoor    = (FWord *)myalloc(num_pts,sizeof(FWord));
  //cd->ycoor    = (FWord *)myalloc(num_pts,sizeof(FWord));
  cd->tt_flags = new BYTE[cd->num_pts];
  cd->xcoor    = new FWord[cd->num_pts];
  cd->ycoor    = new FWord[cd->num_pts];

  /* Read the flags array, uncompressing it as we go. */
  /* There is danger of overflow here. */
  for (x = 0; x < cd->num_pts; ) {
    cd->tt_flags[x++] = c = *(glyph++);

    if (c&8) {          /* If next byte is repeat count, */
      ct = *(glyph++);

      if( (x + ct) > cd->num_pts ) {
        qWarning("Fatal Error in TT flags");
        return;
      }

      while (ct--)
        cd->tt_flags[x++] = c;
    }
  }

  /* Read the x coordinates */
  for (x = 0; x < cd->num_pts; x++) {
    if (cd->tt_flags[x] & 2) {          /* one byte value with */
                                        /* external sign */
      c = *(glyph++);
      cd->xcoor[x] = (cd->tt_flags[x] & 0x10) ? c : (-1 * (int)c);
    } else if(cd->tt_flags[x] & 0x10) { /* repeat last */
      cd->xcoor[x] = 0;
    } else {                            /* two byte signed value */
      cd->xcoor[x] = getFWord(glyph);
      glyph+=2;
    }
  }

  /* Read the y coordinates */
  for(x = 0; x < cd->num_pts; x++) {
    if (cd->tt_flags[x] & 4) {          /* one byte value with */
                                        /* external sign */
      c = *(glyph++);
      cd->ycoor[x] = (cd->tt_flags[x] & 0x20) ? c : (-1 * (int)c);
    } else if (cd->tt_flags[x] & 0x20) {        /* repeat last value */
      cd->ycoor[x] = 0;
    } else {                    /* two byte signed value */
      cd->ycoor[x] = getUSHORT(glyph);
      glyph+=2;
    }
  }

  /* Convert delta values to absolute values. */
  for(x = 1; x < cd->num_pts; x++) {
    cd->xcoor[x] += cd->xcoor[x-1];
    cd->ycoor[x] += cd->ycoor[x-1];
  }

  for(x=0; x < cd->num_pts; x++) {
    cd->xcoor[x] = topost(cd->xcoor[x]);
    cd->ycoor[x] = topost(cd->ycoor[x]);
  }
}

#define ARG_1_AND_2_ARE_WORDS 1
#define ARGS_ARE_XY_VALUES 2
#define ROUND_XY_TO_GRID 4
#define WE_HAVE_A_SCALE 8
/* RESERVED 16 */
#define MORE_COMPONENTS 32
#define WE_HAVE_AN_X_AND_Y_SCALE 64
#define WE_HAVE_A_TWO_BY_TWO 128
#define WE_HAVE_INSTRUCTIONS 256
#define USE_MY_METRICS 512

void QPSPrinterFontTTF::subsetGlyph(int charindex,bool* glyphset)
{
  USHORT flags;
  USHORT glyphIndex;
  charproc_data cd;

  glyphset[charindex] = TRUE;
  //printf("subsetting %s ==> ",glyphName(charindex).latin1());

  /* Get a pointer to the data. */
  BYTE* glyph = charprocFindGlyphData( charindex );

  /* If the character is blank, it has no bounding box, */
  /* otherwise read the bounding box. */
  if( glyph == (BYTE*)NULL ) {
    cd.num_ctr=0;
  } else {
    cd.num_ctr = getSHORT(glyph);
    /* Advance the pointer past bounding box. */
    glyph += 10;
  }

  if( cd.num_ctr < 0 ) { // composite
    /* Once around this loop for each component. */
    do {
      flags = getUSHORT(glyph); /* read the flags word */
      glyph += 2;
      glyphIndex = getUSHORT(glyph);    /* read the glyphindex word */
      glyph += 2;

      glyphset[ glyphIndex ] = TRUE;
      subsetGlyph( glyphIndex, glyphset );
      //printf("subset contains: %d %s ",glyphIndex, glyphName(glyphIndex).latin1());

      if(flags & ARG_1_AND_2_ARE_WORDS) {
        glyph += 2;
        glyph += 2;
      } else {
        glyph += 1;
        glyph += 1;
      }

      if(flags & WE_HAVE_A_SCALE) {
        glyph += 2;
      } else if(flags & WE_HAVE_AN_X_AND_Y_SCALE) {
        glyph += 2;
        glyph += 2;
      } else if(flags & WE_HAVE_A_TWO_BY_TWO) {
        glyph += 2;
        glyph += 2;
        glyph += 2;
        glyph += 2;
      } else {
      }
    } while(flags & MORE_COMPONENTS);
  }
  //printf("\n");
}


/*
** Emmit PostScript code for a composite character.
*/
void QPSPrinterFontTTF::charprocComposite(BYTE *glyph, QTextStream& s, bool *glyphSet)
{
  USHORT flags;
  USHORT glyphIndex;
  int arg1;
  int arg2;
  float xscale = 1;
  float yscale = 1;
#ifdef DEBUG_TRUETYPE
  float scale01 = 0;
  float scale10 = 0;
#endif

  /* Once around this loop for each component. */
  do {
      flags = getUSHORT(glyph);   /* read the flags word */
      glyph += 2;

      glyphIndex = getUSHORT(glyph);      /* read the glyphindex word */
      glyph += 2;

      if(flags & ARG_1_AND_2_ARE_WORDS) {
                                /* The tt spec. seems to say these are signed. */
	  arg1 = getSHORT(glyph);
	  glyph += 2;
	  arg2 = getSHORT(glyph);
	  glyph += 2;
      } else {                    /* The tt spec. does not clearly indicate */
                                /* whether these values are signed or not. */
	  arg1 = (char)*(glyph++);
	  arg2 = (char)*(glyph++);
      }

      if(flags & WE_HAVE_A_SCALE) {
	  xscale = yscale = f2dot14( getUSHORT(glyph) );
	  glyph += 2;
      } else if(flags & WE_HAVE_AN_X_AND_Y_SCALE) {
	  xscale = f2dot14( getUSHORT(glyph) );
	  glyph += 2;
	  yscale = f2dot14( getUSHORT(glyph) );
	  glyph += 2;
      } else if(flags & WE_HAVE_A_TWO_BY_TWO) {
	  xscale = f2dot14( getUSHORT(glyph) );
	  glyph += 2;
#ifdef DEBUG_TRUETYPE
	  scale01 = f2dot14( getUSHORT(glyph) );
#endif
	  glyph += 2;
#ifdef DEBUG_TRUETYPE
	  scale10 = f2dot14( getUSHORT(glyph) );
#endif
	  glyph += 2;
	  yscale = f2dot14( getUSHORT(glyph) );
	  glyph += 2;
      }

      /* Debugging */
#ifdef DEBUG_TRUETYPE
      s << "% flags=" << flags << ", arg1=" << arg1 << ", arg2=" << arg2 << ", xscale=" << xscale << ", yscale=" << yscale <<
	  ", scale01=" << scale01 << ", scale10=" << scale10 << endl;
#endif


      if ( (flags & ARGS_ARE_XY_VALUES) != ARGS_ARE_XY_VALUES ) {
	  s << "% unimplemented shift, arg1=" << arg1;
	  s << ", arg2=" << arg2 << "\n";
	  arg1 = arg2 = 0;
      }

      /* If we have an (X,Y) shif and it is non-zero, */
      /* translate the coordinate system. */
      if ( flags & (WE_HAVE_A_TWO_BY_TWO|WE_HAVE_AN_X_AND_Y_SCALE) ) {
#if 0
	  // code similar to this would be needed for two_by_two
	  s << "gsave [ " << xscale << " " << scale01 << " " << scale10 << " "
	    << yscale << " " << topost(arg1) << " " << topost(arg2) << "] SM\n";
#endif
	  if ( flags & WE_HAVE_A_TWO_BY_TWO )
	      s << "% Two by two transformation, unimplemented\n";
	  s << "gsave " << topost(arg1);
	  s << " " << topost(arg2);
	  s << " translate\n";
	  s << xscale << " " << yscale << " scale\n";
      } else if ( flags & ARGS_ARE_XY_VALUES && ( arg1 != 0 || arg2 != 0 ) ) {
	  s << "gsave " << topost(arg1);
	  s << " " << topost(arg2);
	  s << " translate\n";
      }

      /* Invoke the CharStrings procedure to print the component. */
      s << "false CharStrings /";
      s << glyphName( glyphIndex, glyphSet );
      s << " get exec\n";

      //  printf("false CharStrings /%s get exec\n",
      //ttfont_CharStrings_getname(font,glyphIndex));

      /* If we translated the coordinate system, */
      /* put it back the way it was. */
      if( (flags & ARGS_ARE_XY_VALUES && (arg1 != 0 || arg2 != 0) ) ||
	  ( flags & (WE_HAVE_A_TWO_BY_TWO|WE_HAVE_AN_X_AND_Y_SCALE) ) ) {
	  s << "grestore ";
      }
  } while (flags & MORE_COMPONENTS);
}

/*
** Return a pointer to a specific glyph's data.
*/
BYTE* QPSPrinterFontTTF::charprocFindGlyphData(int charindex)
{
  ULONG off;
  ULONG length;

  /* Read the glyph offset from the index to location table. */
  if(indexToLocFormat == 0) {
    off = getUSHORT( loca_table + (charindex * 2) );
    off *= 2;
    length = getUSHORT( loca_table + ((charindex+1) * 2) );
    length *= 2;
    length -= off;
  } else {
    off = getULONG( loca_table + (charindex * 4) );
    length = getULONG( loca_table + ((charindex+1) * 4) );
    length -= off;
  }

  if(length > 0)
    return glyf_table + off;
  else
    return (BYTE*)NULL;
}

void QPSPrinterFontTTF::charproc(int charindex, QTextStream& s, bool *glyphSet )
{
  int llx,lly,urx,ury;
  int advance_width;
  charproc_data cd;

#ifdef DEBUG_TRUETYPE
  s << "% tt_type3_charproc for ";
  s << charindex;
  s << "\n";
#endif

  /* Get a pointer to the data. */
  BYTE* glyph = charprocFindGlyphData( charindex );

  /* If the character is blank, it has no bounding box, */
  /* otherwise read the bounding box. */
  if( glyph == (BYTE*)NULL ) {
    llx=lly=urx=ury=0;  /* A blank char has an all zero BoundingBox */
    cd.num_ctr=0;               /* Set this for later if()s */
  } else {
    /* Read the number of contours. */
    cd.num_ctr = getSHORT(glyph);

    /* Read PostScript bounding box. */
    llx = getFWord(glyph + 2);
    lly = getFWord(glyph + 4);
    urx = getFWord(glyph + 6);
    ury = getFWord(glyph + 8);

    /* Advance the pointer. */
    glyph += 10;
  }

  /* If it is a simple character, load its data. */
  if (cd.num_ctr > 0)
    charprocLoad(glyph, &cd);
  else
    cd.num_pts=0;

  /* Consult the horizontal metrics table to determine */
  /* the character width. */
  if( charindex < numberOfHMetrics )
    advance_width = getuFWord( hmtx_table + (charindex * 4) );
  else
    advance_width = getuFWord( hmtx_table + ((numberOfHMetrics-1) * 4) );

  /* Execute setcachedevice in order to inform the font machinery */
  /* of the character bounding box and advance width. */
  stack(cd.num_pts,7,s);
  s << topost(advance_width);
  s << " 0 ";
  s << topost(llx);
  s << " ";
  s << topost(lly);
  s << " ";
  s << topost(urx);
  s << " ";
  s << topost(ury);
  s << " _sc\n";

  /* If it is a simple glyph, convert it, */
  /* otherwise, close the stack business. */
  if( cd.num_ctr > 0 ) {        // simple
    PSConvert(s,&cd);
    delete [] cd.tt_flags;
    delete [] cd.xcoor;
    delete [] cd.ycoor;
    delete [] cd.epts_ctr;
  } else if( cd.num_ctr < 0 ) { // composite
    charprocComposite(glyph,s, glyphSet);
  }

  stack_end(s);
} /* end of tt_type3_charproc() */


// ================== PFA ====================

class QPSPrinterFontPFA
    : public QPSPrinterFontPrivate {
public:
    QPSPrinterFontPFA(const QFontEngine *f, QByteArray& data);
    virtual void    download(QTextStream& s, bool global);
    virtual bool embedded() { return TRUE; }
private:
    QByteArray     data;
};

QPSPrinterFontPFA::QPSPrinterFontPFA(const QFontEngine *f, QByteArray& d)
{
  data = d;

  int pos = 0;
  char* p = data.data();
  QString fontname;

  if (p[ pos ] != '%' || p[ pos+1 ] != '!') { // PFA marker
    qWarning("invalid pfa file");
    return;
  }

  char* fontnameptr = strstr(p+pos,"/FontName");
  if (fontnameptr == NULL)
    return;

  fontnameptr += strlen("/FontName") + 1;
  while (*fontnameptr == ' ' || *fontnameptr == '/') fontnameptr++;
  int l=0;
  while (fontnameptr[l] != ' ') l++;

  psname = QString::fromLatin1(fontnameptr,l);
  replacementList = makePSFontNameList( f, psname );
}

void QPSPrinterFontPFA::download(QTextStream& s, bool global)
{
    emitPSFontNameList( s, psname, replacementList);

    if ( !embedFonts ) {
        downloadMapping(s, global);
        return;
    }
    
    //qDebug("downloading pfa font %s", psname.latin1() );
    char* p = data.data();
    
    s << "% Font resource\n";
    for (int i=0; i < (int)data.size(); i++) s << p[i];
    s << "% End of font resource\n";
    downloadMapping( s, global );
}

// ================== PFB ====================

class QPSPrinterFontPFB
    : public QPSPrinterFontPrivate {
public:
    QPSPrinterFontPFB(const QFontEngine *f, QByteArray& data);
    virtual void    download(QTextStream& s, bool global);
    virtual bool embedded() { return TRUE; }
private:
    QByteArray     data;
};

QPSPrinterFontPFB::QPSPrinterFontPFB(const QFontEngine *f, QByteArray& d)
{
  data = d;

  int pos = 0;
  int len;
  //  int typ;
  unsigned char* p = (unsigned char*) data.data();
  QString fontname;

  if (p[ pos ] != 0x80) { // PFB marker
    qWarning("pfb file does not start with 0x80");
    return;
  }
  pos++;
  //  typ = p[ pos ]; // 1=ascii 2=binary 3=done
  pos++;
  len = p[ pos ];          pos++;
  len |= (p[ pos ] << 8) ; pos++;
  len |= (p[ pos ] << 16); pos++;
  len |= (p[ pos ] << 24); pos++;

  //printf("font block type %d len %d\n",typ,len);

  char* fontnameptr = strstr((char*)p+pos,"/FontName");
  if (fontnameptr == NULL)
    return;

  fontnameptr += strlen("/FontName") + 1;
  while (*fontnameptr == ' ' || *fontnameptr == '/') fontnameptr++;
  int l=0;
  while (fontnameptr[l] != ' ') l++;

  psname = QString::fromLatin1(fontnameptr,l);
  replacementList = makePSFontNameList( f, psname );
}

void QPSPrinterFontPFB::download(QTextStream& s, bool global)
{
    emitPSFontNameList( s, psname, replacementList);
    
    if ( !embedFonts ) {
        downloadMapping(s, global);
        return;
    }
    
    //qDebug("downloading pfb font %s", psname.latin1() );
    unsigned char* p = (unsigned char*) data.data();
    int pos;
    int len;
    int typ;

    int hexcol = 0;
    int line_length = 64;

    s << "% Font resource\n";

    pos = 0;
    typ = -1;
    while (typ != 3) { // not end of file
	if (p[ pos ] != 0x80) // PFB marker
	    return; // pfb file does not start with 0x80
	pos++;
	typ = p[ pos ]; // 1=ascii 2=binary 3=done
	pos++;

	if (typ == 3) break;

	len = p[ pos ];         pos++;
	len |= (p[ pos ] << 8) ; pos++;
	len |= (p[ pos ] << 16); pos++;
	len |= (p[ pos ] << 24); pos++;

	//qDebug("font block type %d len %d",typ,len);

	int end = pos + len;
	if (typ==1) {
	    while (pos < end) {
		if (hexcol > 0) {
		    s << "\n";
		    hexcol = 0;
		}
		//qWarning(QString::fromLatin1((char*)(p+pos),1));
		if (p[pos] == '\r' || p[pos] == '\n') {
		    s << "\n";
		    while (pos < end && (p[pos] == '\r' || p[pos] == '\n'))
			pos++;
		} else {
		    s << QString::fromLatin1((char*)(p+pos),1);
		    pos++;
		}
	    }
	}
	if (typ==2) {
	    static const char *hexchar = "0123456789abcdef";
	    while (pos < end) {
		/* trim hexadecimal lines to line_length columns */
		if (hexcol >= line_length) {
		    s << "\n";
		    hexcol = 0;
		}
		s << QString::fromLatin1(hexchar+((p[pos] >> 4) & 0xf),1)
		  << QString::fromLatin1(hexchar+((p[pos]     ) & 0xf),1);
		pos++;
		hexcol += 2;
	    }
	}
    }
    s << "% End of font resource\n";
    downloadMapping( s, global );
}

// ================== AFontFileNotFound ============



class QPSPrinterFontNotFound
  : public QPSPrinterFontPrivate {
public:
  QPSPrinterFontNotFound(const QFontEngine* f);
  virtual void    download(QTextStream& s, bool global);
private:
  QByteArray     data;
};

QPSPrinterFontNotFound::QPSPrinterFontNotFound(const QFontEngine* f)
{
    psname = makePSFontName( f );
    replacementList = makePSFontNameList( f );
}

void QPSPrinterFontNotFound::download(QTextStream& s, bool)
{
    //qDebug("downloading not found font %s", psname.latin1() );
    emitPSFontNameList( s, psname, replacementList );
    s << "% No embeddable font for ";
    s << psname;
    s << " found\n";
    QPSPrinterFontPrivate::download(s, TRUE);
}

#ifndef QT_NO_TEXTCODEC
// =================== A font file for asian ============

class QPSPrinterFontAsian
  : public QPSPrinterFontPrivate {
public:
      QPSPrinterFontAsian()
	  : QPSPrinterFontPrivate(), codec( 0 ) {}
      void download(QTextStream& s, bool global);
      QString defineFont( QTextStream &stream, const QString &ps, const QFont &f, const QString &key,
                          QPSPrinterPrivate *d );
      void drawText( QTextStream &stream, const QPoint &p, QTextEngine *engine, int item,
                     const QString &text, QPSPrinterPrivate *d, QPainter *paint );

      QString makePSFontName( const QFontEngine *f, int type ) const;
      virtual QString extension() const = 0;

      QTextCodec *codec;
};

QString QPSPrinterFontAsian::makePSFontName( const QFontEngine *f, int type ) const
{
    QString ps;
    int i;

    QString family = f->fontDef.family.lower();

    // try to make a "good" postscript name
    ps = family.simplifyWhiteSpace();
    i = 0;
    while( (unsigned int)i < ps.length() ) {
        if ( i != 0 && ps[i] == '[') {
          if ( ps[i-1] == ' ' )
	    ps.truncate (i-1);
          else
	    ps.truncate (i);
          break;
        }
	if ( i == 0 || ps[i-1] == ' ' ) {
	    ps[i] = ps[i].upper();
	    if ( i )
		ps.remove( i-1, 1 );
	    else
		i++;
	} else {
	    i++;
	}
    }

    switch ( type ) {
	case 1:
	    ps.append( QString::fromLatin1("-Italic") );
	    break;
	case 2:
	    ps.append( QString::fromLatin1("-Bold") );
	    break;
	case 3:
	    ps.append( QString::fromLatin1("-BoldItalic") );
	    break;
	case 0:
	default:
	    break;
    }

    ps += extension();

    return ps;
}


QString QPSPrinterFontAsian::defineFont( QTextStream &stream, const QString &ps, const QFont &f,
					 const QString &key, QPSPrinterPrivate *d)
{
    QString fontName;
    QString fontName2;

    QString *tmp = d->headerFontNames.find( ps );

    if ( d->buffer ) {
        if ( tmp ) {
            fontName = *tmp;
        } else {
	    fontName.sprintf( "F%d", ++d->headerFontNumber );
	    d->fontStream << "/" << fontName << " false " << ps << "List MF\n";
	    d->headerFontNames.insert( ps, new QString( fontName ) );
	}
        fontName2.sprintf( "F%d", ++d->headerFontNumber );
        d->fontStream << "/" << fontName2 << " "
                      << pointSize( f, d->scale ) << "/" << fontName << " DF\n";
        d->headerFontNames.insert( key, new QString( fontName2 ) );
    } else {
        if ( tmp ) {
            fontName = *tmp;
        } else {
	    fontName.sprintf( "F%d", ++d->pageFontNumber );
	    stream << "/" << fontName << " false " << ps << "List MF\n";
	    d->pageFontNames.insert( ps, new QString( fontName ) );
	}
        fontName2.sprintf( "F%d", ++d->pageFontNumber );
        stream << "/" << fontName2 << " "
               << pointSize( f, d->scale ) << "/" << fontName << " DF\n";
        d->pageFontNames.insert( key, new QString( fontName2 ) );
    }
    return fontName2;
}


void QPSPrinterFontAsian::download(QTextStream& s, bool)
{
    //qDebug("downloading asian font %s", psname.latin1() );
    s << "% Asian postscript font requested. Using "
      << psname << endl;
    emitPSFontNameList( s, psname, replacementList );
}

void QPSPrinterFontAsian::drawText( QTextStream &stream, const QPoint &p, QTextEngine *engine, int item,
				    const QString &text, QPSPrinterPrivate *d, QPainter *paint)
{
    int len = engine->length( item );
    QScriptItem &si = engine->items[item];

    int x = p.x() + si.x;
    int y = p.y() + si.y;
    if ( y != d->textY || d->textY == 0 )
        stream << y << " Y";
    d->textY = y;

    QString mdf;
    if ( paint->font().underline() )
        mdf += " " + QString().setNum( y + d->fm.underlinePos() + d->fm.lineWidth() ) +
               " " + toString( d->fm.lineWidth() ) + " Tl";
    if ( paint->font().strikeOut() )
        mdf += " " + QString().setNum( y + d->fm.strikeOutPos() ) +
               " " + toString( d->fm.lineWidth() ) + " Tl";
    QCString mb;
    QCString out;
    QString dummy( QChar(0x20) );

    if ( si.analysis.bidiLevel % 2 ) {
	for ( int i = len-1; i >= 0; i-- ) {
	    QChar ch = text.unicode()[i];
	    if ( !ch.row() ) {
		; // ignore, we should never get here anyway
	    } else {
		if ( codec ) {
		    dummy[0] = ch;
		    mb = codec->fromUnicode( dummy );
		} else
		    mb = "  ";

		for ( unsigned int j = 0; j < mb.length (); j++ ) {
		    if ( mb.at(j) == '(' || mb.at(j) == ')' || mb.at(j) == '\\' )
			out += "\\";
		    out += mb.at(j);
		}
	    }
	}
    } else {
	for ( int i = 0; i < len; i++ ) {
	    QChar ch = text.unicode()[i];
	    if ( !ch.row() ) {
		; // ignore, we should never get here anyway
	    } else {
		if ( codec ) {
		    dummy[0] = ch;
		    mb = codec->fromUnicode( dummy );
		} else
		    mb = "  ";

		for ( unsigned int j = 0; j < mb.length (); j++ ) {
		    if ( mb.at(j) == '(' || mb.at(j) == ')' || mb.at(j) == '\\' )
			out += "\\";
		    out += mb.at(j);
		}
	    }
	}
    }
    stream << "(" << out << ")" << si.width << " " << x << mdf << " AT\n";
}

// ----------- Japanese --------------

static const psfont Japanese1 [] = {
    { "Ryumin-Light-H", 0, 100. },
    { "Ryumin-Light-H", 0.2, 100. },
    { "GothicBBB-Medium-H", 0, 100. },
    { "GothicBBB-Medium-H", 0.2, 100. }
};

static const psfont Japanese1a [] = {
    { "GothicBBB-Medium-H", 0, 100. },
    { "GothicBBB-Medium-H", 0.2, 100. },
    { "Ryumin-Light-H", 0, 100. },
    { "Ryumin-Light-H", 0.2, 100. }
};

static const psfont Japanese2 [] = {
    { "GothicBBB-Medium-H", 0, 100. },
    { "GothicBBB-Medium-H", 0.2, 100. },
    { "GothicBBB-Medium-H", 0, 100. },
    { "GothicBBB-Medium-H", 0.2, 100. }
};

static const psfont Japanese2a [] = {
    { "Ryumin-Light-H", 0, 100. },
    { "Ryumin-Light-H", 0.2, 100. },
    { "Ryumin-Light-H", 0, 100. },
    { "Ryumin-Light-H", 0.2, 100. }
};


// Wadalab fonts

static const psfont WadaMin [] = {
    { "WadaMin-Regular-H", 0, 100. },
    { "WadaMin-Regular-H", 0.2, 100. },
    { "WadaMin-Bold-H", 0, 100. },
    { "WadaMin-Bold-H", 0.2, 100. }
};

static const psfont WadaGo [] = {
    { "WadaMaruGo-Regular-H", 0, 100. },
    { "WadaMaruGo-Regular-H", 0.2, 100. },
    { "WadaGo-Bold-H", 0, 100. },
    { "WadaGo-Bold-H", 0.2, 100. }
};

// Adobe Wadalab

static const psfont WadaGoAdobe [] = {
    { "WadaMaruGo-RegularH-Hojo-H", 0, 100. },
    { "WadaMaruGo-RegularH-Hojo-H", 0.2, 100. },
    { "WadaMaruGo-RegularH-Hojo-H", 0, 100. },
    { "WadaMaruGo-RegularH-Hojo-H", 0.2, 100. },
};
static const psfont WadaMinAdobe [] = {
    { "WadaMin-RegularH-Hojo-H", 0, 100. },
    { "WadaMin-RegularH-Hojo-H", 0.2, 100. },
    { "WadaMin-RegularH-Hojo-H", 0, 100. },
    { "WadaMin-RegularH-Hojo-H", 0.2, 100. },
};


static const psfont * const Japanese1Replacements[] = {
    Japanese1, Japanese1a, WadaMin, WadaGo, WadaMinAdobe, WadaGoAdobe, 0
};
static const psfont * const Japanese2Replacements[] = {
    Japanese2, Japanese2a, WadaMin, WadaGo, WadaMinAdobe, WadaGoAdobe, 0
};

class QPSPrinterFontJapanese
  : public QPSPrinterFontAsian {
public:
      QPSPrinterFontJapanese(const QFontEngine* f);
      virtual QString extension() const;
};

QPSPrinterFontJapanese::QPSPrinterFontJapanese(const QFontEngine* f)
{
    codec = QTextCodec::codecForMib( 63 ); // jisx0208.1983-0

    int type = getPsFontType( f );
    psname = makePSFontName( f, type );
    QString best = "[ /" + psname + " 1.0 0.0 ]";
    replacementList.append( best );

    const psfont *const *replacements = ( psname.contains( "Helvetica" ) ? Japanese2Replacements : Japanese1Replacements );
    appendReplacements( replacementList, replacements, type );
}

QString QPSPrinterFontJapanese::extension() const
{
    return "-H";
}

// ----------- Korean --------------

// sans serif
static const psfont SMGothic [] = {
    { "SMGothic-Medium-KSC-EUC-H", 0, 100. },
    { "SMGothic-Medium-KSC-EUC-H", 0.2, 100. },
    { "SMGothic-DemiBold-KSC-EUC-H", 0, 100. },
    { "SMGothic-DemiBold-KSC-EUC-H", 0.2, 100. }
};

// serif
#if 0 // ### this is never used?
static const psfont SMMyungjo [] = {
    { "SMMyungjo-Light-KSC-EUC-H", 0, 100. },
    { "SMMyungjo-Light-KSC-EUC-H", 0.2, 100. },
    { "SMMyungjo-Bold-KSC-EUC-H", 0, 100. },
    { "SMMyungjo-Bold-KSC-EUC-H", 0.2, 100. }
};
#endif

static const psfont MKai [] = {
    { "MingMT-Light-KSC-EUC-H", 0, 100. },
    { "MingMT-Light-KSC-EUC-H", 0.2, 100. },
    { "MKai-Medium-KSC-EUC-H", 0, 100. },
    { "MKai-Medium-KSC-EUC-H", 0.2, 100. },
};


static const psfont Munhwa [] = {
    { "Munhwa-Regular-KSC-EUC-H", 0, 100. },
    { "Munhwa-Regular-KSC-EUC-H", 0.2, 100. },
    { "Munhwa-Bold-KSC-EUC-H", 0, 100. },
    { "Munhwa-Bold-KSC-EUC-H", 0.2, 100. }
};

static const psfont MunhwaGothic [] = {
    { "MunhwaGothic-Regular-KSC-EUC-H", 0, 100. },
    { "MunhwaGothic-Regular-KSC-EUC-H", 0.2, 100. },
    { "MunhwaGothic-Bold-KSC-EUC-H", 0, 100. },
    { "MunhwaGothic-Bold-KSC-EUC-H", 0.2, 100. }
};

static const psfont MunhwaGungSeo [] = {
    { "MunhwaGungSeo-Light-KSC-EUC-H", 0, 100. },
    { "MunhwaGungSeo-Light-KSC-EUC-H", 0.2, 100. },
    { "MunhwaGungSeo-Bold-KSC-EUC-H", 0, 100. },
    { "MunhwaGungSeo-Bold-KSC-EUC-H", 0.2, 100. }
};

static const psfont MunhwaGungSeoHeulim [] = {
    { "MunhwaGungSeoHeulim-Light-KSC-EUC-H", 0, 100. },
    { "MunhwaGungSeoHeulim-Light-KSC-EUC-H", 0.2, 100. },
    { "MunhwaGungSeoHeulim-Bold-KSC-EUC-H", 0, 100. },
    { "MunhwaGungSeoHeulim-Bold-KSC-EUC-H", 0.2, 100. }
};

static const psfont MunhwaHoonMin [] = {
    { "MunhwaHoonMin-Regular-KSC-EUC-H", 0, 100. },
    { "MunhwaHoonMin-Regular-KSC-EUC-H", 0.2, 100. },
    { "MunhwaHoonMin-Regular-KSC-EUC-H", 0, 100. },
    { "MunhwaHoonMin-Regular-KSC-EUC-H", 0.2, 100. }
};

static const psfont BaekmukGulim [] = {
    { "Baekmuk-Gulim-KSC-EUC-H", 0, 100. },
    { "Baekmuk-Gulim-KSC-EUC-H", 0.2, 100. },
    { "Baekmuk-Gulim-KSC-EUC-H", 0, 100. },
    { "Baekmuk-Gulim-KSC-EUC-H", 0.2, 100. }
};

static const psfont * const KoreanReplacements[] = {
    BaekmukGulim, SMGothic, Munhwa, MunhwaGothic, MKai, MunhwaGungSeo,
    MunhwaGungSeoHeulim, MunhwaHoonMin, Helvetica, 0
};

class QPSPrinterFontKorean
  : public QPSPrinterFontAsian {
public:
      QPSPrinterFontKorean(const QFontEngine* f);
      QString extension() const;
};

QPSPrinterFontKorean::QPSPrinterFontKorean(const QFontEngine* f)
{
    codec = QTextCodec::codecForMib( 38 ); // eucKR
    int type = getPsFontType( f );
    psname = makePSFontName( f, type );
    QString best = "[ /" + psname + " 1.0 0.0 ]";
    replacementList.append( best );
    appendReplacements( replacementList, KoreanReplacements, type );
}

QString QPSPrinterFontKorean::extension() const
{
    return "-KSC-EUC-H";
}
// ----------- traditional chinese ------------

// Arphic Public License Big5 TrueType fonts (on Debian and CLE and others)
static const psfont ShanHeiSun [] = {
    { "ShanHeiSun-Light-ETen-B5-H", 0, 100. },
    { "ShanHeiSun-Light-ETen-B5-H", 0.2, 100. },
    { "ShanHeiSun-Light-ETen-B5-H", 0, 100. },
    { "ShanHeiSun-Light-ETen-B5-H", 0.2, 100. },
};
static const psfont ZenKai [] = {
    { "ZenKai-Medium-ETen-B5-H", 0, 100. },
    { "ZenKai-Medium-Italic-ETen-B5-H", 0.2, 100. },
    { "ZenKai-Medium-Bold-ETen-B5-H", 0, 100. },
    { "ZenKai-Medium-BoldItalic-ETen-B5-H", 0.2, 100. },
};

// Fonts on Turbolinux
static const psfont SongB5 [] = {
    { "B5-MSung-Light-ETen-B5-H", 0, 100. },
    { "B5-MSung-Italic-ETen-B5-H", 0, 100. },
    { "B5-MSung-Bold-ETen-B5-H", 0, 100. },
    { "B5-MSung-BoldItalic-ETen-B5-H", 0, 100. },
};
static const psfont KaiB5 [] = {
    { "B5-MKai-Medium-ETen-B5-H", 0, 100. },
    { "B5-MKai-Italic-ETen-B5-H", 0, 100. },
    { "B5-MKai-Bold-ETen-B5-H", 0, 100. },
    { "B5-MKai-BoldItalic-ETen-B5-H", 0, 100. },
};
static const psfont HeiB5 [] = {
    { "B5-MHei-Medium-ETen-B5-H", 0, 100. },
    { "B5-MHei-Italic-ETen-B5-H", 0, 100. },
    { "B5-MHei-Bold-ETen-B5-H", 0, 100. },
    { "B5-MHei-BoldItalic-ETen-B5-H", 0, 100. },
};
static const psfont FangSongB5 [] = {
    { "B5-CFangSong-Light-ETen-B5-H", 0, 100. },
    { "B5-CFangSong-Italic-ETen-B5-H", 0, 100. },
    { "B5-CFangSong-Bold-ETen-B5-H", 0, 100. },
    { "B5-CFangSong-BoldItalic-ETen-B5-H", 0, 100. },
};

// Arphic fonts on Thiz Linux
static const psfont LinGothic [] = {
    { "LinGothic-Light-ETen-B5-H", 0, 100. },
    { "LinGothic-Light-Italic-ETen-B5-H", 0.2, 100. },
    { "LinGothic-Light-Bold-ETen-B5-H", 0, 100. },
    { "LinGothic-Light-BoldItalic-ETen-B5-H", 0.2, 100. },
};
static const psfont YenRound [] = {
    { "YenRound-Light-ETen-B5-H", 0, 100. },
    { "YenRound-Light-Italic-ETen-B5-H", 0.2, 100. },
    { "YenRound-Light-Bold-ETen-B5-H", 0, 100. },
    { "YenRound-Light-BoldItalic-ETen-B5-H", 0.2, 100. },
};

// Dr. Wang Hann-Tzong's GPL'ed Big5 TrueType fonts
#if 0 // ### this is never used?
static const psfont HtWFangSong [] = {
    { "HtW-FSong-Light-ETen-B5-H", 0, 100. },
    { "HtW-FSong-Light-Italic-ETen-B5-H", 0.2, 100. },
    { "HtW-FSong-Light-Bold-ETen-B5-H", 0, 100. },
    { "HtW-FSong-Light-BoldItalic-ETen-B5-H", 0.2, 100. },
};
#endif

static const psfont MingB5 [] = {
    { "Ming-Light-ETen-B5-H", 0, 100. },
    { "Ming-Light-Italic-ETen-B5-H", 0.2, 100. },
    { "Ming-Light-Bold-ETen-B5-H", 0, 100. },
    { "Ming-Light-BoldItalic-ETen-B5-H", 0.2, 100. },
};

// Microsoft's Ming/Sung font?
static const psfont MSung [] = {
    { "MSung-Light-ETenms-B5-H", 0, 100. },
    { "MSung-Light-ETenms-B5-H", 0.2, 100. },
    { "MSung-Light-ETenms-B5-H", 0, 100. },
    { "MSung-Light-ETenms-B5-H", 0.2, 100. },
};
// "Standard Sung/Ming" font by Taiwan Ministry of Education
static const psfont MOESung [] = {
    { "MOESung-Regular-B5-H", 0, 100. },
    { "MOESung-Regular-B5-H", 0.2, 100. },
    { "MOESung-Regular-B5-H", 0, 100. },
    { "MOESung-Regular-B5-H", 0.2, 100. },
};

static const psfont MOEKai [] = {
    { "MOEKai-Regular-B5-H", 0, 100. },
    { "MOEKai-Regular-B5-H", 0.2, 100. },
    { "MOEKai-Regular-B5-H", 0, 100. },
    { "MOEKai-Regular-B5-H", 0.2, 100. },
};

static const psfont * const TraditionalReplacements[] = {
    MOESung, SongB5, ShanHeiSun, MingB5, MSung, FangSongB5, KaiB5, ZenKai, HeiB5,
    LinGothic, YenRound, MOEKai, Helvetica, 0
	};

#if 0 // ### these are never used?
static const psfont * const SongB5Replacements[] = {
    SongB5, ShanHeiSun, MingB5, MSung, MOESung, Helvetica, 0
	};

static const psfont * const FangSongB5Replacements[] = {
    FangSongB5, HtWFangSong, Courier, 0
	};
static const psfont * const KaiB5Replacements[] = {
    KaiB5, ZenKai, Times, 0
	};
static const psfont * const HeiB5Replacements[] = {
    HeiB5, LinGothic, YenRound, LucidaSans, 0
	};
static const psfont * const YuanB5Replacements[] = {
    YenRound, LinGothic, HeiB5, LucidaSans, 0
	};
#endif


class QPSPrinterFontTraditionalChinese
  : public QPSPrinterFontAsian {
public:
      QPSPrinterFontTraditionalChinese(const QFontEngine* f);
      QString extension() const;
};

QPSPrinterFontTraditionalChinese::QPSPrinterFontTraditionalChinese(const QFontEngine* f)
{
    codec = QTextCodec::codecForMib( 2026 ); // Big5-0
    int type = getPsFontType( f );
    psname = makePSFontName( f, type );
    QString best = "[ /" + psname + " 1.0 0.0 ]";
    replacementList.append( best );
    appendReplacements( replacementList, TraditionalReplacements, type );
}

QString QPSPrinterFontTraditionalChinese::extension() const
{
    return "-ETen-B5-H";
}

// ----------- simplified chinese ------------

#if 0
// GB18030 fonts on XteamLinux (?)
static const psfont SimplifiedGBK2K [] = {
    { "MSung-Light-GBK2K-H", 0, 100. },
    { "MSung-Light-GBK2K-H", 0.2, 100. },
    { "MKai-Medium-GBK2K-H", 0, 100. },
    { "MKai-Medium-GBK2K-H", 0.2, 100. },
};
#endif

// GB18030 fonts on Turbolinux
static const psfont SongGBK2K [] = {
    { "MSung-Light-GBK2K-H", 0, 100. },
    { "MSung-Italic-GBK2K-H", 0, 100. },
    { "MSung-Bold-GBK2K-H", 0, 100. },
    { "MSung-BoldItalic-GBK2K-H", 0, 100. },
};
static const psfont KaiGBK2K [] = {
    { "MKai-Medium-GBK2K-H", 0, 100. },
    { "MKai-Italic-GBK2K-H", 0, 100. },
    { "MKai-Bold-GBK2K-H", 0, 100. },
    { "MKai-BoldItalic-GBK2K-H", 0, 100. },
};
static const psfont HeiGBK2K [] = {
    { "MHei-Medium-GBK2K-H", 0, 100. },
    { "MHei-Italic-GBK2K-H", 0, 100. },
    { "MHei-Bold-GBK2K-H", 0, 100. },
    { "MHei-BoldItalic-GBK2K-H", 0, 100. },
};
static const psfont FangSongGBK2K [] = {
    { "CFangSong-Light-GBK2K-H", 0, 100. },
    { "CFangSong-Italic-GBK2K-H", 0, 100. },
    { "CFangSong-Bold-GBK2K-H", 0, 100. },
    { "CFangSong-BoldItalic-GBK2K-H", 0, 100. },
};

static const psfont Simplified [] = {
    { "MSung-Light-GBK-EUC-H", 0, 100. },
    { "MSung-Light-GBK-EUC-H", 0.2, 100. },
    { "MKai-Medium-GBK-EUC-H", 0, 100. },
    { "MKai-Medium-GBK-EUC-H", 0.2, 100. },
};

static const psfont MSungGBK [] = {
    { "MSung-Light-GBK-EUC-H", 0, 100. },
    { "MSung-Light-GBK-EUC-H", 0.2, 100. },
    { "MSung-Light-GBK-EUC-H", 0, 100. },
    { "MSung-Light-GBK-EUC-H", 0.2, 100. },
};

static const psfont FangSong [] = {
    { "CFangSong-Light-GBK-EUC-H", 0, 100. },
    { "CFangSong-Light-GBK-EUC-H", 0.2, 100. },
    { "CFangSong-Light-GBK-EUC-H", 0, 100. },
    { "CFangSong-Light-GBK-EUC-H", 0.2, 100. },
};

// Arphic Public License GB2312 TrueType fonts (on Debian and CLE and others)
static const psfont BousungEG [] = {
    { "BousungEG-Light-GB-GB-EUC-H", 0, 100. },
    { "BousungEG-Light-GB-GB-EUC-H", 0.2, 100. },
    { "BousungEG-Light-GB-Bold-GB-EUC-H", 0, 100. },
    { "BousungEG-Light-GB-Bold-GB-EUC-H", 0.2, 100. },
};
static const psfont GBZenKai [] = {
    { "GBZenKai-Medium-GB-GB-EUC-H", 0, 100. },
    { "GBZenKai-Medium-GB-GB-EUC-H", 0.2, 100. },
    { "GBZenKai-Medium-GB-Bold-GB-EUC-H", 0, 100. },
    { "GBZenKai-Medium-GB-Bold-GB-EUC-H", 0.2, 100. },
};

static const psfont * const SimplifiedReplacements[] = {
    SongGBK2K, FangSongGBK2K, KaiGBK2K, HeiGBK2K,
    Simplified, MSungGBK, FangSong, BousungEG, GBZenKai, Helvetica, 0
	};
#if 0
static const psfont * const SongGBK2KReplacements[] = {
    SongGBK2K, MSungGBK, BousungEG, Helvetica, 0
	};
#endif
static const psfont * const FangSongGBK2KReplacements[] = {
    FangSongGBK2K, FangSong, Courier, 0
	};
static const psfont * const KaiGBK2KReplacements[] = {
    KaiGBK2K, GBZenKai, Times, 0
	};
static const psfont * const HeiGBK2KReplacements[] = {
    HeiGBK2K, LucidaSans, 0
	};

class QPSPrinterFontSimplifiedChinese
  : public QPSPrinterFontAsian {
public:
      QPSPrinterFontSimplifiedChinese(const QFontEngine* f);
      QString extension() const;
};

QPSPrinterFontSimplifiedChinese::QPSPrinterFontSimplifiedChinese(const QFontEngine* f)
{
    codec = QTextCodec::codecForMib( 114 ); // GB18030
    int type = getPsFontType( f );
    QString family = f->fontDef.family.lower();
    if( family.contains("kai",FALSE) ) {
	psname = KaiGBK2K[type].psname;
	appendReplacements( replacementList, KaiGBK2KReplacements, type );
    } else if( family.contains("fangsong",FALSE) ) {
	psname = FangSongGBK2K[type].psname;
	appendReplacements( replacementList, FangSongGBK2KReplacements, type );
    } else if( family.contains("hei",FALSE) ) {
	psname = HeiGBK2K[type].psname;
	appendReplacements( replacementList, HeiGBK2KReplacements, type );
    } else {
	psname = SongGBK2K[type].psname;
	appendReplacements( replacementList, SimplifiedReplacements, type );
    }
    //qDebug("simplified chinese: fontname is %s, psname=%s", f.family().latin1(), psname.latin1() );
}

QString QPSPrinterFontSimplifiedChinese::extension() const
{
    return "-GBK2K-H";
}

#endif


// ================== QPSPrinterFont ====================

class QPSPrinterFont {
public:
  QPSPrinterFont(const QFont& f, int script, QPSPrinterPrivate *priv);
  ~QPSPrinterFont();
  QString postScriptFontName()     { return p->postScriptFontName(); }
    QString defineFont( QTextStream &stream, const QString &ps, const QFont &f, const QString &key,
                             QPSPrinterPrivate *d )
    { return p->defineFont( stream, ps, f, key, d ); }
    void    download(QTextStream& s, bool global) { p->download(s, global); }
    QPSPrinterFontPrivate *handle() { return p; }
    QString xfontname;
private:
  QByteArray       data;
  QPSPrinterFontPrivate* p;
};

QPSPrinterFont::~QPSPrinterFont()
{
    // the dict in QFontPrivate does deletion for us.
    //  delete p;
}


QPSPrinterFont::QPSPrinterFont(const QFont &f, int script, QPSPrinterPrivate *priv)
    : p(0)
{
    QString fontfilename;
    QString fontname;

    enum { NONE, PFB, PFA, TTF } type = NONE;

    QFontEngine *engine = f.d->engineForScript( (QFont::Script) script );
    // ### implement similar code for QWS and WIN
    xfontname = makePSFontName( engine );

#if defined( Q_WS_X11 )
    bool xlfd = FALSE;
    //qDebug("engine = %p name=%s, script=%d", engine, engine ? engine->name() : "(null)", script);

#ifndef QT_NO_XFTFREETYPE
    if ( qt_has_xft && engine && engine->type() == QFontEngine::Xft ) {
        XftPattern *pattern = static_cast<QFontEngineXft *>( engine )->pattern();
        char *filename = 0;
        XftPatternGetString (pattern, XFT_FILE, 0, &filename);
        //qDebug("filename for font is '%s'", filename);
        if ( filename ) {
            fontfilename = QString::fromLocal8Bit( filename );
            xfontname = fontfilename;
        }
    } else
#endif
    {
        QString rawName;
        if ( engine && engine != (QFontEngine *)-1 )
            rawName = engine->name();
        int index = rawName.find('-');
        if (index == 0) {
            // this is an XLFD font name
            for (int i=0; i < 6; i++) {
                index = rawName.find('-',index+1);
            }
            xfontname = rawName.mid(0,index);
            if ( xfontname.endsWith( "*" ) )
                xfontname.truncate( xfontname.length() - 1 );
            xlfd = TRUE;
        }
    }
#endif // Q_WS_X11
#ifndef QT_NO_TEXTCODEC
    // map some scripts to something more useful
    if ( script == QFont::Han ) {
	QTextCodec *lc = QTextCodec::codecForLocale();
	switch( lc->mibEnum() ) {
        case 36: // KS C 5601
        case 38: // EUC KR
            script = QFont::Hangul;
            break;

        case 57: // gb2312.1980-0
        case 113: // GBK
        case -113: // gbk-0
        case 114: // GB18030
        case -114: // gb18030-0
        case 2025: // GB2312
        case 2026: // Big5
        case -2026: // Big5-HKSCS
        case 2101: // big5-0, big5.eten-0
        case -2101: // big5hkscs-0, hkscs-1
            break;

        case 16: // JIS7
        case 17: // SJIS
        case 18: // EUC JP
        case 63: // JIS X 0208
        default:
            script = QFont::Hiragana;
            break;
	}
    } else if ( script == QFont::Katakana )
	script = QFont::Hiragana;
    else if ( script == QFont::Bopomofo )
	script = QFont::Han;
#endif

    QString searchname = xfontname;
#if defined(Q_WS_X11)
    // we need an extension here due to the fact that we use different
    // fonts for different scripts
    if ( xlfd && script >= QFont::Han && script <= QFont::Bopomofo )
	xfontname += "/" + toString( script );
#endif

    //qDebug("looking for font %s in dict", xfontname.latin1() );
    p = priv->fonts.find(xfontname);
    if ( p )
	return;

#if defined(Q_WS_X11)
    if ( xlfd ) {

	for (QStringList::Iterator it=priv->fontpath.begin(); it!=priv->fontpath.end() && fontfilename.isEmpty(); ++it) {
	    if ((*it).left(1) != "/") continue; // not a path name, a font server
	    QString fontmapname;
	    int num = 0;
	    // search font.dir and font.scale for the right file
	    while ( num < 2 ) {
		if ( num == 0 )
		    fontmapname = (*it) + "/fonts.scale";
		else
		    fontmapname = (*it) + "/fonts.dir";
		//qWarning(fontmapname);
		QFile fontmap(fontmapname);
		if (fontmap.open(IO_ReadOnly)) {
		    while (!fontmap.atEnd()) {
			QString mapping;
			fontmap.readLine(mapping,512);
			// fold to lower (since X folds to lowercase)
			//qWarning(xfontname);
			//qWarning(mapping);
			if (mapping.lower().contains(searchname.lower())) {
			    int index = mapping.find(' ',0);
			    QString ffn = mapping.mid(0,index);
                            // remove the most common bitmap formats
			    if( !ffn.contains( ".pcf" ) && !ffn.contains( ".bdf" ) &&
				!ffn.contains( ".spd" ) && !ffn.contains( ".phont" ) ) {
				fontfilename = (*it) + QString("/") + ffn;
				if ( QFile::exists(fontfilename) ) {
				    //qDebug("found font file %s", fontfilename.latin1());
				    break;
				} else // unset fontfilename
				    fontfilename = QString();
			    }
			}
		    }
		    fontmap.close();
		}
		num++;
	    }
	}
    }
#endif

    //qDebug("font=%s, fontname=%s, file=%s, p=%p", f.family().latin1(), xfontname.latin1(), fontfilename.latin1(), p);

    // memory mapping would be better here
    if (fontfilename.length() > 0) { // maybe there is no file name
	QFile fontfile(fontfilename);
	if ( fontfile.exists() ) {
	    //printf("font name %s size = %d\n",fontfilename.latin1(),fontfile.size());
	    data = QByteArray( fontfile.size() );

	    fontfile.open(IO_Raw | IO_ReadOnly);
	    fontfile.readBlock(data.data(), fontfile.size());
	    fontfile.close();
	}
    }

    if (!data.isNull() && data.size() > 0) {
        unsigned char* d = (unsigned char *)data.data();
        if (d[0] == 0x80 && d[1] == 0x01 && d[6] == '%' && d[7] == '!')
            type = PFB;
        else if (d[0] == '%' && d[1] == '!' && d[2] == 'P' && d[3] == 'S')
            type = PFA;
        else if (d[0]==0x00 && d[1]==0x01 && d[2]==0x00 && d[3]==0x00)
            type = TTF;
        else
            type = NONE;
    } else
        type = NONE;

    //qDebug("font is of type %d", type );
    switch (type) {
    case TTF :
        p = new QPSPrinterFontTTF(engine, data);
        break;
    case PFB:
        p = new QPSPrinterFontPFB(engine, data);
        break;
    case PFA:
        p = new QPSPrinterFontPFA(engine, data);
        break;
    case NONE:
    default:

#ifndef QT_NO_TEXTCODEC

        if ( script == QFont::Hiragana )
            p = new QPSPrinterFontJapanese( engine );
        else if ( script == QFont::Hangul )
            p = new QPSPrinterFontKorean( engine );
        else if ( script == QFont::Han ) {
            QTextCodec *lc = QTextCodec::codecForLocale();
            switch( lc->mibEnum() ) {
            case 2025: // GB2312
            case 57: // gb2312.1980-0
            case 113: // GBK
            case -113: // gbk-0
            case 114: // GB18030
            case -114: // gb18030-0
                p = new QPSPrinterFontSimplifiedChinese( engine );
                break;
            case 2026: // Big5
            case -2026: // big5-0, big5.eten-0
            case 2101: // Big5-HKSCS
            case -2101: // big5hkscs-0, hkscs-1
                p = new QPSPrinterFontTraditionalChinese( engine );
                break;
            default:
                p = new QPSPrinterFontJapanese( engine );
            }
        } else
#endif
            //qDebug("didnt find font for %s", xfontname.latin1());
            p = new QPSPrinterFontNotFound( engine );
        break;
    }

    if (p->postScriptFontName() == "Symbol")
	p->setSymbol();

    // this is needed to make sure we don't get the same postscriptname twice
    QDictIterator<QPSPrinterFontPrivate> it( priv->fonts );
    for( it.toFirst(); it.current(); ++it ) {
	if ( *(*it) == *p ) {
//	    qWarning("Post script driver: font already in dict");
	    delete p;
	    p = *it;
	    return;
	}
    }

    //qDebug("inserting font %s in dict psname=%s", xfontname.latin1(), p->postScriptFontName().latin1() );
    priv->fonts.insert( xfontname, p );
}

// ================= END OF PS FONT METHODS ============


QPSPrinterPrivate::QPSPrinterPrivate( QPrinter *prt, int filedes )
    : buffer( 0 ), outDevice( 0 ), fd( filedes ), pageBuffer( 0 ), fonts(27, FALSE), fontBuffer(0), savedImage( 0 ),
      dirtypen( FALSE ), dirtybrush( FALSE ), dirtyBkColor( FALSE ), bkMode( Qt::TransparentMode ), dirtyBkMode( FALSE ),
#ifndef QT_NO_TEXTCODEC
      currentFontCodec( 0 ),
#endif
      fm( QFont() ), textY( 0 )
{
    printer = prt;
    headerFontNames.setAutoDelete( TRUE );
    pageFontNames.setAutoDelete( TRUE );
    fonts.setAutoDelete( TRUE );
    currentFontFile = 0;
    scale = 1.;
    scriptUsed = -1;

#ifdef Q_WS_X11
    // append qsettings fontpath
    QSettings settings;
    embedFonts = settings.readBoolEntry( "/qt/embedFonts", TRUE );

    int npaths;
    char** font_path;
    font_path = XGetFontPath( qt_xdisplay(), &npaths);
    bool xfsconfig_read = FALSE;
    for (int i=0; i<npaths; i++) {
        // If we're using xfs, append font paths from /etc/X11/fs/config
        // can't hurt, and chances are we'll get all fonts that way.
        if (((font_path[i])[0] != '/') && !xfsconfig_read) {
            // We're using xfs -> read its config
            bool finished = FALSE;
            QFile f("/etc/X11/fs/config");
            if ( !f.exists() )
                f.setName("/usr/X11R6/lib/X11/fs/config");
            if ( !f.exists() )
                f.setName("/usr/X11/lib/X11/fs/config");
            if ( f.exists() ) {
                f.open(IO_ReadOnly);
                while(f.status()==IO_Ok && !finished) {
                    QString fs;
                    f.readLine(fs, 1024);
                    fs=fs.stripWhiteSpace();
                    if (fs.left(9)=="catalogue" && fs.contains('=')) {
                        fs=fs.mid(fs.find('=')+1).stripWhiteSpace();
                        bool end = FALSE;
                        while( f.status()==IO_Ok && !end ) {
                            if ( fs[int(fs.length())-1] == ',' )
                                fs = fs.left(fs.length()-1);
                            else
                                end = TRUE;
                            if (fs[0] != '#' && !fs.contains(":unscaled"))
                                fontpath += fs;
                            f.readLine(fs, 1024);
                            fs=fs.stripWhiteSpace();
                        }
                        finished = TRUE;
                    }
                }
                f.close();
            }
            xfsconfig_read = TRUE;
        } else if(!strstr(font_path[i], ":unscaled")) {
            // Fonts paths marked :unscaled are always bitmapped fonts
            // -> we can as well ignore them now and save time
            fontpath += font_path[i];
        }
    }
    XFreeFontPath(font_path);

    // append qsettings fontpath
    QStringList fp = settings.readListEntry( "/qt/fontPath", ':' );
    if ( !fp.isEmpty() )
        fontpath += fp;
#else
    embedFonts = FALSE;
#endif
}

QPSPrinterPrivate::~QPSPrinterPrivate()
{
    delete pageBuffer;
}

void QPSPrinterPrivate::setFont( const QFont & fnt, int script )
{
    QFont f = fnt;
    if ( f.rawMode() ) {
        QFont fnt( QString::fromLatin1("Helvetica"), 12 );
        setFont( fnt, QFont::Unicode );
        return;
    }
    if ( f.pointSize() == 0 ) {
#if defined(CHECK_RANGE)
        qWarning( "QPrinter: Cannot set a font with zero point size." );
#endif
        f.setPointSize(QApplication::font().pointSize());
        if ( f.pointSize() == 0 )
            f.setPointSize( 11 );
    }

    QPSPrinterFont ff( f, script, this );
    QString ps = ff.postScriptFontName();

    QString s = ps;
    s.append( ' ' );
    s.prepend( ' ' );

    QString key = ff.xfontname;

    if ( f.pointSize() != -1 )
	key += " " + toString( f.pointSize() );
    else
	key += " px" + toString( f.pixelSize() );
    QString * tmp;
    if ( !buffer )
        tmp = pageFontNames.find( key );
    else
        tmp = headerFontNames.find( key );

    QString fontName;
    if ( tmp )
        fontName = *tmp;

    if ( fontName.isEmpty() ) {
        fontName = ff.defineFont( pageStream, ps, f, key, this );
    }
    pageStream << fontName << " F\n";

    ps.append( ' ' );
    ps.prepend( ' ' );
    if ( !fontsUsed.contains( ps ) )
        fontsUsed += ps;

#ifndef QT_NO_TEXTCODEC
    QTextCodec * codec = 0;
// ###
// #ifndef QT_NO_TEXTCODEC
//     i = 0;
//     do {
//      if ( unicodevalues[i].cs == f.charSet() )
//          codec = QTextCodec::codecForMib( unicodevalues[i++].mib );
//     } while( codec == 0 && unicodevalues[i++].cs != unicodevalues_LAST );
// #endif
    currentFontCodec = codec;
#endif
    currentFont = fontName;
    currentFontFile = ff.handle();
    scriptUsed = script;
}


static void ps_r7( QTextStream& stream, const char * s, int l )
{
    int i = 0;
    uchar line[79];
    int col = 0;

    while( i < l ) {
        line[col++] = s[i++];
        if ( col >= 76 ) {
            line[col++] = '\n';
            line[col++] = '\0';
            stream << (const char *)line;
            col = 0;
        }
    }
    if ( col > 0 ) {
        while( (col&3) != 0 )
            line[col++] = '%'; // use a comment as padding
        line[col++] = '\n';
        line[col++] = '\0';
        stream << (const char *)line;
    }
}


static const int quoteSize = 3; // 1-8 pixels
static const int maxQuoteLength = 4+16+32+64+128+256; // magic extended quote
static const int quoteReach = 10; // ... 1-1024 pixels back
static const int tableSize = 1024; // 2 ** quoteReach;
static const int numAttempts = 128;

static const int hashSize = 71;

static const int None = INT_MAX;

/* puts the lowest numBits of data into the out array starting at postion (byte/bit).
   Adjusts byte and bit to point ot the next position.

   Need to make sure the out array is long enough before calling the method.
*/
static void emitBits( char *out, int & byte, int & bit,
                      int numBits, uint data )
{
    int b = 0;
    uint d = data;
    while( b < numBits ) {
        if ( bit == 0 )
            out[byte] = 0;
        if ( d & 1 )
            out[byte] = (uchar)out[byte] | ( 1 << bit );
        d = d >> 1;
        b++;
        bit++;
        if ( bit > 6 ) {
            bit = 0;
            byte++;
        }
    }
}

//#define DEBUG_COMPRESS
#ifdef DEBUG_COMPRESS
#include <qdatetime.h>
#endif

static QByteArray compress( const QImage & image, bool gray ) {
#ifdef DEBUG_COMPRESS
    QTime t;
    t.start();
    int sizeUncompressed[11];
    for( int i = 0; i < 11; i++ )
	sizeUncompressed[i] = 0;
    int sizeCompressed[11];
    for( int i = 0; i < 11; i++ )
	sizeCompressed[i] = 0;
#endif

    int width = image.width();
    int height = image.height();
    int depth = image.depth();
    int size = width*height;

    int pastPixel[tableSize];
    int mostRecentPixel[hashSize];
    if ( depth == 1 )
	size = (width+7)/8*height;
    else if ( !gray )
	size = size*3;

    unsigned char *pixel = new unsigned char[size+1];
    int i = 0;
    if ( depth == 1 ) {
	QImage::Endian bitOrder = image.bitOrder();
	memset( pixel, 0xff, size );
        for( int y=0; y < height; y++ ) {
            uchar * s = image.scanLine( y );
	    for( int x=0; x < width; x++ ) {
		// need to copy bit for bit...
		bool b = ( bitOrder == QImage::LittleEndian ) ?
			 (*(s + (x >> 3)) >> (x & 7)) & 1 :
			  (*(s + (x >> 3)) << (x & 7)) & 0x80 ;
		if ( b )
		    pixel[i >> 3] ^= (0x80 >> ( i & 7 ));
		i++;
	    }
	    // we need to align to 8 bit here
	    i = (i+7) & 0xffffff8;
        }
    } else if ( depth == 8 ) {
        for( int y=0; y < height; y++ ) {
            uchar * s = image.scanLine( y );
            for( int x=0; x < width; x++ ) {
                QRgb rgb = image.color( s[x] );
		if ( gray ) {
		    pixel[i] = (unsigned char) qGray( rgb );
		    i++;
		} else {
		    pixel[i] = (unsigned char) qRed( rgb );
		    pixel[i+1] = (unsigned char) qGreen( rgb );
		    pixel[i+2] = (unsigned char) qBlue( rgb );
		    i += 3;
		}
            }
        }
    } else {
        bool alpha = image.hasAlphaBuffer();
        for( int y=0; y < height; y++ ) {
            QRgb * s = (QRgb*)(image.scanLine( y ));
            for( int x=0; x < width; x++ ) {
		QRgb rgb = (*s++);
		if ( alpha && qAlpha( rgb ) < 0x40 ) // 25% alpha, convert to white -
		    rgb = qRgb( 0xff, 0xff, 0xff );
		if ( gray ) {
		    pixel[i] = (unsigned char) qGray( rgb );
		    i++;
		} else {
		    pixel[i] = (unsigned char) qRed( rgb );
		    pixel[i+1] = (unsigned char) qGreen( rgb );
		    pixel[i+2] = (unsigned char) qBlue( rgb );
		    i += 3;
		}
	    }
	}
    }

    pixel[size] = 0;

    /* this compression function emits blocks of data, where each
       block is an unquoted series of pixels, or a quote from earlier
       pixels. if the six-letter string "banana" were a six-pixel
       image, it might be unquoted "ban" followed by a 3-pixel quote
       from -2.  note that the final "a" is then copied from the
       second "a", which is copied from the first "a" in the same copy
       operation.

       the scanning for quotable blocks uses a cobol-like loop and a
       hash table: we know how many pixels we need to quote, hash the
       first and last pixel we need, and then go backwards in time
       looking for some spot where those pixels of those two colours
       occur at the right distance from each other.

       when we find a spot, we'll try a string-compare of all the
       intervening pixels. we only do a maximum of 128 both-ends
       compares or 64 full-string compares. it's more important to be
       fast than get the ultimate in compression.

       The format of the compressed stream is as follows:
       // 2 bits step size for search and backreference ( 1 or 3 )
       1 bit compressed or uncompressed block follows

       uncompressed block:
       3 bits size of block in bytes
       size*8 bits data

       compressed block:
       3 bits compression header
           0-2 size of block is 1-3 bytes
           3-7 size of block is bigger, 4-8 additional bits specifying size follow
       0/4-8 additional size fields
       10 location of backreference
    */

    for( i=0; i < hashSize; i++ )
        mostRecentPixel[i] = None;
    int index = 0;
    int emittedUntil = 0;
    char *out = (char *)malloc( 256 * sizeof( char ) );
    int outLen = 256;
    int outOffset = 0;
    int outBit = 0;

    /* we process pixels serially, emitting as necessary/possible. */
    while( index <= size ) {
        int bestCandidate = None;
        int bestLength = 0;
        i = index % tableSize;
        int h = pixel[index] % hashSize;
        int start, end;
        start = end = pastPixel[i] = mostRecentPixel[h];
        mostRecentPixel[h] = index;
        /* if our first candidate quote is unusable, or we don't need
           to quote because we've already emitted something for this
           pixel, just skip. */
        if ( start < index - tableSize || index >= size ||
             emittedUntil > index)
            start = end = None;
        int attempts = 0;
        /* scan for suitable quote candidates: not too far back, and
           if we've found one that's as big as it can get, don't look
           for more */
        while( start != None && end != None &&
               bestLength < maxQuoteLength &&
               start >= index - tableSize &&
               end >= index - tableSize + bestLength ) {
            /* scan backwards, looking for something good enough to
               try a (slow) string comparison. we maintain indexes to
               the start and the end of the quote candidate here */
            while( start != None && end != None &&
                   ( pixel[start] != pixel[index] ||
                     pixel[end] != pixel[index+bestLength] ) ) {
                if ( attempts++ > numAttempts ) {
                    start = None;
                } else if ( pixel[end] % hashSize ==
                            pixel[index+bestLength] % hashSize ) {
                    /* we move the area along the end index' chain */
                    end = pastPixel[end%tableSize];
                    start = end - bestLength;
                } else if ( pixel[start] % hashSize ==
                            pixel[index] % hashSize ) {
                    /* ... or along the start index' chain */
                    start = pastPixel[start%tableSize];
                    end = start + bestLength;
                } else {
#if 0
                    /* this should never happen: both the start and
                       the end pointers ran off their tracks. */
                    qDebug( "oops! %06x %06x %06x %06x %5d %5d %5d %d",
                            pixel[start], pixel[end],
                            pixel[index], pixel[index+bestLength],
                            start, end, index, bestLength );
#endif
                    /* but if it should happen, no problem. we'll just
                       say we found nothing, and the compression will
                       be a bit worse. */
                    start = None;
                }
                /* if we've moved either index too far to use the
                   quote candidate, let's just give up here. there's
                   also a guard against "start" insanity. */
                if ( start < index - tableSize || start < 0 || start >= index )
                    start = None;
                if ( end < index - tableSize + bestLength || end < bestLength )
                    end = None;
            }
            /* ok, now start and end point to an area of suitable
               length whose first and last points match, or one/both
               is/are set to None. */
            if ( start != None && end != None ) {
                /* slow string compare... */
                int length = 0;
                while( length < maxQuoteLength &&
                       index+length < size &&
                       pixel[start+length] == pixel[index+length] )
                    length++;
                /* if we've found something that overlaps the index
                   point, maybe we can move the quote point back?  if
                   we're copying 10 pixels from 8 pixels back (an
                   overlap of 2), that'll be faster than copying from
                   4 pixels back (an overlap of 6). */
                if ( start + length > index && length > 0 ) {
                    int d = index-start;
                    int equal = TRUE;
                    while( equal && start + length > index &&
                           start > d && start-d >= index-tableSize ) {
                        int i = 0;
                        while( equal && i < d ) {
                            if( pixel[start+i] != pixel[start+i-d] )
                                equal = FALSE;
                            i++;
                        }
                        if ( equal )
                            start -= d;
                    }
                }
                /* if what we have is longer than the best previous
                   candidate, we'll use this one. */
                if ( length > bestLength ) {
                    attempts = 0;
                    bestCandidate = start;
                    bestLength = length;
                    if ( length < maxQuoteLength && index + length < size )
                        end = mostRecentPixel[pixel[index+length]%hashSize];
                } else {
                    /* and if it ins't, we'll try some more. but we'll
                       count each string compare extra, since they're
                       so expensive. */
                    attempts += 2;
                    if ( attempts > numAttempts ) {
                        start = None;
                    } else if ( pastPixel[start%tableSize] + bestLength <
                                pastPixel[end%tableSize] ) {
                        start = pastPixel[start%tableSize];
                        end = start + bestLength;
                    } else {
                        end = pastPixel[end%tableSize];
                        start = end - bestLength;
                    }
                }
                /* again, if we can't make use of the current quote
                   candidate, we don't try any more */
                if ( start < index - tableSize || start < 0 || start > size+1 )
                    start = None;
                if ( end < index - tableSize + bestLength || end < 0 || end > size+1 )
                    end = None;
            }
        }
	/* backreferences to 1 byte of data are actually more costly than
         emitting the data directly, 2 bytes don't save much. */
	if ( bestCandidate != None && bestLength < 3 )
	    bestCandidate = None;
        /* at this point, bestCandidate is a candidate of bestLength
         length, or else it's None. if we have such a candidate, or
         we're at the end, we have to emit all unquoted data. */
        if ( index == size || bestCandidate != None ) {
            /* we need a double loop, because there's a maximum length
               on the "unquoted data" section. */
            while( emittedUntil < index ) {
#ifdef DEBUG_COMPRESS
		int x = 0;
		int bl = emittedUntil - index;
		while ( (bl /= 2) )
		    x++;
		if ( x > 10 ) x = 10;
		sizeUncompressed[x]++;
#endif
                int l = QMIN( 8, index - emittedUntil );
		if ( outOffset + l + 2 >= outLen ) {
		    outLen *= 2;
		    out = (char *) realloc( out, outLen );
		}
                emitBits( out, outOffset, outBit,
                          1, 0 );
                emitBits( out, outOffset, outBit,
                          quoteSize, l-1 );
                while( l-- ) {
		    emitBits( out, outOffset, outBit,
			      8, pixel[emittedUntil] );
                    emittedUntil++;
                }
            }
        }
        /* if we have some quoted data to output, do it. */
        if ( bestCandidate != None ) {
#ifdef DEBUG_COMPRESS
	    int x = 0;
	    int bl = bestLength;
	    while ( (bl /= 2) )
		x++;
	    if ( x > 10 ) x = 10;
	    sizeCompressed[x]++;
#endif
	    if ( outOffset + 4 >= outLen ) {
		outLen *= 2;
		out = (char *) realloc( out, outLen );
	    }
            emitBits( out, outOffset, outBit,
                      1, 1 );
	    int l = bestLength - 3;
	    const struct off_len {
		int off;
		int bits;
	    } ol_table [] = {
		/* Warning: if you change the table here, change /uc in the PS code! */
		{ 3, 0/*dummy*/ },
		{ 16, 4 },
		{ 32, 5 },
		{ 64, 6 },
		{ 128, 7 },
		{ /*256*/ 0xfffffff, 8 },
	    };

            if ( l < ol_table[0].off ) {
                emitBits( out, outOffset, outBit,
                          quoteSize, l );
	    } else {
		const off_len *ol = ol_table;
		l -= ol->off;
		ol++;
		while ( l >= ol->off ) {
		    l -= ol->off;
		    ol++;
		}
                emitBits( out, outOffset, outBit,
                          quoteSize, ol->bits-1 );
                emitBits( out, outOffset, outBit,
                          ol->bits, l );
	    }
	    emitBits( out, outOffset, outBit,
		      quoteReach, index - bestCandidate - 1 );
	    emittedUntil += bestLength;
	}
	index++;
    }
    /* we've output all the data; time to clean up and finish off the
       last characters. */
    if ( outBit )
        outOffset++;
    i = 0;
    /* we have to make sure the data is encoded in a stylish way :) */
    while( i < outOffset ) {
        uchar c = out[i];
        c += 42;
        if ( c > 'Z' && ( c != 't' || i == 0 || out[i-1] != 'Q' ) )
            c += 84;
        out[i] = c;
        i++;
    }
    QByteArray outarr;
    outarr.duplicate( out, outOffset );
    free( out );
    delete [] pixel;

#ifdef DEBUG_COMPRESS
    qDebug( "------------- image compression statistics ----------------" );
    qDebug(" compression time %d", t.elapsed() );
    qDebug( "Size dist of uncompressed blocks:" );
    qDebug( "\t%d\t%d\t%d\t%d\t%d\t%d\n", sizeUncompressed[0], sizeUncompressed[1],
	    sizeUncompressed[2], sizeUncompressed[3], sizeUncompressed[4], sizeUncompressed[5]);
    qDebug( "\t%d\t%d\t%d\t%d\t%d\n", sizeUncompressed[6], sizeUncompressed[7],
	    sizeUncompressed[8], sizeUncompressed[9], sizeUncompressed[10] );
    qDebug( "Size dist of compressed blocks:" );
    qDebug( "\t%d\t%d\t%d\t%d\t%d\t%d\n", sizeCompressed[0], sizeCompressed[1],
	    sizeCompressed[2], sizeCompressed[3], sizeCompressed[4], sizeCompressed[5]);
    qDebug( "\t%d\t%d\t%d\t%d\t%d\n", sizeCompressed[6], sizeCompressed[7],
	    sizeCompressed[8], sizeCompressed[9], sizeCompressed[10] );
    qDebug( "===> total compression ratio %d/%d = %f", outOffset, size, (float)outOffset/(float)size );
    qDebug( "-----------------------------------------------------------" );
#endif

    return outarr;
}

#undef XCOORD
#undef YCOORD
#undef WIDTH
#undef HEIGHT
#undef POINT
#undef RECT
#undef INT_ARG

#define XCOORD(x)       (float)(x)
#define YCOORD(y)       (float)(y)
#define WIDTH(w)        (float)(w)
#define HEIGHT(h)       (float)(h)

#define POINT(index)    XCOORD(p[index].point->x()) << ' ' <<           \
                        YCOORD(p[index].point->y()) << ' '
#define RECT(index)     XCOORD(p[index].rect->normalize().x())  << ' ' <<     \
                        YCOORD(p[index].rect->normalize().y())  << ' ' <<     \
                        WIDTH (p[index].rect->normalize().width()) << ' ' <<  \
                        HEIGHT(p[index].rect->normalize().height()) << ' '
#define INT_ARG(index)  p[index].ival << ' '

static char returnbuffer[13];
static const char * color( const QColor &c, QPrinter * printer )
{
    if ( c == Qt::black )
        qstrcpy( returnbuffer, "B " );
    else if ( c == Qt::white )
        qstrcpy( returnbuffer, "W " );
    else if ( c.red() == c.green() && c.red() == c.blue() )
        sprintf( returnbuffer, "%d d2 ", c.red() );
    else if ( printer->colorMode() == QPrinter::GrayScale )
        sprintf( returnbuffer, "%d d2 ",
                 qGray( c.red(), c.green(),c.blue() ) );
    else
        sprintf( returnbuffer, "%d %d %d ",
                 c.red(), c.green(), c.blue() );
    return returnbuffer;
}


static const char * psCap( Qt::PenCapStyle p )
{
    if ( p == Qt::SquareCap )
        return "2 ";
    else if ( p == Qt::RoundCap )
        return "1 ";
    return "0 ";
}


static const char * psJoin( Qt::PenJoinStyle p ) {
    if ( p == Qt::BevelJoin )
        return "2 ";
    else if ( p == Qt::RoundJoin )
        return "1 ";
    return "0 ";
}



void QPSPrinterPrivate::drawImage( QPainter *paint, float x, float y, float w, float h,
				   const QImage &img, const QImage &mask )
{
    if ( !w || !h || img.isNull() ) return;

    int width  = img.width();
    int height = img.height();
    float scaleX = (float)width/w;
    float scaleY = (float)height/h;

    bool gray = (printer->colorMode() == QPrinter::GrayScale) ||
		img.allGray();
    int splitSize = 21830 * (gray ? 3 : 1 );
    if ( width * height > splitSize ) { // 65535/3, tolerance for broken printers
        int images, subheight;
        images = ( width * height + splitSize - 1 ) / splitSize;
        subheight = ( height + images-1 ) / images;
        while ( subheight * width > splitSize ) {
            images++;
            subheight = ( height + images-1 ) / images;
        }
        int suby = 0;
        while( suby < height ) {
            drawImage(paint, x, y + suby/scaleY, w, QMIN( subheight, height-suby )/scaleY,
		      img.copy( 0, suby, width, QMIN( subheight, height-suby ) ),
		      mask.isNull() ? mask : mask.copy( 0, suby, width, QMIN( subheight, height-suby ) ));
            suby += subheight;
        }
    } else {
	QByteArray out;
	int size = 0;
	const char *bits;

	if ( !mask.isNull() ) {
	    out = ::compress( mask, TRUE );
	    size = (width+7)/8*height;
	    pageStream << "/mask " << size << " string uc\n";
	    ps_r7( pageStream, out, out.size() );
	    pageStream << "d\n";
	}
	if ( img.depth() == 1 ) {
	    size = (width+7)/8*height;
	    bits = "1 ";
	} else if ( gray ) {
	    size = width*height;
	    bits = "8 ";
        } else {
	    size = width*height*3;
	    bits = "24 ";
        }

	out = ::compress( img, gray );
	pageStream << "/sl " << size << " string uc\n";
	ps_r7( pageStream, out, out.size() );
	pageStream << "d\n"
		   << width << ' ' << height << "[" << scaleX << " 0 0 " << scaleY << " 0 0]sl "
		   << bits << (!mask.isNull() ? "mask " : "false ")
		   << x << ' ' << y << " di\n";
    }
}


void QPSPrinterPrivate::matrixSetup( QPainter *paint )
{
#ifndef QT_NO_TRANSFORMATIONS
    QWMatrix tmp;
    if ( paint->hasViewXForm() ) {
        QRect viewport = paint->viewport();
        QRect window   = paint->window();
        tmp.translate( viewport.x(), viewport.y() );
        tmp.scale( 1.0 * viewport.width()  / window.width(),
                   1.0 * viewport.height() / window.height() );
        tmp.translate( -window.x(), -window.y() );
    }
    if ( paint->hasWorldXForm() ) {
        tmp = paint->worldMatrix() * tmp;
    }
    pageStream << "["
           << tmp.m11() << ' ' << tmp.m12() << ' '
           << tmp.m21() << ' ' << tmp.m22() << ' '
           << tmp.dx()  << ' ' << tmp.dy()
           << "]ST\n";
#else
    QPoint p(0,0);
    p = paint->xForm(p);
    pageStream << "["
           << 0 << ' ' << 0 << ' '
           << 0 << ' ' << 0 << ' '
           << p.x()    << ' ' << p.y()
           << "]ST\n";
#endif
    dirtyMatrix = FALSE;
}

void QPSPrinterPrivate::orientationSetup()
{
    if ( printer->orientation() == QPrinter::Landscape )
        pageStream << "QLS\n";
}


void QPSPrinterPrivate::emitHeader( bool finished )
{
    QString title = printer->docName();
    QString creator = printer->creator();
    if ( !creator )                             // default creator
        creator = QString::fromLatin1("Qt " QT_VERSION_STR);
    outDevice = new QFile();
    (void)((QFile *)outDevice)->open( IO_WriteOnly, fd );
    outStream.setDevice( outDevice );
    outStream << "%!PS-Adobe-1.0";
    QPaintDeviceMetrics m( printer );
    scale = 72. / ((float) m.logicalDpiY());
    uint mtop, mleft, mbottom, mright;
    printer->margins( &mtop, &mleft, &mbottom, &mright );
    int width = m.width();
    int height = m.height();
    bool fullPage = printer->fullPage();
    if ( finished && pageCount == 1 && printer->numCopies() == 1 &&
         ( ( printer->fullPage() && qt_gen_epsf ) ||
	   ( printer->outputToFile() && printer->outputFileName().endsWith( ".eps" ) ) )
	) {
        if ( !boundingBox.isValid() )
            boundingBox.setRect( 0, 0, width, height );
	if ( printer->orientation() == QPrinter::Landscape ) {
	    if ( !fullPage )
		boundingBox.moveBy( -mleft, -mtop );
	    outStream << " EPSF-3.0\n%%BoundingBox: "
		      << (int)(m.height() - boundingBox.bottom())*scale << " " // llx
		      << (int)(m.width() - boundingBox.right())*scale - 1 << " " // lly
		      << (int)(m.height() - boundingBox.top())*scale + 1 << " " // urx
		      << (int)(m.width() - boundingBox.left())*scale; // ury
	} else {
	    if ( !fullPage )
		boundingBox.moveBy( mleft, -mtop );
	    outStream << " EPSF-3.0\n%%BoundingBox: "
		      << (int)(boundingBox.left())*scale << " "
		      << (int)(m.height() - boundingBox.bottom())*scale - 1 << " "
		      << (int)(boundingBox.right())*scale + 1 << " "
		      << (int)(m.height() - boundingBox.top())*scale;
	}
    } else {
	int w = width + (fullPage ? 0 : mleft + mright);
        int h = height + (fullPage ? 0 : mtop + mbottom);
	w = (int)(w*scale);
	h = (int)(h*scale);
        // set a bounding box according to the DSC
        if ( printer->orientation() == QPrinter::Landscape )
            outStream << "\n%%BoundingBox: 0 0 " << h << " " << w;
        else
            outStream << "\n%%BoundingBox: 0 0 " << w << " " << h;
    }
    outStream << "\n" << wrapDSC( "%%Creator: " + creator );
    if ( !!title )
        outStream << wrapDSC( "%%Title: " + title );
    outStream << "%%CreationDate: " << QDateTime::currentDateTime().toString();
    outStream << "\n%%Orientation: ";
    if ( printer->orientation() == QPrinter::Landscape )
        outStream << "Landscape";
    else
        outStream << "Portrait";
    if ( finished )
        outStream << "\n%%Pages: " << pageCount << "\n"
		  << wrapDSC( "%%DocumentFonts: " + fontsUsed );
    else
        outStream << "%%Pages: (atend)"
               << "\n%%DocumentFonts: (atend)";
    outStream << "\n%%EndComments\n";

    outStream << "%%BeginProlog\n";
    const char * const prologLicense = "% Prolog copyright 1994-2006 Trolltech. "
                                 "You may copy this prolog in any way\n"
                                 "% that is directly related to this "
                                 "document. For other use of this prolog,\n"
                                 "% see your licensing agreement for Qt.\n";
    outStream << prologLicense << ps_header << "\n";

    // we have to do this here, as scaling can affect this.
    QString lineStyles = "/LArr["                                       // Pen styles:
			 " [] []"                       //   solid line
			 " [ w s ] [ s w ]"                 //   dash line
			 " [ s s ] [ s s ]"                  //   dot line
			 " [ m s s s ] [ s m s s ]"      //   dash dot line
			 " [ m s s s s ] [ s m s s s s ]"         //   dash dot dot line
			 " ] d\n";
    lineStyles.replace( QRegExp( "w" ), toString( 10./scale ) );
    lineStyles.replace( QRegExp( "m" ), toString( 5./scale ) );
    lineStyles.replace( QRegExp( "s" ), toString( 3./scale ) );

    outStream << lineStyles;

    outStream << "/pageinit {\n";
    if ( !printer->fullPage() ) {
        if ( printer->orientation() == QPrinter::Portrait )
            outStream << mleft*scale << " "
                   << mbottom*scale << " translate\n";
        else
            outStream << mtop*scale << " "
                   << mleft*scale << " translate\n";
    }
    if ( printer->orientation() == QPrinter::Portrait ) {
        outStream << "% " << m.widthMM() << "*" << m.heightMM()
               << "mm (portrait)\n0 " << height*scale
               << " translate " << scale << " -" << scale << " scale/defM matrix CM d } d\n";
    } else {
        outStream << "% " << m.heightMM() << "*" << m.widthMM()
               << " mm (landscape)\n 90 rotate " << scale << " -" << scale << " scale/defM matrix CM d } d\n";
    }
    outStream << "%%EndProlog\n";


    outStream << "%%BeginSetup\n";
    if ( printer->numCopies() > 1 ) {
	outStream << "/#copies " << printer->numCopies() << " def\n";
	outStream << "/NumCopies " << printer->numCopies() << " SPD\n";
	outStream << "/Collate " << (printer->collateCopies() ? "true" : "false") << " SPD\n";
    }
    if ( fontBuffer->buffer().size() ) {
        if ( pageCount == 1 || finished )
            outStream << "% Fonts and encodings used\n";
        else
            outStream << "% Fonts and encodings used on pages 1-"
                   << pageCount << "\n";
        QDictIterator<QPSPrinterFontPrivate> it(fonts);
        while (it.current()) {
          it.current()->download(outStream,TRUE); // true means its global
          ++it;
                }
        outStream.writeRawBytes( fontBuffer->buffer().data(),
                              fontBuffer->buffer().size() );
    }
    outStream << "%%EndSetup\n";

    outStream.writeRawBytes( buffer->buffer().data(),
                          buffer->buffer().size() );

    delete buffer;
    buffer = 0;
    fontStream.unsetDevice();
    delete fontBuffer;
    fontBuffer = 0;
}


/* Called whenever a restore has been done. Currently done at the top of a
  new page and whenever clipping is turned off. */
void QPSPrinterPrivate::resetDrawingTools( QPainter *paint )
{
    QPen   defaultPen;                  // default drawing tools
    QBrush defaultBrush;

    QColor c = paint->backgroundColor();
    if ( c != Qt::white )
        pageStream << color( c, printer ) << "BC\n";

    if ( paint->backgroundMode() != Qt::TransparentMode )
            pageStream << "/OMo true d\n";

    //currentUsed = currentSet;
    //setFont( currentSet );
    currentFontFile = 0;

    QBrush b = paint->brush();
    if ( b != defaultBrush ) {
        if ( b == Qt::CustomPattern ) {
#if defined(CHECK_RANGE)
            qWarning( "QPrinter: Pixmap brush not supported" );
#endif
        } else {
            cbrush = b;
        }
    }

    dirtypen = TRUE;
    dirtybrush = TRUE;

    if ( paint->hasViewXForm() || paint->hasWorldXForm() )
        matrixSetup( paint );
}


static void putRect( QTextStream &stream, const QRect &r )
{
    stream << r.x() << " "
           << r.y() << " "
           << r.width() << " "
           << r.height() << " ";
}


void QPSPrinterPrivate::setClippingOff( QPainter *paint )
{
        pageStream << "CLO\n";              // clipping off, includes a restore
        resetDrawingTools( paint );     // so drawing tools must be reset
}


void QPSPrinterPrivate::clippingSetup( QPainter *paint )
{
    if ( paint->hasClipping() ) {
        if ( !firstClipOnPage )
            setClippingOff( paint );
        const QRegion rgn = paint->clipRegion();
        QMemArray<QRect> rects = rgn.rects();
        int i;
        pageStream<< "CLSTART\n";           // start clipping
        for( i = 0 ; i < (int)rects.size() ; i++ ) {
            putRect( pageStream, rects[i] );
            pageStream << "ACR\n";          // add clip rect
            if ( pageCount == 1 )
                boundingBox = boundingBox.unite( rects[i] );
        }
        pageStream << "CLEND\n";            // end clipping
        firstClipOnPage = FALSE;
    } else {
        if ( !firstClipOnPage )      // no need to turn off if first on page
            setClippingOff( paint );
        // if we're painting without clipping, the bounding box must
        // be everything.  NOTE: this assumes that this function is
        // only ever called when something is to be painted.
        QPaintDeviceMetrics m( printer );
        if ( !boundingBox.isValid() )
            boundingBox.setRect( 0, 0, m.width(), m.height() );
    }
    dirtyClipping = FALSE;
}

void QPSPrinterPrivate::initPage(QPainter *paint)
{

    // a restore undefines all the fonts that have been defined
    // inside the scope (normally within pages) and all the glyphs that
    // have been added in the scope.

    QDictIterator<QPSPrinterFontPrivate> it(fonts);
    while (it.current()) {
      it.current()->restore();
      ++it;
    }
    if ( !buffer ) {
        pageFontNames.clear();
    }

    pageStream.unsetDevice();
    if ( pageBuffer )
        delete pageBuffer;
    pageBuffer = new QBuffer();
    pageBuffer->open( IO_WriteOnly );
    pageStream.setEncoding( QTextStream::Latin1 );
    pageStream.setDevice( pageBuffer );
    delete savedImage;
    savedImage = 0;
    textY = 0;
    dirtyClipping   = TRUE;
    firstClipOnPage = TRUE;


    resetDrawingTools( paint );
    dirtyNewPage      = FALSE;
    pageFontNumber = headerFontNumber;
}

void QPSPrinterPrivate::flushPage( bool last )
{
    if ( last && !pageBuffer )
	return;
    bool pageFonts = ( buffer == 0 );
    if ( buffer &&
//         ( last || pagesInBuffer++ > -1 ||
//           ( pagesInBuffer > 4 && buffer->size() > 262144 ) ) )
#ifdef Q_WS_QWS
         (last || buffer->size() > 2000000) // embedded is usually limited in memory
#else
         (last || buffer->size() > 50000000)
#endif
        ) {
//        qDebug("emiting header at page %d", pageCount );
        emitHeader( last );
    }
    outStream << "%%Page: "
              << pageCount << ' ' << pageCount << endl
              << "%%BeginPageSetup\n"
              << "QI\n";
    if (!dirtyNewPage) {
        if ( pageFonts ) {
            //qDebug("page fonts for page %d", pageCount);
            // we have already downloaded the header. Maybe we have page fonts here
            QDictIterator<QPSPrinterFontPrivate> it(fonts);
            while (it.current()) {
                it.current()->download( outStream, FALSE ); // FALSE means its for the page only
                ++it;
            }
        }
        outStream  << "%%EndPageSetup\n";
        if ( pageBuffer ) 
            outStream.writeRawBytes( pageBuffer->buffer().data(),
                                     pageBuffer->buffer().size() );
    }
    outStream << "\nQP\n";
    pageCount++;
}

// ================ PSPrinter class ========================

QPSPrinter::QPSPrinter( QPrinter *prt, int fd )
    : QPaintDevice( QInternal::Printer | QInternal::ExternalDevice )
{
    d = new QPSPrinterPrivate( prt, fd );
}


QPSPrinter::~QPSPrinter()
{
    if ( d->fd >= 0 )
#if defined(_OS_WIN32_)
        ::_close( d->fd );
#else
        ::close( d->fd );
#endif
    delete d;
}



static void ignoreSigPipe(bool b)
{
    static struct sigaction *users_sigpipe_handler = 0;

    if (b) {
	if (users_sigpipe_handler != 0)
    	    return; // already ignoring sigpipe

	users_sigpipe_handler = new struct sigaction;
	struct sigaction tmp_sigpipe_handler;
	tmp_sigpipe_handler.sa_handler = SIG_IGN;
	sigemptyset(&tmp_sigpipe_handler.sa_mask);
	tmp_sigpipe_handler.sa_flags = 0;

	if (sigaction(SIGPIPE, &tmp_sigpipe_handler, users_sigpipe_handler) == -1) {
    	    delete users_sigpipe_handler;
	    users_sigpipe_handler = 0;
	}
    }
    else {
	if (users_sigpipe_handler == 0)
    	    return; // not ignoring sigpipe

	if (sigaction(SIGPIPE, users_sigpipe_handler, 0) == -1)
	    qWarning("QPSPrinter: could not restore SIGPIPE handler");

	delete users_sigpipe_handler;
	users_sigpipe_handler = 0;
    }
}

bool QPSPrinter::cmd( int c , QPainter *paint, QPDevCmdParam *p )
{
    if ( c == PdcBegin ) {              // start painting
        d->pagesInBuffer = 0;
        d->buffer = new QBuffer();
        d->buffer->open( IO_WriteOnly );
        d->outStream.setEncoding( QTextStream::Latin1 );
        d->outStream.setDevice( d->buffer );
        d->fontBuffer = new QBuffer();
        d->fontBuffer->open( IO_WriteOnly );
        d->fontStream.setEncoding( QTextStream::Latin1 );
        d->fontStream.setDevice( d->fontBuffer );
        d->headerFontNumber = 0;
        d->pageCount           = 1;                // initialize state
        d->dirtyMatrix         = TRUE;
        d->dirtyClipping    = TRUE;
        d->dirtyNewPage        = TRUE;
        d->firstClipOnPage  = TRUE;
        d->boundingBox = QRect( 0, 0, -1, -1 );
        d->fontsUsed = QString::fromLatin1("");

        QPaintDeviceMetrics m( d->printer );
        d->scale = 72. / ((float) m.logicalDpiY());

        return TRUE;
    }

    if ( c == PdcEnd ) {                        // painting done
        bool pageCountAtEnd = (d->buffer != 0);

    	// we're writing to lp/lpr through a pipe, we don't want to crash with SIGPIPE
	// if lp/lpr dies
	ignoreSigPipe(TRUE);
        d->flushPage( TRUE );
        d->outStream << "%%Trailer\n";
        if ( pageCountAtEnd )
            d->outStream << "%%Pages: " << d->pageCount - 1 << "\n" <<
		wrapDSC( "%%DocumentFonts: " + d->fontsUsed );
        d->outStream << "%%EOF\n";
	ignoreSigPipe(FALSE);

        d->outStream.unsetDevice();
	if ( d->outDevice )
	    d->outDevice->close();
        if ( d->fd >= 0 )
            ::close( d->fd );
        d->fd = -1;
        delete d->outDevice;
        d->outDevice = 0;
    }

    if ( c >= PdcDrawFirst && c <= PdcDrawLast ) {
        if ( !paint )
            return FALSE; // sanity
        if ( d->dirtyNewPage )
            d->initPage( paint );
        if ( d->dirtyMatrix )
            d->matrixSetup( paint );
        if ( d->dirtyClipping ) // Must be after matrixSetup and initPage
            d->clippingSetup( paint );
        if ( d->dirtypen ) {
            // we special-case for narrow solid lines with the default
            // cap and join styles
            if ( d->cpen.style() == Qt::SolidLine && d->cpen.width() == 0 &&
                 d->cpen.capStyle() == Qt::FlatCap &&
                 d->cpen.joinStyle() == Qt::MiterJoin )
                d->pageStream << color( d->cpen.color(), d->printer ) << "P1\n";
            else
                d->pageStream << (int)d->cpen.style() << ' ' << d->cpen.width()
                       << ' ' << color( d->cpen.color(), d->printer )
                       << psCap( d->cpen.capStyle() )
                       << psJoin( d->cpen.joinStyle() ) << "PE\n";
            d->dirtypen = FALSE;
        }
        if ( d->dirtybrush ) {
            // we special-case for nobrush and solid white, since
            // those are the two most common brushes
            if ( d->cbrush.style() == Qt::NoBrush )
                d->pageStream << "NB\n";
            else if ( d->cbrush.style() == Qt::SolidPattern &&
                      d->cbrush.color() == Qt::white )
                d->pageStream << "WB\n";
            else
                d->pageStream << (int)d->cbrush.style() << ' '
                       << color( d->cbrush.color(), d->printer ) << "BR\n";
            d->dirtybrush = FALSE;
        }
	if ( d->dirtyBkColor ) {
	    d->pageStream << color( d->bkColor, d->printer ) << "BC\n";
	    d->dirtyBkColor = FALSE;
	}
	if ( d->dirtyBkMode ) {
	    if ( d->bkMode == Qt::TransparentMode )
		d->pageStream << "/OMo false d\n";
	    else
		d->pageStream << "/OMo true d\n";
	    d->dirtyBkMode = FALSE;
	}
    }

    switch( c ) {
    case PdcDrawPoint:
        d->pageStream << POINT(0) << "P\n";
        break;
    case PdcMoveTo:
        d->pageStream << POINT(0) << "M\n";
        break;
    case PdcLineTo:
        d->pageStream << POINT(0) << "L\n";
        break;
    case PdcDrawLine:
        if ( p[0].point->y() == p[1].point->y() )
            d->pageStream << POINT(1) << p[0].point->x() << " HL\n";
        else if ( p[0].point->x() == p[1].point->x() )
            d->pageStream << POINT(1) << p[0].point->y() << " VL\n";
        else
            d->pageStream << POINT(1) << POINT(0) << "DL\n";
        break;
    case PdcDrawRect:
        d->pageStream << RECT(0) << "R\n";
        break;
    case PdcDrawRoundRect:
        d->pageStream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "RR\n";
        break;
    case PdcDrawEllipse:
        d->pageStream << RECT(0) << "E\n";
        break;
    case PdcDrawArc:
        d->pageStream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "A\n";
        break;
    case PdcDrawPie:
        d->pageStream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "PIE\n";
        break;
    case PdcDrawChord:
        d->pageStream << RECT(0) << INT_ARG(1) << INT_ARG(2) << "CH\n";
        break;
    case PdcDrawLineSegments:
        if ( p[0].ptarr->size() > 0 ) {
            QPointArray a = *p[0].ptarr;
            QPoint pt;
            d->pageStream << "NP\n";
            for ( int i=0; i<(int)a.size(); i+=2 ) {
                pt = a.point( i );
                d->pageStream << XCOORD(pt.x()) << ' '
                       << YCOORD(pt.y()) << " MT\n";
                pt = a.point( i+1 );
                d->pageStream << XCOORD(pt.x()) << ' '
                       << YCOORD(pt.y()) << " LT\n";
            }
            d->pageStream << "QS\n";
        }
        break;
    case PdcDrawPolyline:
        if ( p[0].ptarr->size() > 1 ) {
            QPointArray a = *p[0].ptarr;
            QPoint pt = a.point( 0 );
            d->pageStream << "NP\n"
                   << XCOORD(pt.x()) << ' ' << YCOORD(pt.y()) << " MT\n";
            for ( int i=1; i<(int)a.size(); i++ ) {
                pt = a.point( i );
                d->pageStream << XCOORD(pt.x()) << ' '
                       << YCOORD(pt.y()) << " LT\n";
            }
            d->pageStream << "QS\n";
        }
        break;
    case PdcDrawPolygon:
        if ( p[0].ptarr->size() > 2 ) {
            QPointArray a = *p[0].ptarr;
            if ( p[1].ival )
                d->pageStream << "/WFi true d\n";
            QPoint pt = a.point(0);
            d->pageStream << "NP\n";
            d->pageStream << XCOORD(pt.x()) << ' '
                   << YCOORD(pt.y()) << " MT\n";
            for( int i=1; i<(int)a.size(); i++) {
                pt = a.point( i );
                d->pageStream << XCOORD(pt.x()) << ' '
                       << YCOORD(pt.y()) << " LT\n";
            }
            d->pageStream << "CP BF QS\n";
            if ( p[1].ival )
                d->pageStream << "/WFi false d\n";
        }
        break;
    case PdcDrawCubicBezier:
        if ( p[0].ptarr->size() == 4 ) {
            d->pageStream << "NP\n";
            QPointArray a = *p[0].ptarr;
            d->pageStream << XCOORD(a[0].x()) << ' '
                   << YCOORD(a[0].y()) << " MT ";
            for ( int i=1; i<4; i++ ) {
                d->pageStream << XCOORD(a[i].x()) << ' '
                       << YCOORD(a[i].y()) << ' ';
            }
            d->pageStream << "BZ\n";
        }
        break;
    case PdcDrawText2:
	// we use drawTextItem instead
	return TRUE;
    case PdcDrawText2Formatted:
        return TRUE;
    case PdcDrawTextItem: {
	const QTextItem *ti = p[1].textItem;
	QScriptItem &si = ti->engine->items[ti->item];
	int len = ti->engine->length( ti->item );
	if ( si.isSpace || si.isObject )
	    return FALSE;

	if ( d->currentSet != d->currentUsed || d->scriptUsed != si.analysis.script || !d->currentFontFile ) {
	    d->currentUsed = d->currentSet;
	    d->setFont( d->currentSet, si.analysis.script );
	}
	if( d->currentFontFile ) // better not crash in case somethig goes wrong.
	    d->currentFontFile->drawText( d->pageStream, *p[0].point, ti->engine, ti->item,
					  ti->engine->string.mid( si.position, len ), d, paint);
        return FALSE;
    }
    case PdcDrawPixmap: {
        if ( p[1].pixmap->isNull() )
            break;
        QRect r = *p[0].rect;
        QImage img;
        img = *(p[1].pixmap);
	QImage mask;
	if ( p[1].pixmap->mask() )
	    mask = *(p[1].pixmap->mask());
        d->drawImage(paint, r.x(), r.y(), r.width(), r.height(), img, mask);
        break;
    }
    case PdcDrawImage: {
        if ( p[1].image->isNull() )
            break;
        QRect r = *(p[0].rect);
        QImage img = *(p[1].image);
	QImage mask;
#ifndef QT_NO_IMAGE_DITHER_TO_1
	if ( img.hasAlphaBuffer() )
	    mask = img.createAlphaMask();
#endif
        d->drawImage(paint, r.x(), r.y(), r.width(), r.height(), img, mask);
        break;
    }
    case PdcSetBkColor:
    {
	if ( d->bkColor != *(p[0].color) ) {
	    d->bkColor = *(p[0].color);
	    d->dirtyBkColor = TRUE;
	}
        break;
    }
    case PdcSetBkMode:
    {
	if ( d->bkMode != p[0].ival ) {
	    d->bkMode = (Qt::BGMode) p[0].ival;
	    d->dirtyBkMode = TRUE;
	}
        break;
    }
    case PdcSetROP:
#if defined(CHECK_RANGE)
        if ( p[0].ival != Qt::CopyROP )
            qWarning( "QPrinter: Raster operation setting not supported" );
#endif
        break;
    case PdcSetBrushOrigin:
        break;
    case PdcSetFont:
        d->currentSet = *(p[0].font);
	d->fm = paint->fontMetrics();
        // turn these off - they confuse the 'avoid font change' logic
        d->currentSet.setUnderline( FALSE );
        d->currentSet.setStrikeOut( FALSE );
        break;
    case PdcSetPen:
        if ( d->cpen != *(p[0].pen) ) {
            d->dirtypen = TRUE;
            d->cpen = *(p[0].pen);
        }
        break;
    case PdcSetBrush:
        if ( p[0].brush->style() == Qt::CustomPattern ) {
#if defined(CHECK_RANGE)
            qWarning( "QPrinter: Pixmap brush not supported" );
#endif
            return FALSE;
        }
        if ( d->cbrush != *(p[0].brush) ) {
            d->dirtybrush = TRUE;
            d->cbrush = *(p[0].brush);
        }
        break;
    case PdcSetTabStops:
    case PdcSetTabArray:
        return FALSE;
    case PdcSetUnit:
        break;
    case PdcSetVXform:
    case PdcSetWindow:
    case PdcSetViewport:
    case PdcSetWXform:
    case PdcSetWMatrix:
    case PdcRestoreWMatrix:
        d->dirtyMatrix = TRUE;
        break;
    case PdcSetClip:
        d->dirtyClipping = TRUE;
        break;
    case PdcSetClipRegion:
        d->dirtyClipping = TRUE;
        break;
    case NewPage:
    	// we're writing to lp/lpr through a pipe, we don't want to crash with SIGPIPE
	// if lp/lpr dies
    	ignoreSigPipe(TRUE);
        d->flushPage();
	ignoreSigPipe(FALSE);

        d->dirtyNewPage = TRUE;
        break;
    case AbortPrinting:
        break;
    default:
        break;
    }
    return TRUE;
}

#endif // QT_NO_PRINTER
