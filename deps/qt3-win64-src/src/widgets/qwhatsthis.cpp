/****************************************************************************
** $Id: qwhatsthis.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QWhatsThis class
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

#include "qwhatsthis.h"
#ifndef QT_NO_WHATSTHIS
#include "qapplication.h"
#include "qpaintdevicemetrics.h"
#include "qpixmap.h"
#include "qpainter.h"
#include "qtimer.h"
#include "qptrdict.h"
#include "qtoolbutton.h"
#include "qshared.h"
#include "qcursor.h"
#include "qbitmap.h"
#include "qtooltip.h"
#include "qsimplerichtext.h"
#include "qstylesheet.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif
#if defined(Q_WS_WIN)
#include "qt_windows.h"
#ifndef SPI_GETDROPSHADOW
#define SPI_GETDROPSHADOW                   0x1024
#endif
#endif

/*!
    \class QWhatsThis qwhatsthis.h
    \brief The QWhatsThis class provides a simple description of any
    widget, i.e. answering the question "What's this?".

    \ingroup helpsystem
    \mainclass

    "What's this?" help is part of an application's online help system
    that provides users with information about functionality, usage,
    background etc., in various levels of detail from short tool tips
    to full text browsing help windows.

    QWhatsThis provides a single window with an explanatory text that
    pops up when the user asks "What's this?". The default way to do
    this is to focus the relevant widget and press Shift+F1. The help
    text appears immediately; it goes away as soon as the user does
    something else.

    (Note that if there is an accelerator for Shift+F1, this mechanism
    will not work.)

    To add "What's this?" text to a widget you simply call
    QWhatsThis::add() for the widget. For example, to assign text to a
    menu item, call QMenuData::setWhatsThis(); for a global
    accelerator key, call QAccel::setWhatsThis() and If you're using
    actions, use QAction::setWhatsThis().

    The text can be either rich text or plain text. If you specify a
    rich text formatted string, it will be rendered using the default
    stylesheet. This makes it possible to embed images. See
    QStyleSheet::defaultSheet() for details.

    \quotefile action/application.cpp
    \skipto fileOpenText
    \printuntil setWhatsThis

    An alternative way to enter "What's this?" mode is to use the
    ready-made tool bar tool button from
    QWhatsThis::whatsThisButton(). By invoking this context help
    button (in the picture below the first one from the right) the
    user switches into "What's this?" mode. If they now click on a
    widget the appropriate help text is shown. The mode is left when
    help is given or when the user presses Esc.

    \img whatsthis.png

    If you are using QMainWindow you can also use the
    QMainWindow::whatsThis() slot to invoke the mode from a menu item.

    For more control you can create a dedicated QWhatsThis object for
    a special widget. By subclassing and reimplementing
    QWhatsThis::text() it is possible to have different help texts,
    depending on the position of the mouse click. By reimplementing
    QWhatsThis::clicked() it is possible to have hyperlinks inside the
    help texts.

    If you wish to control the "What's this?" behavior of a widget
    manually see QWidget::customWhatsThis().

    The What's This object can be removed using QWhatsThis::remove(),
    although this is rarely necessary because it is automatically
    removed when the widget is destroyed.

    \sa QToolTip
*/

// a special button
class QWhatsThisButton: public QToolButton
{
    Q_OBJECT

public:
    QWhatsThisButton( QWidget * parent, const char * name );
    ~QWhatsThisButton();

public slots:
    void mouseReleased();

};


class QWhatsThat : public QWidget
{
    Q_OBJECT
public:
    QWhatsThat( QWidget* w, const QString& txt, QWidget* parent, const char* name );
    ~QWhatsThat() ;

public slots:
    void hide();
    inline void widgetDestroyed() { widget = 0; }

protected:
    void mousePressEvent( QMouseEvent* );
    void mouseReleaseEvent( QMouseEvent* );
    void mouseMoveEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent* );
    void paintEvent( QPaintEvent* );

private:
    QString text;
#ifndef QT_NO_RICHTEXT
    QSimpleRichText* doc;
#endif
    QString anchor;
    bool pressed;
    QWidget* widget;
};


