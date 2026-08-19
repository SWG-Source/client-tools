/**********************************************************************
** $Id: qcanvas.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QCanvas and associated classes
**
** Created : 991211
**
** Copyright (C) 1999-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the canvas module of the Qt GUI Toolkit.
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

#include "qcanvas.h"
#ifndef QT_NO_CANVAS
#include "qapplication.h"
#include "qbitmap.h"
#include "qimage.h"
#include "qptrdict.h"
#include "qpainter.h"
#include "qpolygonscanner.h"
#include "qtimer.h"
#include "qtl.h"

#include <stdlib.h>

class QCanvasData {
public:
    QCanvasData() :
	itemDict(1013), animDict(503)
    {
    }

    QPtrList<QCanvasView> viewList;
    QPtrDict<void> itemDict;
    QPtrDict<void> animDict;
};

class QCanvasViewData {
public:
    QCanvasViewData() : repaint_from_moving( FALSE ) {}
#ifndef QT_NO_TRANSFORMATIONS
    QWMatrix xform;
    QWMatrix ixform;
#endif
    bool repaint_from_moving;
};

// clusterizer

class QCanvasClusterizer {
public:
    QCanvasClusterizer(int maxclusters);
    ~QCanvasClusterizer();

    void add(int x, int y); // 1x1 rectangle (point)
    void add(int x, int y, int w, int h);
    void add(const QRect& rect);

    void clear();
    int clusters() { return count; }
    const QRect& operator[](int i);

private:
    QRect* cluster;
    int count;
    const int maxcl;
};

static
void include(QRect& r, const QRect& rect)
{
    if (rect.left()<r.left()) {
	    r.setLeft(rect.left());
    }
    if (rect.right()>r.right()) {
	    r.setRight(rect.right());
    }
    if (rect.top()<r.top()) {
	    r.setTop(rect.top());
    }
    if (rect.bottom()>r.bottom()) {
	    r.setBottom(rect.bottom());
    }
}

/*
A QCanvasClusterizer groups rectangles (QRects) into non-overlapping rectangles
by a merging heuristic.
*/
QCanvasClusterizer::QCanvasClusterizer(int maxclusters) :
    cluster(new QRect[maxclusters]),
    count(0),
    maxcl(maxclusters)
{ }

QCanvasClusterizer::~QCanvasClusterizer()
{
    delete [] cluster;
}

void QCanvasClusterizer::clear()
{
    count=0;
}

void QCanvasClusterizer::add(int x, int y)
{
    add(QRect(x,y,1,1));
}

void QCanvasClusterizer::add(int x, int y, int w, int h)
{
    add(QRect(x,y,w,h));
}

void QCanvasClusterizer::add(const QRect& rect)
{
    QRect biggerrect(rect.x()-1,rect.y()-1,rect.width()+2,rect.height()+2);

    //assert(rect.width()>0 && rect.height()>0);

    int cursor;

    for (cursor=0; cursor<count; cursor++) {
	if (cluster[cursor].contains(rect)) {
	    // Wholly contained already.
	    return;
	}
    }

    int lowestcost=9999999;
    int cheapest=-1;
    cursor = 0;
    while( cursor<count ) {
	if (cluster[cursor].intersects(biggerrect)) {
	    QRect larger=cluster[cursor];
	    include(larger,rect);
	    int cost = larger.width()*larger.height() -
		       cluster[cursor].width()*cluster[cursor].height();

	    if (cost < lowestcost) {
		bool bad=FALSE;
		for (int c=0; c<count && !bad; c++) {
		    bad=cluster[c].intersects(larger) && c!=cursor;
		}
		if (!bad) {
		    cheapest=cursor;
		    lowestcost=cost;
		}
	    }
	}
	cursor++;
    }

    if (cheapest>=0) {
	include(cluster[cheapest],rect);
	return;
    }

    if (count < maxcl) {
	cluster[count++]=rect;
	return;
    }

    // Do cheapest of:
    //     add to closest cluster
    //     do cheapest cluster merge, add to new cluster

    lowestcost=9999999;
    cheapest=-1;
    cursor=0;
    while( cursor<count ) {
	QRect larger=cluster[cursor];
	include(larger,rect);
	int cost=larger.width()*larger.height()
		- cluster[cursor].width()*cluster[cursor].height();
	if (cost < lowestcost) {
	    bool bad=FALSE;
	    for (int c=0; c<count && !bad; c++) {
		bad=cluster[c].intersects(larger) && c!=cursor;
	    }
	    if (!bad) {
		cheapest=cursor;
		lowestcost=cost;
	    }
	}
	cursor++;
    }

    // ###
    // could make an heuristic guess as to whether we need to bother
    // looking for a cheap merge.

    int cheapestmerge1 = -1;
    int cheapestmerge2 = -1;

    int merge1 = 0;
    while( merge1 < count ) {
	int merge2=0;
	while( merge2 < count ) {
	    if( merge1!=merge2) {
		QRect larger=cluster[merge1];
		include(larger,cluster[merge2]);
		int cost=larger.width()*larger.height()
		    - cluster[merge1].width()*cluster[merge1].height()
		    - cluster[merge2].width()*cluster[merge2].height();
		if (cost < lowestcost) {
		    bool bad=FALSE;
		    for (int c=0; c<count && !bad; c++) {
			bad=cluster[c].intersects(larger) && c!=cursor;
		    }
		    if (!bad) {
			cheapestmerge1=merge1;
			cheapestmerge2=merge2;
			lowestcost=cost;
		    }
		}
	    }
	    merge2++;
	}
	merge1++;
    }

    if (cheapestmerge1>=0) {
	include(cluster[cheapestmerge1],cluster[cheapestmerge2]);
	cluster[cheapestmerge2]=cluster[count--];
    } else {
	// if (!cheapest) debugRectangles(rect);
	include(cluster[cheapest],rect);
    }

    // NB: clusters do not intersect (or intersection will
    //     overwrite). This is a result of the above algorithm,
    //     given the assumption that (x,y) are ordered topleft
    //     to bottomright.

    // ###
    //
    // add explicit x/y ordering to that comment, move it to the top
    // and rephrase it as pre-/post-conditions.
}

const QRect& QCanvasClusterizer::operator[](int i)
{
    return cluster[i];
}

// end of clusterizer



class QM_EXPORT_CANVAS QCanvasItemPtr {
public:
    QCanvasItemPtr() : ptr(0) { }
    QCanvasItemPtr( QCanvasItem* p ) : ptr(p) { }

    bool operator<=(const QCanvasItemPtr& that) const
    {
	// Order same-z objects by identity.
	if (that.ptr->z()==ptr->z())
	    return that.ptr <= ptr;
	return that.ptr->z() <= ptr->z();
    }
    bool operator<(const QCanvasItemPtr& that) const
    {
	// Order same-z objects by identity.
	if (that.ptr->z()==ptr->z())
	    return that.ptr < ptr;
	return that.ptr->z() < ptr->z();
    }
    bool operator>(const QCanvasItemPtr& that) const
    {
	// Order same-z objects by identity.
	if (that.ptr->z()==ptr->z())
	    return that.ptr > ptr;
	return that.ptr->z() > ptr->z();
    }
    bool operator==(const QCanvasItemPtr& that) const
    {
	    return that.ptr == ptr;
    }
    operator QCanvasItem*() const { return ptr; }

private:
    QCanvasItem* ptr;
};


/*!
    \class QCanvasItemList
    \brief The QCanvasItemList class is a list of QCanvasItems.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module canvas
    \ingroup graphics
    \ingroup images

    QCanvasItemList is a QValueList of pointers to \l{QCanvasItem}s.
    This class is used by some methods in QCanvas that need to return
    a list of canvas items.

    The \l QValueList documentation describes how to use this list.
*/

/*!
  \internal
*/
void QCanvasItemList::sort()
{
    qHeapSort(*((QValueList<QCanvasItemPtr>*)this));
}

/*!
  \internal
*/
void QCanvasItemList::drawUnique( QPainter& painter )
{
    QCanvasItem* prev=0;
    for (Iterator it=fromLast(); it!=end(); --it) {
	QCanvasItem *g=*it;
	if (g!=prev) {
	    g->draw(painter);
	    prev=g;
	}
    }
}

/*!
    Returns the concatenation of this list and list \a l.
*/
QCanvasItemList QCanvasItemList::operator+(const QCanvasItemList &l) const
{
    QCanvasItemList l2(*this);
    for(const_iterator it = l.begin(); it != l.end(); ++it)
       l2.append(*it);
    return l2;
}

class QCanvasChunk {
public:
    QCanvasChunk() : changed(TRUE) { }
    // Other code assumes lists are not deleted. Assignment is also
    // done on ChunkRecs. So don't add that sort of thing here.

    void sort()
    {
	list.sort();
    }

    const QCanvasItemList* listPtr() const
    {
	return &list;
    }

    void add(QCanvasItem* item)
    {
	list.prepend(item);
	changed = TRUE;
    }

    void remove(QCanvasItem* item)
    {
	list.remove(item);
	changed = TRUE;
    }

    void change()
    {
	changed = TRUE;
    }

    bool hasChanged() const
    {
	return changed;
    }

    bool takeChange()
    {
	bool y = changed;
	changed = FALSE;
	return y;
    }

private:
    QCanvasItemList list;
    bool changed;
};


static int gcd(int a, int b)
{
    int r;
    while ( (r = a%b) ) {
	a=b;
	b=r;
    }
    return b;
}

static int scm(int a, int b)
{
    int g = gcd(a,b);
    return a/g*b;
}



/*!
    \class QCanvas qcanvas.h
    \brief The QCanvas class provides a 2D area that can contain QCanvasItem objects.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup abstractwidgets
    \ingroup graphics
    \ingroup images
    \mainclass
    \module canvas

    The QCanvas class manages its 2D graphic area and all the canvas
    items the area contains. The canvas has no visual appearance of
    its own. Instead, it is displayed on screen using a QCanvasView.
    Multiple QCanvasView widgets may be associated with a canvas to
    provide multiple views of the same canvas.

    The canvas is optimized for large numbers of items, particularly
    where only a small percentage of the items change at any
    one time. If the entire display changes very frequently, you should
    consider using your own custom QScrollView subclass.

    Qt provides a rich
    set of canvas item classes, e.g. QCanvasEllipse, QCanvasLine,
    QCanvasPolygon, QCanvasPolygonalItem, QCanvasRectangle, QCanvasSpline,
    QCanvasSprite and QCanvasText. You can subclass to create your own
    canvas items; QCanvasPolygonalItem is the most common base class used
    for this purpose.

    Items appear on the canvas after their \link QCanvasItem::show()
    show()\endlink function has been called (or \link
    QCanvasItem::setVisible() setVisible(TRUE)\endlink), and \e after
    update() has been called. The canvas only shows items that are
    \link QCanvasItem::setVisible() visible\endlink, and then only if
    \l update() is called. (By default the canvas is white and so are
    canvas items, so if nothing appears try changing colors.)

    If you created the canvas without passing a width and height to
    the constructor you must also call resize().

    Although a canvas may appear to be similar to a widget with child
    widgets, there are several notable differences:

    \list
    \i Canvas items are usually much faster to manipulate and redraw than
    child widgets, with the speed advantage becoming especially great when
    there are \e many canvas items and non-rectangular items. In most
    situations canvas items are also a lot more memory efficient than child
    widgets.

    \i It's easy to detect overlapping items (collision detection).

    \i The canvas can be larger than a widget. A million-by-million canvas
    is perfectly possible. At such a size a widget might be very
    inefficient, and some window systems might not support it at all,
    whereas QCanvas scales well. Even with a billion pixels and a million
    items, finding a particular canvas item, detecting collisions, etc.,
    is still fast (though the memory consumption may be prohibitive
    at such extremes).

    \i Two or more QCanvasView objects can view the same canvas.

    \i An arbitrary transformation matrix can be set on each QCanvasView
    which makes it easy to zoom, rotate or shear the viewed canvas.

    \i Widgets provide a lot more functionality, such as input (QKeyEvent,
    QMouseEvent etc.) and layout management (QGridLayout etc.).

    \endlist

    A canvas consists of a background, a number of canvas items organized by
    x, y and z coordinates, and a foreground. A canvas item's z coordinate
    can be treated as a layer number -- canvas items with a higher z
    coordinate appear in front of canvas items with a lower z coordinate.

    The background is white by default, but can be set to a different color
    using setBackgroundColor(), or to a repeated pixmap using
    setBackgroundPixmap() or to a mosaic of smaller pixmaps using
    setTiles(). Individual tiles can be set with setTile(). There
    are corresponding get functions, e.g. backgroundColor() and
    backgroundPixmap().

    Note that QCanvas does not inherit from QWidget, even though it has some
    functions which provide the same functionality as those in QWidget. One
    of these is setBackgroundPixmap(); some others are resize(), size(),
    width() and height(). \l QCanvasView is the widget used to display a
    canvas on the screen.

    Canvas items are added to a canvas by constructing them and passing the
    canvas to the canvas item's constructor. An item can be moved to a
    different canvas using QCanvasItem::setCanvas().

    Canvas items are movable (and in the case of QCanvasSprites, animated)
    objects that inherit QCanvasItem. Each canvas item has a position on the
    canvas (x, y coordinates) and a height (z coordinate), all of which are
    held as floating-point numbers. Moving canvas items also have x and y
    velocities. It's possible for a canvas item to be outside the canvas
    (for example QCanvasItem::x() is greater than width()). When a canvas
    item is off the canvas, onCanvas() returns FALSE and the canvas
    disregards the item. (Canvas items off the canvas do not slow down any
    of the common operations on the canvas.)

    Canvas items can be moved with QCanvasItem::move(). The advance()
    function moves all QCanvasItem::animated() canvas items and
    setAdvancePeriod() makes QCanvas move them automatically on a periodic
    basis. In the context of the QCanvas classes, to `animate' a canvas item
    is to set it in motion, i.e. using QCanvasItem::setVelocity(). Animation
    of a canvas item itself, i.e. items which change over time, is enabled
    by calling QCanvasSprite::setFrameAnimation(), or more generally by
    subclassing and reimplementing QCanvasItem::advance(). To detect collisions
    use one of the QCanvasItem::collisions() functions.

    The changed parts of the canvas are redrawn (if they are visible in a
    canvas view) whenever update() is called. You can either call update()
    manually after having changed the contents of the canvas, or force
    periodic updates using setUpdatePeriod(). If you have moving objects on
    the canvas, you must call advance() every time the objects should
    move one step further. Periodic calls to advance() can be forced using
    setAdvancePeriod(). The advance() function will call
    QCanvasItem::advance() on every item that is \link
    QCanvasItem::animated() animated\endlink and trigger an update of the
    affected areas afterwards. (A canvas item that is `animated' is simply
    a canvas item that is in motion.)

    QCanvas organizes its canvas items into \e chunks; these are areas on
    the canvas that are used to speed up most operations. Many operations
    start by eliminating most chunks (i.e. those which haven't changed)
    and then process only the canvas items that are in the few interesting
    (i.e. changed) chunks. A valid chunk, validChunk(), is one which is on
    the canvas.

    The chunk size is a key factor to QCanvas's speed: if there are too many
    chunks, the speed benefit of grouping canvas items into chunks is
    reduced. If the chunks are too large, it takes too long to process each
    one. The QCanvas constructor tries to pick a suitable size, but you
    can call retune() to change it at any time. The chunkSize() function
    returns the current chunk size. The canvas items always make sure
    they're in the right chunks; all you need to make sure of is that
    the canvas uses the right chunk size. A good rule of thumb is that
    the size should be a bit smaller than the average canvas item
    size. If you have moving objects, the chunk size should be a bit
    smaller than the average size of the moving items.

    The foreground is normally nothing, but if you reimplement
    drawForeground(), you can draw things in front of all the canvas
    items.

    Areas can be set as changed with setChanged() and set unchanged with
    setUnchanged(). The entire canvas can be set as changed with
    setAllChanged(). A list of all the items on the canvas is returned by
    allItems().

    An area can be copied (painted) to a QPainter with drawArea().

    If the canvas is resized it emits the resized() signal.

    The examples/canvas application and the 2D graphics page of the
    examples/demo application demonstrate many of QCanvas's facilities.

    \sa QCanvasView QCanvasItem
*/
void QCanvas::init(int w, int h, int chunksze, int mxclusters)
{
    d = new QCanvasData;
    awidth=w;
    aheight=h;
    chunksize=chunksze;
    maxclusters=mxclusters;
    chwidth=(w+chunksize-1)/chunksize;
    chheight=(h+chunksize-1)/chunksize;
    chunks=new QCanvasChunk[chwidth*chheight];
    update_timer = 0;
    bgcolor = white;
    grid = 0;
    htiles = 0;
    vtiles = 0;
    dblbuf = TRUE;
    debug_redraw_areas = FALSE;
}

/*!
    Create a QCanvas with no size. \a parent and \a name are passed to
    the QObject superclass.

    \warning You \e must call resize() at some time after creation to
    be able to use the canvas.
*/
QCanvas::QCanvas( QObject* parent, const char* name )
    : QObject( parent, name )
{
    init(0,0);
}

/*!
    Constructs a QCanvas that is \a w pixels wide and \a h pixels high.
*/
QCanvas::QCanvas(int w, int h)
{
    init(w,h);
}

/*!
    Constructs a QCanvas which will be composed of \a h tiles
    horizontally and \a v tiles vertically. Each tile will be an image
    \a tilewidth by \a tileheight pixels taken from pixmap \a p.

    The pixmap \a p is a list of tiles, arranged left to right, (and
    in the case of pixmaps that have multiple rows of tiles, top to
    bottom), with tile 0 in the top-left corner, tile 1 next to the
    right, and so on, e.g.

    \table
    \row \i 0 \i 1 \i 2 \i 3
    \row \i 4 \i 5 \i 6 \i 7
    \endtable

    The QCanvas is initially sized to show exactly the given number of
    tiles horizontally and vertically. If it is resized to be larger,
    the entire matrix of tiles will be repeated as often as necessary
    to cover the area. If it is smaller, tiles to the right and bottom
    will not be visible.

    \sa setTiles()
*/
QCanvas::QCanvas( QPixmap p,
	int h, int v, int tilewidth, int tileheight )
{
    init(h*tilewidth, v*tileheight, scm(tilewidth,tileheight) );
    setTiles( p, h, v, tilewidth, tileheight );
}

void qt_unview(QCanvas* c)
{
    for (QCanvasView* view=c->d->viewList.first(); view != 0; view=c->d->viewList.next()) {
	view->viewing = 0;
    }
}

/*!
    Destroys the canvas and all the canvas's canvas items.
*/
QCanvas::~QCanvas()
{
    qt_unview(this);
    QCanvasItemList all = allItems();
    for (QCanvasItemList::Iterator it=all.begin(); it!=all.end(); ++it)
	delete *it;
    delete [] chunks;
    delete [] grid;
    delete d;
}

/*!
\internal
Returns the chunk at a chunk position \a i, \a j.
*/
QCanvasChunk& QCanvas::chunk(int i, int j) const
{
    return chunks[i+chwidth*j];
}

