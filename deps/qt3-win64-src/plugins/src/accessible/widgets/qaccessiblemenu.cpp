#include "qaccessiblemenu.h"

#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qstyle.h>

class MyPopupMenu : public QPopupMenu
{
    friend class QAccessiblePopup;
};

QAccessiblePopup::QAccessiblePopup( QObject *o )
: QAccessibleWidget( o )
{
    Q_ASSERT( o->inherits("QPopupMenu") );
}

QPopupMenu *QAccessiblePopup::popupMenu() const
{
    return (QPopupMenu*)object();
}

int QAccessiblePopup::childCount() const
{
    return popupMenu()->count();
}

QRESULT QAccessiblePopup::queryChild( int /*control*/, QAccessibleInterface **iface ) const
{
    *iface = 0;
    return QS_FALSE;
}

QRect QAccessiblePopup::rect( int control ) const
{
    if ( !control )
	return QAccessibleWidget::rect( control );

    QRect r = popupMenu()->itemGeometry( control - 1 );
    QPoint tlp = popupMenu()->mapToGlobal( QPoint( 0,0 ) );

    return QRect( tlp.x() + r.x(), tlp.y() + r.y(), r.width(), r.height() );
}

int QAccessiblePopup::controlAt( int x, int y ) const
{
    QPoint p = popupMenu()->mapFromGlobal( QPoint( x,y ) );
    MyPopupMenu *mp = (MyPopupMenu*)popupMenu();
    return mp->itemAtPos( p, FALSE ) + 1;
}

int QAccessiblePopup::navigate( NavDirection direction, int startControl ) const
{
    if ( direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl )
	return QAccessibleWidget::navigate( direction, startControl );

    switch ( direction ) {
    case NavFirstChild:
	return 1;
    case NavLastChild:
	return childCount();
    case NavNext:
    case NavDown:
	return (startControl + 1) > childCount() ? -1 : startControl + 1;
    case NavPrevious:
    case NavUp:
	return (startControl - 1) < 1 ? -1 : startControl - 1;
    default:
	break;
    }
    return -1;
}

QString QAccessiblePopup::text( Text t, int control ) const
{
    QString tx = QAccessibleWidget::text( t, control );
    if ( !!tx )
	return tx;

    int id;
    QMenuItem *item = 0;
    if ( control ) {
	id = popupMenu()->idAt( control - 1 );
	item = popupMenu()->findItem( id );
    }

    switch ( t ) {
    case Name:
	if ( !control )
	    return popupMenu()->caption();
	return stripAmp( popupMenu()->text( id ) );
    case Help:
	return popupMenu()->whatsThis( id );
    case Accelerator:
	return hotKey( popupMenu()->text( id ) );
    case DefaultAction:
	if ( !item || item->isSeparator() || !item->isEnabled() )
	    break;
	if ( item->popup() )
	    return QPopupMenu::tr("Open");
	return QPopupMenu::tr("Execute");
    default:
	break;
    }
    return tx;
}

QAccessible::Role QAccessiblePopup::role( int control ) const
{
    if ( !control )
	return PopupMenu;
    
    QMenuItem *item = popupMenu()->findItem( popupMenu()->idAt( control -1 ) );
    if ( item && item->isSeparator() )
	return Separator;
    return MenuItem;
}

QAccessible::State QAccessiblePopup::state( int control ) const
{
    int s = QAccessibleWidget::state( control );
    if ( !control )
	return (State)s;

    int id = popupMenu()->idAt( control -1 );
    QMenuItem *item = popupMenu()->findItem( id );
    if ( !item )
	return (State)s;

    if ( popupMenu()->style().styleHint( QStyle::SH_PopupMenu_MouseTracking ) )
	s |= HotTracked;
    if ( item->isSeparator() || !item->isEnabled() )
	s |= Unavailable;
    if ( popupMenu()->isCheckable() && item->isChecked() )
	s |= Checked;
    if ( popupMenu()->isItemActive( id ) )
	s |= Focused;

    return (State)s;
}

bool QAccessiblePopup::doDefaultAction( int control )
{
    if ( !control )
	return FALSE;

    int id = popupMenu()->idAt( control -1 );
    QMenuItem *item = popupMenu()->findItem( id );
    if ( !item || !item->isEnabled() )
	return FALSE;

    popupMenu()->activateItemAt( control - 1);
    return TRUE;
}