class QWhatsThisPrivate: public QObject
{
    Q_OBJECT
public:

    // an item for storing texts
    struct WhatsThisItem: public QShared
    {
	WhatsThisItem(): QShared() { whatsthis = 0; }
	~WhatsThisItem();
	QString s;
	QWhatsThis* whatsthis;
    };

    // the (these days pretty small) state machine
    enum State { Inactive, Waiting };

    QWhatsThisPrivate();
    ~QWhatsThisPrivate();

    bool eventFilter( QObject *, QEvent * );

    WhatsThisItem* newItem( QWidget * widget );
    void add( QWidget * widget, QWhatsThis* special );
    void add( QWidget * widget, const QString& text );

    // say it.
    void say( QWidget *, const QString&, const QPoint&  );

    // setup and teardown
    static void setUpWhatsThis();

    void enterWhatsThisMode();
    void leaveWhatsThisMode();

    // variables
    QWhatsThat * whatsThat;
    QPtrDict<WhatsThisItem> * dict;
    QPtrDict<QWidget> * tlw;
    QPtrDict<QWhatsThisButton> * buttons;
    State state;

private slots:
    void cleanupWidget()
    {
	const QObject* o = sender();
	if ( o->isWidgetType() ) // sanity
	    QWhatsThis::remove((QWidget*)o);
    }

};

// static, but static the less-typing way
static QWhatsThisPrivate * wt = 0;

// shadowWidth not const, for XP drop-shadow-fu turns it to 0
static int shadowWidth = 6;   // also used as '5' and '6' and even '8' below
const int vMargin = 8;
const int hMargin = 12;

// Lets QPopupMenu destroy the QWhatsThat.
void qWhatsThisBDH()
{
    if ( wt && wt->whatsThat )
	wt->whatsThat->hide();
}


QWhatsThat::QWhatsThat( QWidget* w, const QString& txt, QWidget* parent, const char* name )
    : QWidget( parent, name, WType_Popup ), text( txt ), pressed( FALSE ), widget( w )
{

    setBackgroundMode( NoBackground );
    setPalette( QToolTip::palette() );
    setMouseTracking( TRUE );
#ifndef QT_NO_CURSOR
    setCursor( arrowCursor );
#endif

    if ( widget )
	connect( widget, SIGNAL( destroyed() ), this, SLOT( widgetDestroyed() ) );


    QRect r;
#ifndef QT_NO_RICHTEXT
    doc = 0;
    if ( QStyleSheet::mightBeRichText( text ) ) {
	QFont f = QApplication::font( this );
	doc = new QSimpleRichText( text, f );
	doc->adjustSize();
	r.setRect( 0, 0, doc->width(), doc->height() );
    }
    else
#endif
    {
	int sw = QApplication::desktop()->width() / 3;
	if ( sw < 200 )
	    sw = 200;
	else if ( sw > 300 )
	    sw = 300;

	r = fontMetrics().boundingRect( 0, 0, sw, 1000,
					AlignAuto + AlignTop + WordBreak + ExpandTabs,
					text );
    }
#if defined(Q_WS_WIN)
    if ( (qWinVersion()&WV_NT_based) > WV_2000 ) {
	BOOL shadow;
	SystemParametersInfo( SPI_GETDROPSHADOW, 0, &shadow, 0 );
	shadowWidth = shadow ? 0 : 6;
    }
#endif
    resize( r.width() + 2*hMargin + shadowWidth, r.height() + 2*vMargin + shadowWidth );
}

QWhatsThat::~QWhatsThat()
{
    if ( wt && wt->whatsThat == this )
	wt->whatsThat = 0;
#ifndef QT_NO_RICHTEXT
    if ( doc )
	delete doc;
#endif
}

void QWhatsThat::hide()
{
    QWidget::hide();
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( this, 0, QAccessible::ContextHelpEnd );
#endif
}

void QWhatsThat::mousePressEvent( QMouseEvent* e )
{
    pressed = TRUE;
    if ( e->button() == LeftButton && rect().contains( e->pos() ) ) {
#ifndef QT_NO_RICHTEXT
	if ( doc )
	    anchor = doc->anchorAt( e->pos() -  QPoint( hMargin, vMargin) );
#endif
	return;
    }
    hide();
}