/*!
\internal
Returns the chunk at a pixel position \a x, \a y.
*/
QCanvasChunk& QCanvas::chunkContaining(int x, int y) const
{
    return chunk(x/chunksize,y/chunksize);
}

/*!
    Returns a list of all the items in the canvas.
*/
QCanvasItemList QCanvas::allItems()
{
    QCanvasItemList list;
    for (QPtrDictIterator<void> it=d->itemDict; it.currentKey(); ++it) {
	list.prepend((QCanvasItem*)it.currentKey());
    }
    return list;
}


/*!
    Changes the size of the canvas to have a width of \a w and a
    height of \a h. This is a slow operation.
*/
void QCanvas::resize(int w, int h)
{
    if (awidth==w && aheight==h)
	return;

    QCanvasItem* item;
    QPtrList<QCanvasItem> hidden;
    for (QPtrDictIterator<void> it=d->itemDict; it.currentKey(); ++it) {
	if (((QCanvasItem*)it.currentKey())->isVisible()) {
	    ((QCanvasItem*)it.currentKey())->hide();
	    hidden.append(((QCanvasItem*)it.currentKey()));
	}
    }

    int nchwidth=(w+chunksize-1)/chunksize;
    int nchheight=(h+chunksize-1)/chunksize;

    QCanvasChunk* newchunks = new QCanvasChunk[nchwidth*nchheight];

    // Commit the new values.
    //
    awidth=w;
    aheight=h;
    chwidth=nchwidth;
    chheight=nchheight;
    delete [] chunks;
    chunks=newchunks;

    for (item=hidden.first(); item != 0; item=hidden.next()) {
	item->show();
    }

    setAllChanged();

    emit resized();
}

/*!
    \fn void QCanvas::resized()

    This signal is emitted whenever the canvas is resized. Each
    QCanvasView connects to this signal to keep the scrollview's size
    correct.
*/

/*!
    Change the efficiency tuning parameters to \a mxclusters clusters,
    each of size \a chunksze. This is a slow operation if there are
    many objects on the canvas.

    The canvas is divided into chunks which are rectangular areas \a
    chunksze wide by \a chunksze high. Use a chunk size which is about
    the average size of the canvas items. If you choose a chunk size
    which is too small it will increase the amount of calculation
    required when drawing since each change will affect many chunks.
    If you choose a chunk size which is too large the amount of
    drawing required will increase because for each change, a lot of
    drawing will be required since there will be many (unchanged)
    canvas items which are in the same chunk as the changed canvas
    items.

    Internally, a canvas uses a low-resolution "chunk matrix" to keep
    track of all the items in the canvas. A 64x64 chunk matrix is the
    default for a 1024x1024 pixel canvas, where each chunk collects
    canvas items in a 16x16 pixel square. This default is also
    affected by setTiles(). You can tune this default using this
    function. For example if you have a very large canvas and want to
    trade off speed for memory then you might set the chunk size to 32
    or 64.

    The \a mxclusters argument is the number of rectangular groups of
    chunks that will be separately drawn. If the canvas has a large
    number of small, dispersed items, this should be about that
    number. Our testing suggests that a large number of clusters is
    almost always best.

*/
void QCanvas::retune(int chunksze, int mxclusters)
{
    maxclusters=mxclusters;

    if ( chunksize!=chunksze ) {
	QPtrList<QCanvasItem> hidden;
	for (QPtrDictIterator<void> it=d->itemDict; it.currentKey(); ++it) {
	    if (((QCanvasItem*)it.currentKey())->isVisible()) {
		((QCanvasItem*)it.currentKey())->hide();
		hidden.append(((QCanvasItem*)it.currentKey()));
	    }
	}

	chunksize=chunksze;

	int nchwidth=(awidth+chunksize-1)/chunksize;
	int nchheight=(aheight+chunksize-1)/chunksize;

	QCanvasChunk* newchunks = new QCanvasChunk[nchwidth*nchheight];

	// Commit the new values.
	//
	chwidth=nchwidth;
	chheight=nchheight;
	delete [] chunks;
	chunks=newchunks;

	for (QCanvasItem* item=hidden.first(); item != 0; item=hidden.next()) {
	    item->show();
	}
    }
}

/*!
    \fn int QCanvas::width() const

    Returns the width of the canvas, in pixels.
*/

/*!
    \fn int QCanvas::height() const

    Returns the height of the canvas, in pixels.
*/

/*!
    \fn QSize QCanvas::size() const

    Returns the size of the canvas, in pixels.
*/

/*!
    \fn QRect QCanvas::rect() const

    Returns a rectangle the size of the canvas.
*/


/*!
    \fn bool QCanvas::onCanvas( int x, int y ) const

    Returns TRUE if the pixel position (\a x, \a y) is on the canvas;
    otherwise returns FALSE.

    \sa validChunk()
*/

/*!
    \fn bool QCanvas::onCanvas( const QPoint& p ) const
    \overload

    Returns TRUE if the pixel position \a p is on the canvas;
    otherwise returns FALSE.

    \sa validChunk()
*/

/*!
    \fn bool QCanvas::validChunk( int x, int y ) const

    Returns TRUE if the chunk position (\a x, \a y) is on the canvas;
    otherwise returns FALSE.

    \sa onCanvas()
*/

/*!
  \fn bool QCanvas::validChunk( const QPoint& p ) const
  \overload

  Returns TRUE if the chunk position \a p is on the canvas; otherwise
  returns FALSE.

  \sa onCanvas()
*/

/*!
    \fn int QCanvas::chunkSize() const

    Returns the chunk size of the canvas.

    \sa retune()
*/

/*!
\fn bool QCanvas::sameChunk(int x1, int y1, int x2, int y2) const
\internal
Tells if the points ( \a x1, \a y1 ) and ( \a x2, \a y2 ) are within the same chunk.
*/

/*!
\internal
This method adds an the item \a item to the list of QCanvasItem objects
in the QCanvas. The QCanvasItem class calls this.
*/
void QCanvas::addItem(QCanvasItem* item)
{
    d->itemDict.insert((void*)item,(void*)1);
}

/*!
\internal
This method adds the item \a item to the list of QCanvasItem objects
to be moved. The QCanvasItem class calls this.
*/
void QCanvas::addAnimation(QCanvasItem* item)
{
    d->animDict.insert((void*)item,(void*)1);
}

/*!
\internal
This method adds the item \a item  to the list of QCanvasItem objects
which are no longer to be moved. The QCanvasItem class calls this.
*/
void QCanvas::removeAnimation(QCanvasItem* item)
{
    d->animDict.remove((void*)item);
}

/*!
\internal
This method removes the item \a item from the list of QCanvasItem objects
in this QCanvas. The QCanvasItem class calls this.
*/
void QCanvas::removeItem(QCanvasItem* item)
{
    d->itemDict.remove((void*)item);
}

/*!
\internal
This method adds the view \a view to the list of QCanvasView objects
viewing this QCanvas. The QCanvasView class calls this.
*/
void QCanvas::addView(QCanvasView* view)
{
    d->viewList.append(view);
    if ( htiles>1 || vtiles>1 || pm.isNull() )
	view->viewport()->setBackgroundColor(backgroundColor());
}

/*!
\internal
This method removes the view \a view from the list of QCanvasView objects
viewing this QCanvas. The QCanvasView class calls this.
*/
void QCanvas::removeView(QCanvasView* view)
{
    d->viewList.removeRef(view);
}

/*!
    Sets the canvas to call advance() every \a ms milliseconds. Any
    previous setting by setAdvancePeriod() or setUpdatePeriod() is
    overridden.

    If \a ms is less than 0 advancing will be stopped.
*/
void QCanvas::setAdvancePeriod(int ms)
{
    if ( ms<0 ) {
	if ( update_timer )
	    update_timer->stop();
    } else {
	if ( update_timer )
	    delete update_timer;
	update_timer = new QTimer(this);
	connect(update_timer,SIGNAL(timeout()),this,SLOT(advance()));
	update_timer->start(ms);
    }
}

/*!
    Sets the canvas to call update() every \a ms milliseconds. Any
    previous setting by setAdvancePeriod() or setUpdatePeriod() is
    overridden.

    If \a ms is less than 0 automatic updating will be stopped.
*/
void QCanvas::setUpdatePeriod(int ms)
{
    if ( ms<0 ) {
	if ( update_timer )
	    update_timer->stop();
    } else {
	if ( update_timer )
	    delete update_timer;
	update_timer = new QTimer(this);
	connect(update_timer,SIGNAL(timeout()),this,SLOT(update()));
	update_timer->start(ms);
    }
}

/*!
    Moves all QCanvasItem::animated() canvas items on the canvas and
    refreshes all changes to all views of the canvas. (An `animated'
    item is an item that is in motion; see setVelocity().)

    The advance takes place in two phases. In phase 0, the
    QCanvasItem::advance() function of each QCanvasItem::animated()
    canvas item is called with paramater 0. Then all these canvas
    items are called again, with parameter 1. In phase 0, the canvas
    items should not change position, merely examine other items on
    the canvas for which special processing is required, such as
    collisions between items. In phase 1, all canvas items should
    change positions, ignoring any other items on the canvas. This
    two-phase approach allows for considerations of "fairness",
    although no QCanvasItem subclasses supplied with Qt do anything
    interesting in phase 0.

    The canvas can be configured to call this function periodically
    with setAdvancePeriod().

    \sa update()
*/
void QCanvas::advance()
{
    QPtrDictIterator<void> it=d->animDict;
    while ( it.current() ) {
	QCanvasItem* i = (QCanvasItem*)(void*)it.currentKey();
	++it;
	if ( i )
	    i->advance(0);
    }
    // we expect the dict contains the exact same items as in the
    // first pass.
    it.toFirst();
    while ( it.current() ) {
	QCanvasItem* i = (QCanvasItem*)(void*)it.currentKey();
	++it;
	if ( i )
	    i->advance(1);
    }
    update();
}

// Don't call this unless you know what you're doing.
// p is in the content's co-ordinate example.
/*!
  \internal
*/
void QCanvas::drawViewArea( QCanvasView* view, QPainter* p, const QRect& vr, bool dbuf )
{
    QPoint tl = view->contentsToViewport(QPoint(0,0));

#ifndef QT_NO_TRANSFORMATIONS
    QWMatrix wm = view->worldMatrix();
    QWMatrix iwm = wm.invert();
    // ivr = covers all chunks in vr
    QRect ivr = iwm.map(vr);
    ivr.addCoords(-1, -1, 1, 1);
    QWMatrix twm;
    twm.translate(tl.x(),tl.y());
#else
    QRect ivr = vr;
#endif

    QRect all(0,0,width(),height());

    if ( !all.contains(ivr) ) {
	// Need to clip with edge of canvas.

#ifndef QT_NO_TRANSFORMATIONS
	// For translation-only transformation, it is safe to include the right
	// and bottom edges, but otherwise, these must be excluded since they
	// are not precisely defined (different bresenham paths).
	QPointArray a;
	if ( wm.m12()==0.0 && wm.m21()==0.0 && wm.m11() == 1.0 && wm.m22() == 1.0 )
	    a = QPointArray( QRect(all.x(),all.y(),all.width()+1,all.height()+1) );
	else
	    a = QPointArray( all );

	a = (wm*twm).map(a);
#else
	QPointArray a( QRect(all.x(),all.y(),all.width()+1,all.height()+1) );
#endif
	if ( view->viewport()->backgroundMode() == NoBackground ) {
	    QRect cvr = vr; cvr.moveBy(tl.x(),tl.y());
	    p->setClipRegion(QRegion(cvr)-QRegion(a));
	    p->fillRect(vr,view->viewport()->palette()
                        .brush(QPalette::Active,QColorGroup::Background));
	}
	p->setClipRegion(a);
    }

    if ( dbuf ) {
        offscr = QPixmap(vr.size().expandedTo(QSize(1, 1)));
#ifdef Q_WS_X11
        offscr.x11SetScreen(p->device()->x11Screen());
#endif
        QPainter dbp(&offscr);
#ifndef QT_NO_TRANSFORMATIONS
        twm.translate(-vr.x(),-vr.y());
        twm.translate(-tl.x(),-tl.y());
        dbp.setWorldMatrix( wm*twm, TRUE );
#else
        dbp.translate(-vr.x()-tl.x(),-vr.y()-tl.y());
#endif
        dbp.setClipRect(0,0,vr.width(), vr.height());
        drawCanvasArea(ivr,&dbp,FALSE);
        dbp.end();
        p->drawPixmap(vr.x(), vr.y(), offscr, 0, 0, vr.width(), vr.height());
    } else {
	QRect r = vr; r.moveBy(tl.x(),tl.y()); // move to untransformed co-ords
	if ( !all.contains(ivr) ) {
	    QRegion inside = p->clipRegion() & r;
	    //QRegion outside = p->clipRegion() - r;
	    //p->setClipRegion(outside);
	    //p->fillRect(outside.boundingRect(),red);
	    p->setClipRegion(inside);
	} else {
	    p->setClipRect(r);
	}
#ifndef QT_NO_TRANSFORMATIONS
	p->setWorldMatrix( wm*twm );
#else
#endif
	p->setBrushOrigin(tl.x(), tl.y());
	drawCanvasArea(ivr,p,FALSE);
    }
}

/*!
    Repaints changed areas in all views of the canvas.

    \sa advance()
*/
void QCanvas::update()
{
    QCanvasClusterizer clusterizer(d->viewList.count());
#ifndef QT_NO_TRANSFORMATIONS
    QPtrList<QRect> doneareas;
    doneareas.setAutoDelete(TRUE);
#endif

    QPtrListIterator<QCanvasView> it(d->viewList);
    QCanvasView* view;
    while( (view=it.current()) != 0 ) {
	++it;
#ifndef QT_NO_TRANSFORMATIONS
	QWMatrix wm = view->worldMatrix();
#endif
	QRect area(view->contentsX(),view->contentsY(),
		   view->visibleWidth(),view->visibleHeight());
	if (area.width()>0 && area.height()>0) {
#ifndef QT_NO_TRANSFORMATIONS
	    if ( !wm.isIdentity() ) {
		// r = Visible area of the canvas where there are changes
		QRect r = changeBounds(view->inverseWorldMatrix().map(area));
		if ( !r.isEmpty() ) {
		    QPainter p(view->viewport());
		    // Translate to the coordinate system of drawViewArea().
		    QPoint tl = view->contentsToViewport(QPoint(0,0));
		    p.translate(tl.x(),tl.y());
		    drawViewArea( view, &p, wm.map(r), dblbuf );
		    doneareas.append(new QRect(r));
		}
	    } else
#endif
	    {
		clusterizer.add(area);
	    }
	}
    }

    for (int i=0; i<clusterizer.clusters(); i++)
	drawChanges(clusterizer[i]);

#ifndef QT_NO_TRANSFORMATIONS
    for ( QRect* r=doneareas.first(); r != 0; r=doneareas.next() )
	setUnchanged(*r);
#endif
}


// ### warwick - setAllChanged() is not a set function. please rename
// it. ditto setChanged(). markChanged(), perhaps?
// ### unfortunately this function is virtual, which makes renaming more difficult. Lars

/*!
    Marks the whole canvas as changed.
    All views of the canvas will be entirely redrawn when
    update() is called next.
*/
void QCanvas::setAllChanged()
{
    setChanged(QRect(0,0,width(),height()));
}

/*!
    Marks \a area as changed. This \a area will be redrawn in all
    views that are showing it when update() is called next.
*/
void QCanvas::setChanged(const QRect& area)
{
    QRect thearea = area.intersect(QRect(0,0,width(),height()));

    int mx = (thearea.x()+thearea.width()+chunksize)/chunksize;
    int my = (thearea.y()+thearea.height()+chunksize)/chunksize;
    if (mx>chwidth)
	mx=chwidth;
    if (my>chheight)
	my=chheight;

    int x=thearea.x()/chunksize;
    while( x<mx) {
	int y = thearea.y()/chunksize;
	while( y<my ) {
	    chunk(x,y).change();
	    y++;
	}
	x++;
    }
}

/*!
    Marks \a area as \e unchanged. The area will \e not be redrawn in
    the views for the next update(), unless it is marked or changed
    again before the next call to update().
*/
void QCanvas::setUnchanged(const QRect& area)
{
    QRect thearea = area.intersect(QRect(0,0,width(),height()));

    int mx = (thearea.x()+thearea.width()+chunksize)/chunksize;
    int my = (thearea.y()+thearea.height()+chunksize)/chunksize;
    if (mx>chwidth)
	mx=chwidth;
    if (my>chheight)
	my=chheight;

    int x=thearea.x()/chunksize;
    while( x<mx) {
	int y = thearea.y()/chunksize;
	while( y<my ) {
	    chunk(x,y).takeChange();
	    y++;
	}
	x++;
    }
}


/*!
  \internal
*/
QRect QCanvas::changeBounds(const QRect& inarea)
{
    QRect area=inarea.intersect(QRect(0,0,width(),height()));

    int mx = (area.x()+area.width()+chunksize)/chunksize;
    int my = (area.y()+area.height()+chunksize)/chunksize;
    if (mx > chwidth)
	mx=chwidth;
    if (my > chheight)
	my=chheight;

    QRect result;

    int x=area.x()/chunksize;
    while( x<mx ) {
	int y=area.y()/chunksize;
	while( y<my ) {
	    QCanvasChunk& ch=chunk(x,y);
	    if ( ch.hasChanged() )
		result |= QRect(x,y,1,1);
	    y++;
	}
	x++;
    }

    if ( !result.isEmpty() ) {
	result.rLeft() *= chunksize;
	result.rTop() *= chunksize;
	result.rRight() *= chunksize;
	result.rBottom() *= chunksize;
	result.rRight() += chunksize;
	result.rBottom() += chunksize;
    }

    return result;
}

/*!
\internal
Redraws the area \a inarea of the QCanvas.
*/
void QCanvas::drawChanges(const QRect& inarea)
{
    QRect area=inarea.intersect(QRect(0,0,width(),height()));

    QCanvasClusterizer clusters(maxclusters);

    int mx = (area.x()+area.width()+chunksize)/chunksize;
    int my = (area.y()+area.height()+chunksize)/chunksize;
    if (mx > chwidth)
	mx=chwidth;
    if (my > chheight)
	my=chheight;

    int x=area.x()/chunksize;
    while( x<mx ) {
	int y=area.y()/chunksize;
	while( y<my ) {
	    QCanvasChunk& ch=chunk(x,y);
	    if ( ch.hasChanged() )
		clusters.add(x,y);
	    y++;
	}
	x++;
    }

    for (int i=0; i<clusters.clusters(); i++) {
	QRect elarea=clusters[i];
	elarea.setRect(
	    elarea.left()*chunksize,
	    elarea.top()*chunksize,
	    elarea.width()*chunksize,
	    elarea.height()*chunksize
	);
	drawCanvasArea(elarea);
    }
}

