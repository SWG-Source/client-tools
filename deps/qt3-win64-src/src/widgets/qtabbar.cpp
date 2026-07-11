/****************************************************************************
** $Id: qtabbar.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QTab and QTabBar classes
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

#include "qtabbar.h"
#ifndef QT_NO_TABBAR
#include "qaccel.h"
#include "qbitmap.h"
#include "qtoolbutton.h"
#include "qtooltip.h"
#include "qapplication.h"
#include "qstyle.h"
#include "qpainter.h"
#include "qiconset.h"
#include "qcursor.h"
#include "../kernel/qinternal_p.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif

#ifdef Q_WS_MACX
#include <qmacstyle_mac.h>
#endif


/*!
    \class QTab qtabbar.h
    \brief The QTab class provides the structures in a QTabBar.

    \ingroup advanced

    This class is used for custom QTabBar tab headings.

    \sa QTabBar
*/


/*!
    Constructs an empty tab. All fields are set to empty.
*/

QTab::QTab()
    : enabled( TRUE ),
      id ( 0 ),
      iconset( 0 ),
      tb( 0 )
{
}

/*!
    Constructs a tab with the text \a text.
*/

QTab::QTab( const QString &text )
    : label( text ),
      enabled( TRUE ),
      id( 0 ),
      iconset( 0 ),
      tb( 0 )
{
}

/*!
    Constructs a tab with an \a icon and the text, \a text.
*/

QTab::QTab( const QIconSet& icon, const QString& text )
    : label( text ),
      enabled( TRUE ),
      id( 0 ),
      iconset( new QIconSet(icon) ),
      tb( 0 )
{
}

/*!
    \fn QString QTab::text() const

    Returns the text of the QTab label.
*/

/*!
    \fn QIconSet QTab::iconSet() const

    Return the QIconSet of the QTab.
*/

/*!
    \fn void QTab::setRect( const QRect &rect )

    Set the QTab QRect to \a rect.
*/

/*!
    \fn QRect QTab::rect() const

    Return the QRect for the QTab.
*/

/*!
    \fn void QTab::setEnabled( bool enable )

    If \a enable is TRUE enable the QTab, otherwise disable it.
*/

/*!
    \fn bool QTab::isEnabled() const

    Returns TRUE if the QTab is enabled; otherwise returns FALSE.
*/

/*!
    \fn void QTab::setIdentifier( int i )

    Set the identifier for the QTab to \a i. Each QTab's identifier
    within a QTabBar must be unique.
*/

/*!
    \fn int QTab::identifier() const

    Return the QTab's identifier.
*/



/*!
    Destroys the tab and frees up all allocated resources.
*/

QTab::~QTab()
{
    delete iconset;
    tb = 0;
}

/*!
    \class QTabBar qtabbar.h
    \brief The QTabBar class provides a tab bar, e.g. for use in tabbed dialogs.

    \ingroup advanced

    QTabBar is straightforward to use; it draws the tabs using one of
    the predefined \link QTabBar::Shape shapes\endlink, and emits a
    signal when a tab is selected. It can be subclassed to tailor the
    look and feel. Qt also provides a ready-made \l{QTabWidget} and a
    \l{QTabDialog}.

    The choice of tab shape is a matter of taste, although tab dialogs
    (for preferences and similar) invariably use \c RoundedAbove;
    nobody uses \c TriangularAbove. Tab controls in windows other than
    dialogs almost always use either \c RoundedBelow or \c
    TriangularBelow. Many spreadsheets and other tab controls in which
    all the pages are essentially similar use \c TriangularBelow,
    whereas \c RoundedBelow is used mostly when the pages are
    different (e.g. a multi-page tool palette).

    The most important part of QTabBar's API is the selected() signal.
    This is emitted whenever the selected page changes (even at
    startup, when the selected page changes from 'none'). There is
    also a slot, setCurrentTab(), which can be used to select a page
    programmatically.

    QTabBar creates automatic accelerator keys in the manner of
    QButton; e.g. if a tab's label is "\&Graphics", Alt+G becomes an
    accelerator key for switching to that tab.

    The following virtual functions may need to be reimplemented:
    \list
    \i paint() paints a single tab. paintEvent() calls paint() for
    each tab so that any overlap will look right.
    \i addTab() creates a new tab and adds it to the bar.
    \i selectTab() decides which tab, if any, the user selects with the mouse.
    \endlist

    The index of the current tab is returned by currentTab(). The tab
    with a particular index is returned by tabAt(), the tab with a
    particular id is returned by tab(). The index of a tab is returned
    by indexOf(). The current tab can be set by index or tab pointer
    using one of the setCurrentTab() functions.

    <img src=qtabbar-m.png> <img src=qtabbar-w.png>
*/

