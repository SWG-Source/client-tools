/****************************************************************************
** $Id: qaction.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QAction class
**
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
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

#include "qaction.h"

#ifndef QT_NO_ACTION

#include "qtoolbar.h"
#include "qptrlist.h"
#include "qpopupmenu.h"
#include "qaccel.h"
#include "qtoolbutton.h"
#include "qcombobox.h"
#include "qtooltip.h"
#include "qwhatsthis.h"
#include "qstatusbar.h"
#include "qobjectlist.h"


/*!
    \class QAction qaction.h
    \brief The QAction class provides an abstract user interface
    action that can appear both in menus and tool bars.

    \ingroup basic
    \ingroup application
    \mainclass

    In GUI applications many commands can be invoked via a menu
    option, a toolbar button and a keyboard accelerator. Since the
    same action must be performed regardless of how the action was
    invoked, and since the menu and toolbar should be kept in sync, it
    is useful to represent a command as an \e action. An action can be
    added to a menu and a toolbar and will automatically keep them in
    sync. For example, if the user presses a Bold toolbar button the
    Bold menu item will automatically be checked.

    A QAction may contain an icon, a menu text, an accelerator, a
    status text, a whats this text and a tool tip. Most of these can
    be set in the constructor. They can also be set independently with
    setIconSet(), setText(), setMenuText(), setToolTip(),
    setStatusTip(), setWhatsThis() and setAccel().

    An action may be a toggle action e.g. a Bold toolbar button, or a
    command action, e.g. 'Open File' to invoke an open file dialog.
    Toggle actions emit the toggled() signal when their state changes.
    Both command and toggle actions emit the activated() signal when
    they are invoked. Use setToggleAction() to set an action's toggled
    status. To see if an action is a toggle action use
    isToggleAction(). A toggle action may be "on", isOn() returns
    TRUE, or "off", isOn() returns FALSE.

    Actions are added to widgets (menus or toolbars) using addTo(),
    and removed using removeFrom().

    Once a QAction has been created it should be added to the relevant
    menu and toolbar and then connected to the slot which will perform
    the action. For example:

    \quotefile action/application.cpp
    \skipto QPixmap( fileopen
    \printuntil connect

    We create a "File Save" action with a menu text of "&Save" and
    \e{Ctrl+S} as the keyboard accelerator. We connect the
    fileSaveAction's activated() signal to our own save() slot. Note
    that at this point there is no menu or toolbar action, we'll add
    them next:

    \skipto new QToolBar
    \printline
    \skipto fileSaveAction->addTo
    \printline
    \skipto new QPopupMenu
    \printuntil insertItem
    \skipto fileSaveAction->addTo
    \printline

    We create a toolbar and add our fileSaveAction to it. Similarly we
    create a menu, add a top-level menu item, and add our
    fileSaveAction.

    We recommend that actions are created as children of the window
    that they are used in. In most cases actions will be children of
    the application's main window.

    To prevent recursion, don't create an action as a child of a
    widget that the action is later added to.
*/

class QActionPrivate
{
public:
    QActionPrivate(QAction *act);
    ~QActionPrivate();
    QIconSet *iconset;
    QString text;
    QString menutext;
    QString tooltip;
    QString statustip;
    QString whatsthis;
#ifndef QT_NO_ACCEL
    QKeySequence key;
    QAccel* accel;
    int accelid;
#endif
    uint enabled : 1;
    uint visible : 1;
    uint toggleaction : 1;
    uint on : 1;
    uint forceDisabled : 1;
    uint forceInvisible : 1;
#ifndef QT_NO_TOOLTIP
    QToolTipGroup tipGroup;
#endif
    QActionGroupPrivate* d_group;
    QAction *action;

    struct MenuItem {
	MenuItem():popup(0),id(0){}
	QPopupMenu* popup;
	int id;
    };
    // ComboItem is only necessary for actions that are
    // in dropdown/exclusive actiongroups. The actiongroup
    // will clean this up
    struct ComboItem {
	ComboItem():combo(0), id(0) {}
	QComboBox *combo;
	int id;
    };
    QPtrList<MenuItem> menuitems;
    QPtrList<QToolButton> toolbuttons;
    QPtrList<ComboItem> comboitems;

    enum Update { Icons = 1, Visibility = 2, State = 4, EverythingElse = 8 };
    void update( uint upd = EverythingElse );

    QString menuText() const;
    QString toolTip() const;
    QString statusTip() const;
};

QActionPrivate::QActionPrivate(QAction *act)
    : iconset( 0 ),
#ifndef QT_NO_ACCEL
      key( 0 ), accel( 0 ), accelid( 0 ),
#endif
      enabled( TRUE ), visible( TRUE ), toggleaction( FALSE ), on( FALSE ),
      forceDisabled( FALSE ), forceInvisible( FALSE ),
#ifndef QT_NO_TOOLTIP
      tipGroup( 0 ),
#endif
      d_group( 0 ), action(act)
{
    menuitems.setAutoDelete( TRUE );
    comboitems.setAutoDelete( TRUE );
#ifndef QT_NO_TOOLTIP
    tipGroup.setDelay( FALSE );
#endif
}

QActionPrivate::~QActionPrivate()
{
    QPtrListIterator<QToolButton> ittb( toolbuttons );
    QToolButton *tb;

    while ( ( tb = ittb.current() ) ) {
	++ittb;
	delete tb;
    }

    QPtrListIterator<QActionPrivate::MenuItem> itmi( menuitems);
    QActionPrivate::MenuItem* mi;
    while ( ( mi = itmi.current() ) ) {
	++itmi;
	QPopupMenu* menu = mi->popup;
	if ( menu->findItem( mi->id ) )
	    menu->removeItem( mi->id );
    }

    QPtrListIterator<QActionPrivate::ComboItem> itci(comboitems);
    QActionPrivate::ComboItem* ci;
    while ( ( ci = itci.current() ) ) {
	++itci;
	QComboBox* combo = ci->combo;
	combo->clear();
	QActionGroup *group = ::qt_cast<QActionGroup*>(action->parent());
	QObjectList *siblings = group ? group->queryList("QAction") : 0;
	if (siblings) {
	    QObjectListIt it(*siblings);
	    while (it.current()) {
		QAction *sib = ::qt_cast<QAction*>(it.current());
		++it;
		sib->removeFrom(combo);
	    }
	    it = QObjectListIt(*siblings);
	    while (it.current()) {
		QAction *sib = ::qt_cast<QAction*>(it.current());
		++it;
		if (sib == action)
		    continue;
		sib->addTo(combo);
	    }
	}
	delete siblings;
    }

#ifndef QT_NO_ACCEL
    delete accel;
#endif
    delete iconset;
}

class QActionGroupPrivate
{
public:
    uint exclusive: 1;
    uint dropdown: 1;
    QPtrList<QAction> actions;
    QAction* selected;
    QAction* separatorAction;

    struct MenuItem {
	MenuItem():popup(0),id(0){}
	QPopupMenu* popup;
	int id;
    };

    QPtrList<QComboBox> comboboxes;
    QPtrList<QToolButton> menubuttons;
    QPtrList<MenuItem> menuitems;
    QPtrList<QPopupMenu> popupmenus;

    void update( const QActionGroup * );
};

