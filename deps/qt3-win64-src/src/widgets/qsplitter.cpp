/****************************************************************************
** $Id: qsplitter.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QSplitter class
**
**  Created : 980105
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

#include "qsplitter.h"
#ifndef QT_NO_SPLITTER

#include "qlayout.h"
#include "../kernel/qlayoutengine_p.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qdrawutil.h"
#include "qmemarray.h"
#include "qobjectlist.h"
#include "qpainter.h"
#include "qptrlist.h"
#include "qstyle.h"

class QSplitterHandle : public QWidget
{
    Q_OBJECT
public:
    QSplitterHandle( Orientation o,
		     QSplitter *parent, const char* name=0 );
    void setOrientation( Orientation o );
    Orientation orientation() const { return orient; }

    bool opaque() const { return s->opaqueResize(); }

    QSize sizeHint() const;

    int id() const { return myId; } // d->list.at(id())->wid == this
    void setId( int i ) { myId = i; }

protected:
    void paintEvent( QPaintEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );

private:
    Orientation orient;
    bool opaq;
    int myId;

    QSplitter *s;
};

#include "qsplitter.moc"

const uint Default = 2;

static int mouseOffset;
static int opaqueOldPos = -1; // this assumes that there's only one mouse

static QPoint toggle( QWidget *w, QPoint pos )
{
    QSize minS = qSmartMinSize( w );
    return -pos - QPoint( minS.width(), minS.height() );
}

static bool isCollapsed( QWidget *w )
{
    return w->x() < 0 || w->y() < 0;
}

static QPoint topLeft( QWidget *w )
{
    if ( isCollapsed(w) ) {
	return toggle( w, w->pos() );
    } else {
	return w->pos();
    }
}

static QPoint bottomRight( QWidget *w )
{
    if ( isCollapsed(w) ) {
	return toggle( w, w->pos() ) - QPoint( 1, 1 );
    } else {
	return w->geometry().bottomRight();
    }
}

QSplitterHandle::QSplitterHandle( Orientation o, QSplitter *parent,
				  const char * name )
    : QWidget( parent, name )
{
    s = parent;
    setOrientation( o );
}

QSize QSplitterHandle::sizeHint() const
{
    int hw = s->handleWidth();
    return parentWidget()->style().sizeFromContents( QStyle::CT_Splitter, s,
						     QSize(hw, hw) )
				  .expandedTo( QApplication::globalStrut() );
}

void QSplitterHandle::setOrientation( Orientation o )
{
    orient = o;
#ifndef QT_NO_CURSOR
    setCursor( o == QSplitter::Horizontal ? splitHCursor : splitVCursor );
#endif
}

void QSplitterHandle::mouseMoveEvent( QMouseEvent *e )
{
    if ( !(e->state()&LeftButton) )
	return;
    QCOORD pos = s->pick( parentWidget()->mapFromGlobal(e->globalPos()) )
		 - mouseOffset;
    if ( opaque() ) {
	s->moveSplitter( pos, id() );
    } else {
	s->setRubberband( s->adjustPos(pos, id()) );
    }
}

void QSplitterHandle::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == LeftButton )
	mouseOffset = s->pick( e->pos() );
}

void QSplitterHandle::mouseReleaseEvent( QMouseEvent *e )
{
    if ( !opaque() && e->button() == LeftButton ) {
	QCOORD pos = s->pick( parentWidget()->mapFromGlobal(e->globalPos()) )
		     - mouseOffset;
	s->setRubberband( -1 );
	s->moveSplitter( pos, id() );
    }
}

void QSplitterHandle::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    parentWidget()->style().drawPrimitive( QStyle::PE_Splitter, &p, rect(),
					   colorGroup(),
					   (orientation() == Horizontal ?
					    QStyle::Style_Horizontal : 0) );
}

class QSplitterLayoutStruct : public Qt
{
public:
    QCOORD sizer;
    uint isHandle : 1;
    uint collapsible : 2;
    uint resizeMode : 2;
    QWidget *wid;

    QSplitterLayoutStruct()
	: sizer( -1 ), collapsible( Default ) { }
    QCOORD getSizer( Orientation orient );
};

QCOORD QSplitterLayoutStruct::getSizer( Orientation orient )
{
    if ( sizer == -1 ) {
	QSize s = wid->sizeHint();
	if ( !s.isValid() || wid->testWState(WState_Resized) )
	    s = wid->size();
	sizer = ( orient == Horizontal ) ? s.width() : s.height();
    }
    return sizer;
}

class QSplitterPrivate
{
public:
    QSplitterPrivate()
	: opaque( FALSE ), firstShow( TRUE ), childrenCollapsible( TRUE ),
	  handleWidth( 0 ) { }

    QPtrList<QSplitterLayoutStruct> list;
    bool opaque : 8;
    bool firstShow : 8;
    bool childrenCollapsible : 8;
    int handleWidth;
};


/*!
    \class QSplitter
    \brief The QSplitter class implements a splitter widget.

    \ingroup organizers
    \mainclass

    A splitter lets the user control the size of child widgets by
    dragging the boundary between the children. Any number of widgets
    may be controlled by a single splitter.

    To show a QListBox, a QListView and a QTextEdit side by side:
    \code
	QSplitter *split = new QSplitter( parent );
	QListBox *lb = new QListBox( split );
	QListView *lv = new QListView( split );
	QTextEdit *ed = new QTextEdit( split );
    \endcode

    QSplitter lays out its children horizontally (side by side); you
    can use setOrientation(QSplitter::Vertical) to lay out the
    children vertically.

    By default, all widgets can be as large or as small as the user
    wishes, between the \l minimumSizeHint() (or \l minimumSize())
    and \l maximumSize() of the widgets. Use setResizeMode() to
    specify that a widget should keep its size when the splitter is
    resized, or set the stretch component of the \l sizePolicy.

    Although QSplitter normally resizes the children only at the end
    of a resize operation, if you call setOpaqueResize(TRUE) the
    widgets are resized as often as possible.

    The initial distribution of size between the widgets is determined
    by the initial size of each widget. You can also use setSizes() to
    set the sizes of all the widgets. The function sizes() returns the
    sizes set by the user.

    If you hide() a child its space will be distributed among the
    other children. It will be reinstated when you show() it again. It
    is also possible to reorder the widgets within the splitter using
    moveToFirst() and moveToLast().

    <img src=qsplitter-m.png> <img src=qsplitter-w.png>

    \sa QTabBar
*/


