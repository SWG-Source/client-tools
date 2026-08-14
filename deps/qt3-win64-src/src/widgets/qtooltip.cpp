/****************************************************************************
** $Id: qtooltip.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Tool Tips (or Balloon Help) for any widget or rectangle
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

#include "qtooltip.h"
#ifndef QT_NO_TOOLTIP
#include "qlabel.h"
#include "qptrdict.h"
#include "qapplication.h"
#include "qguardedptr.h"
#include "qtimer.h"
#include "qeffects_p.h"

static bool globally_enabled = TRUE;

// Magic value meaning an entire widget - if someone tries to insert a
// tool tip on this part of a widget it will be interpreted as the
// entire widget.

static inline QRect entireWidget()
{
    return QRect( -QWIDGETSIZE_MAX, -QWIDGETSIZE_MAX,
		  2*QWIDGETSIZE_MAX, 2*QWIDGETSIZE_MAX );
}

// Internal class - don't touch

class QTipLabel : public QLabel
{
    Q_OBJECT
public:
    QTipLabel( QWidget* parent, const QString& text) : QLabel( parent, "toolTipTip",
	     WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM )
    {
	setMargin(1);
	setAutoMask( FALSE );
	setFrameStyle( QFrame::Plain | QFrame::Box );
	setLineWidth( 1 );
	setAlignment( AlignAuto | AlignTop );
	setIndent(0);
	polish();
	setText(text);
	adjustSize();
    }
    void setWidth( int w ) { resize( sizeForWidth( w ) ); }
};

// Internal class - don't touch

class QTipManager : public QObject
{
    Q_OBJECT
public:
    QTipManager();
   ~QTipManager();

    struct Tip
    {
	QRect		rect;
	QString		text;
	QString	        groupText;
	QToolTipGroup  *group;
	QToolTip       *tip;
	bool	        autoDelete;
	QRect 		geometry;
	Tip	       *next;
    };

    bool    eventFilter( QObject * o, QEvent * e );
    void    add( const QRect &gm, QWidget *, const QRect &, const QString& ,
		 QToolTipGroup *, const QString& , QToolTip *, bool );
    void    add( QWidget *, const QRect &, const QString& ,
		 QToolTipGroup *, const QString& , QToolTip *, bool );
    void    remove( QWidget *, const QRect &, bool delayhide = FALSE );
    void    remove( QWidget * );

    void    removeFromGroup( QToolTipGroup * );

    void    hideTipAndSleep();

    QString find( QWidget *, const QPoint& );
    void setWakeUpDelay(int);

public slots:
    void    hideTip();

private slots:
    void    labelDestroyed();
    void    clientWidgetDestroyed();
    void    showTip();
    void    allowAnimation();

private:
    QTimer  wakeUp;
    int wakeUpDelay;
    QTimer  fallAsleep;

    QPtrDict<Tip> *tips;
    QTipLabel *label;
    QPoint pos;
    QGuardedPtr<QWidget> widget;
    Tip *currentTip;
    Tip *previousTip;
    bool preventAnimation;
    bool isApplicationFilter;
    QTimer *removeTimer;
};


// We have a global, internal QTipManager object

static QTipManager *tipManager	  = 0;

static void initTipManager()
{
    if ( !tipManager ) {
	tipManager = new QTipManager;
	Q_CHECK_PTR( tipManager );
    }
}


QTipManager::QTipManager()
    : QObject( qApp, "toolTipManager" )
{
    wakeUpDelay = 700;
    tips = new QPtrDict<QTipManager::Tip>( 313 );
    currentTip = 0;
    previousTip = 0;
    label = 0;
    preventAnimation = FALSE;
    isApplicationFilter = FALSE;
    connect( &wakeUp, SIGNAL(timeout()), SLOT(showTip()) );
    connect( &fallAsleep, SIGNAL(timeout()), SLOT(hideTip()) );
    removeTimer = new QTimer( this );
}


QTipManager::~QTipManager()
{
    if ( isApplicationFilter && !qApp->closingDown() ) {
	qApp->setGlobalMouseTracking( FALSE );
	qApp->removeEventFilter( tipManager );
    }

    delete label;
    label = 0;

    if ( tips ) {
	QPtrDictIterator<QTipManager::Tip> i( *tips );
	QTipManager::Tip *t, *n;
	void *k;
	while( (t = i.current()) != 0 ) {
	    k = i.currentKey();
	    ++i;
	    tips->take( k );
	    while ( t ) {
		n = t->next;
		delete t;
		t = n;
	    }
	}
	delete tips;
    }

    tipManager = 0;
}

void QTipManager::add( const QRect &gm, QWidget *w,
		       const QRect &r, const QString &s,
		       QToolTipGroup *g, const QString& gs,
		       QToolTip *tt, bool a )
{
    remove( w, r, TRUE );
    QTipManager::Tip *h = (*tips)[ w ];
    QTipManager::Tip *t = new QTipManager::Tip;
    t->next = h;
    t->tip = tt;
    t->autoDelete = a;
    t->text = s;
    t->rect = r;
    t->groupText = gs;
    t->group = g;
    t->geometry = gm;

    if ( h ) {
	tips->take( w );
	if ( h != currentTip && h->autoDelete ) {
	    t->next = h->next;
	    delete h;
	}
    } else
	connect( w, SIGNAL(destroyed()), this, SLOT(clientWidgetDestroyed()) );

    tips->insert( w, t );

    if ( a && t->rect.contains( pos ) && (!g || g->enabled()) ) {
	removeTimer->stop();
	showTip();
    }

    if ( !isApplicationFilter && qApp ) {
	isApplicationFilter = TRUE;
	qApp->installEventFilter( tipManager );
	qApp->setGlobalMouseTracking( TRUE );
    }

    if ( t->group ) {
	disconnect( removeTimer, SIGNAL( timeout() ),
		 t->group, SIGNAL( removeTip() ) );
	connect( removeTimer, SIGNAL( timeout() ),
		 t->group, SIGNAL( removeTip() ) );
    }
}

void QTipManager::add( QWidget *w, const QRect &r, const QString &s,
		       QToolTipGroup *g, const QString& gs,
		       QToolTip *tt, bool a )
{
    add( QRect( -1, -1, -1, -1 ), w, r, s, g, gs, tt, a );
}


void QTipManager::remove( QWidget *w, const QRect & r, bool delayhide )
{
    QTipManager::Tip *t = (*tips)[ w ];
    if ( t == 0 )
	return;

    if ( t == currentTip )
	if (!delayhide)
	    hideTip();
	else
	    currentTip->autoDelete = TRUE;

    if ( t == previousTip )
	previousTip = 0;

    if ( ( currentTip != t || !delayhide ) && t->rect == r ) {
	tips->take( w );
	if ( t->next )
	    tips->insert( w, t->next );
	delete t;
    } else {
	while(  t->next && t->next->rect != r && ( currentTip != t->next || !delayhide ))
	    t = t->next;
	if ( t->next ) {
	    QTipManager::Tip *d = t->next;
	    t->next = t->next->next;
	    delete d;
	}
    }

    if ( (*tips)[ w ] == 0 )
	disconnect( w, SIGNAL(destroyed()), this, SLOT(clientWidgetDestroyed()) );
#if 0 // not needed, leads sometimes to crashes
    if ( tips->isEmpty() ) {
	// the manager will be recreated if needed
	delete tipManager;
	tipManager = 0;
    }
#endif
}


/*
  The label was destroyed in the program cleanup phase.
*/