/*!
    Paints all canvas items that are in the area \a clip to \a
    painter, using double-buffering if \a dbuf is TRUE.

    e.g. to print the canvas to a printer:
    \code
    QPrinter pr;
    if ( pr.setup() ) {
	QPainter p(&pr);
	canvas.drawArea( canvas.rect(), &p );
    }
    \endcode
*/
void QCanvas::drawArea(const QRect& clip, QPainter* painter, bool dbuf)
{
    if ( painter )
	drawCanvasArea( clip, painter, dbuf );
}

/*!
  \internal
*/
void QCanvas::drawCanvasArea(const QRect& inarea, QPainter* p, bool double_buffer)
{
    QRect area=inarea.intersect(QRect(0,0,width(),height()));

    if ( !dblbuf )
	double_buffer = FALSE;

    if (!d->viewList.first() && !p) return; // Nothing to do.

    int lx=area.x()/chunksize;
    int ly=area.y()/chunksize;
    int mx=area.right()/chunksize;
    int my=area.bottom()/chunksize;
    if (mx>=chwidth)
	mx=chwidth-1;
    if (my>=chheight)
	my=chheight-1;

    QCanvasItemList allvisible;

    // Stores the region within area that need to be drawn. It is relative
    // to area.topLeft()  (so as to keep within bounds of 16-bit XRegions)
    QRegion rgn;

    for (int x=lx; x<=mx; x++) {
	for (int y=ly; y<=my; y++) {
	    // Only reset change if all views updating, and
	    // wholy within area. (conservative:  ignore entire boundary)
	    //
	    // Disable this to help debugging.
	    //
	    if (!p) {
		if ( chunk(x,y).takeChange() ) {
		    // ### should at least make bands
		    rgn |= QRegion(x*chunksize-area.x(),y*chunksize-area.y(),
				    chunksize,chunksize);
		    allvisible += *chunk(x,y).listPtr();
		}
	    } else {
		allvisible += *chunk(x,y).listPtr();
	    }
	}
    }
    allvisible.sort();

    if ( double_buffer ) {
        offscr = QPixmap(area.size().expandedTo(QSize(1, 1)));
#ifdef Q_WS_X11
        if (p)
            offscr.x11SetScreen(p->device()->x11Screen());
#endif
    }

    if ( double_buffer && !offscr.isNull() ) {
	QPainter painter;
	painter.begin(&offscr);
	painter.translate(-area.x(),-area.y());
        painter.setBrushOrigin(-area.x(),-area.y());
	if ( p ) {
	    painter.setClipRect(QRect(0,0,area.width(),area.height()));
	} else {
	    painter.setClipRegion(rgn);
	}
	drawBackground(painter,area);
	allvisible.drawUnique(painter);
	drawForeground(painter,area);
	painter.end();
	if ( p ) {
	    p->drawPixmap( area.x(), area.y(), offscr,
		0, 0, area.width(), area.height() );
	    return;
	}
    } else if ( p ) {
	drawBackground(*p,area);
	allvisible.drawUnique(*p);
	drawForeground(*p,area);
	return;
    }

    QPoint trtr; // keeps track of total translation of rgn

    trtr -= area.topLeft();

    for (QCanvasView* view=d->viewList.first(); view; view=d->viewList.next()) {
#ifndef QT_NO_TRANSFORMATIONS
	if ( !view->worldMatrix().isIdentity() )
	    continue; // Cannot paint those here (see callers).
#endif
	QPainter painter(view->viewport());
	QPoint tr = view->contentsToViewport(area.topLeft());
	QPoint nrtr = view->contentsToViewport(QPoint(0,0)); // new translation
	QPoint rtr = nrtr - trtr; // extra translation of rgn
	trtr += rtr; // add to total
	if (double_buffer) {
	    rgn.translate(rtr.x(),rtr.y());
	    painter.setClipRegion(rgn);
	    painter.drawPixmap(tr,offscr, QRect(QPoint(0,0),area.size()));
	} else {
	    painter.translate(nrtr.x(),nrtr.y());
	    rgn.translate(rtr.x(),rtr.y());
	    painter.setClipRegion(rgn);
	    drawBackground(painter,area);
	    allvisible.drawUnique(painter);
	    drawForeground(painter,area);
	    painter.translate(-nrtr.x(),-nrtr.y());
	}
    }
}

/*!
\internal
This method to informs the QCanvas that a given chunk is
`dirty' and needs to be redrawn in the next Update.

(\a x,\a y) is a chunk location.

The sprite classes call this. Any new derived class of QCanvasItem
must do so too. SetChangedChunkContaining can be used instead.
*/
void QCanvas::setChangedChunk(int x, int y)
{
    if (validChunk(x,y)) {
	QCanvasChunk& ch=chunk(x,y);
	ch.change();
    }
}

/*!
\internal
This method to informs the QCanvas that the chunk containing a given
pixel is `dirty' and needs to be redrawn in the next Update.

(\a x,\a y) is a pixel location.

The item classes call this. Any new derived class of QCanvasItem must
do so too. SetChangedChunk can be used instead.
*/
void QCanvas::setChangedChunkContaining(int x, int y)
{
    if (x>=0 && x<width() && y>=0 && y<height()) {
	QCanvasChunk& chunk=chunkContaining(x,y);
	chunk.change();
    }
}

/*!
\internal
This method adds the QCanvasItem \a g to the list of those which need to be
drawn if the given chunk at location ( \a x, \a y ) is redrawn. Like
SetChangedChunk and SetChangedChunkContaining, this method marks the
chunk as `dirty'.
*/
void QCanvas::addItemToChunk(QCanvasItem* g, int x, int y)
{
    if (validChunk(x,y)) {
	chunk(x,y).add(g);
    }
}

/*!
\internal
This method removes the QCanvasItem \a g from the list of those which need to
be drawn if the given chunk at location ( \a x, \a y ) is redrawn. Like
SetChangedChunk and SetChangedChunkContaining, this method marks the chunk
as `dirty'.
*/
void QCanvas::removeItemFromChunk(QCanvasItem* g, int x, int y)
{
    if (validChunk(x,y)) {
	chunk(x,y).remove(g);
    }
}


/*!
\internal
This method adds the QCanvasItem \a g to the list of those which need to be
drawn if the chunk containing the given pixel ( \a x, \a y ) is redrawn. Like
SetChangedChunk and SetChangedChunkContaining, this method marks the
chunk as `dirty'.
*/
void QCanvas::addItemToChunkContaining(QCanvasItem* g, int x, int y)
{
    if (x>=0 && x<width() && y>=0 && y<height()) {
	chunkContaining(x,y).add(g);
    }
}

/*!
\internal
This method removes the QCanvasItem \a g from the list of those which need to
be drawn if the chunk containing the given pixel ( \a x, \a y ) is redrawn.
Like SetChangedChunk and SetChangedChunkContaining, this method
marks the chunk as `dirty'.
*/
void QCanvas::removeItemFromChunkContaining(QCanvasItem* g, int x, int y)
{
    if (x>=0 && x<width() && y>=0 && y<height()) {
	chunkContaining(x,y).remove(g);
    }
}

/*!
    Returns the color set by setBackgroundColor(). By default, this is
    white.

    This function is not a reimplementation of
    QWidget::backgroundColor() (QCanvas is not a subclass of QWidget),
    but all QCanvasViews that are viewing the canvas will set their
    backgrounds to this color.

    \sa setBackgroundColor(), backgroundPixmap()
*/
QColor QCanvas::backgroundColor() const
{
    return bgcolor;
}

/*!
    Sets the solid background to be the color \a c.

    \sa backgroundColor(), setBackgroundPixmap(), setTiles()
*/
void QCanvas::setBackgroundColor( const QColor& c )
{
    if ( bgcolor != c ) {
	bgcolor = c;
	QCanvasView* view=d->viewList.first();
	while ( view != 0 ) {
	    /* XXX this doesn't look right. Shouldn't this
	       be more like setBackgroundPixmap? : Ian */
	    view->viewport()->setEraseColor( bgcolor );
	    view=d->viewList.next();
	}
	setAllChanged();
    }
}

/*!
    Returns the pixmap set by setBackgroundPixmap(). By default,
    this is a null pixmap.

    \sa setBackgroundPixmap(), backgroundColor()
*/
QPixmap QCanvas::backgroundPixmap() const
{
    return pm;
}

/*!
    Sets the solid background to be the pixmap \a p repeated as
    necessary to cover the entire canvas.

    \sa backgroundPixmap(), setBackgroundColor(), setTiles()
*/
void QCanvas::setBackgroundPixmap( const QPixmap& p )
{
    setTiles(p, 1, 1, p.width(), p.height());
    QCanvasView* view = d->viewList.first();
    while ( view != 0 ) {
	view->updateContents();
	view = d->viewList.next();
    }
}

/*!
    This virtual function is called for all updates of the canvas. It
    renders any background graphics using the painter \a painter, in
    the area \a clip. If the canvas has a background pixmap or a tiled
    background, that graphic is used, otherwise the canvas is cleared
    using the background color.

    If the graphics for an area change, you must explicitly call
    setChanged(const QRect&) for the result to be visible when
    update() is next called.

    \sa setBackgroundColor(), setBackgroundPixmap(), setTiles()
*/
void QCanvas::drawBackground(QPainter& painter, const QRect& clip)
{
    if ( pm.isNull() ) {
	painter.fillRect(clip,bgcolor);
    } else if ( !grid ) {
	for (int x=clip.x()/pm.width();
	    x<(clip.x()+clip.width()+pm.width()-1)/pm.width(); x++)
	{
	    for (int y=clip.y()/pm.height();
		y<(clip.y()+clip.height()+pm.height()-1)/pm.height(); y++)
	    {
		painter.drawPixmap(x*pm.width(), y*pm.height(),pm);
	    }
	}
    } else {
	const int x1 = clip.left()/tilew;
	int x2 = clip.right()/tilew;
	const int y1 = clip.top()/tileh;
	int y2 = clip.bottom()/tileh;

	const int roww = pm.width()/tilew;

	for (int j=y1; j<=y2; j++) {
	    int jj = j%tilesVertically();
	    for (int i=x1; i<=x2; i++) {
		int t = tile(i%tilesHorizontally(), jj);
		int tx = t % roww;
		int ty = t / roww;
		painter.drawPixmap( i*tilew, j*tileh, pm,
				tx*tilew, ty*tileh, tilew, tileh );
	    }
	}
    }
}

/*!
    This virtual function is called for all updates of the canvas. It
    renders any foreground graphics using the painter \a painter, in
    the area \a clip.

    If the graphics for an area change, you must explicitly call
    setChanged(const QRect&) for the result to be visible when
    update() is next called.

    The default is to draw nothing.
*/
void QCanvas::drawForeground(QPainter& painter, const QRect& clip)
{
    if ( debug_redraw_areas ) {
	painter.setPen(red);
	painter.setBrush(NoBrush);
	painter.drawRect(clip);
    }
}

/*!
    If \a y is TRUE (the default) double-buffering is switched on;
    otherwise double-buffering is switched off.

    Turning off double-buffering causes the redrawn areas to flicker a
    little and also gives a (usually small) performance improvement.
*/
void QCanvas::setDoubleBuffering(bool y)
{
    dblbuf = y;
}


/*!
    Sets the QCanvas to be composed of \a h tiles horizontally and \a
    v tiles vertically. Each tile will be an image \a tilewidth by \a
    tileheight pixels from pixmap \a p.

    The pixmap \a p is a list of tiles, arranged left to right, (and
    in the case of pixmaps that have multiple rows of tiles, top to
    bottom), with tile 0 in the top-left corner, tile 1 next to the
    right, and so on, e.g.

    \table
    \row \i 0 \i 1 \i 2 \i 3
    \row \i 4 \i 5 \i 6 \i 7
    \endtable

    If the canvas is larger than the matrix of tiles, the entire
    matrix is repeated as necessary to cover the whole canvas. If it
    is smaller, tiles to the right and bottom are not visible.

    The width and height of \a p must be a multiple of \a tilewidth
    and \a tileheight. If they are not the function will do nothing.

    If you want to unset any tiling set, then just pass in a null
    pixmap and 0 for \a h, \a v, \a tilewidth, and
    \a tileheight.
*/
void QCanvas::setTiles( QPixmap p,
			int h, int v, int tilewidth, int tileheight )
{
    if ( !p.isNull() && (!tilewidth || !tileheight ||
	 p.width() % tilewidth != 0 || p.height() % tileheight != 0 ) )
    	return;

    htiles = h;
    vtiles = v;
    delete[] grid;
    pm = p;
    if ( h && v && !p.isNull() ) {
	grid = new ushort[h*v];
	memset( grid, 0, h*v*sizeof(ushort) );
	tilew = tilewidth;
	tileh = tileheight;
    } else {
	grid = 0;
    }
    if ( h + v > 10 ) {
	int s = scm(tilewidth,tileheight);
	retune( s < 128 ? s : QMAX(tilewidth,tileheight) );
    }
    setAllChanged();
}

/*!
    \fn int QCanvas::tile( int x, int y ) const

    Returns the tile at position (\a x, \a y). Initially, all tiles
    are 0.

    The parameters must be within range, i.e.
	0 \< \a x \< tilesHorizontally() and
	0 \< \a y \< tilesVertically().

    \sa setTile()
*/

/*!
    \fn int QCanvas::tilesHorizontally() const

    Returns the number of tiles horizontally.
*/

/*!
    \fn int QCanvas::tilesVertically() const

    Returns the number of tiles vertically.
*/

/*!
    \fn int QCanvas::tileWidth() const

    Returns the width of each tile.
*/

/*!
    \fn int QCanvas::tileHeight() const

    Returns the height of each tile.
*/


/*!
    Sets the tile at (\a x, \a y) to use tile number \a tilenum, which
    is an index into the tile pixmaps. The canvas will update
    appropriately when update() is next called.

    The images are taken from the pixmap set by setTiles() and are
    arranged left to right, (and in the case of pixmaps that have
    multiple rows of tiles, top to bottom), with tile 0 in the
    top-left corner, tile 1 next to the right, and so on, e.g.

    \table
    \row \i 0 \i 1 \i 2 \i 3
    \row \i 4 \i 5 \i 6 \i 7
    \endtable

    \sa tile() setTiles()
*/
void QCanvas::setTile( int x, int y, int tilenum )
{
    ushort& t = grid[x+y*htiles];
    if ( t != tilenum ) {
	t = tilenum;
	if ( tilew == tileh && tilew == chunksize )
	    setChangedChunk( x, y );	    // common case
	else
	    setChanged( QRect(x*tilew,y*tileh,tilew,tileh) );
    }
}


// lesser-used data in canvas item, plus room for extension.
// Be careful adding to this - check all usages.
class QCanvasItemExtra {
    QCanvasItemExtra() : vx(0.0), vy(0.0) { }
    double vx,vy;
    friend class QCanvasItem;
};


/*!
    \class QCanvasItem qcanvas.h
    \brief The QCanvasItem class provides an abstract graphic object on a QCanvas.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module canvas
    \ingroup graphics
    \ingroup images

    A variety of QCanvasItem subclasses provide immediately usable
    behaviour. This class is a pure abstract superclass providing the
    behaviour that is shared among all the concrete canvas item classes.
    QCanvasItem is not intended for direct subclassing. It is much easier
    to subclass one of its subclasses, e.g. QCanvasPolygonalItem (the
    commonest base class), QCanvasRectangle, QCanvasSprite, QCanvasEllipse
    or QCanvasText.

    Canvas items are added to a canvas by constructing them and passing the
    canvas to the canvas item's constructor. An item can be moved to a
    different canvas using setCanvas().

    Items appear on the canvas after their \link show() show()\endlink
    function has been called (or \link setVisible()
    setVisible(TRUE)\endlink), and \e after update() has been called. The
    canvas only shows items that are \link setVisible() visible\endlink,
    and then only if \l update() is called. If you created the canvas
    without passing a width and height to the constructor you'll also need
    to call \link QCanvas::resize() resize()\endlink. Since the canvas
    background defaults to white and canvas items default to white,
    you may need to change colors to see your items.

    A QCanvasItem object can be moved in the x(), y() and z() dimensions
    using functions such as move(), moveBy(), setX(), setY() and setZ(). A
    canvas item can be set in motion, `animated', using setAnimated() and
    given a velocity in the x and y directions with setXVelocity() and
    setYVelocity() -- the same effect can be achieved by calling
    setVelocity(). Use the collidesWith() function to see if the canvas item
    will collide on the \e next advance(1) and use collisions() to see what
    collisions have occurred.

    Use QCanvasSprite or your own subclass of QCanvasSprite to create canvas
    items which are animated, i.e. which change over time.

    The size of a canvas item is given by boundingRect(). Use
    boundingRectAdvanced() to see what the size of the canvas item will be
    \e after the next advance(1) call.

    The rtti() function is used for identifying subclasses of QCanvasItem.
    The canvas() function returns a pointer to the canvas which contains the
    canvas item.

    QCanvasItem provides the show() and isVisible() functions like those in
    QWidget.

    QCanvasItem also provides the setEnabled(), setActive() and
    setSelected() functions; these functions set the relevant boolean and
    cause a repaint but the boolean values they set are not used in
    QCanvasItem itself. You can make use of these booleans in your subclasses.

    By default, canvas items have no velocity, no size, and are not in
    motion. The subclasses provided in Qt do not change these defaults
    except where noted.

*/

/*!
    \enum QCanvasItem::RttiValues

    This enum is used to name the different types of canvas item.

    \value Rtti_Item Canvas item abstract base class
    \value Rtti_Ellipse
    \value Rtti_Line
    \value Rtti_Polygon
    \value Rtti_PolygonalItem
    \value Rtti_Rectangle
    \value Rtti_Spline
    \value Rtti_Sprite
    \value Rtti_Text

*/

/*!
    \fn void QCanvasItem::update()

    Call this function to repaint the canvas's changed chunks.
*/

/*!
    Constructs a QCanvasItem on canvas \a canvas.

    \sa setCanvas()
*/
QCanvasItem::QCanvasItem(QCanvas* canvas) :
    cnv(canvas),
    myx(0),myy(0),myz(0)
{
    ani=0;
    vis=0;
    val=0;
    sel=0;
    ena=0;
    act=0;

    ext = 0;
    if (cnv) cnv->addItem(this);
}

/*!
    Destroys the QCanvasItem and removes it from its canvas.
*/
QCanvasItem::~QCanvasItem()
{
    if (cnv) {
	cnv->removeItem(this);
	cnv->removeAnimation(this);
    }
    delete ext;
}

QCanvasItemExtra& QCanvasItem::extra()
{
    if ( !ext )
	ext = new QCanvasItemExtra;
    return *ext;
}

/*!
    \fn double QCanvasItem::x() const

    Returns the horizontal position of the canvas item. Note that
    subclasses often have an origin other than the top-left corner.
*/

/*!
    \fn double QCanvasItem::y() const

    Returns the vertical position of the canvas item. Note that
    subclasses often have an origin other than the top-left corner.
*/

/*!
    \fn double QCanvasItem::z() const

    Returns the z index of the canvas item, which is used for visual
    order: higher-z items obscure (are in front of) lower-z items.
*/

