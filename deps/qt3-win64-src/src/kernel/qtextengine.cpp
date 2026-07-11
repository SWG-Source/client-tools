/****************************************************************************
** $Id: qtextengine.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Text engine classes
**
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
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

#include "qtextengine_p.h"

#include "qscriptengine_p.h"
#include <qfont.h>
#include "qfontdata_p.h"
#include "qfontengine_p.h"
#include <qstring.h>
#include <private/qunicodetables_p.h>
#include <stdlib.h>

// -----------------------------------------------------------------------------------------------------
//
// The BiDi algorithm
//
// -----------------------------------------------------------------------------------------------------


#define BIDI_DEBUG 0//2
#if (BIDI_DEBUG >= 1)
#include <iostream>
using namespace std;

static const char *directions[] = {
    "DirL", "DirR", "DirEN", "DirES", "DirET", "DirAN", "DirCS", "DirB", "DirS", "DirWS", "DirON",
    "DirLRE", "DirLRO", "DirAL", "DirRLE", "DirRLO", "DirPDF", "DirNSM", "DirBN"
};

#endif

struct BidiStatus {
    BidiStatus() {
	eor = QChar::DirON;
	lastStrong = QChar::DirON;
	last = QChar:: DirON;
	dir = QChar::DirON;
    }
    QChar::Direction eor;
    QChar::Direction lastStrong;
    QChar::Direction last;
    QChar::Direction dir;
};

struct BidiControl {
    struct Context {
	unsigned char level : 6;
	unsigned char override : 1;
	unsigned char unused : 1;
    };

    inline BidiControl( bool rtl )
	: cCtx( 0 ), singleLine( FALSE ) {
	ctx[0].level = (rtl ? 1 : 0);
	ctx[0].override = FALSE;
    }

    inline void embed( int level, bool override = FALSE ) {
	if ( ctx[cCtx].level < 61 && cCtx < 61 ) {
	    (void) ++cCtx;
	    ctx[cCtx].level = level;
	    ctx[cCtx].override = override;
	}
    }
    inline void pdf() {
	if ( cCtx ) (void) --cCtx;
    }

    inline uchar level() const {
	return ctx[cCtx].level;
    }
    inline bool override() const {
	return ctx[cCtx].override;
    }
    inline QChar::Direction basicDirection() {
	return (ctx[0].level ? QChar::DirR : QChar:: DirL );
    }
    inline uchar baseLevel() {
	return ctx[0].level;
    }
    inline QChar::Direction direction() {
	return ((ctx[cCtx].level%2) ? QChar::DirR : QChar:: DirL );
    }

    Context ctx[63];
    unsigned int cCtx : 8;
    bool singleLine : 8;
};

static QChar::Direction basicDirection( const QString &str )
{
    int len = str.length();
    int pos = 0;
    const QChar *uc = str.unicode() + pos;
    while( pos < len ) {
	switch( direction( *uc ) )
	{
	case QChar::DirL:
	case QChar::DirLRO:
	case QChar::DirLRE:
	    return QChar::DirL;
	case QChar::DirR:
	case QChar::DirAL:
	case QChar::DirRLO:
	case QChar::DirRLE:
	    return QChar::DirR;
	default:
	    break;
	}
	++pos;
	++uc;
    }
    return QChar::DirL;
}


static void qAppendItems(QTextEngine *engine, int &start, int &stop, BidiControl &control, QChar::Direction dir )
{
    QScriptItemArray &items = engine->items;
    const QChar *text = engine->string.unicode();

    if ( start > stop ) {
	// #### the algorithm is currently not really safe against this. Still needs fixing.
// 	qWarning( "Bidi: appendItems() internal error" );
	return;
    }

    int level = control.level();

    if(dir != QChar::DirON && !control.override()) {
	// add level of run (cases I1 & I2)
	if( level % 2 ) {
	    if(dir == QChar::DirL || dir == QChar::DirAN || dir == QChar::DirEN )
		level++;
	} else {
	    if( dir == QChar::DirR )
		level++;
	    else if( dir == QChar::DirAN || dir == QChar::DirEN )
		level += 2;
	}
    }

#if (BIDI_DEBUG >= 1)
    qDebug("new run: dir=%s from %d, to %d level = %d\n", directions[dir], start, stop, level);
#endif
    QFont::Script script = QFont::NoScript;
    QScriptItem item;
    item.position = start;
    item.analysis.script = script;
    item.analysis.bidiLevel = level;
    item.analysis.override = control.override();
    item.analysis.reserved = 0;

    if ( control.singleLine ) {
	for ( int i = start; i <= stop; i++ ) {

	    unsigned short uc = text[i].unicode();
	    QFont::Script s = (QFont::Script)scriptForChar( uc );
	    if (s == QFont::UnknownScript || s == QFont::CombiningMarks)
		s = script;

	    if (s != script) {
		item.analysis.script = s;
		item.analysis.bidiLevel = level;
		item.position = i;
		items.append( item );
		script = s;
	    }
	}
    } else {
	for ( int i = start; i <= stop; i++ ) {

	    unsigned short uc = text[i].unicode();
	    QFont::Script s = (QFont::Script)scriptForChar( uc );
	    if (s == QFont::UnknownScript || s == QFont::CombiningMarks)
		s = script;

	    QChar::Category category = ::category( uc );
	    if ( uc == 0xfffcU || uc == 0x2028U ) {
		item.analysis.bidiLevel = level % 2 ? level-1 : level;
		item.analysis.script = QFont::Latin;
		item.isObject = TRUE;
		s = QFont::NoScript;
	    } else if ((uc >= 9 && uc <=13) ||
		       (category >= QChar::Separator_Space && category <= QChar::Separator_Paragraph)) {
		item.analysis.script = QFont::Latin;
		item.isSpace = TRUE;
		item.isTab = ( uc == '\t' );
		item.analysis.bidiLevel = item.isTab ? control.baseLevel() : level;
		s = QFont::NoScript;
	    } else if ( s != script && (category != QChar::Mark_NonSpacing || script == QFont::NoScript)) {
		item.analysis.script = s;
		item.analysis.bidiLevel = level;
	    } else {
                if (i - start < 32000)
                    continue;
                start = i;
	    }

	    item.position = i;
	    items.append( item );
	    script = s;
	    item.isSpace = item.isTab = item.isObject = FALSE;
	}
    }
    ++stop;
    start = stop;
}

typedef void (* fAppendItems)(QTextEngine *, int &start, int &stop, BidiControl &control, QChar::Direction dir);
static fAppendItems appendItems = qAppendItems;

// creates the next QScript items.
static void bidiItemize( QTextEngine *engine, bool rightToLeft, int mode )
{
    BidiControl control( rightToLeft );
    if ( mode & QTextEngine::SingleLine )
	control.singleLine = TRUE;

    int sor = 0;
    int eor = -1;

    // ### should get rid of this!
    bool first = TRUE;

    int length = engine->string.length();

    if ( !length )
	return;

    const QChar *unicode = engine->string.unicode();
    int current = 0;

    QChar::Direction dir = rightToLeft ? QChar::DirR : QChar::DirL;
    BidiStatus status;
    QChar::Direction sdir = direction( *unicode );
    if ( sdir != QChar::DirL && sdir != QChar::DirR && sdir != QChar::DirAL && sdir != QChar::DirEN && sdir != QChar::DirAN )
	sdir = QChar::DirON;
    else
        dir = QChar::DirON;
    status.eor = sdir;
    status.lastStrong = rightToLeft ? QChar::DirR : QChar::DirL;
    status.last = status.lastStrong;
    status.dir = sdir;
#if (BIDI_DEBUG >= 2)
    qDebug("---- bidiReorder --- '%s'", engine->string.utf8().data());
    qDebug("rightToLeft = %d", rightToLeft);
#endif


    while ( current <= length ) {

	QChar::Direction dirCurrent;
	if ( current == (int)length )
	    dirCurrent = control.basicDirection();
	else
	    dirCurrent = direction( unicode[current] );

#if (BIDI_DEBUG >= 2)
	cout << "pos=" << current << " dir=" << directions[dir]
	     << " current=" << directions[dirCurrent] << " last=" << directions[status.last]
	     << " eor=" << eor << "/" << directions[status.eor]
	     << " sor=" << sor << " lastStrong="
	     << directions[status.lastStrong]
	     << " level=" << (int)control.level() << endl;
#endif

	switch(dirCurrent) {

	    // embedding and overrides (X1-X9 in the BiDi specs)
	case QChar::DirRLE:
	case QChar::DirRLO:
	case QChar::DirLRE:
	case QChar::DirLRO:
	    {
		bool rtl = (dirCurrent == QChar::DirRLE || dirCurrent == QChar::DirRLO );
		bool override = (dirCurrent == QChar::DirLRO || dirCurrent == QChar::DirRLO );

		uchar level = control.level();
		if( (level%2 != 0) == rtl  )
		    level += 2;
		else
		    level++;
		if(level < 61) {
		    eor = current-1;
		    appendItems(engine, sor, eor, control, dir);
		    eor = current;
		    control.embed( level, override );
		    QChar::Direction edir = (rtl ? QChar::DirR : QChar::DirL );
		    dir = status.eor = edir;
		    status.lastStrong = edir;
		}
		break;
	    }
	case QChar::DirPDF:
	    {
		if (dir != control.direction()) {
		    eor = current-1;
		    appendItems(engine, sor, eor, control, dir);
		    dir = control.direction();
		}
		eor = current;
		appendItems(engine, sor, eor, control, dir);
		dir = QChar::DirON; status.eor = QChar::DirON;
		status.last = control.direction();
		control.pdf();
		if ( control.override() )
		    dir = control.direction();
		else
		    dir = QChar::DirON;
		status.lastStrong = control.direction();
		break;
	    }

	    // strong types
	case QChar::DirL:
	    if(dir == QChar::DirON)
		dir = QChar::DirL;
	    switch(status.last)
		{
		case QChar::DirL:
		    eor = current; status.eor = QChar::DirL; break;
		case QChar::DirR:
		case QChar::DirAL:
		case QChar::DirEN:
		case QChar::DirAN:
		    if ( !first ) {
			appendItems(engine, sor, eor, control, dir);
			dir = eor < length ? direction( unicode[eor] ) : control.basicDirection();
			status.eor = dir;
		    } else {
                        eor = current; status.eor = dir;
                    }
		    break;
		case QChar::DirES:
		case QChar::DirET:
		case QChar::DirCS:
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:
		    if(dir != QChar::DirL) {
			//last stuff takes embedding dir
			if( control.direction() == QChar::DirR ) {
			    if(status.eor != QChar::DirR) {
				// AN or EN
				appendItems(engine, sor, eor, control, dir);
				status.eor = QChar::DirON;
				dir = QChar::DirR;
			    }
			    eor = current - 1;
			    appendItems(engine, sor, eor, control, dir);
			    dir = eor < length ? direction( unicode[eor] ) : control.basicDirection();
			    status.eor = dir;
			} else {
			    if(status.eor != QChar::DirL) {
				appendItems(engine, sor, eor, control, dir);
				status.eor = QChar::DirON;
				dir = QChar::DirL;
			    } else {
				eor = current; status.eor = QChar::DirL; break;
			    }
			}
		    } else {
			eor = current; status.eor = QChar::DirL;
		    }
		default:
		    break;
		}
	    status.lastStrong = QChar::DirL;
	    break;
	case QChar::DirAL:
	case QChar::DirR:
	    if(dir == QChar::DirON) dir = QChar::DirR;
	    switch(status.last)
		{
		case QChar::DirL:
		case QChar::DirEN:
		case QChar::DirAN:
		    if ( !first ) {
			appendItems(engine, sor, eor, control, dir);
			dir = QChar::DirON; status.eor = QChar::DirON;
			break;
		    }
		case QChar::DirR:
		case QChar::DirAL:
		    eor = current; status.eor = QChar::DirR; break;
		case QChar::DirES:
		case QChar::DirET:
		case QChar::DirCS:
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:
		    if( status.eor != QChar::DirR && status.eor != QChar::DirAL ) {
			//last stuff takes embedding dir
			if(control.direction() == QChar::DirR
                           || status.lastStrong == QChar::DirR || status.lastStrong == QChar::DirAL) {
			    appendItems(engine, sor, eor, control, dir);
			    dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirR;
			    eor = current;
			} else {
			    eor = current - 1;
			    appendItems(engine, sor, eor, control, dir);
			    dir = QChar::DirON; status.eor = QChar::DirON;
			    dir = QChar::DirR;
			}
		    } else {
			eor = current; status.eor = QChar::DirR;
		    }
		default:
		    break;
		}
	    status.lastStrong = dirCurrent;
	    break;

	    // weak types:

	case QChar::DirNSM:
	    if (eor == current-1)
		eor = current;
	    break;
	case QChar::DirEN:
	    // if last strong was AL change EN to AN
	    if(status.lastStrong != QChar::DirAL) {
		if(dir == QChar::DirON) {
		    if(status.lastStrong == QChar::DirL)
			dir = QChar::DirL;
		    else
			dir = QChar::DirEN;
		}
		switch(status.last)
		    {
		    case QChar::DirET:
			if ( status.lastStrong == QChar::DirR || status.lastStrong == QChar::DirAL ) {
			    appendItems(engine, sor, eor, control, dir);
			    status.eor = QChar::DirON;
			    dir = QChar::DirAN;
			}
			// fall through
		    case QChar::DirEN:
		    case QChar::DirL:
			eor = current;
			status.eor = dirCurrent;
			break;
		    case QChar::DirR:
		    case QChar::DirAL:
		    case QChar::DirAN:
			if ( !first )
			    appendItems(engine, sor, eor, control, dir);
			status.eor = QChar::DirEN;
			dir = QChar::DirAN; break;
		    case QChar::DirES:
		    case QChar::DirCS:
			if(status.eor == QChar::DirEN || dir == QChar::DirAN) {
			    eor = current; break;
			}
		    case QChar::DirBN:
		    case QChar::DirB:
		    case QChar::DirS:
		    case QChar::DirWS:
		    case QChar::DirON:
			if(status.eor == QChar::DirR) {
			    // neutrals go to R
			    eor = current - 1;
			    appendItems(engine, sor, eor, control, dir);
			    dir = QChar::DirON; status.eor = QChar::DirEN;
			    dir = QChar::DirAN;
			}
			else if( status.eor == QChar::DirL ||
				 (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
			    eor = current; status.eor = dirCurrent;
			} else {
			    // numbers on both sides, neutrals get right to left direction
			    if(dir != QChar::DirL) {
				appendItems(engine, sor, eor, control, dir);
				dir = QChar::DirON; status.eor = QChar::DirON;
				eor = current - 1;
				dir = QChar::DirR;
				appendItems(engine, sor, eor, control, dir);
				dir = QChar::DirON; status.eor = QChar::DirON;
				dir = QChar::DirAN;
			    } else {
				eor = current; status.eor = dirCurrent;
			    }
			}
		    default:
			break;
		    }
		break;
	    }
	case QChar::DirAN:
	    dirCurrent = QChar::DirAN;
	    if(dir == QChar::DirON) dir = QChar::DirAN;
	    switch(status.last)
		{
		case QChar::DirL:
		case QChar::DirAN:
		    eor = current; status.eor = QChar::DirAN; break;
		case QChar::DirR:
		case QChar::DirAL:
		case QChar::DirEN:
		    if ( !first )
			appendItems(engine, sor, eor, control, dir);
		    dir = QChar::DirON; status.eor = QChar::DirAN;
		    break;
		case QChar::DirCS:
		    if(status.eor == QChar::DirAN) {
			eor = current; break;
		    }
		case QChar::DirES:
		case QChar::DirET:
		case QChar::DirBN:
		case QChar::DirB:
		case QChar::DirS:
		case QChar::DirWS:
		case QChar::DirON:
		    if(status.eor == QChar::DirR) {
			// neutrals go to R
			eor = current - 1;
			appendItems(engine, sor, eor, control, dir);
			status.eor = QChar::DirAN;
			dir = QChar::DirAN;
		    } else if( status.eor == QChar::DirL ||
			       (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
			eor = current; status.eor = dirCurrent;
		    } else {
			// numbers on both sides, neutrals get right to left direction
			if(dir != QChar::DirL) {
			    appendItems(engine, sor, eor, control, dir);
			    status.eor = QChar::DirON;
			    eor = current - 1;
			    dir = QChar::DirR;
			    appendItems(engine, sor, eor, control, dir);
			    status.eor = QChar::DirAN;
			    dir = QChar::DirAN;
			} else {
			    eor = current; status.eor = dirCurrent;
			}
		    }
		default:
		    break;
		}
	    break;
	case QChar::DirES:
	case QChar::DirCS:
	    break;
	case QChar::DirET:
	    if(status.last == QChar::DirEN) {
		dirCurrent = QChar::DirEN;
		eor = current; status.eor = dirCurrent;
	    }
	    break;

	    // boundary neutrals should be ignored
	case QChar::DirBN:
	    break;
	    // neutrals
	case QChar::DirB:
	    // ### what do we do with newline and paragraph separators that come to here?
	    break;
	case QChar::DirS:
	    // ### implement rule L1
	    break;
	case QChar::DirWS:
	case QChar::DirON:
	    break;
	default:
	    break;
	}

	//cout << "     after: dir=" << //        dir << " current=" << dirCurrent << " last=" << status.last << " eor=" << status.eor << " lastStrong=" << status.lastStrong << " embedding=" << control.direction() << endl;

	if(current >= (int)length) break;

	// set status.last as needed.
	switch(dirCurrent) {
	case QChar::DirET:
	case QChar::DirES:
	case QChar::DirCS:
	case QChar::DirS:
	case QChar::DirWS:
	case QChar::DirON:
	    switch(status.last)
	    {
	    case QChar::DirL:
	    case QChar::DirR:
	    case QChar::DirAL:
	    case QChar::DirEN:
	    case QChar::DirAN:
		status.last = dirCurrent;
		break;
	    default:
		status.last = QChar::DirON;
	    }
	    break;
	case QChar::DirNSM:
	case QChar::DirBN:
	    // ignore these
	    break;
	case QChar::DirLRO:
	case QChar::DirLRE:
	    status.last = QChar::DirL;
	    break;
	case QChar::DirRLO:
	case QChar::DirRLE:
	    status.last = QChar::DirR;
	    break;
	case QChar::DirEN:
	    if ( status.last == QChar::DirL ) {
		status.last = QChar::DirL;
		break;
	    }
	    // fall through
	default:
	    status.last = dirCurrent;
	}

	first = FALSE;
	++current;
    }

#if (BIDI_DEBUG >= 1)
    cout << "reached end of line current=" << current << ", eor=" << eor << endl;
#endif
    eor = current - 1; // remove dummy char

    if ( sor <= eor )
	appendItems(engine, sor, eor, control, dir);


}

void QTextEngine::bidiReorder( int numItems, const Q_UINT8 *levels, int *visualOrder )
{

    // first find highest and lowest levels
    uchar levelLow = 128;
    uchar levelHigh = 0;
    int i = 0;
    while ( i < numItems ) {
	//printf("level = %d\n", r->level);
	if ( levels[i] > levelHigh )
	    levelHigh = levels[i];
	if ( levels[i] < levelLow )
	    levelLow = levels[i];
	i++;
    }

    // implements reordering of the line (L2 according to BiDi spec):
    // L2. From the highest level found in the text to the lowest odd level on each line,
    // reverse any contiguous sequence of characters that are at that level or higher.

    // reversing is only done up to the lowest odd level
    if(!(levelLow%2)) levelLow++;

#if (BIDI_DEBUG >= 1)
    cout << "reorderLine: lineLow = " << (uint)levelLow << ", lineHigh = " << (uint)levelHigh << endl;
#endif

    int count = numItems - 1;
    for ( i = 0; i < numItems; i++ )
	visualOrder[i] = i;

    while(levelHigh >= levelLow) {
	int i = 0;
	while ( i < count ) {
	    while(i < count && levels[i] < levelHigh) i++;
	    int start = i;
	    while(i <= count && levels[i] >= levelHigh) i++;
	    int end = i-1;

	    if(start != end) {
		//cout << "reversing from " << start << " to " << end << endl;
		for(int j = 0; j < (end-start+1)/2; j++) {
		    int tmp = visualOrder[start+j];
		    visualOrder[start+j] = visualOrder[end-j];
		    visualOrder[end-j] = tmp;
		}
	    }
	    i++;
	}
	levelHigh--;
    }

#if (BIDI_DEBUG >= 1)
    cout << "visual order is:" << endl;
    for ( i = 0; i < numItems; i++ )
	cout << visualOrder[i] << endl;
#endif
}


// -----------------------------------------------------------------------------------------------------
//
// The line break algorithm. See http://www.unicode.org/reports/tr14/tr14-13.html
//
// -----------------------------------------------------------------------------------------------------

/* The Unicode algorithm does in our opinion allow line breaks at some
   places they shouldn't be allowed. The following changes were thus
   made in comparison to the Unicode reference:

   CL->AL from Dbk to Ibk
   CL->PR from Dbk to Ibk
   EX->AL from Dbk to Ibk
   IS->AL from Dbk to Ibk
   PO->AL from Dbk to Ibk
   SY->AL from Dbk to Ibk
   SY->PO from Dbk to Ibk
   SY->PR from Dbk to Ibk
   SY->OP from Dbk to Ibk
   Al->OP from Dbk to Ibk
   AL->HY from Dbk to Ibk
   AL->PR from Dbk to Ibk
   AL->PO from Dbk to Ibk
   PR->PR from Dbk to Ibk
   PO->PO from Dbk to Ibk
   PR->PO from Dbk to Ibk
   PO->PR from Dbk to Ibk
   HY->PO from Dbk to Ibk
   HY->PR from Dbk to Ibk
   HY->OP from Dbk to Ibk
   PO->OP from Dbk to Ibk
   NU->EX from Dbk to Ibk
   NU->PR from Dbk to Ibk
   PO->NU from Dbk to Ibk
   EX->PO from Dbk to Ibk
*/

