/****************************************************************************
** $Id: qheader.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QHeader widget class (table header)
**
** Created : 961105
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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

#include "qheader.h"
#ifndef QT_NO_HEADER
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qbitarray.h"
#include "qptrvector.h"
#include "qapplication.h"
#include "qstyle.h"

class QHeaderData
{
public:
    QHeaderData(int n)
    {
	count = n;
	labels.setAutoDelete( TRUE );
	iconsets.setAutoDelete( TRUE );
	sizes.resize(n);
	positions.resize(n);
	labels.resize(n);
	if ( int( iconsets.size() ) < n )
	    iconsets.resize( n );
	i2s.resize(n);
	s2i.resize(n);
	clicks.resize(n);
	resize.resize(n);
	int p =0;
	for ( int i = 0; i < n; i ++ ) {
	    sizes[i] = 88;
	    i2s[i] = i;
	    s2i[i] = i;
	    positions[i] = p;
	    p += sizes[i];
	}
	clicks_default = TRUE;
	resize_default = TRUE;
	clicks.fill( clicks_default );
	resize.fill( resize_default );
	move = TRUE;
	sortSection = -1;
	sortDirection = TRUE;
	positionsDirty = TRUE;
	lastPos = 0;
	fullSize = -2;
	pos_dirty = FALSE;
	is_a_table_header = FALSE;
	focusIdx = 0;
    }


    QMemArray<QCOORD>	sizes;
    int height; // we abuse the heights as widths for vertical layout
    bool heightDirty;
    QMemArray<QCOORD>	positions; // sorted by index
    QPtrVector<QString>	labels;
    QPtrVector<QIconSet> iconsets;
    QMemArray<int>	        i2s;
    QMemArray<int>	        s2i;

    QBitArray           clicks;
    QBitArray           resize;
    uint move : 1;
    uint clicks_default : 1; // default value for new clicks bits
    uint resize_default : 1; // default value for new resize bits
    uint pos_dirty : 1;
    uint is_a_table_header : 1;
    bool sortDirection;
    bool positionsDirty;
    int sortSection;
    int count;
    int lastPos;
    int fullSize;
    int focusIdx;
    int pressDelta;

    int sectionAt( int pos ) {
	// positions is sorted by index, not by section
	if ( !count )
	    return -1;
	int l = 0;
	int r = count - 1;
	int i = ( (l+r+1) / 2 );
	while ( r - l ) {
	    if ( positions[i] > pos )
		r = i -1;
	    else
		l = i;
	    i = ( (l+r+1) / 2 );
	}
	if ( positions[i] <= pos && pos <= positions[i] + sizes[ i2s[i] ] )
	    return i2s[i];
	return -1;
    }
};


/*!
    \class QHeader qheader.h
    \brief The QHeader class provides a header row or column, e.g. for
    tables and listviews.

    \ingroup advanced

    This class provides a header, e.g. a vertical header to display
    row labels, or a horizontal header to display column labels. It is
    used by QTable and QListView for example.

    A header is composed of one or more \e sections, each of which can
    display a text label and an \link QIconSet iconset\endlink. A sort
    indicator (an arrow) can also be displayed using
    setSortIndicator().

    Sections are added with addLabel() and removed with removeLabel().
    The label and iconset are set in addLabel() and can be changed
    later with setLabel(). Use count() to retrieve the number of
    sections in the header.

    The orientation of the header is set with setOrientation(). If
    setStretchEnabled() is TRUE, the sections will expand to take up
    the full width (height for vertical headers) of the header. The
    user can resize the sections manually if setResizeEnabled() is
    TRUE. Call adjustHeaderSize() to have the sections resize to
    occupy the full width (or height).

    A section can be moved with moveSection(). If setMovingEnabled()
    is TRUE (the default)the user may drag a section from one position
    to another. If a section is moved, the index positions at which
    sections were added (with addLabel()), may not be the same after the
    move. You don't have to worry about this in practice because the
    QHeader API works in terms of section numbers, so it doesn't matter
    where a particular section has been moved to.

    If you want the current index position of a section call
    mapToIndex() giving it the section number. (This is the number
    returned by the addLabel() call which created the section.) If you
    want to get the section number of a section at a particular index
    position call mapToSection() giving it the index number.

    Here's an example to clarify mapToSection() and mapToIndex():

    \table
    \header \i41 Index positions
    \row \i 0 \i 1 \i 2 \i 3
    \header \i41 Original section ordering
    \row \i Sect 0 \i Sect 1 \i Sect 2 \i Sect 3
    \header \i41 Ordering after the user moves a section
    \row \i Sect 0 \i Sect 2 \i Sect 3 \i Sect 1
    \endtable

    \table
    \header \i \e k \i mapToSection(\e k) \i mapToIndex(\e k)
    \row \i 0 \i 0 \i 0
    \row \i 1 \i 2 \i 3
    \row \i 2 \i 3 \i 1
    \row \i 3 \i 1 \i 2
    \endtable

    In the example above, if we wanted to find out which section is at
    index position 3 we'd call mapToSection(3) and get a section
    number of 1 since section 1 was moved. Similarly, if we wanted to
    know which index position section 2 occupied we'd call
    mapToIndex(2) and get an index of 1.

    QHeader provides the clicked(), pressed() and released() signals.
    If the user changes the size of a section, the sizeChange() signal
    is emitted. If you want to have a sizeChange() signal emitted
    continuously whilst the user is resizing (rather than just after
    the resizing is finished), use setTracking(). If the user moves a
    section the indexChange() signal is emitted.

    <img src=qheader-m.png> <img src=qheader-w.png>

    \sa QListView QTable
*/



/*!
    Constructs a horizontal header called \a name, with parent \a
    parent.
*/

QHeader::QHeader( QWidget *parent, const char *name )
    : QWidget( parent, name, WStaticContents )
{
    orient = Horizontal;
    init( 0 );
}

/*!
    Constructs a horizontal header called \a name, with \a n sections
    and parent \a parent.
*/

QHeader::QHeader( int n,  QWidget *parent, const char *name )
    : QWidget( parent, name, WStaticContents )
{
    orient = Horizontal;
    init( n );
}

/*!
    Destroys the header and all its sections.
*/

QHeader::~QHeader()
{
    delete d;
    d = 0;
}

/*! \reimp
 */

void QHeader::showEvent( QShowEvent *e )
{
    calculatePositions();
    QWidget::showEvent( e );
}

/*!
    \fn void QHeader::sizeChange( int section, int oldSize, int newSize )

    This signal is emitted when the user has changed the size of a \a
    section from \a oldSize to \a newSize. This signal is typically
    connected to a slot that repaints the table or list that contains
    the header.
*/

/*!
    \fn void QHeader::clicked( int section )

    If isClickEnabled() is TRUE, this signal is emitted when the user
    clicks section \a section.

    \sa pressed(), released()
*/

/*!
    \fn void QHeader::pressed( int section )

    This signal is emitted when the user presses section \a section
    down.

    \sa released()
*/

/*!
    \fn void QHeader::released( int section )

    This signal is emitted when section \a section is released.

    \sa pressed()
*/


/*!
    \fn void QHeader::indexChange( int section, int fromIndex, int toIndex )

    This signal is emitted when the user moves section \a section from
    index position \a fromIndex, to index position \a toIndex.
*/