void QWhatsThat::mouseReleaseEvent( QMouseEvent* e )
{
    if ( !pressed )
	return;
#ifndef QT_NO_RICHTEXT
    if ( e->button() == LeftButton && doc && rect().contains( e->pos() ) ) {
	QString a = doc->anchorAt( e->pos() -  QPoint( hMargin, vMargin ) );
	QString href;
	if ( anchor == a )
	    href = a;
	anchor = QString::null;
	if ( widget && wt && wt->dict ) {
	    QWhatsThisPrivate::WhatsThisItem * i = wt->dict->find( widget );
	    if ( i  && i->whatsthis && !i->whatsthis->clicked( href ) )
		return;
	}
    }
#endif
    hide();
}

void QWhatsThat::mouseMoveEvent( QMouseEvent* e)
{
#ifndef QT_NO_RICHTEXT
#ifndef QT_NO_CURSOR
    if ( !doc )
	return;
    QString a = doc->anchorAt( e->pos() -  QPoint( hMargin, vMargin ) );
    if ( !a.isEmpty() )
	setCursor( pointingHandCursor );
    else
	setCursor( arrowCursor );
#endif
#endif
}


void QWhatsThat::keyPressEvent( QKeyEvent* )
{
    hide();
}



void QWhatsThat::paintEvent( QPaintEvent* )
{
    bool drawShadow = TRUE;
#if defined(Q_WS_WIN)
    if ( (qWinVersion()&WV_NT_based) > WV_2000 ) {
	BOOL shadow;
	SystemParametersInfo( SPI_GETDROPSHADOW, 0, &shadow, 0 );
	drawShadow = !shadow;
    }
#elif defined(Q_WS_MACX)
    drawShadow = FALSE; //never draw it on OS X we get it for free
#endif

    QRect r = rect();
    if ( drawShadow )
	r.addCoords( 0, 0, -shadowWidth, -shadowWidth );
    QPainter p( this);
    p.setPen( colorGroup().foreground() );
    p.drawRect( r );
    p.setPen( colorGroup().mid() );
    p.setBrush( colorGroup().brush( QColorGroup::Background ) );
    int w = r.width();
    int h = r.height();
    p.drawRect( 1, 1, w-2, h-2 );
    if ( drawShadow ) {
	p.setPen( colorGroup().shadow() );
	p.drawPoint( w + 5, 6 );
	p.drawLine( w + 3, 6, w + 5, 8 );
	p.drawLine( w + 1, 6, w + 5, 10 );
	int i;
	for( i=7; i < h; i += 2 )
	    p.drawLine( w, i, w + 5, i + 5 );
	for( i = w - i + h; i > 6; i -= 2 )
	    p.drawLine( i, h, i + 5, h + 5 );
	for( ; i > 0 ; i -= 2 )
	    p.drawLine( 6, h + 6 - i, i + 5, h + 5 );
    }
    p.setPen( colorGroup().foreground() );
    r.addCoords( hMargin, vMargin, -hMargin, -vMargin );

#ifndef QT_NO_RICHTEXT
    if ( doc ) {
	doc->draw( &p, r.x(), r.y(), r, colorGroup(), 0 );
    }
    else
#endif
    {
	p.drawText( r, AlignAuto + AlignTop + WordBreak + ExpandTabs, text );
    }
}

// the item
QWhatsThisPrivate::WhatsThisItem::~WhatsThisItem()
{
    if ( count )
	qFatal( "QWhatsThis: Internal error (%d)", count );
    delete whatsthis;
}


static const char * const button_image[] = {
"16 16 3 1",
" 	c None",
"o	c #000000",
"a	c #000080",
"o        aaaaa  ",
"oo      aaa aaa ",
"ooo    aaa   aaa",
"oooo   aa     aa",
"ooooo  aa     aa",
"oooooo  a    aaa",
"ooooooo     aaa ",
"oooooooo   aaa  ",
"ooooooooo aaa   ",
"ooooo     aaa   ",
"oo ooo          ",
"o  ooo    aaa   ",
"    ooo   aaa   ",
"    ooo         ",
"     ooo        ",
"     ooo        "};

