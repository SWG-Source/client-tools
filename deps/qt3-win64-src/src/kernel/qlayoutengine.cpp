/****************************************************************************
** $Id: qlayoutengine.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QLayout functionality
**
** Created : 981231
**
** Copyright (C) 1998-2007 Trolltech ASA.  All rights reserved.
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

#include "qlayout.h"
#include "private/qlayoutengine_p.h"

#ifndef QT_NO_LAYOUT

static inline int toFixed( int i ) { return i * 256; }
static inline int fRound( int i ) {
    return ( i % 256 < 128 ) ? i / 256 : 1 + i / 256;
}

/*
  This is the main workhorse of the QGridLayout. It portions out
  available space to the chain's children.

  The calculation is done in fixed point: "fixed" variables are
  scaled by a factor of 256.

  If the layout runs "backwards" (i.e. RightToLeft or Up) the layout
  is computed mirror-reversed, and it's the caller's responsibility
  do reverse the values before use.

  chain contains input and output parameters describing the geometry.
  count is the count of items in the chain; pos and space give the
  interval (relative to parentWidget topLeft).
*/
Q_EXPORT void qGeomCalc( QMemArray<QLayoutStruct> &chain, int start, int count,
			 int pos, int space, int spacer )
{
    typedef int fixed;
    int cHint = 0;
    int cMin = 0;
    int cMax = 0;
    int sumStretch = 0;
    int spacerCount = 0;

    bool wannaGrow = FALSE; // anyone who really wants to grow?
    //    bool canShrink = FALSE; // anyone who could be persuaded to shrink?

    int i;
    for ( i = start; i < start + count; i++ ) {
	chain[i].done = FALSE;
	cHint += chain[i].smartSizeHint();
	cMin += chain[i].minimumSize;
	cMax += chain[i].maximumSize;
	sumStretch += chain[i].stretch;
	if ( !chain[i].empty )
	    spacerCount++;
	wannaGrow = wannaGrow || chain[i].expansive || chain[i].stretch > 0;
    }

    int extraspace = 0;
    if ( spacerCount )
	spacerCount--; // only spacers between things
    if ( space < cMin + spacerCount * spacer ) {
	for ( i = start; i < start+count; i++ ) {
	    chain[i].size = chain[i].minimumSize;
	    chain[i].done = TRUE;
	}
    } else if ( space < cHint + spacerCount*spacer ) {
	/*
	  Less space than smartSizeHint(), but more than minimumSize.
	  Currently take space equally from each, as in Qt 2.x.
	  Commented-out lines will give more space to stretchier
	  items.
	*/
	int n = count;
	int space_left = space - spacerCount*spacer;
	int overdraft = cHint - space_left;

	// first give to the fixed ones:
	for ( i = start; i < start + count; i++ ) {
	    if ( !chain[i].done
		 && chain[i].minimumSize >= chain[i].smartSizeHint() ) {
		chain[i].size = chain[i].smartSizeHint();
		chain[i].done = TRUE;
		space_left -= chain[i].smartSizeHint();
		// sumStretch -= chain[i].stretch;
		n--;
	    }
	}
	bool finished = n == 0;
	while ( !finished ) {
	    finished = TRUE;
	    fixed fp_over = toFixed( overdraft );
	    fixed fp_w = 0;

	    for ( i = start; i < start+count; i++ ) {
		if ( chain[i].done )
		    continue;
		// if ( sumStretch <= 0 )
		fp_w += fp_over / n;
		// else
		//    fp_w += (fp_over * chain[i].stretch) / sumStretch;
		int w = fRound( fp_w );
		chain[i].size = chain[i].smartSizeHint() - w;
		fp_w -= toFixed( w ); // give the difference to the next
		if ( chain[i].size < chain[i].minimumSize ) {
		    chain[i].done = TRUE;
		    chain[i].size = chain[i].minimumSize;
		    finished = FALSE;
		    overdraft -= ( chain[i].smartSizeHint()
				   - chain[i].minimumSize );
		    // sumStretch -= chain[i].stretch;
		    n--;
		    break;
		}
	    }
	}
    } else { // extra space
	int n = count;
	int space_left = space - spacerCount*spacer;
	// first give to the fixed ones, and handle non-expansiveness
	for ( i = start; i < start + count; i++ ) {
	    if ( !chain[i].done
		 && (chain[i].maximumSize <= chain[i].smartSizeHint()
		     || (wannaGrow && !chain[i].expansive && chain[i].stretch == 0)) ) {
		chain[i].size = chain[i].smartSizeHint();
		chain[i].done = TRUE;
		space_left -= chain[i].smartSizeHint();
		sumStretch -= chain[i].stretch;
		n--;
	    }
	}
	extraspace = space_left;

	/*
	  Do a trial distribution and calculate how much it is off.
	  If there are more deficit pixels than surplus pixels, give
	  the minimum size items what they need, and repeat.
	  Otherwise give to the maximum size items, and repeat.

	  Paul Olav Tvete has a wonderful mathematical proof of the
	  correctness of this principle, but unfortunately this
	  comment is too small to contain it.
	*/
	int surplus, deficit;
	do {
	    surplus = deficit = 0;
	    fixed fp_space = toFixed( space_left );
	    fixed fp_w = 0;
	    for ( i = start; i < start+count; i++ ) {
		if ( chain[i].done )
		    continue;
		extraspace = 0;
		if ( sumStretch <= 0 )
		    fp_w += fp_space / n;
		else
		    fp_w += (fp_space * chain[i].stretch) / sumStretch;
		int w = fRound( fp_w );
		chain[i].size = w;
		fp_w -= toFixed( w ); // give the difference to the next
		if ( w < chain[i].smartSizeHint() ) {
		    deficit +=  chain[i].smartSizeHint() - w;
		} else if ( w > chain[i].maximumSize ) {
		    surplus += w - chain[i].maximumSize;
		}
	    }
	    if ( deficit > 0 && surplus <= deficit ) {
		// give to the ones that have too little
		for ( i = start; i < start+count; i++ ) {
		    if ( !chain[i].done &&
			 chain[i].size < chain[i].smartSizeHint() ) {
			chain[i].size = chain[i].smartSizeHint();
			chain[i].done = TRUE;
			space_left -= chain[i].smartSizeHint();
			sumStretch -= chain[i].stretch;
			n--;
		    }
		}
	    }
	    if ( surplus > 0 && surplus >= deficit ) {
		// take from the ones that have too much
		for ( i = start; i < start+count; i++ ) {
		    if ( !chain[i].done &&
			 chain[i].size > chain[i].maximumSize ) {
			chain[i].size = chain[i].maximumSize;
			chain[i].done = TRUE;
			space_left -= chain[i].maximumSize;
			sumStretch -= chain[i].stretch;
			n--;
		    }
		}
	    }
	} while ( n > 0 && surplus != deficit );
	if ( n == 0 )
	    extraspace = space_left;
    }

    /*
      As a last resort, we distribute the unwanted space equally
      among the spacers (counting the start and end of the chain). We
      could, but don't, attempt a sub-pixel allocation of the extra
      space.
    */
    int extra = extraspace / ( spacerCount + 2 );
    int p = pos + extra;
    for ( i = start; i < start+count; i++ ) {
	chain[i].pos = p;
	p = p + chain[i].size;
	if ( !chain[i].empty )
	    p += spacer+extra;
    }
}