/*!
  \fn void QHeader::moved( int fromIndex, int toIndex )
  \obsolete

  Use indexChange() instead.

  This signal is emitted when the user has moved the section which
  is displayed at the index \a fromIndex to the index \a toIndex.
*/

/*!
  \fn void QHeader::sectionClicked( int index )
  \obsolete

  Use clicked() instead.

  This signal is emitted when a part of the header is clicked. \a
  index is the index at which the section is displayed.

  In a list view this signal would typically be connected to a slot
  that sorts the specified column (or row).
*/

/*! \fn int QHeader::cellSize( int ) const
  \obsolete

  Use sectionSize() instead.

  Returns the size in pixels of the section that is displayed at
  the index \a i.
*/

/*!
    \fn void QHeader::sectionHandleDoubleClicked( int section )

    This signal is emitted when the user doubleclicks on the edge
    (handle) of section \a section.
*/

/*!
  \obsolete

  Use sectionPos() instead.

  Returns the position in pixels of the section that is displayed at the
  index \a i. The  position is measured from the start of the header.
*/

int QHeader::cellPos( int i ) const
{
    if ( i == count() && i > 0 )
	return  d->positions[i-1] + d->sizes[d->i2s[i-1]]; // compatibility
    return sectionPos( mapToSection(i) );
}


/*!
    \property QHeader::count
    \brief the number of sections in the header
*/

int QHeader::count() const
{
    return d->count;
}


/*!
    \property QHeader::tracking
    \brief whether the sizeChange() signal is emitted continuously

    If tracking is on, the sizeChange() signal is emitted continuously
    while the mouse is moved (i.e. when the header is resized),
    otherwise it is only emitted when the mouse button is released at
    the end of resizing.

    Tracking defaults to FALSE.
*/


/*
    Initializes with \a n columns.
*/
void QHeader::init( int n )
{
    state = Idle;
    cachedPos = 0; // unused
    d = new QHeaderData( n );
    d->height = 0;
    d->heightDirty = TRUE;
    offs = 0;
    if( reverse() )
	offs = d->lastPos - width();
    oldHandleIdx = oldHIdxSize = handleIdx = 0;

    setMouseTracking( TRUE );
    trackingIsOn = FALSE;
    setBackgroundMode( PaletteButton );
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
}

/*!
    \property QHeader::orientation
    \brief the header's orientation

    The orientation is either \c Vertical or \c Horizontal (the
    default).

    Call setOrientation() before adding labels if you don't provide a
    size parameter otherwise the sizes will be incorrect.
*/

void QHeader::setOrientation( Orientation orientation )
{
    if ( orient == orientation )
	return;
    orient = orientation;
    if ( orient == Horizontal )
	setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    else
	setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred ) );
    update();
    updateGeometry();
}


/*
    Paints a rectangle starting at \a p, with length \s.
*/
void QHeader::paintRect( int p, int s )
{
    QPainter paint( this );
    paint.setPen( QPen( black, 1, DotLine ) );
    if ( reverse() )
	paint.drawRect( p - s, 3, s, height() - 5 );
    else if ( orient == Horizontal )
	paint.drawRect( p, 3, s, height() - 5 );
    else
	paint.drawRect( 3, p, height() - 5, s );
}

/*
  Marks the division line at \a idx.
*/
void QHeader::markLine( int idx )
{
    QPainter paint( this );
    paint.setPen( QPen( black, 1, DotLine ) );
    int MARKSIZE = style().pixelMetric( QStyle::PM_HeaderMarkSize );
    int p = pPos( idx );
    int x = p - MARKSIZE/2;
    int y = 2;
    int x2 = p + MARKSIZE/2;
    int y2 = height() - 3;
    if ( orient == Vertical ) {
	int t = x; x = y; y = t;
	t = x2; x2 = y2; y2 = t;
    }

    paint.drawLine( x, y, x2, y );
    paint.drawLine( x, y+1, x2, y+1 );

    paint.drawLine( x, y2, x2, y2 );
    paint.drawLine( x, y2-1, x2, y2-1 );

    paint.drawLine( x, y, x, y2 );
    paint.drawLine( x+1, y, x+1, y2 );

    paint.drawLine( x2, y, x2, y2 );
    paint.drawLine( x2-1, y, x2-1, y2 );
}

/*
  Removes the mark at the division line at \a idx.
*/
void QHeader::unMarkLine( int idx )
{
    if ( idx < 0 )
	return;
    int MARKSIZE = style().pixelMetric( QStyle::PM_HeaderMarkSize );
    int p = pPos( idx );
    int x = p - MARKSIZE/2;
    int y = 2;
    int x2 = p + MARKSIZE/2;
    int y2 = height() - 3;
    if ( orient == Vertical ) {
	int t = x; x = y; y = t;
	t = x2; x2 = y2; y2 = t;
    }
    repaint( x, y, x2-x+1, y2-y+1 );
}

/*! \fn int QHeader::cellAt( int ) const
  \obsolete

  Use sectionAt() instead.

  Returns the index at which the section is displayed, which contains
  \a pos in widget coordinates, or -1 if \a pos is outside the header
  sections.
*/

/*
  Tries to find a line that is not a neighbor of  \c handleIdx.
*/
int QHeader::findLine( int c )
{
    int i = 0;
    if ( c > d->lastPos || (reverse() && c < 0 )) {
	return d->count;
    } else {
	int section = sectionAt( c );
	if ( section < 0 )
	    return handleIdx;
	i = d->s2i[section];
    }
    int MARKSIZE = style().pixelMetric( QStyle::PM_HeaderMarkSize );
    if ( i == handleIdx )
	return i;
    if ( i == handleIdx - 1 &&  pPos( handleIdx ) - c > MARKSIZE/2 )
	return i;
    if ( i == handleIdx + 1 && c - pPos( i ) > MARKSIZE/2 )
	return i + 1;
    if ( c - pPos( i ) > pSize( i ) / 2 )
	return i + 1;
    else
	return i;
}

/*!
    Returns the handle at position \a p, or -1 if there is no handle at \a p.
*/
int QHeader::handleAt(int p)
{
    int section = d->sectionAt( p );
    if ( section >= 0 ) {
	int GripMargin = (bool)d->resize[ section ] ?
	    style().pixelMetric( QStyle::PM_HeaderGripMargin ) : 0;
	int index = d->s2i[section];
	if ( (index > 0 && p < d->positions[index] + GripMargin) ||
	    (p > d->positions[index] + d->sizes[section] - GripMargin) ) {
	    if ( index > 0 && p < d->positions[index]  + GripMargin )
		section = d->i2s[--index];
	    // dont show icon if streaching is enabled it is at the end of the last section
	    if ( d->resize.testBit(section) && (d->fullSize == -2 || index != count() - 1)) {
		return section;
	    }
	}
    }

    return -1;
}

/*!
  \obsolete

  Use moveSection() instead.

  Moves the section that is currently displayed at index \a fromIdx
  to index \a toIdx.
*/

void QHeader::moveCell( int fromIdx, int toIdx )
{
    moveSection( mapToSection(fromIdx), toIdx );
}



/*!
  Move and signal and repaint.
 */