enum break_action {
    Dbk, // Direct break
    Ibk, // Indirect break; only allowed if space between the two chars
    Pbk // Prohibited break; no break allowed even if space between chars
};

// The following line break classes are not treated by the table:
// SA, BK, CR, LF, SG, CB, SP
static const Q_UINT8 breakTable[QUnicodeTables::LineBreak_CM+1][QUnicodeTables::LineBreak_CM+1] =
{
    // OP,  CL,  QU,  GL, NS,  EX,  SY,  IS,  PR,  PO,  NU,  AL,  ID,  IN,  HY,  BA,  BB,  B2,  ZW,  CM
    { Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk }, // OP
    { Dbk, Pbk, Ibk, Pbk, Pbk, Pbk, Pbk, Pbk, Ibk, Ibk, Dbk, Ibk, Dbk, Dbk, Ibk, Ibk, Pbk, Pbk, Pbk, Pbk }, // CL
    { Pbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Pbk, Pbk }, // QU
    { Ibk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Pbk, Pbk }, // GL
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // NS
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Ibk, Ibk, Ibk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // EX
    { Ibk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // SY
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // IS
    { Ibk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Ibk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Pbk }, // PR
    { Ibk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // PO
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Dbk, Ibk, Ibk, Ibk, Dbk, Dbk, Pbk, Pbk }, // NU
    { Ibk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Dbk, Ibk, Ibk, Ibk, Dbk, Dbk, Pbk, Pbk }, // AL
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Ibk, Dbk, Dbk, Dbk, Ibk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // ID
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Dbk, Dbk, Dbk, Dbk, Ibk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // IN
    { Ibk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // HY
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // BA
    { Ibk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Pbk, Ibk }, // BB
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Ibk, Ibk, Dbk, Pbk, Pbk, Ibk }, // B2
    { Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Pbk, Ibk }, // ZW
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Ibk, Dbk, Dbk, Dbk, Ibk, Ibk, Ibk, Dbk, Dbk, Pbk, Pbk }  // CM
};

