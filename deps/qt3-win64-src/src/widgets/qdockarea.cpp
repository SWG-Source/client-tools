/****************************************************************************
** $Id: qdockarea.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of the QDockArea class
**
** Created : 001010
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the workspace module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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

#include "qdockarea.h"

#ifndef QT_NO_MAINWINDOW
#include "qsplitter.h"
#include "qlayout.h"
#include "qptrvector.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qwidgetlist.h"
#include "qmap.h"
#include "qmainwindow.h"

//#define QDOCKAREA_DEBUG

struct Q_EXPORT DockData
{
    DockData() : w( 0 ), rect() {}
    DockData( QDockWindow *dw, const QRect &r ) : w( dw ), rect( r ) {}
    QDockWindow *w;
    QRect rect;

    Q_DUMMY_COMPARISON_OPERATOR( DockData )
};

static int fix_x( QDockWindow* w, int width = -1 ) {
    if ( QApplication::reverseLayout() ) {
	if ( width < 0 )
	    width = w->width();
	return w->parentWidget()->width() - w->x() - width;
    }
    return w->x();
}
static int fix_x( QDockWindow* w, int x, int width = -1 ) {
    if ( QApplication::reverseLayout() ) {
	if ( width < 0 )
	    width = w->width();
	return w->parentWidget()->width() - x - width;
    }
    return x;
}

static QPoint fix_pos( QDockWindow* w ) {
    if ( QApplication::reverseLayout() ) {
	QPoint p = w->pos();
	p.rx() = w->parentWidget()->width() - p.x() - w->width();
	return p;
    }
    return w->pos();
}


void QDockAreaLayout::setGeometry( const QRect &r )
{
    QLayout::setGeometry( r );
    layoutItems( r );
}

QLayoutIterator QDockAreaLayout::iterator()
{
    return 0;
}

QSize QDockAreaLayout::sizeHint() const
{
    if ( !dockWindows || !dockWindows->first() )
	return QSize( 0, 0 );

    if ( dirty ) {
	QDockAreaLayout *that = (QDockAreaLayout *) this;
	that->layoutItems( geometry() );
    }

    int w = 0;
    int h = 0;
    QPtrListIterator<QDockWindow> it( *dockWindows );
    QDockWindow *dw = 0;
    it.toFirst();
    int y = -1;
    int x = -1;
    int ph = 0;
    int pw = 0;
    while ( ( dw = it.current() ) != 0 ) {
	int plush = 0, plusw = 0;
	++it;
	if ( dw->isHidden() )
	    continue;
	if ( hasHeightForWidth() ) {
	    if ( y != dw->y() )
		plush = ph;
	    y = dw->y();
	    ph = dw->height();
	} else {
	    if ( x != dw->x() )
		plusw = pw;
	    x = dw->x();
	    pw = dw->width();
	}
	h = QMAX( h, dw->height() + plush );
	w = QMAX( w, dw->width() + plusw );
    }

    if ( hasHeightForWidth() )
	return QSize( 0, h );
    return QSize( w, 0 );
}

bool QDockAreaLayout::hasHeightForWidth() const
{
    return orient == Horizontal;
}

void QDockAreaLayout::init()
{
    dirty = TRUE;
    cached_width = 0;
    cached_height = 0;
    cached_hfw = -1;
    cached_wfh = -1;
}

void QDockAreaLayout::invalidate()
{
    dirty = TRUE;
    cached_width = 0;
    cached_height = 0;
}

static int start_pos( const QRect &r, Qt::Orientation o )
{
    if ( o == Qt::Horizontal ) {
	return QMAX( 0, r.x() );
    } else {
	return QMAX( 0, r.y() );
    }
}

static void add_size( int s, int &pos, Qt::Orientation o )
{
    if ( o == Qt::Horizontal ) {
	pos += s;
    } else {
	pos += s;
    }
}

static int space_left( const QRect &r, int pos, Qt::Orientation o )
{
    if ( o == Qt::Horizontal ) {
	return ( r.x() + r.width() ) - pos;
    } else {
	return ( r.y() + r.height() ) - pos;
    }
}

static int dock_extent( QDockWindow *w, Qt::Orientation o, int maxsize )
{
    if ( o == Qt::Horizontal )
	return QMIN( maxsize, QMAX( w->sizeHint().width(), w->fixedExtent().width() ) );
    else
	return QMIN( maxsize, QMAX( w->sizeHint().height(), w->fixedExtent().height() ) );
}

static int dock_strut( QDockWindow *w, Qt::Orientation o )
{
    if ( o != Qt::Horizontal ) {
	int wid;
	if ( ( wid = w->fixedExtent().width() ) != -1 )
	    return QMAX( wid, QMAX( w->minimumSize().width(), w->minimumSizeHint().width() ) );
	return QMAX( w->sizeHint().width(), QMAX( w->minimumSize().width(), w->minimumSizeHint().width() ) );
    } else {
	int hei;
	if ( ( hei = w->fixedExtent().height() ) != -1 )
	    return QMAX( hei, QMAX( w->minimumSizeHint().height(), w->minimumSize().height() ) );
	return QMAX( w->sizeHint().height(), QMAX( w->minimumSizeHint().height(), w->minimumSize().height() ) );
    }
}

static void set_geometry( QDockWindow *w, int pos, int sectionpos, int extent, int strut, Qt::Orientation o )
{
    if ( o == Qt::Horizontal )
	w->setGeometry( fix_x( w, pos, extent), sectionpos, extent, strut );
    else
	w->setGeometry( sectionpos, pos, strut, extent );
}

static int size_extent( const QSize &s, Qt::Orientation o, bool swap = FALSE )
{
    return o == Qt::Horizontal ? ( swap ? s.height() : s.width() ) : ( swap ? s.width() :  s.height() );
}

static int point_pos( const QPoint &p, Qt::Orientation o, bool swap = FALSE )
{
    return o == Qt::Horizontal ? ( swap ? p.y() : p.x() ) : ( swap ? p.x() : p.y() );
}

static void shrink_extend( QDockWindow *dw, int &dockExtend, int /*spaceLeft*/, Qt::Orientation o )
{
    QToolBar *tb = ::qt_cast<QToolBar*>(dw);
    if ( o == Qt::Horizontal ) {
	int mw = 0;
	if ( !tb )
	    mw = dw->minimumWidth();
	else
	    mw = dw->sizeHint().width();
	dockExtend = mw;
    } else {
	int mh = 0;
	if ( !tb )
	    mh = dw->minimumHeight();
	else
	    mh = dw->sizeHint().height();
	dockExtend = mh;
    }
}

