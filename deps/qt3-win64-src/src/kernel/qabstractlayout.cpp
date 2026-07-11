/****************************************************************************
** $Id: qabstractlayout.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of the abstract layout base class
**
** Created : 960416
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

#include "qlayout.h"

#ifndef QT_NO_LAYOUT
#include "qapplication.h"
#include "qlayoutengine_p.h"
#include "qmenubar.h"
#include "qtoolbar.h"

static int menuBarHeightForWidth( QMenuBar *menubar, int w )
{
#ifndef QT_NO_MENUBAR
    if ( menubar && !menubar->isHidden() && !menubar->isTopLevel() )
	return menubar->heightForWidth( QMAX(w, menubar->minimumWidth()) );
    else
#endif
	return 0;
}

/*!
    \class QLayoutItem
    \ingroup appearance
    \ingroup geomanagement
    \brief The QLayoutItem class provides an abstract item that a
    QLayout manipulates.

    This is used by custom layouts.

    Pure virtual functions are provided to return information about
    the layout, including, sizeHint(), minimumSize(), maximumSize()
    and expanding().

    The layout's geometry can be set and retrieved with setGeometry()
    and geometry(), and its alignment with setAlignment() and
    alignment().

    isEmpty() returns whether the layout is empty. iterator() returns
    an iterator for the layout's children. If the concrete item is a
    QWidget, it can be retrieved using widget(). Similarly for
    layout() and spacerItem().

    \sa QLayout
*/

/*!
    \class QSpacerItem
    \ingroup appearance
    \ingroup geomanagement
    \brief The QSpacerItem class provides blank space in a layout.

    This class is used by custom layouts.

    \sa QLayout QLayout::spacerItem()
*/

/*!
    \class QWidgetItem
    \ingroup appearance
    \ingroup geomanagement
    \brief The QWidgetItem class is a layout item that represents a widget.

    This is used by custom layouts.

    \sa QLayout QLayout::widget()
*/

/*!
    \fn QLayoutItem::QLayoutItem( int alignment )

    Constructs a layout item with an \a alignment that is a bitwise OR
    of the \l{Qt::AlignmentFlags}. Not all subclasses support
    alignment.
*/

/*!
    \fn int QLayoutItem::alignment() const

    Returns the alignment of this item.
*/

/*!
    Sets the alignment of this item to \a a, which is a bitwise OR of
    the \l{Qt::AlignmentFlags}. Not all subclasses support alignment.
*/
void QLayoutItem::setAlignment( int a )
{
    align = a;
}

/*!
    \fn QSize QLayoutItem::maximumSize() const

    Implemented in subclasses to return the maximum size of this item.
*/

/*!
    \fn QSize QLayoutItem::minimumSize() const

    Implemented in subclasses to return the minimum size of this item.
*/

/*!
    \fn QSize QLayoutItem::sizeHint() const

    Implemented in subclasses to return the preferred size of this item.
*/

/*!
    \fn QSizePolicy::ExpandData QLayoutItem::expanding() const

    Implemented in subclasses to return the direction(s) this item
    "wants" to expand in (if any).
*/

/*!
    \fn void QLayoutItem::setGeometry( const QRect &r )

    Implemented in subclasses to set this item's geometry to \a r.
*/

/*!
    \fn QRect QLayoutItem::geometry() const

    Returns the rectangle covered by this layout item.
*/

/*!
    \fn virtual bool QLayoutItem::isEmpty() const

    Implemented in subclasses to return whether this item is empty,
    i.e. whether it contains any widgets.
*/

/*!
    \fn QSpacerItem::QSpacerItem( int w, int h, QSizePolicy::SizeType hData, QSizePolicy::SizeType vData )

    Constructs a spacer item with preferred width \a w, preferred
    height \a h, horizontal size policy \a hData and vertical size
    policy \a vData.

    The default values provide a gap that is able to stretch if
    nothing else wants the space.
*/

/*!
    Changes this spacer item to have preferred width \a w, preferred
    height \a h, horizontal size policy \a hData and vertical size
    policy \a vData.

    The default values provide a gap that is able to stretch if
    nothing else wants the space.
*/
void QSpacerItem::changeSize( int w, int h, QSizePolicy::SizeType hData,
			      QSizePolicy::SizeType vData )
{
    width = w;
    height = h;
    sizeP = QSizePolicy( hData, vData );
}

/*!
    \fn QWidgetItem::QWidgetItem (QWidget * w)

    Creates an item containing widget \a w.
*/

/*!
    Destroys the QLayoutItem.
*/
QLayoutItem::~QLayoutItem()
{
}

/*!
    Invalidates any cached information in this layout item.
*/
void QLayoutItem::invalidate()
{
}

/*!
    If this item is a QLayout, it is returned as a QLayout; otherwise
    0 is returned. This function provides type-safe casting.
*/
QLayout * QLayoutItem::layout()
{
    return 0;
}

/*!
    If this item is a QSpacerItem, it is returned as a QSpacerItem;
    otherwise 0 is returned. This function provides type-safe casting.
*/
QSpacerItem * QLayoutItem::spacerItem()
{
    return 0;
}

/*!
    \reimp
*/
QLayout * QLayout::layout()
{
    return this;
}

/*!
    \reimp
*/
QSpacerItem * QSpacerItem::spacerItem()
{
    return this;
}

/*!
    If this item is a QWidget, it is returned as a QWidget; otherwise
    0 is returned. This function provides type-safe casting.
*/
QWidget * QLayoutItem::widget()
{
    return 0;
}

/*!
    Returns the widget managed by this item.
*/
QWidget * QWidgetItem::widget()
{
    return wid;
}

/*!
    Returns TRUE if this layout's preferred height depends on its
    width; otherwise returns FALSE. The default implementation returns
    FALSE.

    Reimplement this function in layout managers that support height
    for width.

    \sa heightForWidth(), QWidget::heightForWidth()
*/
bool QLayoutItem::hasHeightForWidth() const
{
    return FALSE;
}

/*!
    Returns an iterator over this item's QLayoutItem children. The
    default implementation returns an empty iterator.

    Reimplement this function in subclasses that can have children.
*/
QLayoutIterator QLayoutItem::iterator()
{
    return QLayoutIterator( 0 );
}