void QTipManager::labelDestroyed()
{
    label = 0;
}


/*
  Remove sender() from the tool tip data structures.
*/

void QTipManager::clientWidgetDestroyed()
{
    const QObject *s = sender();
    if ( s )
	remove( (QWidget*) s );
}


void QTipManager::remove( QWidget *w )
{
    QTipManager::Tip *t = (*tips)[ w ];
    if ( t == 0 )
	return;

    tips->take( w );
    QTipManager::Tip * d;
    while ( t ) {
	if ( t == currentTip )
	    hideTip();
	d = t->next;
	delete t;
	t = d;
    }

    disconnect( w, SIGNAL(destroyed()), this, SLOT(clientWidgetDestroyed()) );
#if 0
    if ( tips->isEmpty() ) {
	delete tipManager;
	tipManager = 0;
    }
#endif
}


void QTipManager::removeFromGroup( QToolTipGroup *g )
{
    QPtrDictIterator<QTipManager::Tip> i( *tips );
    QTipManager::Tip *t;
    while( (t = i.current()) != 0 ) {
	++i;
	while ( t ) {
	    if ( t->group == g ) {
		if ( t->group )
		    disconnect( removeTimer, SIGNAL( timeout() ),
				t->group, SIGNAL( removeTip() ) );
		t->group = 0;
	    }
	    t = t->next;
	}
    }
}