/*!
    \fn void QCanvasItem::setX(double x)

    Moves the canvas item so that its x-position is \a x.

    \sa x(), move()
*/

/*!
    \fn void QCanvasItem::setY(double y)

    Moves the canvas item so that its y-position is \a y.

    \sa y(), move()
*/

/*!
    \fn void QCanvasItem::setZ(double z)

    Sets the z index of the canvas item to \a z. Higher-z items
    obscure (are in front of) lower-z items.

    \sa z(), move()
*/


/*!
    Moves the canvas item relative to its current position by (\a dx,
    \a dy).
*/
void QCanvasItem::moveBy( double dx, double dy )
{
    if ( dx || dy ) {
	removeFromChunks();
	myx += dx;
	myy += dy;
	addToChunks();
    }
}


/*!
    Moves the canvas item to the absolute position (\a x, \a y).
*/
void QCanvasItem::move( double x, double y )
{
    moveBy( x-myx, y-myy );
}


/*!
    Returns TRUE if the canvas item is in motion; otherwise returns
    FALSE.

    \sa setVelocity(), setAnimated()
*/
bool QCanvasItem::animated() const
{
    return (bool)ani;
}

/*!
    Sets the canvas item to be in motion if \a y is TRUE, or not if \a
    y is FALSE. The speed and direction of the motion is set with
    setVelocity(), or with setXVelocity() and setYVelocity().

    \sa advance(), QCanvas::advance()
*/
void QCanvasItem::setAnimated(bool y)
{
    if ( y != (bool)ani ) {
	ani = (uint)y;
	if ( y ) {
	    cnv->addAnimation(this);
	} else {
	    cnv->removeAnimation(this);
	}
    }
}

/*!
    \fn void QCanvasItem::setXVelocity( double vx )

    Sets the horizontal component of the canvas item's velocity to \a vx.

    \sa setYVelocity() setVelocity()
*/

/*!
    \fn void QCanvasItem::setYVelocity( double vy )

    Sets the vertical component of the canvas item's velocity to \a vy.

    \sa setXVelocity() setVelocity()
*/

/*!
    Sets the canvas item to be in motion, moving by \a vx and \a vy
    pixels in the horizontal and vertical directions respectively.

    \sa advance() setXVelocity() setYVelocity()
*/
void QCanvasItem::setVelocity( double vx, double vy)
{
    if ( ext || vx!=0.0 || vy!=0.0 ) {
	if ( !ani )
	    setAnimated(TRUE);
	extra().vx = vx;
	extra().vy = vy;
    }
}

/*!
    Returns the horizontal velocity component of the canvas item.
*/
double QCanvasItem::xVelocity() const
{
    return ext ? ext->vx : 0;
}

/*!
    Returns the vertical velocity component of the canvas item.
*/
double QCanvasItem::yVelocity() const
{
    return ext ? ext->vy : 0;
}

/*!
    The default implementation moves the canvas item, if it is
    animated(), by the preset velocity if \a phase is 1, and does
    nothing if \a phase is 0.

    Note that if you reimplement this function, the reimplementation
    must not change the canvas in any way, for example it must not add
    or remove items.

    \sa QCanvas::advance() setVelocity()
*/
void QCanvasItem::advance(int phase)
{
    if ( ext && phase==1 )
	moveBy(ext->vx,ext->vy);
}

/*!
    \fn void QCanvasItem::draw(QPainter& painter)

    This abstract virtual function draws the canvas item using \a painter.

    \warning When you reimplement this function, make sure that you
    leave the painter in the same state as you found it. For example,
    if you start by calling QPainter::translate(50, 50), end your
    code by calling QPainter::translate(-50, -50). Be also aware that
    the painter might already have some transformations set (i.e.,
    don't call QPainter::resetXForm() when you're done).
*/

/*!
    Sets the QCanvas upon which the canvas item is to be drawn to \a c.

    \sa canvas()
*/
void QCanvasItem::setCanvas(QCanvas* c)
{
    bool v=isVisible();
    setVisible(FALSE);
    if (cnv) {
	if (ext)
	    cnv->removeAnimation(this);
	cnv->removeItem(this);
    }
    cnv=c;
    if (cnv) {
	cnv->addItem(this);
	if ( ext )
	    cnv->addAnimation(this);
    }
    setVisible(v);
}

/*!
    \fn QCanvas* QCanvasItem::canvas() const

    Returns the canvas containing the canvas item.
*/

/*! Shorthand for setVisible(TRUE). */
void QCanvasItem::show()
{
    setVisible(TRUE);
}

/*! Shorthand for setVisible(FALSE). */
void QCanvasItem::hide()
{
    setVisible(FALSE);
}

/*!
    Makes the canvas item visible if \a yes is TRUE, or invisible if
    \a yes is FALSE. The change takes effect when QCanvas::update() is
    next called.
*/
void QCanvasItem::setVisible(bool yes)
{
    if ((bool)vis!=yes) {
	if (yes) {
	    vis=(uint)yes;
	    addToChunks();
	} else {
	    removeFromChunks();
	    vis=(uint)yes;
	}
    }
}
/*!
    \obsolete
    \fn bool QCanvasItem::visible() const
    Use isVisible() instead.
*/

/*!
    \fn bool QCanvasItem::isVisible() const

    Returns TRUE if the canvas item is visible; otherwise returns
    FALSE.

    Note that in this context TRUE does \e not mean that the canvas
    item is currently in a view, merely that if a view is showing the
    area where the canvas item is positioned, and the item is not
    obscured by items with higher z values, and the view is not
    obscured by overlaying windows, it would be visible.

    \sa setVisible(), z()
*/

/*!
    \obsolete
    \fn bool QCanvasItem::selected() const
    Use isSelected() instead.
*/

/*!
    \fn bool QCanvasItem::isSelected() const

    Returns TRUE if the canvas item is selected; otherwise returns FALSE.
*/

/*!
    Sets the selected flag of the item to \a yes. If this changes the
    item's selected state the item will be redrawn when
    QCanvas::update() is next called.

    The QCanvas, QCanvasItem and the Qt-supplied QCanvasItem
    subclasses do not make use of this value. The setSelected()
    function is supplied because many applications need it, but it is
    up to you how you use the isSelected() value.
*/
void QCanvasItem::setSelected(bool yes)
{
    if ((bool)sel!=yes) {
	sel=(uint)yes;
	changeChunks();
    }
}

/*!
    \obsolete
    \fn bool QCanvasItem::enabled() const
    Use isEnabled() instead.
*/

/*!
    \fn bool QCanvasItem::isEnabled() const

    Returns TRUE if the QCanvasItem is enabled; otherwise returns FALSE.
*/

/*!
    Sets the enabled flag of the item to \a yes. If this changes the
    item's enabled state the item will be redrawn when
    QCanvas::update() is next called.

    The QCanvas, QCanvasItem and the Qt-supplied QCanvasItem
    subclasses do not make use of this value. The setEnabled()
    function is supplied because many applications need it, but it is
    up to you how you use the isEnabled() value.
*/
void QCanvasItem::setEnabled(bool yes)
{
    if (ena!=(uint)yes) {
	ena=(uint)yes;
	changeChunks();
    }
}

/*!
    \obsolete
    \fn bool QCanvasItem::active() const
    Use isActive() instead.
*/

/*!
    \fn bool QCanvasItem::isActive() const

    Returns TRUE if the QCanvasItem is active; otherwise returns FALSE.
*/

/*!
    Sets the active flag of the item to \a yes. If this changes the
    item's active state the item will be redrawn when
    QCanvas::update() is next called.

    The QCanvas, QCanvasItem and the Qt-supplied QCanvasItem
    subclasses do not make use of this value. The setActive() function
    is supplied because many applications need it, but it is up to you
    how you use the isActive() value.
*/
void QCanvasItem::setActive(bool yes)
{
    if (act!=(uint)yes) {
	act=(uint)yes;
	changeChunks();
    }
}

bool qt_testCollision(const QCanvasSprite* s1, const QCanvasSprite* s2)
{
    const QImage* s2image = s2->imageAdvanced()->collision_mask;
    QRect s2area = s2->boundingRectAdvanced();

    QRect cyourarea(s2area.x(),s2area.y(),
	    s2area.width(),s2area.height());

    QImage* s1image=s1->imageAdvanced()->collision_mask;

    QRect s1area = s1->boundingRectAdvanced();

    QRect ourarea = s1area.intersect(cyourarea);

    if ( ourarea.isEmpty() )
	return FALSE;

    int x2=ourarea.x()-cyourarea.x();
    int y2=ourarea.y()-cyourarea.y();
    int x1=ourarea.x()-s1area.x();
    int y1=ourarea.y()-s1area.y();
    int w=ourarea.width();
    int h=ourarea.height();

    if ( !s2image ) {
	if ( !s1image )
	    return w>0 && h>0;
	// swap everything around
	int t;
	t=x1; x1=x2; x2=t;
	t=y1; x1=y2; y2=t;
	s2image = s1image;
	s1image = 0;
    }

    // s2image != 0

    // A non-linear search may be more efficient.
    // Perhaps spiralling out from the center, or a simpler
    // vertical expansion from the centreline.

    // We assume that sprite masks don't have
    // different bit orders.
    //
    // Q_ASSERT(s1image->bitOrder()==s2image->bitOrder());

    if (s1image) {
	if (s1image->bitOrder() == QImage::LittleEndian) {
	    for (int j=0; j<h; j++) {
		uchar* ml = s1image->scanLine(y1+j);
		uchar* yl = s2image->scanLine(y2+j);
		for (int i=0; i<w; i++) {
		    if (*(yl + ((x2+i) >> 3)) & (1 << ((x2+i) & 7))
		    && *(ml + ((x1+i) >> 3)) & (1 << ((x1+i) & 7)))
		    {
			return TRUE;
		    }
		}
	    }
	} else {
	    for (int j=0; j<h; j++) {
		uchar* ml = s1image->scanLine(y1+j);
		uchar* yl = s2image->scanLine(y2+j);
		for (int i=0; i<w; i++) {
		    if (*(yl + ((x2+i) >> 3)) & (1 << (7-((x2+i) & 7)))
		    && *(ml + ((x1+i) >> 3)) & (1 << (7-((x1+i) & 7))))
		    {
			return TRUE;
		    }
		}
	    }
	}
    } else {
	if (s2image->bitOrder() == QImage::LittleEndian) {
	    for (int j=0; j<h; j++) {
		uchar* yl = s2image->scanLine(y2+j);
		for (int i=0; i<w; i++) {
		    if (*(yl + ((x2+i) >> 3)) & (1 << ((x2+i) & 7)))
		    {
			return TRUE;
		    }
		}
	    }
	} else {
	    for (int j=0; j<h; j++) {
		uchar* yl = s2image->scanLine(y2+j);
		for (int i=0; i<w; i++) {
		    if (*(yl + ((x2+i) >> 3)) & (1 << (7-((x2+i) & 7))))
		    {
			return TRUE;
		    }
		}
	    }
	}
    }

    return FALSE;
}

static bool collision_double_dispatch( const QCanvasSprite* s1,
				       const QCanvasPolygonalItem* p1,
				       const QCanvasRectangle* r1,
				       const QCanvasEllipse* e1,
				       const QCanvasText* t1,
				       const QCanvasSprite* s2,
				       const QCanvasPolygonalItem* p2,
				       const QCanvasRectangle* r2,
				       const QCanvasEllipse* e2,
				       const QCanvasText* t2 )
{
    const QCanvasItem* i1 = s1 ?
			    (const QCanvasItem*)s1 : p1 ?
			    (const QCanvasItem*)p1 : r1 ?
			    (const QCanvasItem*)r1 : e1 ?
			    (const QCanvasItem*)e1 : (const QCanvasItem*)t1;
    const QCanvasItem* i2 = s2 ?
			    (const QCanvasItem*)s2 : p2 ?
			    (const QCanvasItem*)p2 : r2 ?
			    (const QCanvasItem*)r2 : e2 ?
			    (const QCanvasItem*)e2 : (const QCanvasItem*)t2;

    if ( s1 && s2 ) {
	// a
	return qt_testCollision(s1,s2);
    } else if ( (r1 || t1 || s1) && (r2 || t2 || s2) ) {
	// b
	QRect rc1 = i1->boundingRectAdvanced();
	QRect rc2 = i2->boundingRectAdvanced();
	return rc1.intersects(rc2);
    } else if ( e1 && e2
		&& e1->angleLength()>=360*16 && e2->angleLength()>=360*16
		&& e1->width()==e1->height()
		&& e2->width()==e2->height() ) {
	// c
	double xd = (e1->x()+e1->xVelocity())-(e2->x()+e1->xVelocity());
	double yd = (e1->y()+e1->yVelocity())-(e2->y()+e1->yVelocity());
	double rd = (e1->width()+e2->width())/2;
	return xd*xd+yd*yd <= rd*rd;
    } else if ( p1 && (p2 || s2 || t2) ) {
	// d
	QPointArray pa1 = p1->areaPointsAdvanced();
	QPointArray pa2 = p2 ? p2->areaPointsAdvanced()
			  : QPointArray(i2->boundingRectAdvanced());
	bool col= !(QRegion(pa1) & QRegion(pa2,TRUE)).isEmpty();

	return col;
    } else {
	return collision_double_dispatch(s2,p2,r2,e2,t2,
					 s1,p1,r1,e1,t1);
    }
}

/*!
    \fn bool QCanvasItem::collidesWith( const QCanvasItem* other ) const

    Returns TRUE if the canvas item will collide with the \a other
    item \e after they have moved by their current velocities;
    otherwise returns FALSE.

    \sa collisions()
*/


/*!
    \class QCanvasSprite qcanvas.h
    \brief The QCanvasSprite class provides an animated canvas item on a QCanvas.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module canvas
    \ingroup graphics
    \ingroup images

    A canvas sprite is an object which can contain any number of images
    (referred to as frames), only one of which is current, i.e.
    displayed, at any one time. The images can be passed in the
    constructor or set or changed later with setSequence(). If you
    subclass QCanvasSprite you can change the frame that is displayed
    periodically, e.g. whenever QCanvasItem::advance(1) is called to
    create the effect of animation.

    The current frame can be set with setFrame() or with move(). The
    number of frames available is given by frameCount(). The bounding
    rectangle of the current frame is returned by boundingRect().

    The current frame's image can be retrieved with image(); use
    imageAdvanced() to retrieve the image for the frame that will be
    shown after advance(1) is called. Use the image() overload passing
    it an integer index to retrieve a particular image from the list of
    frames.

    Use width() and height() to retrieve the dimensions of the current
    frame.

    Use leftEdge() and rightEdge() to retrieve the current frame's
    left-hand and right-hand x-coordinates respectively. Use
    bottomEdge() and topEdge() to retrieve the current frame's bottom
    and top y-coordinates respectively. These functions have an overload
    which will accept an integer frame number to retrieve the
    coordinates of a particular frame.

    QCanvasSprite draws very quickly, at the expense of memory.

    The current frame's image can be drawn on a painter with draw().

    Like any other canvas item, canvas sprites can be moved with
    move() which sets the x and y coordinates and the frame number, as
    well as with QCanvasItem::move() and QCanvasItem::moveBy(), or by
    setting coordinates with QCanvasItem::setX(), QCanvasItem::setY()
    and QCanvasItem::setZ().

*/


/*!
  \reimp
*/
bool QCanvasSprite::collidesWith( const QCanvasItem* i ) const
{
    return i->collidesWith(this,0,0,0,0);
}

/*!
    Returns TRUE if the canvas item collides with any of the given
    items; otherwise returns FALSE. The parameters, \a s, \a p, \a r,
    \a e and \a t, are all the same object, this is just a type
    resolution trick.
*/
bool QCanvasSprite::collidesWith( const QCanvasSprite* s,
				  const QCanvasPolygonalItem* p,
				  const QCanvasRectangle* r,
				  const QCanvasEllipse* e,
				  const QCanvasText* t ) const
{
    return collision_double_dispatch(s,p,r,e,t,this,0,0,0,0);
}

/*!
  \reimp
*/
bool QCanvasPolygonalItem::collidesWith( const QCanvasItem* i ) const
{
    return i->collidesWith(0,this,0,0,0);
}

bool QCanvasPolygonalItem::collidesWith(  const QCanvasSprite* s,
				 const QCanvasPolygonalItem* p,
				 const QCanvasRectangle* r,
				 const QCanvasEllipse* e,
				 const QCanvasText* t ) const
{
    return collision_double_dispatch(s,p,r,e,t,0,this,0,0,0);
}

/*!
  \reimp
*/
bool QCanvasRectangle::collidesWith( const QCanvasItem* i ) const
{
    return i->collidesWith(0,this,this,0,0);
}

bool QCanvasRectangle::collidesWith(  const QCanvasSprite* s,
				 const QCanvasPolygonalItem* p,
				 const QCanvasRectangle* r,
				 const QCanvasEllipse* e,
				 const QCanvasText* t ) const
{
    return collision_double_dispatch(s,p,r,e,t,0,this,this,0,0);
}


/*!
  \reimp
*/
bool QCanvasEllipse::collidesWith( const QCanvasItem* i ) const
{
    return i->collidesWith(0,this,0,this,0);
}

bool QCanvasEllipse::collidesWith(  const QCanvasSprite* s,
				 const QCanvasPolygonalItem* p,
				 const QCanvasRectangle* r,
				 const QCanvasEllipse* e,
				 const QCanvasText* t ) const
{
    return collision_double_dispatch(s,p,r,e,t,0,this,0,this,0);
}

/*!
  \reimp
*/
bool QCanvasText::collidesWith( const QCanvasItem* i ) const
{
    return i->collidesWith(0,0,0,0,this);
}

bool QCanvasText::collidesWith(  const QCanvasSprite* s,
				 const QCanvasPolygonalItem* p,
				 const QCanvasRectangle* r,
				 const QCanvasEllipse* e,
				 const QCanvasText* t ) const
{
    return collision_double_dispatch(s,p,r,e,t,0,0,0,0,this);
}

/*!
    Returns the list of canvas items that this canvas item has
    collided with.

    A collision is generally defined as occurring when the pixels of
    one item draw on the pixels of another item, but not all
    subclasses are so precise. Also, since pixel-wise collision
    detection can be slow, this function works in either exact or
    inexact mode, according to the \a exact parameter.

    If \a exact is TRUE, the canvas items returned have been
    accurately tested for collision with the canvas item.

    If \a exact is FALSE, the canvas items returned are \e near the
    canvas item. You can test the canvas items returned using
    collidesWith() if any are interesting collision candidates. By
    using this approach, you can ignore some canvas items for which
    collisions are not relevant.

    The returned list is a list of QCanvasItems, but often you will
    need to cast the items to their subclass types. The safe way to do
    this is to use rtti() before casting. This provides some of the
    functionality of the standard C++ dynamic cast operation even on
    compilers where dynamic casts are not available.

    Note that a canvas item may be `on' a canvas, e.g. it was created
    with the canvas as parameter, even though its coordinates place it
    beyond the edge of the canvas's area. Collision detection only
    works for canvas items which are wholly or partly within the
    canvas's area.

    Note that if items have a velocity (see \l setVelocity()), then
    collision testing is done based on where the item \e will be when
    it moves, not its current location. For example, a "ball" item
    doesn't need to actually embed into a "wall" item before a
    collision is detected. For items without velocity, plain
    intersection is used.
*/
QCanvasItemList QCanvasItem::collisions(bool exact) const
{
    return canvas()->collisions(chunks(),this,exact);
}