/*!
    Returns the preferred height for this layout item, given the width
    \a w.

    The default implementation returns -1, indicating that the
    preferred height is independent of the width of the item. Using
    the function hasHeightForWidth() will typically be much faster
    than calling this function and testing for -1.

    Reimplement this function in layout managers that support height
    for width. A typical implementation will look like this:
    \code
	int MyLayout::heightForWidth( int w ) const
	{
	    if ( cache_dirty || cached_width != w ) {
		// not all C++ compilers support "mutable"
		MyLayout *that = (MyLayout*)this;
		int h = calculateHeightForWidth( w );
		that->cached_hfw = h;
		return h;
	    }
	    return cached_hfw;
	}
    \endcode

    Caching is strongly recommended; without it layout will take
    exponential time.

    \sa hasHeightForWidth()
*/
int QLayoutItem::heightForWidth( int /* w */ ) const
{
    return -1;
}

/*!
    Stores the spacer item's rect \a r so that it can be returned by
    geometry().
*/
void QSpacerItem::setGeometry( const QRect &r )
{
    rect = r;
}

/*!
    Sets the geometry of this item's widget to be contained within
    rect \a r, taking alignment and maximum size into account.
*/
void QWidgetItem::setGeometry( const QRect &r )
{
    QSize s = r.size().boundedTo( qSmartMaxSize( this ) );
    int x = r.x();
    int y = r.y();
    if ( align & (Qt::AlignHorizontal_Mask | Qt::AlignVertical_Mask) ) {
	QSize pref = wid->sizeHint().expandedTo( wid->minimumSize() ); //###
	if ( align & Qt::AlignHorizontal_Mask )
	    s.setWidth( QMIN( s.width(), pref.width() ) );
	if ( align & Qt::AlignVertical_Mask ) {
	    if ( hasHeightForWidth() )
		s.setHeight( QMIN( s.height(), heightForWidth(s.width()) ) );
	    else
		s.setHeight( QMIN( s.height(), pref.height() ) );
	}
    }
    int alignHoriz = QApplication::horizontalAlignment( align );
    if ( alignHoriz & Qt::AlignRight )
	x = x + ( r.width() - s.width() );
    else if ( !(alignHoriz & Qt::AlignLeft) )
	x = x + ( r.width() - s.width() ) / 2;

    if ( align & Qt::AlignBottom )
	y = y + ( r.height() - s.height() );
    else if ( !(align & Qt::AlignTop) )
	y = y + ( r.height() - s.height() ) / 2;

    if ( !isEmpty() )
	wid->setGeometry( x, y, s.width(), s.height() );
}

/*!
    \reimp
*/
QRect QSpacerItem::geometry() const
{
    return rect;
}

/*!
    \reimp
*/
QRect QWidgetItem::geometry() const
{
    return wid->geometry();
}

/*!
    \reimp
*/
QRect QLayout::geometry() const
{
    return rect;
}

/*!
    \reimp
*/
bool QWidgetItem::hasHeightForWidth() const
{
    if ( isEmpty() )
	return FALSE;
    if ( wid->layout() )
	return wid->layout()->hasHeightForWidth();
    return wid->sizePolicy().hasHeightForWidth();
}

/*!
    \reimp
*/
int QWidgetItem::heightForWidth( int w ) const
{
    if ( isEmpty() )
	return -1;
    int hfw;
    if ( wid->layout() )
	hfw = wid->layout()->totalHeightForWidth( w );
    else
	hfw = wid->heightForWidth( w );

    if ( hfw > wid->maximumHeight() )
	hfw = wid->maximumHeight();
    if ( hfw < wid->minimumHeight() )
	hfw = wid->minimumHeight();
    if ( hfw < 1 )
	hfw = 1;
    return hfw;
}

/*!
    Returns the direction in which this spacer item will expand.

    \sa QSizePolicy::ExpandData
*/
QSizePolicy::ExpandData QSpacerItem::expanding() const
{
    return sizeP.expanding();
}

/*!
    Returns whether this item's widget can make use of more space than
    sizeHint(). A value of \c Vertical or \c Horizontal means that it wants
    to grow in only one dimension, whereas \c BothDirections means that
    it wants to grow in both dimensions and \c NoDirection means that
    it doesn't want to grow at all.
*/
QSizePolicy::ExpandData QWidgetItem::expanding() const
{
    if ( isEmpty() )
	return QSizePolicy::NoDirection;

    int e = wid->sizePolicy().expanding();
    /*
      If the layout is expanding, we make the widget expanding, even if
      its own size policy isn't expanding. This behavior should be
      reconsidered in Qt 4.0. (###)
    */
    if ( wid->layout() ) {
	if ( wid->sizePolicy().mayGrowHorizontally()
		&& (wid->layout()->expanding() & QSizePolicy::Horizontally) )
	    e |= QSizePolicy::Horizontally;
	if ( wid->sizePolicy().mayGrowVertically()
		&& (wid->layout()->expanding() & QSizePolicy::Vertically) )
	    e |= QSizePolicy::Vertically;
    }

    if ( align & Qt::AlignHorizontal_Mask )
	e &= ~QSizePolicy::Horizontally;
    if ( align & Qt::AlignVertical_Mask)
	e &= ~QSizePolicy::Vertically;
    return (QSizePolicy::ExpandData)e;
}

/*!
    Returns the minimum size of this spacer item.
*/
QSize QSpacerItem::minimumSize() const
{
    return QSize( sizeP.mayShrinkHorizontally() ? 0 : width,
		  sizeP.mayShrinkVertically() ? 0 : height );
}

/*!
    Returns the minimum size of this item.
*/
QSize QWidgetItem::minimumSize() const
{
    if ( isEmpty() )
	return QSize( 0, 0 );
    return qSmartMinSize( this );
}

/*!
    Returns the maximum size of this spacer item.
*/
QSize QSpacerItem::maximumSize() const
{
    return QSize( sizeP.mayGrowHorizontally() ? QLAYOUTSIZE_MAX : width,
		  sizeP.mayGrowVertically() ? QLAYOUTSIZE_MAX : height );
}

/*!
    Returns the maximum size of this item.
*/
QSize QWidgetItem::maximumSize() const
{
    if ( isEmpty() ) {
	return QSize( 0, 0 );
    } else {
	return qSmartMaxSize( this, align );
    }
}

/*!
    Returns the preferred size of this spacer item.
*/
QSize QSpacerItem::sizeHint() const
{
    return QSize( width, height );
}

/*!
    Returns the preferred size of this item.
*/
QSize QWidgetItem::sizeHint() const
{
    QSize s;
    if ( isEmpty() ) {
	s = QSize( 0, 0 );
    } else {
	s = wid->sizeHint();
	if ( wid->sizePolicy().horData() == QSizePolicy::Ignored )
	    s.setWidth( 1 );
	if ( wid->sizePolicy().verData() == QSizePolicy::Ignored )
	    s.setHeight( 1 );
	s = s.boundedTo( wid->maximumSize() )
	    .expandedTo( wid->minimumSize() ).expandedTo( QSize(1, 1) );
    }
    return s;
}