void QActionPrivate::update( uint upd )
{
    for ( QPtrListIterator<MenuItem> it( menuitems); it.current(); ++it ) {
	MenuItem* mi = it.current();
	QString t = menuText();
#ifndef QT_NO_ACCEL
	if ( key )
	    t += '\t' + QAccel::keyToString( key );
#endif
	if ( upd & State ) {
	    mi->popup->setItemEnabled( mi->id, enabled );
	    if ( toggleaction )
		mi->popup->setItemChecked( mi->id, on );
	}
	if ( upd & Visibility )
	    mi->popup->setItemVisible( mi->id, visible );

	if ( upd & Icons )
	    if ( iconset )
		mi->popup->changeItem( mi->id, *iconset, t );
	    else
		mi->popup->changeItem( mi->id, QIconSet(), t );
	if ( upd & EverythingElse ) {
	    mi->popup->changeItem( mi->id, t );
	    if ( !whatsthis.isEmpty() )
		mi->popup->setWhatsThis( mi->id, whatsthis );
	    if ( toggleaction ) {
		mi->popup->setCheckable( TRUE );
		mi->popup->setItemChecked( mi->id, on );
	    }
	}
    }
    for ( QPtrListIterator<QToolButton> it2(toolbuttons); it2.current(); ++it2 ) {
	QToolButton* btn = it2.current();
	if ( upd & State ) {
	    btn->setEnabled( enabled );
	    if ( toggleaction )
		btn->setOn( on );
	}
	if ( upd & Visibility )
	    visible ? btn->show() : btn->hide();
	if ( upd & Icons ) {
	    if ( iconset )
		btn->setIconSet( *iconset );
	    else
		btn->setIconSet( QIconSet() );
	}
	if ( upd & EverythingElse ) {
	    btn->setToggleButton( toggleaction );
	    if ( !text.isEmpty() )
		btn->setTextLabel( text, FALSE );
#ifndef QT_NO_TOOLTIP
	    QToolTip::remove( btn );
	    QToolTip::add( btn, toolTip(), &tipGroup, statusTip() );
#endif
#ifndef QT_NO_WHATSTHIS
	    QWhatsThis::remove( btn );
	    if ( !whatsthis.isEmpty() )
		QWhatsThis::add( btn, whatsthis );
#endif
	}
    }
#ifndef QT_NO_ACCEL
    if ( accel ) {
	accel->setEnabled( enabled && visible );
	if ( !whatsthis.isEmpty() )
	    accel->setWhatsThis( accelid, whatsthis );
    }
#endif
    // Only used by actiongroup
    for ( QPtrListIterator<ComboItem> it3( comboitems ); it3.current(); ++it3 ) {
	ComboItem *ci = it3.current();
	if ( !ci->combo )
	    return;
	if ( iconset )
	    ci->combo->changeItem( iconset->pixmap(), text, ci->id );
	else
	    ci->combo->changeItem( text, ci->id );
    }
}

QString QActionPrivate::menuText() const
{
    if ( menutext.isNull() ) {
	QString t(text);
	t.replace('&', "&&");
	return t;
    }
    return menutext;
}

QString QActionPrivate::toolTip() const
{
    if ( tooltip.isNull() ) {
#ifndef QT_NO_ACCEL
	if ( accel )
	    return text + " (" + QAccel::keyToString( accel->key( accelid )) + ")";
#endif
	return text;
    }
    return tooltip;
}

QString QActionPrivate::statusTip() const
{
    if ( statustip.isNull() )
	return toolTip();
    return statustip;
}

/*
  internal: guesses a descriptive text from a menu text
 */
static QString qt_stripMenuText( QString s )
{
    s.remove( QString::fromLatin1("...") );
    s.remove( QChar('&' ) );
    return s.stripWhiteSpace();
}

/*!
    Constructs an action called \a name with parent \a parent.

    If \a parent is a QActionGroup, the new action inserts itself into
    \a parent.

    For accelerators and status tips to work, \a parent must either be
    a widget, or an action group whose parent is a widget.

    \warning To prevent recursion, don't create an action as a child
    of a widget that the action is later added to.
*/
QAction::QAction( QObject* parent, const char* name )
    : QObject( parent, name )
{
    d = new QActionPrivate(this);
    init();
}

/*! \obsolete
    Constructs an action called \a name with parent \a parent.

    If \a toggle is TRUE the action will be a toggle action, otherwise
    it will be a command action.

    If \a parent is a QActionGroup, the new action inserts itself into
    \a parent.

    For accelerators and status tips to work, \a parent must either be
    a widget, or an action group whose parent is a widget.
*/
QAction::QAction( QObject* parent, const char* name, bool toggle )
    : QObject( parent, name )
{
    d = new QActionPrivate(this);
    d->toggleaction = toggle;
    init();
}


#ifndef QT_NO_ACCEL

/*!
    This constructor creates an action with the following properties:
    the icon or iconset \a icon, the menu text \a menuText and
    keyboard accelerator \a accel. It is a child of \a parent and
    called \a name.

    If \a parent is a QActionGroup, the action automatically becomes
    a member of it.

    For accelerators and status tips to work, \a parent must either be
    a widget, or an action group whose parent is a widget.

    The action uses a stripped version of \a menuText (e.g. "\&Menu
    Option..." becomes "Menu Option") as descriptive text for
    toolbuttons. You can override this by setting a specific
    description with setText(). The same text and \a accel will be
    used for tool tips and status tips unless you provide text for
    these using setToolTip() and setStatusTip().

    Call setToggleAction(TRUE) to make the action a toggle action.

    \warning To prevent recursion, don't create an action as a child
    of a widget that the action is later added to.
*/
QAction::QAction( const QIconSet& icon, const QString& menuText, QKeySequence accel,
		  QObject* parent, const char* name )
    : QObject( parent, name )
{
    d = new QActionPrivate(this);
    if ( !icon.isNull() )
	setIconSet( icon );
    d->text = qt_stripMenuText( menuText );
    d->menutext = menuText;
    setAccel( accel );
    init();
}

/*!
    This constructor results in an icon-less action with the the menu
    text \a menuText and keyboard accelerator \a accel. It is a child
    of \a parent and called \a name.

    If  \a parent is a QActionGroup, the action automatically becomes
    a member of it.

    For accelerators and status tips to work, \a parent must either be
    a widget, or an action group whose parent is a widget.

    The action uses a stripped version of \a menuText (e.g. "\&Menu
    Option..." becomes "Menu Option") as descriptive text for
    toolbuttons. You can override this by setting a specific
    description with setText(). The same text and \a accel will be
    used for tool tips and status tips unless you provide text for
    these using setToolTip() and setStatusTip().

    Call setToggleAction(TRUE) to make the action a toggle action.

    \warning To prevent recursion, don't create an action as a child
    of a widget that the action is later added to.
*/
QAction::QAction( const QString& menuText, QKeySequence accel,
		  QObject* parent, const char* name )
    : QObject( parent, name )
{
    d = new QActionPrivate(this);
    d->text = qt_stripMenuText( menuText );
    d->menutext = menuText;
    setAccel( accel );
    init();
}

/*! \obsolete
    This constructor creates an action with the following properties:
    the description \a text, the icon or iconset \a icon, the menu
    text \a menuText and keyboard accelerator \a accel. It is a child
    of \a parent and called \a name. If \a toggle is TRUE the action
    will be a toggle action, otherwise it will be a command action.

    If  \a parent is a QActionGroup, the action automatically becomes
    a member of it.

    For accelerators and status tips to work, \a parent must either be
    a widget, or an action group whose parent is a widget.

    The \a text and \a accel will be used for tool tips and status
    tips unless you provide specific text for these using setToolTip()
    and setStatusTip().
*/
QAction::QAction( const QString& text, const QIconSet& icon, const QString& menuText, QKeySequence accel, QObject* parent, const char* name, bool toggle )
    : QObject( parent, name )
{
    d = new QActionPrivate(this);
    d->toggleaction = toggle;
    if ( !icon.isNull() )
	setIconSet( icon );

    d->text = text;
    d->menutext = menuText;
    setAccel( accel );
    init();
}