// the button class
QWhatsThisButton::QWhatsThisButton( QWidget * parent, const char * name )
    : QToolButton( parent, name )
{
    QPixmap p( (const char**)button_image );
    setPixmap( p );
    setToggleButton( TRUE );
    setAutoRaise( TRUE );
    setFocusPolicy( NoFocus );
    setTextLabel( tr( "What's this?" ) );
    wt->buttons->insert( (void *)this, this );
    connect( this, SIGNAL( released() ),
	     this, SLOT( mouseReleased() ) );
}


QWhatsThisButton::~QWhatsThisButton()
{
    if ( wt && wt->buttons )
	wt->buttons->take( (void *)this );
}


void QWhatsThisButton::mouseReleased()
{
    if ( wt->state == QWhatsThisPrivate::Inactive && isOn() ) {
	QWhatsThisPrivate::setUpWhatsThis();
#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor( whatsThisCursor, FALSE );
#endif
	wt->state = QWhatsThisPrivate::Waiting;
	qApp->installEventFilter( wt );
    }
}

static void qWhatsThisPrivateCleanup()
{
    if( wt ) {
	delete wt;
	wt = 0;
    }
}

// the what's this manager class
QWhatsThisPrivate::QWhatsThisPrivate()
    : QObject( 0, "global what's this object" )
{
    whatsThat = 0;
    dict = new QPtrDict<QWhatsThisPrivate::WhatsThisItem>;
    tlw = new QPtrDict<QWidget>;
    wt = this;
    buttons = new QPtrDict<QWhatsThisButton>;
    state = Inactive;
}

QWhatsThisPrivate::~QWhatsThisPrivate()
{
#ifndef QT_NO_CURSOR
    if ( state == Waiting && qApp )
	QApplication::restoreOverrideCursor();
#endif
    // the two straight-and-simple dicts
    delete tlw;
    delete buttons;

    // then delete the complex one.
    QPtrDictIterator<WhatsThisItem> it( *dict );
    WhatsThisItem * i;
    QWidget * w;
    while( (i=it.current()) != 0 ) {
	w = (QWidget *)it.currentKey();
	++it;
	dict->take( w );
	if ( i->deref() )
	    delete i;
    }
    delete dict;
    if ( whatsThat && !whatsThat->parentWidget() ) {
	delete whatsThat;
    }
    // and finally lose wt
    wt = 0;
}

bool QWhatsThisPrivate::eventFilter( QObject * o, QEvent * e )
{
    switch( state ) {
    case Waiting:
	if ( e->type() == QEvent::MouseButtonPress && o->isWidgetType() ) {
	    QWidget * w = (QWidget *) o;
	    if ( ( (QMouseEvent*)e)->button() == RightButton )
		return FALSE; // ignore RMB
	    if ( w->customWhatsThis() )
		return FALSE;
	    QWhatsThisPrivate::WhatsThisItem * i = 0;
	    QMouseEvent* me = (QMouseEvent*) e;
	    QPoint p = me->pos();
	    while( w && !i ) {
		i = dict->find( w );
		if ( !i ) {
		    p += w->pos();
		    w = w->parentWidget( TRUE );
		}
	    }
	    leaveWhatsThisMode();
	    if (!i ) {
#if defined(QT_ACCESSIBILITY_SUPPORT)
		QAccessible::updateAccessibility( this, 0, QAccessible::ContextHelpEnd );
#endif
		return TRUE;
	    }
	    if ( i->whatsthis )
		say( w, i->whatsthis->text( p ), me->globalPos() );
	    else
		say( w, i->s, me->globalPos() );
	    return TRUE;
	} else if ( e->type() == QEvent::MouseButtonRelease ) {
	    if ( ( (QMouseEvent*)e)->button() == RightButton )
		return FALSE; // ignore RMB
	    return !o->isWidgetType() || !((QWidget*)o)->customWhatsThis();
	} else if ( e->type() == QEvent::MouseMove ) {
	    return !o->isWidgetType() || !((QWidget*)o)->customWhatsThis();
	} else if ( e->type() == QEvent::KeyPress ) {
	    QKeyEvent* kev = (QKeyEvent*)e;

	    if ( kev->key() == Qt::Key_Escape ) {
		leaveWhatsThisMode();
		return TRUE;
	    } else if ( o->isWidgetType() && ((QWidget*)o)->customWhatsThis() ) {
		return FALSE;
	    } else if ( kev->key() == Key_Menu ||
			( kev->key() == Key_F10 &&
			  kev->state() == ShiftButton ) ) {
		// we don't react to these keys, they are used for context menus
		return FALSE;
	    } else if ( kev->state() == kev->stateAfter() &&
			kev->key() != Key_Meta ) {  // not a modifier key
		leaveWhatsThisMode();
	    }
	} else if ( e->type() == QEvent::MouseButtonDblClick ) {
	    return TRUE;
	}
	break;
    case Inactive:
 	if ( e->type() == QEvent::Accel &&
	     ((QKeyEvent *)e)->key() == Key_F1 &&
 	     o->isWidgetType() &&
	     ((QKeyEvent *)e)->state() == ShiftButton ) {
 	    QWidget * w = ((QWidget *)o)->focusWidget();
	    if ( !w )
		break;
	    QString s = QWhatsThis::textFor( w, QPoint(0,0), TRUE );
 	    if ( !s.isNull() ) {
		say ( w, s, w->mapToGlobal( w->rect().center() ) );
		((QKeyEvent *)e)->accept();
		return TRUE;
 	    }
 	}
	break;
    }
    return FALSE;
}