static void place_line( QValueList<DockData> &lastLine, Qt::Orientation o, int linestrut, int fullextent, int tbstrut, int maxsize, QDockAreaLayout * )
{
    QDockWindow *last = 0;
    QRect lastRect;
    for ( QValueList<DockData>::Iterator it = lastLine.begin(); it != lastLine.end(); ++it ) {
	if ( tbstrut != -1 && ::qt_cast<QToolBar*>((*it).w) )
	    (*it).rect.setHeight( tbstrut );
	if ( !last ) {
	    last = (*it).w;
	    lastRect = (*it).rect;
	    continue;
	}
	if ( !last->isStretchable() ) {
	    int w = QMIN( lastRect.width(), maxsize );
	    set_geometry( last, lastRect.x(), lastRect.y(), w, lastRect.height(), o );
	} else {
	    int w = QMIN( (*it).rect.x() - lastRect.x(), maxsize );
	    set_geometry( last, lastRect.x(), lastRect.y(), w,
			  last->isResizeEnabled() ? linestrut : lastRect.height(), o );
	}
	last = (*it).w;
	lastRect = (*it).rect;
    }
    if ( !last )
	return;
    if ( !last->isStretchable() ) {
	int w = QMIN( lastRect.width(), maxsize );
	set_geometry( last, lastRect.x(), lastRect.y(), w, lastRect.height(), o );
    } else {
	int w = QMIN( fullextent - lastRect.x() - ( o == Qt::Vertical ? 1 : 0 ), maxsize );
	set_geometry( last, lastRect.x(), lastRect.y(), w,
		      last->isResizeEnabled() ? linestrut : lastRect.height(), o );
    }
}


QSize QDockAreaLayout::minimumSize() const
{
    if ( !dockWindows || !dockWindows->first() )
	return QSize( 0, 0 );

    if ( dirty ) {
	QDockAreaLayout *that = (QDockAreaLayout *) this;
	that->layoutItems( geometry() );
    }

    int s = 0;

    QPtrListIterator<QDockWindow> it( *dockWindows );
    QDockWindow *dw = 0;
    while ( ( dw = it.current() ) != 0 ) {
	++it;
	if ( dw->isHidden() )
	    continue;
	s = QMAX( s, dock_strut( dw, orientation() ) );
    }

    return orientation() == Horizontal ? QSize( 0, s ? s+2 : 0 ) :  QSize( s, 0 );
}



int QDockAreaLayout::layoutItems( const QRect &rect, bool testonly )
{
    if ( !dockWindows || !dockWindows->first() )
	return 0;

    dirty = FALSE;

    // some corrections
    QRect r = rect;
    if ( orientation() == Vertical )
	r.setHeight( r.height() - 3 );

    // init
    lines.clear();
    ls.clear();
    QPtrListIterator<QDockWindow> it( *dockWindows );
    QDockWindow *dw = 0;
    int start = start_pos( r, orientation() );
    int pos = start;
    int sectionpos = 0;
    int linestrut = 0;
    QValueList<DockData> lastLine;
    int tbstrut = -1;
    int maxsize = size_extent( rect.size(), orientation() );
    int visibleWindows = 0;

    // go through all widgets in the dock
    while ( ( dw = it.current() ) != 0 ) {
	++it;
	if ( dw->isHidden() )
	    continue;
	++visibleWindows;
	// find position for the widget: This is the maximum of the
	// end of the previous widget and the offset of the widget. If
	// the position + the width of the widget dosn't fit into the
	// dock, try moving it a bit back, if possible.
	int op = pos;
	int dockExtend = dock_extent( dw, orientation(), maxsize );
	if ( !dw->isStretchable() ) {
	    pos = QMAX( pos, dw->offset() );
	    if ( pos + dockExtend > size_extent( r.size(), orientation() ) - 1 )
		pos = QMAX( op, size_extent( r.size(), orientation() ) - 1 - dockExtend );
	}
	if ( !lastLine.isEmpty() && !dw->newLine() && space_left( rect, pos, orientation() ) < dockExtend )
	    shrink_extend( dw, dockExtend, space_left( rect, pos, orientation() ), orientation() );
	// if the current widget doesn't fit into the line anymore and it is not the first widget of the line
	if ( !lastLine.isEmpty() &&
	     ( space_left( rect, pos, orientation() ) < dockExtend || dw->newLine() ) ) {
	    if ( !testonly ) // place the last line, if not in test mode
		place_line( lastLine, orientation(), linestrut, size_extent( r.size(), orientation() ), tbstrut, maxsize, this );
	    // remember the line coordinats of the last line
	    if ( orientation() == Horizontal )
		lines.append( QRect( 0, sectionpos, r.width(), linestrut ) );
	    else
		lines.append( QRect( sectionpos, 0, linestrut, r.height() ) );
	    // do some clearing for the next line
	    lastLine.clear();
	    sectionpos += linestrut;
	    linestrut = 0;
	    pos = start;
	    tbstrut = -1;
	}

	// remeber first widget of a line
	if ( lastLine.isEmpty() ) {
	    ls.append( dw );
	    // try to make the best position
	    int op = pos;
	    if ( !dw->isStretchable() )
		pos = QMAX( pos, dw->offset() );
	    if ( pos + dockExtend > size_extent( r.size(), orientation() ) - 1 )
		pos = QMAX( op, size_extent( r.size(), orientation() ) - 1 - dockExtend );
	}
	// do some calculations and add the remember the rect which the docking widget requires for the placing
	QRect dwRect(pos, sectionpos, dockExtend, dock_strut( dw, orientation()  ) );
	lastLine.append( DockData( dw, dwRect ) );
	if ( ::qt_cast<QToolBar*>(dw) )
	    tbstrut = QMAX( tbstrut, dock_strut( dw, orientation() ) );
	linestrut = QMAX( dock_strut( dw, orientation() ), linestrut );
	add_size( dockExtend, pos, orientation() );
    }

    // if some stuff was not placed/stored yet, do it now
    if ( !testonly )
	place_line( lastLine, orientation(), linestrut, size_extent( r.size(), orientation() ), tbstrut, maxsize, this );
    if ( orientation() == Horizontal )
	lines.append( QRect( 0, sectionpos, r.width(), linestrut ) );
    else
	lines.append( QRect( sectionpos, 0, linestrut, r.height() ) );
    if ( *(--lines.end()) == *(--(--lines.end())) )
	lines.remove( lines.at( lines.count() - 1 ) );

    it.toFirst();
    bool hadResizable = FALSE;
    while ( ( dw = it.current() ) != 0 ) {
	++it;
	if ( !dw->isVisibleTo( parentWidget ) )
	    continue;
	hadResizable = hadResizable || dw->isResizeEnabled();
	dw->updateSplitterVisibility( visibleWindows > 1 ); //!dw->area()->isLastDockWindow( dw ) );
    }
    return sectionpos + linestrut;
}