void QHeader::handleColumnMove( int fromIdx, int toIdx )
{
    int s = d->i2s[fromIdx];
    if ( fromIdx < toIdx )
	toIdx++; //Convert to
    QRect r = sRect( fromIdx );
    r |= sRect( toIdx );
    moveSection( s, toIdx );
    update( r );
    emit moved( fromIdx, toIdx );
    emit indexChange( s, fromIdx, toIdx );
}

/*!
  \reimp
*/
void QHeader::keyPressEvent( QKeyEvent *e )
{
    int i = d->focusIdx;
    if ( e->key() == Key_Space ) {
	//don't do it if we're doing something with the mouse
	if ( state == Idle && d->clicks[ d->i2s[d->focusIdx]  ] ) {
	    handleIdx = i;
	    state = Pressed;
	    repaint( sRect( handleIdx ) );
	    emit pressed( d->i2s[i] );
	}
    } else if ( orientation() == Horizontal &&
		(e->key() == Key_Right || e->key() == Key_Left)
		|| orientation() == Vertical &&
		(e->key() == Key_Up || e->key() == Key_Down) ) {
	int dir = e->key() == Key_Right || e->key() == Key_Down ? 1 : -1;
	int s = d->i2s[i];
	if ( e->state() & ControlButton  && d->resize[s] ) {
	    //resize
	    int step = e->state() & ShiftButton ? dir : 10*dir;
	    int c = d->positions[i] + d->sizes[s] +  step;
	    handleColumnResize( i, c, TRUE );
	} else 	if ( e->state() & (AltButton|MetaButton) && d->move ) {
	    //move section
	    int i2 = ( i + count() + dir ) % count();
	    d->focusIdx = i2;
	    handleColumnMove( i, i2 );
	} else {
	    //focus on different section
	    QRect r = sRect( d->focusIdx );
	    d->focusIdx = (d->focusIdx + count() + dir) % count();
	    r |= sRect( d->focusIdx );
	    update( r );
	}
    } else {
	e->ignore();
    }
}

/*!
  \reimp
*/
void QHeader::keyReleaseEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
    case Key_Space:
	//double check that this wasn't started with the mouse
	if ( state == Pressed && handleIdx == d->focusIdx ) {
	    repaint(sRect( handleIdx ), FALSE);
	    int section = d->i2s[d->focusIdx];
	    emit released( section );
	    emit sectionClicked( handleIdx );
	    emit clicked( section );
	    state = Idle;
	    handleIdx = -1;
	}
	break;
    default:
	e->ignore();
    }
}


/*!
  \reimp
*/
void QHeader::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton || state != Idle )
	return;
    oldHIdxSize = handleIdx;
    handleIdx = 0;
    int c = orient == Horizontal ? e->pos().x() : e->pos().y();
    c += offset();
    if ( reverse() )
	c = d->lastPos - c;

    int section = d->sectionAt( c );
    if ( section < 0 )
	return;
    int GripMargin = (bool)d->resize[ section ] ?
	style().pixelMetric( QStyle::PM_HeaderGripMargin ) : 0;
    int index = d->s2i[section];

    if ( (index > 0 && c < d->positions[index] + GripMargin) ||
	 (c > d->positions[index] + d->sizes[section] - GripMargin) ) {
	if ( c < d->positions[index] + GripMargin )
	    handleIdx = index-1;
	else
	    handleIdx = index;
	if ( d->lastPos <= ( orient == Horizontal ? width() :
			     height() ) && d->fullSize != -2 && handleIdx == count() - 1 ) {
	    handleIdx = -1;
	    return;
	}
	oldHIdxSize = d->sizes[ d->i2s[handleIdx] ];
	state = d->resize[ d->i2s[handleIdx]  ] ? Sliding : Blocked;
    } else if ( index >= 0 ) {
	oldHandleIdx = handleIdx = index;
	moveToIdx = -1;
	state = d->clicks[ d->i2s[handleIdx]  ] ? Pressed : Blocked;
	clickPos = c;
	repaint( sRect( handleIdx ) );
	if(oldHandleIdx != handleIdx)
	    repaint( sRect( oldHandleIdx ) );
	emit pressed( section );
    }

    d->pressDelta = c - ( d->positions[handleIdx] + d->sizes[ d->i2s[handleIdx] ] );
}

/*!
  \reimp
*/
void QHeader::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;
    int oldOldHandleIdx = oldHandleIdx;
    State oldState = state;
    state = Idle;
    switch ( oldState ) {
    case Pressed: {
	int section = d->i2s[handleIdx];
	emit released( section );
	if ( sRect( handleIdx ).contains( e->pos() ) ) {
	    oldHandleIdx = handleIdx;
	    emit sectionClicked( handleIdx );
	    emit clicked( section );
	} else {
	    handleIdx = oldHandleIdx;
	}
	repaint(sRect( handleIdx ), FALSE);
	if ( oldOldHandleIdx != handleIdx )
	    repaint(sRect(oldOldHandleIdx ), FALSE );
	} break;
    case Sliding: {
	int c = orient == Horizontal ? e->pos().x() : e->pos().y();
	c += offset();
	if ( reverse() )
	    c = d->lastPos - c;
	handleColumnResize( handleIdx, c - d->pressDelta, TRUE );
    } break;
    case Moving: {
#ifndef QT_NO_CURSOR
	unsetCursor();
#endif
	int section = d->i2s[handleIdx];
	if ( handleIdx != moveToIdx && moveToIdx != -1 ) {
	    moveSection( section, moveToIdx );
	    handleIdx = oldHandleIdx;
	    emit moved( handleIdx, moveToIdx );
	    emit indexChange( section, handleIdx, moveToIdx );
	    emit released( section );
	    repaint(); // a bit overkill, but removes the handle as well
	} else {
	    if ( sRect( handleIdx).contains( e->pos() ) ) {
		oldHandleIdx = handleIdx;
		emit released( section );
		emit sectionClicked( handleIdx );
		emit clicked( section );
	    } else {
		handleIdx = oldHandleIdx;
	    }
	    repaint(sRect( handleIdx ), FALSE );
	    if(oldOldHandleIdx != handleIdx)
		repaint(sRect(oldOldHandleIdx ), FALSE );
	}
	break;
    }
    case Blocked:
	//nothing
	break;
    default:
	// empty, probably.  Idle, at any rate.
	break;
    }
}

/*!
  \reimp
*/
void QHeader::mouseMoveEvent( QMouseEvent *e )
{
    int c = orient == Horizontal ? e->pos().x() : e->pos().y();
    c += offset();

    int pos = c;
    if( reverse() )
	c = d->lastPos - c;

    switch( state ) {
    case Idle:
#ifndef QT_NO_CURSOR
	if ( handleAt(c) < 0 )
	    unsetCursor();
	else if ( orient == Horizontal )
	    setCursor( splitHCursor );
	else
	    setCursor( splitVCursor );
#endif
	break;
    case Blocked:
	break;
    case Pressed:
	if ( QABS( c - clickPos ) > 4 && d->move ) {
	    state = Moving;
	    moveToIdx = -1;
#ifndef QT_NO_CURSOR
	    if ( orient == Horizontal )
		setCursor( sizeHorCursor );
	    else
		setCursor( sizeVerCursor );
#endif
	}
	break;
    case Sliding:
	handleColumnResize( handleIdx, c, FALSE, FALSE );
	break;
    case Moving: {
	int newPos = findLine( pos );
	if ( newPos != moveToIdx ) {
	    if ( moveToIdx == handleIdx || moveToIdx == handleIdx + 1 )
		repaint( sRect(handleIdx) );
	    else
		unMarkLine( moveToIdx );
	    moveToIdx = newPos;
	    if ( moveToIdx == handleIdx || moveToIdx == handleIdx + 1 )
		paintRect( pPos( handleIdx ), pSize( handleIdx ) );
	    else
		markLine( moveToIdx );
	}
	break;
    }
    default:
	qWarning( "QHeader::mouseMoveEvent: (%s) unknown state", name() );
	break;
    }
}