void QWhatsThisPrivate::setUpWhatsThis()
{
    if ( !wt ) {
	wt = new QWhatsThisPrivate();

	// It is necessary to use a post routine, because
	// the destructor deletes pixmaps and other stuff that
	// needs a working X connection under X11.
	qAddPostRoutine( qWhatsThisPrivateCleanup );
    }
}


void QWhatsThisPrivate::enterWhatsThisMode()
{
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( this, 0, QAccessible::ContextHelpStart );
#endif
}


void QWhatsThisPrivate::leaveWhatsThisMode()
{
    if ( state == Waiting ) {
	QPtrDictIterator<QWhatsThisButton> it( *(wt->buttons) );
	QWhatsThisButton * b;
	while( (b=it.current()) != 0 ) {
	    ++it;
	    b->setOn( FALSE );
	}
#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif
	state = Inactive;
	qApp->removeEventFilter( this );
    }
}



void QWhatsThisPrivate::say( QWidget * widget, const QString &text, const QPoint& ppos)
{
    if ( text.isEmpty() )
	return;
    // make a fresh widget, and set it up
    delete whatsThat;
    whatsThat = new QWhatsThat(
			       widget, text,
#if defined(Q_WS_X11)
			       QApplication::desktop()->screen( widget ?
								widget->x11Screen() :
								QCursor::x11Screen() ),
#else
			       0,
#endif
			       "automatic what's this? widget" );


    // okay, now to find a suitable location

    int scr = ( widget ?
		QApplication::desktop()->screenNumber( widget ) :
#if defined(Q_WS_X11)
		QCursor::x11Screen()
#else
		QApplication::desktop()->screenNumber( ppos )
#endif // Q_WS_X11
		);
    QRect screen = QApplication::desktop()->screenGeometry( scr );

    int x;
    int w = whatsThat->width();
    int h = whatsThat->height();
    int sx = screen.x();
    int sy = screen.y();

    // first try locating the widget immediately above/below,
    // with nice alignment if possible.
    QPoint pos;
    if ( widget )
	pos = widget->mapToGlobal( QPoint( 0,0 ) );

    if ( widget && w > widget->width() + 16 )
	x = pos.x() + widget->width()/2 - w/2;
    else
	x = ppos.x() - w/2;

	// squeeze it in if that would result in part of what's this
	// being only partially visible
    if ( x + w  + shadowWidth > sx+screen.width() )
	x = (widget? (QMIN(screen.width(),
			   pos.x() + widget->width())
		      ) : screen.width() )
	    - w;

    if ( x < sx )
	x = sx;

    int y;
    if ( widget && h > widget->height() + 16 ) {
	y = pos.y() + widget->height() + 2; // below, two pixels spacing
	// what's this is above or below, wherever there's most space
	if ( y + h + 10 > sy+screen.height() )
	    y = pos.y() + 2 - shadowWidth - h; // above, overlap
    }
    y = ppos.y() + 2;

	// squeeze it in if that would result in part of what's this
	// being only partially visible
    if ( y + h + shadowWidth > sy+screen.height() )
	y = ( widget ? (QMIN(screen.height(),
			     pos.y() + widget->height())
			) : screen.height() )
	    - h;
    if ( y < sy )
	y = sy;

    whatsThat->move( x, y );
    whatsThat->show();
}

