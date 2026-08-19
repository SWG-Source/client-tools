#include "qaccessiblewidget.h"

#include <qapplication.h>
#include <qstyle.h>
#include <qobjectlist.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qdial.h>
#include <qspinbox.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qprogressbar.h>
#include <qgroupbox.h>
#include <qtoolbutton.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qscrollview.h>
#include <qheader.h>
#include <qtabbar.h>
#include <qcombobox.h>
#include <qrangecontrol.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qiconview.h>
#include <qtextedit.h>
#include <qwidgetstack.h>
#include <private/qtitlebar_p.h>


QString buddyString( QWidget *widget )
{
    QWidget *parent = widget->parentWidget();
    QObjectList *ol = parent->queryList( "QLabel", 0, FALSE, FALSE );
    if ( !ol || !ol->count() ) {
	delete ol;
	return QString::null;
    }

    QString str;

    QObjectListIt it(*ol);
    while ( it.current() ) {
	QLabel *label = (QLabel*)it.current();
	++it;
	if ( label->buddy() == widget ) {
	    str = label->text();
	    break;
	}
    }
    delete ol;
    if ( !!str )
	return str;

    if ( parent->inherits( "QGroupBox" ) )
	return ((QGroupBox*)parent)->title();

    return QString::null;
}

QString stripAmp( const QString &text )
{
    if ( text.isEmpty() )
	return text;

    QString n = text;
    for ( uint i = 0; i < n.length(); i++ ) {
	if ( n[(int)i] == '&' )
	    n.remove( i, 1 );
    }
    return n;
}

QString hotKey( const QString &text )
{
    if ( text.isEmpty() )
	return text;

    QString n = text;
    int fa = 0;
    bool ac = FALSE;
    while ( ( fa = n.find( "&", fa ) ) != -1 ) {
	if ( n.at(fa+1) != '&' ) {
	    ac = TRUE;
	    break;
	}
    }
    if ( fa != -1 && ac )
	return QString( n.at(fa + 1) );

    return QString::null;
}

/*!
  \class QAccessibleWidget qaccessiblewidget.h
  \brief The QAccessibleWidget class implements the QAccessibleInterface for QWidgets.
*/

ulong QAccessibleWidget::objects = 0;

/*!
  Creates a QAccessibleWidget object for \a o.
  \a role, \a name, \a description, \a value, \a help, \a defAction,
  \a accelerator and \a state are optional parameters for static values
  of the object's property.
*/
QAccessibleWidget::QAccessibleWidget( QObject *o, Role role, QString name,
    QString description, QString value, QString help, QString defAction, QString accelerator, State state )
    : QAccessibleObject( o ), role_(role), state_(state), name_(name),
      description_(description),value_(value),help_(help),
      defAction_(defAction), accelerator_(accelerator)
{
    objects++;
}

QAccessibleWidget::~QAccessibleWidget()
{
    objects--;
}

/*! Returns the widget. */
QWidget *QAccessibleWidget::widget() const
{
    Q_ASSERT(object()->isWidgetType());
    if ( !object()->isWidgetType() )
	return 0;
    return (QWidget*)object();
}

/*! \reimp */
int QAccessibleWidget::controlAt( int x, int y ) const
{
    QWidget *w = widget();
    QPoint gp = w->mapToGlobal( QPoint( 0, 0 ) );
    if ( !QRect( gp.x(), gp.y(), w->width(), w->height() ).contains( x, y ) )
	return -1;

    QPoint rp = w->mapFromGlobal( QPoint( x, y ) );

    QObjectList *list = w->queryList( "QWidget", 0, FALSE, FALSE );

    if ( !list || list->isEmpty() )
	return 0;

    QObjectListIt it( *list );
    QWidget *child = 0;
    int index = 1;
    while ( ( child = (QWidget*)it.current() ) ) {
	if ( !child->isTopLevel() && !child->isHidden() && child->geometry().contains( rp ) ) {
	    delete list;
	    return index;
	}
	++it;
	++index;
    }
    delete list;
    return 0;
}

/*! \reimp */
QRect	QAccessibleWidget::rect( int control ) const
{
#if defined(QT_DEBUG)
    if ( control )
	qWarning( "QAccessibleWidget::rect: This implementation does not support subelements! (ID %d unknown for %s)", control, widget()->className() );
#else
    Q_UNUSED(control)
#endif
    QWidget *w = widget();
    QPoint wpos = w->mapToGlobal( QPoint( 0, 0 ) );

    return QRect( wpos.x(), wpos.y(), w->width(), w->height() );
}

/*! \reimp */
int QAccessibleWidget::navigate( NavDirection dir, int startControl ) const
{
#if defined(QT_DEBUG)
    if ( startControl )
	qWarning( "QAccessibleWidget::navigate: This implementation does not support subelements! (ID %d unknown for %s)", startControl, widget()->className() );
#else
    Q_UNUSED(startControl);
#endif
    QWidget *w = widget();
    switch ( dir ) {
    case NavFirstChild:
	{
	    QObjectList *list = w->queryList( "QWidget", 0, FALSE, FALSE );
	    bool has = !list->isEmpty();
	    delete list;
	    return has ? 1 : -1;
	}
    case NavLastChild:
	{
	    QObjectList *list = w->queryList( "QWidget", 0, FALSE, FALSE );
	    bool has = !list->isEmpty();
	    delete list;
	    return has ? childCount() : -1;
	}
    case NavNext:
    case NavPrevious:
	{
	    QWidget *parent = w->parentWidget();
	    QObjectList *sl = parent ? parent->queryList( "QWidget", 0, FALSE, FALSE ) : 0;
	    if ( !sl )
		return -1;
	    QObject *sib;
	    QObjectListIt it( *sl );
	    int index;
	    if ( dir == NavNext ) {
		index = 1;
		while ( ( sib = it.current() ) ) {
		    ++it;
		    ++index;
		    if ( sib == w )
			break;
		}
	    } else {
		it.toLast();
		index = sl->count();
		while ( ( sib = it.current() ) ) {
		    --it;
		    --index;
		    if ( sib == w )
			break;
		}
	    }
	    sib = it.current();
	    delete sl;
	    if ( sib )
		return index;
	    return -1;
	}
	break;
    case NavFocusChild:
	{
	    if ( w->hasFocus() )
		return 0;

	    QWidget *w2 = w->focusWidget();
	    if ( !w2 )
		return -1;

	    QObjectList *list = w->queryList( "QWidget", 0, FALSE, FALSE );
	    int index = list->findRef( w2 );
	    delete list;
	    return ( index != -1 ) ? index+1 : -1;
	}
    default:
	qWarning( "QAccessibleWidget::navigate: unhandled request" );
	break;
    };
    return -1;
}

/*! \reimp */
int QAccessibleWidget::childCount() const
{
    QObjectList *cl = widget()->queryList( "QWidget", 0, FALSE, FALSE );
    if ( !cl )
	return 0;

    int count = cl->count();
    delete cl;
    return count;
}

/*! \reimp */
QRESULT QAccessibleWidget::queryChild( int control, QAccessibleInterface **iface ) const
{
    *iface = 0;
    QObjectList *cl = widget()->queryList( "QWidget", 0, FALSE, FALSE );
    if ( !cl )
	return QS_FALSE;

    QObject *o = 0;
    if ( cl->count() >= (uint)control )
	o = cl->at( control-1 );
    delete cl;

    if ( !o )
	return QS_FALSE;

    return QAccessible::queryAccessibleInterface( o, iface );
}

/*! \reimp */
QRESULT QAccessibleWidget::queryParent( QAccessibleInterface **iface ) const
{
    return QAccessible::queryAccessibleInterface( widget()->parentWidget(), iface );
}

/*! \reimp */
bool QAccessibleWidget::doDefaultAction( int control )
{
#if defined(QT_DEBUG)
    if ( control )
	qWarning( "QAccessibleWidget::doDefaultAction: This implementation does not support subelements! (ID %d unknown for %s)", control, widget()->className() );
#else
    Q_UNUSED(control)
#endif
    return FALSE;
}

/*! \reimp */
QString QAccessibleWidget::text( Text t, int control ) const
{
    switch ( t ) {
    case DefaultAction:
	return defAction_;
    case Description:
	if ( !control && description_.isNull() ) {
	    QString desc = QToolTip::textFor( widget() );
	    return desc;
	}
	return description_;
    case Help:
	if ( !control && help_.isNull() ) {
	    QString help = QWhatsThis::textFor( widget() );
	    return help;
	}
	return help_;
    case Accelerator:
	return accelerator_;
    case Name:
	{
	    if ( !control && name_.isNull() && widget()->isTopLevel() )
		return widget()->caption();
	    return name_;
	}
    case Value:
	return value_;
    default:
	break;
    }
    return QString::null;
}

/*! \reimp */
void QAccessibleWidget::setText( Text t, int /*control*/, const QString &text )
{
    switch ( t ) {
    case DefaultAction:
	defAction_ = text;
	break;
    case Description:
	description_ = text;
	break;
    case Help:
	help_ = text;
	break;
    case Accelerator:
	accelerator_ = text;
	break;
    case Name:
	name_ = text;
	break;
    case Value:
	value_ = text;
	break;
    default:
	break;
    }
}

/*! \reimp */
QAccessible::Role QAccessibleWidget::role( int control ) const
{
    if ( !control )
	return role_;
    return NoRole;
}