/*! \reimp */

void QHeader::mouseDoubleClickEvent( QMouseEvent *e )
{
    int p = orient == Horizontal ? e->pos().x() : e->pos().y();
    p += offset();
    if( reverse() )
	p = d->lastPos - p;

    int header = handleAt(p);
    if (header >= 0)
	emit sectionHandleDoubleClicked( header );
}

/*
  Handles resizing of sections. This means it redraws the relevant parts
  of the header.
*/

void QHeader::handleColumnResize( int index, int c, bool final, bool recalcAll )
{
    int section = d->i2s[index];
    int GripMargin = (bool)d->resize[ section ] ?
	style().pixelMetric( QStyle::PM_HeaderGripMargin ) : 0;
    int lim = d->positions[index] + 2*GripMargin;
    if ( c == lim )
	return;
    if ( c < lim )
	c = lim;
    int oldSize = d->sizes[section];
    int newSize = c - d->positions[index];
    d->sizes[section] = newSize;

    calculatePositions( !recalcAll, !recalcAll ? section : 0 );

    int pos = d->positions[index]-offset();
    if( reverse() ) // repaint the whole thing. Could be optimized (lars)
	repaint( 0, 0, width(), height() );
    else if ( orient == Horizontal )
	repaint( pos, 0, width() - pos, height() );
    else
	repaint( 0, pos, width(), height() - pos );

    int os = 0, ns = 0;
    if ( tracking() && oldSize != newSize ) {
	os = oldSize;
	ns = newSize;
	emit sizeChange( section, oldSize, newSize );
    } else if ( !tracking() && final && oldHIdxSize != newSize ) {
	os = oldHIdxSize;
	ns = newSize;
	emit sizeChange( section, oldHIdxSize, newSize );
    }

    if ( os != ns ) {
	if ( d->fullSize == -1 ) {
	    d->fullSize = count() - 1;
	    adjustHeaderSize();
	    d->fullSize = -1;
	} else if ( d->fullSize >= 0 ) {
	    int old = d->fullSize;
	    d->fullSize = count() - 1;
	    adjustHeaderSize();
	    d->fullSize = old;
	}
    }
}

/*!
    Returns the rectangle covered by the section at index \a index.
*/

QRect QHeader::sRect( int index )
{

    int section = mapToSection( index );
    if ( count() > 0 && index >= count() ) {
	int s = d->positions[count() - 1] - offset() +
		d->sizes[mapToSection(count() - 1)];
	if ( orient == Horizontal )
	    return QRect( s, 0, width() - s + 10, height() );
	else
	    return QRect( 0, s, width(), height() - s + 10 );
    }
    if ( section < 0 )
	return rect(); // ### eeeeevil

    if ( reverse() )
	return QRect(  d->lastPos - d->positions[index] - d->sizes[section] -offset(),
		       0, d->sizes[section], height() );
    else if ( orient == Horizontal )
	return QRect(  d->positions[index]-offset(), 0, d->sizes[section], height() );
    else
	return QRect( 0, d->positions[index]-offset(), width(), d->sizes[section] );
}

/*!
    Returns the rectangle covered by section \a section.
*/

QRect QHeader::sectionRect( int section ) const
{
    int index = mapToIndex( section );
    if ( section < 0 )
	return rect(); // ### eeeeevil

    if ( reverse() )
	return QRect(  d->lastPos - d->positions[index] - d->sizes[section] -offset(),
		       0, d->sizes[section], height() );
    else if ( orient == Horizontal )
	return QRect(  d->positions[index]-offset(), 0, d->sizes[section], height() );
    else
	return QRect( 0, d->positions[index]-offset(), width(), d->sizes[section] );
}

/*!
    \overload

    Sets the icon for section \a section to \a iconset and the text to
    \a s. The section's width is set to \a size if \a size \>= 0;
    otherwise it is left unchanged.

    If the section does not exist, nothing happens.
*/

void QHeader::setLabel( int section, const QIconSet& iconset,
			const QString &s, int size )
{
    if ( section < 0 || section >= count() )
	return;
    d->iconsets.insert( section, new QIconSet( iconset ) );
    setLabel( section, s, size );
}

/*!
    Sets the text of section \a section to \a s. The section's width
    is set to \a size if \a size \>= 0; otherwise it is left
    unchanged. Any icon set that has been set for this section remains
    unchanged.

    If the section does not exist, nothing happens.
*/
void QHeader::setLabel( int section, const QString &s, int size )
{
    if ( section < 0 || section >= count() )
	return;
    if ( s.isNull() )
	d->labels.remove( section );
    else
	d->labels.insert( section, new QString( s ) );

    setSectionSizeAndHeight( section, size );

    if ( isUpdatesEnabled() ) {
	updateGeometry();
	calculatePositions();
	update();
    }
}


bool qt_qheader_label_return_null_strings = FALSE;
/*!
    Returns the text for section \a section. If the section does not
    exist, a QString::null is returned.
*/
QString QHeader::label( int section ) const
{
    if ( section < 0 || section >= count() )
	return QString::null;
    if ( d->labels[ section ] )
	return *( d->labels[ section ] );
    else if ( qt_qheader_label_return_null_strings )
	return QString::null;
    else
	return QString::number( section + 1 );
}

/*!
    Returns the icon set for section \a section. If the section does
    not exist, 0 is returned.
*/

QIconSet *QHeader::iconSet( int section ) const
{
    if ( section < 0 || section >= count() )
	return 0;
    return d->iconsets[ section ];
}


/*!
    \overload

    Adds a new section with iconset \a iconset and label text \a s.
    Returns the index position where the section was added (at the
    right for horizontal headers, at the bottom for vertical headers).
    The section's width is set to \a size, unless size is negative in
    which case the size is calculated taking account of the size of
    the text.
*/
int QHeader::addLabel( const QIconSet& iconset, const QString &s, int size )
{
    int n = count() + 1;
    d->iconsets.resize( n + 1 );
    d->iconsets.insert( n - 1, new QIconSet( iconset ) );
    return addLabel( s, size );
}