int QDockAreaLayout::heightForWidth( int w ) const
{
    if ( dockWindows->isEmpty() && parentWidget )
	return parentWidget->minimumHeight();

    if ( cached_width != w ) {
	QDockAreaLayout * mthis = (QDockAreaLayout*)this;
	mthis->cached_width = w;
	int h = mthis->layoutItems( QRect( 0, 0, w, 0 ), TRUE );
	mthis->cached_hfw = h;
	return h;
    }

    return cached_hfw;
}

int QDockAreaLayout::widthForHeight( int h ) const
{
    if ( cached_height != h ) {
	QDockAreaLayout * mthis = (QDockAreaLayout*)this;
	mthis->cached_height = h;
	int w = mthis->layoutItems( QRect( 0, 0, 0, h ), TRUE );
	mthis->cached_wfh = w;
	return w;
    }
    return cached_wfh;
}




/*!
    \class QDockArea qdockarea.h
    \brief The QDockArea class manages and lays out QDockWindows.

    \ingroup application

    A QDockArea is a container which manages a list of
    \l{QDockWindow}s which it lays out within its area. In cooperation
    with the \l{QDockWindow}s it is responsible for the docking and
    undocking of \l{QDockWindow}s and moving them inside the dock
    area. QDockAreas also handle the wrapping of \l{QDockWindow}s to
    fill the available space as compactly as possible. QDockAreas can
    contain QToolBars since QToolBar is a QDockWindow subclass.

    QMainWindow contains four QDockAreas which you can use for your
    QToolBars and QDockWindows, so in most situations you do not need
    to use the QDockArea class directly. Although QMainWindow contains
    support for its own dock areas it isn't convenient for adding new
    QDockAreas. If you need to create your own dock areas we suggest
    that you create a subclass of QWidget and add your QDockAreas to
    your subclass.

    \img qmainwindow-qdockareas.png QMainWindow's QDockAreas

    \target lines
    \e Lines. QDockArea uses the concept of lines. A line is a
    horizontal region which may contain dock windows side-by-side. A
    dock area may have room for more than one line. When dock windows
    are docked into a dock area they are usually added at the right
    hand side of the top-most line that has room (unless manually
    placed by the user). When users move dock windows they may leave
    empty lines or gaps in non-empty lines. Dock windows can be lined
    up to minimize wasted space using the lineUp() function.

    The QDockArea class maintains a position list of all its child
    dock windows. Dock windows are added to a dock area from position
    0 onwards. Dock windows are laid out sequentially in position
    order from left to right, and in the case of multiple lines of
    dock windows, from top to bottom. If a dock window is floated it
    still retains its position since this is where the window will
    return if the user double clicks its caption. A dock window's
    position can be determined with hasDockWindow(). The position can
    be changed with moveDockWindow().

    To dock or undock a dock window use QDockWindow::dock() and
    QDockWindow::undock() respectively. If you want to control which
    dock windows can dock in a dock area use setAcceptDockWindow(). To
    see if a dock area contains a particular dock window use
    \l{hasDockWindow()}; to see how many dock windows a dock area
    contains use count().

    The streaming operators can write the positions of the dock
    windows in the dock area to a QTextStream. The positions can be
    read back later to restore the saved positions.

    Save the positions to a QTextStream:
    \code
    ts << *myDockArea;
    \endcode

    Restore the positions from a QTextStream:
    \code
    ts >> *myDockArea;
    \endcode
*/

/*!
    \property QDockArea::handlePosition
    \brief where the dock window splitter handle is placed in the dock
    area

    The default position is \c Normal.
*/