bool QTipManager::eventFilter( QObject *obj, QEvent *e )
{
    // avoid dumping core in case of application madness, and return
    // quickly for some common but irrelevant events
    if ( e->type() == QEvent::WindowDeactivate &&
	qApp && !qApp->activeWindow() &&
	label && label->isVisible() )
	hideTipAndSleep();

    if ( !qApp
	 || !obj || !obj->isWidgetType() // isWidgetType() catches most stuff
	 || e->type() == QEvent::Paint
	 || e->type() == QEvent::Timer
	 || e->type() == QEvent::SockAct
	 || !tips )
	return FALSE;
    QWidget *w = (QWidget *)obj;

    if ( e->type() == QEvent::FocusOut || e->type() == QEvent::FocusIn ) {
	// user moved focus somewhere - hide the tip and sleep
	if ( ((QFocusEvent*)e)->reason() != QFocusEvent::Popup )
	    hideTipAndSleep();
	return FALSE;
    }

    QTipManager::Tip *t = 0;
    while( w && !t ) {
	t = (*tips)[ w ];
	if ( !t )
	    w = w->isTopLevel() ? 0 : w->parentWidget();
    }
    if ( !w )
	return FALSE;

    if ( !t && e->type() != QEvent::MouseMove) {
	if ( ( e->type() >= QEvent::MouseButtonPress &&
	       e->type() <= QEvent::FocusOut) || e->type() == QEvent::Leave )
	    hideTip();
	return FALSE;
    }

    // with that out of the way, let's get down to action

    switch( e->type() ) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
	// input - turn off tool tip mode
	hideTipAndSleep();
	break;
    case QEvent::MouseMove:
	{
	    QMouseEvent * m = (QMouseEvent *)e;
	    QPoint mousePos = w->mapFromGlobal( m->globalPos() );

	    if ( currentTip && !currentTip->rect.contains( mousePos ) ) {
		hideTip();
		if ( m->state() == 0 )
		    return FALSE;
	    }

	    wakeUp.stop();
	    if ( m->state() == 0 &&
		    mousePos.x() >= 0 && mousePos.x() < w->width() &&
		    mousePos.y() >= 0 && mousePos.y() < w->height() ) {
		if ( label && label->isVisible() ) {
		    return FALSE;
		} else {
		    if ( fallAsleep.isActive() ) {
			wakeUp.start( 1, TRUE );
		    } else {
			previousTip = 0;
			wakeUp.start( wakeUpDelay, TRUE );
		    }
		    if ( t->group && t->group->ena &&
			 !t->group->del && !t->groupText.isEmpty() ) {
			removeTimer->stop();
			emit t->group->showTip( t->groupText );
			currentTip = t;
		    }
		}
		widget = w;
		pos = mousePos;
		return FALSE;
	    } else {
		hideTip();
	    }
	}
	break;
    case QEvent::Leave:
    case QEvent::Hide:
    case QEvent::Destroy:
	if ( w == widget )
	    hideTip();
	break;
    default:
	break;
    }
    return FALSE;
}