// set the soft break flag at every possible line breaking point. This needs correct clustering information.
static void calcLineBreaks(const QString &str, QCharAttributes *charAttributes)
{
    int len = str.length();
    if (!len)
        return;

    const QChar *uc = str.unicode();
    int cls = lineBreakClass(*uc);
    if (cls >= QUnicodeTables::LineBreak_CM)
        cls = QUnicodeTables::LineBreak_ID;

    charAttributes[0].softBreak = FALSE;
    charAttributes[0].whiteSpace = (cls == QUnicodeTables::LineBreak_SP);
    charAttributes[0].charStop = TRUE;

    for (int i = 1; i < len; ++i) {
        int ncls = ::lineBreakClass(uc[i]);
        int category = ::category(uc[i]);
        if (category == QChar::Mark_NonSpacing)
            goto nsm;
        
        if (category == QChar::Other_Surrogate) {
            // char stop only on first pair
            if (uc[i].unicode() >= 0xd800 && uc[i].unicode() < 0xdc00 && i < len-1
                && uc[i+1].unicode() >= 0xdc00 && uc[i+1].unicode() < 0xe000)
                goto nsm;
            // ### correctly handle second surrogate
        }

        if (ncls == QUnicodeTables::LineBreak_SP) {
            charAttributes[i].softBreak = FALSE;
            charAttributes[i].whiteSpace = TRUE;
            charAttributes[i].charStop = TRUE;
            cls = ncls;
            continue;
        }


	if (cls == QUnicodeTables::LineBreak_SA && ncls == QUnicodeTables::LineBreak_SA) {
            // two complex chars (thai or lao), thai_attributes might override, but here
            // we do a best guess
            charAttributes[i].softBreak = TRUE;
            charAttributes[i].whiteSpace = FALSE;
            charAttributes[i].charStop = TRUE;
            cls = ncls;
            continue;
        }
        {
	    int tcls = ncls;
	    if (tcls >= QUnicodeTables::LineBreak_SA)
		tcls = QUnicodeTables::LineBreak_ID;
	    if (cls >= QUnicodeTables::LineBreak_SA)
		cls = QUnicodeTables::LineBreak_ID;

	    bool softBreak;
	    int brk = breakTable[cls][tcls];
	    if (brk == Ibk)
		softBreak = (cls == QUnicodeTables::LineBreak_SP);
	    else
		softBreak = (brk == Dbk);
//        qDebug("char = %c %04x, cls=%d, ncls=%d, brk=%d soft=%d", uc[i].cell(), uc[i].unicode(), cls, ncls, brk, charAttributes[i].softBreak);
	    charAttributes[i].softBreak = softBreak;
	    charAttributes[i].whiteSpace = FALSE;
	    charAttributes[i].charStop = TRUE;
	    cls = ncls;
	}
        continue;
    nsm:
        charAttributes[i].softBreak = FALSE;
        charAttributes[i].whiteSpace = FALSE;
        charAttributes[i].charStop = FALSE;
    }
}