/*! \reimp */
QAccessible::State QAccessibleWidget::state( int control ) const
{
    if ( control )
	return Normal;

    if ( state_ != Normal )
	return state_;

    int state = Normal;

    QWidget *w = widget();
    if ( w->isHidden() )
	state |= Invisible;
    if ( w->focusPolicy() != QWidget::NoFocus && w->isActiveWindow() )
	state |= Focusable;
    if ( w->hasFocus() )
	state |= Focused;
    if ( !w->isEnabled() )
	state |= Unavailable;
    if ( w->isTopLevel() ) {
	state |= Moveable;
	if ( w->minimumSize() != w->maximumSize() )
	    state |= Sizeable;
    }

    return (State)state;
}

/*! \reimp */
bool QAccessibleWidget::setFocus( int control )
{
#if defined(QT_DEBUG)
    if ( control )
	qWarning( "QAccessibleWidget::setFocus: This implementation does not support subelements! (ID %d unknown for %s)", control, widget()->className() );
#else
    Q_UNUSED(control)
#endif
    if ( widget()->focusPolicy() != QWidget::NoFocus ) {
	widget()->setFocus();
	return TRUE;
    }
    return FALSE;
}

/*! \reimp */
bool QAccessibleWidget::setSelected( int, bool, bool )
{
#if defined(QT_DEBUG)
    qWarning( "QAccessibleWidget::setSelected: This function not supported for simple widgets." );
#endif
    return FALSE;
}

/*! \reimp */
void QAccessibleWidget::clearSelection()
{
#if defined(QT_DEBUG)
    qWarning( "QAccessibleWidget::clearSelection: This function not supported for simple widgets." );
#endif
}

/*! \reimp */
QMemArray<int> QAccessibleWidget::selection() const
{
    return QMemArray<int>();
}

/*!
  \class QAccessibleWidgetStack qaccessible.h
  \brief The QAccessibleWidgetStack class implements the QAccessibleInterface for widget stacks.
*/

/*!
  Creates a QAccessibleWidgetStack object for \a o.
*/
QAccessibleWidgetStack::QAccessibleWidgetStack( QObject *o )
: QAccessibleWidget( o )
{
    Q_ASSERT( o->inherits("QWidgetStack") );
}

/*! Returns the widget stack. */
QWidgetStack *QAccessibleWidgetStack::widgetStack() const
{
    return (QWidgetStack*)object();
}

/*! \reimp */
int QAccessibleWidgetStack::controlAt( int, int ) const
{
    return widgetStack()->id( widgetStack()->visibleWidget() ) + 1;
}

/*! \reimp */
QRESULT QAccessibleWidgetStack::queryChild( int control, QAccessibleInterface **iface ) const
{
    if ( !control ) {
	*iface = (QAccessibleInterface*)this;
	return QS_OK;
    }

    QWidget *widget = widgetStack()->widget( control - 1 );
    if ( !widget )
	return QAccessibleWidget::queryChild( control, iface );
    return QAccessible::queryAccessibleInterface( widgetStack()->widget( control - 1 ), iface );
}


/*!
  \class QAccessibleButton qaccessible.h
  \brief The QAccessibleButton class implements the QAccessibleInterface for button type widgets.
*/

/*!
  Creates a QAccessibleButton object for \a o.
  \a role, \a description and \a help are propagated to the QAccessibleWidget constructor.
*/
QAccessibleButton::QAccessibleButton( QObject *o, Role role, QString description,
				     QString /* help */ )
: QAccessibleWidget( o, role, QString::null, description, QString::null,
		    QString::null, QString::null, QString::null )
{
    Q_ASSERT(o->inherits("QButton"));
}

/*! \reimp */
bool	QAccessibleButton::doDefaultAction( int control )
{
    if ( !widget()->isEnabled() )
	return FALSE;

    Role r = role(control);
    if ( r == PushButton || r ==  CheckBox || r == RadioButton ) {
	((QButton*)object())->animateClick();
    } else if ( object()->inherits("QToolButton") ) {
	QToolButton *tb = (QToolButton*)object();
	tb->openPopup();
    }

    return TRUE;
}

/*! \reimp */
QString QAccessibleButton::text( Text t, int control ) const
{
    QString tx = QAccessibleWidget::text( t, control );
    if ( !!tx )
	return tx;

    switch ( t ) {
    case DefaultAction:
	switch( role(control) ) {
	case PushButton:
	    return QButton::tr("Press");
	case CheckBox:
	    if ( state(control) & Checked )
		return QButton::tr("UnCheck");
	    return QButton::tr("Check");
	case RadioButton:
	    return QButton::tr("Check");
	default:
	    return QButton::tr("Press");
	}
    case Accelerator:
	tx = hotKey( ((QButton*)widget())->text() );
	if ( !!tx ) {
	    tx = "Alt + "+tx;
	} else {
	    tx = hotKey( buddyString( widget() ) );
	    if ( !!tx )
		tx = "Alt + "+tx;
	}
	return tx;
    case Name:
	tx = ((QButton*)widget())->text();
	if ( tx.isEmpty() && widget()->inherits("QToolButton") )
	    tx = ((QToolButton*)widget())->textLabel();
	if ( tx.isEmpty() )
	    tx = buddyString( widget() );

	return stripAmp( tx );
    default:
	break;
    }
    return tx;
}

/*! \reimp */
QAccessible::State QAccessibleButton::state( int control ) const
{
    int state = QAccessibleWidget::state( control );

    QButton *b = (QButton*)widget();
    if ( b->state() == QButton::On )
	state |= Checked;
    else  if ( b->state() == QButton::NoChange )
	    state |= Mixed;
    if ( b->isDown() )
	state |= Pressed;
    if ( b->inherits( "QPushButton" ) ) {
	QPushButton *pb = (QPushButton*)b;
	if ( pb->isDefault() )
	    state |= Default;
    }

    return (State)state;
}

/*!
  \class QAccessibleRangeControl qaccessiblewidget.h
  \brief The QAccessibleRangeControl class implements the QAccessibleInterface for range controls.
*/

/*!
  Constructs a QAccessibleRangeControl object for \a o.
  \a role, \a name, \a description, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleRangeControl::QAccessibleRangeControl( QObject *o, Role role, QString name,
						 QString description, QString help, QString defAction, QString accelerator )
: QAccessibleWidget( o, role, name, description, QString::null, help, defAction, accelerator )
{
}

/*! \reimp */
QString QAccessibleRangeControl::text( Text t, int control ) const
{
    QString tx = QAccessibleWidget::text( t, control );
    if ( !!tx )
	return stripAmp(tx);

    switch ( t ) {
    case Name:
	return stripAmp( buddyString( widget() ) );
    case Accelerator:
	tx = hotKey( buddyString( widget() ) );
	if ( !!tx )
	    return "Alt + "+tx;
	break;
    case Value:
	if ( widget()->inherits( "QSlider" ) ) {
	    QSlider *s = (QSlider*)widget();
	    return QString::number( s->value() );
	} else if ( widget()->inherits( "QDial" ) ) {
	    QDial *d = (QDial*)widget();
	    return QString::number( d->value() );
	} else if ( widget()->inherits( "QSpinBox" ) ) {
	    QSpinBox *s = (QSpinBox*)widget();
	    return s->text();
	} else if ( widget()->inherits( "QScrollBar" ) ) {
	    QScrollBar *s = (QScrollBar*)widget();
	    return QString::number( s->value() );
	} else if ( widget()->inherits( "QProgressBar" ) ) {
	    QProgressBar *p = (QProgressBar*)widget();
	    return QString::number( p->progress() );
	}
    default:
	break;
    }
    return tx;
}


/*!
  \class QAccessibleSpinWidget qaccessiblewidget.h
  \brief The QAccessibleSpinWidget class implements the QAccessibleInterface for up/down widgets.
*/

/*!
  Constructs a QAccessibleSpinWidget object for \a o.
*/
QAccessibleSpinWidget::QAccessibleSpinWidget( QObject *o )
: QAccessibleRangeControl( o, SpinBox )
{
}

/*! \reimp */
int QAccessibleSpinWidget::controlAt( int x, int y ) const
{
    QPoint tl = widget()->mapFromGlobal( QPoint( x, y ) );
    if ( ((QSpinWidget*)widget())->upRect().contains( tl ) )
	return 1;
    else if ( ((QSpinWidget*)widget())->downRect().contains( tl ) )
	return 2;

    return -1;
}

/*! \reimp */
QRect QAccessibleSpinWidget::rect( int control ) const
{
    QRect rect;
    switch( control ) {
    case 1:
	rect = ((QSpinWidget*)widget())->upRect();
	break;
    case 2:
	rect = ((QSpinWidget*)widget())->downRect();
	break;
    default:
	rect = widget()->rect();
    }
    QPoint tl = widget()->mapToGlobal( QPoint( 0, 0 ) );
    return QRect( tl.x() + rect.x(), tl.y() + rect.y(), rect.width(), rect.height() );
}

/*! \reimp */
int QAccessibleSpinWidget::navigate( NavDirection direction, int startControl ) const
{
    if ( direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl )
	return QAccessibleWidget::navigate( direction, startControl );

    switch ( direction ) {
    case NavFirstChild:
	return 1;
    case NavLastChild:
	return 2;
    case NavNext:
    case NavDown:
	startControl += 1;
	if ( startControl > 2 )
	    return -1;
	return startControl;
    case NavPrevious:
    case NavUp:
	startControl -= 1;
	if ( startControl < 1 )
	    return -1;
	return startControl;
    default:
	break;
    }

    return -1;
}