/*!
    \enum QTabBar::Shape

    This enum type lists the built-in shapes supported by QTabBar:

    \value RoundedAbove  the normal rounded look above the pages

    \value RoundedBelow  the normal rounded look below the pages

    \value TriangularAbove  triangular tabs above the pages (very
    unusual; included for completeness)

    \value TriangularBelow  triangular tabs similar to those used in
    the Excel spreadsheet, for example
*/

class QTabBarToolTip;

struct QTabPrivate {
    int id;
    int focus;
#ifndef QT_NO_ACCEL
    QAccel * a;
#endif
    QTab *pressed;
    QTabBar::Shape s;
    QToolButton* rightB;
    QToolButton* leftB;
    int btnWidth;
    bool  scrolls;
    QTabBarToolTip * toolTips;
};

#ifndef QT_NO_TOOLTIP
/* \internal
*/
class QTabBarToolTip : public QToolTip
{
public:
    QTabBarToolTip( QWidget * parent )
	: QToolTip( parent ) {}
    virtual ~QTabBarToolTip() {}

    void add( QTab * tab, const QString & tip )
    {
	tabTips.replace( tab, tip );
    }

    void remove( QTab * tab )
    {
	tabTips.erase( tab );
    }

    QString tipForTab( QTab * tab ) const
    {
	QMapConstIterator<QTab *, QString> it;
	it = tabTips.find( tab );
	if ( it != tabTips.end() )
	    return it.data();
	else
	    return QString();
    }

protected:
    void maybeTip( const QPoint & p )
    {
	QTabBar * tb = (QTabBar *) parentWidget();
	if ( !tb )
	    return;

	// check if the scroll buttons in the tab bar are visible -
	// don't display any tips if the pointer is over one of them
	QRect rectL, rectR;
	rectL.setRect( tb->d->leftB->x(), tb->d->leftB->y(),
		       tb->d->leftB->width(), tb->d->leftB->height() );
	rectR.setRect( tb->d->rightB->x(), tb->d->rightB->y(),
		       tb->d->rightB->width(), tb->d->rightB->height() );
	if ( tb->d->scrolls && (rectL.contains( p ) || rectR.contains( p )) )
	     return;

#ifndef QT_NO_TOOLTIP
	// find and show the tool tip for the tab under the point p
	QMapIterator<QTab *, QString> it;
	for ( it = tabTips.begin(); it != tabTips.end(); ++it ) {
	    if ( it.key()->rect().contains( p ) )
		tip( it.key()->rect(), it.data() );
	}
#endif
    }

private:
    QMap<QTab *, QString> tabTips;
};
#endif

/*!
    \fn void QTabBar::selected( int id )

    QTabBar emits this signal whenever any tab is selected, whether by
    the program or by the user. The argument \a id is the id of the
    tab as returned by addTab().

    show() is guaranteed to emit this signal; you can display your
    page in a slot connected to this signal.
*/

/*!
    \fn void QTabBar::layoutChanged()

    QTabBar emits the signal whenever the layout of the tab bar has
    been recalculated, for example when the contents of a tab change.
*/

/*!
    Constructs a new, empty tab bar; the \a parent and \a name
    arguments are passed on to the QWidget constructor.
*/

QTabBar::QTabBar( QWidget * parent, const char *name )
    : QWidget( parent, name, WNoAutoErase | WNoMousePropagation  )
{
    d = new QTabPrivate;
    d->pressed = 0;
    d->id = 0;
    d->focus = 0;
    d->toolTips = 0;
#ifndef QT_NO_ACCEL
    d->a = new QAccel( this, "tab accelerators" );
    connect( d->a, SIGNAL(activated(int)), this, SLOT(setCurrentTab(int)) );
    connect( d->a, SIGNAL(activatedAmbiguously(int)), this, SLOT(setCurrentTab(int)) );
#endif
    d->s = RoundedAbove;
    d->scrolls = FALSE;
    d->leftB = new QToolButton( LeftArrow, this, "qt_left_btn" );
    connect( d->leftB, SIGNAL( clicked() ), this, SLOT( scrollTabs() ) );
    d->leftB->hide();
    d->rightB = new QToolButton( RightArrow, this, "qt_right_btn" );
    connect( d->rightB, SIGNAL( clicked() ), this, SLOT( scrollTabs() ) );
    d->rightB->hide();
    d->btnWidth = style().pixelMetric(QStyle::PM_TabBarScrollButtonWidth, this);
    l = new QPtrList<QTab>;
    lstatic = new QPtrList<QTab>;
    lstatic->setAutoDelete( TRUE );
    setFocusPolicy( TabFocus );
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
}