QWhatsThisPrivate::WhatsThisItem* QWhatsThisPrivate::newItem( QWidget * widget )
{
    WhatsThisItem * i = dict->find( (void *)widget );
    if ( i )
	QWhatsThis::remove( widget );
    i = new WhatsThisItem;
    dict->insert( (void *)widget, i );
    QWidget * t = widget->topLevelWidget();
    if ( !tlw->find( (void *)t ) ) {
	tlw->insert( (void *)t, t );
	t->installEventFilter( this );
    }
    connect( widget, SIGNAL(destroyed()), this, SLOT(cleanupWidget()) );
    return i;
}

void QWhatsThisPrivate::add( QWidget * widget, QWhatsThis* special )
{
    newItem( widget )->whatsthis = special;
}

void QWhatsThisPrivate::add( QWidget * widget, const QString &text )
{
    newItem( widget )->s = text;
}


// and finally the What's This class itself

/*!
    Adds \a text as "What's this" help for \a widget. If the text is
    rich text formatted (i.e. it contains markup) it will be rendered
    with the default stylesheet QStyleSheet::defaultSheet().

    The text is destroyed if the widget is later destroyed, so it need
    not be explicitly removed.

    \sa remove()
*/
void QWhatsThis::add( QWidget * widget, const QString &text )
{
    if ( text.isEmpty() )
	return; // pointless
    QWhatsThisPrivate::setUpWhatsThis();
    wt->add(widget,text);
}


/*!
    Removes the "What's this?" help associated with the \a widget.
    This happens automatically if the widget is destroyed.

    \sa add()
*/
void QWhatsThis::remove( QWidget * widget )
{
    QWhatsThisPrivate::setUpWhatsThis();
    QWhatsThisPrivate::WhatsThisItem * i = wt->dict->find( (void *)widget );
    if ( !i )
	return;

    wt->dict->take( (void *)widget );

    i->deref();
    if ( !i->count )
	delete i;
}


/*!
    Returns the what's this text for widget \a w or QString::null if
    there is no "What's this?" help for the widget. \a pos contains
    the mouse position; this is useful, for example, if you've
    subclassed to make the text that is displayed position dependent.

    If \a includeParents is TRUE, parent widgets are taken into
    consideration as well when looking for what's this help text.

    \sa add()
*/
QString QWhatsThis::textFor( QWidget * w, const QPoint& pos, bool includeParents )
{
    QWhatsThisPrivate::setUpWhatsThis();
    QWhatsThisPrivate::WhatsThisItem * i = 0;
    QPoint p = pos;
    while( w && !i ) {
	i = wt->dict->find( w );
	if ( !includeParents )
	    break;
	if ( !i ) {
	    p += w->pos();
	    w = w->parentWidget( TRUE );
	}
    }
    if (!i)
	return QString::null;
    if ( i->whatsthis )
	return i->whatsthis->text( p );
    return i->s;
}


/*!
    Creates a QToolButton preconfigured to enter "What's this?" mode
    when clicked. You will often use this with a tool bar as \a
    parent:
    \code
	(void) QWhatsThis::whatsThisButton( my_help_tool_bar );
    \endcode
*/
QToolButton * QWhatsThis::whatsThisButton( QWidget * parent )
{
    QWhatsThisPrivate::setUpWhatsThis();
    return new QWhatsThisButton( parent,
				 "automatic what's this? button" );
}