/*! \reimp */
int QAccessibleSpinWidget::childCount() const
{
    return 2;
}

/*! \reimp */
QRESULT QAccessibleSpinWidget::queryChild( int /*control*/, QAccessibleInterface **iface ) const
{
    *iface = 0;
    return QS_FALSE;
}

/*! \reimp */
QString QAccessibleSpinWidget::text( Text t, int control ) const
{
    switch ( t ) {
    case Name:
	switch ( control ) {
	case 1:
	    return QSpinWidget::tr("More");
	case 2:
	    return QSpinWidget::tr("Less");
	default:
	    break;
	}
	break;
    case DefaultAction:
	switch( control ) {
	case 1:
	case 2:
	    return QSpinWidget::tr("Press");
	default:
	    break;
	}
	break;
    default:
	break;
    }
    return QAccessibleRangeControl::text( t, control );
}

/*! \reimp */
QAccessible::Role QAccessibleSpinWidget::role( int control ) const
{
    switch( control ) {
    case 1:
	return PushButton;
    case 2:
	return PushButton;
    default:
	break;
    }
    return QAccessibleRangeControl::role( control );
}

/*! \reimp */
QAccessible::State QAccessibleSpinWidget::state( int control ) const
{
    int state = QAccessibleRangeControl::state( control );
    switch( control ) {
    case 1:
	if ( !((QSpinWidget*)widget())->isUpEnabled() )
	    state |= Unavailable;
	return (State)state;
    case 2:
	if ( !((QSpinWidget*)widget())->isDownEnabled() )
	    state |= Unavailable;
	return (State)state;
    default:
	break;
    }
    return QAccessibleRangeControl::state( control );
}

/*! \reimp */
bool QAccessibleSpinWidget::doDefaultAction( int control )
{
    switch( control ) {
    case 1:
	if ( !((QSpinWidget*)widget())->isUpEnabled() )
	    return FALSE;
	((QSpinWidget*)widget())->stepUp();
	return TRUE;
    case 2:
	if ( !((QSpinWidget*)widget())->isDownEnabled() )
	    return FALSE;
	((QSpinWidget*)widget())->stepDown();
	return TRUE;
    default:
	break;
    }
    return QAccessibleRangeControl::doDefaultAction( control );
}

/*!
  \class QAccessibleScrollBar qaccessiblewidget.h
  \brief The QAccessibleScrollBar class implements the QAccessibleInterface for scroll bars.
*/

/*!
  Constructs a QAccessibleScrollBar object for \a o.
  \a name, \a description, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleRangeControl constructor.
*/
QAccessibleScrollBar::QAccessibleScrollBar( QObject *o, QString name,
    QString description, QString help, QString defAction, QString accelerator )
: QAccessibleRangeControl( o, ScrollBar, name, description, help, defAction, accelerator )
{
    Q_ASSERT( o->inherits("QScrollBar" ) );
}

/*! Returns the scroll bar. */
QScrollBar *QAccessibleScrollBar::scrollBar() const
{
    return (QScrollBar*)widget();
}

/*! \reimp */
int QAccessibleScrollBar::controlAt( int x, int y ) const
{
    for ( int i = 1; i <= childCount(); i++ ) {
	if ( rect(i).contains( x,y ) )
	    return i;
    }
    return 0;
}

/*! \reimp */
QRect QAccessibleScrollBar::rect( int control ) const
{
    QRect rect;
    QRect srect = scrollBar()->sliderRect();
    int sz = scrollBar()->style().pixelMetric( QStyle::PM_ScrollBarExtent, scrollBar() );
    switch ( control ) {
    case 1:
	if ( scrollBar()->orientation() == Vertical )
	    rect = QRect( 0, 0, sz, sz );
	else
	    rect = QRect( 0, 0, sz, sz );
	break;
    case 2:
	if ( scrollBar()->orientation() == Vertical )
	    rect = QRect( 0, sz, sz, srect.y() - sz );
	else
	    rect = QRect( sz, 0, srect.x() - sz, sz );
	break;
    case 3:
	rect = srect;
	break;
    case 4:
	if ( scrollBar()->orientation() == Vertical )
	    rect = QRect( 0, srect.bottom(), sz, scrollBar()->rect().height() - srect.bottom() - sz );
	else
	    rect = QRect( srect.right(), 0, scrollBar()->rect().width() - srect.right() - sz, sz ) ;
	break;
    case 5:
	if ( scrollBar()->orientation() == Vertical )
	    rect = QRect( 0, scrollBar()->rect().height() - sz, sz, sz );
	else
	    rect = QRect( scrollBar()->rect().width() - sz, 0, sz, sz );
	break;
    default:
	return QAccessibleRangeControl::rect( control );
    }

    QPoint tp = scrollBar()->mapToGlobal( QPoint( 0,0 ) );
    return QRect( tp.x() + rect.x(), tp.y() + rect.y(), rect.width(), rect.height() );
}

/*! \reimp */
int QAccessibleScrollBar::navigate( NavDirection direction, int startControl ) const
{
    if ( direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl )
	return QAccessibleRangeControl::navigate( direction, startControl );

    switch ( direction ) {
    case NavFirstChild:
	return 1;
    case NavLastChild:
	return 5;
    case NavNext:
	return startControl == childCount() ? -1 : startControl + 1;
    case NavDown:
	if ( scrollBar()->orientation() == Horizontal )
	    break;
	return startControl == childCount() ? -1 : startControl + 1;
    case NavRight:
	if ( scrollBar()->orientation() == Vertical )
	    break;
	return startControl == childCount() ? -1 : startControl + 1;
    case NavPrevious:
	return startControl == 1 ? -1 : startControl - 1;
    case NavUp:
	if ( scrollBar()->orientation() == Horizontal )
	    break;
	return startControl == 1 ? -1 : startControl - 1;
    case NavLeft:
	if ( scrollBar()->orientation() == Vertical )
	    break;
	return startControl == 1 ? -1 : startControl - 1;
    default:
	break;
    }

    return -1;
}

/*! \reimp */
int QAccessibleScrollBar::childCount() const
{
    return 5;
}

/*! \reimp */
QRESULT	QAccessibleScrollBar::queryChild( int /*control*/, QAccessibleInterface **iface ) const
{
    *iface = 0;
    return QS_FALSE;
}

/*! \reimp */
QString	QAccessibleScrollBar::text( Text t, int control ) const
{
    switch ( t ) {
    case Value:
	if ( control && control != 3 )
	    return QString::null;
	break;
    case Name:
	switch ( control ) {
	case 1:
	    return QScrollBar::tr("Line up");
	case 2:
	    return QScrollBar::tr("Page up");
	case 3:
	    return QScrollBar::tr("Position");
	case 4:
	    return QScrollBar::tr("Page down");
	case 5:
	    return QScrollBar::tr("Line down");
	}
	break;
    case DefaultAction:
	if ( control != 3 )
	    return QScrollBar::tr("Press");
	break;
    default:
	break;

    }
    return QAccessibleRangeControl::text( t, control );
}

/*! \reimp */
QAccessible::Role QAccessibleScrollBar::role( int control ) const
{
    switch ( control ) {
    case 1:
    case 2:
	return PushButton;
    case 3:
	return Indicator;
    case 4:
    case 5:
	return PushButton;
    default:
	return ScrollBar;
    }
}

/*! \reimp */
bool QAccessibleScrollBar::doDefaultAction( int control )
{
    switch ( control ) {
    case 1:
	scrollBar()->subtractLine();
	return TRUE;
    case 2:
	scrollBar()->subtractPage();
	return TRUE;
    case 4:
	scrollBar()->addPage();
	return TRUE;
    case 5:
	scrollBar()->addLine();
	return TRUE;
    default:
	return FALSE;
    }
}

/*!
  \class QAccessibleSlider qaccessiblewidget.h
  \brief The QAccessibleScrollBar class implements the QAccessibleInterface for sliders.
*/

/*!
  Constructs a QAccessibleScrollBar object for \a o.
  \a name, \a description, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleRangeControl constructor.
*/
QAccessibleSlider::QAccessibleSlider( QObject *o, QString name,
    QString description, QString help, QString defAction, QString accelerator )
: QAccessibleRangeControl( o, ScrollBar, name, description, help, defAction, accelerator )
{
    Q_ASSERT( o->inherits("QSlider" ) );
}

/*! Returns the slider. */
QSlider *QAccessibleSlider::slider() const
{
    return (QSlider*)widget();
}

/*! \reimp */
int QAccessibleSlider::controlAt( int x, int y ) const
{
    for ( int i = 1; i <= childCount(); i++ ) {
	if ( rect(i).contains( x,y ) )
	    return i;
    }
    return 0;
}

/*! \reimp */
QRect QAccessibleSlider::rect( int control ) const
{
    QRect rect;
    QRect srect = slider()->sliderRect();
    switch ( control ) {
    case 1:
	if ( slider()->orientation() == Vertical )
	    rect = QRect( 0, 0, slider()->width(), srect.y() );
	else
	    rect = QRect( 0, 0, srect.x(), slider()->height() );
	break;
    case 2:
	rect = srect;
	break;
    case 3:
	if ( slider()->orientation() == Vertical )
	    rect = QRect( 0, srect.y() + srect.height(), slider()->width(), slider()->height()- srect.y() - srect.height() );
	else
	    rect = QRect( srect.x() + srect.width(), 0, slider()->width() - srect.x() - srect.width(), slider()->height() );
	break;
    default:
	return QAccessibleRangeControl::rect( control );
    }

    QPoint tp = slider()->mapToGlobal( QPoint( 0,0 ) );
    return QRect( tp.x() + rect.x(), tp.y() + rect.y(), rect.width(), rect.height() );
}