/*!
    Returns a list of canvas items that collide with the point \a p.
    The list is ordered by z coordinates, from highest z coordinate
    (front-most item) to lowest z coordinate (rear-most item).
*/
QCanvasItemList QCanvas::collisions(const QPoint& p) const
{
    return collisions(QRect(p,QSize(1,1)));
}

/*!
    \overload

    Returns a list of items which collide with the rectangle \a r. The
    list is ordered by z coordinates, from highest z coordinate
    (front-most item) to lowest z coordinate (rear-most item).
*/
QCanvasItemList QCanvas::collisions(const QRect& r) const
{
    QCanvasRectangle i(r,(QCanvas*)this);
    i.setPen(NoPen);
    i.show(); // doesn't actually show, since we destroy it
    QCanvasItemList l = i.collisions(TRUE);
    l.sort();
    return l;
}

/*!
    \overload

    Returns a list of canvas items which intersect with the chunks
    listed in \a chunklist, excluding \a item. If \a exact is TRUE,
    only those which actually \link QCanvasItem::collidesWith()
    collide with\endlink \a item are returned; otherwise canvas items
    are included just for being in the chunks.

    This is a utility function mainly used to implement the simpler
    QCanvasItem::collisions() function.
*/
QCanvasItemList QCanvas::collisions(const QPointArray& chunklist,
	    const QCanvasItem* item, bool exact) const
{
    QPtrDict<void> seen;
    QCanvasItemList result;
    for (int i=0; i<(int)chunklist.count(); i++) {
	int x = chunklist[i].x();
	int y = chunklist[i].y();
	if ( validChunk(x,y) ) {
	    const QCanvasItemList* l = chunk(x,y).listPtr();
	    for (QCanvasItemList::ConstIterator it=l->begin(); it!=l->end(); ++it) {
		QCanvasItem *g=*it;
		if ( g != item ) {
		    if ( !seen.find(g) ) {
			seen.replace(g,(void*)1);
			if ( !exact || item->collidesWith(g) )
			    result.append(g);
		    }
		}
	    }
	}
    }
    return result;
}

/*!
  \internal
  Adds the item to all the chunks it covers.
*/
void QCanvasItem::addToChunks()
{
    if (isVisible() && canvas()) {
	QPointArray pa = chunks();
	for (int i=0; i<(int)pa.count(); i++)
	    canvas()->addItemToChunk(this,pa[i].x(),pa[i].y());
	val=(uint)TRUE;
    }
}

/*!
  \internal
  Removes the item from all the chunks it covers.
*/
void QCanvasItem::removeFromChunks()
{
    if (isVisible() && canvas()) {
	QPointArray pa = chunks();
	for (int i=0; i<(int)pa.count(); i++)
	    canvas()->removeItemFromChunk(this,pa[i].x(),pa[i].y());
    }
}

/*!
  \internal
  Sets all the chunks covered by the item to be refreshed with QCanvas::update()
  is next called.
*/
void QCanvasItem::changeChunks()
{
    if (isVisible() && canvas()) {
	if (!val)
	    addToChunks();
	QPointArray pa = chunks();
	for (int i=0; i<(int)pa.count(); i++)
	    canvas()->setChangedChunk(pa[i].x(),pa[i].y());
    }
}

/*!
    \fn QRect QCanvasItem::boundingRect() const

    Returns the bounding rectangle in pixels that the canvas item covers.

    \sa boundingRectAdvanced()
*/

/*!
    Returns the bounding rectangle of pixels that the canvas item \e
    will cover after advance(1) is called.

    \sa boundingRect()
*/
QRect QCanvasItem::boundingRectAdvanced() const
{
    int dx = int(x()+xVelocity())-int(x());
    int dy = int(y()+yVelocity())-int(y());
    QRect r = boundingRect();
    r.moveBy(dx,dy);
    return r;
}

/*!
    \class QCanvasPixmap qcanvas.h
    \brief The QCanvasPixmap class provides pixmaps for QCanvasSprites.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module canvas
    \ingroup graphics
    \ingroup images

    If you want to show a single pixmap on a QCanvas use a
    QCanvasSprite with just one pixmap.

    When pixmaps are inserted into a QCanvasPixmapArray they are held
    as QCanvasPixmaps. \l{QCanvasSprite}s are used to show pixmaps on
    \l{QCanvas}es and hold their pixmaps in a QCanvasPixmapArray. If
    you retrieve a frame (pixmap) from a QCanvasSprite it will be
    returned as a QCanvasPixmap.

    The pixmap is a QPixmap and can only be set in the constructor.
    There are three different constructors, one taking a QPixmap, one
    a QImage and one a file name that refers to a file in any
    supported file format (see QImageIO).

    QCanvasPixmap can have a hotspot which is defined in terms of an (x,
    y) offset. When you create a QCanvasPixmap from a PNG file or from
    a QImage that has a QImage::offset(), the offset() is initialized
    appropriately, otherwise the constructor leaves it at (0, 0). You
    can set it later using setOffset(). When the QCanvasPixmap is used
    in a QCanvasSprite, the offset position is the point at
    QCanvasItem::x() and QCanvasItem::y(), not the top-left corner of
    the pixmap.

    Note that for QCanvasPixmap objects created by a QCanvasSprite, the
    position of each QCanvasPixmap object is set so that the hotspot
    stays in the same position.

    \sa QCanvasPixmapArray QCanvasItem QCanvasSprite
*/

#ifndef QT_NO_IMAGEIO

/*!
    Constructs a QCanvasPixmap that uses the image stored in \a
    datafilename.
*/
QCanvasPixmap::QCanvasPixmap(const QString& datafilename)
{
    QImage image(datafilename);
    init(image);
}

#endif

/*!
    Constructs a QCanvasPixmap from the image \a image.
*/
QCanvasPixmap::QCanvasPixmap(const QImage& image)
{
    init(image);
}
/*!
    Constructs a QCanvasPixmap from the pixmap \a pm using the offset
    \a offset.
*/
QCanvasPixmap::QCanvasPixmap(const QPixmap& pm, const QPoint& offset)
{
    init(pm,offset.x(),offset.y());
}

void QCanvasPixmap::init(const QImage& image)
{
    convertFromImage(image);
    hotx = image.offset().x();
    hoty = image.offset().y();
#ifndef QT_NO_IMAGE_DITHER_TO_1
    if( image.hasAlphaBuffer() ) {
	QImage i = image.createAlphaMask();
	collision_mask = new QImage(i);
    } else
#endif
	collision_mask = 0;
}

void QCanvasPixmap::init(const QPixmap& pixmap, int hx, int hy)
{
    (QPixmap&)*this = pixmap;
    hotx = hx;
    hoty = hy;
    if( pixmap.mask() )  {
	QImage i = mask()->convertToImage();
	collision_mask = new QImage(i);
    } else
	collision_mask = 0;
}

/*!
    Destroys the pixmap.
*/
QCanvasPixmap::~QCanvasPixmap()
{
    delete collision_mask;
}

/*!
    \fn int QCanvasPixmap::offsetX() const

    Returns the x-offset of the pixmap's hotspot.

    \sa setOffset()
*/

/*!
    \fn int QCanvasPixmap::offsetY() const

    Returns the y-offset of the pixmap's hotspot.

    \sa setOffset()
*/

/*!
    \fn void QCanvasPixmap::setOffset(int x, int y)

    Sets the offset of the pixmap's hotspot to (\a x, \a y).

    \warning Do not call this function if any QCanvasSprites are
    currently showing this pixmap.
*/

/*!
    \class QCanvasPixmapArray qcanvas.h
    \brief The QCanvasPixmapArray class provides an array of QCanvasPixmaps.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module canvas
    \ingroup graphics
    \ingroup images


    This class is used by QCanvasSprite to hold an array of pixmaps.
    It is used to implement animated sprites, i.e. images that change
    over time, with each pixmap in the array holding one frame.

    Depending on the constructor you use you can load multiple pixmaps
    into the array either from a directory (specifying a wildcard
    pattern for the files), or from a list of QPixmaps. You can also
    read in a set of pixmaps after construction using readPixmaps().

    Individual pixmaps can be set with setImage() and retrieved with
    image(). The number of pixmaps in the array is returned by
    count().

    QCanvasSprite uses an image's mask for collision detection. You
    can change this by reading in a separate set of image masks using
    readCollisionMasks().

*/

/*!
    Constructs an invalid array (i.e. isValid() will return FALSE).
    You must call readPixmaps() before being able to use this
    QCanvasPixmapArray.
*/
QCanvasPixmapArray::QCanvasPixmapArray()
: framecount( 0 ), img( 0 )
{
}

#ifndef QT_NO_IMAGEIO
/*!
    Constructs a QCanvasPixmapArray from files.

    The \a fc parameter sets the number of frames to be loaded for
    this image.

    If \a fc is not 0, \a datafilenamepattern should contain "%1",
    e.g. "foo%1.png". The actual filenames are formed by replacing the
    %1 with four-digit integers from 0 to (fc - 1), e.g. foo0000.png,
    foo0001.png, foo0002.png, etc.

    If \a fc is 0, \a datafilenamepattern is asssumed to be a
    filename, and the image contained in this file will be loaded as
    the first (and only) frame.

    If \a datafilenamepattern does not exist, is not readable, isn't
    an image, or some other error occurs, the array ends up empty and
    isValid() returns FALSE.
*/

QCanvasPixmapArray::QCanvasPixmapArray( const QString& datafilenamepattern,
					int fc )
: framecount( 0 ), img( 0 )
{
    readPixmaps(datafilenamepattern,fc);
}
#endif

/*!
  \obsolete
  Use QCanvasPixmapArray::QCanvasPixmapArray( QValueList<QPixmap>, QPointArray )
  instead.

  Constructs a QCanvasPixmapArray from the list of QPixmaps \a
  list. The \a hotspots list has to be of the same size as \a list.
*/
QCanvasPixmapArray::QCanvasPixmapArray(QPtrList<QPixmap> list, QPtrList<QPoint> hotspots) :
    framecount(list.count()),
    img(new QCanvasPixmap*[list.count()])
{
    if (list.count() != hotspots.count()) {
	qWarning("QCanvasPixmapArray: lists have different lengths");
	reset();
	img = 0;
    } else {
	list.first();
	hotspots.first();
	for (int i=0; i<framecount; i++) {
	    img[i]=new QCanvasPixmap(*list.current(), *hotspots.current());
	    list.next();
	    hotspots.next();
	}
    }
}

/*!
    Constructs a QCanvasPixmapArray from the list of QPixmaps in the
    \a list. Each pixmap will get a hotspot according to the \a
    hotspots array. If no hotspots are specified, each one is set to
    be at position (0, 0).

    If an error occurs, isValid() will return FALSE.
*/
QCanvasPixmapArray::QCanvasPixmapArray(QValueList<QPixmap> list, QPointArray hotspots) :
    framecount((int)list.size()),
    img(new QCanvasPixmap*[list.size()])
{
    bool have_hotspots = ( hotspots.size() != 0 );
    if (have_hotspots && list.count() != hotspots.count()) {
	qWarning("QCanvasPixmapArray: lists have different lengths");
	reset();
	img = 0;
    } else {
	QValueList<QPixmap>::iterator it;
	it = list.begin();
	for (int i=0; i<framecount; i++) {
	    QPoint hs = have_hotspots ? hotspots[i] : QPoint( 0, 0 );
	    img[i]=new QCanvasPixmap( *it, hs );
	    ++it;
	}
    }
}

/*!
    Destroys the pixmap array and all the pixmaps it contains.
*/
QCanvasPixmapArray::~QCanvasPixmapArray()
{
    reset();
}

void QCanvasPixmapArray::reset()
{
    for (int i=0; i<framecount; i++)
	delete img[i];
    delete [] img;
    img = 0;
    framecount = 0;
}

#ifndef QT_NO_IMAGEIO
/*!
    Reads one or more pixmaps into the pixmap array.

    If \a fc is not 0, \a filenamepattern should contain "%1", e.g.
    "foo%1.png". The actual filenames are formed by replacing the %1
    with four-digit integers from 0 to (fc - 1), e.g. foo0000.png,
    foo0001.png, foo0002.png, etc.

    If \a fc is 0, \a filenamepattern is asssumed to be a filename,
    and the image contained in this file will be loaded as the first
    (and only) frame.

    If \a filenamepattern does not exist, is not readable, isn't an
    image, or some other error occurs, this function will return
    FALSE, and isValid() will return FALSE; otherwise this function
    will return TRUE.

    \sa isValid()
*/
bool QCanvasPixmapArray::readPixmaps( const QString& filenamepattern,
				      int fc)
{
    return readPixmaps(filenamepattern,fc,FALSE);
}

/*!
    Reads new collision masks for the array.

    By default, QCanvasSprite uses the image mask of a sprite to
    detect collisions. Use this function to set your own collision
    image masks.

    If count() is 1 \a filename must specify a real filename to read
    the mask from. If count() is greater than 1, the \a filename must
    contain a "%1" that will get replaced by the number of the mask to
    be loaded, just like QCanvasPixmapArray::readPixmaps().

    All collision masks must be 1-bit images or this function call
    will fail.

    If the file isn't readable, contains the wrong number of images,
    or there is some other error, this function will return FALSE, and
    the array will be flagged as invalid; otherwise this function
    returns TRUE.

    \sa isValid()
*/
bool QCanvasPixmapArray::readCollisionMasks(const QString& filename)
{
    return readPixmaps(filename,framecount,TRUE);
}


bool QCanvasPixmapArray::readPixmaps( const QString& datafilenamepattern,
				      int fc, bool maskonly)
{
    if ( !maskonly ) {
	reset();
	framecount = fc;
	if ( !framecount )
	    framecount=1;
	img = new QCanvasPixmap*[framecount];
    }
    if (!img)
        return FALSE;
    bool ok = TRUE;
    bool arg = fc > 1;
    if ( !arg )
	framecount=1;
    for (int i=0; i<framecount; i++) {
	QString r;
	r.sprintf("%04d",i);
	if ( maskonly ) {
            if (!img[i]->collision_mask)
                img[i]->collision_mask = new QImage();
	    img[i]->collision_mask->load(
		arg ? datafilenamepattern.arg(r) : datafilenamepattern);
	    ok = ok
	       && !img[i]->collision_mask->isNull()
	       && img[i]->collision_mask->depth()==1;
	} else {
	    img[i]=new QCanvasPixmap(
		arg ? datafilenamepattern.arg(r) : datafilenamepattern);
	    ok = ok && !img[i]->isNull();
	}
    }
    if ( !ok ) {
	reset();
    }
    return ok;
}
#endif

/*!
  \obsolete

  Use isValid() instead.

  This returns FALSE if the array is valid, and TRUE if it is not.
*/
bool QCanvasPixmapArray::operator!()
{
    return img==0;
}

/*!
    Returns TRUE if the pixmap array is valid; otherwise returns
    FALSE.
*/
bool QCanvasPixmapArray::isValid() const
{
    return (img != 0);
}

/*!
    \fn QCanvasPixmap* QCanvasPixmapArray::image(int i) const

    Returns pixmap \a i in the array, if \a i is non-negative and less
    than than count(), and returns an unspecified value otherwise.
*/

// ### wouldn't it be better to put empty QCanvasPixmaps in there instead of
// initializing the additional elements in the array to 0? Lars
/*!
    Replaces the pixmap at index \a i with pixmap \a p.

    The array takes ownership of \a p and will delete \a p when the
    array itself is deleted.

    If \a i is beyond the end of the array the array is extended to at
    least i+1 elements, with elements count() to i-1 being initialized
    to 0.
*/
void QCanvasPixmapArray::setImage(int i, QCanvasPixmap* p)
{
    if ( i >= framecount ) {
	QCanvasPixmap** newimg = new QCanvasPixmap*[i+1];
	memcpy(newimg, img, sizeof( QCanvasPixmap * )*framecount);
	memset(newimg + framecount, 0, sizeof( QCanvasPixmap * )*( i+1 - framecount ) );
	framecount = i+1;
	delete [] img;
	img = newimg;
    }
    delete img[i]; img[i]=p;
}

/*!
    \fn uint QCanvasPixmapArray::count() const

    Returns the number of pixmaps in the array.
*/

/*!
    Returns the x-coordinate of the current left edge of the sprite.
    (This may change as the sprite animates since different frames may
    have different left edges.)

    \sa rightEdge() bottomEdge() topEdge()
*/
int QCanvasSprite::leftEdge() const
{
    return int(x()) - image()->hotx;
}

/*!
    \overload

    Returns what the x-coordinate of the left edge of the sprite would
    be if the sprite (actually its hotspot) were moved to x-position
    \a nx.

    \sa rightEdge() bottomEdge() topEdge()
*/
int QCanvasSprite::leftEdge(int nx) const
{
    return nx - image()->hotx;
}

/*!
    Returns the y-coordinate of the top edge of the sprite. (This may
    change as the sprite animates since different frames may have
    different top edges.)

    \sa leftEdge() rightEdge() bottomEdge()
*/
int QCanvasSprite::topEdge() const
{
    return int(y()) - image()->hoty;
}

/*!
    \overload

    Returns what the y-coordinate of the top edge of the sprite would
    be if the sprite (actually its hotspot) were moved to y-position
    \a ny.

    \sa leftEdge() rightEdge() bottomEdge()
*/
int QCanvasSprite::topEdge(int ny) const
{
    return ny - image()->hoty;
}

/*!
    Returns the x-coordinate of the current right edge of the sprite.
    (This may change as the sprite animates since different frames may
    have different right edges.)

    \sa leftEdge() bottomEdge() topEdge()
*/
int QCanvasSprite::rightEdge() const
{
    return leftEdge() + image()->width()-1;
}

/*!
    \overload

    Returns what the x-coordinate of the right edge of the sprite
    would be if the sprite (actually its hotspot) were moved to
    x-position \a nx.

    \sa leftEdge() bottomEdge() topEdge()
*/
int QCanvasSprite::rightEdge(int nx) const
{
    return leftEdge(nx) + image()->width()-1;
}

/*!
    Returns the y-coordinate of the current bottom edge of the sprite.
    (This may change as the sprite animates since different frames may
    have different bottom edges.)

    \sa leftEdge() rightEdge() topEdge()
*/
int QCanvasSprite::bottomEdge() const
{
    return topEdge() + image()->height()-1;
}

/*!
    \overload

    Returns what the y-coordinate of the top edge of the sprite would
    be if the sprite (actually its hotspot) were moved to y-position
    \a ny.

    \sa leftEdge() rightEdge() topEdge()
*/
int QCanvasSprite::bottomEdge(int ny) const
{
    return topEdge(ny) + image()->height()-1;
}

