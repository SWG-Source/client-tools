/****************************************************************************
** $Id: qt/qpopupmenu.h   3.3.4   edited Dec 17 2003 $
**
** Definition of QPopupMenu class
**
** Created : 941128
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
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

#ifndef QPOPUPMENU_H
#define QPOPUPMENU_H

#ifndef QT_H
#include "qframe.h"
#include "qmenudata.h"
#endif // QT_H

#ifndef QT_NO_POPUPMENU
class QPopupMenuPrivate;

class Q_EXPORT QPopupMenu : public QFrame, public QMenuData
{
    Q_OBJECT
    Q_PROPERTY( bool checkable READ isCheckable WRITE setCheckable )
public:
    QPopupMenu( QWidget* parent=0, const char* name=0 );
    ~QPopupMenu();

    void	popup( const QPoint & pos, int indexAtPoint = -1 ); // open
    void	updateItem( int id );

    virtual void	setCheckable( bool );
    bool	isCheckable() const;

    void	setFont( const QFont & );
    void	show();
    void	hide();

    int		exec();
    int 	exec( const QPoint & pos, int indexAtPoint = 0 ); // modal

    virtual void	setActiveItem( int );
    QSize	sizeHint() const;

    int 	idAt( int index ) const { return QMenuData::idAt( index ); }
    int 	idAt( const QPoint& pos ) const;

    bool 	customWhatsThis() const;

    int		insertTearOffHandle( int id=-1, int index=-1 );

    void	activateItemAt( int index );
    QRect	itemGeometry( int index );


signals:
    void	activated( int itemId );
    void	highlighted( int itemId );
    void	activatedRedirect( int itemId ); // to parent menu
    void	highlightedRedirect( int itemId );
    void	aboutToShow();
    void	aboutToHide();

protected:
    int 	itemHeight( int ) const;
    int 	itemHeight( QMenuItem* mi ) const;
    void 	drawItem( QPainter* p, int tab, QMenuItem* mi,
		   bool act, int x, int y, int w, int h);

    void 	drawContents( QPainter * );

    void 	closeEvent( QCloseEvent *e );
    void	paintEvent( QPaintEvent * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );
    void	timerEvent( QTimerEvent * );
    void	leaveEvent( QEvent * );
    void 	styleChange( QStyle& );
    void	enabledChange( bool );
    int 	columns() const;

    bool	focusNextPrevChild( bool next );

    int		itemAtPos( const QPoint &, bool ignoreSeparator = TRUE ) const;

private slots:
    void	subActivated( int itemId );
    void	subHighlighted( int itemId );
#ifndef QT_NO_ACCEL
    void	accelActivated( int itemId );
    void	accelDestroyed();
#endif
    void	popupDestroyed( QObject* );
    void	modalActivation( int );

    void	subMenuTimer();
    void	subScrollTimer();
    void	allowAnimation();
    void     toggleTearOff();

    void        performDelayedChanges();

private:
    void        updateScrollerState();
    void	menuContentsChanged();
    void	menuStateChanged();
    void        performDelayedContentsChanged();
    void        performDelayedStateChanged();
    void	menuInsPopup( QPopupMenu * );
    void	menuDelPopup( QPopupMenu * );
    void	frameChanged();

    void	actSig( int, bool = FALSE );
    void	hilitSig( int );
    virtual void setFirstItemActive();
    void	hideAllPopups();
    void	hidePopups();
    bool	tryMenuBar( QMouseEvent * );
    void	byeMenuBar();

    QSize	updateSize(bool force_recalc=FALSE, bool do_resize=TRUE);
    void	updateRow( int row );
#ifndef QT_NO_ACCEL
    void	updateAccel( QWidget * );
    void	enableAccel( bool );
#endif
    QPopupMenuPrivate  *d;
#ifndef QT_NO_ACCEL
    QAccel     *autoaccel;
#endif

#if defined(Q_WS_MAC) && !defined(QMAC_QMENUBAR_NO_NATIVE)
    bool macPopupMenu(const QPoint &, int);
    uint mac_dirty_popup : 1;
#endif

    int popupActive;
    int tab;
    uint accelDisabled : 1;
    uint checkable : 1;
    uint connectModalRecursionSafety : 1;
    uint tornOff : 1;
    uint pendingDelayedContentsChanges : 1;
    uint pendingDelayedStateChanges : 1;
    int maxPMWidth;
    int ncols;
    bool	snapToMouse;
    bool	tryMouseEvent( QPopupMenu *, QMouseEvent * );

    friend class QMenuData;
    friend class QMenuBar;

    void connectModal(QPopupMenu* receiver, bool doConnect);

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPopupMenu( const QPopupMenu & );
    QPopupMenu &operator=( const QPopupMenu & );
#endif
};


#endif // QT_NO_POPUPMENU

#endif // QPOPUPMENU_H