/*! \reimp */
int QAccessibleSlider::navigate( NavDirection direction, int startControl ) const
{
    if ( direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl )
	return QAccessibleRangeControl::navigate( direction, startControl );

    switch ( direction ) {
    case NavFirstChild:
	return 1;
    case NavLastChild:
	return childCount();
    case NavNext:
	return startControl == childCount() ? -1 : startControl + 1;
    case NavDown:
	if ( slider()->orientation() == Horizontal )
	    break;
	return startControl == childCount() ? -1 : startControl + 1;
    case NavRight:
	if ( slider()->orientation() == Vertical )
	    break;
	return startControl == childCount() ? -1 : startControl + 1;
    case NavPrevious:
	return startControl == 1 ? -1 : startControl - 1;
    case NavUp:
	if ( slider()->orientation() == Horizontal )
	    break;
	return startControl == 1 ? -1 : startControl - 1;
    case NavLeft:
	if ( slider()->orientation() == Vertical )
	    break;
	return startControl == 1 ? -1 : startControl - 1;
    default:
	break;
    }

    return -1;
}

/*! \reimp */
int QAccessibleSlider::childCount() const
{
    return 3;
}

/*! \reimp */
QRESULT	QAccessibleSlider::queryChild( int /*control*/, QAccessibleInterface **iface ) const
{
    *iface = 0;
    return QS_FALSE;
}

/*! \reimp */
QString	QAccessibleSlider::text( Text t, int control ) const
{
    switch ( t ) {
    case Value:
	if ( control && control != 2 )
	    return QString::null;
	break;
    case Name:
	switch ( control ) {
	case 1:
	    return QSlider::tr("Page up");
	case 2:
	    return QSlider::tr("Position");
	case 3:
	    return QSlider::tr("Page down");
	}
	break;
    case DefaultAction:
	if ( control != 2 )
	    return QSlider::tr("Press");
	break;
    default:
	break;
    }
    return QAccessibleRangeControl::text( t, control );
}

/*! \reimp */
QAccessible::Role QAccessibleSlider::role( int control ) const
{
    switch ( control ) {
    case 1:
	return PushButton;
    case 2:
	return Indicator;
    case 3:
	return PushButton;
    default:
	return Slider;
    }
}

/*! \reimp */
bool QAccessibleSlider::doDefaultAction( int control )
{
    switch ( control ) {
    case 1:
	slider()->subtractLine();
	return TRUE;
    case 3:
	slider()->addLine();
	return TRUE;
    default:
	return FALSE;
    }
}


/*!
  \class QAccessibleText qaccessiblewidget.h
  \brief The QAccessibleText class implements the QAccessibleInterface for widgets with editable text.
*/

/*!
  Constructs a QAccessibleText object for \a o.
  \a role, \a name, \a description, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleText::QAccessibleText( QObject *o, Role role, QString name, QString description, QString help, QString defAction, QString accelerator )
: QAccessibleWidget( o, role, name, description, QString::null, help, defAction, accelerator )
{
}

/*! \reimp */
QString QAccessibleText::text( Text t, int control ) const
{
    QString str = QAccessibleWidget::text( t, control );
    if ( !!str )
	return str;
    switch ( t ) {
    case Name:
	return stripAmp( buddyString( widget() ) );
    case Accelerator:
	str = hotKey( buddyString( widget() ) );
	if ( !!str )
	    return "Alt + "+str;
	break;
    case Value:
	if ( widget()->inherits( "QLineEdit" ) )
	    return ((QLineEdit*)widget())->text();
	break;
    default:
	break;
    }
    return str;
}

/*! \reimp */
void QAccessibleText::setText(Text t, int control, const QString &text)
{
    if (t != Value || !widget()->inherits("QLineEdit") || control) {
        QAccessibleWidget::setText(t, control, text);
        return;
    }
    ((QLineEdit*)widget())->setText(text);
}

/*! \reimp */
QAccessible::State QAccessibleText::state( int control ) const
{
    int state = QAccessibleWidget::state( control );

    if ( widget()->inherits( "QLineEdit" ) ) {
	QLineEdit *l = (QLineEdit*)widget();
	if ( l->isReadOnly() )
	    state |= ReadOnly;
	if ( l->echoMode() == QLineEdit::Password )
	    state |= Protected;
	state |= Selectable;
	if ( l->hasSelectedText() )
	    state |= Selected;
    }

    return (State)state;
}

/*!
  \class QAccessibleDisplay qaccessiblewidget.h
  \brief The QAccessibleDisplay class implements the QAccessibleInterface for widgets that display static information.
*/

/*!
  Constructs a QAccessibleDisplay object for \a o.
  \a role, \a description, \a value, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleDisplay::QAccessibleDisplay( QObject *o, Role role, QString description, QString value, QString help, QString defAction, QString accelerator )
: QAccessibleWidget( o, role, QString::null, description, value, help, defAction, accelerator )
{
}

/*! \reimp */
QAccessible::Role QAccessibleDisplay::role( int control ) const
{
    if ( widget()->inherits( "QLabel" ) ) {
	QLabel *l = (QLabel*)widget();
	if ( l->pixmap() || l->picture() )
	    return Graphic;
#ifndef QT_NO_PICTURE
	if ( l->picture() )
	    return Graphic;
#endif
#ifndef QT_NO_MOVIE
	if ( l->movie() )
	    return Animation;
#endif
    }
    return QAccessibleWidget::role( control );
}

/*! \reimp */
QString QAccessibleDisplay::text( Text t, int control ) const
{
    QString str = QAccessibleWidget::text( t, control );
    if ( !!str )
	return str;

    switch ( t ) {
    case Name:
	if ( widget()->inherits( "QLabel" ) ) {
	    return stripAmp( ((QLabel*)widget())->text() );
	} else if ( widget()->inherits( "QLCDNumber" ) ) {
	    QLCDNumber *l = (QLCDNumber*)widget();
	    if ( l->numDigits() )
		return QString::number( l->value() );
	    return QString::number( l->intValue() );
	} else if ( widget()->inherits( "QGroupBox" ) ) {
	    return stripAmp( ((QGroupBox*)widget())->title() );
	}
	break;
    default:
	break;
    }
    return str;
}


/*!
  \class QAccessibleHeader qaccessiblewidget.h
  \brief The QAccessibleHeader class implements the QAccessibleInterface for header widgets.
*/

/*!
  Constructs a QAccessibleHeader object for \a o.
  \a role, \a description, \a value, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleHeader::QAccessibleHeader( QObject *o, QString description,
    QString value, QString help, QString defAction, QString accelerator )
    : QAccessibleWidget( o, NoRole, description, value, help, defAction, accelerator )
{
    Q_ASSERT(widget()->inherits("QHeader"));
}

/*! Returns the QHeader. */
QHeader *QAccessibleHeader::header() const
{
    return (QHeader *)widget();
}

/*! \reimp */
int QAccessibleHeader::controlAt( int x, int y ) const
{
    QPoint point = header()->mapFromGlobal( QPoint( x, y ) );
    for ( int i = 0; i < header()->count(); i++ ) {
	if ( header()->sectionRect( i ).contains( point ) )
	    return i+1;
    }
    return -1;
}

/*! \reimp */
QRect QAccessibleHeader::rect( int control ) const
{
    QPoint zero = header()->mapToGlobal( QPoint ( 0,0 ) );
    QRect sect = header()->sectionRect( control - 1 );
    return QRect( sect.x() + zero.x(), sect.y() + zero.y(), sect.width(), sect.height() );
}

/*! \reimp */
int QAccessibleHeader::navigate( NavDirection direction, int startControl ) const
{
    if ( direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl )
	return QAccessibleWidget::navigate( direction, startControl );

    int count = header()->count();
    switch ( direction ) {
    case NavFirstChild:
	return 1;
    case NavLastChild:
	return count;
    case NavNext:
	return startControl + 1 > count ? -1 : startControl + 1;
    case NavPrevious:
	return startControl - 1 < 1 ? -1 : startControl - 1;
    case NavUp:
	if ( header()->orientation() == Vertical )
	    return startControl - 1 < 1 ? -1 : startControl - 1;
	return -1;
    case NavDown:
	if ( header()->orientation() == Vertical )
	    return startControl + 1 > count ? -1 : startControl + 1;
	break;
    case NavLeft:
	if ( header()->orientation() == Horizontal )
	    return startControl - 1 < 1 ? -1 : startControl - 1;
	break;
    case NavRight:
	if ( header()->orientation() == Horizontal )
	    return startControl + 1 > count ? -1 : startControl + 1;
	break;
    default:
	break;
    }
    return -1;
}

/*! \reimp */
int QAccessibleHeader::childCount() const
{
    return header()->count();
}

/*! \reimp */
QRESULT QAccessibleHeader::queryChild( int /*control*/, QAccessibleInterface **iface ) const
{
    *iface = 0;
    return QS_FALSE;
}

/*! \reimp */
QString QAccessibleHeader::text( Text t, int control ) const
{
    QString str = QAccessibleWidget::text( t, control );
    if ( !!str )
	return str;

    switch ( t ) {
    case Name:
	return header()->label( control - 1 );
    default:
	break;
    }
    return str;
}