#if defined( Q_WS_X11 ) || defined ( Q_WS_QWS )
# include "qtextengine_unix.cpp"
#elif defined( Q_WS_WIN )
# include "qtextengine_win.cpp"
#elif defined( Q_WS_MAC )
# include "qtextengine_mac.cpp"
#endif



QTextEngine::QTextEngine( const QString &str, QFontPrivate *f )
    : string( str ), fnt( f ), direction( QChar::DirON ), haveCharAttributes( FALSE ), widthOnly( FALSE )
{
#ifdef Q_WS_WIN
    if ( !resolvedUsp10 )
	resolveUsp10();
#endif
    if ( fnt ) fnt->ref();

    num_glyphs = QMAX( 16, str.length()*3/2 );
    int space_charAttributes = (sizeof(QCharAttributes)*str.length()+sizeof(void*)-1)/sizeof(void*);
    int space_logClusters = (sizeof(unsigned short)*str.length()+sizeof(void*)-1)/sizeof(void*);
    int space_glyphs = (sizeof(glyph_t)*num_glyphs+sizeof(void*)-1)/sizeof(void*);
    int space_advances = (sizeof(advance_t)*num_glyphs+sizeof(void*)-1)/sizeof(void*);
    int space_offsets = (sizeof(qoffset_t)*num_glyphs+sizeof(void*)-1)/sizeof(void*);
    int space_glyphAttributes = (sizeof(GlyphAttributes)*num_glyphs+sizeof(void*)-1)/sizeof(void*);

    allocated = space_charAttributes + space_glyphs + space_advances +
		space_offsets + space_logClusters + space_glyphAttributes;
    memory = (void **)::malloc( allocated*sizeof( void * ) );
    memset( memory, 0, allocated*sizeof( void * ) );

    void **m = memory;
    m += space_charAttributes;
    logClustersPtr = (unsigned short *) m;
    m += space_logClusters;
    glyphPtr = (glyph_t *) m;
    m += space_glyphs;
    advancePtr = (advance_t *) m;
    m += space_advances;
    offsetsPtr = (qoffset_t *) m;
    m += space_offsets;
    glyphAttributesPtr = (GlyphAttributes *) m;

    used = 0;
}

