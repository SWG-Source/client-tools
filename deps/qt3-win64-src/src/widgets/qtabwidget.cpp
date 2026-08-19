/****************************************************************************
** $Id: qtabwidget.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QTabWidget class
**
** Created : 990318
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

#include "qtabwidget.h"
#ifndef QT_NO_TABWIDGET
#include "qobjectlist.h"
#include "qtabbar.h"
#include "qapplication.h"
#include "qwidgetstack.h"
#include "qbitmap.h"
#include "qaccel.h"
#include "qstyle.h"
#include "qpainter.h"
#include "qtoolbutton.h"

#ifdef Q_OS_MACX
#include <qmacstyle_mac.h>
#endif

/*!
    \class QTabWidget qtabwidget.h
    \brief The QTabWidget class provides a stack of tabbed widgets.

    \ingroup organizers
    \ingroup advanced
    \mainclass

    A tab widget provides a tab bar of tabs and a `page area' below
    (or above, see \l{TabPosition}) the tabs. Each tab is associated
    with a different widget (called a `page'). Only the current tab's
    page is shown in the page area; all the other tabs' pages are
    hidden. The user can show a different page by clicking on its tab
    or by pressing its Alt+\e{letter} accelerator if it has one.

    The normal way to use QTabWidget is to do the following in the
    constructor:
    \list 1
    \i Create a QTabWidget.
    \i Create a QWidget for each of the pages in the tab dialog,
    insert children into it, set up geometry management for it and use
    addTab() (or insertTab()) to set up a tab and keyboard accelerator
    for it.
    \i Connect to the signals and slots.
    \endlist

    The position of the tabs is set with setTabPosition(), their shape
    with setTabShape(), and their margin with setMargin().

    If you don't call addTab() and the QTabWidget is already visible,
    then the page you have created will not be visible. Don't
    confuse the object name you supply to the QWidget constructor and
    the tab label you supply to addTab(). addTab() takes a name which
    indicates an accelerator and is meaningful and descriptive to the
    user, whereas the widget name is used primarily for debugging.

    The signal currentChanged() is emitted when the user selects a
    page.

    The current page is available as an index position with
    currentPageIndex() or as a wiget pointer with currentPage(). You
    can retrieve a pointer to a page with a given index using page(),
    and can find the index position of a page with indexOf(). Use
    setCurrentPage() to show a particular page by index, or showPage()
    to show a page by widget pointer.

    You can change a tab's label and iconset using changeTab() or
    setTabLabel() and setTabIconSet(). A tab page can be removed with
    removePage().

    Each tab is either enabled or disabled at any given time (see
    setTabEnabled()). If a tab is enabled, the tab text is drawn
    normally and the user can select that tab. If it is disabled, the
    tab is drawn in a different way and the user cannot select that
    tab. Note that even if a tab is disabled, the page can still be
    visible, for example if all of the tabs happen to be disabled.

    Although tab widgets can be a very good way to split up a complex
    dialog, it's also very easy to get into a mess. See QTabDialog for
    some design hints. An alternative is to use a QWidgetStack for
    which you provide some means of navigating between pages, for
    example, a QToolBar or a QListBox.

    Most of the functionality in QTabWidget is provided by a QTabBar
    (at the top, providing the tabs) and a QWidgetStack (most of the
    area, organizing the individual pages).

    <img src=qtabwidget-m.png> <img src=qtabwidget-w.png>

    \sa QTabDialog, QToolBox
*/

/*!
  \enum Qt::Corner
  This enum type specifies a corner in a rectangle:
  \value TopLeft top left corner
  \value TopRight top right corner
  \value BottomLeft bottom left corner
  \value BottomRight bottom right corner
*/

/*!
    \enum QTabWidget::TabPosition

    This enum type defines where QTabWidget draws the tab row:
    \value Top  above the pages
    \value Bottom  below the pages
*/