/*! \obsolete
    This constructor results in an icon-less action with the
    description \a text, the menu text \a menuText and the keyboard
    accelerator \a accel. Its parent is \a parent and it is called \a
    name. If \a toggle is TRUE the action will be a toggle action,
    otherwise it will be a command action.

    The action automatically becomes a member of \a parent if \a
    parent is a QActionGroup.

    For accelerators and status tips to work, \a parent must either be
    a widget, or an action group whose parent is a widget.

    The \a text and \a accel will be used for tool tips and status
    tips unless you provide specific text for these using setToolTip()
    and setStatusTip().
*/
QAction::QAction( const QString& text, const QString& menuText, QKeySequence accel, QObject* parent, const char* name, bool toggle )
    : QObject( parent, name )
{
    d = new QActionPrivate(this);
    d->toggleaction = toggle;
    d->text = text;
    d->menutext = menuText;
    setAccel( accel );
    init();
}
#endif

/*!
  \internal
*/
void QAction::init()
{
    if ( ::qt_cast<QActionGroup*>(parent()) )
	((QActionGroup*) parent())->add( this );		// insert into action group
}

/*!
    Destroys the object and frees allocated resources.
*/

QAction::~QAction()
{
    delete d;
}

/*!
    \property QAction::iconSet
    \brief  the action's icon

    The icon is used as the tool button icon and in the menu to the
    left of the menu text. There is no default icon.

    If a null icon (QIconSet::isNull() is passed into this function,
    the icon of the action is cleared.

    (See the action/toggleaction/toggleaction.cpp example.)

*/
void QAction::setIconSet( const QIconSet& icon )
{
    register QIconSet *i = d->iconset;
    if ( !icon.isNull() )
	d->iconset = new QIconSet( icon );
    else
	d->iconset = 0;
    delete i;
    d->update( QActionPrivate::Icons );
}

QIconSet QAction::iconSet() const
{
    if ( d->iconset )
	return *d->iconset;
    return QIconSet();
}

/*!
    \property QAction::text
    \brief the action's descriptive text

    If \l QMainWindow::usesTextLabel is TRUE, the text appears as a
    label in the relevant tool button. It also serves as the default
    text in menus and tool tips if these have not been specifically
    defined. There is no default text.

    \sa setMenuText() setToolTip() setStatusTip()
*/
void QAction::setText( const QString& text )
{
    d->text = text;
    d->update();
}

QString QAction::text() const
{
    return d->text;
}


/*!
    \property QAction::menuText
    \brief the action's menu text

    If the action is added to a menu the menu option will consist of
    the icon (if there is one), the menu text and the accelerator (if
    there is one). If the menu text is not explicitly set in the
    constructor or by using setMenuText() the action's description
    text will be used as the menu text. There is no default menu text.

    \sa text
*/
void QAction::setMenuText( const QString& text )
{
    if ( d->menutext == text )
	return;

    d->menutext = text;
    d->update();
}

QString QAction::menuText() const
{
    return d->menuText();
}

/*!
    \property QAction::toolTip
    \brief the action's tool tip

    This text is used for the tool tip. If no status tip has been set
    the tool tip will be used for the status tip.

    If no tool tip is specified the action's text is used, and if that
    hasn't been specified the description text is used as the tool tip
    text.

    There is no default tool tip text.

    \sa setStatusTip() setAccel()
*/
void QAction::setToolTip( const QString& tip )
{
    if ( d->tooltip == tip )
	return;

    d->tooltip = tip;
    d->update();
}

QString QAction::toolTip() const
{
    return d->toolTip();
}

/*!
    \property QAction::statusTip
    \brief the action's status tip

    The statusTip is displayed on all status bars that this action's
    toplevel parent widget provides.

    If no status tip is defined, the action uses the tool tip text.

    There is no default statusTip text.

    \sa setStatusTip() setToolTip()
*/
//#### Please reimp for QActionGroup!
//#### For consistency reasons even action groups should show
//#### status tips (as they already do with tool tips)
//#### Please change QActionGroup class doc appropriately after
//#### reimplementation.
void QAction::setStatusTip( const QString& tip )
{
    if ( d->statustip == tip )
	return;

    d->statustip = tip;
    d->update();
}

QString QAction::statusTip() const
{
    return d->statusTip();
}

/*!
    \property QAction::whatsThis
    \brief the action's "What's This?" help text

    The whats this text is used to provide a brief description of the
    action. The text may contain rich text (HTML-like tags -- see
    QStyleSheet for the list of supported tags). There is no default
    "What's This" text.

    \sa QWhatsThis
*/
void QAction::setWhatsThis( const QString& whatsThis )
{
    if ( d->whatsthis == whatsThis )
	return;
    d->whatsthis = whatsThis;
    d->update();
}

QString QAction::whatsThis() const
{
    return d->whatsthis;
}


#ifndef QT_NO_ACCEL
/*!
    \property QAction::accel
    \brief the action's accelerator key

    The keycodes can be found in \l Qt::Key and \l Qt::Modifier. There
    is no default accelerator key.
*/
//#### Please reimp for QActionGroup!
//#### For consistency reasons even QActionGroups should respond to
//#### their accelerators and e.g. open the relevant submenu.
//#### Please change appropriate QActionGroup class doc after
//#### reimplementation.
void QAction::setAccel( const QKeySequence& key )
{
    if ( d->key == key )
	return;

    d->key = key;
    delete d->accel;
    d->accel = 0;

    if ( !(int)key ) {
	d->update();
	return;
    }

    QObject* p = parent();
    while ( p && !p->isWidgetType() ) {
	p = p->parent();
    }
    if ( p ) {
	d->accel = new QAccel( (QWidget*)p, this, "qt_action_accel" );
	d->accelid = d->accel->insertItem( d->key );
	d->accel->connectItem( d->accelid, this, SLOT( internalActivation() ) );
    }
#if defined(QT_CHECK_STATE)
    else
	qWarning( "QAction::setAccel() (%s) requires widget in parent chain", name() );
#endif
    d->update();
}


QKeySequence QAction::accel() const
{
    return d->key;
}
#endif


/*!
    \property QAction::toggleAction
    \brief whether the action is a toggle action

    A toggle action is one which has an on/off state. For example a
    Bold toolbar button is either on or off. An action which is not a
    toggle action is a command action; a command action is simply
    executed, e.g. file save. This property's default is FALSE.

    In some situations, the state of one toggle action should depend
    on the state of others. For example, "Left Align", "Center" and
    "Right Align" toggle actions are mutually exclusive. To achieve
    exclusive toggling, add the relevant toggle actions to a
    QActionGroup with the \l QActionGroup::exclusive property set to
    TRUE.
*/
void QAction::setToggleAction( bool enable )
{
    if ( enable == (bool)d->toggleaction )
	return;

    if ( !enable )
	d->on = FALSE;

    d->toggleaction = enable;
    d->update();
}

bool QAction::isToggleAction() const
{
    return d->toggleaction;
}

/*!
    Activates the action and executes all connected slots.
    This only works for actions that are not toggle action.

    \sa toggle()
*/
void QAction::activate()
{
    if ( isToggleAction() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QAction::%s() (%s) Toggle actions "
		  "can not be activated", "activate", name() );
#endif
	return;
    }
    emit activated();
}

/*!
    Toggles the state of a toggle action.

    \sa on, activate(), toggled(), isToggleAction()
*/
void QAction::toggle()
{
    if ( !isToggleAction() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QAction::%s() (%s) Only toggle actions "
		  "can be switched", "toggle", name() );
#endif
	return;
    }
    setOn( !isOn() );
}

/*!
    \property QAction::on
    \brief whether a toggle action is on

    This property is always on (TRUE) for command actions and
    \l{QActionGroup}s; setOn() has no effect on them. For action's
    where isToggleAction() is TRUE, this property's default value is
    off (FALSE).

    \sa toggleAction
*/
void QAction::setOn( bool enable )
{
    if ( !isToggleAction() ) {
#if defined(QT_CHECK_STATE)
	if ( enable )
	    qWarning( "QAction::%s() (%s) Only toggle actions "
		      "can be switched", "setOn", name() );
#endif
	return;
    }
    if ( enable == (bool)d->on )
	return;
    d->on = enable;
    d->update( QActionPrivate::State );
    emit toggled( enable );
}