/*!
    \property QDockArea::orientation
    \brief the dock area's orientation

    There is no default value; the orientation is specified in the
    constructor.
*/

/*!
    \enum QDockArea::HandlePosition

    A dock window has two kinds of handles, the dock window handle
    used for dragging the dock window, and the splitter handle used to
    resize the dock window in relation to other dock windows using a
    splitter. (The splitter handle is only visible for docked
    windows.)

    This enum specifies where the dock window splitter handle is
    placed in the dock area.

    \value Normal The splitter handles of dock windows are placed at
    the right or bottom.

    \value Reverse The splitter handles of dock windows are placed at
    the left or top.
*/

/*!
    Constructs a QDockArea with orientation \a o, HandlePosition \a h,
    parent \a parent and called \a name.
*/

QDockArea::QDockArea( Orientation o, HandlePosition h, QWidget *parent, const char *name )
    : QWidget( parent, name ), orient( o ), layout( 0 ), hPos( h )
{
    dockWindows = new QPtrList<QDockWindow>;
    layout = new QDockAreaLayout( this, o, dockWindows, 0, 0, "toollayout" );
    installEventFilter( this );
}

/*!
    Destroys the dock area and all the dock windows docked in the dock
    area.

    Does not affect any floating dock windows or dock windows in other
    dock areas, even if they first appeared in this dock area.
    Floating dock windows are effectively top level windows and are
    not child windows of the dock area. When a floating dock window is
    docked (dragged into a dock area) its parent becomes the dock
    area.
*/

QDockArea::~QDockArea()
{
    dockWindows->setAutoDelete( TRUE );
    delete dockWindows;
    dockWindows = 0;
}

/*!
    Moves the QDockWindow \a w within the dock area. If \a w is not
    already docked in this area, \a w is docked first. If \a index is
    -1 or larger than the number of docked widgets, \a w is appended
    at the end, otherwise it is inserted at the position \a index.
*/

void QDockArea::moveDockWindow( QDockWindow *w, int index )
{
    invalidateFixedSizes();
    QDockWindow *dockWindow = 0;
    int dockWindowIndex = findDockWindow( w );
    if ( dockWindowIndex == -1 ) {
	dockWindow = w;
	dockWindow->reparent( this, QPoint( 0, 0 ), TRUE );
	w->installEventFilter( this );
	updateLayout();
	setSizePolicy( QSizePolicy( orientation() == Horizontal ? QSizePolicy::Expanding : QSizePolicy::Minimum,
				    orientation() == Vertical ? QSizePolicy::Expanding : QSizePolicy::Minimum ) );
	dockWindows->append( w );
    } else {
        if ( w->parent() != this )
	    w->reparent( this, QPoint( 0, 0 ), TRUE );
        if ( index == - 1 ) {
	    dockWindows->removeRef( w );
	    dockWindows->append( w );
        }
    }

    w->dockArea = this;
    w->curPlace = QDockWindow::InDock;
    w->updateGui();

    if ( index != -1 && index < (int)dockWindows->count() ) {
	dockWindows->removeRef( w );
	dockWindows->insert( index, w );
    }
}

/*!
    Returns TRUE if the dock area contains the dock window \a w;
    otherwise returns FALSE. If \a index is not 0 it will be set as
    follows: if the dock area contains the dock window \a *index is
    set to \a w's index position; otherwise \a *index is set to -1.
*/

bool QDockArea::hasDockWindow( QDockWindow *w, int *index )
{
    int i = dockWindows->findRef( w );
    if ( index )
	*index = i;
    return i != -1;
}

int QDockArea::lineOf( int index )
{
    QPtrList<QDockWindow> lineStarts = layout->lineStarts();
    int i = 0;
    for ( QDockWindow *w = lineStarts.first(); w; w = lineStarts.next(), ++i ) {
	if ( dockWindows->find( w ) >= index )
	    return i;
    }
    return i;
}

/*!
    \overload

    Moves the dock window \a w inside the dock area where \a p is the
    new position (in global screen coordinates), \a r is the suggested
    rectangle of the dock window and \a swap specifies whether or not
    the orientation of the docked widget needs to be changed.

    This function is used internally by QDockWindow. You shouldn't
    need to call it yourself.
*/