/*!
    Destroys the tab control, freeing memory used.
*/

QTabBar::~QTabBar()
{
#ifndef QT_NO_TOOLTIP
    if ( d->toolTips )
	delete d->toolTips;
#endif
    delete d;
    d = 0;
    delete l;
    l = 0;
    delete lstatic;
    lstatic = 0;
}


/*!
    Adds the tab, \a newTab, to the tab control.

    Sets \a newTab's id to a new id and places the tab just to the
    right of the existing tabs. If the tab's label contains an
    ampersand, the letter following the ampersand is used as an
    accelerator for the tab, e.g. if the label is "Bro\&wse" then
    Alt+W becomes an accelerator which will move the focus to this
    tab. Returns the id.

    \sa insertTab()
*/

int QTabBar::addTab( QTab * newTab )
{
    return insertTab( newTab );
}


/*!
    Inserts the tab, \a newTab, into the tab control.

    If \a index is not specified, the tab is simply appended.
    Otherwise it's inserted at the specified position.

    Sets \a newTab's id to a new id. If the tab's label contains an
    ampersand, the letter following the ampersand is used as an
    accelerator for the tab, e.g. if the label is "Bro\&wse" then
    Alt+W becomes an accelerator which will move the focus to this
    tab. Returns the id.

    \sa addTab()
*/

int QTabBar::insertTab( QTab * newTab, int index )
{
    newTab->id = d->id++;
    if ( !tab( d->focus ) )
	d->focus = newTab->id;

    newTab->setTabBar( this );
    l->insert( 0, newTab );
    if ( index < 0 || index > int(lstatic->count()) )
	lstatic->append( newTab );
    else
	lstatic->insert( index, newTab );

    layoutTabs();
    updateArrowButtons();
    makeVisible( tab( currentTab() ) );

#ifndef QT_NO_ACCEL
    int p = QAccel::shortcutKey( newTab->label );
    if ( p )
	d->a->insertItem( p, newTab->id );
#endif

    return newTab->id;
}


/*!
    Removes tab \a t from the tab control, and deletes the tab.
*/
void QTabBar::removeTab( QTab * t )
{
    //#### accelerator labels??
#ifndef QT_NO_TOOLTIP
    if ( d->toolTips )
	d->toolTips->remove( t );
#endif
#ifndef QT_NO_ACCEL
    if ( d->a )
	d->a->removeItem( t->id );
#endif
    bool updateFocus = t->id == d->focus;
    // remove the TabBar Reference
    if(d->pressed == t)
	d->pressed = 0;
    t->setTabBar( 0 );
    l->remove( t );
    lstatic->remove( t );
    layoutTabs();
    updateArrowButtons();
    makeVisible( tab( currentTab() ) );
    if ( updateFocus )
	d->focus = currentTab();
    update();
}


/*!
    Enables tab \a id if \a enabled is TRUE or disables it if \a
    enabled is FALSE. If \a id is currently selected,
    setTabEnabled(FALSE) makes another tab selected.

    setTabEnabled() updates the display if this causes a change in \a
    id's status.

    \sa update(), isTabEnabled()
*/

void QTabBar::setTabEnabled( int id, bool enabled )
{
    QTab * t;
    for( t = l->first(); t; t = l->next() ) {
	if ( t && t->id == id ) {
	    if ( t->enabled != enabled ) {
		t->enabled = enabled;
#ifndef QT_NO_ACCEL
		d->a->setItemEnabled( t->id, enabled );
#endif
		QRect r( t->r );
		if ( !enabled && id == currentTab() ) {
		    QPoint p1( t->r.center() ), p2;
		    int m = 2147483647;
		    int distance;
		    // look for the closest enabled tab - measure the
		    // distance between the centers of the two tabs
		    for( QTab * n = l->first(); n; n = l->next() ) {
			if ( n->enabled ) {
			    p2 = n->r.center();
			    distance = (p2.x() - p1.x())*(p2.x() - p1.x()) +
				       (p2.y() - p1.y())*(p2.y() - p1.y());
			    if ( distance < m ) {
				t = n;
				m = distance;
			    }
			}
		    }
		    if ( t->enabled ) {
			r = r.unite( t->r );
			l->append( l->take( l->findRef( t ) ) );
			emit selected( t->id );
		    }
		}
		repaint( r, FALSE );
	    }
	    return;
	}
    }
}


/*!
    Returns TRUE if the tab with id \a id exists and is enabled;
    otherwise returns FALSE.

    \sa setTabEnabled()
*/

bool QTabBar::isTabEnabled( int id ) const
{
    QTab * t = tab( id );
    if ( t )
	return t->enabled;
    return FALSE;
}