QTextEngine::~QTextEngine()
{
    if ( fnt && fnt->deref())
	delete fnt;
    free( memory );
    allocated = 0;
}

void QTextEngine::reallocate( int totalGlyphs )
{
    int new_num_glyphs = totalGlyphs;
    int space_charAttributes = (sizeof(QCharAttributes)*string.length()+sizeof(void*)-1)/sizeof(void*);
    int space_logClusters = (sizeof(unsigned short)*string.length()+sizeof(void*)-1)/sizeof(void*);
    int space_glyphs = (sizeof(glyph_t)*new_num_glyphs+sizeof(void*)-1)/sizeof(void*);
    int space_advances = (sizeof(advance_t)*new_num_glyphs+sizeof(void*)-1)/sizeof(void*);
    int space_offsets = (sizeof(qoffset_t)*new_num_glyphs+sizeof(void*)-1)/sizeof(void*);
    int space_glyphAttributes = (sizeof(GlyphAttributes)*new_num_glyphs+sizeof(void*)-1)/sizeof(void*);

    int newAllocated = space_charAttributes + space_glyphs + space_advances +
		space_offsets + space_logClusters + space_glyphAttributes;
    void ** newMemory = (void **)::malloc( newAllocated*sizeof( void * ) );

    void **nm = newMemory;
    memcpy( nm, memory, string.length()*sizeof(QCharAttributes) );
    nm += space_charAttributes;
    memcpy( nm, logClustersPtr, num_glyphs*sizeof(unsigned short) );
    logClustersPtr = (unsigned short *) nm;
    nm += space_logClusters;
    memcpy( nm, glyphPtr, num_glyphs*sizeof(glyph_t) );
    glyphPtr = (glyph_t *) nm;
    nm += space_glyphs;
    memcpy( nm, advancePtr, num_glyphs*sizeof(advance_t) );
    advancePtr = (advance_t *) nm;
    nm += space_advances;
    memcpy( nm, offsetsPtr, num_glyphs*sizeof(qoffset_t) );
    offsetsPtr = (qoffset_t *) nm;
    nm += space_offsets;
    memcpy( nm, glyphAttributesPtr, num_glyphs*sizeof(GlyphAttributes) );
    glyphAttributesPtr = (GlyphAttributes *) nm;

    free( memory );
    memory = newMemory;
    allocated = newAllocated;
    num_glyphs = new_num_glyphs;
}