void QDockArea::moveDockWindow( QDockWindow *w, const QPoint &p, const QRect &r, bool swap )
{
    invalidateFixedSizes();
    int mse = -10;
    bool hasResizable = FALSE;
    for ( QDockWindow *dw = dockWindows->first(); dw; dw = dockWindows->next() ) {
	if ( dw->isHidden() )
	    continue;
	if ( dw->isResizeEnabled() )
	    hasResizable = TRUE;
	if ( orientation() != Qt::Horizontal )
	    mse = QMAX( QMAX( dw->fixedExtent().width(), dw->width() ), mse );
	else
	    mse = QMAX( QMAX( dw->fixedExtent().height(), dw->height() ), mse );
    }
    if ( !hasResizable && w->isResizeEnabled() ) {
	if ( orientation() != Qt::Horizontal )
	    mse = QMAX( w->fixedExtent().width(), mse );
	else
	    mse = QMAX( w->fixedExtent().height(), mse );
    }

    QDockWindow *dockWindow = 0;
    int dockWindowIndex = findDockWindow( w );
    QPtrList<QDockWindow> lineStarts = layout->lineStarts();
    QValueList<QRect> lines = layout->lineList();
    bool wasAloneInLine = FALSE;
    QPoint pos = mapFromGlobal( p );
    QRect lr = *lines.at( lineOf( dockWindowIndex ) );
    if ( dockWindowIndex != -1 ) {
	if ( lineStarts.find( w ) != -1 &&
	     ( dockWindowIndex < (int)dockWindows->count() - 1 && lineStarts.find( dockWindows->at( dockWindowIndex + 1 ) ) != -1 ||
	       dockWindowIndex == (int)dockWindows->count() - 1 ) )
	    wasAloneInLine = TRUE;
	dockWindow = dockWindows->take( dockWindowIndex );
	if ( !wasAloneInLine ) { // only do the pre-layout if the widget isn't the only one in its line
	    if ( lineStarts.findRef( dockWindow ) != -1 && dockWindowIndex < (int)dockWindows->count() )
		dockWindows->at( dockWindowIndex )->setNewLine( TRUE );
	    layout->layoutItems( QRect( 0, 0, width(), height() ), TRUE );
	}
    } else {
	dockWindow = w;
	dockWindow->reparent( this, QPoint( 0, 0 ), TRUE );
	if ( swap )
	    dockWindow->resize( dockWindow->height(), dockWindow->width() );
	w->installEventFilter( this );
    }

    lineStarts = layout->lineStarts();
    lines = layout->lineList();

    QRect rect = QRect( mapFromGlobal( r.topLeft() ), r.size() );
    if ( orientation() == Horizontal && QApplication::reverseLayout() ) {
	rect = QRect( width() - rect.x() - rect.width(), rect.y(), rect.width(), rect.height() );
	pos.rx() = width() - pos.x();
    }
    dockWindow->setOffset( point_pos( rect.topLeft(), orientation() ) );
    if ( orientation() == Horizontal ) {
	int offs = dockWindow->offset();
	if ( width() - offs < dockWindow->minimumWidth() )
	    dockWindow->setOffset( width() - dockWindow->minimumWidth() );
    } else {
	int offs = dockWindow->offset();
	if ( height() - offs < dockWindow->minimumHeight() )
	    dockWindow->setOffset( height() - dockWindow->minimumHeight() );
    }

    if ( dockWindows->isEmpty() ) {
	dockWindows->append( dockWindow );
    } else {
	int dockLine = -1;
	bool insertLine = FALSE;
	int i = 0;
	QRect lineRect;
	// find the line which we touched with the mouse
	for ( QValueList<QRect>::Iterator it = lines.begin(); it != lines.end(); ++it, ++i ) {
	    if ( point_pos( pos, orientation(), TRUE ) >= point_pos( (*it).topLeft(), orientation(), TRUE ) &&
		 point_pos( pos, orientation(), TRUE ) <= point_pos( (*it).topLeft(), orientation(), TRUE ) +
		 size_extent( (*it).size(), orientation(), TRUE ) ) {
		dockLine = i;
		lineRect = *it;
		break;
	    }
	}
	if ( dockLine == -1 ) { // outside the dock...
	    insertLine = TRUE;
	    if ( point_pos( pos, orientation(), TRUE ) < 0 ) // insert as first line
		dockLine = 0;
	    else
		dockLine = (int)lines.count(); // insert after the last line ### size_t/int cast
	} else { // inside the dock (we have found a dockLine)
	    if ( point_pos( pos, orientation(), TRUE ) <
		 point_pos( lineRect.topLeft(), orientation(), TRUE ) + 4 ) {	// mouse was at the very beginning of the line
		insertLine = TRUE;					// insert a new line before that with the docking widget
	    } else if ( point_pos( pos, orientation(), TRUE ) >
			point_pos( lineRect.topLeft(), orientation(), TRUE ) +
			size_extent( lineRect.size(), orientation(), TRUE ) - 4 ) {	// mouse was at the very and of the line
		insertLine = TRUE;						// insert a line after that with the docking widget
		dockLine++;
	    }
	}

	if ( !insertLine && wasAloneInLine && lr.contains( pos ) ) // if we are alone in a line and just moved in there, re-insert it
	    insertLine = TRUE;

#if defined(QDOCKAREA_DEBUG)
	qDebug( "insert in line %d, and insert that line: %d", dockLine, insertLine );
	qDebug( "     (btw, we have %d lines)", lines.count() );
#endif
	QDockWindow *dw = 0;
	if ( dockLine >= (int)lines.count() ) { // insert after last line
	    dockWindows->append( dockWindow );
	    dockWindow->setNewLine( TRUE );
#if defined(QDOCKAREA_DEBUG)
	    qDebug( "insert at the end" );
#endif
	} else if ( dockLine == 0 && insertLine ) { // insert before first line
	    dockWindows->insert( 0, dockWindow );
	    dockWindows->at( 1 )->setNewLine( TRUE );
#if defined(QDOCKAREA_DEBUG)
	    qDebug( "insert at the begin" );
#endif
	} else { // insert somewhere in between
	    // make sure each line start has a new line
	    for ( dw = lineStarts.first(); dw; dw = lineStarts.next() )
		dw->setNewLine( TRUE );

	    // find the index of the first widget in the search line
	    int searchLine = dockLine;
#if defined(QDOCKAREA_DEBUG)
	    qDebug( "search line start of %d", searchLine );
#endif
	    QDockWindow *lsw = lineStarts.at( searchLine );
	    int index = dockWindows->find( lsw );
	    if ( index == -1 ) { // the linestart widget hasn't been found, try to find it harder
		if ( lsw == w && dockWindowIndex <= (int)dockWindows->count())
		    index = dockWindowIndex;
		else
		    index = 0;
		if ( index < (int)dockWindows->count() )
		    (void)dockWindows->at( index ); // move current to index
	    }
#if defined(QDOCKAREA_DEBUG)
	    qDebug( "     which starts at %d", index );
#endif
	    if ( !insertLine ) { // if we insert the docking widget in the existing line
		// find the index for the widget
		bool inc = TRUE;
		bool firstTime = TRUE;
		for ( dw = dockWindows->current(); dw; dw = dockWindows->next() ) {
		    if ( orientation() == Horizontal )
			dw->setFixedExtentWidth( -1 );
		    else
			dw->setFixedExtentHeight( -1 );
		    if ( !firstTime && lineStarts.find( dw ) != -1 ) // we are in the next line, so break
			break;
		    if ( point_pos( pos, orientation() ) <
			 point_pos( fix_pos( dw ), orientation() ) + size_extent( dw->size(), orientation() ) / 2 ) {
			inc = FALSE;
		    }
		    if ( inc )
			index++;
		    firstTime = FALSE;
		}
#if defined(QDOCKAREA_DEBUG)
		qDebug( "insert at index: %d", index );
#endif
		// if we insert it just before a widget which has a new line, transfer the newline to the docking widget
		// but not if we didn't only mave a widget in its line which was alone in the line before
		if ( !( wasAloneInLine && lr.contains( pos ) )
		     && index >= 0 && index < (int)dockWindows->count() &&
		     dockWindows->at( index )->newLine() && lineOf( index ) == dockLine ) {
#if defined(QDOCKAREA_DEBUG)
		    qDebug( "get rid of the old newline and get me one" );
#endif
		    dockWindows->at( index )->setNewLine( FALSE );
		    dockWindow->setNewLine( TRUE );
		} else if ( wasAloneInLine && lr.contains( pos ) ) {
		    dockWindow->setNewLine( TRUE );
		} else { // if we are somewhere in a line, get rid of the newline
		    dockWindow->setNewLine( FALSE );
		}
	    } else { // insert in a new line, so make sure the dock widget and the widget which will be after it have a newline
#if defined(QDOCKAREA_DEBUG)
		qDebug( "insert a new line" );
#endif
		if ( index < (int)dockWindows->count() ) {
#if defined(QDOCKAREA_DEBUG)
		    qDebug( "give the widget at %d a newline", index );
#endif
		    QDockWindow* nldw = dockWindows->at( index );
		    if ( nldw )
			nldw->setNewLine( TRUE );
		}
#if defined(QDOCKAREA_DEBUG)
		qDebug( "give me a newline" );
#endif
		dockWindow->setNewLine( TRUE );
	    }
	    // finally insert the widget
	    dockWindows->insert( index, dockWindow );
	}
    }

    if ( mse != -10 && w->isResizeEnabled() ) {
	if ( orientation() != Qt::Horizontal )
	    w->setFixedExtentWidth( QMIN( QMAX( w->minimumWidth(), mse ), w->sizeHint().width() ) );
	else
	    w->setFixedExtentHeight( QMIN( QMAX( w->minimumHeight(), mse ), w->sizeHint().height() ) );
    }

    updateLayout();
    setSizePolicy( QSizePolicy( orientation() == Horizontal ? QSizePolicy::Expanding : QSizePolicy::Minimum,
				orientation() == Vertical ? QSizePolicy::Expanding : QSizePolicy::Minimum ) );
}