/*!
    \reimp
*/
QSize QTabBar::sizeHint() const
{
    QSize sz(0, 0);
    if ( QTab * t = l->first() ) {
	QRect r( t->r );
	while ( (t = l->next()) != 0 )
	    r = r.unite( t->r );
	sz = r.size();
    }
    return sz.expandedTo(QApplication::globalStrut());
}

/*!
    \reimp
*/

QSize QTabBar::minimumSizeHint() const
{
    if(style().styleHint( QStyle::SH_TabBar_PreferNoArrows, this ))
	return sizeHint();
    return QSize( d->rightB->sizeHint().width() * 2 + 75, sizeHint().height() );
}

/*!
    Paints the tab \a t using painter \a p. If and only if \a selected
    is TRUE, \a t is drawn currently selected.

    This virtual function may be reimplemented to change the look of
    QTabBar. If you decide to reimplement it, you may also need to
    reimplement sizeHint().
*/

void QTabBar::paint( QPainter * p, QTab * t, bool selected ) const
{
    QStyle::SFlags flags = QStyle::Style_Default;

    if (isEnabled() && t->isEnabled())
	flags |= QStyle::Style_Enabled;
    if (topLevelWidget() == qApp->activeWindow())
	flags |= QStyle::Style_Active;
    if ( selected )
	flags |= QStyle::Style_Selected;
    else if(t == d->pressed)
	flags |= QStyle::Style_Sunken;
    //selection flags
    if(t->rect().contains(mapFromGlobal(QCursor::pos())))
	flags |= QStyle::Style_MouseOver;
    style().drawControl( QStyle::CE_TabBarTab, p, this, t->rect(),
			 colorGroup(), flags, QStyleOption(t) );

    QRect r( t->r );
    p->setFont( font() );

    int iw = 0;
    int ih = 0;
    if ( t->iconset != 0 ) {
	iw = t->iconset->pixmap( QIconSet::Small, QIconSet::Normal ).width() + 4;
	ih = t->iconset->pixmap( QIconSet::Small, QIconSet::Normal ).height();
    }
    QFontMetrics fm = p->fontMetrics();
    int fw = fm.width( t->label );
    fw -= t->label.contains('&') * fm.width('&');
    fw += t->label.contains("&&") * fm.width('&');
    int w = iw + fw + 4;
    int h = QMAX(fm.height() + 4, ih );
    int offset = 3;
#ifdef Q_WS_MAC
    if (::qt_cast<QMacStyle *>(&style()))
        offset = 0;
#endif
    paintLabel( p, QRect( r.left() + (r.width()-w)/2 - offset,
			  r.top() + (r.height()-h)/2,
			  w, h ), t, t->id == keyboardFocusTab() );
}

/*!
    Paints the label of tab \a t centered in rectangle \a br using
    painter \a p. A focus indication is drawn if \a has_focus is TRUE.
*/

void QTabBar::paintLabel( QPainter* p, const QRect& br,
			  QTab* t, bool has_focus ) const
{
    QRect r = br;
    bool selected = currentTab() == t->id;
    if ( t->iconset) {
	// the tab has an iconset, draw it in the right mode
	QIconSet::Mode mode = (t->enabled && isEnabled())
	    ? QIconSet::Normal : QIconSet::Disabled;
	if ( mode == QIconSet::Normal && has_focus )
	    mode = QIconSet::Active;
	QPixmap pixmap = t->iconset->pixmap( QIconSet::Small, mode );
	int pixw = pixmap.width();
	int pixh = pixmap.height();
	r.setLeft( r.left() + pixw + 4 );
	r.setRight( r.right() + 2 );

	int xoff = 0, yoff = 0;
	if(!selected) {
	    xoff = style().pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, this);
	    yoff = style().pixelMetric(QStyle::PM_TabBarTabShiftVertical, this);
	}
	p->drawPixmap( br.left() + 2 + xoff, br.center().y()-pixh/2 + yoff, pixmap );
    }

    QStyle::SFlags flags = QStyle::Style_Default;

    if (isEnabled() && t->isEnabled())
	flags |= QStyle::Style_Enabled;
    if (has_focus)
	flags |= QStyle::Style_HasFocus;
    if ( selected )
	flags |= QStyle::Style_Selected;
    else if(t == d->pressed)
	flags |= QStyle::Style_Sunken;
    if(t->rect().contains(mapFromGlobal(QCursor::pos())))
	flags |= QStyle::Style_MouseOver;
    style().drawControl( QStyle::CE_TabBarLabel, p, this, r,
			 t->isEnabled() ? colorGroup(): palette().disabled(),
			 flags, QStyleOption(t) );
}