bool QAction::isOn() const
{
    return d->on;
}

/*!
    \property QAction::enabled
    \brief whether the action is enabled

    Disabled actions can't be chosen by the user. They don't disappear
    from the menu/tool bar but are displayed in a way which indicates
    that they are unavailable, e.g. they might be displayed grayed
    out.

    What's this? help on disabled actions is still available provided
    the \l QAction::whatsThis property is set.
*/
void QAction::setEnabled( bool enable )
{
    d->forceDisabled = !enable;

    if ( (bool)d->enabled == enable )
	return;

    d->enabled = enable;
    d->update( QActionPrivate::State );
}

bool QAction::isEnabled() const
{
    return d->enabled;
}

/*!
    Disables the action if \a disable is TRUE; otherwise
    enables the action.

    See the \l enabled documentation for more information.
*/
void QAction::setDisabled( bool disable )
{
    setEnabled( !disable );
}

/*!
    \property QAction::visible
    \brief whether the action can be seen (e.g. in menus and toolbars)

    If \e visible is TRUE the action can be seen (e.g. in menus and
    toolbars) and chosen by the user; if \e visible is FALSE the
    action cannot be seen or chosen by the user.

    Actions which are not visible are \e not grayed out; they do not
    appear at all.
*/
void QAction::setVisible( bool visible )
{
    d->forceInvisible = !visible;

    if ( (bool)d->visible == visible )
	return;
    d->visible = visible;
    d->update( QActionPrivate::Visibility );
#if (QT_VERSION-0 >= 0x040000)
#error "QAction::setVisible function wants to be virtual. Also add virtual change() function"
#endif
    if ( d->d_group ) //### this function wants to be virtual in 4.0
	d->d_group->update( (QActionGroup*) this );
}

/*
    Returns TRUE if the action is visible (e.g. in menus and
    toolbars); otherwise returns FALSE.
*/
bool QAction::isVisible() const
{
    return d->visible;
}

/*! \internal
*/
void QAction::internalActivation()
{
    if ( isToggleAction() )
	setOn( !isOn() );
    emit activated();
}

/*! \internal
*/
void QAction::toolButtonToggled( bool on )
{
    if ( !isToggleAction() )
	return;
    setOn( on );
}

/*!
    Adds this action to widget \a w.

    Currently actions may be added to QToolBar and QPopupMenu widgets.

    An action added to a tool bar is automatically displayed as a tool
    button; an action added to a pop up menu appears as a menu option.

    addTo() returns TRUE if the action was added successfully and
    FALSE otherwise. (If \a w is not a QToolBar or QPopupMenu the
    action will not be added and FALSE will be returned.)

    \sa removeFrom()
*/
bool QAction::addTo( QWidget* w )
{
#ifndef QT_NO_TOOLBAR
    if ( ::qt_cast<QToolBar*>(w) ) {
	if ( !qstrcmp( name(), "qt_separator_action" ) ) {
	    ((QToolBar*)w)->addSeparator();
	} else {
	    QCString bname = name() + QCString( "_action_button" );
	    QToolButton* btn = new QToolButton( (QToolBar*) w, bname );
	    addedTo( btn, w );
	    btn->setToggleButton( d->toggleaction );
	    d->toolbuttons.append( btn );
	    if ( d->iconset )
		btn->setIconSet( *d->iconset );
	    d->update( QActionPrivate::State | QActionPrivate::Visibility | QActionPrivate::EverythingElse ) ;
	    connect( btn, SIGNAL( clicked() ), this, SIGNAL( activated() ) );
	    connect( btn, SIGNAL( toggled(bool) ), this, SLOT( toolButtonToggled(bool) ) );
	    connect( btn, SIGNAL( destroyed() ), this, SLOT( objectDestroyed() ) );
#ifndef QT_NO_TOOLTIP
	    connect( &(d->tipGroup), SIGNAL(showTip(const QString&)), this, SLOT(showStatusText(const QString&)) );
	    connect( &(d->tipGroup), SIGNAL(removeTip()), this, SLOT(clearStatusText()) );
#endif
	}
    } else
#endif
    if ( ::qt_cast<QPopupMenu*>(w) ) {
	QActionPrivate::MenuItem* mi = new QActionPrivate::MenuItem;
	mi->popup = (QPopupMenu*) w;
	QIconSet* diconset = d->iconset;
	if ( !qstrcmp( name(), "qt_separator_action" ) )
	    mi->id = ((QPopupMenu*)w)->insertSeparator();
	else if ( diconset )
	    mi->id = mi->popup->insertItem( *diconset, QString::fromLatin1("") );
	else
	    mi->id = mi->popup->insertItem( QString::fromLatin1("") );
	addedTo( mi->popup->indexOf( mi->id ), mi->popup );
	mi->popup->connectItem( mi->id, this, SLOT(internalActivation()) );
	d->menuitems.append( mi );
	d->update( QActionPrivate::State | QActionPrivate::Visibility | QActionPrivate::EverythingElse ) ;
	w->topLevelWidget()->className();
	connect( mi->popup, SIGNAL(highlighted(int)), this, SLOT(menuStatusText(int)) );
	connect( mi->popup, SIGNAL(aboutToHide()), this, SLOT(clearStatusText()) );
	connect( mi->popup, SIGNAL( destroyed() ), this, SLOT( objectDestroyed() ) );
    // Makes only sense when called by QActionGroup::addTo
    } else if ( ::qt_cast<QComboBox*>(w) ) {
	QActionPrivate::ComboItem *ci = new QActionPrivate::ComboItem;
	ci->combo = (QComboBox*)w;
	connect( ci->combo, SIGNAL( destroyed() ), this, SLOT( objectDestroyed() ) );
	ci->id = ci->combo->count();
	if ( qstrcmp( name(), "qt_separator_action" ) ) {
	    if ( d->iconset )
		ci->combo->insertItem( d->iconset->pixmap(), text() );
	    else
		ci->combo->insertItem( text() );
	} else {
	    ci->id = -1;
	}
	d->comboitems.append( ci );

	d->update( QActionPrivate::State | QActionPrivate::EverythingElse );
    } else {
	qWarning( "QAction::addTo(), unknown object" );
	return FALSE;
    }
    return TRUE;
}

/*!
    This function is called from the addTo() function when it has
    created a widget (\a actionWidget) for the action in the \a
    container.
*/

void QAction::addedTo( QWidget *actionWidget, QWidget *container )
{
    Q_UNUSED( actionWidget );
    Q_UNUSED( container );
}

/*!
    \overload

    This function is called from the addTo() function when it has
    created a menu item at the index position \a index in the popup
    menu \a menu.
*/

void QAction::addedTo( int index, QPopupMenu *menu )
{
    Q_UNUSED( index );
    Q_UNUSED( menu );
}

/*!
    Sets the status message to \a text
*/
void QAction::showStatusText( const QString& text )
{
#ifndef QT_NO_STATUSBAR
    // find out whether we are clearing the status bar by the popup that actually set the text
    static QPopupMenu *lastmenu = 0;
    QObject *s = (QObject*)sender();
    if ( s ) {
	QPopupMenu *menu = (QPopupMenu*)s->qt_cast( "QPopupMenu" );
	if ( menu && !!text )
	    lastmenu = menu;
	else if ( menu && text.isEmpty() ) {
	    if ( lastmenu && menu != lastmenu )
		return;
	    lastmenu = 0;
	}
    }

    QObject* par = parent();
    QObject* lpar = 0;
    QStatusBar *bar = 0;
    while ( par && !bar ) {
	lpar = par;
	bar = (QStatusBar*)par->child( 0, "QStatusBar", FALSE );
	par = par->parent();
    }
    if ( !bar && lpar ) {
	QObjectList *l = lpar->queryList( "QStatusBar" );
	if ( !l )
	    return;
	// #### hopefully the last one is the one of the mainwindow...
	bar = (QStatusBar*)l->last();
	delete l;
    }
    if ( bar ) {
	if ( text.isEmpty() )
	    bar->clear();
	else
	    bar->message( text );
    }
#endif
}