/*!
    Returns TRUE because a spacer item never contains widgets.
*/
bool QSpacerItem::isEmpty() const
{
    return TRUE;
}

/*!
    Returns TRUE if the widget has been hidden; otherwise returns
    FALSE.
*/
bool QWidgetItem::isEmpty() const
{
    return wid->isHidden() || wid->isTopLevel();
}

/*!
    \class QLayout
    \brief The QLayout class is the base class of geometry managers.

    \ingroup appearance
    \ingroup geomanagement

    This is an abstract base class inherited by the concrete classes,
    QBoxLayout and QGridLayout.

    For users of QLayout subclasses or of QMainWindow there is seldom
    any need to use the basic functions provided by QLayout, such as
    \l setResizeMode() or setMenuBar(). See the \link layout.html layout
    overview page \endlink for more information.

    To make your own layout manager, subclass QGLayoutIterator and
    implement the functions addItem(), sizeHint(), setGeometry(), and
    iterator(). You should also implement minimumSize() to ensure your
    layout isn't resized to zero size if there is too little space. To
    support children whose heights depend on their widths, implement
    hasHeightForWidth() and heightForWidth(). See the \link
    customlayout.html custom layout page \endlink for an in-depth
    description.

    Geometry management stops when the layout manager is deleted.
*/

/*!
    Constructs a new top-level QLayout called \a name, with main
    widget \a parent. \a parent may not be 0.

    The \a margin is the number of pixels between the edge of the
    widget and the managed children. The \a spacing sets the value of
    spacing(), which gives the spacing between the managed widgets. If
    \a spacing is -1 (the default), spacing is set to the value of \a
    margin.

    There can be only one top-level layout for a widget. It is
    returned by QWidget::layout()
*/
QLayout::QLayout( QWidget *parent, int margin, int spacing, const char *name )
    : QObject( parent, name )
{
    init();
    if ( parent ) {
	if ( parent->layout() ) {
	    qWarning( "QLayout \"%s\" added to %s \"%s\", which already has a"
		      " layout", QObject::name(), parent->className(),
		      parent->name() );
	    parent->removeChild( this );
	} else {
	    topLevel = TRUE;
	    parent->installEventFilter( this );
	    setWidgetLayout( parent, this );
	}
    }
    outsideBorder = margin;
    if ( spacing < 0 )
	insideSpacing = margin;
    else
	insideSpacing = spacing;
}

void QLayout::init()
{
    insideSpacing = 0;
    outsideBorder = 0;
    topLevel = FALSE;
    enabled = TRUE;
    autoNewChild = FALSE;
    frozen = FALSE;
    activated = FALSE;
    marginImpl = FALSE;
    autoMinimum = FALSE;
    autoResizeMode = TRUE;
    extraData = 0;
#ifndef QT_NO_MENUBAR
    menubar = 0;
#endif
}

/*!
    Constructs a new child QLayout called \a name, and places it
    inside \a parentLayout by using the default placement defined by
    addItem().

    If \a spacing is -1, this QLayout inherits \a parentLayout's
    spacing(), otherwise the value of \a spacing is used.
*/
QLayout::QLayout( QLayout *parentLayout, int spacing, const char *name )
    : QObject( parentLayout, name )

{
    init();
    insideSpacing = spacing < 0 ? parentLayout->insideSpacing : spacing;
    parentLayout->addItem( this );
}

/*!
    Constructs a new child QLayout called \a name. If \a spacing is
    -1, this QLayout inherits its parent's spacing(); otherwise the
    value of \a spacing is used.

    This layout has to be inserted into another layout before geometry
    management will work.
*/
QLayout::QLayout( int spacing, const char *name )
    : QObject( 0, name )
{
    init();
    insideSpacing = spacing;
}

/*!
    \fn void QLayout::addItem( QLayoutItem *item )

    Implemented in subclasses to add an \a item. How it is added is
    specific to each subclass.

    The ownership of \a item is transferred to the layout, and it's
    the layout's responsibility to delete it.
*/

/*!
    \fn QLayoutIterator QLayout::iterator()

    Implemented in subclasses to return an iterator that iterates over
    this layout's children.

    A typical implementation will be:
    \code
	QLayoutIterator MyLayout::iterator()
	{
	    QGLayoutIterator *i = new MyLayoutIterator( internal_data );
	    return QLayoutIterator( i );
	}
    \endcode
    where MyLayoutIterator is a subclass of QGLayoutIterator.
*/

/*!
    \fn void QLayout::add( QWidget *w )

    Adds widget \a w to this layout in a manner specific to the
    layout. This function uses addItem().
*/

/*!
    \fn QMenuBar* QLayout::menuBar () const

    Returns the menu bar set for this layout, or 0 if no menu bar is
    set.
*/

/*!
    \fn bool QLayout::isTopLevel () const

    Returns TRUE if this layout is a top-level layout, i.e. not a
    child of another layout; otherwise returns FALSE.
*/

/*!
    \property QLayout::margin
    \brief the width of the outside border of the layout

    For some layout classes this property has an effect only on
    top-level layouts; QBoxLayout and QGridLayout support margins for
    child layouts. The default value is 0.

    \sa spacing
*/

/*!
    \property QLayout::spacing
    \brief the spacing between widgets inside the layout

    The default value is -1, which signifies that the layout's spacing
    should not override the widget's spacing.

    \sa margin
*/
void QLayout::setMargin( int margin )
{
    outsideBorder = margin;
    invalidate();
    if ( mainWidget() ) {
	QEvent *lh = new QEvent( QEvent::LayoutHint );
	QApplication::postEvent( mainWidget(), lh );
    }
}

void QLayout::setSpacing( int spacing )
{
    insideSpacing = spacing;
    if ( spacing >= 0 )
	propagateSpacing( this );
    invalidate();
    if ( mainWidget() ) {
	QEvent *lh = new QEvent( QEvent::LayoutHint );
	QApplication::postEvent( mainWidget(), lh );
    }
}