/*!
    Repaints the tab row. All the painting is done by paint();
    paintEvent() only decides which tabs need painting and in what
    order. The event is passed in \a e.

    \sa paint()
*/

void QTabBar::paintEvent( QPaintEvent * e )
{
    if ( e->rect().isNull() )
	return;

    QSharedDoubleBuffer buffer( this, e->rect() );

    QTab * t;
    t = l->first();
    do {
	QTab * n = l->next();
	if ( t && t->r.intersects( e->rect() ) )
	    paint( buffer.painter(), t, n == 0 );
	t = n;
    } while ( t != 0 );

    if ( d->scrolls && lstatic->first()->r.left() < 0 ) {
	QPointArray a;
	int h = height();
	if ( d->s == RoundedAbove ) {
	    buffer.painter()->fillRect( 0, 3, 4, h-5,
			colorGroup().brush( QColorGroup::Background ) );
	    a.setPoints( 5,  0,2,  3,h/4, 0,h/2, 3,3*h/4, 0,h );
	} else if ( d->s == RoundedBelow ) {
	    buffer.painter()->fillRect( 0, 2, 4, h-5,
			colorGroup().brush( QColorGroup::Background ) );
	    a.setPoints( 5,  0,0,  3,h/4, 0,h/2, 3,3*h/4, 0,h-3 );
	}

	if ( !a.isEmpty() ) {
	    buffer.painter()->setPen( colorGroup().light() );
	    buffer.painter()->drawPolyline( a );
	    a.translate( 1, 0 );
	    buffer.painter()->setPen( colorGroup().midlight() );
	    buffer.painter()->drawPolyline( a );
	}
    }
}


/*!
    This virtual function is called by the mouse event handlers to
    determine which tab is pressed. The default implementation returns
    a pointer to the tab whose bounding rectangle contains \a p, if
    exactly one tab's bounding rectangle contains \a p. Otherwise it
    returns 0.

    \sa mousePressEvent() mouseReleaseEvent()
*/

QTab * QTabBar::selectTab( const QPoint & p ) const
{
    QTab * selected = 0;
    bool moreThanOne = FALSE;

    QPtrListIterator<QTab> i( *l );
    while( i.current() ) {
	QTab * t = i.current();
	++i;

	if ( t && t->r.contains( p ) ) {
	    if ( selected )
		moreThanOne = TRUE;
	    else
		selected = t;
	}
    }

    return moreThanOne ? 0 : selected;
}


/*!
    \reimp
*/
void QTabBar::mousePressEvent( QMouseEvent * e )
{
    if ( e->button() != LeftButton ) {
	e->ignore();
	return;
    }
    QTab *t = selectTab( e->pos() );
    if ( t && t->enabled ) {
	d->pressed = t;
	if(e->type() == style().styleHint( QStyle::SH_TabBar_SelectMouseType, this ))
	    setCurrentTab( t );
	else
	    repaint(t->rect(), FALSE);
    }
}


/*!
    \reimp
*/

void QTabBar::mouseMoveEvent ( QMouseEvent *e )
{
    if ( e->state() != LeftButton ) {
	e->ignore();
	return;
    }
    if(style().styleHint( QStyle::SH_TabBar_SelectMouseType, this ) == QEvent::MouseButtonRelease) {
	QTab *t = selectTab( e->pos() );
	if(t != d->pressed) {
	    if(d->pressed)
		repaint(d->pressed->rect(), FALSE);
	    if((d->pressed = t))
		repaint(t->rect(), FALSE);
	}
    }
}

/*!
    \reimp
*/

void QTabBar::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	e->ignore();
    if(d->pressed) {
	QTab *t = selectTab( e->pos() ) == d->pressed ? d->pressed : 0;
	d->pressed = 0;
	if(t && t->enabled && e->type() == style().styleHint( QStyle::SH_TabBar_SelectMouseType, this ))
	    setCurrentTab( t );
    }
}


/*!
    \reimp
*/
void QTabBar::show()
{
    //  ensures that one tab is selected.
    QTab * t = l->last();
    QWidget::show();

    if ( t )
	emit selected( t->id );
}

/*!
    \property QTabBar::currentTab
    \brief the id of the tab bar's visible tab

    If no tab page is currently visible, the property's value is -1.
    Even if the property's value is not -1, you cannot assume that the
    user can see the relevant page, or that the tab is enabled. When
    you need to display something the value of this property
    represents the best page to display.

    When this property is set to \e id, it will raise the tab with the
    id \e id and emit the selected() signal.

    \sa selected() isTabEnabled()
*/