/*!
    \enum QTabWidget::TabShape

    This enum type defines the shape of the tabs:
    \value Rounded  rounded look (normal)
    \value Triangular  triangular look (very unusual, included for completeness)
*/

/* undocumented now
  \obsolete

  \fn void QTabWidget::selected( const QString &tabLabel );

  This signal is emitted whenever a tab is selected (raised),
  including during the first show().

  \sa raise()
*/


/*!
    \fn void QTabWidget::currentChanged( QWidget* );

    This signal is emitted whenever the current page changes. The
    parameter is the new current page.

    \sa currentPage(), showPage(), tabLabel()
*/

class QTabBarBase : public QWidget
{
public:
    QTabBarBase( QTabWidget* parent=0, const char* name=0 )
        : QWidget( parent, name ) {};
protected:
    void paintEvent( QPaintEvent * )
    {
        QObject * obj = parent();
        if( obj ){
            QTabWidget * t = (QTabWidget *) obj;
            QPainter p( this );
	    QStyle::SFlags flags = QStyle::Style_Default;

	    if ( t->tabPosition() == QTabWidget::Top )
		flags |= QStyle::Style_Top;
	    if ( t->tabPosition() == QTabWidget::Bottom )
		flags |= QStyle::Style_Bottom;
	    if(parentWidget()->isEnabled())
		flags |= QStyle::Style_Enabled;

	    style().drawPrimitive( QStyle::PE_TabBarBase, &p, rect(),
				   colorGroup(), flags );
        }
    }
};

class QTabWidgetData
{
public:
    QTabWidgetData()
        : tabs(0), tabBase(0), stack(0), dirty( TRUE ),
          pos( QTabWidget::Top ), shape( QTabWidget::Rounded ),
	  leftCornerWidget(0), rightCornerWidget(0) {};
    ~QTabWidgetData(){};
    QTabBar* tabs;
    QTabBarBase* tabBase;
    QWidgetStack* stack;
    bool dirty;
    QTabWidget::TabPosition pos;
    QTabWidget::TabShape shape;
    int alignment;
    QWidget* leftCornerWidget;
    QWidget* rightCornerWidget;
};

/*!
    Constructs a tabbed widget called \a name with parent \a parent,
    and widget flags \a f.
*/
QTabWidget::QTabWidget( QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f )
{
    d = new QTabWidgetData;
    d->stack = new QWidgetStack( this, "tab pages" );
    d->stack->installEventFilter( this );
    d->tabBase = new QTabBarBase( this, "tab base" );
    d->tabBase->resize( 1, 1 );
    setTabBar( new QTabBar( this, "tab control" ) );

    d->stack->setFrameStyle( QFrame::TabWidgetPanel | QFrame::Raised );
#ifdef Q_OS_TEMP
    d->pos = Bottom;
#endif

    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    setFocusPolicy( TabFocus );
    setFocusProxy( d->tabs );

    installEventFilter( this );
#ifdef Q_OS_MACX
    if (::qt_cast<QMacStyle*>(&style()))
        setMargin(10); // According to HIGuidelines at least.
#endif
}

/*!
    \reimp
*/
QTabWidget::~QTabWidget()
{
    delete d;
}

/*!
    Adds another tab and page to the tab view.

    The new page is \a child; the tab's label is \a label. Note the
    difference between the widget name (which you supply to widget
    constructors and to setTabEnabled(), for example) and the tab
    label. The name is internal to the program and invariant, whereas
    the label is shown on-screen and may vary according to language
    and other factors.

    If the tab's \a label contains an ampersand, the letter following
    the ampersand is used as an accelerator for the tab, e.g. if the
    label is "Bro\&wse" then Alt+W becomes an accelerator which will
    move the focus to this tab.

    If you call addTab() after show() the screen will flicker and the
    user may be confused.

    Adding the same child twice will have undefined behavior.

    \sa insertTab()
*/
void QTabWidget::addTab( QWidget *child, const QString &label)
{
    insertTab( child, label );
}