/*!
    Constructs a horizontal splitter with the \a parent and \a name
    arguments being passed on to the QFrame constructor.
*/

QSplitter::QSplitter( QWidget *parent, const char *name )
    : QFrame( parent, name, WPaintUnclipped )
{
    orient = Horizontal;
    init();
}


/*!
    Constructs a splitter with orientation \a o with the \a parent and
    \a name arguments being passed on to the QFrame constructor.
*/

QSplitter::QSplitter( Orientation o, QWidget *parent, const char *name )
    : QFrame( parent, name, WPaintUnclipped )
{
    orient = o;
    init();
}


/*!
    Destroys the splitter and any children.
*/

QSplitter::~QSplitter()
{
    delete d;
}


void QSplitter::init()
{
    d = new QSplitterPrivate;
    d->list.setAutoDelete( TRUE );
    QSizePolicy sp( QSizePolicy::Expanding, QSizePolicy::Preferred );
    if ( orient == Vertical )
	sp.transpose();
    setSizePolicy( sp );
    clearWState( WState_OwnSizePolicy );
}

/*!
    \fn void QSplitter::refresh()

    Updates the splitter's state. You should not need to call this
    function.
*/


/*!
    \property QSplitter::orientation
    \brief the orientation of the splitter

    By default the orientation is horizontal (the widgets are side by
    side). The possible orientations are \c Horizontal and
    \c Vertical.
*/

void QSplitter::setOrientation( Orientation o )
{
    if ( orient == o )
	return;

    if ( !testWState( WState_OwnSizePolicy ) ) {
	QSizePolicy sp = sizePolicy();
	sp.transpose();
	setSizePolicy( sp );
	clearWState( WState_OwnSizePolicy );
    }

    orient = o;

    QSplitterLayoutStruct *s = d->list.first();
    while ( s ) {
	if ( s->isHandle )
	    ((QSplitterHandle*)s->wid)->setOrientation( o );
	s = d->list.next();
    }
    recalc( isVisible() );
}

/*!
    \property QSplitter::childrenCollapsible
    \brief whether child widgets can be resized down to size 0 by the user

    By default, children are collapsible. It is possible to enable
    and disable the collapsing of individual children; see
    setCollapsible().
*/

void QSplitter::setChildrenCollapsible( bool collapse )
{
    d->childrenCollapsible = collapse;
}

bool QSplitter::childrenCollapsible() const
{
    return d->childrenCollapsible;
}

/*!
    Sets whether the child widget \a w is collapsible to \a collapse.

    By default, children are collapsible, meaning that the user can
    resize them down to size 0, even if they have a non-zero
    minimumSize() or minimumSizeHint(). This behavior can be changed
    on a per-widget basis by calling this function, or globally for
    all the widgets in the splitter by setting the \l
    childrenCollapsible property.

    \sa childrenCollapsible
*/

void QSplitter::setCollapsible( QWidget *w, bool collapse )
{
    findWidget( w )->collapsible = collapse ? 1 : 0;
}

/*!
    \reimp
*/
void QSplitter::resizeEvent( QResizeEvent * )
{
    doResize();
}