/*!
    Removes the dock window \a w from the dock area. If \a
    makeFloating is TRUE, \a w gets floated, and if \a swap is TRUE,
    the orientation of \a w gets swapped. If \a fixNewLines is TRUE
    (the default) newlines in the area will be fixed.

    You should never need to call this function yourself. Use
    QDockWindow::dock() and QDockWindow::undock() instead.
*/

void QDockArea::removeDockWindow( QDockWindow *w, bool makeFloating, bool swap, bool fixNewLines )
{
    w->removeEventFilter( this );
    QDockWindow *dockWindow = 0;
    int i = findDockWindow( w );
    if ( i == -1 )
	return;
    dockWindow = dockWindows->at( i );
    dockWindows->remove( i );
    QPtrList<QDockWindow> lineStarts = layout->lineStarts();
    if ( fixNewLines && lineStarts.findRef( dockWindow ) != -1 && i < (int)dockWindows->count() )
	dockWindows->at( i )->setNewLine( TRUE );
    if ( makeFloating ) {
	QWidget *p = parentWidget() ? parentWidget() : topLevelWidget();
	dockWindow->reparent( p, WType_Dialog | WStyle_Customize | WStyle_NoBorder | WStyle_Tool, QPoint( 0, 0 ), FALSE );
    }
    if ( swap )
	dockWindow->resize( dockWindow->height(), dockWindow->width() );
    updateLayout();
    if ( dockWindows->isEmpty() )
	setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
}

int QDockArea::findDockWindow( QDockWindow *w )
{
    return dockWindows ? dockWindows->findRef( w ) : -1;
}

void QDockArea::updateLayout()
{
    layout->invalidate();
    layout->activate();
}

/*! \reimp
 */

bool QDockArea::eventFilter( QObject *o, QEvent *e )
{
    if ( e->type() == QEvent::Close ) {
	if ( ::qt_cast<QDockWindow*>(o) ) {
	    o->removeEventFilter( this );
	    QApplication::sendEvent( o, e );
	    if ( ( (QCloseEvent*)e )->isAccepted() )
		removeDockWindow( (QDockWindow*)o, FALSE, FALSE );
	    return TRUE;
	}
    }
    return FALSE;
}

/*! \internal

    Invalidates the offset of the next dock window in the dock area.
 */

void QDockArea::invalidNextOffset( QDockWindow *dw )
{
    int i = dockWindows->find( dw );
    if ( i == -1 || i >= (int)dockWindows->count() - 1 )
	return;
    if ( ( dw = dockWindows->at( ++i ) ) )
	dw->setOffset( 0 );
}

/*!
    \property QDockArea::count
    \brief the number of dock windows in the dock area
*/
int QDockArea::count() const
{
    return dockWindows->count();
}