/*!
    \overload

    Adds another tab and page to the tab view.

    This function is the same as addTab(), but with an additional \a
    iconset.
*/
void QTabWidget::addTab( QWidget *child, const QIconSet& iconset, const QString &label )
{
    insertTab( child, iconset, label );
}

/*!
    \overload

    This is a low-level function for adding tabs. It is useful if you
    are using setTabBar() to set a QTabBar subclass with an overridden
    QTabBar::paint() function for a subclass of QTab. The \a child is
    the new page and \a tab is the tab to put the \a child on.
*/
void QTabWidget::addTab( QWidget *child, QTab* tab )
{
    insertTab( child, tab );
}



/*!
    Inserts another tab and page to the tab view.

    The new page is \a child; the tab's label is \a label. Note the
    difference between the widget name (which you supply to widget
    constructors and to setTabEnabled(), for example) and the tab
    label. The name is internal to the program and invariant, whereas
    the label is shown on-screen and may vary according to language
    and other factors.

    If the tab's \a label contains an ampersand, the letter following
    the ampersand is used as an accelerator for the tab, e.g. if the
    label is "Bro\&wse" then Alt+W becomes an accelerator which will
    move the focus to this tab.

    If \a index is not specified, the tab is simply appended.
    Otherwise it is inserted at the specified position.

    If you call insertTab() after show(), the screen will flicker and
    the user may be confused.

    \sa addTab()
*/
void QTabWidget::insertTab( QWidget *child, const QString &label, int index)
{
    QTab * t = new QTab();
    Q_CHECK_PTR( t );
    t->label = label;
    insertTab( child, t, index );
}


/*!
    \overload

    Inserts another tab and page to the tab view.

    This function is the same as insertTab(), but with an additional
    \a iconset.
*/
void QTabWidget::insertTab( QWidget *child, const QIconSet& iconset, const QString &label, int index )
{
    QTab * t = new QTab();
    Q_CHECK_PTR( t );
    t->label = label;
    t->iconset = new QIconSet( iconset );
    insertTab( child, t, index );
}

/*!
    \overload

    This is a lower-level method for inserting tabs, similar to the
    other insertTab() method. It is useful if you are using
    setTabBar() to set a QTabBar subclass with an overridden
    QTabBar::paint() function for a subclass of QTab. The \a child is
    the new page, \a tab is the tab to put the \a child on and \a
    index is the position in the tab bar that this page should occupy.
*/
void QTabWidget::insertTab( QWidget *child, QTab* tab, int index)
{
    tab->enabled = TRUE;
    int id = d->tabs->insertTab( tab, index );
    d->stack->addWidget( child, id );
    if ( d->stack->frameStyle() != ( QFrame::TabWidgetPanel | QFrame::Raised ) )
        d->stack->setFrameStyle( QFrame::TabWidgetPanel | QFrame::Raised );
    setUpLayout();
}


/*!
    Defines a new \a label for page \a{w}'s tab.
*/
void QTabWidget::changeTab( QWidget *w, const QString &label)
{
    int id = d->stack->id( w );
    if ( id < 0 )
        return;
    QTab* t = d->tabs->tab( id );
    if ( !t )
        return;
    // this will update the accelerators
    t->setText( label );

    d->tabs->layoutTabs();
    d->tabs->update();
    setUpLayout();
}

/*!
    \overload

    Defines a new \a iconset and a new \a label for page \a{w}'s tab.
*/
void QTabWidget::changeTab( QWidget *w, const QIconSet& iconset, const QString &label)
{
    int id = d->stack->id( w );
    if ( id < 0 )
        return;
    QTab* t = d->tabs->tab( id );
    if ( !t )
        return;
    if ( t->iconset ) {
        delete t->iconset;
        t->iconset = 0;
    }
    // this will update the accelerators
    t->iconset = new QIconSet( iconset );
    t->setText( label );

    d->tabs->layoutTabs();
    d->tabs->update();
    setUpLayout();
}