const QCharAttributes *QTextEngine::attributes()
{
    QCharAttributes *charAttributes = (QCharAttributes *) memory;
    if ( haveCharAttributes )
	return charAttributes;

    if ( !items.d )
	itemize();

    ensureSpace(string.length());
    charAttributes = (QCharAttributes *) memory;
    calcLineBreaks(string, charAttributes);
    
    for ( int i = 0; i < items.size(); i++ ) {
	QScriptItem &si = items[i];
#ifdef Q_WS_WIN
        int script = uspScriptForItem(this, i);
#else
        int script = si.analysis.script;
#endif
	Q_ASSERT( script < QFont::NScripts );
        AttributeFunction attributes = scriptEngines[script].charAttributes;
        if (!attributes)
            continue;
	int from = si.position;
	int len = length( i );
	attributes( script, string, from, len, charAttributes );
    }

    haveCharAttributes = TRUE;
    return charAttributes;
}

void QTextEngine::splitItem( int item, int pos )
{
    if ( pos <= 0 )
	return;

    // we have to ensure we get correct shaping for arabic and other
    // complex languages so we have to call shape _before_ we split the item.
    shape(item);

    if ( items.d->size == items.d->alloc )
	items.resize( items.d->size + 1 );

    int numMove = items.d->size - item-1;
    if ( numMove > 0 )
	memmove( items.d->items + item+2, items.d->items +item+1, numMove*sizeof( QScriptItem ) );
    items.d->size++;
    QScriptItem &newItem = items.d->items[item+1];
    QScriptItem &oldItem = items.d->items[item];
    newItem = oldItem;
    items.d->items[item+1].position += pos;
    if ( newItem.fontEngine )
	newItem.fontEngine->ref();

    if (oldItem.num_glyphs) {
	// already shaped, break glyphs aswell
	int breakGlyph = logClusters(&oldItem)[pos];

	newItem.num_glyphs = oldItem.num_glyphs - breakGlyph;
	oldItem.num_glyphs = breakGlyph;
	newItem.glyph_data_offset = oldItem.glyph_data_offset + breakGlyph;

	for (int i = 0; i < newItem.num_glyphs; i++)
	    logClusters(&newItem)[i] -= breakGlyph;

	int w = 0;
	const advance_t *a = advances(&oldItem);
	for(int j = 0; j < breakGlyph; ++j)
	    w += *(a++);

	newItem.width = oldItem.width - w;
	oldItem.width = w;
    }

//     qDebug("split at position %d itempos=%d", pos, item );
}