QSplitterLayoutStruct *QSplitter::findWidget( QWidget *w )
{
    processChildEvents();
    QSplitterLayoutStruct *s = d->list.first();
    while ( s ) {
	if ( s->wid == w )
	    return s;
	s = d->list.next();
    }
    return addWidget( w );
}

/*
    Inserts the widget \a w at the end (or at the beginning if \a
    prepend is TRUE) of the splitter's list of widgets.

    It is the responsibility of the caller to make sure that \a w is
    not already in the splitter and to call recalcId() if needed. (If
    \a prepend is TRUE, then recalcId() is very probably needed.)
*/

QSplitterLayoutStruct *QSplitter::addWidget( QWidget *w, bool prepend )
{
    QSplitterLayoutStruct *s;
    QSplitterHandle *newHandle = 0;
    if ( d->list.count() > 0 ) {
	s = new QSplitterLayoutStruct;
	s->resizeMode = KeepSize;
	QString tmp = "qt_splithandle_";
	tmp += w->name();
	newHandle = new QSplitterHandle( orientation(), this, tmp );
	s->wid = newHandle;
	newHandle->setId( d->list.count() );
	s->isHandle = TRUE;
	s->sizer = pick( newHandle->sizeHint() );
	if ( prepend )
	    d->list.prepend( s );
	else
	    d->list.append( s );
    }
    s = new QSplitterLayoutStruct;
    s->resizeMode = DefaultResizeMode;
    s->wid = w;
    s->isHandle = FALSE;
    if ( prepend )
	d->list.prepend( s );
    else
	d->list.append( s );
    if ( newHandle && isVisible() )
	newHandle->show(); // will trigger sending of post events
    return s;
}


/*!
    Tells the splitter that the child widget described by \a c has
    been inserted or removed.
*/

void QSplitter::childEvent( QChildEvent *c )
{
    if ( c->type() == QEvent::ChildInserted ) {
	if ( !c->child()->isWidgetType() )
	    return;

	if ( ((QWidget*)c->child())->testWFlags( WType_TopLevel ) )
	    return;

	QSplitterLayoutStruct *s = d->list.first();
	while ( s ) {
	    if ( s->wid == c->child() )
		return;
	    s = d->list.next();
	}
	addWidget( (QWidget*)c->child() );
	recalc( isVisible() );
    } else if ( c->type() == QEvent::ChildRemoved ) {
	QSplitterLayoutStruct *prev = 0;
	if ( d->list.count() > 1 )
	    prev = d->list.at( 1 );  // yes, this is correct
	QSplitterLayoutStruct *curr = d->list.first();
	while ( curr ) {
	    if ( curr->wid == c->child() ) {
		d->list.removeRef( curr );
		if ( prev && prev->isHandle ) {
		    QWidget *w = prev->wid;
		    d->list.removeRef( prev );
		    delete w; // will call childEvent()
		}
		recalcId();
		doResize();
		return;
	    }
	    prev = curr;
	    curr = d->list.next();
	}
    }
}


/*!
    Displays a rubber band at position \a p. If \a p is negative, the
    rubber band is removed.
*/

void QSplitter::setRubberband( int p )
{
    QPainter paint( this );
    paint.setPen( gray );
    paint.setBrush( gray );
    paint.setRasterOp( XorROP );
    QRect r = contentsRect();
    const int rBord = 3; // customizable?
    int hw = handleWidth();
    if ( orient == Horizontal ) {
	if ( opaqueOldPos >= 0 )
	    paint.drawRect( opaqueOldPos + hw / 2 - rBord, r.y(),
			    2 * rBord, r.height() );
	if ( p >= 0 )
	    paint.drawRect( p + hw / 2 - rBord, r.y(), 2 * rBord, r.height() );
    } else {
	if ( opaqueOldPos >= 0 )
	    paint.drawRect( r.x(), opaqueOldPos + hw / 2 - rBord,
			    r.width(), 2 * rBord );
	if ( p >= 0 )
	    paint.drawRect( r.x(), p + hw / 2 - rBord, r.width(), 2 * rBord );
    }
    opaqueOldPos = p;
}


/*!
    \reimp
*/

bool QSplitter::event( QEvent *e )
{
    switch ( e->type() ) {
    case QEvent::Show:
	if ( !d->firstShow )
	    break;
	d->firstShow = FALSE;
	// fall through
    case QEvent::LayoutHint:
	recalc( isVisible() );
	break;
    default:
	;
    }
    return QWidget::event( e );
}


/*!
  \obsolete

  Draws the splitter handle in the rectangle described by \a x, \a y,
  \a w, \a h using painter \a p.
  \sa QStyle::drawPrimitive()
*/

// ### Remove this in 4.0

void QSplitter::drawSplitter( QPainter *p,
			      QCOORD x, QCOORD y, QCOORD w, QCOORD h )
{
    style().drawPrimitive(QStyle::PE_Splitter, p, QRect(x, y, w, h), colorGroup(),
			  (orientation() == Horizontal ?
			   QStyle::Style_Horizontal : 0));
}