/*!
    Returns TRUE if the page \a w is enabled; otherwise returns FALSE.

    \sa setTabEnabled(), QWidget::isEnabled()
*/

bool QTabWidget::isTabEnabled( QWidget* w ) const
{
    int id = d->stack->id( w );
    if ( id >= 0 )
        return w->isEnabled();
    else
        return FALSE;
}

/*!
    If \a enable is TRUE, page \a w is enabled; otherwise page \a w is
    disabled. The page's tab is redrawn appropriately.

    QTabWidget uses QWidget::setEnabled() internally, rather than
    keeping a separate flag.

    Note that even a disabled tab/page may be visible. If the page is
    visible already, QTabWidget will not hide it; if all the pages are
    disabled, QTabWidget will show one of them.

    \sa isTabEnabled(), QWidget::setEnabled()
*/

void QTabWidget::setTabEnabled( QWidget* w, bool enable)
{
    int id = d->stack->id( w );
    if ( id >= 0 ) {
        w->setEnabled( enable );
        d->tabs->setTabEnabled( id, enable );
    }
}

/*!
  Sets widget \a w to be the shown in the specified \a corner of the
  tab widget.

  Only the horizontal element of the \a corner will be used.

  \sa cornerWidget(), setTabPosition()
*/
void QTabWidget::setCornerWidget( QWidget * w, Qt::Corner corner )
{
    if ( !w )
	return;
    if ( (uint)corner & 1 )
	d->rightCornerWidget = w;
    else
	d->leftCornerWidget = w;
}

/*!
    Returns the widget shown in the \a corner of the tab widget or 0.
*/
QWidget * QTabWidget::cornerWidget( Qt::Corner corner ) const
{
    if ( (uint)corner & 1 )
	return d->rightCornerWidget;
    return d->leftCornerWidget;
}

/*!
    Ensures that page \a w is shown. This is useful mainly for
    accelerators.

    \warning Used carelessly, this function can easily surprise or
    confuse the user.

    \sa QTabBar::setCurrentTab()
*/
void QTabWidget::showPage( QWidget * w)
{
    int id = d->stack->id( w );
    if ( id >= 0 ) {
        d->stack->raiseWidget( w );
        d->tabs->setCurrentTab( id );
        // ### why overwrite the frame style?
        if ( d->stack->frameStyle() != ( QFrame::TabWidgetPanel |QFrame::Raised ) )
            d->stack->setFrameStyle( QFrame::TabWidgetPanel | QFrame::Raised );
    }
}

/*!
    Removes page \a w from this stack of widgets. Does not delete \a
    w.

    \sa addTab(), showPage(), QWidgetStack::removeWidget()
*/
void QTabWidget::removePage( QWidget * w )
{
    int id = d->stack->id( w );
    if ( id >= 0 ) {
	int oldId = d->stack->id(currentPage());
	bool fixCurrentTab = oldId == id;
	//switches to the next enabled tab
	d->tabs->setTabEnabled( id, FALSE );
	//if no next enabled page we fix the current page
	fixCurrentTab = fixCurrentTab && oldId == d->stack->id(currentPage());

	d->stack->removeWidget( w );
	d->tabs->removeTab( d->tabs->tab(id) );
	if ( fixCurrentTab )
	    showTab( d->tabs->currentTab() );
	setUpLayout();

	if ( d->tabs->count() == 0 )
	    d->stack->setFrameStyle( QFrame::NoFrame );
    }
}

/*!
    Returns the label text for the tab on page \a w.
*/

QString QTabWidget::tabLabel( QWidget * w ) const
{
    QTab * t = d->tabs->tab( d->stack->id( w ) );
    return t ? t->label : QString::null;
}

/*!
    Sets the tab label for page \a w to \a l
*/