int QTabBar::currentTab() const
{
    const QTab * t = l->getLast();

    return t ? t->id : -1;
}

void QTabBar::setCurrentTab( int id )
{
    setCurrentTab( tab( id ) );
}


/*!
    \overload

    Raises \a tab and emits the selected() signal unless the tab was
    already current.

    \sa currentTab() selected()
*/

void QTabBar::setCurrentTab( QTab * tab )
{
    if ( tab && l ) {
	if ( l->last() == tab )
	    return;

	QRect r = l->last()->r;
	if ( l->findRef( tab ) >= 0 )
	    l->append( l->take() );

	d->focus = tab->id;

	setMicroFocusHint( tab->rect().x(), tab->rect().y(), tab->rect().width(), tab->rect().height(), FALSE );

	if ( tab->r.intersects( r ) ) {
	    repaint( r.unite( tab->r ), FALSE );
	} else {
#ifdef Q_WS_MACX
            update();
#else
	    repaint( r, FALSE );
	    repaint( tab->r, FALSE );
#endif
	}
	makeVisible( tab );
	emit selected( tab->id );

#ifdef QT_ACCESSIBILITY_SUPPORT
	QAccessible::updateAccessibility( this, indexOf(tab->id)+1, QAccessible::Focus );
#endif
    }
}

/*!
    \property QTabBar::keyboardFocusTab
    \brief the id of the tab that has the keyboard focus

    This property contains the id of the tab that has the keyboard
    focus or -1 if the tab bar does not have the keyboard focus.
*/

int QTabBar::keyboardFocusTab() const
{
    return hasFocus() ? d->focus : -1;
}


/*!
    \reimp
*/
void QTabBar::keyPressEvent( QKeyEvent * e )
{
    //   The right and left arrow keys move a selector, the spacebar
    //   makes the tab with the selector active.  All other keys are
    //   ignored.

    int old = d->focus;

    bool reverse = QApplication::reverseLayout();
    if ( ( !reverse && e->key() == Key_Left ) || ( reverse && e->key() == Key_Right ) ) {
	// left - skip past any disabled ones
	if ( d->focus >= 0 ) {
	    QTab * t = lstatic->last();
	    while ( t && t->id != d->focus )
		t = lstatic->prev();
	    do {
		t = lstatic->prev();
	    } while ( t && !t->enabled);
	    if (t)
		d->focus = t->id;
	}
	if ( d->focus < 0 )
	    d->focus = old;
    } else if ( ( !reverse && e->key() == Key_Right ) || ( reverse && e->key() == Key_Left ) ) {
	QTab * t = lstatic->first();
	while ( t && t->id != d->focus )
	    t = lstatic->next();
	do {
	    t = lstatic->next();
	} while ( t && !t->enabled);

	if (t)
	    d->focus = t->id;
	if ( d->focus >= d->id )
	    d->focus = old;
    } else {
	// other keys - ignore
	e->ignore();
	return;
    }

    // if the focus moved, repaint and signal
    if ( old != d->focus ) {
	setCurrentTab( d->focus );
    }
}


/*!
    Returns the tab with id \a id or 0 if there is no such tab.

    \sa count()
*/

QTab * QTabBar::tab( int id ) const
{
    QTab * t;
    for( t = l->first(); t; t = l->next() )
	if ( t && t->id == id )
	    return t;
    return 0;
}


/*!
    Returns the tab at position \a index.

    \sa indexOf()
*/

QTab * QTabBar::tabAt( int index ) const
{
    QTab * t;
    t = lstatic->at( index );
    return t;
}


/*!
    Returns the position index of the tab with id \a id or -1 if no
    tab has this \a id.

    \sa tabAt()
*/
int QTabBar::indexOf( int id ) const
{
    QTab * t;
    int idx = 0;
    for( t = lstatic->first(); t; t = lstatic->next() ) {
	if ( t && t->id == id )
	    return idx;
	idx++;
    }
    return -1;
}


/*!
    \property QTabBar::count
    \brief the number of tabs in the tab bar

    \sa tab()
*/
int QTabBar::count() const
{
    return l->count();
}


/*!
    The list of QTab objects in the tab bar.

    This list is unlikely to be in the order that the QTab elements
    appear visually. One way of iterating over the tabs is like this:
    \code
    for ( uint i = 0; i < myTabBar->count(); ++i ) {
	nextTab = myTabBar->tabAt( i );
	// do something with nextTab
    }
    \endcode
*/
QPtrList<QTab> * QTabBar::tabList()
{
    return l;
}


