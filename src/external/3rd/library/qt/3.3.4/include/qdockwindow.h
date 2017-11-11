/****************************************************************************
** $Id: qt/qdockwindow.h   3.3.4   edited Jul 9 2003 $
**
** Definition of the QDockWindow class
**
** Created : 001010
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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

#ifndef QDOCKWINDOW_H
#define QDOCKWINDOW_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

#ifndef QT_NO_MAINWINDOW

class QDockWindowHandle;
class QDockWindowTitleBar;
class QPainter;
class QDockWindowResizeHandle;
class QBoxLayout;
class QHBoxLayout;
class QVBoxLayout;
class QDockArea;
class QWidgetResizeHandler;
class QMainWindow;
class QDockAreaLayout;
class QDockWindowPrivate;
class QToolBar;
class QWindowsXPStyle;

class Q_EXPORT QDockWindow : public QFrame
{
    Q_OBJECT
    Q_ENUMS( CloseMode Place )
    Q_PROPERTY( int closeMode READ closeMode  WRITE setCloseMode ) //### this shouldn't be of type int?!
    Q_PROPERTY( bool resizeEnabled READ isResizeEnabled  WRITE setResizeEnabled )
    Q_PROPERTY( bool movingEnabled READ isMovingEnabled  WRITE setMovingEnabled )
    Q_PROPERTY( bool horizontallyStretchable READ isHorizontallyStretchable  WRITE setHorizontallyStretchable )
    Q_PROPERTY( bool verticallyStretchable READ isVerticallyStretchable  WRITE setVerticallyStretchable )
    Q_PROPERTY( bool stretchable READ isStretchable )
    Q_PROPERTY( bool newLine READ newLine  WRITE setNewLine )
    Q_PROPERTY( bool opaqueMoving READ opaqueMoving  WRITE setOpaqueMoving )
    Q_PROPERTY( int offset READ offset  WRITE setOffset )
    Q_PROPERTY( Place place READ place )

    friend class QDockWindowHandle;
    friend class QDockWindowTitleBar;
    friend class QDockArea;
    friend class QDockAreaLayout;
    friend class QMainWindow;
    friend class QCEMainWindow;
    friend class QToolBar;
    friend class QWindowsXPStyle;

public:
    enum Place { InDock, OutsideDock };
    enum CloseMode { Never = 0, Docked = 1, Undocked = 2, Always = Docked | Undocked };

    QDockWindow( Place p = InDock, QWidget* parent=0, const char* name=0, WFlags f = 0 );
    QDockWindow( QWidget* parent, const char* name=0, WFlags f = 0 );
    ~QDockWindow();

    virtual void setWidget( QWidget *w );
    QWidget *widget() const;

    Place place() const { return curPlace; }

    QDockArea *area() const;

    virtual void setCloseMode( int m );
    bool isCloseEnabled() const;
    int closeMode() const;

    virtual void setResizeEnabled( bool b );
    virtual void setMovingEnabled( bool b );
    bool isResizeEnabled() const;
    bool isMovingEnabled() const;

    virtual void setHorizontallyStretchable( bool b );
    virtual void setVerticallyStretchable( bool b );
    bool isHorizontallyStretchable() const;
    bool isVerticallyStretchable() const;
    void setHorizontalStretchable( bool b ) { setHorizontallyStretchable( b ); }
    void setVerticalStretchable( bool b ) { setVerticallyStretchable( b ); }
    bool isHorizontalStretchable() const { return isHorizontallyStretchable(); }
    bool isVerticalStretchable() const { return isVerticallyStretchable(); }
    bool isStretchable() const;

    virtual void setOffset( int o );
    int offset() const;

    virtual void setFixedExtentWidth( int w );
    virtual void setFixedExtentHeight( int h );
    QSize fixedExtent() const;

    virtual void setNewLine( bool b );
    bool newLine() const;

    Qt::Orientation orientation() const;

    QSize sizeHint() const;
    QSize minimumSize() const;
    QSize minimumSizeHint() const;

    QBoxLayout *boxLayout();

    virtual void setOpaqueMoving( bool b );
    bool opaqueMoving() const;

    bool eventFilter( QObject *o, QEvent *e );

#ifdef QT_NO_WIDGET_TOPEXTRA
    QString caption() const;
#endif

signals:
    void orientationChanged( Orientation o );
    void placeChanged( QDockWindow::Place p );
    void visibilityChanged( bool );

public slots:
    virtual void undock( QWidget *w );
    virtual void undock() { undock( 0 ); }
    virtual void dock();
    virtual void setOrientation( Orientation o );
    void setCaption( const QString &s );

protected:
    void resizeEvent( QResizeEvent *e );
    void showEvent( QShowEvent *e );
    void hideEvent( QHideEvent *e );
    void contextMenuEvent( QContextMenuEvent *e );

    void drawFrame( QPainter * );
    void drawContents( QPainter * );

    bool event( QEvent *e );

private slots:
    void toggleVisible() { if ( !isVisible() ) show(); else hide(); }

private:
    QDockWindow( Place p, QWidget* parent, const char* name, WFlags f, bool toolbar );

    void handleMove( const QPoint &pos, const QPoint &gp, bool drawRect );
    void updateGui();
    void updateSplitterVisibility( bool visible );

    void startRectDraw( const QPoint &so, bool drawRect );
    void endRectDraw( bool drawRect );
    void updatePosition( const QPoint &globalPos  );
    QWidget *areaAt( const QPoint &gp );
    void removeFromDock( bool fixNewLines = TRUE );
    void swapRect( QRect &r, Qt::Orientation o, const QPoint &offset, QDockArea *area );
    void init();

private:
    QDockWindowHandle *horHandle, *verHandle;
    QDockWindowTitleBar *titleBar;
    QWidget *wid;
    QPainter *unclippedPainter;
    QDockArea *dockArea, *tmpDockArea;
    QRect currRect;
    Place curPlace;
    Place state;
    bool resizeEnabled : 1;
    bool moveEnabled : 1;
    bool nl : 1;
    bool opaque : 1;
    bool isToolbar : 1;
    bool stretchable[ 3 ];
    Orientation startOrientation;
    int cMode;
    QPoint startOffset;
    int offs;
    QSize fExtent;
    QDockWindowResizeHandle *hHandleTop, *hHandleBottom, *vHandleLeft, *vHandleRight;
    QVBoxLayout *hbox;
    QHBoxLayout *vbox;
    QBoxLayout *childBox;
    void *dockWindowData;
    QPoint lastPos;
    QSize lastSize;
    QWidgetResizeHandler *widgetResizeHandler;
    QDockWindowPrivate *d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QDockWindow( const QDockWindow & );
    QDockWindow& operator=( const QDockWindow & );
#endif
};

inline QDockArea *QDockWindow::area() const
{
    return dockArea;
}

#define Q_DEFINED_QDOCKWINDOW
#include "qwinexport.h"
#endif

#endif // QDOCKWINDOW_H