/*!
    Returns the ID of the widget to the right of or below the widget
    \a w, or 0 if there is no such widget (i.e. it is either not in
    this QSplitter or \a w is at the end).
*/

int QSplitter::idAfter( QWidget* w ) const
{
    QSplitterLayoutStruct *s = d->list.first();
    bool seen_w = FALSE;
    while ( s ) {
	if ( s->isHandle && seen_w )
	    return d->list.at();
	if ( !s->isHandle && s->wid == w )
	    seen_w = TRUE;
	s = d->list.next();
    }
    return 0;
}


/*!
    Moves the left/top edge of the splitter handle with ID \a id as
    close as possible to position \a p, which is the distance from the
    left (or top) edge of the widget.

    For Arabic, Hebrew and other right-to-left languages the layout is
    reversed.  \a p is then the distance from the right (or top) edge
    of the widget.

    \sa idAfter()
*/
void QSplitter::moveSplitter( QCOORD p, int id )
{
    QSplitterLayoutStruct *s = d->list.at( id );
    int farMin;
    int min;
    int max;
    int farMax;

    p = adjustPos( p, id, &farMin, &min, &max, &farMax );
    int oldP = pick( s->wid->pos() );

    if ( QApplication::reverseLayout() && orient == Horizontal ) {
	int q = p + s->wid->width();
	doMove( FALSE, q, id - 1, -1, (q > oldP), (p > max) );
	doMove( TRUE, q, id, -1, (q > oldP), (p < min) );
    } else {
	doMove( FALSE, p, id, +1, (p < oldP), (p > max) );
	doMove( TRUE, p, id - 1, +1, (p < oldP), (p < min) );
    }
    storeSizes();
}


void QSplitter::setGeo( QWidget *w, int p, int s, bool splitterMoved )
{
    QRect r;
    if ( orient == Horizontal ) {
	if ( QApplication::reverseLayout() && orient == Horizontal
	     && !splitterMoved )
	    p = contentsRect().width() - p - s;
	r.setRect( p, contentsRect().y(), s, contentsRect().height() );
    } else {
	r.setRect( contentsRect().x(), p, contentsRect().width(), s );
    }

    /*
      Hide the child widget, but without calling hide() so that the
      splitter handle is still shown.
    */
    if ( !w->isHidden() && s <= 0 && pick(qSmartMinSize(w)) > 0 )
	r.moveTopLeft( toggle(w, r.topLeft()) );
    w->setGeometry( r );
}


void QSplitter::doMove( bool backwards, int pos, int id, int delta, bool upLeft,
			bool mayCollapse )
{
    if ( id < 0 || id >= (int) d->list.count() )
	return;

    QSplitterLayoutStruct *s = d->list.at( id );
    QWidget *w = s->wid;

    int nextId = backwards ? id - delta : id + delta;

    if ( w->isHidden() ) {
	doMove( backwards, pos, nextId, delta, upLeft, TRUE );
    } else {
	if ( s->isHandle ) {
	    int dd = s->getSizer( orient );
	    int nextPos = backwards ? pos - dd : pos + dd;
	    int left = backwards ? pos - dd : pos;
	    setGeo( w, left, dd, TRUE );
	    doMove( backwards, nextPos, nextId, delta, upLeft, mayCollapse );
	} else {
	    int dd = backwards ? pos - pick( topLeft(w) )
			       : pick( bottomRight(w) ) - pos + 1;
	    if ( dd > 0 || (!isCollapsed(w) && !mayCollapse) ) {
		dd = QMAX( pick(qSmartMinSize(w)),
			   QMIN(dd, pick(w->maximumSize())) );
	    } else {
		dd = 0;
	    }
	    setGeo( w, backwards ? pos - dd : pos, dd, TRUE );
	    doMove( backwards, backwards ? pos - dd : pos + dd, nextId, delta,
		    upLeft, TRUE );
	}
    }
}

int QSplitter::findWidgetJustBeforeOrJustAfter( int id, int delta, int &collapsibleSize )
{
    id += delta;
    do {
	QWidget *w = d->list.at( id )->wid;
	if ( !w->isHidden() ) {
            if ( collapsible(d->list.at(id)) )
                collapsibleSize = pick( qSmartMinSize(w) );
	    return id;
	}
	id += 2 * delta; // go to previous (or next) widget, skip the handle
    } while ( id >= 0 && id < (int)d->list.count() );

    return -1;
}