/*!
    \fn QCanvasPixmap* QCanvasSprite::image() const

    Returns the current frame's image.

    \sa frame(), setFrame()
*/

/*!
    \fn QCanvasPixmap* QCanvasSprite::image(int f) const
    \overload

    Returns the image for frame \a f. Does not do any bounds checking on \a f.
*/

/*!
    Returns the image the sprite \e will have after advance(1) is
    called. By default this is the same as image().
*/
QCanvasPixmap* QCanvasSprite::imageAdvanced() const
{
    return image();
}

/*!
    Returns the bounding rectangle for the image in the sprite's
    current frame. This assumes that the images are tightly cropped
    (i.e. do not have transparent pixels all along a side).
*/
QRect QCanvasSprite::boundingRect() const
{
    return QRect(leftEdge(), topEdge(), width(), height());
}


/*!
  \internal
  Returns the chunks covered by the item.
*/
QPointArray QCanvasItem::chunks() const
{
    QPointArray r;
    int n=0;
    QRect br = boundingRect();
    if (isVisible() && canvas()) {
	int chunksize=canvas()->chunkSize();
	br &= QRect(0,0,canvas()->width(),canvas()->height());
	if ( br.isValid() ) {
	    r.resize((br.width()/chunksize+2)*(br.height()/chunksize+2));
	    for (int j=br.top()/chunksize; j<=br.bottom()/chunksize; j++) {
		for (int i=br.left()/chunksize; i<=br.right()/chunksize; i++) {
		    r[n++] = QPoint(i,j);
		}
	    }
	}
    }
    r.resize(n);
    return r;
}


/*!
  \internal
  Add the sprite to the chunks in its QCanvas which it overlaps.
*/
void QCanvasSprite::addToChunks()
{
    if (isVisible() && canvas()) {
	int chunksize=canvas()->chunkSize();
	for (int j=topEdge()/chunksize; j<=bottomEdge()/chunksize; j++) {
	    for (int i=leftEdge()/chunksize; i<=rightEdge()/chunksize; i++) {
		canvas()->addItemToChunk(this,i,j);
	    }
	}
    }
}

/*!
  \internal
  Remove the sprite from the chunks in its QCanvas which it overlaps.

  \sa addToChunks()
*/
void QCanvasSprite::removeFromChunks()
{
    if (isVisible() && canvas()) {
	int chunksize=canvas()->chunkSize();
	for (int j=topEdge()/chunksize; j<=bottomEdge()/chunksize; j++) {
	    for (int i=leftEdge()/chunksize; i<=rightEdge()/chunksize; i++) {
		canvas()->removeItemFromChunk(this,i,j);
	    }
	}
    }
}

/*!
    The width of the sprite for the current frame's image.

    \sa frame()
*/
//### mark: Why don't we have width(int) and height(int) to be
//consistent with leftEdge() and leftEdge(int)?
int QCanvasSprite::width() const
{
    return image()->width();
}

/*!
    The height of the sprite for the current frame's image.

    \sa frame()
*/
int QCanvasSprite::height() const
{
    return image()->height();
}


/*!
    Draws the current frame's image at the sprite's current position
    on painter \a painter.
*/
void QCanvasSprite::draw(QPainter& painter)
{
    painter.drawPixmap(leftEdge(),topEdge(),*image());
}

/*!
    \class QCanvasView qcanvas.h
    \brief The QCanvasView class provides an on-screen view of a QCanvas.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module canvas
    \ingroup graphics
    \ingroup images

    A QCanvasView is widget which provides a view of a QCanvas.

    If you want users to be able to interact with a canvas view,
    subclass QCanvasView. You might then reimplement
    QScrollView::contentsMousePressEvent(). For example, assuming no
    transformation matrix is set:

    \code
    void MyCanvasView::contentsMousePressEvent( QMouseEvent* e )
    {
	QCanvasItemList l = canvas()->collisions(e->pos());
	for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) {
	    if ( (*it)->rtti() == QCanvasRectangle::RTTI )
		qDebug("A QCanvasRectangle lies somewhere at this point");
	}
    }
    \endcode

    The canvas view shows canvas canvas(); this can be changed using
    setCanvas().

    A transformation matrix can be used to transform the view of the
    canvas in various ways, for example, zooming in or out or rotating.
    For example:

    \code
    QWMatrix wm;
    wm.scale( 2, 2 );   // Zooms in by 2 times
    wm.rotate( 90 );    // Rotates 90 degrees counter clockwise
			// around the origin.
    wm.translate( 0, -canvas->height() );
			// moves the canvas down so what was visible
			// before is still visible.
    myCanvasView->setWorldMatrix( wm );
    \endcode

    Use setWorldMatrix() to set the canvas view's world matrix: you must
    ensure that the world matrix is invertible. The current world matrix
    is retrievable with worldMatrix(), and its inversion is retrievable
    with inverseWorldMatrix().

    Example:

    The following code finds the part of the canvas that is visible in
    this view, i.e. the bounding rectangle of the view in canvas coordinates.

    \code
    QRect rc = QRect( myCanvasView->contentsX(), myCanvasView->contentsY(),
			myCanvasView->visibleWidth(), myCanvasView->visibleHeight() );
    QRect canvasRect = myCanvasView->inverseWorldMatrix().mapRect(rc);
    \endcode

    \sa QWMatrix QPainter::setWorldMatrix()

*/

/*!
    Constructs a QCanvasView with parent \a parent, and name \a name,
    using the widget flags \a f. The canvas view is not associated
    with a canvas, so you must to call setCanvas() to view a
    canvas.
*/
QCanvasView::QCanvasView(QWidget* parent, const char* name, WFlags f) :
    QScrollView(parent,name,f|WResizeNoErase|WStaticContents)
{
    d = new QCanvasViewData;
    viewing = 0;
    setCanvas(0);
    connect(this,SIGNAL(contentsMoving(int,int)),this,SLOT(cMoving(int,int)));
}

/*!
    \overload

    Constructs a QCanvasView which views canvas \a canvas, with parent
    \a parent, and name \a name, using the widget flags \a f.
*/
QCanvasView::QCanvasView(QCanvas* canvas, QWidget* parent, const char* name, WFlags f) :
    QScrollView(parent,name,f|WResizeNoErase|WStaticContents)
{
    d = new QCanvasViewData;
    viewing = 0;
    setCanvas(canvas);

    connect(this,SIGNAL(contentsMoving(int,int)),this,SLOT(cMoving(int,int)));
}

/*!
    Destroys the canvas view. The associated canvas is \e not deleted.
*/
QCanvasView::~QCanvasView()
{
    delete d;
    d = 0;
    setCanvas(0);
}

/*!
    \fn QCanvas* QCanvasView::canvas() const

    Returns a pointer to the canvas which the QCanvasView is currently
    showing.
*/


/*!
    Sets the canvas that the QCanvasView is showing to the canvas \a
    canvas.
*/
void QCanvasView::setCanvas(QCanvas* canvas)
{
    if (viewing) {
	disconnect(viewing);
	viewing->removeView(this);
    }
    viewing=canvas;
    if (viewing) {
	connect(viewing,SIGNAL(resized()), this, SLOT(updateContentsSize()));
	viewing->addView(this);
    }
    if ( d ) // called by d'tor
        updateContentsSize();
}

#ifndef QT_NO_TRANSFORMATIONS
/*!
    Returns a reference to the canvas view's current transformation matrix.

    \sa setWorldMatrix() inverseWorldMatrix()
*/
const QWMatrix &QCanvasView::worldMatrix() const
{
    return d->xform;
}

/*!
    Returns a reference to the inverse of the canvas view's current
    transformation matrix.

    \sa setWorldMatrix() worldMatrix()
*/
const QWMatrix &QCanvasView::inverseWorldMatrix() const
{
    return d->ixform;
}

/*!
    Sets the transformation matrix of the QCanvasView to \a wm. The
    matrix must be invertible (i.e. if you create a world matrix that
    zooms out by 2 times, then the inverse of this matrix is one that
    will zoom in by 2 times).

    When you use this, you should note that the performance of the
    QCanvasView will decrease considerably.

    Returns FALSE if \a wm is not invertable; otherwise returns TRUE.

    \sa worldMatrix() inverseWorldMatrix() QWMatrix::isInvertible()
*/
bool QCanvasView::setWorldMatrix( const QWMatrix & wm )
{
    bool ok = wm.isInvertible();
    if ( ok ) {
	d->xform = wm;
	d->ixform = wm.invert();
	updateContentsSize();
	viewport()->update();
    }
    return ok;
}
#endif

void QCanvasView::updateContentsSize()
{
    if ( viewing ) {
	QRect br;
#ifndef QT_NO_TRANSFORMATIONS
	br = d->xform.map(QRect(0,0,viewing->width(),viewing->height()));
#else
	br = QRect(0,0,viewing->width(),viewing->height());
#endif

	if ( br.width() < contentsWidth() ) {
	    QRect r(contentsToViewport(QPoint(br.width(),0)),
		    QSize(contentsWidth()-br.width(),contentsHeight()));
	    viewport()->erase(r);
	}
	if ( br.height() < contentsHeight() ) {
	    QRect r(contentsToViewport(QPoint(0,br.height())),
		    QSize(contentsWidth(),contentsHeight()-br.height()));
	    viewport()->erase(r);
	}

	resizeContents(br.width(),br.height());
    } else {
	viewport()->erase();
	resizeContents(1,1);
    }
}

void QCanvasView::cMoving(int x, int y)
{
    // A little kludge to smooth up repaints when scrolling
    int dx = x - contentsX();
    int dy = y - contentsY();
    d->repaint_from_moving = QABS(dx) < width()/8 && QABS(dy) < height()/8;
}

/*!
    Repaints part of the QCanvas that the canvas view is showing
    starting at \a cx by \a cy, with a width of \a cw and a height of \a
    ch using the painter \a p.

    \warning When double buffering is enabled, drawContents() will
    not respect the current settings of the painter when setting up
    the painter for the double buffer (e.g., viewport() and
    window()). Also, be aware that QCanvas::update() bypasses
    drawContents(), which means any reimplementation of
    drawContents() is not called.

    \sa QCanvas::setDoubleBuffering()
*/
void QCanvasView::drawContents(QPainter *p, int cx, int cy, int cw, int ch)
{
    QRect r(cx,cy,cw,ch);
    if (viewing) {
	//viewing->drawViewArea(this,p,r,TRUE);
	viewing->drawViewArea(this,p,r,!d->repaint_from_moving);
	d->repaint_from_moving = FALSE;
    } else {
	p->eraseRect(r);
    }
}

/*!
  \reimp
  \internal

  (Implemented to get rid of a compiler warning.)
*/
void QCanvasView::drawContents( QPainter * )
{
}

/*!
    Suggests a size sufficient to view the entire canvas.
*/
QSize QCanvasView::sizeHint() const
{
    if ( !canvas() )
	return QScrollView::sizeHint();
    // should maybe take transformations into account
    return ( canvas()->size() + 2 * QSize(frameWidth(), frameWidth()) )
	   .boundedTo( 3 * QApplication::desktop()->size() / 4 );
}

// ### Qt 4.0 customer request: operate on doubles rather than int.
// ### I know, almost impossible due to the use of QRegion etc.
/*!
    \class QCanvasPolygonalItem qcanvas.h
    \brief The QCanvasPolygonalItem class provides a polygonal canvas item
    on a QCanvas.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module canvas
    \ingroup graphics
    \ingroup images

    The mostly rectangular classes, such as QCanvasSprite and
    QCanvasText, use the object's bounding rectangle for movement,
    repainting and collision calculations. For most other items, the
    bounding rectangle can be far too large -- a diagonal line being
    the worst case, and there are many other cases which are also bad.
    QCanvasPolygonalItem provides polygon-based bounding rectangle
    handling, etc., which is much faster for non-rectangular items.

    Derived classes should try to define as small an area as possible
    to maximize efficiency, but the polygon must \e definitely be
    contained completely within the polygonal area. Calculating the
    exact requirements is usually difficult, but if you allow a small
    overestimate it can be easy and quick, while still getting almost
    all of QCanvasPolygonalItem's speed.

    Note that all subclasses \e must call hide() in their destructor
    since hide() needs to be able to access areaPoints().

    Normally, QCanvasPolygonalItem uses the odd-even algorithm for
    determining whether an object intersects this object. You can
    change this to the winding algorithm using setWinding().

    The bounding rectangle is available using boundingRect(). The
    points bounding the polygonal item are retrieved with
    areaPoints(). Use areaPointsAdvanced() to retrieve the bounding
    points the polygonal item \e will have after
    QCanvasItem::advance(1) has been called.

    If the shape of the polygonal item is about to change while the
    item is visible, call invalidate() before updating with a
    different result from \l areaPoints().

    By default, QCanvasPolygonalItem objects have a black pen and no
    brush (the default QPen and QBrush constructors). You can change
    this with setPen() and setBrush(), but note that some
    QCanvasPolygonalItem subclasses only use the brush, ignoring the
    pen setting.

    The polygonal item can be drawn on a painter with draw().
    Subclasses must reimplement drawShape() to draw themselves.

    Like any other canvas item polygonal items can be moved with
    QCanvasItem::move() and QCanvasItem::moveBy(), or by setting coordinates
    with QCanvasItem::setX(), QCanvasItem::setY() and QCanvasItem::setZ().

*/


/*
  Since most polygonal items don't have a pen, the default is
  NoPen and a black brush.
*/
static const QPen& defaultPolygonPen()
{
    static QPen* dp=0;
    if ( !dp )
	dp = new QPen;
    return *dp;
}

static const QBrush& defaultPolygonBrush()
{
    static QBrush* db=0;
    if ( !db )
	db = new QBrush;
    return *db;
}

/*!
    Constructs a QCanvasPolygonalItem on the canvas \a canvas.
*/
QCanvasPolygonalItem::QCanvasPolygonalItem(QCanvas* canvas) :
    QCanvasItem(canvas),
    br(defaultPolygonBrush()),
    pn(defaultPolygonPen())
{
    wind=0;
}

/*!
    Note that all subclasses \e must call hide() in their destructor
    since hide() needs to be able to access areaPoints().
*/
QCanvasPolygonalItem::~QCanvasPolygonalItem()
{
}

/*!
    Returns TRUE if the polygonal item uses the winding algorithm to
    determine the "inside" of the polygon. Returns FALSE if it uses
    the odd-even algorithm.

    The default is to use the odd-even algorithm.

    \sa setWinding()
*/
bool QCanvasPolygonalItem::winding() const
{
    return wind;
}

/*!
    If \a enable is TRUE, the polygonal item will use the winding
    algorithm to determine the "inside" of the polygon; otherwise the
    odd-even algorithm will be used.

    The default is to use the odd-even algorithm.

    \sa winding()
*/
void QCanvasPolygonalItem::setWinding(bool enable)
{
    wind = enable;
}

/*!
    Invalidates all information about the area covered by the canvas
    item. The item will be updated automatically on the next call that
    changes the item's status, for example, move() or update(). Call
    this function if you are going to change the shape of the item (as
    returned by areaPoints()) while the item is visible.
*/
void QCanvasPolygonalItem::invalidate()
{
    val = (uint)FALSE;
    removeFromChunks();
}

/*!
    \fn QCanvasPolygonalItem::isValid() const

    Returns TRUE if the polygonal item's area information has not been
    invalidated; otherwise returns FALSE.

    \sa invalidate()
*/

/*!
    Returns the points the polygonal item \e will have after
    QCanvasItem::advance(1) is called, i.e. what the points are when
    advanced by the current xVelocity() and yVelocity().
*/
QPointArray QCanvasPolygonalItem::areaPointsAdvanced() const
{
    int dx = int(x()+xVelocity())-int(x());
    int dy = int(y()+yVelocity())-int(y());
    QPointArray r = areaPoints();
    r.detach(); // Explicit sharing is stupid.
    if ( dx || dy )
	r.translate(dx,dy);
    return r;
}

//#define QCANVAS_POLYGONS_DEBUG
#ifdef QCANVAS_POLYGONS_DEBUG
static QWidget* dbg_wid=0;
static QPainter* dbg_ptr=0;
#endif

class QPolygonalProcessor {
public:
    QPolygonalProcessor(QCanvas* c, const QPointArray& pa) :
	canvas(c)
    {
	QRect pixelbounds = pa.boundingRect();
	int cs = canvas->chunkSize();
	QRect canvasbounds = pixelbounds.intersect(canvas->rect());
	bounds.setLeft(canvasbounds.left()/cs);
	bounds.setRight(canvasbounds.right()/cs);
	bounds.setTop(canvasbounds.top()/cs);
	bounds.setBottom(canvasbounds.bottom()/cs);
	bitmap = QImage(bounds.width() + 1, bounds.height(), 1, 2, QImage::LittleEndian);
	pnt = 0;
	bitmap.fill(0);
#ifdef QCANVAS_POLYGONS_DEBUG
	dbg_start();
#endif
    }

    inline void add(int x, int y)
    {
	if ( pnt >= (int)result.size() ) {
	    result.resize(pnt*2+10);
	}
	result[pnt++] = QPoint(x+bounds.x(),y+bounds.y());
#ifdef QCANVAS_POLYGONS_DEBUG
	if ( dbg_ptr ) {
	    int cs = canvas->chunkSize();
	    QRect r(x*cs+bounds.x()*cs,y*cs+bounds.y()*cs,cs-1,cs-1);
	    dbg_ptr->setPen(Qt::blue);
	    dbg_ptr->drawRect(r);
	}
#endif
    }

    inline void addBits(int x1, int x2, uchar newbits, int xo, int yo)
    {
	for (int i=x1; i<=x2; i++)
	    if ( newbits & (1<<i) )
		add(xo+i,yo);
    }

#ifdef QCANVAS_POLYGONS_DEBUG
    void dbg_start()
    {
	if ( !dbg_wid ) {
	    dbg_wid = new QWidget;
	    dbg_wid->resize(800,600);
	    dbg_wid->show();
	    dbg_ptr = new QPainter(dbg_wid);
	    dbg_ptr->setBrush(Qt::NoBrush);
	}
	dbg_ptr->fillRect(dbg_wid->rect(),Qt::white);
    }
#endif