/*!
    Sets the status message to the menu item's status text, or to the
    tooltip, if there is no status text.
*/
void QAction::menuStatusText( int id )
{
    static int lastId = 0;
    QString text;
    QPtrListIterator<QActionPrivate::MenuItem> it( d->menuitems);
    QActionPrivate::MenuItem* mi;
    while ( ( mi = it.current() ) ) {
	++it;
	if ( mi->id == id ) {
	    text = statusTip();
	    break;
	}
    }

    if ( !text.isEmpty() )
	showStatusText( text );
    else if ( id != lastId )
	clearStatusText();
    lastId = id;
}

/*!
    Clears the status text.
*/
void QAction::clearStatusText()
{
    if (!statusTip().isEmpty())
	showStatusText( QString::null );
}

/*!
    Removes the action from widget \a w.

    Returns TRUE if the action was removed successfully; otherwise
    returns FALSE.

    \sa addTo()
*/
bool QAction::removeFrom( QWidget* w )
{
#ifndef QT_NO_TOOLBAR
    if ( ::qt_cast<QToolBar*>(w) ) {
	QPtrListIterator<QToolButton> it( d->toolbuttons);
	QToolButton* btn;
	while ( ( btn = it.current() ) ) {
	    ++it;
	    if ( btn->parentWidget() == w ) {
		d->toolbuttons.removeRef( btn );
		disconnect( btn, SIGNAL( destroyed() ), this, SLOT( objectDestroyed() ) );
		delete btn;
		// no need to disconnect from statusbar
	    }
	}
    } else
#endif
    if ( ::qt_cast<QPopupMenu*>(w) ) {
	QPtrListIterator<QActionPrivate::MenuItem> it( d->menuitems);
	QActionPrivate::MenuItem* mi;
	while ( ( mi = it.current() ) ) {
	    ++it;
	    if ( mi->popup == w ) {
		disconnect( mi->popup, SIGNAL(highlighted(int)), this, SLOT(menuStatusText(int)) );
		disconnect( mi->popup, SIGNAL(aboutToHide()), this, SLOT(clearStatusText()) );
		disconnect( mi->popup, SIGNAL( destroyed() ), this, SLOT( objectDestroyed() ) );
		mi->popup->removeItem( mi->id );
		d->menuitems.removeRef( mi );
	    }
	}
    } else if ( ::qt_cast<QComboBox*>(w) ) {
	QPtrListIterator<QActionPrivate::ComboItem> it( d->comboitems );
	QActionPrivate::ComboItem *ci;
	while ( ( ci = it.current() ) ) {
	    ++it;
	    if ( ci->combo == w ) {
		disconnect( ci->combo, SIGNAL(destroyed()), this, SLOT(objectDestroyed()) );
		d->comboitems.removeRef( ci );
	    }
	}
    } else {
	qWarning( "QAction::removeFrom(), unknown object" );
	return FALSE;
    }
    return TRUE;
}

/*!
  \internal
*/
void QAction::objectDestroyed()
{
    const QObject* obj = sender();
    QPtrListIterator<QActionPrivate::MenuItem> it( d->menuitems );
    QActionPrivate::MenuItem* mi;
    while ( ( mi = it.current() ) ) {
	++it;
	if ( mi->popup == obj )
	    d->menuitems.removeRef( mi );
    }
    QActionPrivate::ComboItem *ci;
    QPtrListIterator<QActionPrivate::ComboItem> it2( d->comboitems );
    while ( ( ci = it2.current() ) ) {
	++it2;
	if ( ci->combo == obj )
	    d->comboitems.removeRef( ci );
    }
    d->toolbuttons.removeRef( (QToolButton*) obj );
}

/*!
    \fn void QAction::activated()

    This signal is emitted when an action is activated by the user,
    e.g. when the user clicks a menu option or a toolbar button or
    presses an action's accelerator key combination.

    Connect to this signal for command actions. Connect to the
    toggled() signal for toggle actions.
*/

/*!
    \fn void QAction::toggled(bool on)

    This signal is emitted when a toggle action changes state; command
    actions and \l{QActionGroup}s don't emit toggled().

    The \a on argument denotes the new state: If \a on is TRUE the
    toggle action is switched on, and if \a on is FALSE the toggle
    action is switched off.

    To trigger a user command depending on whether a toggle action has
    been switched on or off connect it to a slot that takes a bool to
    indicate the state, e.g.

    \quotefile action/toggleaction/toggleaction.cpp
    \skipto QMainWindow * window
    \printline QMainWindow * window
    \skipto labelonoffaction
    \printline labelonoffaction
    \skipto connect
    \printuntil setUsesTextLabel

    \sa activated() setToggleAction() setOn()
*/

void QActionGroupPrivate::update( const QActionGroup* that )
{
    for ( QPtrListIterator<QAction> it( actions ); it.current(); ++it ) {
	if ( that->isEnabled() && !it.current()->d->forceDisabled ) {
	    it.current()->setEnabled( TRUE );
	} else if ( !that->isEnabled() && it.current()->isEnabled() ) {
	    it.current()->setEnabled( FALSE );
	    it.current()->d->forceDisabled = FALSE;
	}
	if ( that->isVisible() && !it.current()->d->forceInvisible ) {
	    it.current()->setVisible( TRUE );
	} else if ( !that->isVisible() && it.current()->isVisible() ) {
	    it.current()->setVisible( FALSE );
	    it.current()->d->forceInvisible = FALSE;
	}
    }
    for ( QPtrListIterator<QComboBox> cb( comboboxes ); cb.current(); ++cb ) {
	QComboBox *combobox = cb.current();
	combobox->setEnabled( that->isEnabled() );
	combobox->setShown( that->isVisible() );

#ifndef QT_NO_TOOLTIP
	QToolTip::remove( combobox );
	if ( !!that->toolTip() )
	    QToolTip::add( combobox, that->toolTip() );
#endif
#ifndef QT_NO_WHATSTHIS
	QWhatsThis::remove( combobox );
	if ( !!that->whatsThis() )
	    QWhatsThis::add( combobox, that->whatsThis() );
#endif

    }
    for ( QPtrListIterator<QToolButton> mb( menubuttons ); mb.current(); ++mb ) {
	QToolButton *button = mb.current();
	button->setEnabled( that->isEnabled() );
	button->setShown( that->isVisible() );

	if ( !that->text().isNull() )
	    button->setTextLabel( that->text() );
	if ( !that->iconSet().isNull() )
	    button->setIconSet( that->iconSet() );

#ifndef QT_NO_TOOLTIP
	QToolTip::remove( mb.current() );
	if ( !!that->toolTip() )
	    QToolTip::add( button, that->toolTip() );
#endif
#ifndef QT_NO_WHATSTHIS
	QWhatsThis::remove( button );
	if ( !!that->whatsThis() )
	    QWhatsThis::add( button, that->whatsThis() );
#endif
    }
    for ( QPtrListIterator<QActionGroupPrivate::MenuItem> pu( menuitems ); pu.current(); ++pu ) {
	QWidget* parent = pu.current()->popup->parentWidget();
	if ( ::qt_cast<QPopupMenu*>(parent) ) {
	    QPopupMenu* ppopup = (QPopupMenu*)parent;
	    ppopup->setItemEnabled( pu.current()->id, that->isEnabled() );
	    ppopup->setItemVisible( pu.current()->id, that->isVisible() );
	} else {
	    pu.current()->popup->setEnabled( that->isEnabled() );
	}
    }
    for ( QPtrListIterator<QPopupMenu> pm( popupmenus ); pm.current(); ++pm ) {
	QPopupMenu *popup = pm.current();
	QPopupMenu *parent = ::qt_cast<QPopupMenu*>(popup->parentWidget());
	if ( !parent )
	    continue;

	int index;
	parent->findPopup( popup, &index );
	int id = parent->idAt( index );
	if ( !that->iconSet().isNull() )
	    parent->changeItem( id, that->iconSet(), that->menuText() );
	else
	    parent->changeItem( id, that->menuText() );
	parent->setItemEnabled( id, that->isEnabled() );
#ifndef QT_NO_ACCEL
	parent->setAccel( that->accel(), id );
#endif
    }
}