/*!
    Returns the main widget (parent widget) of this layout, or 0 if
    this layout is a sub-layout that is not yet inserted.
*/
QWidget *QLayout::mainWidget()
{
    if ( !topLevel ) {
	if ( parent() ) {
	    QLayout *parentLayout = ::qt_cast<QLayout*>(parent());
	    Q_ASSERT(parentLayout);
	    return parentLayout->mainWidget();
	} else {
	    return 0;
	}
    } else {
	Q_ASSERT(parent() && parent()->isWidgetType());
	return (QWidget*)parent();
    }
}

/*!
    Returns TRUE if this layout is empty. The default implementation
    returns FALSE.
*/
bool QLayout::isEmpty() const
{
    return FALSE; //### should check
}

/*!
    Sets widget \a w's layout to layout \a l.
*/
void QLayout::setWidgetLayout( QWidget *w, QLayout *l )
{
    w->setLayout( l );
}

/*!
    This function is reimplemented in subclasses to perform layout.

    The default implementation maintains the geometry() information
    given by rect \a r. Reimplementors must call this function.
*/
void QLayout::setGeometry( const QRect &r )
{
    rect = r;
}

/*!
    Invalidates cached information. Reimplementations must call this.
*/
void QLayout::invalidate()
{
    rect = QRect();
}

static bool removeWidgetRecursively( QLayoutItem *lay, QWidget *w )
{
    bool didSomething = FALSE;
    QLayoutIterator it = lay->iterator();
    QLayoutItem *child;
    while ( (child = it.current()) != 0 ) {
	if ( child->widget() == w ) {
	    it.deleteCurrent();
	    lay->invalidate(); // maybe redundant
	    didSomething = TRUE;
	} else if ( removeWidgetRecursively(child, w) ) {
	    lay->invalidate(); // maybe redundant
	    didSomething = TRUE;
	} else {
	    ++it;
	}
    }
    return didSomething;
}

/*!
    \reimp
    Performs child widget layout when the parent widget is resized.
    Also handles removal of widgets and child layouts. \a e is the
    event the occurred on object \a o.
*/
bool QLayout::eventFilter( QObject *o, QEvent *e )
{
    if ( !enabled )
	return FALSE;

    if ( !o->isWidgetType() )
	return FALSE;

    switch ( e->type() ) {
    case QEvent::Resize:
	if ( activated ) {
	    QResizeEvent *r = (QResizeEvent *)e;
	    int mbh = 0;
#ifndef QT_NO_MENUBAR
	    mbh = menuBarHeightForWidth( menubar, r->size().width() );
#endif
	    int b = marginImpl ? 0 : outsideBorder;
	    setGeometry( QRect( b, mbh + b, r->size().width() - 2 * b,
				r->size().height() - mbh - 2 * b ) );
	} else {
	    activate();
	}
	break;
    case QEvent::ChildRemoved:
	{
	    QChildEvent *c = (QChildEvent *)e;
	    if ( c->child()->isWidgetType() ) {
		QWidget *w = (QWidget *)c->child();
#ifndef QT_NO_MENUBAR
		if ( w == menubar )
		    menubar = 0;
#endif
		if ( removeWidgetRecursively( this, w ) ) {
		    QEvent *lh = new QEvent( QEvent::LayoutHint );
		    QApplication::postEvent( o, lh );
		}
	    }
	}
	break;
    case QEvent::ChildInserted:
	if ( topLevel && autoNewChild ) {
	    QChildEvent *c = (QChildEvent *)e;
	    if ( c->child()->isWidgetType() ) {
		QWidget *w = (QWidget *)c->child();
		if ( !w->isTopLevel() ) {
#if !defined(QT_NO_MENUBAR) && !defined(QT_NO_TOOLBAR)
		    if ( ::qt_cast<QMenuBar*>(w) && !::qt_cast<QToolBar*>(w->parentWidget()) )
			menubar = (QMenuBar *)w;
		    else
#endif
			addItem( new QWidgetItem( w ) );
		    QEvent *lh = new QEvent( QEvent::LayoutHint );
		    QApplication::postEvent( o, lh );
		}
	    }
	}
	break;
    case QEvent::LayoutHint:
	activate();
	break;
    default:
	break;
    }
    return QObject::eventFilter( o, e );
}

/*!
    \reimp
*/
void QLayout::childEvent( QChildEvent *e )
{
    if ( !enabled )
	return;

    if ( e->type() == QEvent::ChildRemoved ) {
	QChildEvent *c = (QChildEvent*)e;
	QLayoutIterator it = iterator();
	QLayoutItem *item;
	while ( (item = it.current() ) ) {
	    if ( item == (QLayout*)c->child() ) {
		it.takeCurrent();
		invalidate();
		break;
	    } else {
		++it;
	    }
	}
    }
}

/*!
  \internal
  Also takes margin() and menu bar into account.
*/
int QLayout::totalHeightForWidth( int w ) const
{
    if ( topLevel ) {
	QWidget *mw = (QWidget*)parent();
	if ( mw && !mw->testWState(WState_Polished) ) {
	    mw->polish();
	}
    }
    int b = ( topLevel && !marginImpl ) ? 2 * outsideBorder : 0;
    int h = heightForWidth( w - b ) + b;
#ifndef QT_NO_MENUBAR
    h += menuBarHeightForWidth( menubar, w );
#endif
    return h;
}

/*!
  \internal
  Also takes margin() and menu bar into account.
*/
QSize QLayout::totalMinimumSize() const
{
    if ( topLevel ) {
	QWidget *mw = (QWidget*)parent();
	if ( mw && !mw->testWState(WState_Polished) )
	    mw->polish();
    }
    int b = ( topLevel && !marginImpl ) ? 2 * outsideBorder : 0;

    QSize s = minimumSize();
    int h = b;
#ifndef QT_NO_MENUBAR
    h += menuBarHeightForWidth( menubar, s.width() );
#endif
    return s + QSize( b, h );
}

/*!
  \internal
  Also takes margin() and menu bar into account.
*/
QSize QLayout::totalSizeHint() const
{
    if ( topLevel ) {
	QWidget *mw = (QWidget*)parent();
	if ( mw && !mw->testWState(WState_Polished) )
	    mw->polish();
    }
    int b = ( topLevel && !marginImpl ) ? 2 * outsideBorder : 0;

    QSize s = sizeHint();
    if ( hasHeightForWidth() )
	s.setHeight( heightForWidth(s.width()) );
    int h = b;
#ifndef QT_NO_MENUBAR
    h += menuBarHeightForWidth( menubar, s.width() );
#endif
    return s + QSize( b, h );
}