void QTipManager::showTip()
{
    if ( !widget || !globally_enabled
#ifndef Q_WS_X11
	 || !widget->isActiveWindow()
#endif
	)
	return;

    QTipManager::Tip *t = (*tips)[ widget ];
    while ( t && !t->rect.contains( pos ) )
	t = t->next;
    if ( t == 0 )
	return;

    if ( t == currentTip && label && label->isVisible() )
	return; // nothing to do

    if ( t->tip ) {
	t->tip->maybeTip( pos );
	return;
    }

    if ( t->group && !t->group->ena )
	return;

    int scr;
    if ( QApplication::desktop()->isVirtualDesktop() )
	scr = QApplication::desktop()->screenNumber( widget->mapToGlobal( pos ) );
    else
	scr = QApplication::desktop()->screenNumber( widget );

    if ( label
#if defined(Q_WS_X11)
	 && label->x11Screen() == widget->x11Screen()
#endif
	 ) {
	// the next two lines are a workaround for QLabel being too intelligent.
	// QLabel turns on the wordbreak flag once it gets a richtext. The two lines below
	// ensure we get correct textflags when switching back and forth between a richtext and
	// non richtext tooltip
	label->setText( "" );
	label->setAlignment( AlignAuto | AlignTop );
	label->setText( t->text );
	label->adjustSize();
	if ( t->geometry != QRect( -1, -1, -1, -1 ) )
	    label->resize( t->geometry.size() );
    } else {
	delete label;
	label = new QTipLabel( QApplication::desktop()->screen( scr ), t->text);
	if ( t->geometry != QRect( -1, -1, -1, -1 ) )
	    label->resize( t->geometry.size() );
	Q_CHECK_PTR( label );
	connect( label, SIGNAL(destroyed()), SLOT(labelDestroyed()) );
    }
    // the above deletion and creation of a QTipLabel causes events to be sent. We had reports that the widget
    // pointer was 0 after this. This is in principle possible if the wrong kind of events get sent through our event
    // filter in this time. So better be safe and check widget once again here.
    if (!widget)
	return;

#ifdef Q_WS_MAC
    QRect screen = QApplication::desktop()->availableGeometry( scr );
#else
    QRect screen = QApplication::desktop()->screenGeometry( scr );
#endif
    QPoint p;
    if ( t->geometry == QRect( -1, -1, -1, -1 ) ) {
	p = widget->mapToGlobal( pos ) +
#ifdef Q_WS_WIN
	    QPoint( 2, 24 );
#else
	    QPoint( 2, 16 );
#endif
	if ( p.x() + label->width() > screen.x() + screen.width() )
	    p.rx() -= 4 + label->width();
	if ( p.y() + label->height() > screen.y() + screen.height() )
	    p.ry() -= 24 + label->height();
    } else {
	p = widget->mapToGlobal( t->geometry.topLeft() );
	label->setAlignment( WordBreak | AlignCenter );
	label->setWidth( t->geometry.width() - 4 );
    }
    if ( p.y() < screen.y() )
	p.setY( screen.y() );
    if ( p.x() + label->width() > screen.x() + screen.width() )
	p.setX( screen.x() + screen.width() - label->width() );
    if ( p.x() < screen.x() )
	p.setX( screen.x() );
    if ( p.y() + label->height() > screen.y() + screen.height() )
	p.setY( screen.y() + screen.height() - label->height() );
    if ( label->text().length() ) {
	label->move( p );

#ifndef QT_NO_EFFECTS
	if ( QApplication::isEffectEnabled( UI_AnimateTooltip ) == FALSE ||
	     previousTip || preventAnimation )
	    label->show();
	else if ( QApplication::isEffectEnabled( UI_FadeTooltip ) )
	    qFadeEffect( label );
	else
	    qScrollEffect( label );
#else
	label->show();
#endif

	label->raise();
	fallAsleep.start( 10000, TRUE );
    }

    if ( t->group && t->group->del && !t->groupText.isEmpty() ) {
	removeTimer->stop();
	emit t->group->showTip( t->groupText );
    }

    currentTip = t;
    previousTip = 0;
}


void QTipManager::hideTip()
{
    QTimer::singleShot( 250, this, SLOT(allowAnimation()) );
    preventAnimation = TRUE;

    if ( label && label->isVisible() ) {
	label->hide();
	fallAsleep.start( 2000, TRUE );
	wakeUp.stop();
	if ( currentTip && currentTip->group )
	    removeTimer->start( 100, TRUE );
    } else if ( wakeUp.isActive() ) {
	wakeUp.stop();
	if ( currentTip && currentTip->group &&
	     !currentTip->group->del && !currentTip->groupText.isEmpty() )
	    removeTimer->start( 100, TRUE );
    } else if ( currentTip && currentTip->group ) {
	removeTimer->start( 100, TRUE );
    }

    previousTip = currentTip;
    currentTip = 0;
    if ( previousTip && previousTip->autoDelete )
	remove( widget, previousTip->rect );
    widget = 0;
}