/*!
    Removes section \a section. If the section does not exist, nothing
    happens.
*/
void QHeader::removeLabel( int section )
{
    if ( section < 0 || section > count() - 1 )
	return;

    int index = d->s2i[section];
    int n = --d->count;
    int i;
    for ( i = section; i < n; ++i ) {
	d->sizes[i] = d->sizes[i+1];
	d->labels.insert( i, d->labels.take( i + 1 ) );
	d->iconsets.insert( i, d->iconsets.take( i + 1 ) );
    }

    d->sizes.resize( n );
    d->positions.resize( n );
    d->labels.resize( n );
    d->iconsets.resize( n );

    for ( i = section; i < n; ++i )
	d->s2i[i] = d->s2i[i+1];
    d->s2i.resize( n );

    if ( isUpdatesEnabled() ) {
	for ( i = 0; i < n; ++i )
	    if ( d->s2i[i] > index )
		--d->s2i[i];
    }

    for ( i = index; i < n; ++i )
	d->i2s[i] = d->i2s[i+1];
    d->i2s.resize( n );

    if ( isUpdatesEnabled() ) {
	for ( i = 0; i < n; ++i )
	    if ( d->i2s[i] > section )
		--d->i2s[i];
    }

    if ( isUpdatesEnabled() ) {
	updateGeometry();
	calculatePositions();
	update();
    }
}

QSize QHeader::sectionSizeHint( int section, const QFontMetrics& fm ) const
{
    int iw = 0;
    int ih = 0;
    if ( d->iconsets[section] != 0 ) {
	QSize isize = d->iconsets[section]->pixmap( QIconSet::Small,
						    QIconSet::Normal ).size();
	iw = isize.width() + 2;
	ih = isize.height();
    }

    QRect bound;
    QString *label = d->labels[section];
    if ( label ) {
	int lines = label->contains( '\n' ) + 1;
	int w = 0;
        if (lines > 1) {
            bound.setHeight(fm.height() + fm.lineSpacing() * (lines - 1));
            QStringList list = QStringList::split('\n', *label);
            for (int i=0; i <(int)list.count(); ++i) {
                int tmpw = fm.width(*(list.at(i)));
                w = QMAX(w, tmpw);
            }
        } else {
            bound.setHeight(fm.height());
            w = fm.width(*label);
        }
	bound.setWidth( w );
    }
    int arrowWidth = 0;
    if ( d->sortSection == section )
	arrowWidth = ( ( orient == Qt::Horizontal ? height() : width() ) / 2 ) + 8;
    int height = QMAX( bound.height() + 2, ih ) + 4;
    int width = bound.width() + style().pixelMetric( QStyle::PM_HeaderMargin ) * 4
	+ iw + arrowWidth;
    return QSize( width, height );
}

/*
    Sets d->sizes[\a section] to a bounding rect based on its size
    hint and font metrics, but constrained by \a size. It also updates
    d->height.
*/
void QHeader::setSectionSizeAndHeight( int section, int size )
{
    QSize sz = sectionSizeHint( section, fontMetrics() );

    if ( size < 0 ) {
	if ( d->sizes[section] < 0 )
	    d->sizes[section] = ( orient == Horizontal ) ? sz.width()
							 : sz.height();
    } else {
	d->sizes[section] = size;
    }

    int newHeight = ( orient == Horizontal ) ? sz.height() : sz.width();
    if ( newHeight > d->height ) {
	d->height = newHeight;
    } else if ( newHeight < d->height ) {
	/*
	  We could be smarter, but we aren't. This makes a difference
	  only for users with many columns and '\n's in their headers
	  at the same time.
	*/
	d->heightDirty = TRUE;
    }
}

/*!
    Adds a new section with label text \a s. Returns the index
    position where the section was added (at the right for horizontal
    headers, at the bottom for vertical headers). The section's width
    is set to \a size. If \a size \< 0, an appropriate size for the
    text \a s is chosen.
*/
int QHeader::addLabel( const QString &s, int size )
{
    int n = ++d->count;
    if ( (int)d->iconsets.size() < n  )
	d->iconsets.resize( n );
    if ( (int)d->sizes.size() < n  ) {
	d->labels.resize( n );
	d->sizes.resize( n );
	d->positions.resize( n );
	d->i2s.resize( n );
	d->s2i.resize( n );
	d->clicks.resize( n );
	d->resize.resize( n );
    }
    int section = d->count - 1;
    if ( !d->is_a_table_header || !s.isNull() )
	d->labels.insert( section, new QString( s ) );

    if ( size >= 0 && s.isNull() && d->is_a_table_header ) {
	d->sizes[section] = size;
    } else {
	d->sizes[section] = -1;
	setSectionSizeAndHeight( section, size );
    }

    int index = section;
    d->positions[index] = d->lastPos;

    d->s2i[section] = index;
    d->i2s[index] = section;
    d->clicks.setBit( section, d->clicks_default );
    d->resize.setBit( section, d->resize_default );

    if ( isUpdatesEnabled() ) {
	updateGeometry();
	calculatePositions();
	update();
    }
    return index;
}

void QHeader::resizeArrays( int size )
{
    d->iconsets.resize( size );
    d->labels.resize( size );
    d->sizes.resize( size );
    d->positions.resize( size );
    d->i2s.resize( size );
    d->s2i.resize( size );
    d->clicks.resize( size );
    d->resize.resize( size );
}

void QHeader::setIsATableHeader( bool b )
{
    d->is_a_table_header = b;
}

/*! \reimp */
QSize QHeader::sizeHint() const
{
    int width;
    int height;

    constPolish();
    QFontMetrics fm = fontMetrics();

    if ( d->heightDirty ) {
	d->height = fm.lineSpacing() + 6;
	for ( int i = 0; i < count(); i++ ) {
	    int h = orient == Horizontal ?
		    sectionSizeHint( i, fm ).height() : sectionSizeHint( i, fm ).width();
	    d->height = QMAX( d->height, h );
	}
	d->heightDirty = FALSE;
    }

    if ( orient == Horizontal ) {
	height = fm.lineSpacing() + 6;
	width = 0;
	height = QMAX( height, d->height );
	for ( int i = 0; i < count(); i++ )
	    width += d->sizes[i];
    } else {
	width = fm.width( ' ' );
	height = 0;
	width = QMAX( width, d->height );
	for ( int i = 0; i < count(); i++ )
	    height += d->sizes[i];
    }
    return (style().sizeFromContents(QStyle::CT_Header, this,
				     QSize(width, height)).expandedTo(QApplication::globalStrut()));
}

/*!
    \property QHeader::offset
    \brief the header's left-most (or top-most) visible pixel

    Setting this property will scroll the header so that \e offset
    becomes the left-most (or top-most for vertical headers) visible
    pixel.
*/
int QHeader::offset() const
{
    if ( reverse() )
	return d->lastPos - width() - offs;
    return offs;
}

void QHeader::setOffset( int x )
{
    int oldOff = offset();
    offs = x;
    if( d->lastPos < ( orient == Horizontal ? width() : height() ) )
	offs = 0;
    else if ( reverse() )
	offs = d->lastPos - width() - x;
    if ( orient == Horizontal )
	scroll( oldOff-offset(), 0 );
    else
	scroll( 0, oldOff-offset());
}



/*
  Returns the position of actual division line \a i in widget
  coordinates. May return a position outside the widget.

  Note that the last division line is numbered count(). (There is one
  more line than the number of sections).
*/
int QHeader::pPos( int i ) const
{
    int pos;
    if ( i == count() )
	pos = d->lastPos;
    else
	pos = d->positions[i];
    if ( reverse() )
	pos = d->lastPos - pos;
    return pos - offset();
}


/*
  Returns the size of the section at index position \a i.
*/
int QHeader::pSize( int i ) const
{
    return d->sizes[ d->i2s[i] ];
}

