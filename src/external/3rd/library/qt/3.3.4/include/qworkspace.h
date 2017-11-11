/****************************************************************************
** $Id: qt/qworkspace.h   3.3.4   edited May 27 2003 $
**
** Definition of the QWorkspace class
**
** Created : 990210
**
** Copyright (C) 1999-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the workspace module of the Qt GUI Toolkit.
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

#ifndef QWORKSPACE_H
#define QWORKSPACE_H

#ifndef QT_H
#include "qwidget.h"
#include "qwidgetlist.h"
#endif // QT_H

#ifndef QT_NO_WORKSPACE

#if !defined( QT_MODULE_WORKSPACE ) || defined( QT_INTERNAL_WORKSPACE )
#define QM_EXPORT_WORKSPACE
#else
#define QM_EXPORT_WORKSPACE Q_EXPORT
#endif

class QWorkspaceChild;
class QShowEvent;
class QWorkspacePrivate;
class QPopupMenu;
class QDockWindow;

class QM_EXPORT_WORKSPACE QWorkspace : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( bool scrollBarsEnabled READ scrollBarsEnabled WRITE setScrollBarsEnabled )

#ifdef QT_WORKSPACE_WINDOWMODE
public:
#endif
    enum WindowMode { TopLevel, MDI, AutoDetect };
    WindowMode windowMode() const;
#ifdef QT_WORKSPACE_WINDOWMODE
private:
#endif

public:
#ifdef QT_WORKSPACE_WINDOWMODE
    QWorkspace( WindowMode mode, QWidget* parent=0, const char* name=0 );
#endif
    QWorkspace( QWidget* parent=0, const char* name=0 );

    ~QWorkspace();

    enum WindowOrder { CreationOrder, StackingOrder };

    QWidget* activeWindow() const;
    QWidgetList windowList() const; // ### merge with below in 4.0
    QWidgetList windowList( WindowOrder order ) const;

    QSize sizeHint() const;

    bool scrollBarsEnabled() const;
    void setScrollBarsEnabled( bool enable );

    void setPaletteBackgroundColor( const QColor & );
    void setPaletteBackgroundPixmap( const QPixmap & );

signals:
    void windowActivated( QWidget* w);

public slots:
    void cascade();
    void tile();
    void closeActiveWindow();
    void closeAllWindows();
    void activateNextWindow();
    void activatePrevWindow();

protected:
#ifndef QT_NO_STYLE
    void styleChange( QStyle& );
#endif
    void childEvent( QChildEvent * );
    void resizeEvent( QResizeEvent * );
    bool eventFilter( QObject *, QEvent * );
    void showEvent( QShowEvent *e );
    void hideEvent( QHideEvent *e );
#ifndef QT_NO_WHEELEVENT
    void wheelEvent( QWheelEvent *e );
#endif

private slots:
    void normalizeActiveWindow();
    void minimizeActiveWindow();
    void showOperationMenu();
    void popupOperationMenu( const QPoint& );
    void operationMenuActivated( int );
    void operationMenuAboutToShow();
    void toolMenuAboutToShow();
    void activatePreviousWindow(); // ### remove in Qt 4.0
    void dockWindowsShow();
    void scrollBarChanged();

private:
    void init();
    void handleUndock( QDockWindow* w);
    void insertIcon( QWidget* w);
    void removeIcon( QWidget* w);
    void place( QWidget* );

    QWorkspaceChild* findChild( QWidget* w);
    void showMaximizeControls();
    void hideMaximizeControls();
    void activateWindow( QWidget* w, bool change_focus = TRUE );
    void showWindow( QWidget* w);
    void maximizeWindow( QWidget* w);
    void minimizeWindow( QWidget* w);
    void normalizeWindow( QWidget* w);

    QRect updateWorkspace();

    QPopupMenu* popup;
    QWorkspacePrivate* d;

    friend class QWorkspacePrivate;
    friend class QWorkspaceChild;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWorkspace( const QWorkspace & );
    QWorkspace& operator=( const QWorkspace & );
#endif
};


#endif // QT_NO_WORKSPACE

#endif // QWORKSPACE_H
