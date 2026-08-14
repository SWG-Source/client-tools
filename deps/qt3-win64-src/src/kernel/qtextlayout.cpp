/****************************************************************************
** $Id: qtextlayout.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** ???
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

#include "qtextlayout_p.h"
#include "qtextengine_p.h"

#include <qfont.h>
#include <qapplication.h>
#include <qpainter.h>


QRect QTextItem::rect() const
{
    QScriptItem& si = engine->items[item];
    return QRect( si.x, si.y, si.width, si.ascent+si.descent );
}

int QTextItem::x() const
{
    return engine->items[item].x;
}

int QTextItem::y() const
{
    return engine->items[item].y;
}

int QTextItem::width() const
{
    return engine->items[item].width;
}

int QTextItem::ascent() const
{
    return engine->items[item].ascent;
}

int QTextItem::descent() const
{
    return engine->items[item].descent;
}

void QTextItem::setWidth( int w )
{
    engine->items[item].width = w;
}

void QTextItem::setAscent( int a )
{
    engine->items[item].ascent = a;
}

void QTextItem::setDescent( int d )
{
    engine->items[item].descent = d;
}

int QTextItem::from() const
{
    return engine->items[item].position;
}

int QTextItem::length() const
{
    return engine->length(item);
}


int QTextItem::cursorToX( int *cPos, Edge edge ) const
{
    int pos = *cPos;
    QScriptItem *si = &engine->items[item];

    engine->shape( item );
    advance_t *advances = engine->advances( si );
    GlyphAttributes *glyphAttributes = engine->glyphAttributes( si );
    unsigned short *logClusters = engine->logClusters( si );

    int l = engine->length( item );
    if ( pos > l )
	pos = l;
    if ( pos < 0 )
	pos = 0;

    int glyph_pos = pos == l ? si->num_glyphs : logClusters[pos];
    if ( edge == Trailing ) {
	// trailing edge is leading edge of next cluster
	while ( glyph_pos < si->num_glyphs && !glyphAttributes[glyph_pos].clusterStart )
	    glyph_pos++;
    }

    int x = 0;
    bool reverse = engine->items[item].analysis.bidiLevel % 2;

    if ( reverse ) {
	for ( int i = si->num_glyphs-1; i >= glyph_pos; i-- )
	    x += advances[i];
    } else {
	for ( int i = 0; i < glyph_pos; i++ )
	    x += advances[i];
    }
//     qDebug("cursorToX: pos=%d, gpos=%d x=%d", pos, glyph_pos, x );
    *cPos = pos;
    return x;
}

int QTextItem::xToCursor( int x, CursorPosition cpos ) const
{
    QScriptItem *si = &engine->items[item];
    engine->shape( item );
    advance_t *advances = engine->advances( si );
    unsigned short *logClusters = engine->logClusters( si );

    int l = engine->length( item );
    bool reverse = si->analysis.bidiLevel % 2;
    if ( x < 0 )
	return reverse ? l : 0;


    if ( reverse ) {
	int width = 0;
	for ( int i = 0; i < si->num_glyphs; i++ ) {
	    width += advances[i];
	}
	x = -x + width;
    }
    int cp_before = 0;
    int cp_after = 0;
    int x_before = 0;
    int x_after = 0;

    int lastCluster = 0;
    for ( int i = 1; i <= l; i++ ) {
	int newCluster = i < l ? logClusters[i] : si->num_glyphs;
	if ( newCluster != lastCluster ) {
	    // calculate cluster width
	    cp_before = cp_after;
	    x_before = x_after;
	    cp_after = i;
	    for ( int j = lastCluster; j < newCluster; j++ )
		x_after += advances[j];
	    // 		qDebug("cluster boundary: lastCluster=%d, newCluster=%d, x_before=%d, x_after=%d",
	    // 		       lastCluster, newCluster, x_before, x_after );
	    if ( x_after > x )
		break;
	    lastCluster = newCluster;
	}
    }

    bool before = ( cpos == OnCharacters || (x - x_before) < (x_after - x) );

//     qDebug("got cursor position for %d: %d/%d, x_ba=%d/%d using %d",
// 	   x, cp_before,cp_after,  x_before, x_after,  before ? cp_before : cp_after );

    return before ? cp_before : cp_after;

}


bool QTextItem::isRightToLeft() const
{
    return (engine->items[item].analysis.bidiLevel % 2);
}

bool QTextItem::isObject() const
{
    return engine->items[item].isObject;
}

bool QTextItem::isSpace() const
{
    return engine->items[item].isSpace;
}

bool QTextItem::isTab() const
{
    return engine->items[item].isTab;
}


QTextLayout::QTextLayout()
    :d(0) {}

QTextLayout::QTextLayout( const QString& string, QPainter *p )
{
    QFontPrivate *f = p ? ( p->pfont ? p->pfont->d : p->cfont.d ) : QApplication::font().d;
    d = new QTextEngine( (string.isNull() ? (const QString&)QString::fromLatin1("") : string), f );
}

QTextLayout::QTextLayout( const QString& string, const QFont& fnt )
{
    d = new QTextEngine( (string.isNull() ? (const QString&)QString::fromLatin1("") : string), fnt.d );
}

QTextLayout::~QTextLayout()
{
    delete d;
}

void QTextLayout::setText( const QString& string, const QFont& fnt )
{
    delete d;
    d = new QTextEngine( (string.isNull() ? (const QString&)QString::fromLatin1("") : string), fnt.d );
}

/* add an additional item boundary eg. for style change */
void QTextLayout::setBoundary( int strPos )
{
    if ( strPos <= 0 || strPos >= (int)d->string.length() )
	return;

    int itemToSplit = 0;
    while ( itemToSplit < d->items.size() && d->items[itemToSplit].position <= strPos )
	itemToSplit++;
    itemToSplit--;
    if ( d->items[itemToSplit].position == strPos ) {
	// already a split at the requested position
	return;
    }
    d->splitItem( itemToSplit, strPos - d->items[itemToSplit].position );
}