/*!
  \obsolete

  Use mapToSection() instead.

  Translates from actual index \a a (index at which the section is displayed)  to
  logical index of the section.  Returns -1 if \a a is outside the legal range.

  \sa mapToActual()
*/

int QHeader::mapToLogical( int a ) const
{
    return mapToSection( a );
}


/*!
  \obsolete

  Use mapToIndex() instead.

  Translates from logical index \a l to actual index (index at which the section \a l is displayed) .
  Returns -1 if \a l is outside the legal range.

  \sa mapToLogical()
*/

int QHeader::mapToActual( int l ) const
{
    return mapToIndex( l );
}


/*!
  \obsolete

  Use resizeSection() instead.

  Sets the size of the section \a section to \a s pixels.

  \warning does not repaint or send out signals
*/

void QHeader::setCellSize( int section, int s )
{
    if ( section < 0 || section >= count() )
	return;
    d->sizes[ section ] = s;
    if ( isUpdatesEnabled() )
	calculatePositions();
}


/*!
    If \a enable is TRUE the user may resize section \a section;
    otherwise the section may not be manually resized.

    If \a section is negative (the default) then the \a enable value
    is set for all existing sections and will be applied to any new
    sections that are added.
    Example:
    \code
    // Allow resizing of all current and future sections
    header->setResizeEnabled(TRUE);
    // Disable resizing of section 3, (the fourth section added)
    header->setResizeEnabled(FALSE, 3);
    \endcode

    If the user resizes a section, a sizeChange() signal is emitted.

    \sa setMovingEnabled() setClickEnabled() setTracking()
*/

void QHeader::setResizeEnabled( bool enable, int section )
{
    if ( section < 0 ) {
	d->resize.fill( enable );
	// and future ones...
	d->resize_default = enable;
    } else if ( section < count() ) {
	d->resize[ section ] = enable;
    }
}


/*!
    \property QHeader::moving
    \brief whether the header sections can be moved

    If this property is TRUE (the default) the user can move sections.
    If the user moves a section the indexChange() signal is emitted.

    \sa setClickEnabled(), setResizeEnabled()
*/

void QHeader::setMovingEnabled( bool enable )
{
    d->move = enable;
}


/*!
    If \a enable is TRUE, any clicks on section \a section will result
    in clicked() signals being emitted; otherwise the section will
    ignore clicks.

    If \a section is -1 (the default) then the \a enable value is set
    for all existing sections and will be applied to any new sections
    that are added.

    \sa setMovingEnabled(), setResizeEnabled()
*/

void QHeader::setClickEnabled( bool enable, int section )
{
    if ( section < 0 ) {
	d->clicks.fill( enable );
	// and future ones...
	d->clicks_default = enable;
    } else if ( section < count() ) {
	d->clicks[ section ] = enable;
    }
}


/*!
    Paints the section at position \a index, inside rectangle \a fr
    (which uses widget coordinates) using painter \a p.

    Calls paintSectionLabel().
*/

void QHeader::paintSection( QPainter *p, int index, const QRect& fr )
{
    int section = mapToSection( index );

    if ( section < 0 ) {
	style().drawPrimitive( QStyle::PE_HeaderSection, p, fr,
			       colorGroup(), QStyle::Style_Raised |
			       (isEnabled() ? QStyle::Style_Enabled : 0) |
			       ( orient == Horizontal ? QStyle::Style_Horizontal : 0 ),
			       QStyleOption( this ) );
	return;
    }

    if ( sectionSize( section ) <= 0 )
	return;

    QStyle::SFlags flags = (orient == Horizontal ? QStyle::Style_Horizontal : QStyle::Style_Default);
    //pass in some hint about the sort indicator if it is used
    if(d->sortSection != section)
	flags |= QStyle::Style_Off;
    else if(!d->sortDirection)
	flags |= QStyle::Style_Up;
    if(isEnabled())
	flags |= QStyle::Style_Enabled;
    if(isClickEnabled(section)) {
	if(section == d->sortSection)
	    flags |= QStyle::Style_Sunken; //currently selected
	if((state == Pressed || state == Moving) && index == handleIdx)
	    flags |= QStyle::Style_Down; //currently pressed

    }
    if(!(flags & QStyle::Style_Down))
	flags |= QStyle::Style_Raised;
    p->setBrushOrigin( fr.topLeft() );
    if ( d->clicks[section] ) {
	style().drawPrimitive( QStyle::PE_HeaderSection, p, fr,
			       colorGroup(), flags,
			       QStyleOption( this ) );
    } else {
	p->save();
	p->setClipRect( fr ); // hack to keep styles working
	if ( orientation() == Horizontal ) {
	    style().drawPrimitive( QStyle::PE_HeaderSection, p,
				   QRect(fr.x() - 2, fr.y() - 2, fr.width() + 4, fr.height() + 4),
				   colorGroup(), flags,
				   QStyleOption( this ) );

	    p->setPen( colorGroup().color( QColorGroup::Mid ) );
	    p->drawLine( fr.x(), fr.y() + fr.height() - 1,
			 fr.x() + fr.width() - 1, fr.y() + fr.height() - 1 );
	    p->drawLine( fr.x() + fr.width() - 1, fr.y(),
			 fr.x() + fr.width() - 1, fr.y() + fr.height() - 1 );
	    p->setPen( colorGroup().color( QColorGroup::Light ) );
	    if ( index > 0 )
		p->drawLine( fr.x(), fr.y(), fr.x(), fr.y() + fr.height() - 1 );
	    if ( index == count() - 1 ) {
		p->drawLine( fr.x() + fr.width() - 1, fr.y(),
			     fr.x() + fr.width() - 1, fr.y() + fr.height() - 1 );
		p->setPen( colorGroup().color( QColorGroup::Mid ) );
		p->drawLine( fr.x() + fr.width() - 2, fr.y(),
			     fr.x() + fr.width() - 2, fr.y() + fr.height() - 1 );
	    }
	} else {
	    style().drawPrimitive( QStyle::PE_HeaderSection, p,
				   QRect(fr.x() - 2, fr.y() - 2, fr.width() + 4, fr.height() + 4),
				   colorGroup(), flags,
				   QStyleOption( this ) );

	    p->setPen( colorGroup().color( QColorGroup::Mid ) );
	    p->drawLine( fr.x() + width() - 1, fr.y(),
			 fr.x() + fr.width() - 1, fr.y() + fr.height() - 1 );
	    p->drawLine( fr.x(), fr.y() + fr.height() - 1,
			 fr.x() + fr.width() - 1, fr.y() + fr.height() - 1 );
	    p->setPen( colorGroup().color( QColorGroup::Light ) );
	    if ( index > 0 )
		p->drawLine( fr.x(), fr.y(), fr.x() + fr.width() - 1, fr.y() );
	    if ( index == count() - 1 ) {
		p->drawLine( fr.x(), fr.y() + fr.height() - 1,
			     fr.x() + fr.width() - 1, fr.y() + fr.height() - 1 );
		p->setPen( colorGroup().color( QColorGroup::Mid ) );
		p->drawLine( fr.x(), fr.y() + fr.height() - 2,
			     fr.x() + fr.width() - 1, fr.y() + fr.height() - 2 );
	    }
	}
	p->restore();
    }

    paintSectionLabel( p, index, fr );
}