void QTabWidget::setTabLabel( QWidget * w, const QString &l )
{
    QTab * t = d->tabs->tab( d->stack->id( w ) );
    if ( t )
        t->label = l;
    d->tabs->layoutTabs();
    d->tabs->update();
    setUpLayout();
}

/*!
    Returns a pointer to the page currently being displayed by the tab
    dialog. The tab dialog does its best to make sure that this value
    is never 0 (but if you try hard enough, it can be).
*/

QWidget * QTabWidget::currentPage() const
{
    return page( currentPageIndex() );
}

/*!
    \property QTabWidget::autoMask
    \brief whether the tab widget is automatically masked

    \sa QWidget::setAutoMask()
*/

/*!
    \property QTabWidget::currentPage
    \brief the index position of the current tab page

  \sa QTabBar::currentTab()
*/

int QTabWidget::currentPageIndex() const
{
    return d->tabs->indexOf( d->tabs->currentTab() );
}

void QTabWidget::setCurrentPage( int index )
{
    d->tabs->setCurrentTab( d->tabs->tabAt( index ) );
    showTab( d->tabs->currentTab() );
}


/*!
    Returns the index position of page \a w, or -1 if the widget
    cannot be found.
*/
int QTabWidget::indexOf( QWidget* w ) const
{
    return d->tabs->indexOf( d->stack->id( w ) );
}


/*!
    \reimp
*/
void QTabWidget::resizeEvent( QResizeEvent *e )
{
    QWidget::resizeEvent( e );
    setUpLayout();
}

/*!
    Replaces the dialog's QTabBar heading with the tab bar \a tb. Note
    that this must be called \e before any tabs have been added, or
    the behavior is undefined.

    \sa tabBar()
*/
void QTabWidget::setTabBar( QTabBar* tb)
{
    if ( tb->parentWidget() != this )
        tb->reparent( this, QPoint(0,0), TRUE );
    delete d->tabs;
    d->tabs = tb;
    setFocusProxy( d->tabs );
    connect( d->tabs, SIGNAL(selected(int)),
             this,    SLOT(showTab(int)) );
    setUpLayout();
}


/*!
    Returns the current QTabBar.

    \sa setTabBar()
*/
QTabBar* QTabWidget::tabBar() const
{
    return d->tabs;
}

/*!
    Ensures that the selected tab's page is visible and appropriately
    sized.
*/

void QTabWidget::showTab( int i )
{
    if ( d->stack->widget( i ) ) {
	d->stack->raiseWidget( i );
	emit selected( d->tabs->tab( i )->label );
	emit currentChanged( d->stack->widget( i ) );
    }
}