/*!
    \property QDockArea::empty
    \brief whether the dock area is empty
*/

bool QDockArea::isEmpty() const
{
    return dockWindows->isEmpty();
}


/*!
    Returns a list of the dock windows in the dock area.
*/

QPtrList<QDockWindow> QDockArea::dockWindowList() const
{
    return *dockWindows;
}

/*!
    Lines up the dock windows in this dock area to minimize wasted
    space. If \a keepNewLines is TRUE, only space within lines is
    cleaned up. If \a keepNewLines is FALSE the number of lines might
    be changed.
*/

void QDockArea::lineUp( bool keepNewLines )
{
    for ( QDockWindow *dw = dockWindows->first(); dw; dw = dockWindows->next() ) {
	dw->setOffset( 0 );
	if ( !keepNewLines )
	    dw->setNewLine( FALSE );
    }
    layout->activate();
}

QDockArea::DockWindowData *QDockArea::dockWindowData( QDockWindow *w )
{
    DockWindowData *data = new DockWindowData;
    data->index = findDockWindow( w );
    if ( data->index == -1 ) {
	delete data;
	return 0;
    }
    QPtrList<QDockWindow> lineStarts = layout->lineStarts();
    int i = -1;
    for ( QDockWindow *dw = dockWindows->first(); dw; dw = dockWindows->next() ) {
	if ( lineStarts.findRef( dw ) != -1 )
	    ++i;
	if ( dw == w )
	    break;
    }
    data->line = i;
    data->offset = point_pos( QPoint( fix_x(w), w->y() ), orientation() );
    data->area = this;
    data->fixedExtent = w->fixedExtent();
    return data;
}

void QDockArea::dockWindow( QDockWindow *dockWindow, DockWindowData *data )
{
    if ( !data )
	return;

    dockWindow->reparent( this, QPoint( 0, 0 ), FALSE );
    dockWindow->installEventFilter( this );
    dockWindow->dockArea = this;
    dockWindow->updateGui();

    if ( dockWindows->isEmpty() ) {
	dockWindows->append( dockWindow );
    } else {
	QPtrList<QDockWindow> lineStarts = layout->lineStarts();
	int index = 0;
	if ( (int)lineStarts.count() > data->line )
	    index = dockWindows->find( lineStarts.at( data->line ) );
	if ( index == -1 ) {
	    index = 0;
	    (void)dockWindows->at( index );
	}
	bool firstTime = TRUE;
	int offset = data->offset;
	for ( QDockWindow *dw = dockWindows->current(); dw; dw = dockWindows->next() ) {
	    if ( !firstTime && lineStarts.find( dw ) != -1 )
		break;
	    if ( offset <
		 point_pos( fix_pos( dw ), orientation() ) + size_extent( dw->size(), orientation() ) / 2 )
		break;
	    index++;
	    firstTime = FALSE;
	}
	if ( index >= 0 && index < (int)dockWindows->count() &&
	     dockWindows->at( index )->newLine() && lineOf( index ) == data->line ) {
	    dockWindows->at( index )->setNewLine( FALSE );
	    dockWindow->setNewLine( TRUE );
	} else {
	    dockWindow->setNewLine( FALSE );
	}

	dockWindows->insert( index, dockWindow );
    }
    dockWindow->show();

    dockWindow->setFixedExtentWidth( data->fixedExtent.width() );
    dockWindow->setFixedExtentHeight( data->fixedExtent.height() );

    updateLayout();
    setSizePolicy( QSizePolicy( orientation() == Horizontal ? QSizePolicy::Expanding : QSizePolicy::Minimum,
				orientation() == Vertical ? QSizePolicy::Expanding : QSizePolicy::Minimum ) );

}

/*!
    Returns TRUE if dock window \a dw could be docked into the dock
    area; otherwise returns FALSE.

    \sa setAcceptDockWindow()
*/

bool QDockArea::isDockWindowAccepted( QDockWindow *dw )
{
    if ( !dw )
	return FALSE;
    if ( forbiddenWidgets.findRef( dw ) != -1 )
	return FALSE;

    QMainWindow *mw = ::qt_cast<QMainWindow*>(parentWidget());
    if ( !mw )
	return TRUE;
    if ( !mw->hasDockWindow( dw ) )
	return FALSE;
    if ( !mw->isDockEnabled( this ) )
	return FALSE;
    if ( !mw->isDockEnabled( dw, this ) )
	return FALSE;
    return TRUE;
}

/*!
    If \a accept is TRUE, dock window \a dw can be docked in the dock
    area. If \a accept is FALSE, dock window \a dw cannot be docked in
    the dock area.

    \sa isDockWindowAccepted()
*/

void QDockArea::setAcceptDockWindow( QDockWindow *dw, bool accept )
{
    if ( accept )
	forbiddenWidgets.removeRef( dw );
    else if ( forbiddenWidgets.findRef( dw ) == -1 )
	forbiddenWidgets.append( dw );
}

void QDockArea::invalidateFixedSizes()
{
    for ( QDockWindow *dw = dockWindows->first(); dw; dw = dockWindows->next() ) {
	if ( orientation() == Qt::Horizontal )
	    dw->setFixedExtentWidth( -1 );
	else
	    dw->setFixedExtentHeight( -1 );
    }
}