int QTextLayout::numItems() const
{
    return d->items.size();
}

QTextItem QTextLayout::itemAt( int i ) const
{
    return QTextItem( i, d );
}


QTextItem QTextLayout::findItem( int strPos ) const
{
    if ( strPos == 0 && d->items.size() )
	return QTextItem( 0, d );
    // ## TODO use bsearch
    for ( int i = d->items.size()-1; i >= 0; --i ) {
	if ( d->items[i].position < strPos )
	    return QTextItem( i, d );
    }
    return QTextItem();
}


void QTextLayout::beginLayout( QTextLayout::LayoutMode m )
{
    d->items.clear();
    QTextEngine::Mode mode = QTextEngine::Full;
    if (m == NoBidi)
	mode = QTextEngine::NoBidi;
    else if (m == SingleLine)
	mode = QTextEngine::SingleLine;
    d->itemize( mode );
    d->currentItem = 0;
    d->firstItemInLine = -1;
}

void QTextLayout::beginLine( int width )
{
    d->lineWidth = width;
    d->widthUsed = 0;
    d->firstItemInLine = -1;
}

bool QTextLayout::atEnd() const
{
    return d->currentItem >= d->items.size();
}

QTextItem QTextLayout::nextItem()
{
    d->currentItem++;

    if ( d->currentItem >= d->items.size() )
	return QTextItem();

    d->shape( d->currentItem );
    return QTextItem( d->currentItem, d );
}

QTextItem QTextLayout::currentItem()
{
    if ( d->currentItem >= d->items.size() )
	return QTextItem();

    d->shape( d->currentItem );
    return QTextItem( d->currentItem, d );
}

/* ## maybe also currentItem() */
void QTextLayout::setLineWidth( int newWidth )
{
    d->lineWidth = newWidth;
}

int QTextLayout::lineWidth() const
{
    return d->lineWidth;
}

int QTextLayout::widthUsed() const
{
    return d->widthUsed;
}

int QTextLayout::availableWidth() const
{
    return d->lineWidth - d->widthUsed;
}


/* returns true if completely added */
QTextLayout::Result QTextLayout::addCurrentItem()
{
    if ( d->firstItemInLine == -1 )
	d->firstItemInLine = d->currentItem;
    QScriptItem &current = d->items[d->currentItem];
    d->shape( d->currentItem );
    d->widthUsed += current.width;
//     qDebug("trying to add item %d with width %d, remaining %d", d->currentItem, current.width, d->lineWidth-d->widthUsed );

    d->currentItem++;

    return (d->widthUsed <= d->lineWidth
	    || (d->currentItem < d->items.size() && d->items[d->currentItem].isSpace)) ? Ok : LineFull;
}