/*! \reimp */
QAccessible::Role QAccessibleHeader::role( int /*control*/ ) const
{
    if ( header()->orientation() == Qt::Horizontal )
	return ColumnHeader;
    else
	return RowHeader;
}

/*! \reimp */
QAccessible::State QAccessibleHeader::state( int control ) const
{
    return QAccessibleWidget::state( control );
}


/*!
  \class QAccessibleTabBar qaccessiblewidget.h
  \brief The QAccessibleTabBar class implements the QAccessibleInterface for tab bars.
*/

/*!
  Constructs a QAccessibleTabBar object for \a o.
  \a role, \a description, \a value, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleTabBar::QAccessibleTabBar( QObject *o, QString description,
    QString value, QString help, QString defAction, QString accelerator )
    : QAccessibleWidget( o, NoRole, description, value, help, defAction, accelerator )
{
    Q_ASSERT(widget()->inherits("QTabBar"));
}

/*! Returns the QHeader. */
QTabBar *QAccessibleTabBar::tabBar() const
{
    return (QTabBar*)widget();
}

/*! \reimp */
int QAccessibleTabBar::controlAt( int x, int y ) const
{
    int wc = QAccessibleWidget::controlAt( x, y );
    if ( wc )
	return wc + tabBar()->count();

    QPoint tp = tabBar()->mapFromGlobal( QPoint( x,y ) );
    QTab *tab = tabBar()->selectTab( tp );
    return tabBar()->indexOf( tab->identifier() ) + 1;
}

/*! \reimp */
QRect QAccessibleTabBar::rect( int control ) const
{
    if ( !control )
	return QAccessibleWidget::rect( 0 );
    if ( control > tabBar()->count() ) {
	QAccessibleInterface *iface;
	QAccessibleWidget::queryChild( control - tabBar()->count(), &iface );
	if ( !iface )
	    return QRect();
	return iface->rect( 0 );
    }

    QTab *tab = tabBar()->tabAt( control - 1 );

    QPoint tp = tabBar()->mapToGlobal( QPoint( 0,0 ) );
    QRect rec = tab->rect();
    return QRect( tp.x() + rec.x(), tp.y() + rec.y(), rec.width(), rec.height() );
}

/*! \reimp */
QRESULT	QAccessibleTabBar::queryChild( int /*control*/, QAccessibleInterface **iface ) const
{
    *iface = 0;
    return QS_FALSE;
}

/*! \reimp */
int QAccessibleTabBar::navigate( NavDirection direction, int startControl ) const
{
    if ( direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl )
	return QAccessibleWidget::navigate( direction, startControl );

    switch ( direction ) {
    case NavFirstChild:
	return 1;
	break;
    case NavLastChild:
	return childCount();
	break;
    case NavNext:
    case NavRight:
	return startControl + 1 > childCount() ? -1 : startControl + 1;
    case NavPrevious:
    case NavLeft:
	return startControl -1 < 1 ? -1 : startControl - 1;
    default:
	break;
    }
    return -1;
}

/*! \reimp */
int QAccessibleTabBar::childCount() const
{
    int wc = QAccessibleWidget::childCount();
    wc += tabBar()->count();
    return wc;
}

/*! \reimp */
QString QAccessibleTabBar::text( Text t, int control ) const
{
    QString str = QAccessibleWidget::text( t, control );
    if ( !!str )
	return str;

    if ( !control )
	return QAccessibleWidget::text( t, control );
    if ( control > tabBar()->count() ) {
	QAccessibleInterface *iface;
	QAccessibleWidget::queryChild( control - tabBar()->count(), &iface );
	if ( !iface )
	    return QAccessibleWidget::text( t, 0 );
	return iface->text( t, 0 );
    }

    QTab *tab = tabBar()->tabAt( control - 1 );
    if ( !tab )
	return QAccessibleWidget::text( t, 0 );

    switch ( t ) {
    case Name:
	return stripAmp( tab->text() );
    case DefaultAction:
	return QTabBar::tr( "Switch" );
    default:
	break;
    }
    return str;
}

/*! \reimp */
QAccessible::Role QAccessibleTabBar::role( int control ) const
{
    if ( !control )
	return PageTabList;
    if ( control > tabBar()->count() ) {
	QAccessibleInterface *iface;
	QAccessibleWidget::queryChild( control - tabBar()->count(), &iface );
	if ( !iface )
	    return QAccessibleWidget::role( 0 );
	return iface->role( 0 );
    }

    return PageTab;
}

/*! \reimp */
QAccessible::State QAccessibleTabBar::state( int control ) const
{
    int st = QAccessibleWidget::state( 0 );

    if ( !control )
	return (State)st;
    if ( control > tabBar()->count() ) {
	QAccessibleInterface *iface;
	QAccessibleWidget::queryChild( control - tabBar()->count(), &iface );
	if ( !iface )
	    return (State)st;
	return iface->state( 0 );
    }

    QTab *tab = tabBar()->tabAt( control - 1 );
    if ( !tab )
	return (State)st;

    if ( !tab->isEnabled() )
	st |= Unavailable;
    else
	st |= Selectable;

    if ( tabBar()->currentTab() == tab->identifier() )
	st |= Selected;

    return (State)st;
}

/*! \reimp */
bool QAccessibleTabBar::doDefaultAction( int control )
{
    if ( !control )
	return FALSE;
    if ( control > tabBar()->count() ) {
	QAccessibleInterface *iface;
	QAccessibleWidget::queryChild( control - tabBar()->count(), &iface );
	if ( !iface )
	    return FALSE;
	return iface->doDefaultAction( 0 );
    }

    QTab *tab = tabBar()->tabAt( control - 1 );
    if ( !tab || !tab->isEnabled() )
	return FALSE;
    tabBar()->setCurrentTab( tab );
    return TRUE;
}

/*! \reimp */
bool QAccessibleTabBar::setSelected( int control, bool on, bool extend )
{
    if ( !control || !on || extend || control > tabBar()->count() )
	return FALSE;

    QTab *tab = tabBar()->tabAt( control - 1 );
    if ( !tab || !tab->isEnabled() )
	return FALSE;
    tabBar()->setCurrentTab( tab );
    return TRUE;
}

/*! \reimp */
void QAccessibleTabBar::clearSelection()
{
}

/*! \reimp */
QMemArray<int> QAccessibleTabBar::selection() const
{
    QMemArray<int> array( 1 );
    array.at(0) = tabBar()->indexOf( tabBar()->currentTab() ) + 1;

    return array;
}

/*!
  \class QAccessibleComboBox qaccessiblewidget.h
  \brief The QAccessibleComboBox class implements the QAccessibleInterface for editable and read-only combo boxes.
*/


/*!
  Constructs a QAccessibleComboBox object for \a o.
*/
QAccessibleComboBox::QAccessibleComboBox( QObject *o )
: QAccessibleWidget( o, ComboBox )
{
    Q_ASSERT(o->inherits("QComboBox"));
}

/*!
  Returns the combo box.
*/
QComboBox *QAccessibleComboBox::comboBox() const
{
    return (QComboBox*)object();
}

/*! \reimp */
int QAccessibleComboBox::controlAt( int x, int y ) const
{
    for ( int i = childCount(); i >= 0; --i ) {
	if ( rect( i ).contains( x, y ) )
	    return i;
    }
    return -1;
}

/*! \reimp */
QRect QAccessibleComboBox::rect( int control ) const
{
    QPoint tp;
    QRect r;

    switch( control ) {
    case 1:
	if ( comboBox()->editable() ) {
	    tp = comboBox()->lineEdit()->mapToGlobal( QPoint( 0,0 ) );
	    r = comboBox()->lineEdit()->rect();
	} else  {
	    tp = comboBox()->mapToGlobal( QPoint( 0,0 ) );
	    r = comboBox()->style().querySubControlMetrics( QStyle::CC_ComboBox, comboBox(), QStyle::SC_ComboBoxEditField );
	}
	break;
    case 2:
	tp = comboBox()->mapToGlobal( QPoint( 0,0 ) );
	r = comboBox()->style().querySubControlMetrics( QStyle::CC_ComboBox, comboBox(), QStyle::SC_ComboBoxArrow );
	break;
    default:
	return QAccessibleWidget::rect( control );
    }
    return QRect( tp.x() + r.x(), tp.y() + r.y(), r.width(), r.height() );
}

/*! \reimp */
int QAccessibleComboBox::navigate( NavDirection direction, int startControl ) const
{
    if ( direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl )
	return QAccessibleWidget::navigate( direction, startControl );

    switch ( direction ) {
    case NavFirstChild:
	return 1;
	break;
    case NavLastChild:
	return childCount();
	break;
    case NavNext:
    case NavRight:
	return startControl + 1 > childCount() ? -1 : startControl + 1;
    case NavPrevious:
    case NavLeft:
	return startControl -1 < 1 ? -1 : startControl - 1;
    default:
	break;
    }
    return -1;
}

/*! \reimp */
int QAccessibleComboBox::childCount() const
{
    return 2;
}

/*! \reimp */
QRESULT	QAccessibleComboBox::queryChild( int /*control*/, QAccessibleInterface **iface ) const
{
    *iface = 0;
    return QS_FALSE;
}