int QDockArea::maxSpace( int hint, QDockWindow *dw )
{
    int index = findDockWindow( dw );
    if ( index == -1 || index + 1 >= (int)dockWindows->count() ) {
	if ( orientation() == Horizontal )
	    return dw->width();
	return dw->height();
    }

    QDockWindow *w = 0;
    int i = 0;
    do {
	w = dockWindows->at( index + (++i) );
    } while ( i + 1 < (int)dockWindows->count() && ( !w || w->isHidden() ) );
    if ( !w || !w->isResizeEnabled() || i >= (int)dockWindows->count() ) {
	if ( orientation() == Horizontal )
	    return dw->width();
	return dw->height();
    }
    int min = 0;
    QToolBar *tb = ::qt_cast<QToolBar*>(w);
    if ( orientation() == Horizontal ) {
	w->setFixedExtentWidth( -1 );
	if ( !tb )
	    min = QMAX( w->minimumSize().width(), w->minimumSizeHint().width() );
	else
	    min = w->sizeHint().width();
    } else {
	w->setFixedExtentHeight( -1 );
	if ( !tb )
	    min = QMAX( w->minimumSize().height(), w->minimumSizeHint().height() );
	else
	    min = w->sizeHint().height();
    }

    int diff = hint - ( orientation() == Horizontal ? dw->width() : dw->height() );

    if ( ( orientation() == Horizontal ? w->width() : w->height() ) - diff < min )
	hint = ( orientation() == Horizontal ? dw->width() : dw->height() ) + ( orientation() == Horizontal ? w->width() : w->height() ) - min;

    diff = hint - ( orientation() == Horizontal ? dw->width() : dw->height() );
    if ( orientation() == Horizontal )
	w->setFixedExtentWidth( w->width() - diff );
    else
	w->setFixedExtentHeight( w->height() - diff );
    return hint;
}

void QDockArea::setFixedExtent( int d, QDockWindow *dw )
{
    QPtrList<QDockWindow> lst;
    QDockWindow *w;
    for ( w = dockWindows->first(); w; w = dockWindows->next() ) {
	if ( w->isHidden() )
	    continue;
	if ( orientation() == Horizontal ) {
	    if ( dw->y() != w->y() )
		continue;
	} else {
	    if ( dw->x() != w->x() )
		continue;
	}
	if ( orientation() == Horizontal )
	    d = QMAX( d, w->minimumHeight() );
	else
	    d = QMAX( d, w->minimumWidth() );
	if ( w->isResizeEnabled() )
	    lst.append( w );
    }
    for ( w = lst.first(); w; w = lst.next() ) {
	if ( orientation() == Horizontal )
	    w->setFixedExtentHeight( d );
	else
	    w->setFixedExtentWidth( d );
    }
}

bool QDockArea::isLastDockWindow( QDockWindow *dw )
{
    int i = dockWindows->find( dw );
    if ( i == -1 || i >= (int)dockWindows->count() - 1 )
	return TRUE;
    QDockWindow *w = 0;
    if ( ( w = dockWindows->at( ++i ) ) ) {
	if ( orientation() == Horizontal && dw->y() < w->y() )
	    return TRUE;
	if ( orientation() == Vertical && dw->x() < w->x() )
	    return TRUE;
    } else {
	return TRUE;
    }
    return FALSE;
}

#ifndef QT_NO_TEXTSTREAM

/*!
    \relates QDockArea

    Writes the layout of the dock windows in dock area \a dockArea to
    the text stream \a ts.

    \sa operator>>()
*/

QTextStream &operator<<( QTextStream &ts, const QDockArea &dockArea )
{
    QString str;
    QPtrList<QDockWindow> l = dockArea.dockWindowList();

    for ( QDockWindow *dw = l.first(); dw; dw = l.next() )
	str += "[" + QString( dw->caption() ) + "," + QString::number( (int)dw->offset() ) +
	       "," + QString::number( (int)dw->newLine() ) + "," + QString::number( dw->fixedExtent().width() ) +
	       "," + QString::number( dw->fixedExtent().height() ) + "," + QString::number( (int)!dw->isHidden() ) + "]";
    ts << str << endl;

    return ts;
}

/*!
    \relates QDockArea

    Reads the layout description of the dock windows in dock area \a
    dockArea from the text stream \a ts and restores it. The layout
    description must have been previously written by the operator<<()
    function.

    \sa operator<<()
*/

QTextStream &operator>>( QTextStream &ts, QDockArea &dockArea )
{
    QString s = ts.readLine();

    QString name, offset, newLine, width, height, visible;

    enum State { Pre, Name, Offset, NewLine, Width, Height, Visible, Post };
    int state = Pre;
    QChar c;
    QPtrList<QDockWindow> l = dockArea.dockWindowList();

    for ( int i = 0; i < (int)s.length(); ++i ) {
	c = s[ i ];
	if ( state == Pre && c == '[' ) {
	    state++;
	    continue;
	}
	if ( c == ',' &&
	     ( state == Name || state == Offset || state == NewLine || state == Width || state == Height ) ) {
	    state++;
	    continue;
	}
	if ( state == Visible && c == ']' ) {
	    for ( QDockWindow *dw = l.first(); dw; dw = l.next() ) {
		if ( QString( dw->caption() ) == name ) {
		    dw->setNewLine( (bool)newLine.toInt() );
		    dw->setOffset( offset.toInt() );
		    dw->setFixedExtentWidth( width.toInt() );
		    dw->setFixedExtentHeight( height.toInt() );
		    if ( !(bool)visible.toInt() )
			dw->hide();
		    else
			dw->show();
		    break;
		}
	    }

	    name = offset = newLine = width = height = visible = "";

	    state = Pre;
	    continue;
	}
	if ( state == Name )
	    name += c;
	else if ( state == Offset )
	    offset += c;
	else if ( state == NewLine )
	    newLine += c;
	else if ( state == Width )
	    width += c;
	else if ( state == Height )
	    height += c;
	else if ( state == Visible )
	    visible += c;
    }

    dockArea.QWidget::layout()->invalidate();
    dockArea.QWidget::layout()->activate();
    return ts;
}
#endif

#endif //QT_NO_MAINWINDOW