QTextLayout::Result QTextLayout::endLine( int x, int y, int alignment,
					  int *ascent, int *descent, int *lineLeft, int *lineRight )
{
    int available = d->lineWidth;
    int numRuns = 0;
    int numSpaceItems = 0;
    Q_UINT8 _levels[128];
    int _visual[128];
    Q_UINT8 *levels = _levels;
    int *visual = _visual;
    int i;
    QTextLayout::Result result = LineEmpty;

//    qDebug("endLine x=%d, y=%d, first=%d, current=%d lw=%d wu=%d", x,  y, d->firstItemInLine, d->currentItem, d->lineWidth, d->widthUsed );
    int width_nobreak_found = d->widthUsed;
    if ( d->firstItemInLine == -1 )
	goto end;

    if ( !(alignment & (Qt::SingleLine|Qt::IncludeTrailingSpaces))
	&& d->currentItem > d->firstItemInLine && d->items[d->currentItem-1].isSpace ) {
	int i = d->currentItem-1;
	while ( i > d->firstItemInLine && d->items[i].isSpace ) {
	    numSpaceItems++;
	    d->widthUsed -= d->items[i--].width;
	}
    }

    if ( (alignment & (Qt::WordBreak|Qt::BreakAnywhere)) &&
	 d->widthUsed > d->lineWidth ) {
	// find linebreak

	// even though we removed trailing spaces the line was too wide. We'll have to break at an earlier
	// position. To not confuse the layouting below, reset the number of space items
	numSpaceItems = 0;


	bool breakany = alignment & Qt::BreakAnywhere;

	const QCharAttributes *attrs = d->attributes();
	int w = 0;
	int itemWidth = 0;
	int breakItem = d->firstItemInLine;
	int breakPosition = -1;
#if 0
	// we iterate backwards or forward depending on what we guess is closer
	if ( d->widthUsed - d->lineWidth < d->lineWidth ) {
	    // backwards search should be faster

	} else
#endif
	{
	    int tmpWidth = 0;
	    int swidth = 0;
	    // forward search is probably faster
	    for ( int i = d->firstItemInLine; i < d->currentItem; i++ ) {
		const QScriptItem *si = &d->items[i];
		int length = d->length( i );
		const QCharAttributes *itemAttrs = attrs + si->position;

		advance_t *advances = d->advances( si );
		unsigned short *logClusters = d->logClusters( si );

		int lastGlyph = 0;
		int tmpItemWidth = 0;

//     		qDebug("looking for break in item %d, isSpace=%d", i, si->isSpace );
		if(si->isSpace && !(alignment & (Qt::SingleLine|Qt::IncludeTrailingSpaces))) {
		    swidth += si->width;
		} else {
		    tmpWidth += swidth;
		    swidth = 0;
		    for ( int pos = 0; pos < length; pos++ ) {
//  			qDebug("advance=%d, w=%d, tmpWidth=%d, softbreak=%d, whitespace=%d",
//                                *advances, w, tmpWidth, itemAttrs->softBreak, itemAttrs->whiteSpace );
			int glyph = logClusters[pos];
			if ( lastGlyph != glyph ) {
			    while ( lastGlyph < glyph )
				tmpItemWidth += advances[lastGlyph++];
			    if ( breakPosition != -1 && w + tmpWidth + tmpItemWidth > d->lineWidth ) {
// 				qDebug("found break at w=%d, tmpWidth=%d, tmpItemWidth=%d", w, tmpWidth, tmpItemWidth);
				d->widthUsed = w;
				goto found;
			    }
			}
			if ( (itemAttrs->softBreak ||
			      ( breakany && itemAttrs->charStop ) ) &&
			     (i != d->firstItemInLine || pos != 0) ) {
			    if ( breakItem != i )
				itemWidth = 0;
			    if (itemAttrs->softBreak)
				breakany = FALSE;
			    breakItem = i;
			    breakPosition = pos;
//     			    qDebug("found possible break at item %d, position %d (absolute=%d), w=%d, tmpWidth=%d, tmpItemWidth=%d", breakItem, breakPosition, d->items[breakItem].position+breakPosition, w, tmpWidth, tmpItemWidth);
			    w += tmpWidth + tmpItemWidth;
			    itemWidth += tmpItemWidth;
			    tmpWidth = 0;
			    tmpItemWidth = 0;
			}
			itemAttrs++;
		    }
		    while ( lastGlyph < si->num_glyphs )
			tmpItemWidth += advances[lastGlyph++];
		    tmpWidth += tmpItemWidth;
		    if ( w + tmpWidth > d->lineWidth ) {
			d->widthUsed = w;
			goto found;
		    }
		}
	    }
	}

    found:
	// no valid break point found
	if ( breakPosition == -1 ) {
            d->widthUsed = width_nobreak_found;
	    goto nobreak;
        }

//   	qDebug("linebreak at item %d, position %d, wu=%d", breakItem, breakPosition, d->widthUsed );
	// split the line
	if ( breakPosition > 0 ) {
// 	    int length = d->length( breakItem );

//   	    qDebug("splitting item, itemWidth=%d", itemWidth);
	    // not a full item, need to break
	    d->splitItem( breakItem, breakPosition );
	    d->currentItem = breakItem+1;
	} else {
	    d->currentItem = breakItem;
	}
    }

    result = Ok;

 nobreak:
    // position the objects in the line
    available -= d->widthUsed;

    numRuns = d->currentItem - d->firstItemInLine - numSpaceItems;
    if ( numRuns > 127 ) {
	levels = new Q_UINT8[numRuns];
	visual = new int[numRuns];
    }

//     qDebug("reordering %d runs, numSpaceItems=%d", numRuns, numSpaceItems );
    for ( i = 0; i < numRuns; i++ ) {
	levels[i] = d->items[i+d->firstItemInLine].analysis.bidiLevel;
// 	qDebug("    level = %d", d->items[i+d->firstItemInLine].analysis.bidiLevel );
    }
    d->bidiReorder( numRuns, levels, visual );

 end:
    // ### FIXME
    if ( alignment & Qt::AlignJustify ) {
	// #### justify items
	alignment = Qt::AlignAuto;
    }
    if ( (alignment & Qt::AlignHorizontal_Mask) == Qt::AlignAuto )
	alignment = Qt::AlignLeft;
    if ( alignment & Qt::AlignRight )
	x += available;
    else if ( alignment & Qt::AlignHCenter )
	x += available/2;


    int asc = ascent ? *ascent : 0;
    int desc = descent ? *descent : 0;

    for ( i = 0; i < numRuns; i++ ) {
	QScriptItem &si = d->items[d->firstItemInLine+visual[i]];
	asc = QMAX( asc, si.ascent );
	desc = QMAX( desc, si.descent );
    }

    int left = x;
    for ( i = 0; i < numRuns; i++ ) {
	QScriptItem &si = d->items[d->firstItemInLine+visual[i]];
//  	qDebug("positioning item %d with width %d (from=%d/length=%d) at %d", d->firstItemInLine+visual[i], si.width, si.position,
// 	       d->length(d->firstItemInLine+visual[i]), x );
	si.x = x;
	si.y = y + asc;
	x += si.width;
    }
    int right = x;

    if ( numSpaceItems ) {
	if ( d->items[d->firstItemInLine+numRuns].analysis.bidiLevel % 2 ) {
	    x = left;
	    for ( i = 0; i < numSpaceItems; i++ ) {
		QScriptItem &si = d->items[d->firstItemInLine + numRuns + i];
		x -= si.width;
		si.x = x;
		si.y = y + asc;
	    }
	} else {
	    for ( i = 0; i < numSpaceItems; i++ ) {
		QScriptItem &si = d->items[d->firstItemInLine + numRuns + i];
		si.x = x;
		si.y = y + asc;
		x += si.width;
	    }
	}
    }

    if ( lineLeft )
	*lineLeft = left;
    if ( lineRight )
	*lineRight = right;
    if ( ascent )
	*ascent = asc;
    if ( descent )
	*descent = desc;

    if (levels != _levels)
	delete []levels;
    if (visual != _visual)
	delete []visual;

    return result;
}