/*
    Set up the layout.
*/
void QTabWidget::setUpLayout( bool onlyCheck )
{
    if ( onlyCheck && !d->dirty )
	return; // nothing to do

    if ( !isVisible() ) {
	d->dirty = TRUE;
	return; // we'll do it later
    }

    QSize t( 0, d->stack->frameWidth() );
    if ( d->tabs->isVisibleTo(this) )
	t = d->tabs->sizeHint();
    int lcw = 0;
    if ( d->leftCornerWidget && d->leftCornerWidget->isVisible()  ) {
	QSize sz = d->leftCornerWidget->sizeHint();
	d->leftCornerWidget->resize(sz);
	lcw = sz.width();
	if ( t.height() > lcw )
	    lcw = t.height();
     }
    int rcw = 0;
    if ( d->rightCornerWidget && d->rightCornerWidget->isVisible() ) {
	QSize sz = d->rightCornerWidget->sizeHint();
	d->rightCornerWidget->resize(sz);
	rcw = sz.width();
	if ( t.height() > rcw )
	    rcw = t.height();
    }
    int tw = width() - lcw - rcw;
    if ( t.width() > tw )
	t.setWidth( tw );
    int lw = d->stack->lineWidth();
    bool reverse = QApplication::reverseLayout();
    int tabx, taby, stacky, exty, exth, overlap;

    exth = style().pixelMetric( QStyle::PM_TabBarBaseHeight, this );
    overlap = style().pixelMetric( QStyle::PM_TabBarBaseOverlap, this );

    if ( reverse )
	tabx = QMIN( width() - t.width(), width() - t.width() - lw + 2 ) - lcw;
    else
	tabx = QMAX( 0, lw - 2 ) + lcw;
    if ( d->pos == Bottom ) {
	taby = height() - t.height() - lw;
	stacky = 0;
	exty = taby - (exth - overlap);
    } else { // Top
	taby = 0;
	stacky = t.height()-lw + (exth - overlap);
	exty = taby + t.height() - overlap;
    }

    // do alignment
    int alignment = style().styleHint( QStyle::SH_TabBar_Alignment, this );
    if ( alignment != AlignLeft && t.width() < width() ) {
	if ( alignment == AlignHCenter )
	    tabx += (width()-lcw-rcw)/2 - t.width()/2;
	else if ( alignment == AlignRight )
	    tabx += width() - t.width() - rcw;
    }

    d->tabs->setGeometry( tabx, taby, t.width(), t.height() );
    d->tabBase->setGeometry( 0, exty, width(), exth );
    if ( exth == 0 )
	d->tabBase->hide();
    else
	d->tabBase->show();

    d->stack->setGeometry( 0, stacky, width(), height() - (exth-overlap) -
			   t.height()+QMAX(0, lw-2));

    d->dirty = FALSE;

    // move cornerwidgets
    if ( d->leftCornerWidget ) {
	int y = ( t.height() / 2 ) - ( d->leftCornerWidget->height() / 2 );
	int x = ( reverse ? width() - lcw + y : y );
	d->leftCornerWidget->move( x, y + taby );
    }
    if ( d->rightCornerWidget ) {
	int y = ( t.height() / 2 ) - ( d->rightCornerWidget->height() / 2 );
	int x = ( reverse ? y : width() - rcw + y );
	d->rightCornerWidget->move( x, y + taby );
    }    
    if ( !onlyCheck )
	update();
    updateGeometry();
    if ( autoMask() )
	updateMask();
}

/*!
    \reimp
*/
QSize QTabWidget::sizeHint() const
{
    QSize lc(0, 0), rc(0, 0);

    if (d->leftCornerWidget)
	lc = d->leftCornerWidget->sizeHint();
    if(d->rightCornerWidget)
	rc = d->rightCornerWidget->sizeHint();
    if ( !d->dirty ) {
	QTabWidget *that = (QTabWidget*)this;
	that->setUpLayout( TRUE );
    }
    QSize s( d->stack->sizeHint() );
    QSize t( d->tabs->sizeHint() );
    if(!style().styleHint(QStyle::SH_TabBar_PreferNoArrows, d->tabs))
	t = t.boundedTo( QSize(200,200) );
    else
        t = t.boundedTo( QApplication::desktop()->size() );

    QSize sz( QMAX( s.width(), t.width() + rc.width() + lc.width() ),
	      s.height() + (QMAX( rc.height(), QMAX(lc.height(), t.height()))) + ( d->tabBase->isVisible() ? d->tabBase->height() : 0 ) );
    return style().sizeFromContents(QStyle::CT_TabWidget, this, sz).expandedTo(QApplication::globalStrut());
}