/*!
  \internal
  Also takes margin() and menu bar into account.
*/
QSize QLayout::totalMaximumSize() const
{
    if ( topLevel ) {
	QWidget *mw = (QWidget*)parent();
	if ( mw && !mw->testWState(WState_Polished) ) {
	    mw->polish();
	}
    }
    int b = ( topLevel && !marginImpl ) ? 2 * outsideBorder : 0;

    QSize s = maximumSize();
    int h = b;
#ifndef QT_NO_MENUBAR
    h += menuBarHeightForWidth( menubar, s.width() );
#endif

    if ( isTopLevel() )
	s = QSize( QMIN( s.width() + b, QLAYOUTSIZE_MAX ),
		   QMIN( s.height() + h, QLAYOUTSIZE_MAX ) );
    return s;
}

/*!
  \internal
  Destroys the layout, deleting all child layouts.
  Geometry management stops when a top-level layout is deleted.

  The layout classes will probably be fatally confused if you delete
  a sublayout.
*/
QLayout::~QLayout()
{
    /*
      This function may be called during the QObject destructor,
      when the parent no longer is a QWidget.
    */
    if ( isTopLevel() && parent() && parent()->isWidgetType() &&
	 ((QWidget*)parent())->layout() == this )
	setWidgetLayout( (QWidget*)parent(), 0 );
}

/*!
    Removes and deletes all items in this layout.
*/
void QLayout::deleteAllItems()
{
    QLayoutIterator it = iterator();
    QLayoutItem *l;
    while ( (l = it.takeCurrent()) )
	delete l;
}

/*!
    This function is called from addLayout() functions in subclasses
    to add layout \a l as a sub-layout.
*/
void QLayout::addChildLayout( QLayout *l )
{
    if ( l->parent() ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QLayout::addChildLayout: layout already has a parent" );
#endif
	return;
    }
    insertChild( l );
    if ( l->insideSpacing < 0 ) {
	l->insideSpacing = insideSpacing;
	propagateSpacing( l );
    }
}

/*! \fn int QLayout::defaultBorder() const

  \internal
*/

/*! \fn void QLayout::freeze()

  \internal
*/

/*!
  \internal
  Fixes the size of the main widget and distributes the available
  space to the child widgets. For widgets which should not be
  resizable, but where a QLayout subclass is used to set up the initial
  geometry.

  As a special case, freeze(0, 0) is equivalent to setResizeMode(Fixed).
*/
void QLayout::freeze( int w, int h )
{
    if ( w <= 0 || h <= 0 ) {
	setResizeMode( Fixed );
    } else {
	setResizeMode( FreeResize ); // layout will not change min/max size
	mainWidget()->setFixedSize( w, h );
    }
}

#ifndef QT_NO_MENUBAR

/*!
    Makes the geometry manager take account of the menu bar \a w. All
    child widgets are placed below the bottom edge of the menu bar.

    A menu bar does its own geometry management: never do addWidget()
    on a QMenuBar.
*/
void QLayout::setMenuBar( QMenuBar *w )
{
    menubar = w;
}

#endif

/*!
    Returns the minimum size of this layout. This is the smallest size
    that the layout can have while still respecting the
    specifications. Does not include what's needed by margin() or
    menuBar().

    The default implementation allows unlimited resizing.
*/
QSize QLayout::minimumSize() const
{
    return QSize( 0, 0 );
}

/*!
    Returns the maximum size of this layout. This is the largest size
    that the layout can have while still respecting the
    specifications. Does not include what's needed by margin() or
    menuBar().

    The default implementation allows unlimited resizing.
*/
QSize QLayout::maximumSize() const
{
    return QSize( QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX );
}

/*!
    Returns whether this layout can make use of more space than
    sizeHint(). A value of \c Vertical or \c Horizontal means that it wants
    to grow in only one dimension, whereas \c BothDirections means that
    it wants to grow in both dimensions.

    The default implementation returns \c BothDirections.
*/
QSizePolicy::ExpandData QLayout::expanding() const
{
    return QSizePolicy::BothDirections;
}

static void invalidateRecursive( QLayoutItem *lay )
{
    lay->invalidate();
    QLayoutIterator it = lay->iterator();
    QLayoutItem *child;
    while ( (child = it.current()) != 0 ) {
	invalidateRecursive( child );
	++it;
    }
}

/*!
    Redoes the layout for mainWidget(). You should generally not need
    to call this because it is automatically called at the most
    appropriate times.

    However, if you set up a QLayout for a visible widget without
    resizing that widget, you will need to call this function in order
    to lay it out.

    \sa QWidget::updateGeometry()
*/
bool QLayout::activate()
{
    invalidateRecursive( this );
    if ( !topLevel )
	return FALSE;

    QWidget *mw = mainWidget();
    if (!mw) {
#if defined( QT_CHECK_NULL )
	qWarning( "QLayout::activate: %s \"%s\" does not have a main widget",
		  QObject::className(), QObject::name() );
#endif
	return FALSE;
    }
    activated = TRUE;
    QSize s = mw->size();
    QSize ms;
    int mbh = 0;
#ifndef QT_NO_MENUBAR
    mbh = menuBarHeightForWidth( menubar, s.width() );
#endif
    int b = marginImpl ? 0 : outsideBorder;
    setGeometry(QRect(b, mbh + b, s.width() - 2 * b, s.height() - mbh - 2 * b));
    if ( frozen ) {
	// will trigger resize
	mw->setFixedSize( totalSizeHint() );
    } else if ( autoMinimum ) {
	ms = totalMinimumSize();
    } else if ( autoResizeMode && topLevel && mw->isTopLevel() ) {
	ms = totalMinimumSize();
	if ( hasHeightForWidth() ) {
	    int h;
	    int mmbh = menuBarHeightForWidth( menubar, ms.width() );
	    // ### 4.0: remove this 'if' when minimumHeightForWidth() is virtual
	    if ( inherits("QBoxLayout") ) {
		h = ((QBoxLayout *) this)->minimumHeightForWidth( ms.width() );
	    } else if ( inherits("QGridLayout") ) {
		h = ((QGridLayout *) this)->minimumHeightForWidth( ms.width() );
	    } else {
		h = heightForWidth( ms.width() );
	    }
	    if ( h + mmbh > ms.height() )
#if 1
                //old behaviour:
 		ms = QSize( 0, 0 );
#else
            //better, but too big a change for a patch release in a stable branch:
		ms.setHeight( 0 );
#endif
	}
    }

    if (ms.isValid())
	mw->setMinimumSize( ms );

    // ideally only if sizeHint() or sizePolicy() has changed
    mw->updateGeometry();
    return TRUE;
}