void QTextLayout::endLayout()
{
    // nothing to do currently
}


int QTextLayout::nextCursorPosition( int oldPos, CursorMode mode ) const
{
//     qDebug("looking for next cursor pos for %d", oldPos );
    const QCharAttributes *attributes = d->attributes();
    int len = d->string.length();
    if ( oldPos >= len )
	return oldPos;
    oldPos++;
    if ( mode == SkipCharacters ) {
	while ( oldPos < len && !attributes[oldPos].charStop )
	    oldPos++;
    } else {
	while ( oldPos < len && !attributes[oldPos].wordStop && !attributes[oldPos-1].whiteSpace )
	    oldPos++;
    }
//     qDebug("  -> %d",  oldPos );
    return oldPos;
}

int QTextLayout::previousCursorPosition( int oldPos, CursorMode mode ) const
{
//     qDebug("looking for previous cursor pos for %d", oldPos );
    const QCharAttributes *attributes = d->attributes();
    if ( oldPos <= 0 )
	return 0;
    oldPos--;
    if ( mode == SkipCharacters ) {
	while ( oldPos && !attributes[oldPos].charStop )
	    oldPos--;
    } else {
	while ( oldPos && !attributes[oldPos].wordStop && !attributes[oldPos-1].whiteSpace )
	    oldPos--;
    }
//     qDebug("  -> %d",  oldPos );
    return oldPos;
}


bool QTextLayout::validCursorPosition( int pos ) const
{
    const QCharAttributes *attributes = d->attributes();
    if ( pos < 0 || pos > (int)d->string.length() )
	return FALSE;
    return attributes[pos].charStop;
}

void QTextLayout::setDirection(QChar::Direction dir)
{
    d->direction = dir;
}