/*!
    \property QTabBar::shape
    \brief the shape of the tabs in the tab bar

    The value of this property is one of the following: \c
    RoundedAbove (default), \c RoundedBelow, \c TriangularAbove or \c
    TriangularBelow.

    \sa Shape
*/
QTabBar::Shape QTabBar::shape() const
{
    return d ? d->s : RoundedAbove;
}

void QTabBar::setShape( Shape s )
{
    if ( !d || d->s == s )
	return;
    //######### must recalculate heights
    d->s = s;
    update();
}

/*!
    Lays out all existing tabs according to their label and their
    iconset.
 */
void QTabBar::layoutTabs()
{
    if ( lstatic->isEmpty() )
	return;

    QSize oldSh(0, 0);
    if ( QTab * t = l->first() ) {
	QRect r( t->r );
	while ( (t = l->next()) != 0 )
	    r = r.unite( t->r );
	oldSh = r.size();
    }

    d->btnWidth = style().pixelMetric(QStyle::PM_TabBarScrollButtonWidth, this);
    int hframe, vframe, overlap;
    hframe  = style().pixelMetric( QStyle::PM_TabBarTabHSpace, this );
    vframe  = style().pixelMetric( QStyle::PM_TabBarTabVSpace, this );
    overlap = style().pixelMetric( QStyle::PM_TabBarTabOverlap, this );

    QFontMetrics fm = fontMetrics();
    QRect r;
    QTab *t;
    bool reverse = QApplication::reverseLayout();
    if ( reverse )
	t = lstatic->last();
    else
	t = lstatic->first();
    int x = 0;
    int offset = (t && d->scrolls) ? t->r.x() : 0;
    while ( t ) {
	int lw = fm.width( t->label );
	lw -= t->label.contains('&') * fm.width('&');
	lw += t->label.contains("&&") * fm.width('&');
	int iw = 0;
	int ih = 0;
	if ( t->iconset != 0 ) {
	    iw = t->iconset->pixmap( QIconSet::Small, QIconSet::Normal ).width() + 4;
	    ih = t->iconset->pixmap( QIconSet::Small, QIconSet::Normal ).height();
	}
	int h = QMAX( fm.height(), ih );
	h = QMAX( h, QApplication::globalStrut().height() );

	h += vframe;
	t->r = QRect(QPoint(x, 0), style().sizeFromContents(QStyle::CT_TabBarTab, this,
	             QSize( QMAX( lw + hframe + iw, QApplication::globalStrut().width() ), h ),
		     QStyleOption(t) ));
	x += t->r.width() - overlap;
	r = r.unite( t->r );
	if ( reverse )
	    t = lstatic->prev();
	else
	    t = lstatic->next();
    }
    x += overlap;
    int w = (d->scrolls) ? d->leftB->x() : width();
    if (x + offset < w)
	offset = w - x;
    if (offset > 0)
	offset = 0;

    for ( t = lstatic->first(); t; t = lstatic->next() ) {
	t->r.moveBy( offset, 0 );
	t->r.setHeight( r.height() );
    }

    if ( sizeHint() != oldSh )
	updateGeometry();

    emit layoutChanged();
}

/*!
    \reimp
*/

bool QTabBar::event( QEvent *e )
{
    if ( e->type() == QEvent::LanguageChange ) {
	layoutTabs();
	updateArrowButtons();
	makeVisible( tab( currentTab() ));
    }

    return QWidget::event( e );
}

/*!
  \reimp
*/

void QTabBar::styleChange( QStyle& old )
{
    layoutTabs();
    updateArrowButtons();
    QWidget::styleChange( old );
}

/*!
    \reimp
*/
void QTabBar::focusInEvent( QFocusEvent * )
{
    QTab *t = tab( d->focus );
    if ( t )
	repaint( t->r, FALSE );
}

/*!
    \reimp
*/
void QTabBar::focusOutEvent( QFocusEvent * )
{
    QTab *t = tab( d->focus );
    if ( t )
	repaint( t->r, FALSE );
}

/*!
    \reimp
*/
void QTabBar::resizeEvent( QResizeEvent * )
{
    const int arrowWidth = QMAX( d->btnWidth, QApplication::globalStrut().width() );;
    d->rightB->setGeometry( width() - arrowWidth, 0, arrowWidth, height() );
    d->leftB->setGeometry( width() - 2*arrowWidth, 0, arrowWidth, height() );
    layoutTabs();
    updateArrowButtons();
    makeVisible( tab( currentTab() ));
}

void QTabBar::scrollTabs()
{
    QTab* left = 0;
    QTab* right = 0;
    for ( QTab* t = lstatic->first(); t; t = lstatic->next() ) {
	if ( t->r.left() < 0 && t->r.right() > 0 )
	    left = t;
	if ( t->r.left() < d->leftB->x()+2 )
	    right = t;
    }

    if ( sender() == d->leftB )
	makeVisible( left );
    else  if ( sender() == d->rightB )
	makeVisible( right );
}