/*!
    \class QActionGroup qaction.h
    \brief The QActionGroup class groups actions together.

    \ingroup basic
    \ingroup application

    In some situations it is useful to group actions together. For
    example, if you have a left justify action, a right justify action
    and a center action, only one of these actions should be active at
    any one time, and one simple way of achieving this is to group the
    actions together in an action group.

    An action group can also be added to a menu or a toolbar as a
    single unit, with all the actions within the action group
    appearing as separate menu options and toolbar buttons.

    Here's an example from examples/textedit:
    \quotefile textedit/textedit.cpp
    \skipto QActionGroup
    \printuntil connect

    Here we create a new action group. Since the action group is exclusive
    by default, only one of the actions in the group is ever active at any
    one time. We then connect the group's selected() signal to our
    textAlign() slot.

    \printuntil actionAlignLeft->setToggleAction

    We create a left align action, add it to the toolbar and the menu
    and make it a toggle action. We create center and right align
    actions in exactly the same way.

    \omit
    A QActionGroup emits an activated() signal when one of its actions
    is activated.
    \endomit
    The actions in an action group emit their activated() (and for
    toggle actions, toggled()) signals as usual.

    The setExclusive() function is used to ensure that only one action
    is active at any one time: it should be used with actions which
    have their \c toggleAction set to TRUE.

    Action group actions appear as individual menu options and toolbar
    buttons. For exclusive action groups use setUsesDropDown() to
    display the actions in a subwidget of any widget the action group
    is added to. For example, the actions would appear in a combobox
    in a toolbar or as a submenu in a menu.

    Actions can be added to an action group using add(), but normally
    they are added by creating the action with the action group as
    parent. Actions can have separators dividing them using
    addSeparator(). Action groups are added to widgets with addTo().
*/

/*!
    Constructs an action group called \a name, with parent \a parent.

    The action group is exclusive by default. Call setExclusive(FALSE) to make
    the action group non-exclusive.
*/
QActionGroup::QActionGroup( QObject* parent, const char* name )
    : QAction( parent, name )
{
    d = new QActionGroupPrivate;
    d->exclusive = TRUE;
    d->dropdown = FALSE;
    d->selected = 0;
    d->separatorAction = 0;
    QAction::d->d_group = d;

    connect( this, SIGNAL(selected(QAction*)), SLOT(internalToggle(QAction*)) );
}

/*!
    Constructs an action group called \a name, with parent \a parent.

    If \a exclusive is TRUE only one toggle action in the group will
    ever be active.

    \sa exclusive
*/
QActionGroup::QActionGroup( QObject* parent, const char* name, bool exclusive )
    : QAction( parent, name )
{
    d = new QActionGroupPrivate;
    d->exclusive = exclusive;
    d->dropdown = FALSE;
    d->selected = 0;
    d->separatorAction = 0;
    QAction::d->d_group = d;

    connect( this, SIGNAL(selected(QAction*)), SLOT(internalToggle(QAction*)) );
}

/*!
    Destroys the object and frees allocated resources.
*/

QActionGroup::~QActionGroup()
{
    QPtrListIterator<QActionGroupPrivate::MenuItem> mit( d->menuitems );
    while ( mit.current() ) {
	QActionGroupPrivate::MenuItem *mi = mit.current();
	++mit;
	if ( mi->popup )
	    mi->popup->disconnect( SIGNAL(destroyed()), this, SLOT(objectDestroyed()) );
    }

    QPtrListIterator<QComboBox> cbit( d->comboboxes );
    while ( cbit.current() ) {
	QComboBox *cb = cbit.current();
	++cbit;
	cb->disconnect(  SIGNAL(destroyed()), this, SLOT(objectDestroyed()) );
    }
    QPtrListIterator<QToolButton> mbit( d->menubuttons );
    while ( mbit.current() ) {
	QToolButton *mb = mbit.current();
	++mbit;
	mb->disconnect(  SIGNAL(destroyed()), this, SLOT(objectDestroyed()) );
    }
    QPtrListIterator<QPopupMenu> pmit( d->popupmenus );
    while ( pmit.current() ) {
	QPopupMenu *pm = pmit.current();
	++pmit;
	pm->disconnect(  SIGNAL(destroyed()), this, SLOT(objectDestroyed()) );
    }

    delete d->separatorAction;
    d->menubuttons.setAutoDelete( TRUE );
    d->comboboxes.setAutoDelete( TRUE );
    d->menuitems.setAutoDelete( TRUE );
    d->popupmenus.setAutoDelete( TRUE );
    delete d;
}

/*!
    \property QActionGroup::exclusive
    \brief whether the action group does exclusive toggling

    If exclusive is TRUE only one toggle action in the action group
    can ever be active at any one time. If the user chooses another
    toggle action in the group the one they chose becomes active and
    the one that was active becomes inactive.

    \sa QAction::toggleAction
*/
void QActionGroup::setExclusive( bool enable )
{
    d->exclusive = enable;
}

bool QActionGroup::isExclusive() const
{
    return d->exclusive;
}

/*!
    \property QActionGroup::usesDropDown
    \brief whether the group's actions are displayed in a subwidget of
    the widgets the action group is added to

    Exclusive action groups added to a toolbar display their actions
    in a combobox with the action's \l QAction::text and \l
    QAction::iconSet properties shown. Non-exclusive groups are
    represented by a tool button showing their \l QAction::iconSet and
    -- depending on \l QMainWindow::usesTextLabel() -- text()
    property.

    In a popup menu the member actions are displayed in a submenu.

    Changing usesDropDown only affects \e subsequent calls to addTo().

    Note that setting this property for actions in a combobox causes
    calls to their \link QAction::setVisible()\endlink,
    \link QAction::setEnabled()\endlink, and
    \link QAction::setDisabled()\endlink functions to have no effect.

    This property's default is FALSE.

*/
void QActionGroup::setUsesDropDown( bool enable )
{
    d->dropdown = enable;
}

bool QActionGroup::usesDropDown() const
{
    return d->dropdown;
}

/*!
    Adds action \a action to this group.

    Normally an action is added to a group by creating it with the
    group as parent, so this function is not usually used.

    \sa addTo()
*/
void QActionGroup::add( QAction* action )
{
    if ( d->actions.containsRef( action ) )
	return;

    d->actions.append( action );

    if ( action->whatsThis().isNull() )
	action->setWhatsThis( whatsThis() );
    if ( action->toolTip().isNull() )
	action->setToolTip( toolTip() );

    if (!action->d->forceDisabled)
	action->d->enabled = isEnabled();
    if (!action->d->forceInvisible)
	action->d->visible = isVisible();

    connect( action, SIGNAL( destroyed() ), this, SLOT( childDestroyed() ) );
    connect( action, SIGNAL( activated() ), this, SIGNAL( activated() ) );
    connect( action, SIGNAL( toggled(bool) ), this, SLOT( childToggled(bool) ) );

    for ( QPtrListIterator<QComboBox> cb( d->comboboxes ); cb.current(); ++cb ) {
	action->addTo( cb.current() );
    }
    for ( QPtrListIterator<QToolButton> mb( d->menubuttons ); mb.current(); ++mb ) {
	QPopupMenu* popup = mb.current()->popup();
	if ( !popup )
	    continue;
	action->addTo( popup );
    }
    for ( QPtrListIterator<QActionGroupPrivate::MenuItem> mi( d->menuitems ); mi.current(); ++mi ) {
	QPopupMenu* popup = mi.current()->popup;
	if ( !popup )
	    continue;
	action->addTo( popup );
    }
}