/*!
    \class QSizePolicy
    \brief The QSizePolicy class is a layout attribute describing horizontal
    and vertical resizing policy.

    \ingroup appearance
    \ingroup geomanagement

    The size policy of a widget is an expression of its willingness to
    be resized in various ways.

    Widgets that reimplement QWidget::sizePolicy() return a QSizePolicy
    that describes the horizontal and vertical resizing policy they
    prefer when being laid out. Only \link #interesting one of the
    constructors\endlink is of interest in most applications.

    QSizePolicy contains two independent SizeType objects; one describes
    the widgets's horizontal size policy, and the other describes its
    vertical size policy. It also contains a flag to indicate whether the
    height and width of its preferred size are related.

    The horizontal and vertical \l{SizeType}s are set in the usual constructor
    and can be queried using a variety of functions.

    The hasHeightForWidth() flag indicates whether the widget's sizeHint()
    is width-dependent (such as a word-wrapping label) or not.

    \sa QSizePolicy::SizeType
*/

/*!
    \enum QSizePolicy::SizeType

    The per-dimension sizing types used when constructing a
    QSizePolicy are:

    \value Fixed  The QWidget::sizeHint() is the only acceptable
    alternative, so the widget can never grow or shrink (e.g. the
    vertical direction of a push button).

    \value Minimum  The sizeHint() is minimal, and sufficient. The
    widget can be expanded, but there is no advantage to it being
    larger (e.g. the horizontal direction of a push button).
    It cannot be smaller than the size provided by sizeHint().

    \value Maximum  The sizeHint() is a maximum. The widget can be
    shrunk any amount without detriment if other widgets need the
    space (e.g. a separator line).
    It cannot be larger than the size provided by sizeHint().

    \value Preferred  The sizeHint() is best, but the widget can be
    shrunk and still be useful. The widget can be expanded, but there
    is no advantage to it being larger than sizeHint() (the default
    QWidget policy).

    \value Expanding  The sizeHint() is a sensible size, but the
    widget can be shrunk and still be useful. The widget can make use
    of extra space, so it should get as much space as possible (e.g.
    the horizontal direction of a slider).

    \value MinimumExpanding  The sizeHint() is minimal, and sufficient.
    The widget can make use of extra space, so it should get as much
    space as possible (e.g. the horizontal direction of a slider).

    \value Ignored the sizeHint() is ignored. The widget will get as
    much space as possible.
*/

/*!
    \enum QSizePolicy::ExpandData

    This enum type describes in which directions a widget can make use
    of extra space. There are four possible values:

    \value NoDirection  the widget cannot make use of extra space in
    any direction.

    \value Horizontally  the widget can usefully be wider than the
    sizeHint().

    \value Vertically  the widget can usefully be taller than the
    sizeHint().

    \value BothDirections  the widget can usefully be both wider and
    taller than the sizeHint().
*/

/*!
    \fn QSizePolicy::QSizePolicy()

    Constructs a minimally initialized QSizePolicy.
*/

/*!
    \fn QSizePolicy::QSizePolicy( SizeType hor, SizeType ver, bool hfw )

    \target interesting
    This is the constructor normally used to return a value in the
    overridden \l QWidget::sizePolicy() function of a QWidget
    subclass.

    It constructs a QSizePolicy with independent horizontal and
    vertical sizing types, \a hor and \a ver respectively. These \link
    QSizePolicy::SizeType sizing types\endlink affect how the widget
    is treated by the \link QLayout layout engine\endlink.

    If \a hfw is TRUE, the preferred height of the widget is dependent
    on the width of the widget (for example, a QLabel with line
    wrapping).

    \sa horData() verData() hasHeightForWidth()
*/

/*!
    \fn QSizePolicy::QSizePolicy( SizeType hor, SizeType ver, uchar horStretch, uchar verStretch, bool hfw )

    Constructs a QSizePolicy with independent horizontal and vertical
    sizing types \a hor and \a ver, and stretch factors \a horStretch
    and \a verStretch.

    If \a hfw is TRUE, the preferred height of the widget is dependent on the
    width of the widget.

    \sa horStretch() verStretch()
*/

/*!
    \fn QSizePolicy::SizeType QSizePolicy::horData() const

    Returns the horizontal component of the size policy.

    \sa setHorData() verData() horStretch()
*/

/*!
    \fn QSizePolicy::SizeType QSizePolicy::verData() const

    Returns the vertical component of the size policy.

    \sa setVerData() horData() verStretch()
*/

/*!
    \fn bool QSizePolicy::mayShrinkHorizontally() const

    Returns TRUE if the widget can sensibly be narrower than its
    sizeHint(); otherwise returns FALSE.

    \sa mayShrinkVertically() mayGrowHorizontally()
*/

/*!
    \fn bool QSizePolicy::mayShrinkVertically() const

    Returns TRUE if the widget can sensibly be shorter than its
    sizeHint(); otherwise returns FALSE.

    \sa mayShrinkHorizontally() mayGrowVertically()
*/

/*!
    \fn bool QSizePolicy::mayGrowHorizontally() const

    Returns TRUE if the widget can sensibly be wider than its
    sizeHint(); otherwise returns FALSE.

    \sa mayGrowVertically() mayShrinkHorizontally()
*/

/*!
    \fn bool QSizePolicy::mayGrowVertically() const

    Returns TRUE if the widget can sensibly be taller than its
    sizeHint(); otherwise returns FALSE.

    \sa mayGrowHorizontally() mayShrinkVertically()
*/

/*!
    \fn QSizePolicy::ExpandData QSizePolicy::expanding() const

    Returns whether this layout can make use of more space than
    sizeHint(). A value of \c Vertical or \c Horizontal means that it wants
    to grow in only one dimension, whereas \c BothDirections means that
    it wants to grow in both dimensions.

    \sa mayShrinkHorizontally() mayGrowHorizontally()
	mayShrinkVertically() mayGrowVertically()
*/

/*!
    \fn void QSizePolicy::setHorData( SizeType d )

    Sets the horizontal component of the size policy to size type \a
    d.

    \sa horData() setVerData()
*/

/*!
    \fn void QSizePolicy::setVerData( SizeType d )

    Sets the vertical component of the size policy to size type \a d.

    \sa verData() setHorData()
*/

/*!
    \fn bool QSizePolicy::hasHeightForWidth() const

    Returns TRUE if the widget's preferred height depends on its
    width; otherwise returns FALSE.

    \sa setHeightForWidth()
*/