    void doSpans(int n, QPoint* pt, int* w)
    {
	int cs = canvas->chunkSize();
	for (int j=0; j<n; j++) {
	    int y = pt[j].y()/cs-bounds.y();
	    if (y >= bitmap.height() || y < 0) continue;
	    uchar* l = bitmap.scanLine(y);
	    int x = pt[j].x();
	    int x1 = x/cs-bounds.x();
	    if (x1 > bounds.width()) continue;
	    x1  = QMAX(0,x1);
	    int x2 = (x+w[j])/cs-bounds.x();
	    if (x2 < 0) continue;
	    x2 = QMIN(bounds.width(), x2);
	    int x1q = x1/8;
	    int x1r = x1%8;
	    int x2q = x2/8;
	    int x2r = x2%8;
#ifdef QCANVAS_POLYGONS_DEBUG
	    if ( dbg_ptr ) dbg_ptr->setPen(Qt::yellow);
#endif
	    if ( x1q == x2q ) {
		uchar newbits = (~l[x1q]) & (((2<<(x2r-x1r))-1)<<x1r);
		if ( newbits ) {
#ifdef QCANVAS_POLYGONS_DEBUG
		    if ( dbg_ptr ) dbg_ptr->setPen(Qt::darkGreen);
#endif
		    addBits(x1r,x2r,newbits,x1q*8,y);
		    l[x1q] |= newbits;
		}
	    } else {
#ifdef QCANVAS_POLYGONS_DEBUG
		if ( dbg_ptr ) dbg_ptr->setPen(Qt::blue);
#endif
		uchar newbits1 = (~l[x1q]) & (0xff<<x1r);
		if ( newbits1 ) {
#ifdef QCANVAS_POLYGONS_DEBUG
		    if ( dbg_ptr ) dbg_ptr->setPen(Qt::green);
#endif
		    addBits(x1r,7,newbits1,x1q*8,y);
		    l[x1q] |= newbits1;
		}
		for (int i=x1q+1; i<x2q; i++) {
		    if ( l[i] != 0xff ) {
			addBits(0,7,~l[i],i*8,y);
			l[i]=0xff;
		    }
		}
		uchar newbits2 = (~l[x2q]) & (0xff>>(7-x2r));
		if ( newbits2 ) {
#ifdef QCANVAS_POLYGONS_DEBUG
		    if ( dbg_ptr ) dbg_ptr->setPen(Qt::red);
#endif
		    addBits(0,x2r,newbits2,x2q*8,y);
		    l[x2q] |= newbits2;
		}
	    }
#ifdef QCANVAS_POLYGONS_DEBUG
	    if ( dbg_ptr ) {
		dbg_ptr->drawLine(pt[j],pt[j]+QPoint(w[j],0));
	    }
#endif
	}
	result.resize(pnt);
    }

    int pnt;
    QPointArray result;
    QCanvas* canvas;
    QRect bounds;
    QImage bitmap;
};


QPointArray QCanvasPolygonalItem::chunks() const
{
    QPointArray pa = areaPoints();

    if ( !pa.size() ) {
	pa.detach(); // Explicit sharing is stupid.
	return pa;
    }

    QPolygonalProcessor processor(canvas(),pa);

    scanPolygon(pa, wind, processor);

    return processor.result;
}
/*!
    Simply calls QCanvasItem::chunks().
*/
QPointArray QCanvasRectangle::chunks() const
{
    // No need to do a polygon scan!
    return QCanvasItem::chunks();
}

/*!
    Returns the bounding rectangle of the polygonal item, based on
    areaPoints().
*/
QRect QCanvasPolygonalItem::boundingRect() const
{
    return areaPoints().boundingRect();
}

/*!
    Reimplemented from QCanvasItem, this draws the polygonal item by
    setting the pen and brush for the item on the painter \a p and
    calling drawShape().
*/
void QCanvasPolygonalItem::draw(QPainter & p)
{
    p.setPen(pn);
    p.setBrush(br);
    drawShape(p);
}

/*!
    \fn void QCanvasPolygonalItem::drawShape(QPainter & p)

    Subclasses must reimplement this function to draw their shape. The
    pen and brush of \a p are already set to pen() and brush() prior
    to calling this function.

    \warning When you reimplement this function, make sure that you
    leave the painter in the same state as you found it. For example,
    if you start by calling QPainter::translate(50, 50), end your
    code by calling QPainter::translate(-50, -50). Be also aware that
    the painter might already have some transformations set (i.e.,
    don't call QPainter::resetXForm() when you're done).

    \sa draw()
*/

/*!
    \fn QPen QCanvasPolygonalItem::pen() const

    Returns the QPen used to draw the outline of the item, if any.

    \sa setPen()
*/

/*!
    \fn QBrush QCanvasPolygonalItem::brush() const

    Returns the QBrush used to fill the item, if filled.

    \sa setBrush()
*/

/*!
    Sets the QPen used when drawing the item to the pen \a p.
    Note that many QCanvasPolygonalItems do not use the pen value.

    \sa setBrush(), pen(), drawShape()
*/
void QCanvasPolygonalItem::setPen(QPen p)
{
    if ( pn != p ) {
	removeFromChunks();
	pn = p;
	addToChunks();
    }
}

/*!
    Sets the QBrush used when drawing the polygonal item to the brush \a b.

    \sa setPen(), brush(), drawShape()
*/
void QCanvasPolygonalItem::setBrush(QBrush b)
{
    if ( br != b) {
	br = b;
	changeChunks();
    }
}


/*!
    \class QCanvasPolygon qcanvas.h
    \brief The QCanvasPolygon class provides a polygon on a QCanvas.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module canvas
    \ingroup graphics
    \ingroup images

    Paints a polygon with a QBrush. The polygon's points can be set in
    the constructor or set or changed later using setPoints(). Use
    points() to retrieve the points, or areaPoints() to retrieve the
    points relative to the canvas's origin.

    The polygon can be drawn on a painter with drawShape().

    Like any other canvas item polygons can be moved with
    QCanvasItem::move() and QCanvasItem::moveBy(), or by setting
    coordinates with QCanvasItem::setX(), QCanvasItem::setY() and
    QCanvasItem::setZ().

    Note: QCanvasPolygon does not use the pen.
*/

/*!
    Constructs a point-less polygon on the canvas \a canvas. You
    should call setPoints() before using it further.
*/
QCanvasPolygon::QCanvasPolygon(QCanvas* canvas) :
    QCanvasPolygonalItem(canvas)
{
}

/*!
    Destroys the polygon.
*/
QCanvasPolygon::~QCanvasPolygon()
{
    hide();
}

/*!
    Draws the polygon using the painter \a p.

    Note that QCanvasPolygon does not support an outline (the pen is
    always NoPen).
*/
void QCanvasPolygon::drawShape(QPainter & p)
{
    // ### why can't we draw outlines? We could use drawPolyline for it. Lars
    // ### see other message. Warwick

    p.setPen(NoPen); // since QRegion(QPointArray) excludes outline :-(  )-:
    p.drawPolygon(poly);
}

/*!
    Sets the points of the polygon to be \a pa. These points will have
    their x and y coordinates automatically translated by x(), y() as
    the polygon is moved.
*/
void QCanvasPolygon::setPoints(QPointArray pa)
{
    removeFromChunks();
    poly = pa;
    poly.detach(); // Explicit sharing is stupid.
    poly.translate((int)x(),(int)y());
    addToChunks();
}

/*!
  \reimp
*/
void QCanvasPolygon::moveBy(double dx, double dy)
{
    // Note: does NOT call QCanvasPolygonalItem::moveBy(), since that
    // only does half this work.
    //
    int idx = int(x()+dx)-int(x());
    int idy = int(y()+dy)-int(y());
    if ( idx || idy ) {
	removeFromChunks();
	poly.translate(idx,idy);
    }
    myx+=dx;
    myy+=dy;
    if ( idx || idy ) {
	addToChunks();
    }
}

/*!
    \class QCanvasSpline qcanvas.h
    \brief The QCanvasSpline class provides multi-bezier splines on a QCanvas.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module canvas
    \ingroup graphics
    \ingroup images

    A QCanvasSpline is a sequence of 4-point bezier curves joined
    together to make a curved shape.

    You set the control points of the spline with setControlPoints().

    If the bezier is closed(), then the first control point will be
    re-used as the last control point. Therefore, a closed bezier must
    have a multiple of 3 control points and an open bezier must have
    one extra point.

    The beziers are not necessarily joined "smoothly". To ensure this,
    set control points appropriately (general reference texts about
    beziers will explain this in detail).

    Like any other canvas item splines can be moved with
    QCanvasItem::move() and QCanvasItem::moveBy(), or by setting
    coordinates with QCanvasItem::setX(), QCanvasItem::setY() and
    QCanvasItem::setZ().

*/

/*!
    Create a spline with no control points on the canvas \a canvas.

    \sa setControlPoints()
*/
QCanvasSpline::QCanvasSpline(QCanvas* canvas) :
    QCanvasPolygon(canvas),
    cl(TRUE)
{
}

/*!
    Destroy the spline.
*/
QCanvasSpline::~QCanvasSpline()
{
}

// ### shouldn't we handle errors more gracefully than with an assert? Lars
// ### no, since it's a programming error. Warwick
/*!
    Set the spline control points to \a ctrl.

    If \a close is TRUE, then the first point in \a ctrl will be
    re-used as the last point, and the number of control points must
    be a multiple of 3. If \a close is FALSE, one additional control
    point is required, and the number of control points must be one of
    (4, 7, 10, 13, ...).

    If the number of control points doesn't meet the above conditions,
    the number of points will be truncated to the largest number of
    points that do meet the requirement.
*/
void QCanvasSpline::setControlPoints(QPointArray ctrl, bool close)
{
    if ( (int)ctrl.count() % 3 != (close ? 0 : 1) ) {
	qWarning( "QCanvasSpline::setControlPoints(): Number of points doesn't fit." );
	int numCurves = (ctrl.count() - (close ? 0 : 1 ))/ 3;
	ctrl.resize( numCurves*3 + ( close ? 0 : 1 ) );
    }

    cl = close;
    bez = ctrl;
    recalcPoly();
}

/*!
    Returns the current set of control points.

    \sa setControlPoints(), closed()
*/
QPointArray QCanvasSpline::controlPoints() const
{
    return bez;
}

/*!
    Returns TRUE if the control points are a closed set; otherwise
    returns FALSE.
*/
bool QCanvasSpline::closed() const
{
    return cl;
}

void QCanvasSpline::recalcPoly()
{
    QPtrList<QPointArray> segs;
    segs.setAutoDelete(TRUE);
    int n=0;
    for (int i=0; i<(int)bez.count()-1; i+=3) {
	QPointArray ctrl(4);
	ctrl[0] = bez[i+0];
	ctrl[1] = bez[i+1];
	ctrl[2] = bez[i+2];
	if ( cl )
	    ctrl[3] = bez[(i+3)%(int)bez.count()];
	else
	    ctrl[3] = bez[i+3];
	QPointArray *seg = new QPointArray(ctrl.cubicBezier());
	n += seg->count()-1;
	segs.append(seg);
    }
    QPointArray p(n+1);
    n=0;
    for (QPointArray* seg = segs.first(); seg; seg = segs.next()) {
	for (int i=0; i<(int)seg->count()-1; i++)
	    p[n++] = seg->point(i);
	if ( n == (int)p.count()-1 )
	    p[n] = seg->point(seg->count()-1);
    }
    QCanvasPolygon::setPoints(p);
}

/*!
    \fn QPointArray QCanvasPolygonalItem::areaPoints() const

    This function must be reimplemented by subclasses. It \e must
    return the points bounding (i.e. outside and not touching) the
    shape or drawing errors will occur.
*/

/*!
    \fn QPointArray QCanvasPolygon::points() const

    Returns the vertices of the polygon, not translated by the position.

    \sa setPoints(), areaPoints()
*/
QPointArray QCanvasPolygon::points() const
{
    QPointArray pa = areaPoints();
    pa.translate(int(-x()),int(-y()));
    return pa;
}

/*!
    Returns the vertices of the polygon translated by the polygon's
    current x(), y() position, i.e. relative to the canvas's origin.

    \sa setPoints(), points()
*/
QPointArray QCanvasPolygon::areaPoints() const
{
    return poly.copy();
}

// ### mark: Why don't we offer a constructor that lets the user set the
// points -- that way for some uses just the constructor call would be
// required?
/*!
    \class QCanvasLine qcanvas.h
    \brief The QCanvasLine class provides a line on a QCanvas.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module canvas
    \ingroup graphics
    \ingroup images

    The line inherits functionality from QCanvasPolygonalItem, for
    example the setPen() function. The start and end points of the
    line are set with setPoints().

    Like any other canvas item lines can be moved with
    QCanvasItem::move() and QCanvasItem::moveBy(), or by setting
    coordinates with QCanvasItem::setX(), QCanvasItem::setY() and
    QCanvasItem::setZ().
*/

/*!
    Constructs a line from (0,0) to (0,0) on \a canvas.

    \sa setPoints().
*/
QCanvasLine::QCanvasLine(QCanvas* canvas) :
    QCanvasPolygonalItem(canvas)
{
    x1 = y1 = x2 = y2 = 0;
}

/*!
    Destroys the line.
*/
QCanvasLine::~QCanvasLine()
{
    hide();
}

/*!
  \reimp
*/
void QCanvasLine::setPen(QPen p)
{
    QCanvasPolygonalItem::setPen(p);
}

/*!
    \fn QPoint QCanvasLine::startPoint () const

    Returns the start point of the line.

    \sa setPoints(), endPoint()
*/

/*!
    \fn QPoint QCanvasLine::endPoint () const

    Returns the end point of the line.

    \sa setPoints(), startPoint()
*/

/*!
    Sets the line's start point to (\a xa, \a ya) and its end point to
    (\a xb, \a yb).
*/
void QCanvasLine::setPoints(int xa, int ya, int xb, int yb)
{
    if ( x1 != xa || x2 != xb || y1 != ya || y2 != yb ) {
	removeFromChunks();
	x1 = xa;
	y1 = ya;
	x2 = xb;
	y2 = yb;
	addToChunks();
    }
}

/*!
  \reimp
*/
void QCanvasLine::drawShape(QPainter &p)
{
    p.drawLine((int)(x()+x1), (int)(y()+y1), (int)(x()+x2), (int)(y()+y2));
}

/*!
    \reimp

    Note that the area defined by the line is somewhat thicker than
    the line that is actually drawn.
*/
QPointArray QCanvasLine::areaPoints() const
{
    QPointArray p(4);
    int xi = int(x());
    int yi = int(y());
    int pw = pen().width();
    int dx = QABS(x1-x2);
    int dy = QABS(y1-y2);
    pw = pw*4/3+2; // approx pw*sqrt(2)
    int px = x1<x2 ? -pw : pw ;
    int py = y1<y2 ? -pw : pw ;
    if ( dx && dy && (dx > dy ? (dx*2/dy <= 2) : (dy*2/dx <= 2)) ) {
	// steep
	if ( px == py ) {
	    p[0] = QPoint(x1+xi   ,y1+yi+py);
	    p[1] = QPoint(x2+xi-px,y2+yi   );
	    p[2] = QPoint(x2+xi   ,y2+yi-py);
	    p[3] = QPoint(x1+xi+px,y1+yi   );
	} else {
	    p[0] = QPoint(x1+xi+px,y1+yi   );
	    p[1] = QPoint(x2+xi   ,y2+yi-py);
	    p[2] = QPoint(x2+xi-px,y2+yi   );
	    p[3] = QPoint(x1+xi   ,y1+yi+py);
	}
    } else if ( dx > dy ) {
	// horizontal
	p[0] = QPoint(x1+xi+px,y1+yi+py);
	p[1] = QPoint(x2+xi-px,y2+yi+py);
	p[2] = QPoint(x2+xi-px,y2+yi-py);
	p[3] = QPoint(x1+xi+px,y1+yi-py);
    } else {
	// vertical
	p[0] = QPoint(x1+xi+px,y1+yi+py);
	p[1] = QPoint(x2+xi+px,y2+yi-py);
	p[2] = QPoint(x2+xi-px,y2+yi-py);
	p[3] = QPoint(x1+xi-px,y1+yi+py);
    }
    return p;
}

/*!
    \reimp

*/

void QCanvasLine::moveBy(double dx, double dy)
{
    QCanvasPolygonalItem::moveBy(dx, dy);
}

/*!
    \class QCanvasRectangle qcanvas.h
    \brief The QCanvasRectangle class provides a rectangle on a QCanvas.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module canvas
    \ingroup graphics
    \ingroup images

    This item paints a single rectangle which may have any pen() and
    brush(), but may not be tilted/rotated. For rotated rectangles,
    use QCanvasPolygon.

    The rectangle's size and initial position can be set in the
    constructor. The size can be set or changed later using setSize().
    Use height() and width() to retrieve the rectangle's dimensions.

    The rectangle can be drawn on a painter with drawShape().

    Like any other canvas item rectangles can be moved with
    QCanvasItem::move() and QCanvasItem::moveBy(), or by setting
    coordinates with QCanvasItem::setX(), QCanvasItem::setY() and
    QCanvasItem::setZ().

*/

/*!
    Constructs a rectangle at position (0,0) with both width and
    height set to 32 pixels on \a canvas.
*/
QCanvasRectangle::QCanvasRectangle(QCanvas* canvas) :
    QCanvasPolygonalItem(canvas),
    w(32), h(32)
{
}

/*!
    Constructs a rectangle positioned and sized by \a r on \a canvas.
*/
QCanvasRectangle::QCanvasRectangle(const QRect& r, QCanvas* canvas) :
    QCanvasPolygonalItem(canvas),
    w(r.width()), h(r.height())
{
    move(r.x(),r.y());
}

/*!
    Constructs a rectangle at position (\a x, \a y) and size \a width
    by \a height, on \a canvas.
*/
QCanvasRectangle::QCanvasRectangle(int x, int y, int width, int height,
	QCanvas* canvas) :
    QCanvasPolygonalItem(canvas),
    w(width), h(height)
{
    move(x,y);
}

/*!
    Destroys the rectangle.
*/
QCanvasRectangle::~QCanvasRectangle()
{
    hide();
}


/*!
    Returns the width of the rectangle.
*/
int QCanvasRectangle::width() const
{
    return w;
}

/*!
    Returns the height of the rectangle.
*/
int QCanvasRectangle::height() const
{
    return h;
}

/*!
    Sets the \a width and \a height of the rectangle.
*/
void QCanvasRectangle::setSize(int width, int height)
{
    if ( w != width || h != height ) {
	removeFromChunks();
	w = width;
	h = height;
	addToChunks();
    }
}

/*!
    \fn QSize QCanvasRectangle::size() const

    Returns the width() and height() of the rectangle.

    \sa rect(), setSize()
*/

/*!
    \fn QRect QCanvasRectangle::rect() const

    Returns the integer-converted x(), y() position and size() of the
    rectangle as a QRect.
*/

/*!
  \reimp
*/
QPointArray QCanvasRectangle::areaPoints() const
{
    QPointArray pa(4);
    int pw = (pen().width()+1)/2;
    if ( pw < 1 ) pw = 1;
    if ( pen() == NoPen ) pw = 0;
    pa[0] = QPoint((int)x()-pw,(int)y()-pw);
    pa[1] = pa[0] + QPoint(w+pw*2,0);
    pa[2] = pa[1] + QPoint(0,h+pw*2);
    pa[3] = pa[0] + QPoint(0,h+pw*2);
    return pa;
}

/*!
    Draws the rectangle on painter \a p.
*/
void QCanvasRectangle::drawShape(QPainter & p)
{
    p.drawRect((int)x(), (int)y(), w, h);
}