/*! \reimp */
QString QAccessibleComboBox::text( Text t, int control ) const
{
    QString str;

    switch ( t ) {
    case Name:
	if ( control < 2 )
	    return stripAmp( buddyString( comboBox() ) );
	return QComboBox::tr("Open");
    case Accelerator:
	if ( control < 2 ) {
	    str = hotKey( buddyString( comboBox() ) );
	    if ( !!str )
		return "Alt + " + str;
	    return str;
	}
	return QComboBox::tr("Alt + Down Arrow" );
    case Value:
	if ( control < 2 ) {
	    if ( comboBox()->editable() )
		return comboBox()->lineEdit()->text();
	    return comboBox()->currentText();
	}
	break;
    case DefaultAction:
	if ( control == 2 )
	    return QComboBox::tr("Open");
	break;
    default:
	str = QAccessibleWidget::text( t, 0 );
	break;
    }
    return str;
}

/*! \reimp */
QAccessible::Role QAccessibleComboBox::role( int control ) const
{
    switch ( control ) {
    case 0:
	return ComboBox;
    case 1:
	if ( comboBox()->editable() )
	    return EditableText;
	return StaticText;
    case 2:
	return PushButton;
    default:
	return List;
    }
}

/*! \reimp */
QAccessible::State QAccessibleComboBox::state( int /*control*/ ) const
{
    return QAccessibleWidget::state( 0 );
}

/*! \reimp */
bool QAccessibleComboBox::doDefaultAction( int control )
{
    if ( control != 2 )
	return FALSE;
    comboBox()->popup();
    return TRUE;
}

/*!
  \class QAccessibleTitleBar qaccessiblewidget.h
  \brief The QAccessibleTitleBar class implements the QAccessibleInterface for title bars.
*/

/*!
  Constructs a QAccessibleComboBox object for \a o.
*/
QAccessibleTitleBar::QAccessibleTitleBar( QObject *o )
: QAccessibleWidget( o, ComboBox )
{
    Q_ASSERT(o->inherits("QTitleBar"));
}

/*!
  Returns the title bar.
*/
QTitleBar *QAccessibleTitleBar::titleBar() const
{
    return (QTitleBar*)object();
}

/*! \reimp */
int QAccessibleTitleBar::controlAt( int x, int y ) const
{
    int ctrl = titleBar()->style().querySubControl( QStyle::CC_TitleBar, titleBar(), titleBar()->mapFromGlobal( QPoint( x,y ) ) );

    switch ( ctrl )
    {
    case QStyle::SC_TitleBarSysMenu:
	return 1;
    case QStyle::SC_TitleBarLabel:
	return 2;
    case QStyle::SC_TitleBarMinButton:
	return 3;
    case QStyle::SC_TitleBarMaxButton:
	return 4;
    case QStyle::SC_TitleBarCloseButton:
	return 5;
    default:
	break;
    }
    return 0;
}

/*! \reimp */
QRect QAccessibleTitleBar::rect( int control ) const
{
    if ( !control )
	return QAccessibleWidget::rect( control );

    QRect r;
    switch ( control ) {
    case 1:
	r = titleBar()->style().querySubControlMetrics( QStyle::CC_TitleBar, titleBar(), QStyle::SC_TitleBarSysMenu );
	break;
    case 2:
	r = titleBar()->style().querySubControlMetrics( QStyle::CC_TitleBar, titleBar(), QStyle::SC_TitleBarLabel );
	break;
    case 3:
	r = titleBar()->style().querySubControlMetrics( QStyle::CC_TitleBar, titleBar(), QStyle::SC_TitleBarMinButton );
	break;
    case 4:
	r = titleBar()->style().querySubControlMetrics( QStyle::CC_TitleBar, titleBar(), QStyle::SC_TitleBarMaxButton );
	break;
    case 5:
	r = titleBar()->style().querySubControlMetrics( QStyle::CC_TitleBar, titleBar(), QStyle::SC_TitleBarCloseButton );
	break;
    default:
	break;
    }

    QPoint tp = titleBar()->mapToGlobal( QPoint( 0,0 ) );
    return QRect( tp.x() + r.x(), tp.y() + r.y(), r.width(), r.height() );
}

/*! \reimp */
int QAccessibleTitleBar::navigate( NavDirection direction, int startControl ) const
{
    if ( direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl )
	return QAccessibleWidget::navigate( direction, startControl );

    switch ( direction ) {
    case NavFirstChild:
	return 1;
	break;
    case NavLastChild:
	return childCount();
	break;
    case NavNext:
    case NavRight:
	return startControl + 1 > childCount() ? -1 : startControl + 1;
    case NavPrevious:
    case NavLeft:
	return startControl -1 < 1 ? -1 : startControl - 1;
    default:
	break;
    }
    return -1;
}

/*! \reimp */
int QAccessibleTitleBar::childCount() const
{
    return 5;
}

/*! \reimp */
QRESULT QAccessibleTitleBar::queryChild( int /*control*/, QAccessibleInterface **iface ) const
{
    *iface = 0;
    return QS_FALSE;
}

/*! \reimp */
QString QAccessibleTitleBar::text( Text t, int control ) const
{
    QString str = QAccessibleWidget::text( t, control );
    if ( !!str )
	return str;

    switch ( t ) {
    case Name:
	switch ( control ) {
	case 1:
	    return QTitleBar::tr("System");
	case 3:
	    if ( titleBar()->window()->isMinimized() )
		return QTitleBar::tr("Restore up");
	    return QTitleBar::tr("Minimize");
	case 4:
	    if ( titleBar()->window()->isMaximized() )
		return QTitleBar::tr("Restore down");
	    return QTitleBar::tr("Maximize");
	case 5:
	    return QTitleBar::tr("Close");
	default:
	    break;
	}
	break;
    case Value:
	if ( !control || control == 2 )
	    return titleBar()->window()->caption();
	break;
    case DefaultAction:
	if ( control > 2 )
	    return QTitleBar::tr("Press");
	break;
    case Description:
	switch ( control ) {
	case 1:
	    return QTitleBar::tr("Contains commands to manipulate the window");
	case 3:
	    if ( titleBar()->window()->isMinimized() )
		return QTitleBar::tr("Puts a minimized back to normal");
	    return QTitleBar::tr("Moves the window out of the way");
	case 4:
	    if ( titleBar()->window()->isMaximized() )
		return QTitleBar::tr("Puts a maximized window back to normal");
	    return QTitleBar::tr("Makes the window full screen");
	case 5:
	    return QTitleBar::tr("Closes the window");
	default:
	    return QTitleBar::tr("Displays the name of the window and contains controls to manipulate it");
	}
    default:
	break;
    }
    return str;
}

/*! \reimp */
QAccessible::Role QAccessibleTitleBar::role( int control ) const
{
    switch ( control )
    {
    case 1:
    case 3:
    case 4:
    case 5:
	return PushButton;
    default:
	return TitleBar;
    }
}

/*! \reimp */
QAccessible::State QAccessibleTitleBar::state( int control ) const
{
    return QAccessibleWidget::state( control );
}

/*! \reimp */
bool QAccessibleTitleBar::doDefaultAction( int control )
{
    switch ( control ) {
    case 3:
	if ( titleBar()->window()->isMinimized() )
	    titleBar()->window()->showNormal();
	else
	    titleBar()->window()->showMinimized();
	return TRUE;
    case 4:
	if ( titleBar()->window()->isMaximized() )
	    titleBar()->window()->showNormal();
	else
	    titleBar()->window()->showMaximized();
	return TRUE;
    case 5:
	titleBar()->window()->close();
	return TRUE;
    default:
	break;
    }
    return FALSE;
}


/*!
  \class QAccessibleViewport qaccessiblewidget.h
  \brief The QAccessibleViewport class hides the viewport of scrollviews for accessibility.
  \internal
*/

QAccessibleViewport::QAccessibleViewport( QObject *o, QObject *sv )
    : QAccessibleWidget( o )
{
    Q_ASSERT( sv->inherits("QScrollView") );
    scrollview = (QScrollView*)sv;
}

QAccessibleScrollView *QAccessibleViewport::scrollView() const
{
    QAccessibleInterface *iface = 0;
    queryAccessibleInterface( scrollview, &iface );
    Q_ASSERT(iface);
    return (QAccessibleScrollView *)iface;
}

int QAccessibleViewport::controlAt( int x, int y ) const
{
    int control = QAccessibleWidget::controlAt( x, y );
    if ( control > 0 )
	return control;

    QPoint p = widget()->mapFromGlobal( QPoint( x,y ) );
    return scrollView()->itemAt( p.x(), p.y() );
}

QRect QAccessibleViewport::rect( int control ) const
{
    if ( !control )
	return QAccessibleWidget::rect( control );
    QRect rect = scrollView()->itemRect( control );
    QPoint tl = widget()->mapToGlobal( QPoint( 0,0 ) );
    return QRect( tl.x() + rect.x(), tl.y() + rect.y(), rect.width(), rect.height() );
}

int QAccessibleViewport::navigate( NavDirection direction, int startControl ) const
{
    if ( direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl )
	return QAccessibleWidget::navigate( direction, startControl );

    // ### call itemUp/Down etc. here
    const int items = scrollView()->itemCount();
    switch( direction ) {
    case NavFirstChild:
	return 1;
    case NavLastChild:
	return items;
    case NavNext:
    case NavDown:
	return startControl + 1 > items ? -1 : startControl + 1;
    case NavPrevious:
    case NavUp:
	return startControl - 1 < 1 ? -1 : startControl - 1;
    default:
	break;
    }

    return -1;
}

int QAccessibleViewport::childCount() const
{
    int widgets = QAccessibleWidget::childCount();
    return widgets ? widgets : scrollView()->itemCount();
}