void  QTipManager::hideTipAndSleep()
{
    hideTip();
    fallAsleep.stop();
}


void QTipManager::allowAnimation()
{
    preventAnimation = FALSE;
}

QString QTipManager::find( QWidget *w, const QPoint& pos )
{
    Tip *t = (*tips)[ w ];
    while ( t && !t->rect.contains( pos ) )
	t = t->next;

    return t ? t->text : QString::null;
}

void QTipManager::setWakeUpDelay ( int i )
{
    wakeUpDelay = i;
}

/*!
    \class QToolTip qtooltip.h
    \brief The QToolTip class provides tool tips (balloon help) for
    any widget or rectangular part of a widget.

    \ingroup helpsystem
    \mainclass

    The tip is a short, single line of text reminding the user of the
    widget's or rectangle's function. It is drawn immediately below
    the region in a distinctive black-on-yellow combination.

    The tip can be any Rich-Text formatted string.

    QToolTipGroup provides a way for tool tips to display another text
    elsewhere (most often in a \link QStatusBar status bar\endlink).

    At any point in time, QToolTip is either dormant or active. In
    dormant mode the tips are not shown and in active mode they are.
    The mode is global, not particular to any one widget.

    QToolTip switches from dormant to active mode when the user hovers
    the mouse on a tip-equipped region for a second or so and remains
    active until the user either clicks a mouse button, presses a key,
    lets the mouse hover for five seconds or moves the mouse outside
    \e all tip-equipped regions for at least a second.

    The QToolTip class can be used in three different ways:
    \list 1
    \i Adding a tip to an entire widget.
    \i Adding a tip to a fixed rectangle within a widget.
    \i Adding a tip to a dynamic rectangle within a widget.
    \endlist

    To add a tip to a widget, call the \e static function
    QToolTip::add() with the widget and tip as arguments:

    \code
	QToolTip::add( quitButton, "Leave the application" );
    \endcode

    This is the simplest and most common use of QToolTip. The tip
    will be deleted automatically when \e quitButton is deleted, but
    you can remove it yourself, too:

    \code
	QToolTip::remove( quitButton );
    \endcode

    You can also display another text (typically in a \link QStatusBar
    status bar),\endlink courtesy of \l{QToolTipGroup}. This example
    assumes that \e grp is a \c{QToolTipGroup *} and is already
    connected to the appropriate status bar:

    \code
	QToolTip::add( quitButton, "Leave the application", grp,
		       "Leave the application, prompting to save if necessary" );
	QToolTip::add( closeButton, "Close this window", grp,
		       "Close this window, prompting to save if necessary" );
    \endcode

    To add a tip to a fixed rectangle within a widget, call the static
    function QToolTip::add() with the widget, rectangle and tip as
    arguments. (See the \c tooltip/tooltip.cpp example.) Again, you
    can supply a \c{QToolTipGroup *} and another text if you want.

    Both of these are one-liners and cover the majority of cases. The
    third and most general way to use QToolTip requires you to
    reimplement a pure virtual function to decide whether to pop up a
    tool tip. The \c tooltip/tooltip.cpp example demonstrates this
    too. This mode can be used to implement tips for text that can
    move as the user scrolls, for example.

    To use QToolTip like this, you must subclass QToolTip and
    reimplement maybeTip(). QToolTip calls maybeTip() when a tip
    should pop up, and maybeTip() decides whether to show a tip.

    Tool tips can be globally disabled using
    QToolTip::setGloballyEnabled() or disabled in groups with
    QToolTipGroup::setEnabled().

    You can retrieve the text of a tooltip for a given position within
    a widget using textFor().

    The global tooltip font and palette can be set with the static
    setFont() and setPalette() functions respectively.

    \sa QStatusBar QWhatsThis QToolTipGroup
    \link guibooks.html#fowler GUI Design Handbook: Tool Tip\endlink
*/


/*!
    Returns the font common to all tool tips.

    \sa setFont()
*/

QFont QToolTip::font()
{
    QTipLabel l(0,"");
    return QApplication::font( &l );
}


/*!
    Sets the font for all tool tips to \a font.

    \sa font()
*/