void QSplitter::getRange( int id, int *farMin, int *min, int *max, int *farMax )
{
    int n = d->list.count();
    if ( id <= 0 || id >= n - 1 )
	return;

    int collapsibleSizeBefore = 0;
    int idJustBefore = findWidgetJustBeforeOrJustAfter( id, -1, collapsibleSizeBefore );

    int collapsibleSizeAfter = 0;
    int idJustAfter = findWidgetJustBeforeOrJustAfter( id, +1, collapsibleSizeAfter );

    int minBefore = 0;
    int minAfter = 0;
    int maxBefore = 0;
    int maxAfter = 0;
    int i;

    for ( i = 0; i < id; i++ )
	addContribution( i, &minBefore, &maxBefore, i == idJustBefore );
    for ( i = id; i < n; i++ )
	addContribution( i, &minAfter, &maxAfter, i == idJustAfter );

    QRect r = contentsRect();
    int farMinVal;
    int minVal;
    int maxVal;
    int farMaxVal;

    int smartMinBefore = QMAX( minBefore, pick(r.size()) - maxAfter );
    int smartMaxBefore = QMIN( maxBefore, pick(r.size()) - minAfter );

    if ( orient == Vertical || !QApplication::reverseLayout() ) {
	minVal = pick( r.topLeft() ) + smartMinBefore;
	maxVal = pick( r.topLeft() ) + smartMaxBefore;

	farMinVal = minVal;
	if ( minBefore - collapsibleSizeBefore >= pick(r.size()) - maxAfter )
	    farMinVal -= collapsibleSizeBefore;
	farMaxVal = maxVal;
	if ( pick(r.size()) - (minAfter - collapsibleSizeAfter) <= maxBefore )
	    farMaxVal += collapsibleSizeAfter;
    } else {
	int hw = handleWidth();
	minVal = r.width() - smartMaxBefore - hw;
	maxVal = r.width() - smartMinBefore - hw;

	farMinVal = minVal;
	if ( pick(r.size()) - (minAfter - collapsibleSizeAfter) <= maxBefore )
	    farMinVal -= collapsibleSizeAfter;
	farMaxVal = maxVal;
	if ( minBefore - collapsibleSizeBefore >= pick(r.size()) - maxAfter )
	    farMaxVal += collapsibleSizeBefore;
    }

    if ( farMin )
	*farMin = farMinVal;
    if ( min )
	*min = minVal;
    if ( max )
	*max = maxVal;
    if ( farMax )
	*farMax = farMaxVal;
}

/*!
    Returns the valid range of the splitter with ID \a id in \a *min
    and \a *max if \a min and \a max are not 0.

    \sa idAfter()
*/

void QSplitter::getRange( int id, int *min, int *max )
{
    getRange( id, min, 0, 0, max );
}


/*!
    Returns the closest legal position to \a pos of the widget with ID
    \a id.

    \sa idAfter()
*/

int QSplitter::adjustPos( int pos, int id )
{
    int x, i, n, u;
    return adjustPos( pos, id, &u, &n, &i, &x );
}

int QSplitter::adjustPos( int pos, int id, int *farMin, int *min, int *max,
			  int *farMax )
{
    const int Threshold = 40;

    getRange( id, farMin, min, max, farMax );

    if ( pos >= *min ) {
	if ( pos <= *max ) {
	    return pos;
	} else {
	    int delta = pos - *max;
	    int width = *farMax - *max;

	    if ( delta > width / 2 && delta >= QMIN(Threshold, width) ) {
		return *farMax;
	    } else {
		return *max;
	    }
	}
    } else {
	int delta = *min - pos;
	int width = *min - *farMin;

	if ( delta > width / 2 && delta >= QMIN(Threshold, width) ) {
	    return *farMin;
	} else {
	    return *min;
	}
    }
}

bool QSplitter::collapsible( QSplitterLayoutStruct *s )
{
    if (pick(qSmartMinSize(s->wid)) == 1)
        return FALSE;
    if ( s->collapsible != Default ) {
	return (bool) s->collapsible;
    } else {
	return d->childrenCollapsible;
    }
}