QString QAccessibleViewport::text( Text t, int control ) const
{
    return scrollView()->text( t, control );
}

bool QAccessibleViewport::doDefaultAction( int control )
{
    return scrollView()->doDefaultAction( control );
}

QAccessible::Role QAccessibleViewport::role( int control ) const
{
    return scrollView()->role( control );
}

QAccessible::State QAccessibleViewport::state( int control ) const
{
    return scrollView()->state( control );
}

bool QAccessibleViewport::setFocus( int control )
{
    return scrollView()->setFocus( control );
}

bool QAccessibleViewport::setSelected( int control, bool on, bool extend )
{
    return scrollView()->setSelected( control, on, extend );
}

void QAccessibleViewport::clearSelection()
{
    scrollView()->clearSelection();
}

QMemArray<int> QAccessibleViewport::selection() const
{
    return scrollView()->selection();
}

/*!
  \class QAccessibleScrollView qaccessiblewidget.h
  \brief The QAccessibleScrollView class implements the QAccessibleInterface for scrolled widgets.
*/

/*!
  Constructs a QAccessibleScrollView object for \a o.
  \a role, \a description, \a value, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleScrollView::QAccessibleScrollView( QObject *o, Role role, QString name,
    QString description, QString value, QString help, QString defAction, QString accelerator )
    : QAccessibleWidget( o, role, name, description, value, help, defAction, accelerator )
{
}

/*! \reimp */
QString QAccessibleScrollView::text( Text t, int control ) const
{
    QString str = QAccessibleWidget::text( t, control );
    if ( !!str )
	return str;
    switch ( t ) {
    case Name:
	return buddyString( widget() );
    default:
	break;
    }

    return str;
}

/*!
  Returns the ID of the item at viewport position \a x, \a y.
*/
int QAccessibleScrollView::itemAt( int /*x*/, int /*y*/ ) const
{
    return 0;
}

/*!
  Returns the location of the item with ID \a item in viewport coordinates.
*/
QRect QAccessibleScrollView::itemRect( int /*item*/ ) const
{
    return QRect();
}

/*!
  Returns the number of items.
*/
int QAccessibleScrollView::itemCount() const
{
    return 0;
}

/*!
  \class QAccessibleListBox qaccessiblewidget.h
  \brief The QAccessibleListBox class implements the QAccessibleInterface for list boxes.
*/

/*!
  Constructs a QAccessibleListBox object for \a o.
*/
QAccessibleListBox::QAccessibleListBox( QObject *o )
    : QAccessibleScrollView( o, List )
{
    Q_ASSERT(widget()->inherits("QListBox"));
}

/*! Returns the list box. */
QListBox *QAccessibleListBox::listBox() const
{
    return (QListBox*)widget();
}

/*! \reimp */
int QAccessibleListBox::itemAt( int x, int y ) const
{
    QListBoxItem *item = listBox()->itemAt( QPoint( x, y ) );
    return listBox()->index( item ) + 1;
}

/*! \reimp */
QRect QAccessibleListBox::itemRect( int item ) const
{
    return listBox()->itemRect( listBox()->item( item-1 ) );
}

/*! \reimp */
int QAccessibleListBox::itemCount() const
{
    return listBox()->count();
}

/*! \reimp */
QString QAccessibleListBox::text( Text t, int control ) const
{
    if ( !control || t != Name )
	return QAccessibleScrollView::text( t, control );

    QListBoxItem *item = listBox()->item( control - 1 );
    if ( item )
	return item->text();
    return QString::null;
}

/*! \reimp */
QAccessible::Role QAccessibleListBox::role( int control ) const
{
    if ( !control )
	return QAccessibleScrollView::role( control );
    return ListItem;
}

/*! \reimp */
QAccessible::State QAccessibleListBox::state( int control ) const
{
    int state = QAccessibleScrollView::state( control );
    QListBoxItem *item;
    if ( !control || !( item = listBox()->item( control - 1 ) ) )
	return (State)state;

    if ( item->isSelectable() ) {
	if ( listBox()->selectionMode() == QListBox::Multi )
	    state |= MultiSelectable;
	else if ( listBox()->selectionMode() == QListBox::Extended )
	    state |= ExtSelectable;
	else if ( listBox()->selectionMode() == QListBox::Single )
	    state |= Selectable;
	if ( item->isSelected() )
	    state |= Selected;
    }
    if ( listBox()->focusPolicy() != QWidget::NoFocus ) {
	state |= Focusable;
	if ( item->isCurrent() )
	    state |= Focused;
    }
    if ( !listBox()->itemVisible( item ) )
	state |= Invisible;

    return (State)state;
}

/*! \reimp */
bool QAccessibleListBox::setFocus( int control )
{
    bool res = QAccessibleScrollView::setFocus( 0 );
    if ( !control || !res )
	return res;

    QListBoxItem *item = listBox()->item( control -1 );
    if ( !item )
	return FALSE;
    listBox()->setCurrentItem( item );
    return TRUE;
}

/*! \reimp */
bool QAccessibleListBox::setSelected( int control, bool on, bool extend )
{
    if ( !control || ( extend &&
	listBox()->selectionMode() != QListBox::Extended &&
	listBox()->selectionMode() != QListBox::Multi ) )
	return FALSE;

    QListBoxItem *item = listBox()->item( control -1 );
    if ( !item )
	return FALSE;
    if ( !extend ) {
	listBox()->setSelected( item, on );
    } else {
	int current = listBox()->currentItem();
	bool down = control > current;
	for ( int i = current; i != control;) {
	    down ? i++ : i--;
	    listBox()->setSelected( i, on );
	}

    }
    return TRUE;
}

/*! \reimp */
void QAccessibleListBox::clearSelection()
{
    listBox()->clearSelection();
}

/*! \reimp */
QMemArray<int> QAccessibleListBox::selection() const
{
    QMemArray<int> array;
    uint size = 0;
    const uint c = listBox()->count();
    array.resize( c );
    for ( uint i = 0; i < c; ++i ) {
	if ( listBox()->isSelected( i ) ) {
	    ++size;
	    array[ (int)size-1 ] = i+1;
	}
    }
    array.resize( size );
    return array;
}

/*!
  \class QAccessibleListView qaccessiblewidget.h
  \brief The QAccessibleListView class implements the QAccessibleInterface for list views.
*/

static QListViewItem *findLVItem( QListView* listView, int control )
{
    int id = 1;
    QListViewItemIterator it( listView );
    QListViewItem *item = it.current();
    while ( item && id < control ) {
	++it;
	++id;
	item = it.current();
    }
    return item;
}

/*!
  Constructs a QAccessibleListView object for \a o.
*/
QAccessibleListView::QAccessibleListView( QObject *o )
    : QAccessibleScrollView( o, Outline )
{
}

/*! Returns the list view. */
QListView *QAccessibleListView::listView() const
{
    Q_ASSERT(widget()->inherits("QListView"));
    return (QListView*)widget();
}

/*! \reimp */
int QAccessibleListView::itemAt( int x, int y ) const
{
    QListViewItem *item = listView()->itemAt( QPoint( x, y ) );
    if ( !item )
	return 0;

    QListViewItemIterator it( listView() );
    int c = 1;
    while ( it.current() ) {
	if ( it.current() == item )
	    return c;
	++c;
	++it;
    }
    return 0;
}

/*! \reimp */
QRect QAccessibleListView::itemRect( int control ) const
{
    QListViewItem *item = findLVItem( listView(), control );
    if ( !item )
	return QRect();
    return listView()->itemRect( item );
}

/*! \reimp */
int QAccessibleListView::itemCount() const
{
    QListViewItemIterator it( listView() );
    int c = 0;
    while ( it.current() ) {
	++c;
	++it;
    }

    return c;
}

/*! \reimp */
QString QAccessibleListView::text( Text t, int control ) const
{
    if ( !control || t != Name )
	return QAccessibleScrollView::text( t, control );

    QListViewItem *item = findLVItem( listView(), control );
    if ( !item )
	return QString::null;
    return item->text( 0 );
}

/*! \reimp */
QAccessible::Role QAccessibleListView::role( int control ) const
{
    if ( !control )
	return QAccessibleScrollView::role( control );
    return OutlineItem;
}

/*! \reimp */
QAccessible::State QAccessibleListView::state( int control ) const
{
    int state = QAccessibleScrollView::state( control );
    QListViewItem *item;
    if ( !control || !( item = findLVItem( listView(), control ) ) )
	return (State)state;

    if ( item->isSelectable() ) {
	if ( listView()->selectionMode() == QListView::Multi )
	    state |= MultiSelectable;
	else if ( listView()->selectionMode() == QListView::Extended )
	    state |= ExtSelectable;
	else if ( listView()->selectionMode() == QListView::Single )
	    state |= Selectable;
	if ( item->isSelected() )
	    state |= Selected;
    }
    if ( listView()->focusPolicy() != QWidget::NoFocus ) {
	state |= Focusable;
	if ( item == listView()->currentItem() )
	    state |= Focused;
    }
    if ( item->childCount() ) {
	if ( item->isOpen() )
	    state |= Expanded;
	else
	    state |= Collapsed;
    }
    if ( !listView()->itemRect( item ).isValid() )
	state |= Invisible;

    if ( item->rtti() == QCheckListItem::RTTI ) {
	if ( ((QCheckListItem*)item)->isOn() )
	    state|=Checked;
    }
    return (State)state;
}