bool QAccessiblePopup::setFocus( int control )
{
    if ( !control )
	return FALSE;

    int id = popupMenu()->idAt( control -1 );
    QMenuItem *item = popupMenu()->findItem( id );
    if ( !item || !item->isEnabled() )
	return FALSE;

    popupMenu()->setActiveItem( control - 1 );
    return TRUE;
}


class MyMenuBar : public QMenuBar
{
    friend class QAccessibleMenuBar;
};

QAccessibleMenuBar::QAccessibleMenuBar( QObject *o )
: QAccessibleWidget( o )
{
    Q_ASSERT( o->inherits( "QMenuBar" ) );
}

QMenuBar *QAccessibleMenuBar::menuBar() const
{
    return (QMenuBar*)object();
}

int QAccessibleMenuBar::childCount() const
{
    return menuBar()->count();
}

QRESULT QAccessibleMenuBar::queryChild( int /*control*/, QAccessibleInterface **iface ) const
{
    *iface = 0;
    return QS_FALSE;
}

QRect QAccessibleMenuBar::rect( int control ) const
{
    if ( !control )
	return QAccessibleWidget::rect( control );

    MyMenuBar *mb = (MyMenuBar*)menuBar();
    QRect r = mb->itemRect( control - 1 );
    QPoint tlp = mb->mapToGlobal( QPoint( 0,0 ) );

    return QRect( tlp.x() + r.x(), tlp.y() + r.y(), r.width(), r.height() );
}

int QAccessibleMenuBar::controlAt( int x, int y ) const
{
    MyMenuBar *mb = (MyMenuBar*)menuBar();
    QPoint p = mb->mapFromGlobal( QPoint( x,y ) );
    return mb->itemAtPos( p ) + 1;
}

int QAccessibleMenuBar::navigate( NavDirection direction, int startControl ) const
{
    if ( direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl )
	return QAccessibleWidget::navigate( direction, startControl );

    switch ( direction ) {
    case NavFirstChild:
	return 1;
    case NavLastChild:
	return childCount();
    case NavNext:
    case NavRight:
	return (startControl + 1) > childCount() ? -1 : startControl + 1;
    case NavPrevious:
    case NavLeft:
	return (startControl - 1) < 1 ? -1 : startControl - 1;
    default:
	break;
    }

    return -1;
}

QString QAccessibleMenuBar::text( Text t, int control ) const
{
    QString tx = QAccessibleWidget::text( t, control );
    if ( !!tx )
	return tx;
    if ( !control )
	return tx;

    int id = menuBar()->idAt( control - 1 );
    switch ( t ) {
    case Name:
	return stripAmp( menuBar()->text( id ) );
    case Accelerator:
	tx = hotKey( menuBar()->text( id ) );
	if ( !!tx )
	    return "Alt + "+tx;
	break;
    case DefaultAction:
	return QMenuBar::tr("Open");
    default:
	break;
    }

    return tx;
}

QAccessible::Role QAccessibleMenuBar::role( int control ) const
{
    if ( !control )
	return MenuBar;

    QMenuItem *item = menuBar()->findItem( menuBar()->idAt( control -1 ) );
    if ( item && item->isSeparator() )
	return Separator;
    return MenuItem;
}

QAccessible::State QAccessibleMenuBar::state( int control ) const
{
    int s = QAccessibleWidget::state( control );
    if ( !control )
	return (State)s;

    int id = menuBar()->idAt( control -1 );
    QMenuItem *item = menuBar()->findItem( id );
    if ( !item )
	return (State)s;

    if ( menuBar()->style().styleHint( QStyle::SH_PopupMenu_MouseTracking ) )
	s |= HotTracked;
    if ( item->isSeparator() || !item->isEnabled() )
	s |= Unavailable;
    if ( menuBar()->isItemActive( id ) )
	s |= Focused;

    return (State)s;
}

bool QAccessibleMenuBar::doDefaultAction( int control )
{
    if ( !control )
	return FALSE;

    int id = menuBar()->idAt( control -1 );
    QMenuItem *item = menuBar()->findItem( id );
    if ( !item || !item->isEnabled() )
	return FALSE;

    menuBar()->activateItemAt( control - 1);
    return TRUE;
}

bool QAccessibleMenuBar::setFocus( int control )
{
    if ( !control )
	return FALSE;

    int id = menuBar()->idAt( control -1 );
    QMenuItem *item = menuBar()->findItem( id );
    if ( !item || !item->isEnabled() )
	return FALSE;

    return TRUE;
}