void QSplitter::doResize()
{
    QRect r = contentsRect();
    int n = d->list.count();
    QMemArray<QLayoutStruct> a( n );

    for ( int pass = 0; pass < 2; pass++ ) {
	int numAutoWithStretch = 0;
	int numAutoWithoutStretch = 0;

	for ( int i = 0; i < n; i++ ) {
	    a[i].init();
	    QSplitterLayoutStruct *s = d->list.at( i );
	    if ( s->wid->isHidden() || isCollapsed(s->wid) ) {
		a[i].maximumSize = 0;
	    } else if ( s->isHandle ) {
		a[i].sizeHint = a[i].minimumSize = a[i].maximumSize = s->sizer;
		a[i].empty = FALSE;
	    } else {
		int mode = s->resizeMode;
		int stretch = 1;

		if ( mode == DefaultResizeMode ) {
		    QSizePolicy p = s->wid->sizePolicy();
		    int sizePolicyStretch =
			    pick( QSize(p.horStretch(), p.verStretch()) );
		    if ( sizePolicyStretch > 0 ) {
			mode = Stretch;
			stretch = sizePolicyStretch;
			numAutoWithStretch++;
		    } else {
			/*
			  Do things differently on the second pass,
			  if there's one. A second pass is necessary
			  if it was found out during the first pass
			  that all DefaultResizeMode items are
			  KeepSize items. In that case, we make them
			  all Stretch items instead, for a more Qt
			  3.0-compatible behavior.
			*/
			mode = ( pass == 0 ) ? KeepSize : Stretch;
			numAutoWithoutStretch++;
		    }
		}

		a[i].minimumSize = pick( qSmartMinSize(s->wid) );
		a[i].maximumSize = pick( s->wid->maximumSize() );
		a[i].empty = FALSE;

		if ( mode == Stretch ) {
		    if ( s->getSizer(orient) > 1 )
			stretch *= s->getSizer( orient );
		    // QMIN(): ad hoc work-around for layout engine limitation
		    a[i].stretch = QMIN( stretch, 8192 );
		    a[i].sizeHint = a[i].minimumSize;
		} else if ( mode == KeepSize ) {
		    a[i].sizeHint = s->getSizer( orient );
		} else { // mode == FollowSizeHint
		    a[i].sizeHint = pick( s->wid->sizeHint() );
		}
	    }
	}

	// a second pass would yield the same results
	if ( numAutoWithStretch > 0 || numAutoWithoutStretch == 0 )
	    break;
    }

    qGeomCalc( a, 0, n, pick( r.topLeft() ), pick( r.size() ), 0 );

    for ( int i = 0; i < n; i++ ) {
	QSplitterLayoutStruct *s = d->list.at(i);
	setGeo( s->wid, a[i].pos, a[i].size, FALSE );
    }
}

void QSplitter::recalc( bool update )
{
    int fi = 2 * frameWidth();
    int maxl = fi;
    int minl = fi;
    int maxt = QWIDGETSIZE_MAX;
    int mint = fi;
    int n = d->list.count();
    bool first = TRUE;

    /*
      Splitter handles before the first visible widget or right
      before a hidden widget must be hidden.
    */
    for ( int i = 0; i < n; i++ ) {
	QSplitterLayoutStruct *s = d->list.at( i );
	if ( !s->isHandle ) {
	    QSplitterLayoutStruct *p = 0;
	    if ( i > 0 )
		p = d->list.at( i - 1 );

	    // may trigger new recalc
	    if ( p && p->isHandle )
		p->wid->setHidden( first || s->wid->isHidden() );

	    if ( !s->wid->isHidden() )
		first = FALSE;
	}
    }

    bool empty = TRUE;
    for ( int j = 0; j < n; j++ ) {
	QSplitterLayoutStruct *s = d->list.at( j );
	if ( !s->wid->isHidden() ) {
	    empty = FALSE;
	    if ( s->isHandle ) {
		minl += s->getSizer( orient );
		maxl += s->getSizer( orient );
	    } else {
		QSize minS = qSmartMinSize( s->wid );
		minl += pick( minS );
		maxl += pick( s->wid->maximumSize() );
		mint = QMAX( mint, trans(minS) );
		int tm = trans( s->wid->maximumSize() );
		if ( tm > 0 )
		    maxt = QMIN( maxt, tm );
	    }
	}
    }
    if ( empty ) {
	if ( ::qt_cast<QSplitter*>(parentWidget()) ) {
	    // nested splitters; be nice
	    maxl = maxt = 0;
	} else {
	    // QSplitter with no children yet
	    maxl = QWIDGETSIZE_MAX;
	}
    } else {
	maxl = QMIN( maxl, QWIDGETSIZE_MAX );
    }
    if ( maxt < mint )
	maxt = mint;

    if ( orient == Horizontal ) {
	setMaximumSize( maxl, maxt );
	setMinimumSize( minl, mint );
    } else {
	setMaximumSize( maxt, maxl );
	setMinimumSize( mint, minl );
    }
    if ( update )
	doResize();
    else
	d->firstShow = TRUE;
}

/*!
    \enum QSplitter::ResizeMode

    This enum type describes how QSplitter will resize each of its
    child widgets.

    \value Auto  The widget will be resized according to the stretch
    factors set in its sizePolicy().

    \value Stretch  The widget will be resized when the splitter
    itself is resized.

    \value KeepSize  QSplitter will try to keep the widget's size
    unchanged.

    \value FollowSizeHint  QSplitter will resize the widget when the
    widget's size hint changes.
*/

/*!
    Sets resize mode of widget \a w to \a mode. (The default is \c
    Auto.)
*/

void QSplitter::setResizeMode( QWidget *w, ResizeMode mode )
{
    findWidget( w )->resizeMode = mode;
}


/*!
    \property QSplitter::opaqueResize
    \brief whether resizing is opaque

    Opaque resizing is off by default.
*/

bool QSplitter::opaqueResize() const
{
    return d->opaque;
}