/*!
    Constructs a dynamic "What's this?" object for \a widget. The
    object is deleted when the \a widget is destroyed.

    When the widget is queried by the user the text() function of this
    QWhatsThis will be called to provide the appropriate text, rather
    than using the text assigned by add().
*/
QWhatsThis::QWhatsThis( QWidget * widget)
{
    QWhatsThisPrivate::setUpWhatsThis();
    wt->add(widget,this);
}


/*!
    Destroys the object and frees any allocated resources.
*/
QWhatsThis::~QWhatsThis()
{
}


/*!
    This virtual function returns the text for position \e p in the
    widget that this "What's this?" object documents. If there is no
    "What's this?" text for the position, QString::null is returned.

    The default implementation returns QString::null.
*/
QString QWhatsThis::text( const QPoint & )
{
    return QString::null;
}

/*!
    \fn bool QWhatsThis::clicked( const QString& href )

    This virtual function is called when the user clicks inside the
    "What's this?" window. \a href is the link the user clicked on, or
    QString::null if there was no link.

    If the function returns TRUE (the default), the "What's this?"
    window is closed, otherwise it remains visible.

    The default implementation ignores \a href and returns TRUE.
*/
bool QWhatsThis::clicked( const QString& )
{
    return TRUE;
}


/*!
    Enters "What's this?" mode and returns immediately.

    Qt will install a special cursor and take over mouse input until
    the user clicks somewhere. It then shows any help available and
    ends "What's this?" mode. Finally, Qt removes the special cursor
    and help window and then restores ordinary event processing, at
    which point the left mouse button is no longer pressed.

    The user can also use the Esc key to leave "What's this?" mode.

    \sa inWhatsThisMode(), leaveWhatsThisMode()
*/

void QWhatsThis::enterWhatsThisMode()
{
    QWhatsThisPrivate::setUpWhatsThis();
    if ( wt->state == QWhatsThisPrivate::Inactive ) {
	wt->enterWhatsThisMode();
#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor( whatsThisCursor, FALSE );
#endif
	wt->state = QWhatsThisPrivate::Waiting;
	qApp->installEventFilter( wt );
    }
}


/*!
    Returns TRUE if the application is in "What's this?" mode;
    otherwise returns FALSE.

    \sa enterWhatsThisMode(), leaveWhatsThisMode()
*/
bool QWhatsThis::inWhatsThisMode()
{
    if (!wt)
	return FALSE;
    return wt->state == QWhatsThisPrivate::Waiting;
}


/*!
    Leaves "What's this?" question mode.

    This function is used internally by widgets that support
    QWidget::customWhatsThis(); applications do not usually call it.
    An example of such a widget is QPopupMenu: menus still work
    normally in "What's this?" mode but also provide help texts for
    individual menu items.

    If \a text is not QString::null, a "What's this?" help window is
    displayed at the global screen position \a pos. If widget \a w is
    not 0 and has its own dedicated QWhatsThis object, this object
    will receive clicked() messages when the user clicks on hyperlinks
    inside the help text.

    \sa inWhatsThisMode(), enterWhatsThisMode(), QWhatsThis::clicked()
*/
void QWhatsThis::leaveWhatsThisMode( const QString& text, const QPoint& pos, QWidget* w )
{
    if ( !inWhatsThisMode() )
	return;

    wt->leaveWhatsThisMode();
    if ( !text.isNull() )
	wt->say( w, text, pos );
}

/*!
    Display \a text in a help window at the global screen position \a
    pos.

    If widget \a w is not 0 and has its own dedicated QWhatsThis
    object, this object will receive clicked() messages when the user
    clicks on hyperlinks inside the help text.

    \sa QWhatsThis::clicked()
*/
void QWhatsThis::display( const QString& text, const QPoint& pos, QWidget* w )
{
    if ( inWhatsThisMode() ) {
	leaveWhatsThisMode( text, pos, w );
	return;
    }
    QWhatsThisPrivate::setUpWhatsThis();
    wt->say( w, text, pos );
}

/*!
  Sets the font for all "What's this?" helps to \a font.
*/
void QWhatsThis::setFont( const QFont &font )
{
    QApplication::setFont( font, TRUE, "QWhatsThat" );
}

#include "qwhatsthis.moc"
#endif