/*!
    \reimp

    Returns a suitable minimum size for the tab widget.
*/
QSize QTabWidget::minimumSizeHint() const
{
    QSize lc(0, 0), rc(0, 0);

    if(d->leftCornerWidget)
	lc = d->leftCornerWidget->minimumSizeHint();
    if(d->rightCornerWidget)
	rc = d->rightCornerWidget->minimumSizeHint();
    if ( !d->dirty ) {
	QTabWidget *that = (QTabWidget*)this;
	that->setUpLayout( TRUE );
    }
    QSize s( d->stack->minimumSizeHint() );
    QSize t( d->tabs->minimumSizeHint() );

    QSize sz( QMAX( s.width(), t.width() + rc.width() + lc.width() ),
	      s.height() + (QMAX( rc.height(), QMAX(lc.height(), t.height()))) + ( d->tabBase->isVisible() ? d->tabBase->height() : 0 ) );
    return style().sizeFromContents(QStyle::CT_TabWidget, this, sz).expandedTo(QApplication::globalStrut());
}

/*!
    \reimp
 */
void QTabWidget::showEvent( QShowEvent * )
{
    setUpLayout();
}


/*!
    \property QTabWidget::tabPosition
    \brief the position of the tabs in this tab widget

    Possible values for this property are \c QTabWidget::Top and \c
    QTabWidget::Bottom.

    \sa TabPosition
*/
QTabWidget::TabPosition QTabWidget::tabPosition() const
{
    return d->pos;
}

void QTabWidget::setTabPosition( TabPosition pos)
{
    if (d->pos == pos)
        return;
    d->pos = pos;
    if (d->tabs->shape() == QTabBar::TriangularAbove || d->tabs->shape() == QTabBar::TriangularBelow ) {
        if ( pos == Bottom )
            d->tabs->setShape( QTabBar::TriangularBelow );
        else
            d->tabs->setShape( QTabBar::TriangularAbove );
    }
    else {
        if ( pos == Bottom )
            d->tabs->setShape( QTabBar::RoundedBelow );
        else
            d->tabs->setShape( QTabBar::RoundedAbove );
    }
    d->tabs->layoutTabs();
    setUpLayout();
}

/*!
    \property QTabWidget::tabShape
    \brief the shape of the tabs in this tab widget

    Possible values for this property are \c QTabWidget::Rounded
    (default) or \c QTabWidget::Triangular.

    \sa TabShape
*/

QTabWidget::TabShape QTabWidget::tabShape() const
{
    return d->shape;
}

void QTabWidget::setTabShape( TabShape s )
{
    if ( d->shape == s )
        return;
    d->shape = s;
    if ( d->pos == Top ) {
        if ( s == Rounded )
            d->tabs->setShape( QTabBar::RoundedAbove );
        else
            d->tabs->setShape( QTabBar::TriangularAbove );
    } else {
        if ( s == Rounded )
            d->tabs->setShape( QTabBar::RoundedBelow );
        else
            d->tabs->setShape( QTabBar::TriangularBelow );
    }
    d->tabs->layoutTabs();
    setUpLayout();
}


/*!
    \property QTabWidget::margin
    \brief the margin in this tab widget

    The margin is the distance between the innermost pixel of the
    frame and the outermost pixel of the pages.
*/
int QTabWidget::margin() const
{
    return d->stack->margin();
}

void QTabWidget::setMargin( int w )
{
    d->stack->setMargin( w );
    setUpLayout();
}


/*!
    \reimp
 */
void QTabWidget::styleChange( QStyle& old )
{
    QWidget::styleChange( old );
    setUpLayout();
}


/*!
    \reimp
 */
void QTabWidget::updateMask()
{
    if ( !autoMask() )
        return;

    QRect r;
    QRegion reg( r );
    reg += QRegion( d->tabs->geometry() );
    reg += QRegion( d->stack->geometry() );
    setMask( reg );
}


/*!
    \reimp
 */