/*!
    Adds a separator to the group.
*/
void QActionGroup::addSeparator()
{
    if ( !d->separatorAction )
	d->separatorAction = new QAction( 0, "qt_separator_action" );
    d->actions.append( d->separatorAction );
}


/*! \fn void QActionGroup::insert( QAction* a )

  \obsolete

  Use add() instead, or better still create the action with the action
  group as its parent.
 */

/*!
    Adds this action group to the widget \a w.

    If isExclusive() is FALSE or usesDropDown() is FALSE, the actions within
    the group are added to the widget individually. For example, if the widget
    is a menu, the actions will appear as individual menu options, and
    if the widget is a toolbar, the actions will appear as toolbar buttons.

    If both isExclusive() and usesDropDown() are TRUE, the actions
    are presented either in a combobox (if \a w is a toolbar) or in a
    submenu (if \a w is a menu).

    All actions should be added to the action group \e before the
    action group is added to the widget. If actions are added to the
    action group \e after the action group has been added to the
    widget these later actions will \e not appear.

    \sa setExclusive() setUsesDropDown() removeFrom()
*/
bool QActionGroup::addTo( QWidget* w )
{
#ifndef QT_NO_TOOLBAR
    if ( ::qt_cast<QToolBar*>(w) ) {
	if ( d->dropdown ) {
	    if ( !d->exclusive ) {
		QPtrListIterator<QAction> it( d->actions);
		if ( !it.current() )
		    return TRUE;

		QAction *defAction = it.current();

		QToolButton* btn = new QToolButton( (QToolBar*) w, "qt_actiongroup_btn" );
		addedTo( btn, w );
		connect( btn, SIGNAL(destroyed()), SLOT(objectDestroyed()) );
		d->menubuttons.append( btn );

		if ( !iconSet().isNull() )
		    btn->setIconSet( iconSet() );
		else if ( !defAction->iconSet().isNull() )
		    btn->setIconSet( defAction->iconSet() );
		if ( !!text() )
		    btn->setTextLabel( text() );
		else if ( !!defAction->text() )
		    btn->setTextLabel( defAction->text() );
#ifndef QT_NO_TOOLTIP
		if ( !!toolTip() )
		    QToolTip::add( btn, toolTip() );
		else if ( !!defAction->toolTip() )
		    QToolTip::add( btn, defAction->toolTip() );
#endif
#ifndef QT_NO_WHATSTHIS
		if ( !!whatsThis() )
		    QWhatsThis::add( btn, whatsThis() );
		else if ( !!defAction->whatsThis() )
		    QWhatsThis::add( btn, defAction->whatsThis() );
#endif

		connect( btn, SIGNAL( clicked() ), defAction, SIGNAL( activated() ) );
		connect( btn, SIGNAL( toggled(bool) ), defAction, SLOT( toolButtonToggled(bool) ) );
		connect( btn, SIGNAL( destroyed() ), defAction, SLOT( objectDestroyed() ) );

		QPopupMenu *menu = new QPopupMenu( btn, "qt_actiongroup_menu" );
		btn->setPopupDelay( 0 );
		btn->setPopup( menu );

		while( it.current() ) {
		    it.current()->addTo( menu );
		    ++it;
		}
		d->update( this );
		return TRUE;
	    } else {
		QComboBox *box = new QComboBox( FALSE, w, "qt_actiongroup_combo" );
		addedTo( box, w );
		connect( box, SIGNAL(destroyed()), SLOT(objectDestroyed()) );
		d->comboboxes.append( box );
#ifndef QT_NO_TOOLTIP
		if ( !!toolTip() )
		    QToolTip::add( box, toolTip() );
#endif
#ifndef QT_NO_WHATSTHIS
		if ( !!whatsThis() )
		    QWhatsThis::add( box, whatsThis() );
#endif

		int onIndex = 0;
		bool foundOn = FALSE;
		for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
		    QAction *action = it.current();
		    if ( !foundOn )
			foundOn = action->isOn();
		    if ( qstrcmp( action->name(), "qt_separator_action" ) && !foundOn )
			onIndex++;
		    action->addTo( box );
		}
		if ( foundOn )
		    box->setCurrentItem( onIndex );
		connect( box, SIGNAL(activated(int)), this, SLOT( internalComboBoxActivated(int)) );
		connect( box, SIGNAL(highlighted(int)), this, SLOT( internalComboBoxHighlighted(int)) );
		d->update( this );
		return TRUE;
	    }
	}
    } else
#endif
    if ( ::qt_cast<QPopupMenu*>(w) ) {
	QPopupMenu *popup;
	if ( d->dropdown ) {
	    QPopupMenu *menu = (QPopupMenu*)w;
	    popup = new QPopupMenu( w, "qt_actiongroup_menu" );
	    d->popupmenus.append( popup );
	    connect( popup, SIGNAL(destroyed()), SLOT(objectDestroyed()) );

	    int id;
	    if ( !iconSet().isNull() ) {
		if ( menuText().isEmpty() )
		    id = menu->insertItem( iconSet(), text(), popup );
		else
		    id = menu->insertItem( iconSet(), menuText(), popup );
	    } else {
		if ( menuText().isEmpty() )
		    id = menu->insertItem( text(), popup );
		else
		    id = menu->insertItem( menuText(), popup );
	    }

	    addedTo( menu->indexOf( id ), menu );

	    QActionGroupPrivate::MenuItem *item = new QActionGroupPrivate::MenuItem;
	    item->id = id;
	    item->popup = popup;
	    d->menuitems.append( item );
	} else {
	    popup = (QPopupMenu*)w;
	}
	for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
	    // #### do an addedTo( index, popup, action), need to find out index
	    it.current()->addTo( popup );
	}
	return TRUE;
    }

    for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
	// #### do an addedTo( index, popup, action), need to find out index
	it.current()->addTo( w );
    }

    return TRUE;
}

/*! \reimp
*/
bool QActionGroup::removeFrom( QWidget* w )
{
    for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
	it.current()->removeFrom( w );
    }

#ifndef QT_NO_TOOLBAR
    if ( ::qt_cast<QToolBar*>(w) ) {
	QPtrListIterator<QComboBox> cb( d->comboboxes );
	while( cb.current() ) {
	    QComboBox *box = cb.current();
	    ++cb;
	    if ( box->parentWidget() == w )
		delete box;
	}
	QPtrListIterator<QToolButton> mb( d->menubuttons );
	while( mb.current() ) {
	    QToolButton *btn = mb.current();
	    ++mb;
	    if ( btn->parentWidget() == w )
		delete btn;
	}
    } else
#endif
    if ( ::qt_cast<QPopupMenu*>(w) ) {
	QPtrListIterator<QActionGroupPrivate::MenuItem> pu( d->menuitems );
	while ( pu.current() ) {
	    QActionGroupPrivate::MenuItem *mi = pu.current();
	    ++pu;
	    if ( d->dropdown && mi->popup )
		( (QPopupMenu*)w )->removeItem( mi->id );
	    delete mi->popup;
	}
    }

    return TRUE;
}