/*!
    \fn void QSizePolicy::setHeightForWidth( bool b )

    Sets the hasHeightForWidth() flag to \a b.

    \sa hasHeightForWidth()
*/

/*!
    \fn uint QSizePolicy::horStretch() const

    Returns the horizontal stretch factor of the size policy.

    \sa setHorStretch() verStretch()
*/

/*!
    \fn uint QSizePolicy::verStretch() const

    Returns the vertical stretch factor of the size policy.

    \sa setVerStretch() horStretch()
*/

/*!
    \fn void QSizePolicy::setHorStretch( uchar sf )

    Sets the horizontal stretch factor of the size policy to \a sf.

    \sa horStretch() setVerStretch()
*/

/*!
    \fn void QSizePolicy::setVerStretch( uchar sf )

    Sets the vertical stretch factor of the size policy to \a sf.

    \sa verStretch() setHorStretch()
*/

/*!
    \fn void QSizePolicy::transpose()

    Swaps the horizontal and vertical policies and stretches.
*/


/*!
    \fn bool QSizePolicy::operator==( const QSizePolicy &s ) const

    Returns TRUE if this policy is equal to \a s; otherwise returns
    FALSE.

    \sa operator!=()
*/

/*!
    \fn bool QSizePolicy::operator!=( const QSizePolicy &s ) const

    Returns TRUE if this policy is different from \a s; otherwise
    returns FALSE.

    \sa operator==()
*/

/*!
    \class QGLayoutIterator
    \brief The QGLayoutIterator class is an abstract base class of internal layout iterators.

    \ingroup appearance
    \ingroup geomanagement

    (This class is \e not OpenGL related, it just happens to start with
    the letters QGL...)

    Subclass this class to create a custom layout. The functions that
    must be implemented are next(), current(), and takeCurrent().

    The QGLayoutIterator implements the functionality of
    QLayoutIterator. Each subclass of QLayout needs a
    QGLayoutIterator subclass.
*/

/*!
    \fn QLayoutItem *QGLayoutIterator::next()

    Implemented in subclasses to move the iterator to the next item
    and return that item, or 0 if there is no next item.
*/

/*!
    \fn QLayoutItem *QGLayoutIterator::current()

    Implemented in subclasses to return the current item, or 0 if
    there is no current item.
*/

/*!
    \fn QLayoutItem *QGLayoutIterator::takeCurrent()

    Implemented in subclasses. The function must remove the current
    item from the layout without deleting it, move the iterator to the
    next item and return the removed item, or 0 if no item was
    removed.
*/

/*!
    Destroys the iterator
*/
QGLayoutIterator::~QGLayoutIterator()
{
}

/*!
    \class QLayoutIterator
    \brief The QLayoutIterator class provides iterators over QLayoutItem.

    \ingroup appearance
    \ingroup geomanagement

    Use QLayoutItem::iterator() to create an iterator over a layout.

    QLayoutIterator uses \e explicit sharing with a reference count.
    If an iterator is copied and one of the copies is modified, both
    iterators will be modified.

    A QLayoutIterator is not protected against changes in its layout. If
    the layout is modified or deleted the iterator will become invalid.
    It is not possible to test for validity. It is safe to delete an
    invalid layout; any other access may lead to an illegal memory
    reference and the abnormal termination of the program.

    Calling takeCurrent() or deleteCurrent() leaves the iterator in a
    valid state, but may invalidate any other iterators that access the
    same layout.

    The following code will draw a rectangle for each layout item in
    the layout structure of the widget.
    \code
    static void paintLayout( QPainter *p, QLayoutItem *lay )
    {
	QLayoutIterator it = lay->iterator();
	QLayoutItem *child;
	while ( (child = it.current()) != 0 ) {
	    paintLayout( p, child );
	    ++it;
	}
	p->drawRect( lay->geometry() );
    }
    void ExampleWidget::paintEvent( QPaintEvent * )
    {
	QPainter p( this );
	if ( layout() )
	    paintLayout( &p, layout() );
    }
    \endcode

    All the functionality of QLayoutIterator is implemented by
    subclasses of \l QGLayoutIterator. QLayoutIterator itself is not
    designed to be subclassed.
*/

/*!
    \fn QLayoutIterator::QLayoutIterator( QGLayoutIterator *gi )

    Constructs an iterator based on \a gi. The constructed iterator
    takes ownership of \a gi and will delete it.

    This constructor is provided for layout implementors. Application
    programmers should use QLayoutItem::iterator() to create an
    iterator over a layout.
*/

/*!
    \fn QLayoutIterator::QLayoutIterator( const QLayoutIterator &i )

    Creates a shallow copy of \a i, i.e. if the copy is modified, then
    the original will also be modified.
*/

/*!
    \fn QLayoutIterator::~QLayoutIterator()

    Destroys the iterator.
*/

/*!
    \fn QLayoutIterator &QLayoutIterator::operator=( const QLayoutIterator &i )

    Assigns \a i to this iterator and returns a reference to this
    iterator.
*/

/*!
    \fn QLayoutItem *QLayoutIterator::operator++()

    Moves the iterator to the next child item and returns that item,
    or 0 if there is no such item.
*/

/*!
    \fn QLayoutItem *QLayoutIterator::current()

    Returns the current item, or 0 if there is no current item.
*/

/*!
    \fn QLayoutItem *QLayoutIterator::takeCurrent()

    Removes the current child item from the layout without deleting
    it, and moves the iterator to the next item. Returns the removed
    item, or 0 if there was no item to be removed. This iterator will
    still be valid, but any other iterator over the same layout may
    become invalid.
*/

/*!
    \fn void QLayoutIterator::deleteCurrent()

    Removes and deletes the current child item from the layout and
    moves the iterator to the next item. This iterator will still be
    valid, but any other iterator over the same layout may become
    invalid.
*/

/*!
    \enum QLayout::ResizeMode

    The possible values are:

    \value Auto  If the main widget is a top-level widget with no
		 height-for-width (hasHeightForWidth()), this is
		 the same as \c Minimium; otherwise, this is the
		 same as \c FreeResize.
    \value Fixed  The main widget's size is set to sizeHint(); it
		  cannot be resized at all.
    \value Minimum  The main widget's minimum size is set to
		    minimumSize(); it cannot be smaller.
    \value FreeResize  The widget is not constrained.
*/

/*!
    \property QLayout::resizeMode
    \brief the resize mode of the layout

    The default mode is \c Auto.

    \sa QLayout::ResizeMode
*/