bool QTabWidget::eventFilter( QObject *o, QEvent * e)
{
    if ( o == this ) {
	if ( e->type() == QEvent::LanguageChange || e->type() == QEvent::LayoutHint ) {
	    d->dirty = TRUE;
	    setUpLayout();
	    updateGeometry();
	} else if ( e->type() == QEvent::KeyPress ) {
	    QKeyEvent *ke = (QKeyEvent*) e;
	    if ( ( ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab ) &&
		 count() > 1 &&
		 ke->state() & Qt::ControlButton ) {
		int page = currentPageIndex();
		if ( ke->key() == Qt::Key_Backtab || ke->state() & Qt::ShiftButton ) {
		    page--;
		    if ( page < 0 )
			page = count() - 1;
		} else {
		    page++;
		    if ( page >= count() )
			page = 0;
		}
		setCurrentPage( page );
		if ( !qApp->focusWidget() )
		    d->tabs->setFocus();
		return TRUE;
	    }
	}

    } else if ( o == d->stack ) {
	if ( e->type() == QEvent::ChildRemoved
	     && ( (QChildEvent*)e )->child()->isWidgetType() ) {
	    removePage( (QWidget*)  ( (QChildEvent*)e )->child() );
	    return TRUE;
	} else if ( e->type() == QEvent::LayoutHint ) {
	    updateGeometry();
	}
    }
    return FALSE;
}

/*!
    Returns the tab page at index position \a index or 0 if the \a
    index is out of range.
*/
QWidget *QTabWidget::page( int index ) const
{
    QTab *t = d->tabs->tabAt(index);
    if ( t )
	return d->stack->widget( t->id );
    // else
    return 0;
}

/*!
    Returns the label of the tab at index position \a index or
    QString::null if the \a index is out of range.
*/
QString QTabWidget::label( int index ) const
{
    QTab *t = d->tabs->tabAt( index );
    if ( t )
 	return t->label;
    // else
    return QString::null;
}

/*!
    \property QTabWidget::count
    \brief the number of tabs in the tab bar
*/
int QTabWidget::count() const
{
    return d->tabs->count();
}

/*!
    Returns the iconset of page \a w or a \link QIconSet::QIconSet()
    null iconset\endlink if \a w is not a tab page or does not have an
    iconset.
*/
QIconSet QTabWidget::tabIconSet( QWidget * w ) const
{
    int id = d->stack->id( w );
    if ( id < 0 )
        return QIconSet();
    QTab* t = d->tabs->tab( id );
    if ( !t )
        return QIconSet();
    if ( t->iconset )
	return QIconSet( *t->iconset );
    else
	return QIconSet();
}

/*!
    Sets the iconset for page \a w to \a iconset.
*/
void QTabWidget::setTabIconSet( QWidget * w, const QIconSet & iconset )
{
    int id = d->stack->id( w );
    if ( id < 0 )
        return;
    QTab* t = d->tabs->tab( id );
    if ( !t )
        return;
    if ( t->iconset )
        delete t->iconset;
    t->iconset = new QIconSet( iconset );

    d->tabs->layoutTabs();
    d->tabs->update();
    setUpLayout();
}

/*!
    Sets the tab tool tip for page \a w to \a tip.

    \sa removeTabToolTip(), tabToolTip()
*/
void QTabWidget::setTabToolTip( QWidget * w, const QString & tip )
{
    int index = d->tabs->indexOf( d->stack->id( w ) );
    if ( index < 0 )
        return;
    d->tabs->setToolTip( index, tip );
}

/*!
    Returns the tab tool tip for page \a w or QString::null if no tool
    tip has been set.

    \sa setTabToolTip(), removeTabToolTip()
*/
QString QTabWidget::tabToolTip( QWidget * w ) const
{
    int index = d->tabs->indexOf( d->stack->id( w ) );
    if ( index < 0 )
        return QString();
    return d->tabs->toolTip( index );
}

/*!
    Removes the tab tool tip for page \a w. If the page does not have
    a tip, nothing happens.

    \sa setTabToolTip(), tabToolTip()
*/
void QTabWidget::removeTabToolTip( QWidget * w )
{
    int index = d->tabs->indexOf( d->stack->id( w ) );
    if ( index < 0 )
        return;
    d->tabs->removeToolTip( index );
}

#endif