/*!
    Paints the label of the section at position \a index, inside
    rectangle \a fr (which uses widget coordinates) using painter \a
    p.

    Called by paintSection()
*/
void QHeader::paintSectionLabel( QPainter *p, int index, const QRect& fr )
{
    int section = mapToSection( index );
    if ( section < 0 )
	return;

    int dx = 0, dy = 0;
    QStyle::SFlags flags = QStyle::Style_Default;
    if ( index == handleIdx && ( state == Pressed || state == Moving ) ) {
	dx = style().pixelMetric( QStyle::PM_ButtonShiftHorizontal, this );
	dy = style().pixelMetric( QStyle::PM_ButtonShiftVertical, this );
	flags |= QStyle::Style_Sunken;
    }
    if ( isEnabled() )
	flags |= QStyle::Style_Enabled;


    QRect r( fr.x() + style().pixelMetric( QStyle::PM_HeaderMargin ) + dx, fr.y() + 2 + dy,
	     fr.width() - 6, fr.height() - 4 );

    style().drawControl( QStyle::CE_HeaderLabel, p, this, r, colorGroup(), flags,
			 QStyleOption( section ) );

    int arrowWidth = ( orient == Qt::Horizontal ? height() : width() ) / 2;
    int arrowHeight = fr.height() - 6;
    QSize ssh = sectionSizeHint( section, p->fontMetrics() );
    int tw = ( orient == Qt::Horizontal ? ssh.width() : ssh.height() );
    int ew = 0;

    if ( style().styleHint( QStyle::SH_Header_ArrowAlignment, this ) & AlignRight )
	ew = fr.width() - tw - 8;
    if ( d->sortSection == section && tw <= fr.width() ) {
	if ( reverse() ) {
	    tw = fr.width() - tw;
	    ew = fr.width() - ew - tw;
	}
	QStyle::SFlags flags = QStyle::Style_Default;
	if ( isEnabled() )
	    flags |= QStyle::Style_Enabled;
	if ( d->sortDirection )
	    flags |= QStyle::Style_Down;
	else
	    flags |= QStyle::Style_Up;
        QRect ar(fr.x() + tw - arrowWidth - 6 + ew, 4, arrowWidth, arrowHeight);
        if (label(section).isRightToLeft())
            ar.moveBy( 2*(fr.right() - ar.right()) + ar.width() - fr.width(), 0 );
	style().drawPrimitive( QStyle::PE_HeaderArrow, p,
			       ar, colorGroup(), flags, QStyleOption( this ) );
    }
}


/*! \reimp */
void QHeader::paintEvent( QPaintEvent *e )
{
    QPainter p( this );
    p.setPen( colorGroup().buttonText() );
    int pos = orient == Horizontal ? e->rect().left() : e->rect().top();
    int id = mapToIndex( sectionAt( pos + offset() ) );
    if ( id < 0 ) {
	if ( pos > 0 )
	    id = d->count;
	else if ( reverse() )
	    id = d->count - 1;
	else
	    id = 0;
    }
    if ( reverse() ) {
	for ( int i = id; i >= 0; i-- ) {
	    QRect r = sRect( i );
	    paintSection( &p, i, r );
	    if ( r.right() >= e->rect().right() )
		return;
	}
    } else {
	if ( count() > 0 ) {
	    for ( int i = id; i <= count(); i++ ) {
		QRect r = sRect( i );
		/*
		  If the last section is clickable (and thus is
		  painted raised), draw the virtual section count()
		  as well. Otherwise it looks ugly.
		*/
		if ( i < count() || d->clicks[ mapToSection( count() - 1 ) ] )
		    paintSection( &p, i, r );
		if ( hasFocus() && d->focusIdx == i ) {
		    QRect fr( r.x()+2, r.y()+2, r.width()-4, r.height()-4 );
		    style().drawPrimitive( QStyle::PE_FocusRect, &p, fr,
					   colorGroup() );
		}
		if ( orient == Horizontal && r. right() >= e->rect().right() ||
		     orient == Vertical && r. bottom() >= e->rect().bottom() )
		    return;
	    }
	}
    }
}

/*! \overload
  \obsolete
  Use the other overload instead.
*/

void QHeader::setSortIndicator( int section, bool ascending )
{
    d->sortSection = section;
    if ( section != -1 )
 	oldHandleIdx = section;
    d->sortDirection = ascending;
    update();
    updateGeometry();
}

/*!
  \fn void QHeader::setSortIndicator(int section, SortOrder order)

  Sets a sort indicator onto the specified \a section. The indicator's
  \a order is either Ascending or Descending.

  Only one section can show a sort indicator at any one time. If you
  don't want any section to show a sort indicator pass a \a section
  number of -1.

  \sa sortIndicatorSection(), sortIndicatorOrder()
*/

/*!
    Returns the section showing the sort indicator or -1 if there is no sort indicator.

    \sa setSortIndicator(), sortIndicatorOrder()
*/

int QHeader::sortIndicatorSection() const
{
    return d->sortSection;
}

/*!
    Returns the implied sort order of the QHeaders sort indicator.

    \sa setSortIndicator(), sortIndicatorSection()
*/

Qt::SortOrder QHeader::sortIndicatorOrder() const
{
    return d->sortDirection ? Ascending : Descending;
}

/*!
    Resizes section \a section to \a s pixels wide (or high).
*/

void QHeader::resizeSection( int section, int s )
{
    setCellSize( section, s );
    update();
}

/*!
    Returns the width (or height) of the \a section in pixels.
*/

int QHeader::sectionSize( int section ) const
{
    if ( section < 0 || section >= count() )
	return 0;
    return d->sizes[section];
}

/*!
    Returns the position (in pixels) at which the \a section starts.

    \sa offset()
*/

int QHeader::sectionPos( int section ) const
{
    if ( d->positionsDirty )
	((QHeader *)this)->calculatePositions();
    if ( section < 0 || section >= count()  )
	return 0;
    return d->positions[ d->s2i[section] ];
}

/*!
    Returns the index of the section which contains the position \a
    pos given in pixels from the left (or top).

    \sa offset()
*/

int QHeader::sectionAt( int pos ) const
{
    if ( reverse() )
	pos = d->lastPos - pos;
    return d->sectionAt( pos );
}

/*!
    Returns the number of the section that corresponds to the specified \a index.

    \warning If QTable is used to move header sections as a result of user
    interaction, the mapping exposed by this function will not reflect the
    order of the headers in the table; i.e., QTable does not call moveSection()
    to move sections but handles move operations internally.

    \sa mapToIndex()
*/

int QHeader::mapToSection( int index ) const
{
    return ( index >= 0 && index < count() ) ? d->i2s[ index ] : -1;
}

/*!
    Returns the index position corresponding to the specified \a section number.

    \warning If QTable is used to move header sections as a result of user
    interaction, the mapping exposed by this function will not reflect the
    order of the headers in the table; i.e., QTable does not call moveSection()
    to move sections but handles move operations internally.

    \sa mapToSection()
*/

int QHeader::mapToIndex( int section ) const
{
    return ( section >= 0 && section < count() ) ? d->s2i[ section ] : -1;
}

/*!
    Moves section \a section to index position \a toIndex.
*/