/*!
    \class QCanvasEllipse qcanvas.h
    \brief The QCanvasEllipse class provides an ellipse or ellipse segment on a QCanvas.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module canvas
    \ingroup graphics
    \ingroup images

    A canvas item that paints an ellipse or ellipse segment with a QBrush.
    The ellipse's height, width, start angle and angle length can be set
    at construction time. The size can be changed at runtime with
    setSize(), and the angles can be changed (if you're displaying an
    ellipse segment rather than a whole ellipse) with setAngles().

    Note that angles are specified in 16ths of a degree.

    \target anglediagram
    \img qcanvasellipse.png Ellipse

    If a start angle and length angle are set then an ellipse segment
    will be drawn. The start angle is the angle that goes from zero in a
    counter-clockwise direction (shown in green in the diagram). The
    length angle is the angle from the start angle in a
    counter-clockwise direction (shown in blue in the diagram). The blue
    segment is the segment of the ellipse that would be drawn. If no
    start angle and length angle are specified the entire ellipse is
    drawn.

    The ellipse can be drawn on a painter with drawShape().

    Like any other canvas item ellipses can be moved with move() and
    moveBy(), or by setting coordinates with setX(), setY() and setZ().

    Note: QCanvasEllipse does not use the pen.
*/

/*!
    Constructs a 32x32 ellipse, centered at (0, 0) on \a canvas.
*/
QCanvasEllipse::QCanvasEllipse(QCanvas* canvas) :
    QCanvasPolygonalItem(canvas),
    w(32), h(32),
    a1(0), a2(360*16)
{
}

/*!
    Constructs a \a width by \a height pixel ellipse, centered at
    (0, 0) on \a canvas.
*/
QCanvasEllipse::QCanvasEllipse(int width, int height, QCanvas* canvas) :
    QCanvasPolygonalItem(canvas),
    w(width),h(height),
    a1(0),a2(360*16)
{
}

// ### add a constructor taking degrees in float. 1/16 degrees is stupid. Lars
// ### it's how QPainter does it, so QCanvas does too for consistency. If it's
// ###  a good idea, it should be added to QPainter, not just to QCanvas. Warwick
/*!
    Constructs a \a width by \a height pixel ellipse, centered at
    (0, 0) on \a canvas. Only a segment of the ellipse is drawn,
    starting at angle \a startangle, and extending for angle \a angle
    (the angle length).

    Note that angles are specified in
    <small><sup>1</sup>/<sub>16</sub></small>ths of a degree.
*/
QCanvasEllipse::QCanvasEllipse(int width, int height,
    int startangle, int angle, QCanvas* canvas) :
    QCanvasPolygonalItem(canvas),
    w(width),h(height),
    a1(startangle),a2(angle)
{
}

/*!
    Destroys the ellipse.
*/
QCanvasEllipse::~QCanvasEllipse()
{
    hide();
}

/*!
    Returns the width of the ellipse.
*/
int QCanvasEllipse::width() const
{
    return w;
}

/*!
    Returns the height of the ellipse.
*/
int QCanvasEllipse::height() const
{
    return h;
}

/*!
    Sets the \a width and \a height of the ellipse.
*/
void QCanvasEllipse::setSize(int width, int height)
{
    if ( w != width || h != height ) {
	removeFromChunks();
	w = width;
	h = height;
	addToChunks();
    }
}

/*!
    \fn int QCanvasEllipse::angleStart() const

    Returns the start angle in 16ths of a degree. Initially
    this will be 0.

    \sa setAngles(), angleLength()
*/

/*!
    \fn int QCanvasEllipse::angleLength() const

    Returns the length angle (the extent of the ellipse segment) in
    16ths of a degree. Initially this will be 360 * 16 (a complete
    ellipse).

    \sa setAngles(), angleStart()
*/

/*!
    Sets the angles for the ellipse. The start angle is \a start and
    the extent of the segment is \a length (the angle length) from the
    \a start. The angles are specified in 16ths of a degree. By
    default the ellipse will start at 0 and have an angle length of
    360 * 16 (a complete ellipse).

    \sa angleStart(), angleLength()
*/
void QCanvasEllipse::setAngles(int start, int length)
{
    if ( a1 != start || a2 != length ) {
	removeFromChunks();
	a1 = start;
	a2 = length;
	addToChunks();
    }
}

/*!
  \reimp
*/
QPointArray QCanvasEllipse::areaPoints() const
{
    QPointArray r;
    // makeArc at 0,0, then translate so that fixed point math doesn't overflow
    r.makeArc(int(x()-w/2.0+0.5)-1, int(y()-h/2.0+0.5)-1, w+3, h+3, a1, a2);
    r.resize(r.size()+1);
    r.setPoint(r.size()-1,int(x()),int(y()));
    return r;
}

// ### support outlines! Lars
// ### QRegion doesn't, so we cannot (try it). Warwick
/*!
    Draws the ellipse, centered at x(), y() using the painter \a p.

    Note that QCanvasEllipse does not support an outline (the pen is
    always NoPen).
*/
void QCanvasEllipse::drawShape(QPainter & p)
{
    p.setPen(NoPen); // since QRegion(QPointArray) excludes outline :-(  )-:
    if ( !a1 && a2 == 360*16 ) {
	p.drawEllipse(int(x()-w/2.0+0.5), int(y()-h/2.0+0.5), w, h);
    } else {
	p.drawPie(int(x()-w/2.0+0.5), int(y()-h/2.0+0.5), w, h, a1, a2);
    }
}


/*!
    \class QCanvasText qcanvas.h
    \brief The QCanvasText class provides a text object on a QCanvas.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module canvas
    \ingroup graphics
    \ingroup images

    A canvas text item has text with font, color and alignment
    attributes. The text and font can be set in the constructor or set
    or changed later with setText() and setFont(). The color is set
    with setColor() and the alignment with setTextFlags(). The text
    item's bounding rectangle is retrieved with boundingRect().

    The text can be drawn on a painter with draw().

    Like any other canvas item text items can be moved with
    QCanvasItem::move() and QCanvasItem::moveBy(), or by setting
    coordinates with QCanvasItem::setX(), QCanvasItem::setY() and
    QCanvasItem::setZ().
*/

/*!
    Constructs a QCanvasText with the text "\<text\>", on \a canvas.
*/
QCanvasText::QCanvasText(QCanvas* canvas) :
    QCanvasItem(canvas),
    txt("<text>"), flags(0)
{
    setRect();
}

// ### add textflags to the constructor? Lars
/*!
    Constructs a QCanvasText with the text \a t, on canvas \a canvas.
*/
QCanvasText::QCanvasText(const QString& t, QCanvas* canvas) :
    QCanvasItem(canvas),
    txt(t), flags(0)
{
    setRect();
}

// ### see above
/*!
    Constructs a QCanvasText with the text \a t and font \a f, on the
    canvas \a canvas.
*/
QCanvasText::QCanvasText(const QString& t, QFont f, QCanvas* canvas) :
    QCanvasItem(canvas),
    txt(t), flags(0),
    fnt(f)
{
    setRect();
}

/*!
    Destroys the canvas text item.
*/
QCanvasText::~QCanvasText()
{
    removeFromChunks();
}

/*!
    Returns the bounding rectangle of the text.
*/
QRect QCanvasText::boundingRect() const { return brect; }

void QCanvasText::setRect()
{
    brect = QFontMetrics(fnt).boundingRect(int(x()), int(y()), 0, 0, flags, txt);
    brect.setWidth(brect.width()+1);
}

/*!
    \fn int QCanvasText::textFlags() const

    Returns the currently set alignment flags.

    \sa setTextFlags() Qt::AlignmentFlags
*/


/*!
    Sets the alignment flags to \a f. These are a bitwise OR of the
    flags available to QPainter::drawText() -- see the
    \l{Qt::AlignmentFlags}.

    \sa setFont() setColor()
*/
void QCanvasText::setTextFlags(int f)
{
    if ( flags != f ) {
	removeFromChunks();
	flags = f;
	setRect();
	addToChunks();
    }
}

/*!
    Returns the text item's text.

    \sa setText()
*/
QString QCanvasText::text() const
{
    return txt;
}


/*!
    Sets the text item's text to \a t. The text may contain newlines.

    \sa text(), setFont(), setColor() setTextFlags()
*/
void QCanvasText::setText( const QString& t )
{
    if ( txt != t ) {
	removeFromChunks();
	txt = t;
	setRect();
	addToChunks();
    }
}

/*!
    Returns the font in which the text is drawn.

    \sa setFont()
*/
QFont QCanvasText::font() const
{
    return fnt;
}

/*!
    Sets the font in which the text is drawn to font \a f.

    \sa font()
*/
void QCanvasText::setFont( const QFont& f )
{
    if ( f != fnt ) {
	removeFromChunks();
	fnt = f;
	setRect();
	addToChunks();
    }
}

/*!
    Returns the color of the text.

    \sa setColor()
*/
QColor QCanvasText::color() const
{
    return col;
}

/*!
    Sets the color of the text to the color \a c.

    \sa color(), setFont()
*/
void QCanvasText::setColor(const QColor& c)
{
    col=c;
    changeChunks();
}


/*!
  \reimp
*/
void QCanvasText::moveBy(double dx, double dy)
{
    int idx = int(x()+dx)-int(x());
    int idy = int(y()+dy)-int(y());
    if ( idx || idy ) {
	removeFromChunks();
    }
    myx+=dx;
    myy+=dy;
    if ( idx || idy ) {
	brect.moveBy(idx,idy);
	addToChunks();
    }
}

/*!
    Draws the text using the painter \a painter.
*/
void QCanvasText::draw(QPainter& painter)
{
    painter.setFont(fnt);
    painter.setPen(col);
    painter.drawText(brect, flags, txt);
}

/*!
  \reimp
*/
void QCanvasText::changeChunks()
{
    if (isVisible() && canvas()) {
	int chunksize=canvas()->chunkSize();
	for (int j=brect.top()/chunksize; j<=brect.bottom()/chunksize; j++) {
	    for (int i=brect.left()/chunksize; i<=brect.right()/chunksize; i++) {
		canvas()->setChangedChunk(i,j);
	    }
	}
    }
}

/*!
    Adds the text item to the appropriate chunks.
*/
void QCanvasText::addToChunks()
{
    if (isVisible() && canvas()) {
	int chunksize=canvas()->chunkSize();
	for (int j=brect.top()/chunksize; j<=brect.bottom()/chunksize; j++) {
	    for (int i=brect.left()/chunksize; i<=brect.right()/chunksize; i++) {
		canvas()->addItemToChunk(this,i,j);
	    }
	}
    }
}

/*!
    Removes the text item from the appropriate chunks.
*/
void QCanvasText::removeFromChunks()
{
    if (isVisible() && canvas()) {
	int chunksize=canvas()->chunkSize();
	for (int j=brect.top()/chunksize; j<=brect.bottom()/chunksize; j++) {
	    for (int i=brect.left()/chunksize; i<=brect.right()/chunksize; i++) {
		canvas()->removeItemFromChunk(this,i,j);
	    }
	}
    }
}


/*!
    Returns 0 (QCanvasItem::Rtti_Item).

    Make your derived classes return their own values for rtti(), so
    that you can distinguish between objects returned by
    QCanvas::at(). You should use values greater than 1000 to allow
    for extensions to this class.

    Overuse of this functionality can damage it's extensibility. For
    example, once you have identified a base class of a QCanvasItem
    found by QCanvas::at(), cast it to that type and call meaningful
    methods rather than acting upon the object based on its rtti
    value.

    For example:

    \code
	QCanvasItem* item;
	// Find an item, e.g. with QCanvasItem::collisions().
	...
	if (item->rtti() == MySprite::RTTI ) {
	    MySprite* s = (MySprite*)item;
	    if (s->isDamagable()) s->loseHitPoints(1000);
	    if (s->isHot()) myself->loseHitPoints(1000);
	    ...
	}
    \endcode
*/
int QCanvasItem::rtti() const { return RTTI; }
int QCanvasItem::RTTI = Rtti_Item;

/*!
    Returns 1 (QCanvasItem::Rtti_Sprite).

    \sa QCanvasItem::rtti()
*/
int QCanvasSprite::rtti() const { return RTTI; }
int QCanvasSprite::RTTI = Rtti_Sprite;

/*!
    Returns 2 (QCanvasItem::Rtti_PolygonalItem).

    \sa QCanvasItem::rtti()
*/
int QCanvasPolygonalItem::rtti() const { return RTTI; }
int QCanvasPolygonalItem::RTTI = Rtti_PolygonalItem;

/*!
    Returns 3 (QCanvasItem::Rtti_Text).

    \sa QCanvasItem::rtti()
*/
int QCanvasText::rtti() const { return RTTI; }
int QCanvasText::RTTI = Rtti_Text;

/*!
    Returns 4 (QCanvasItem::Rtti_Polygon).

    \sa QCanvasItem::rtti()
*/
int QCanvasPolygon::rtti() const { return RTTI; }
int QCanvasPolygon::RTTI = Rtti_Polygon;

/*!
    Returns 5 (QCanvasItem::Rtti_Rectangle).

    \sa QCanvasItem::rtti()
*/
int QCanvasRectangle::rtti() const { return RTTI; }
int QCanvasRectangle::RTTI = Rtti_Rectangle;

/*!
    Returns 6 (QCanvasItem::Rtti_Ellipse).

    \sa QCanvasItem::rtti()
*/
int QCanvasEllipse::rtti() const { return RTTI; }
int QCanvasEllipse::RTTI = Rtti_Ellipse;

/*!
    Returns 7 (QCanvasItem::Rtti_Line).

    \sa QCanvasItem::rtti()
*/
int QCanvasLine::rtti() const { return RTTI; }
int QCanvasLine::RTTI = Rtti_Line;

/*!
    Returns 8 (QCanvasItem::Rtti_Spline).

    \sa QCanvasItem::rtti()
*/
int QCanvasSpline::rtti() const { return RTTI; }
int QCanvasSpline::RTTI = Rtti_Spline;

/*!
    Constructs a QCanvasSprite which uses images from the
    QCanvasPixmapArray \a a.

    The sprite in initially positioned at (0, 0) on \a canvas, using
    frame 0.
*/
QCanvasSprite::QCanvasSprite(QCanvasPixmapArray* a, QCanvas* canvas) :
    QCanvasItem(canvas),
    frm(0),
    anim_val(0),
    anim_state(0),
    anim_type(0),
    images(a)
{
}


/*!
    Set the array of images used for displaying the sprite to the
    QCanvasPixmapArray \a a.

    If the current frame() is larger than the number of images in \a
    a, the current frame will be reset to 0.
*/
void QCanvasSprite::setSequence(QCanvasPixmapArray* a)
{
    bool isvisible = isVisible();
    if ( isvisible && images )
	hide();
    images = a;
    if ( frm >= (int)images->count() )
	frm = 0;
    if ( isvisible )
	show();
}

/*!
\internal

Marks any chunks the sprite touches as changed.
*/
void QCanvasSprite::changeChunks()
{
    if (isVisible() && canvas()) {
	int chunksize=canvas()->chunkSize();
	for (int j=topEdge()/chunksize; j<=bottomEdge()/chunksize; j++) {
	    for (int i=leftEdge()/chunksize; i<=rightEdge()/chunksize; i++) {
		canvas()->setChangedChunk(i,j);
	    }
	}
    }
}

/*!
    Destroys the sprite and removes it from the canvas. Does \e not
    delete the images.
*/
QCanvasSprite::~QCanvasSprite()
{
    removeFromChunks();
}

/*!
    Sets the animation frame used for displaying the sprite to \a f,
    an index into the QCanvasSprite's QCanvasPixmapArray. The call
    will be ignored if \a f is larger than frameCount() or smaller
    than 0.

    \sa frame() move()
*/
void QCanvasSprite::setFrame(int f)
{
    move(x(),y(),f);
}

/*!
    \enum QCanvasSprite::FrameAnimationType

    This enum is used to identify the different types of frame
    animation offered by QCanvasSprite.

    \value Cycle at each advance the frame number will be incremented by
    1 (modulo the frame count).
    \value Oscillate at each advance the frame number will be
    incremented by 1 up to the frame count then decremented to by 1 to
    0, repeating this sequence forever.
*/

/*!
    Sets the animation characteristics for the sprite.

    For \a type == \c Cycle, the frames will increase by \a step
    at each advance, modulo the frameCount().

    For \a type == \c Oscillate, the frames will increase by \a step
    at each advance, up to the frameCount(), then decrease by \a step
    back to 0, repeating forever.

    The \a state parameter is for internal use.
*/
void QCanvasSprite::setFrameAnimation(FrameAnimationType type, int step, int state)
{
    anim_val = step;
    anim_type = type;
    anim_state = state;
    setAnimated(TRUE);
}

/*!
    Extends the default QCanvasItem implementation to provide the
    functionality of setFrameAnimation().

    The \a phase is 0 or 1: see QCanvasItem::advance() for details.

    \sa QCanvasItem::advance() setVelocity()
*/
void QCanvasSprite::advance(int phase)
{
    if ( phase==1 ) {
	int nf = frame();
	if ( anim_type == Oscillate ) {
	    if ( anim_state )
		nf += anim_val;
	    else
		nf -= anim_val;
	    if ( nf < 0 ) {
		nf = abs(anim_val);
		anim_state = !anim_state;
	    } else if ( nf >= frameCount() ) {
		nf = frameCount()-1-abs(anim_val);
		anim_state = !anim_state;
	    }
	} else {
	    nf = (nf + anim_val + frameCount()) % frameCount();
	}
	move(x()+xVelocity(),y()+yVelocity(),nf);
    }
}


/*!
    \fn int QCanvasSprite::frame() const

    Returns the index of the current animation frame in the
    QCanvasSprite's QCanvasPixmapArray.

    \sa setFrame(), move()
*/

/*!
    \fn int QCanvasSprite::frameCount() const

    Returns the number of frames in the QCanvasSprite's
    QCanvasPixmapArray.
*/


/*!
  \reimp
  \internal
  Moves the sprite to the position \a x, \a y.
    Keep it visible.
*/
void QCanvasSprite::move(double x, double y) { QCanvasItem::move(x,y); }

/*!
    \fn void QCanvasSprite::move(double nx, double ny, int nf)

    Set the position of the sprite to \a nx, \a ny and the current
    frame to \a nf. \a nf will be ignored if it is larger than
    frameCount() or smaller than 0.
*/
void QCanvasSprite::move(double nx, double ny, int nf)
{
    if (isVisible() && canvas()) {
	hide();
	QCanvasItem::move(nx,ny);
	if ( nf >= 0 && nf < frameCount() )
	    frm=nf;
	show();
    } else {
	QCanvasItem::move(nx,ny);
	if ( nf >= 0 && nf < frameCount() )
	    frm=nf;
    }
}

class QCanvasPolygonScanner : public QPolygonScanner {
    QPolygonalProcessor& processor;
public:
    QCanvasPolygonScanner(QPolygonalProcessor& p) :
	processor(p)
    {
    }
    void processSpans( int n, QPoint* point, int* width )
    {
	processor.doSpans(n,point,width);
    }
};

void QCanvasPolygonalItem::scanPolygon(const QPointArray& pa, int winding, QPolygonalProcessor& process) const
{
    QCanvasPolygonScanner scanner(process);
    scanner.scan(pa,winding);
}


#endif // QT_NO_CANVAS