void QToolTip::setFont( const QFont &font )
{
    QApplication::setFont( font, TRUE, "QTipLabel" );
}


/*!
    Returns the palette common to all tool tips.

    \sa setPalette()
*/

QPalette QToolTip::palette()
{
    QTipLabel l(0,"");
    return QApplication::palette( &l );
}


/*!
    Sets the palette for all tool tips to \a palette.

    \sa palette()
*/

void QToolTip::setPalette( const QPalette &palette )
{
    QApplication::setPalette( palette, TRUE, "QTipLabel" );
}

/*!
    Constructs a tool tip object. This is only necessary if you need
    tool tips on regions that can move within the widget (most often
    because the widget's contents can scroll).

    \a widget is the widget you want to add dynamic tool tips to and
    \a group (optional) is the tool tip group they should belong to.

    \warning QToolTip is not a subclass of QObject, so the instance of
    QToolTip is not deleted when \a widget is deleted.

    \warning If you delete the tool tip before you have deleted
    \a widget then you need to make sure you call remove() yourself from
    \a widget in your reimplemented QToolTip destructor.

    \code
	MyToolTip::~MyToolTip()
	{
	    remove( widget );
	}
    \endcode

    \sa maybeTip().
*/

QToolTip::QToolTip( QWidget * widget, QToolTipGroup * group )
{
    p = widget;
    g = group;
    initTipManager();
    tipManager->add( p, entireWidget(),
		     QString::null, g, QString::null, this, FALSE );
}


/*!
    Adds a tool tip to \a widget. \a text is the text to be shown in
    the tool tip.

    This is the most common entry point to the QToolTip class; it is
    suitable for adding tool tips to buttons, checkboxes, comboboxes
    and so on.
*/

void QToolTip::add( QWidget *widget, const QString &text )
{
    initTipManager();
    tipManager->add( widget, entireWidget(),
		     text, 0, QString::null, 0, FALSE );
}


/*!
    \overload

    Adds a tool tip to \a widget and to tool tip group \a group.

    \a text is the text shown in the tool tip and \a longText is the
    text emitted from \a group.

    Normally, \a longText is shown in a \link QStatusBar status
    bar\endlink or similar.
*/

void QToolTip::add( QWidget *widget, const QString &text,
		    QToolTipGroup *group, const QString& longText )
{
    initTipManager();
    tipManager->add( widget, entireWidget(), text, group, longText, 0, FALSE );
}


/*!
    Removes the tool tip from \a widget.

    If there is more than one tool tip on \a widget, only the one
    covering the entire widget is removed.
*/

void QToolTip::remove( QWidget * widget )
{
    if ( tipManager )
	tipManager->remove( widget, entireWidget() );
}

/*!
    \overload

    Adds a tool tip to a fixed rectangle, \a rect, within \a widget.
    \a text is the text shown in the tool tip.
*/

void QToolTip::add( QWidget * widget, const QRect & rect, const QString &text )
{
    initTipManager();
    tipManager->add( widget, rect, text, 0, QString::null, 0, FALSE );
}


/*!
    \overload

    Adds a tool tip to an entire \a widget and to tool tip group \a
    group. The tooltip will disappear when the mouse leaves the \a
    rect.

    \a text is the text shown in the tool tip and \a groupText is the
    text emitted from \a group.

    Normally, \a groupText is shown in a \link QStatusBar status
    bar\endlink or similar.
*/

void QToolTip::add( QWidget *widget, const QRect &rect,
		    const QString& text,
		    QToolTipGroup *group, const QString& groupText )
{
    initTipManager();
    tipManager->add( widget, rect, text, group, groupText, 0, FALSE );
}


/*!
    \overload

    Removes any tool tip for \a rect from \a widget.

    If there is more than one tool tip on \a widget, only the one
    covering rectangle \a rect is removed.
*/

void QToolTip::remove( QWidget * widget, const QRect & rect )
{
    if ( tipManager )
	tipManager->remove( widget, rect );
}

/*!
    Returns the tool tip text for \a widget at position \a pos, or
    QString::null if there is no tool tip for the given widget and
    position.
*/

QString QToolTip::textFor( QWidget *widget, const QPoint& pos )
{
    if ( tipManager )
	return tipManager->find( widget, pos );
    return QString::null;
}