/*! \reimp
QAccessibleInterface *QAccessibleListView::focusChild( int *control ) const
{
    QListViewItem *item = listView()->currentItem();
    if ( !item )
	return 0;

    QListViewItemIterator it( listView() );
    int c = 1;
    while ( it.current() ) {
	if ( it.current() == item ) {
	    *control = c;
	    return (QAccessibleInterface*)this;
	}
	++c;
	++it;
    }
    return 0;
}
*/
/*! \reimp */
bool QAccessibleListView::setFocus( int control )
{
    bool res = QAccessibleScrollView::setFocus( 0 );
    if ( !control || !res )
	return res;

    QListViewItem *item = findLVItem( listView(), control );
    if ( !item )
	return FALSE;
    listView()->setCurrentItem( item );
    return TRUE;
}

/*! \reimp */
bool QAccessibleListView::setSelected( int control, bool on, bool extend )
{
    if ( !control || ( extend &&
	listView()->selectionMode() != QListView::Extended &&
	listView()->selectionMode() != QListView::Multi ) )
	return FALSE;

    QListViewItem *item = findLVItem( listView(), control );
    if ( !item )
	return FALSE;
    if ( !extend ) {
	listView()->setSelected( item, on );
    } else {
	QListViewItem *current = listView()->currentItem();
	if ( !current )
	    return FALSE;
	bool down = item->itemPos() > current->itemPos();
	QListViewItemIterator it( current );
	while ( it.current() ) {
	    listView()->setSelected( it.current(), on );
	    if ( it.current() == item )
		break;
	    if ( down )
		++it;
	    else
		--it;
	}
    }
    return TRUE;
}

/*! \reimp */
void QAccessibleListView::clearSelection()
{
    listView()->clearSelection();
}

/*! \reimp */
QMemArray<int> QAccessibleListView::selection() const
{
    QMemArray<int> array;
    uint size = 0;
    int id = 1;
    array.resize( size );
    QListViewItemIterator it( listView() );
    while ( it.current() ) {
	if ( it.current()->isSelected() ) {
	    ++size;
	    array.resize( size );
	    array[ (int)size-1 ] = id;
	}
	++it;
	++id;
    }
    return array;
}

#ifndef QT_NO_ICONVIEW
/*!
  \class QAccessibleIconView qaccessiblewidget.h
  \brief The QAccessibleIconView class implements the QAccessibleInterface for icon views.
*/

static QIconViewItem *findIVItem( QIconView *iconView, int control )
{
    int id = 1;
    QIconViewItem *item = iconView->firstItem();
    while ( item && id < control ) {
	item = item->nextItem();
	++id;
    }

    return item;
}

/*!
  Constructs a QAccessibleIconView object for \a o.
*/
QAccessibleIconView::QAccessibleIconView( QObject *o )
    : QAccessibleScrollView( o, Outline )
{
    Q_ASSERT(widget()->inherits("QIconView"));
}

/*! Returns the icon view. */
QIconView *QAccessibleIconView::iconView() const
{
    return (QIconView*)widget();
}

/*! \reimp */
int QAccessibleIconView::itemAt( int x, int y ) const
{
    QIconViewItem *item = iconView()->findItem( QPoint( x, y ) );
    return iconView()->index( item ) + 1;
}

/*! \reimp */
QRect QAccessibleIconView::itemRect( int control ) const
{
    QIconViewItem *item = findIVItem( iconView(), control );

    if ( !item )
	return QRect();
    return item->rect();
}

/*! \reimp */
int QAccessibleIconView::itemCount() const
{
    return iconView()->count();
}

/*! \reimp */
QString QAccessibleIconView::text( Text t, int control ) const
{
    if ( !control || t != Name )
	return QAccessibleScrollView::text( t, control );

    QIconViewItem *item = findIVItem( iconView(), control );
    if ( !item )
	return QString::null;
    return item->text();
}

/*! \reimp */
QAccessible::Role QAccessibleIconView::role( int control ) const
{
    if ( !control )
	return QAccessibleScrollView::role( control );
    return OutlineItem;
}

/*! \reimp */
QAccessible::State QAccessibleIconView::state( int control ) const
{
    int state = QAccessibleScrollView::state( control );
    QIconViewItem *item;
    if ( !control || !( item = findIVItem( iconView(), control ) ) )
	return (State)state;

    if ( item->isSelectable() ) {
	if ( iconView()->selectionMode() == QIconView::Multi )
	    state |= MultiSelectable;
	else if ( iconView()->selectionMode() == QIconView::Extended )
	    state |= ExtSelectable;
	else if ( iconView()->selectionMode() == QIconView::Single )
	    state |= Selectable;
	if ( item->isSelected() )
	    state |= Selected;
    }
    if ( iconView()->itemsMovable() )
	state |= Moveable;
    if ( iconView()->focusPolicy() != QWidget::NoFocus ) {
	state |= Focusable;
	if ( item == iconView()->currentItem() )
	    state |= Focused;
    }

    return (State)state;
}

/*! \reimp
QAccessibleInterface *QAccessibleIconView::focusChild( int *control ) const
{
    QIconViewItem *item = iconView()->currentItem();
    if ( !item )
	return 0;

    *control = iconView()->index( item );
    return (QAccessibleInterface*)this;
}
*/
/*! \reimp */
bool QAccessibleIconView::setFocus( int control )
{
    bool res = QAccessibleScrollView::setFocus( 0 );
    if ( !control || !res )
	return res;

    QIconViewItem *item = findIVItem( iconView(), control );
    if ( !item )
	return FALSE;
    iconView()->setCurrentItem( item );
    return TRUE;
}

/*! \reimp */
bool QAccessibleIconView::setSelected( int control, bool on, bool extend  )
{
    if ( !control || ( extend &&
	iconView()->selectionMode() != QIconView::Extended &&
	iconView()->selectionMode() != QIconView::Multi ) )
	return FALSE;

    QIconViewItem *item = findIVItem( iconView(), control );
    if ( !item )
	return FALSE;
    if ( !extend ) {
	iconView()->setSelected( item, on, TRUE );
    } else {
	QIconViewItem *current = iconView()->currentItem();
	if ( !current )
	    return FALSE;
	bool down = FALSE;
	QIconViewItem *temp = current;
	while ( ( temp = temp->nextItem() ) ) {
	    if ( temp == item ) {
		down = TRUE;
		break;
	    }
	}
	temp = current;
	if ( down ) {
	    while ( ( temp = temp->nextItem() ) ) {
		iconView()->setSelected( temp, on, TRUE );
		if ( temp == item )
		    break;
	    }
	} else {
	    while ( ( temp = temp->prevItem() ) ) {
		iconView()->setSelected( temp, on, TRUE );
		if ( temp == item )
		    break;
	    }
	}
    }
    return TRUE;
}

/*! \reimp */
void QAccessibleIconView::clearSelection()
{
    iconView()->clearSelection();
}

/*! \reimp */
QMemArray<int> QAccessibleIconView::selection() const
{
    QMemArray<int> array;
    uint size = 0;
    int id = 1;
    array.resize( iconView()->count() );
    QIconViewItem *item = iconView()->firstItem();
    while ( item ) {
	if ( item->isSelected() ) {
	    ++size;
	    array[ (int)size-1 ] = id;
	}
	item = item->nextItem();
	++id;
    }
    array.resize( size );
    return array;
}
#endif


/*!
  \class QAccessibleTextEdit qaccessiblewidget.h
  \brief The QAccessibleTextEdit class implements the QAccessibleInterface for richtext editors.
*/

/*!
  Constructs a QAccessibleTextEdit object for \a o.
*/
QAccessibleTextEdit::QAccessibleTextEdit( QObject *o )
: QAccessibleScrollView( o, Pane )
{
    Q_ASSERT(widget()->inherits("QTextEdit"));
}

/*! Returns the text edit. */
QTextEdit *QAccessibleTextEdit::textEdit() const
{

    return (QTextEdit*)widget();
}

/*! \reimp */
int QAccessibleTextEdit::itemAt( int x, int y ) const
{
    int p;
    QPoint cp = textEdit()->viewportToContents( QPoint( x,y ) );
    textEdit()->charAt( cp , &p );
    return p + 1;
}

/*! \reimp */
QRect QAccessibleTextEdit::itemRect( int item ) const
{
    QRect rect = textEdit()->paragraphRect( item - 1 );
    if ( !rect.isValid() )
	return QRect();
    QPoint ntl = textEdit()->contentsToViewport( QPoint( rect.x(), rect.y() ) );
    return QRect( ntl.x(), ntl.y(), rect.width(), rect.height() );
}

/*! \reimp */
int QAccessibleTextEdit::itemCount() const
{
    return textEdit()->paragraphs();
}

/*! \reimp */
QString QAccessibleTextEdit::text( Text t, int control ) const
{
    if (t == Name) {
        if (control)
            return textEdit()->text(control - 1);
        else
            return stripAmp(buddyString(widget()));
    } else if (t == Value) {
        if (control)
            return textEdit()->text(control - 1);
        else
            return textEdit()->text();
    }

    return QAccessibleScrollView::text( t, control );
}

/*! \reimp */
void QAccessibleTextEdit::setText(Text t, int control, const QString &text)
{
    if (control || t != Value) {
        QAccessibleScrollView::setText(t, control, text);
        return;
    }
    textEdit()->setText(text);
}

/*! \reimp */
QAccessible::Role QAccessibleTextEdit::role( int control ) const
{
    if ( control )
	return EditableText;
    return QAccessibleScrollView::role( control );
}