void QSplitter::setOpaqueResize( bool on )
{
    d->opaque = on;
}


/*!
    Moves widget \a w to the leftmost/top position.
*/

void QSplitter::moveToFirst( QWidget *w )
{
    processChildEvents();
    bool found = FALSE;
    QSplitterLayoutStruct *s = d->list.first();
    while ( s ) {
	if ( s->wid == w ) {
	    found = TRUE;
	    QSplitterLayoutStruct *p = d->list.prev();
	    if ( p ) { // not already at first place
		d->list.take(); // take p
		d->list.take(); // take s
		d->list.prepend( p );
		d->list.prepend( s );
	    }
	    break;
	}
	s = d->list.next();
    }
    if ( !found )
	addWidget( w, TRUE );
    recalcId();
}


/*!
    Moves widget \a w to the rightmost/bottom position.
*/

void QSplitter::moveToLast( QWidget *w )
{
    processChildEvents();
    bool found = FALSE;
    QSplitterLayoutStruct *s = d->list.first();
    while ( s ) {
	if ( s->wid == w ) {
	    found = TRUE;
	    d->list.take(); // take s
	    QSplitterLayoutStruct *p = d->list.current();
	    if ( p ) { // the splitter handle after s
		d->list.take(); // take p
		d->list.append( p );
	    }
	    d->list.append( s );
	    break;
	}
	s = d->list.next();
    }
    if ( !found )
	addWidget( w );
    recalcId();
}


void QSplitter::recalcId()
{
    int n = d->list.count();
    for ( int i = 0; i < n; i++ ) {
	QSplitterLayoutStruct *s = d->list.at( i );
	if ( s->isHandle )
	    ((QSplitterHandle*)s->wid)->setId( i );
    }
}


/*!
    \reimp
*/
QSize QSplitter::sizeHint() const
{
    constPolish();
    int l = 0;
    int t = 0;
    if ( children() ) {
	const QObjectList * c = children();
	QObjectListIt it( *c );
	QObject * o;

	while( (o = it.current()) != 0 ) {
	    ++it;
	    if ( o->isWidgetType() && !((QWidget*)o)->isHidden() ) {
		QSize s = ((QWidget*)o)->sizeHint();
		if ( s.isValid() ) {
		    l += pick( s );
		    t = QMAX( t, trans( s ) );
		}
	    }
	}
    }
    return orientation() == Horizontal ? QSize( l, t ) : QSize( t, l );
}


/*!
    \reimp
*/

QSize QSplitter::minimumSizeHint() const
{
    constPolish();
    int l = 0;
    int t = 0;
    if ( children() ) {
	const QObjectList * c = children();
	QObjectListIt it( *c );
	QObject * o;

	while ( (o = it.current()) != 0 ) {
	    ++it;
	    if ( o->isWidgetType() && !((QWidget*)o)->isHidden() ) {
		QSize s = qSmartMinSize( (QWidget*)o );
		if ( s.isValid() ) {
		    l += pick( s );
		    t = QMAX( t, trans( s ) );
		}
	    }
	}
    }
    return orientation() == Horizontal ? QSize( l, t ) : QSize( t, l );
}


void QSplitter::storeSizes()
{
    QSplitterLayoutStruct *s = d->list.first();
    while ( s ) {
	if ( !s->isHandle )
	    s->sizer = pick( s->wid->size() );
	s = d->list.next();
    }
}


void QSplitter::addContribution( int id, int *min, int *max,
				 bool mayCollapse )
{
    QSplitterLayoutStruct *s = d->list.at( id );
    if ( !s->wid->isHidden() ) {
	if ( s->isHandle ) {
	    *min += s->getSizer( orient );
	    *max += s->getSizer( orient );
	} else {
	    if ( mayCollapse || !isCollapsed(s->wid) )
		*min += pick( qSmartMinSize(s->wid) );
	    *max += pick( s->wid->maximumSize() );
	}
    }
}


/*!
    Returns a list of the size parameters of all the widgets in this
    splitter.

    If the splitter's orientation is horizontal, the list is a list of
    widget widths; if the orientation is vertical, the list is a list
    of widget heights.

    Giving the values to another splitter's setSizes() function will
    produce a splitter with the same layout as this one.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QValueList<int> list = mySplitter.sizes();
    QValueList<int>::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

    \sa setSizes()
*/

QValueList<int> QSplitter::sizes() const
{
    if ( !testWState(WState_Polished) )
	constPolish();

    QValueList<int> list;
    QSplitterLayoutStruct *s = d->list.first();
    while ( s ) {
	if ( !s->isHandle )
	    list.append( isCollapsed(s->wid) ? 0 : pick(s->wid->size()));
	s = d->list.next();
    }
    return list;
}