/*!
    Hides any tip that is currently being shown.

    Normally, there is no need to call this function; QToolTip takes
    care of showing and hiding the tips as the user moves the mouse.
*/

void QToolTip::hide()
{
    if ( tipManager )
	tipManager->hideTipAndSleep();
}

/*!
    \fn virtual void QToolTip::maybeTip( const QPoint & p);

    This pure virtual function is half of the most versatile interface
    QToolTip offers.

    It is called when there is a possibility that a tool tip should be
    shown and must decide whether there is a tool tip for the point \a
    p in the widget that this QToolTip object relates to. If so,
    maybeTip() must call tip() with the rectangle the tip applies to,
    the tip's text and optionally the QToolTipGroup details and the
    geometry in screen coordinates.

    \a p is given in that widget's local coordinates. Most maybeTip()
    implementations will be of the form:

    \code
	if ( <something> ) {
	    tip( <something>, <something> );
	}
    \endcode

    The first argument to tip() (a rectangle) must encompass \a p,
    i.e. the tip must apply to the current mouse position; otherwise
    QToolTip's operation is undefined.

    Note that the tip will disappear once the mouse moves outside the
    rectangle you give to tip(), and will not reappear if the mouse
    moves back in: maybeTip() is called again instead.

    \sa tip()
*/


/*!
    Immediately pops up a tip saying \a text and removes the tip once
    the cursor moves out of rectangle \a rect (which is given in the
    coordinate system of the widget this QToolTip relates to).

    The tip will not reappear if the cursor moves back; your
    maybeTip() must reinstate it each time.
*/

void QToolTip::tip( const QRect & rect, const QString &text )
{
    initTipManager();
    tipManager->add( parentWidget(), rect, text, 0, QString::null, 0, TRUE );
}

/*!
    \overload

    Immediately pops up a tip saying \a text and removes that tip once
    the cursor moves out of rectangle \a rect (which is given in the
    coordinate system of the widget this QToolTip relates to). \a
    groupText is the text emitted from the group.

    The tip will not reappear if the cursor moves back; your
    maybeTip() must reinstate it each time.
*/

void QToolTip::tip( const QRect & rect, const QString &text,
		    const QString& groupText )
{
    initTipManager();
    tipManager->add( parentWidget(), rect, text, group(), groupText, 0, TRUE );
}

/*!
    \overload

    Immediately pops up a tip within the rectangle \a geometry, saying
    \a text and removes the tip once the cursor moves out of rectangle
    \a rect. Both rectangles are given in the coordinate system of the
    widget this QToolTip relates to.

    The tip will not reappear if the cursor moves back; your
    maybeTip() must reinstate it each time.

    If the tip does not fit inside \a geometry, the tip expands.
*/

void QToolTip::tip( const QRect &rect, const QString &text, const QRect &geometry )
{
    initTipManager();
    tipManager->add( geometry, parentWidget(), rect, text, 0, QString::null, 0, TRUE );
}

/*!
    \overload

    Immediately pops up a tip within the rectangle \a geometry, saying
    \a text and removes the tip once the cursor moves out of rectangle
    \a rect. \a groupText is the text emitted from the group. Both
    rectangles are given in the coordinate system of the widget this
    QToolTip relates to.

    The tip will not reappear if the cursor moves back; your
    maybeTip() must reinstate it each time.

    If the tip does not fit inside \a geometry, the tip expands.
*/

void QToolTip::tip( const QRect &rect, const QString &text, const QString& groupText, const QRect &geometry )
{
    initTipManager();
    tipManager->add( geometry, parentWidget(), rect, text, group(), groupText, 0, TRUE );
}



/*!
    Immediately removes all tool tips for this tooltip's parent
    widget.
*/

void QToolTip::clear()
{
    if ( tipManager )
	tipManager->remove( parentWidget() );
}


/*!
    \fn QWidget * QToolTip::parentWidget() const

    Returns the widget this QToolTip applies to.

    The tool tip is destroyed automatically when the parent widget is
    destroyed.

    \sa group()
*/