void QTabBar::makeVisible( QTab* tab  )
{
    bool tooFarLeft = ( tab && tab->r.left() < 0 );
    bool tooFarRight = ( tab && tab->r.right() >= d->leftB->x() );

    if ( !d->scrolls || ( !tooFarLeft && ! tooFarRight ) )
	return;

    bool bs = signalsBlocked();
    blockSignals(TRUE);
    layoutTabs();
    blockSignals(bs);

    int offset = 0;

    if ( tooFarLeft ) {
	offset = tab->r.left();
	if (tab != lstatic->first())
	    offset -= 8;
    } else if ( tooFarRight ) {
	offset = tab->r.right() - d->leftB->x() + 1;
    }

    for ( QTab* t = lstatic->first(); t; t = lstatic->next() )
	t->r.moveBy( -offset, 0 );

    d->leftB->setEnabled( lstatic->first()->r.left() < 0);
    d->rightB->setEnabled( lstatic->last()->r.right() >= d->leftB->x() );

    // Make sure disabled buttons pop up again
    if ( !d->leftB->isEnabled() && d->leftB->isDown() )
	d->leftB->setDown( FALSE );
    if ( !d->rightB->isEnabled() && d->rightB->isDown() )
	d->rightB->setDown( FALSE );

    update();
    emit layoutChanged();
}

void QTabBar::updateArrowButtons()
{
    if (lstatic->isEmpty()) {
	d->scrolls = FALSE;
    } else {
	d->scrolls = (lstatic->last()->r.right() - lstatic->first()->r.left() > width());
    }
    if ( d->scrolls ) {
	const int arrowWidth = QMAX( d->btnWidth, QApplication::globalStrut().width() );
	if ( QApplication::reverseLayout() ) {
	    d->rightB->setGeometry( arrowWidth, 0, arrowWidth, height() );
	    d->leftB->setGeometry( 0, 0, arrowWidth, height() );
	} else {
	    d->rightB->setGeometry( width() - arrowWidth, 0, arrowWidth, height() );
	    d->leftB->setGeometry( width() - 2*arrowWidth, 0, arrowWidth, height() );
	}

	d->leftB->setEnabled( lstatic->first()->r.left() < 0);
	d->rightB->setEnabled( lstatic->last()->r.right() >= d->leftB->x() );
	d->leftB->show();
	d->rightB->show();
    } else {
	d->leftB->hide();
	d->rightB->hide();
	layoutTabs();
    }
}

/*!
    Removes the tool tip for the tab at index position \a index.
*/
void QTabBar::removeToolTip( int index )
{
#ifndef QT_NO_TOOLTIP
    QTab * tab = tabAt( index );
    if ( !tab || !d->toolTips )
	return;
    d->toolTips->remove( tab );
#endif
}

/*!
    Sets the tool tip for the tab at index position \a index to \a
    tip.
*/
void QTabBar::setToolTip( int index, const QString & tip )
{
#ifndef QT_NO_TOOLTIP
    QTab * tab = tabAt( index );
    if ( !tab )
	return;
    if ( d->toolTips == 0 )
	d->toolTips = new QTabBarToolTip( this );
    d->toolTips->add( tab, tip );
#endif
}

/*!
    Returns the tool tip for the tab at index position \a index.
*/
QString QTabBar::toolTip( int index ) const
{
#ifndef QT_NO_TOOLTIP
    if ( d->toolTips )
	return d->toolTips->tipForTab( tabAt( index ) );
    else
#endif
	return QString();
}

/*!
    Sets the text of the tab to \a text.
*/
void QTab::setText( const QString& text )
{
    label = text;
    if ( tb ) {
#ifndef QT_NO_ACCEL
	tb->d->a->removeItem( id );
	int p = QAccel::shortcutKey( text );
	if ( p )
	    tb->d->a->insertItem( p, id );
#endif
	tb->layoutTabs();
	tb->repaint(FALSE);

#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( tb, tb->indexOf(id)+1, QAccessible::NameChanged );
#endif
    }
}

/*!
    Sets the tab's iconset to \a icon
*/
void QTab::setIconSet( const QIconSet &icon )
{
    iconset = new QIconSet( icon );
}

// this allows us to handle accelerators that are in a QTabBar.
void QTab::setTabBar( QTabBar *newTb )
{
    tb = newTb;
}

/*!
    \internal
*/
void QTabBar::fontChange( const QFont & oldFont )
{
    layoutTabs();
    QWidget::fontChange( oldFont );
}

#endif