int QTextEngine::width( int from, int len ) const
{
    int w = 0;

//     qDebug("QTextEngine::width( from = %d, len = %d ), numItems=%d, strleng=%d", from,  len, items.size(), string.length() );
    for ( int i = 0; i < items.size(); i++ ) {
	QScriptItem *si = &items[i];
	int pos = si->position;
	int ilen = length( i );
//  	qDebug("item %d: from %d len %d", i, pos, ilen );
	if ( pos >= from + len )
	    break;
	if ( pos + ilen > from ) {
	    if ( !si->num_glyphs )
		shape( i );

	    advance_t *advances = this->advances( si );
	    unsigned short *logClusters = this->logClusters( si );

// 	    fprintf( stderr, "  logclusters:" );
// 	    for ( int k = 0; k < ilen; k++ )
// 		fprintf( stderr, " %d", logClusters[k] );
// 	    fprintf( stderr, "\n" );
	    // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
	    int charFrom = from - pos;
	    if ( charFrom < 0 )
		charFrom = 0;
	    int glyphStart = logClusters[charFrom];
	    if ( charFrom > 0 && logClusters[charFrom-1] == glyphStart )
		while ( charFrom < ilen && logClusters[charFrom] == glyphStart )
		    charFrom++;
	    if ( charFrom < ilen ) {
		glyphStart = logClusters[charFrom];
		int charEnd = from + len - 1 - pos;
		if ( charEnd >= ilen )
		    charEnd = ilen-1;
		int glyphEnd = logClusters[charEnd];
		while ( charEnd < ilen && logClusters[charEnd] == glyphEnd )
		    charEnd++;
		glyphEnd = (charEnd == ilen) ? si->num_glyphs : logClusters[charEnd];

// 		qDebug("char: start=%d end=%d / glyph: start = %d, end = %d", charFrom, charEnd, glyphStart, glyphEnd );
		for ( int i = glyphStart; i < glyphEnd; i++ )
		    w += advances[i];
	    }
	}
    }
//     qDebug("   --> w= %d ", w );
    return w;
}