/*!
    \fn QToolTipGroup * QToolTip::group() const

    Returns the tool tip group this QToolTip is a member of or 0 if it
    isn't a member of any group.

    The tool tip group is the object responsible for maintaining
    contact between tool tips and a \link QStatusBar status
    bar\endlink or something else which can show the longer help text.

    \sa parentWidget(), QToolTipGroup
*/


/*!
    \class QToolTipGroup qtooltip.h
    \brief The QToolTipGroup class collects tool tips into related groups.

    \ingroup helpsystem

    Tool tips can display \e two texts: one in the tip and
    (optionally) one that is typically in a \link QStatusBar status
    bar\endlink. QToolTipGroup provides a way to link tool tips to
    this status bar.

    QToolTipGroup has practically no API; it is only used as an
    argument to QToolTip's member functions, for example like this:

    \code
	QToolTipGroup * grp = new QToolTipGroup( this, "tool tip relay" );
	connect( grp, SIGNAL(showTip(const QString&)),
		 myLabel, SLOT(setText(const QString&)) );
	connect( grp, SIGNAL(removeTip()),
		 myLabel, SLOT(clear()) );
	QToolTip::add( giraffeButton, "feed giraffe",
		       grp, "Give the giraffe a meal" );
	QToolTip::add( gorillaButton, "feed gorilla",
		       grp, "Give the gorilla a meal" );
    \endcode

    This example makes the object myLabel (which you must supply)
    display (one assumes, though you can make myLabel do anything, of
    course) the strings "Give the giraffe a meal" and "Give the
    gorilla a meal" while the relevant tool tips are being displayed.

    Deleting a tool tip group removes the tool tips in it.
*/

/*!
    \fn void QToolTipGroup::showTip (const QString &longText)

    This signal is emitted when one of the tool tips in the group is
    displayed. \a longText is the extra text for the displayed tool
    tip.

    \sa removeTip()
*/

/*!
    \fn void QToolTipGroup::removeTip ()

    This signal is emitted when a tool tip in this group is hidden.
    See the QToolTipGroup documentation for an example of use.

    \sa showTip()
*/


/*!
    Constructs a tool tip group called \a name, with parent \a parent.
*/

QToolTipGroup::QToolTipGroup( QObject *parent, const char *name )
    : QObject( parent, name )
{
    del = TRUE;
    ena = TRUE;
}


/*!
    Destroys this tool tip group and all tool tips in it.
*/

QToolTipGroup::~QToolTipGroup()
{
    if ( tipManager )
	tipManager->removeFromGroup( this );
}


/*!
    \property QToolTipGroup::delay
    \brief whether the display of the group text is delayed.

    If set to TRUE (the default), the group text is displayed at the
    same time as the tool tip. Otherwise, the group text is displayed
    immediately when the cursor enters the widget.
*/

bool QToolTipGroup::delay() const
{
    return del;
}

void QToolTipGroup::setDelay( bool enable )
{
#if 0
    if ( enable && !del ) {
	// maybe we should show the text at once?
    }
#endif
    del = enable;
}

/*!
    \fn static void QToolTip::setEnabled( bool enable )

    \obsolete
*/
/*!
    \fn static bool QToolTip::enabled()

    \obsolete
*/
/*!
    \property QToolTipGroup::enabled
    \brief whether tool tips in the group are enabled.

    This property's default is TRUE.
*/

void QToolTipGroup::setEnabled( bool enable )
{
    ena = enable;
}

bool QToolTipGroup::enabled() const
{
    return (bool)ena;
}

/*!
    If \a enable is TRUE sets all tool tips to be enabled (shown when
    needed); if \a enable is FALSE sets all tool tips to be disabled
    (never shown).

    By default, tool tips are enabled. Note that this function affects
    all tool tips in the entire application.

    \sa QToolTipGroup::setEnabled()
*/

void QToolTip::setGloballyEnabled( bool enable )
{
    globally_enabled = enable;
}

/*!
    Returns whether tool tips are enabled globally.

    \sa setGloballyEnabled()
*/
bool QToolTip::isGloballyEnabled()
{
    return globally_enabled;
}

/*!
  Sets the wakeup delay for all tooltips to \a i
  milliseconds.
*/
void QToolTip::setWakeUpDelay ( int i )
{
    initTipManager();
    tipManager->setWakeUpDelay(i);
}


#include "qtooltip.moc"
#endif