void QHeader::moveSection( int section, int toIndex )
{
    int fromIndex = mapToIndex( section );
    if ( fromIndex == toIndex ||
	 fromIndex < 0 || fromIndex > count() ||
	 toIndex < 0 || toIndex > count() )
	return;
    int i;
    int idx = d->i2s[fromIndex];
    if ( fromIndex < toIndex ) {
	for ( i = fromIndex; i < toIndex - 1; i++ ) {
	    int t;
	    d->i2s[i] = t = d->i2s[i+1];
	    d->s2i[t] = i;
	}
	d->i2s[toIndex-1] = idx;
	d->s2i[idx] = toIndex-1;
    } else {
	for ( i = fromIndex; i > toIndex; i-- ) {
	    int t;
	    d->i2s[i] = t = d->i2s[i-1];
	    d->s2i[t] = i;
	}
	d->i2s[toIndex] = idx;
	d->s2i[idx] = toIndex;
    }
    calculatePositions();
}

/*!
    Returns TRUE if section \a section is clickable; otherwise returns
    FALSE.

    If \a section is out of range (negative or larger than count() -
    1): returns TRUE if all sections are clickable; otherwise returns
    FALSE.

    \sa setClickEnabled()
*/

bool QHeader::isClickEnabled( int section ) const
{
    if ( section >= 0 && section < count() ) {
	return (bool)d->clicks[ section ];
    }

    for ( int i = 0; i < count(); ++i ) {
	if ( !d->clicks[ i ] )
	    return FALSE;
    }
    return TRUE;
}

/*!
    Returns TRUE if section \a section is resizeable; otherwise
    returns FALSE.

    If \a section is -1 then this function applies to all sections,
    i.e. returns TRUE if all sections are resizeable; otherwise
    returns FALSE.

    \sa setResizeEnabled()
*/

bool QHeader::isResizeEnabled( int section ) const
{
    if ( section >= 0 && section < count() ) {
	return (bool)d->resize[ section ];
    }

    for ( int i = 0; i < count();++i ) {
	if ( !d->resize[ i ] )
	    return FALSE;
    }
    return TRUE;
}

bool QHeader::isMovingEnabled() const
{
    return d->move;
}

/*! \reimp */

void QHeader::setUpdatesEnabled( bool enable )
{
    if ( enable )
	calculatePositions();
    QWidget::setUpdatesEnabled( enable );
}


bool QHeader::reverse () const
{
#if 0
    return ( orient == Qt::Horizontal && QApplication::reverseLayout() );
#else
    return FALSE;
#endif
}

/*! \reimp */
void QHeader::resizeEvent( QResizeEvent *e )
{
    if ( e )
	QWidget::resizeEvent( e );

    if( d->lastPos < width() ) {
	    offs = 0;
    }

    if ( e ) {
	adjustHeaderSize( orientation() == Horizontal ?
			  width() - e->oldSize().width() : height() - e->oldSize().height() );
	if ( (orientation() == Horizontal && height() != e->oldSize().height())
	     || (orientation() == Vertical && width() != e->oldSize().width()) )
	    update();
    } else
	adjustHeaderSize();
}

/*!
    \fn void QHeader::adjustHeaderSize()

    Adjusts the size of the sections to fit the size of the header as
    completely as possible. Only sections for which isStretchEnabled()
    is TRUE will be resized.
*/

void QHeader::adjustHeaderSize( int diff )
{
    if ( !count() )
	return;

    // we skip the adjustHeaderSize when trying to resize the last column which is set to stretchable
    if ( d->fullSize == (count() -1) &&
	 (d->lastPos - d->sizes[count() -1]) > ( orient == Horizontal ? width() : height() ) )
  	return;

    if ( d->fullSize >= 0 ) {
	int sec = mapToSection( d->fullSize );
	int lsec = mapToSection( count() - 1 );
	int ns = sectionSize( sec ) +
		 ( orientation() == Horizontal ?
		   width() : height() ) - ( sectionPos( lsec ) + sectionSize( lsec ) );
	int os = sectionSize( sec );
	if ( ns < 20 )
	    ns = 20;
	setCellSize( sec, ns );
	repaint( FALSE );
	emit sizeChange( sec, os, ns );
    } else if ( d->fullSize == -1 ) {
	int df = diff / count();
	int part = orientation() == Horizontal ? width() / count() : height() / count();
	for ( int i = 0; i < count() - 1; ++i ) {
	    int sec = mapToIndex( i );
	    int os = sectionSize( sec );
	    int ns = diff != -1 ? os + df : part;
	    if ( ns < 20 )
		ns = 20;
	    setCellSize( sec, ns );
	    emit sizeChange( sec, os, ns );
	}
	int sec = mapToIndex( count() - 1 );
	int ns = ( orientation() == Horizontal ? width() : height() ) - sectionPos( sec );
	int os = sectionSize( sec );
	if ( ns < 20 )
	    ns = 20;
	setCellSize( sec, ns );
	repaint( FALSE );
	emit sizeChange( sec, os, ns );
    }
}

/*!
    Returns the total width of all the header columns.
*/
int QHeader::headerWidth() const
{
    if ( d->pos_dirty ) {
	( (QHeader*)this )->calculatePositions();
	d->pos_dirty = FALSE;
    }
    return d->lastPos;
}

void QHeader::calculatePositions( bool onlyVisible, int start )
{
    d->positionsDirty = FALSE;
    d->lastPos = count() > 0 ? d->positions[start] : 0;
    for ( int i = start; i < count(); i++ ) {
  	d->positions[i] = d->lastPos;
	d->lastPos += d->sizes[d->i2s[i]];
 	if ( onlyVisible && d->lastPos > offset() +
	     ( orientation() == Horizontal ? width() : height() ) )
 	    break;
    }
    d->pos_dirty = onlyVisible;
}


/*!
    \property QHeader::stretching
    \brief whether the header sections always take up the full width
    (or height) of the header
*/


/*!
    If \a b is TRUE, section \a section will be resized when the
    header is resized, so that the sections take up the full width (or
    height for vertical headers) of the header; otherwise section \a
    section will be set to be unstretchable and will not resize when
    the header is resized.

    If \a section is -1, and if \a b is TRUE, then all sections will
    be resized equally when the header is resized so that they take up
    the full width (or height for vertical headers) of the header;
    otherwise all the sections will be set to be unstretchable and
    will not resize when the header is resized.

    \sa adjustHeaderSize()
*/

void QHeader::setStretchEnabled( bool b, int section )
{
    if ( b )
	d->fullSize = section;
    else
	d->fullSize = -2;
    adjustHeaderSize();
}

bool QHeader::isStretchEnabled() const
{
    return d->fullSize == -1;
}

/*!
    \overload

    Returns TRUE if section \a section will resize to take up the full
    width (or height) of the header; otherwise returns FALSE. If at
    least one section has stretch enabled the sections will always
    take up the full width of the header.

    \sa setStretchEnabled()
*/

bool QHeader::isStretchEnabled( int section ) const
{
    return d->fullSize == section;
}

/*!
  \reimp
*/
void QHeader::fontChange( const QFont &oldFont )
{
    QFontMetrics fm = fontMetrics();
    d->height = ( orient == Horizontal ) ? fm.lineSpacing() + 6 : fm.width( ' ' );
    QWidget::fontChange( oldFont );
}

#endif // QT_NO_HEADER