void QTextEngine::itemize( int mode )
{
    if ( !items.d ) {
	int size = 8;
	items.d = (QScriptItemArrayPrivate *)malloc( sizeof( QScriptItemArrayPrivate ) +
						    sizeof( QScriptItem ) * size );
	items.d->alloc = size;
    }
    items.d->size = 0;
    if ( string.length() == 0 )
	return;

    if ( !(mode & NoBidi) ) {
	if ( direction == QChar::DirON )
	    direction = basicDirection( string );
	bidiItemize( this, direction == QChar::DirR, mode );
    } else {
	BidiControl control( FALSE );
	if ( mode & QTextEngine::SingleLine )
	    control.singleLine = TRUE;
	int start = 0;
	int stop = string.length() - 1;
	appendItems(this, start, stop, control, QChar::DirL);
    }
    if ( (mode & WidthOnly) == WidthOnly )
	widthOnly = TRUE;
}

glyph_metrics_t QTextEngine::boundingBox( int from,  int len ) const
{
    glyph_metrics_t gm;

    for ( int i = 0; i < items.size(); i++ ) {
	QScriptItem *si = &items[i];
	int pos = si->position;
	int ilen = length( i );
	if ( pos > from + len )
	    break;
	if ( pos + len > from ) {
	    if ( !si->num_glyphs )
		shape( i );
	    advance_t *advances = this->advances( si );
	    unsigned short *logClusters = this->logClusters( si );
	    glyph_t *glyphs = this->glyphs( si );
	    qoffset_t *offsets = this->offsets( si );

	    // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
	    int charFrom = from - pos;
	    if ( charFrom < 0 )
		charFrom = 0;
	    int glyphStart = logClusters[charFrom];
	    if ( charFrom > 0 && logClusters[charFrom-1] == glyphStart )
		while ( charFrom < ilen && logClusters[charFrom] == glyphStart )
		    charFrom++;
	    if ( charFrom < ilen ) {
		glyphStart = logClusters[charFrom];
		int charEnd = from + len - 1 - pos;
		if ( charEnd >= ilen )
		    charEnd = ilen-1;
		int glyphEnd = logClusters[charEnd];
		while ( charEnd < ilen && logClusters[charEnd] == glyphEnd )
		    charEnd++;
		glyphEnd = (charEnd == ilen) ? si->num_glyphs : logClusters[charEnd];
		if ( glyphStart <= glyphEnd  ) {
		    QFontEngine *fe = si->fontEngine;
		    glyph_metrics_t m = fe->boundingBox( glyphs+glyphStart, advances+glyphStart,
						       offsets+glyphStart, glyphEnd-glyphStart );
		    gm.x = QMIN( gm.x, m.x + gm.xoff );
		    gm.y = QMIN( gm.y, m.y + gm.yoff );
		    gm.width = QMAX( gm.width, m.width+gm.xoff );
		    gm.height = QMAX( gm.height, m.height+gm.yoff );
		    gm.xoff += m.xoff;
		    gm.yoff += m.yoff;
		}
	    }
	}
    }
    return gm;
}