Q_EXPORT QSize qSmartMinSize( const QWidgetItem *i )
{
    QWidget *w = ((QWidgetItem *)i)->widget();

    QSize s( 0, 0 );
    if ( w->layout() ) {
	s = w->layout()->totalMinimumSize();
    } else {
	QSize sh;

	if ( w->sizePolicy().horData() != QSizePolicy::Ignored ) {
	    if ( w->sizePolicy().mayShrinkHorizontally() ) {
		s.setWidth( w->minimumSizeHint().width() );
	    } else {
		sh = w->sizeHint();
		s.setWidth( sh.width() );
	    }
	}

	if ( w->sizePolicy().verData() != QSizePolicy::Ignored ) {
	    if ( w->sizePolicy().mayShrinkVertically() ) {
		s.setHeight( w->minimumSizeHint().height() );
	    } else {
		s.setHeight( sh.isValid() ? sh.height()
			     : w->sizeHint().height() );
	    }
	}
    }
    s = s.boundedTo( w->maximumSize() );
    QSize min = w->minimumSize();
    if ( min.width() > 0 )
	s.setWidth( min.width() );
    if ( min.height() > 0 )
	s.setHeight( min.height() );

    if ( i->hasHeightForWidth() && min.height() == 0 && min.width() > 0 )
	s.setHeight( i->heightForWidth(s.width()) );

    s = s.expandedTo( QSize(1, 1) );
    return s;
}

Q_EXPORT QSize qSmartMinSize( QWidget *w )
{
    QWidgetItem item( w );
    return qSmartMinSize( &item );
}

Q_EXPORT QSize qSmartMaxSize( const QWidgetItem *i, int align )
{
    QWidget *w = ( (QWidgetItem*)i )->widget();
    if ( align & Qt::AlignHorizontal_Mask && align & Qt::AlignVertical_Mask )
	return QSize( QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX );
    QSize s = w->maximumSize();
    if ( s.width() == QWIDGETSIZE_MAX && !(align & Qt::AlignHorizontal_Mask) )
	if ( !w->sizePolicy().mayGrowHorizontally() )
	    s.setWidth( w->sizeHint().width() );

    if ( s.height() == QWIDGETSIZE_MAX && !(align & Qt::AlignVertical_Mask) )
	if ( !w->sizePolicy().mayGrowVertically() )
	    s.setHeight( w->sizeHint().height() );

    s = s.expandedTo( w->minimumSize() );

    if ( align & Qt::AlignHorizontal_Mask )
	s.setWidth( QLAYOUTSIZE_MAX );
    if ( align & Qt::AlignVertical_Mask )
	s.setHeight( QLAYOUTSIZE_MAX );
    return s;
}

Q_EXPORT QSize qSmartMaxSize( QWidget *w, int align )
{
    QWidgetItem item( w );
    return qSmartMaxSize( &item, align );
}

#endif // QT_NO_LAYOUT