void QLayout::setResizeMode( ResizeMode mode )
{
    if ( mode == resizeMode() )
	return;

    switch ( mode ) {
    case Auto:
	frozen = FALSE;
	autoMinimum = FALSE;
	autoResizeMode = TRUE;
	break;
    case Fixed:
	frozen = TRUE;
	autoMinimum = FALSE;
	autoResizeMode = FALSE;
	break;
    case FreeResize:
	frozen = FALSE;
	autoMinimum = FALSE;
	autoResizeMode = FALSE;
	break;
    case Minimum:
	frozen = FALSE;
	autoMinimum = TRUE;
	autoResizeMode = FALSE;
    }
    if ( mainWidget() && mainWidget()->isVisible() )
	activate();
}

QLayout::ResizeMode QLayout::resizeMode() const
{
    return ( autoResizeMode ? Auto :
	     (frozen ? Fixed : (autoMinimum ? Minimum : FreeResize)) );
}

/*!
    \fn bool QLayout::autoAdd() const

    Returns TRUE if this layout automatically grabs all new
    mainWidget()'s new children and adds them as defined by addItem();
    otherwise returns FALSE. This has effect only for top-level
    layouts, i.e. layouts that are direct children of their
    mainWidget().

    autoAdd() is disabled by default.

    Note that a top-level layout is not necessarily associated with
    the top-level widget.

    \sa setAutoAdd()
*/

/*!
    If \a b is TRUE, auto-add is enabled; otherwise auto-add is
    disabled.

    \warning If auto-add is enabled, you cannot set stretch factors
    on the child widgets until the widgets are actually inserted in
    the layout (after control returned to the event loop). We
    therefore recommend that you avoid the auto-add feature in new
    programs.

    \sa autoAdd()
*/
void QLayout::setAutoAdd( bool b )
{
    autoNewChild = b;
}

/*!
    \fn  bool QLayout::supportsMargin() const

    Returns TRUE if this layout supports \l QLayout::margin on
    non-top-level layouts; otherwise returns FALSE.

    \sa margin
*/

/*!
    Sets the value returned by supportsMargin(). If \a b is TRUE,
    margin() handling is implemented by the subclass. If \a b is
    FALSE (the default), QLayout will add margin() around top-level
    layouts.

    If \a b is TRUE, margin handling needs to be implemented in
    setGeometry(), maximumSize(), minimumSize(), sizeHint() and
    heightForWidth().

    \sa supportsMargin()
*/
void QLayout::setSupportsMargin( bool b )
{
    marginImpl = b;
}

/*!
    Returns the rectangle that should be covered when the geometry of
    this layout is set to \a r, provided that this layout supports
    setAlignment().

    The result is derived from sizeHint() and expanding(). It is never
    larger than \a r.
*/
QRect QLayout::alignmentRect( const QRect &r ) const
{
    QSize s = sizeHint();
    int a = alignment();

    /*
      This is a hack to obtain the real maximum size, not
      QSize(QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX), the value consistently
      returned by QLayoutItems that have an alignment.
    */
    QLayout *that = (QLayout *) this;
    that->setAlignment( 0 );
    QSize ms = maximumSize();
    that->setAlignment( a );

    if ( (expanding() & QSizePolicy::Horizontally) ||
	 !(a & Qt::AlignHorizontal_Mask ) ) {
	s.setWidth( QMIN(r.width(), ms.width()) );
    }
    if ( (expanding() & QSizePolicy::Vertically) ||
	 !(a & Qt::AlignVertical_Mask) ) {
	s.setHeight( QMIN(r.height(), ms.height()) );
    } else if ( hasHeightForWidth() ) {
	int hfw = heightForWidth( s.width() );
	if ( hfw < s.height() )
	    s.setHeight( QMIN(hfw, ms.height()) );
    }

    int x = r.x();
    int y = r.y();

    if ( a & Qt::AlignBottom )
	y += ( r.height() - s.height() );
    else if ( !(a & Qt::AlignTop) )
	y += ( r.height() - s.height() ) / 2;

    a = QApplication::horizontalAlignment( a );
    if ( a & Qt::AlignRight )
	x += ( r.width() - s.width() );
    else if ( !(a & Qt::AlignLeft) )
	x += ( r.width() - s.width() ) / 2;

    return QRect( x, y, s.width(), s.height() );
}

/*!
    Removes the widget \a widget from the layout. After this call, it
    is the caller's responsibility to give the widget a reasonable
    geometry or to put the widget back into a layout.

    \sa removeItem(), QWidget::setGeometry(), add()
*/
void QLayout::remove( QWidget *widget )
{
    QLayoutIterator it = iterator();
    QLayoutItem *child;
    while ( (child = it.current()) != 0 ) {
	if ( child->widget() == widget ) {
	    it.deleteCurrent();
	    invalidate(); // maybe redundant
	    QApplication::postEvent( mainWidget(),
				     new QEvent(QEvent::LayoutHint) );
	} else {
	    ++it;
	}
    }
}

/*!
    Removes the layout item \a item from the layout. It is the
    caller's responsibility to delete the item.

    Notice that \a item can be a layout (since QLayout inherits
    QLayoutItem).

    \sa remove(), addItem()
*/
void QLayout::removeItem( QLayoutItem *item )
{
    QLayoutIterator it = iterator();
    QLayoutItem *child;
    while ( (child = it.current()) != 0 ) {
	if ( child == item ) {
	    it.takeCurrent();
	    invalidate(); // maybe redundant
	    QApplication::postEvent( mainWidget(),
				     new QEvent(QEvent::LayoutHint) );
	} else {
	    ++it;
	}
    }
}

/*!
    Enables this layout if \a enable is TRUE, otherwise disables it.

    An enabled layout adjusts dynamically to changes; a disabled
    layout acts as if it did not exist.

    By default all layouts are enabled.

    \sa isEnabled()
*/
void QLayout::setEnabled( bool enable )
{
    enabled = enable;
}

/*!
    Returns TRUE if the layout is enabled; otherwise returns FALSE.

    \sa setEnabled()
*/
bool QLayout::isEnabled() const
{
    return enabled;
}

void QLayout::propagateSpacing( QLayout *parent )
{
    QLayoutIterator it = parent->iterator();
    QLayoutItem *child;
    while ( (child = it.current()) ) {
	QLayout *childLayout = child->layout();
	if ( childLayout && childLayout->insideSpacing < 0 ) {
	    childLayout->insideSpacing = parent->insideSpacing;
	    propagateSpacing( childLayout );
	}
	++it;
    }
}

#endif // QT_NO_LAYOUT