/*! \internal
*/
void QActionGroup::childToggled( bool b )
{
    if ( !isExclusive() )
	return;
    QAction* s = (QAction*) sender();
    if ( b ) {
	if ( s != d->selected ) {
	    d->selected = s;
	    for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
		if ( it.current()->isToggleAction() && it.current() != s )
		    it.current()->setOn( FALSE );
	    }
	    emit activated();
	    emit selected( s );
	} else if ( !s->isToggleAction() ) {
	    emit activated();
	}
    } else {
	if ( s == d->selected ) {
	    // at least one has to be selected
	    s->setOn( TRUE );
	}
    }
}

/*! \internal
*/
void QActionGroup::childDestroyed()
{
    d->actions.removeRef( (QAction*) sender() );
    if ( d->selected == sender() )
	d->selected = 0;
}

/*! \reimp
*/
void QActionGroup::setEnabled( bool enable )
{
    if ( enable == isEnabled() )
	return;

    QAction::setEnabled( enable );
    d->update( this );
}

/*! \reimp
*/
void QActionGroup::setToggleAction( bool toggle )
{
    for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it )
	it.current()->setToggleAction( toggle );

    QAction::setToggleAction( TRUE );
    d->update( this );
}

/*! \reimp
*/
void QActionGroup::setOn( bool on )
{
    for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
	QAction *act = it.current();
	if ( act->isToggleAction() )
	    act->setOn( on );
    }

    QAction::setOn( on );
    d->update( this );
}

/*! \reimp
*/
void QActionGroup::setIconSet( const QIconSet& icon )
{
    QAction::setIconSet( icon );
    d->update( this );
}

/*! \reimp
*/
void QActionGroup::setText( const QString& txt )
{
    if ( txt == text() )
	return;

    QAction::setText( txt );
    d->update( this );
}

/*! \reimp
*/
void QActionGroup::setMenuText( const QString& text )
{
    if ( text == menuText() )
	return;

    QAction::setMenuText( text );
    d->update( this );
}

/*! \reimp
*/
void QActionGroup::setToolTip( const QString& text )
{
    if ( text == toolTip() )
	return;
    for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
	if ( it.current()->toolTip().isNull() )
	    it.current()->setToolTip( text );
    }
    QAction::setToolTip( text );
    d->update( this );
}

/*! \reimp
*/
void QActionGroup::setWhatsThis( const QString& text )
{
    if ( text == whatsThis() )
	return;
    for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
	if ( it.current()->whatsThis().isNull() )
	    it.current()->setWhatsThis( text );
    }
    QAction::setWhatsThis( text );
    d->update( this );
}

/*! \reimp
*/
void QActionGroup::childEvent( QChildEvent *e )
{
    if ( !e->removed() )
	return;

    QAction *action = ::qt_cast<QAction*>(e->child());
    if ( !action )
	return;

    for ( QPtrListIterator<QComboBox> cb( d->comboboxes ); cb.current(); ++cb ) {
	for ( int i = 0; i < cb.current()->count(); i++ ) {
	    if ( cb.current()->text( i ) == action->text() ) {
		cb.current()->removeItem( i );
		break;
	    }
	}
    }
    for ( QPtrListIterator<QToolButton> mb( d->menubuttons ); mb.current(); ++mb ) {
	QPopupMenu* popup = mb.current()->popup();
	if ( !popup )
	    continue;
	action->removeFrom( popup );
    }
    for ( QPtrListIterator<QActionGroupPrivate::MenuItem> mi( d->menuitems ); mi.current(); ++mi ) {
	QPopupMenu* popup = mi.current()->popup;
	if ( !popup )
	    continue;
	action->removeFrom( popup );
    }
}

/*!
    \fn void QActionGroup::selected( QAction* )

    This signal is emitted from exclusive groups when toggle actions
    change state.

    The argument is the action whose state changed to "on".

    \sa setExclusive(), isOn() QAction::toggled()
*/

/*! \internal
*/
void QActionGroup::internalComboBoxActivated( int index )
{
    QAction *a = 0;
    for ( int i = 0; i <= index && i < (int)d->actions.count(); ++i ) {
	a = d->actions.at( i );
	if ( a && !qstrcmp( a->name(), "qt_separator_action" ) )
	    index++;
    }
    a = d->actions.at( index );
    if ( a ) {
	if ( a != d->selected ) {
	    d->selected = a;
	    for ( QPtrListIterator<QAction> it( d->actions); it.current(); ++it ) {
		if ( it.current()->isToggleAction() && it.current() != a )
		    it.current()->setOn( FALSE );
	    }
	    if ( a->isToggleAction() )
		a->setOn( TRUE );

	    emit activated();
	    if ( a->isToggleAction() )
		emit selected( d->selected );
	    emit ((QActionGroup*)a)->activated();
	} else if ( !a->isToggleAction() ) {
	    emit activated();
	    emit ((QActionGroup*)a)->activated();
	}
	a->clearStatusText();
    }
}

/*! \internal
*/
void QActionGroup::internalComboBoxHighlighted( int index )
{
    QAction *a = 0;
    for ( int i = 0; i <= index && i < (int)d->actions.count(); ++i ) {
	a = d->actions.at( i );
	if ( a && !qstrcmp( a->name(), "qt_separator_action" ) )
	    index++;
    }
    a = d->actions.at( index );
    if ( a )
	a->showStatusText(a->statusTip());
    else
	clearStatusText();
}

/*! \internal
*/
void QActionGroup::internalToggle( QAction *a )
{
    int index = d->actions.find( a );
    if ( index == -1 )
	return;

    int lastItem = index;
    for ( int i = 0; i < lastItem; i++ ) {
	QAction *action = d->actions.at( i );
	if ( !qstrcmp( action->name(), "qt_separator_action" ) )
	    index--;
    }

    for ( QPtrListIterator<QComboBox> it( d->comboboxes); it.current(); ++it )
	    it.current()->setCurrentItem( index );
}

/*! \internal
*/
void QActionGroup::objectDestroyed()
{
    const QObject* obj = sender();
    d->menubuttons.removeRef( (QToolButton*)obj );
    for ( QPtrListIterator<QActionGroupPrivate::MenuItem> mi( d->menuitems ); mi.current(); ++mi ) {
	if ( mi.current()->popup == obj ) {
	    d->menuitems.removeRef( mi.current() );
	    break;
	}
    }
    d->popupmenus.removeRef( (QPopupMenu*)obj );
    d->comboboxes.removeRef( (QComboBox*)obj );
}

/*!
    \internal
    
    This function is called from the addTo() function when it has
    created a widget (\a actionWidget) for the child action \a a in
    the \a container.
*/

void QActionGroup::addedTo( QWidget *actionWidget, QWidget *container, QAction *a )
{
    Q_UNUSED( actionWidget );
    Q_UNUSED( container );
    Q_UNUSED( a );
}

/*!
    \overload
    \internal
    
    This function is called from the addTo() function when it has
    created a menu item for the child action at the index position \a
    index in the popup menu \a menu.
*/

void QActionGroup::addedTo( int index, QPopupMenu *menu, QAction *a )
{
    Q_UNUSED( index );
    Q_UNUSED( menu );
    Q_UNUSED( a );
}

/*!
    \reimp
    \overload

    This function is called from the addTo() function when it has
    created a widget (\a actionWidget) in the \a container.
*/

void QActionGroup::addedTo( QWidget *actionWidget, QWidget *container )
{
    Q_UNUSED( actionWidget );
    Q_UNUSED( container );
}

/*!
    \reimp
    \overload

    This function is called from the addTo() function when it has
    created a menu item at the index position \a index in the popup
    menu \a menu.
*/

void QActionGroup::addedTo( int index, QPopupMenu *menu )
{
    Q_UNUSED( index );
    Q_UNUSED( menu );
}

#endif