/*!
    Sets the size parameters to the values given in the \a list. If
    the splitter is horizontal, the values set the widths of each
    widget going from left to right. If the splitter is vertical, the
    values set the heights of each widget going from top to bottom.
    Extra values in the \a list are ignored.

    If \a list contains too few values, the result is undefined but
    the program will still be well-behaved.

    Note that the values in \a list should be the height/width that
    the widgets should be resized to.

    \sa sizes()
*/

void QSplitter::setSizes( QValueList<int> list )
{
    processChildEvents();
    QValueList<int>::Iterator it = list.begin();
    QSplitterLayoutStruct *s = d->list.first();
    while ( s && it != list.end() ) {
	if ( !s->isHandle ) {
	    s->sizer = QMAX( *it, 0 );
	    int smartMinSize = pick( qSmartMinSize(s->wid) );
	    // Make sure that we reset the collapsed state.
	    if ( s->sizer == 0 ) {
		if ( collapsible(s) && smartMinSize > 0 ) {
		    s->wid->move( -1, -1 );
		} else {
		    s->sizer = smartMinSize;
		    s->wid->move( 0, 0 );
		}
	    } else {
		if ( s->sizer < smartMinSize )
		    s->sizer = smartMinSize;
		s->wid->move( 0, 0 );
	    }
	    ++it;
	}
	s = d->list.next();
    }
    doResize();
}

/*!
    \property QSplitter::handleWidth
    \brief the width of the splitter handle
*/

int QSplitter::handleWidth() const
{
    if ( d->handleWidth > 0 ) {
	return d->handleWidth;
    } else {
	return style().pixelMetric( QStyle::PM_SplitterWidth, this );
    }
}

void QSplitter::setHandleWidth( int width )
{
    d->handleWidth = width;
    updateHandles();
}

/*!
    Processes all posted child events, ensuring that the internal state of
    the splitter is kept consistent.
*/

void QSplitter::processChildEvents()
{
    QApplication::sendPostedEvents( this, QEvent::ChildInserted );
}

/*!
    \reimp
*/

void QSplitter::styleChange( QStyle& old )
{
    updateHandles();
    QFrame::styleChange( old );
}

void QSplitter::updateHandles()
{
    int hw = handleWidth();
    QSplitterLayoutStruct *s = d->list.first();
    while ( s ) {
	if ( s->isHandle )
	    s->sizer = hw;
	s = d->list.next();
    }
    recalc( isVisible() );
}

#ifndef QT_NO_TEXTSTREAM
/*!
    \relates QSplitter

    Writes the sizes and the hidden state of the widgets in the
    splitter \a splitter to the text stream \a ts.

    \sa operator>>(), sizes(), QWidget::isHidden()
*/

QTextStream& operator<<( QTextStream& ts, const QSplitter& splitter )
{
    QSplitterLayoutStruct *s = splitter.d->list.first();
    bool first = TRUE;
    ts << "[";

    while ( s != 0 ) {
	if ( !s->isHandle ) {
	    if ( !first )
		ts << ",";

	    if ( s->wid->isHidden() ) {
		ts << "H";
	    } else if ( isCollapsed(s->wid) ) {
		ts << 0;
	    } else {
		ts << s->getSizer( splitter.orientation() );
	    }
	    first = FALSE;
	}
	s = splitter.d->list.next();
    }
    ts << "]" << endl;
    return ts;
}

/*!
    \relates QSplitter

    Reads the sizes and the hidden state of the widgets in the
    splitter \a splitter from the text stream \a ts. The sizes must
    have been previously written by the operator<<() function.

    \sa operator<<(), setSizes(), QWidget::hide()
*/

QTextStream& operator>>( QTextStream& ts, QSplitter& splitter )
{
#undef SKIP_SPACES
#define SKIP_SPACES() \
    while ( line[i].isSpace() ) \
	i++

    splitter.processChildEvents();
    QSplitterLayoutStruct *s = splitter.d->list.first();
    QString line = ts.readLine();
    int i = 0;

    SKIP_SPACES();
    if ( line[i] == '[' ) {
	i++;
	SKIP_SPACES();
	while ( line[i] != ']' ) {
	    while ( s != 0 && s->isHandle )
		s = splitter.d->list.next();
	    if ( s == 0 )
		break;

	    if ( line[i].upper() == 'H' ) {
		s->wid->hide();
		i++;
	    } else {
		s->wid->show();
		int dim = 0;
		while ( line[i].digitValue() >= 0 ) {
		    dim *= 10;
		    dim += line[i].digitValue();
		    i++;
		}
		s->sizer = dim;
		if ( dim == 0 )
		    splitter.setGeo( s->wid, 0, 0, FALSE );
	    }
	    SKIP_SPACES();
	    if ( line[i] == ',' ) {
		i++;
	    } else {
		break;
	    }
	    SKIP_SPACES();
	    s = splitter.d->list.next();
	}
    }
    splitter.doResize();
    return ts;
}
#endif

#endif
