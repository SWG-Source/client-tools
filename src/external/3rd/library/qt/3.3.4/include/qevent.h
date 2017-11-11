/****************************************************************************
** $Id: qt/qevent.h   3.3.4   edited Feb 24 2004 $
**
** Definition of event classes
**
** Created : 931029
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#ifndef QEVENT_H
#define QEVENT_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qregion.h"
#include "qnamespace.h"
#include "qmime.h"
#include "qpair.h"
#endif // QT_H

class Q_EXPORT QEvent: public Qt		// event base class
{
public:
    enum Type {

	/*
	  If you get a strange compiler error on the line with None,
	  it's probably because you're also including X11 headers,
	  which #define the symbol None. Put the X11 includes after
	  the Qt includes to solve this problem.
	*/

	None = 0,				// invalid event


	Timer = 1,				// timer event
	MouseButtonPress = 2,			// mouse button pressed
	MouseButtonRelease = 3,			// mouse button released
	MouseButtonDblClick = 4,		// mouse button double click
	MouseMove = 5,				// mouse move
	KeyPress = 6,				// key pressed
	KeyRelease = 7,				// key released
	FocusIn = 8,				// keyboard focus received
	FocusOut = 9,				// keyboard focus lost
	Enter = 10,				// mouse enters widget
	Leave = 11,				// mouse leaves widget
	Paint = 12,				// paint widget
	Move = 13,				// move widget
	Resize = 14,				// resize widget
	Create = 15,				// after object creation
	Destroy = 16,				// during object destruction
	Show = 17,				// widget is shown
	Hide = 18,				// widget is hidden
	Close = 19,				// request to close widget
	Quit = 20,				// request to quit application
	Reparent = 21,				// widget has been reparented
	ShowMinimized = 22,		       	// widget is shown minimized
	ShowNormal = 23,	       		// widget is shown normal
	WindowActivate = 24,	       		// window was activated
	WindowDeactivate = 25,	       		// window was deactivated
	ShowToParent = 26,	       		// widget is shown to parent
	HideToParent = 27,	       		// widget is hidden to parent
	ShowMaximized = 28,		       	// widget is shown maximized
	ShowFullScreen = 29,			// widget is shown full-screen
	Accel = 30,				// accelerator event
	Wheel = 31,				// wheel event
	AccelAvailable = 32,			// accelerator available event
	CaptionChange = 33,			// caption changed
	IconChange = 34,			// icon changed
	ParentFontChange = 35,			// parent font changed
	ApplicationFontChange = 36,		// application font changed
	ParentPaletteChange = 37,		// parent palette changed
	ApplicationPaletteChange = 38,		// application palette changed
	PaletteChange = 39,			// widget palette changed
	Clipboard = 40,				// internal clipboard event
	Speech = 42,				// reserved for speech input
	SockAct = 50,				// socket activation
	AccelOverride = 51,			// accelerator override event
	DeferredDelete = 52,			// deferred delete event
	DragEnter = 60,				// drag moves into widget
	DragMove = 61,				// drag moves in widget
	DragLeave = 62,				// drag leaves or is cancelled
	Drop = 63,				// actual drop
	DragResponse = 64,			// drag accepted/rejected
	ChildInserted = 70,			// new child widget
	ChildRemoved = 71,			// deleted child widget
	LayoutHint = 72,			// child min/max size changed
	ShowWindowRequest = 73,			// widget's window should be mapped
	WindowBlocked = 74,			// window is about to be blocked modally
	WindowUnblocked = 75,			// windows modal blocking has ended
	ActivateControl = 80,			// ActiveX activation
	DeactivateControl = 81,			// ActiveX deactivation
	ContextMenu = 82,			// context popup menu
	IMStart = 83,				// input method composition start
	IMCompose = 84,				// input method composition
	IMEnd = 85,				// input method composition end
	Accessibility = 86,			// accessibility information is requested
	TabletMove = 87,			// Wacom tablet event
	LocaleChange = 88,			// the system locale changed
	LanguageChange = 89,			// the application language changed
	LayoutDirectionChange = 90,		// the layout direction changed
	Style = 91,				// internal style event
	TabletPress = 92,			// tablet press
	TabletRelease = 93,			// tablet release
	OkRequest = 94,				// CE (Ok) button pressed
	HelpRequest = 95,			// CE (?)  button pressed
	WindowStateChange = 96,			// window state has changed
	IconDrag = 97,                          // proxy icon dragged
	User = 1000,				// first user event id
	MaxUser = 65535				// last user event id
    };


    QEvent( Type type ) : t(type), posted(FALSE), spont(FALSE) {}
    virtual ~QEvent();
    Type  type() const	{ return t; }
    bool spontaneous() const 	{ return spont; }
protected:
    Type  t;
private:
    uint posted : 1;
    uint spont : 1;


    friend class QApplication;
    friend class QAccelManager;
    friend class QBaseApplication;
    friend class QETWidget;
};


class Q_EXPORT QTimerEvent : public QEvent
{
public:
    QTimerEvent( int timerId )
	: QEvent(Timer), id(timerId) {}
    int	  timerId()	const	{ return id; }
protected:
    int	  id;
};


class Q_EXPORT QMouseEvent : public QEvent
{
public:
    QMouseEvent( Type type, const QPoint &pos, int button, int state );

    QMouseEvent( Type type, const QPoint &pos, const QPoint&globalPos,
		 int button, int state )
	: QEvent(type), p(pos), g(globalPos), b((ushort)button),s((ushort)state),accpt(TRUE) {};

    const QPoint &pos() const	{ return p; }
    const QPoint &globalPos() const { return g; }
    int	   x()		const	{ return p.x(); }
    int	   y()		const	{ return p.y(); }
    int	   globalX()	const	{ return g.x(); }
    int	   globalY()	const	{ return g.y(); }
    ButtonState button() const	{ return (ButtonState) b; }
    ButtonState state()	const	{ return (ButtonState) s; }
    ButtonState stateAfter() const;
    bool   isAccepted() const	{ return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
protected:
    QPoint p;
    QPoint g;
    ushort b;
    ushort s;
    uint   accpt:1;
};


#ifndef QT_NO_WHEELEVENT
class Q_EXPORT QWheelEvent : public QEvent
{
public:
    QWheelEvent( const QPoint &pos, int delta, int state, Orientation orient = Vertical );
    QWheelEvent( const QPoint &pos, const QPoint& globalPos, int delta, int state, Orientation orient = Vertical  )
	: QEvent(Wheel), p(pos), g(globalPos), d(delta), s((ushort)state),
	  accpt(TRUE), o(orient) {}
    int	   delta()	const	{ return d; }
    const QPoint &pos() const	{ return p; }
    const QPoint &globalPos() const	{ return g; }
    int	   x()		const	{ return p.x(); }
    int	   y()		const	{ return p.y(); }
    int	   globalX()	const	{ return g.x(); }
    int	   globalY()	const	{ return g.y(); }
    ButtonState state()	const	{ return ButtonState(s); }
    Orientation orientation() 	const 	{ return o; }
    bool   isAccepted() const	{ return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
protected:
    QPoint p;
    QPoint g;
    int d;
    ushort s;
    bool   accpt;
    Orientation o;
};
#endif

class Q_EXPORT QTabletEvent : public QEvent
{
public:
    enum TabletDevice { NoDevice = -1, Puck, Stylus, Eraser };
    QTabletEvent( Type t, const QPoint &pos, const QPoint &globalPos, int device,
		  int pressure, int xTilt, int yTilt, const QPair<int,int> &uId );
    QTabletEvent( const QPoint &pos, const QPoint &globalPos, int device,
		  int pressure, int xTilt, int yTilt, const QPair<int,int> &uId )
	: QEvent( TabletMove ), mPos( pos ), mGPos( globalPos ), mDev( device ),
	  mPress( pressure ), mXT( xTilt ), mYT( yTilt ), mType( uId.first ),
	  mPhy( uId.second ), mbAcc(TRUE)
    {}
    int pressure()	const { return mPress; }
    int xTilt()		const { return mXT; }
    int yTilt()		const { return mYT; }
    const QPoint &pos()	const { return mPos; }
    const QPoint &globalPos() const { return mGPos; }
    int x()		const { return mPos.x(); }
    int y()		const { return mPos.y(); }
    int globalX()	const { return mGPos.x(); }
    int globalY()	const { return mGPos.y(); }
    TabletDevice device() 	const { return TabletDevice(mDev); }
    int isAccepted() const { return mbAcc; }
    void accept() { mbAcc = TRUE; }
    void ignore() { mbAcc = FALSE; }
    QPair<int,int> uniqueId() { return QPair<int,int>( mType, mPhy); }
protected:
    QPoint mPos;
    QPoint mGPos;
    int mDev,
	mPress,
	mXT,
	mYT,
	mType,
	mPhy;
    bool mbAcc;

};

class Q_EXPORT QKeyEvent : public QEvent
{
public:
    QKeyEvent( Type type, int key, int ascii, int state,
		const QString& text=QString::null, bool autorep=FALSE, ushort count=1 )
	: QEvent(type), txt(text), k((ushort)key), s((ushort)state),
	    a((uchar)ascii), accpt(TRUE), autor(autorep), c(count)
    {
	if ( key >= Key_Back && key <= Key_MediaLast )
	    accpt = FALSE;
    }
    int	   key()	const	{ return k; }
    int	   ascii()	const	{ return a; }
    ButtonState state()	const	{ return ButtonState(s); }
    ButtonState stateAfter() const;
    bool   isAccepted() const	{ return accpt; }
    QString text()      const   { return txt; }
    bool   isAutoRepeat() const	{ return autor; }
    int   count() const	{ return int(c); }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }

protected:
    QString txt;
    ushort k, s;
    uchar  a;
    uint   accpt:1;
    uint   autor:1;
    ushort c;
};


class Q_EXPORT QFocusEvent : public QEvent
{
public:

    QFocusEvent( Type type )
	: QEvent(type) {}

    bool   gotFocus()	const { return type() == FocusIn; }
    bool   lostFocus()	const { return type() == FocusOut; }

    enum Reason { Mouse, Tab, Backtab, ActiveWindow, Popup, Shortcut, Other };
    static Reason reason();
    static void setReason( Reason reason );
    static void resetReason();

private:
    static Reason m_reason;
    static Reason prev_reason;
};


class Q_EXPORT QPaintEvent : public QEvent
{
public:
    QPaintEvent( const QRegion& paintRegion, bool erased = TRUE)
	: QEvent(Paint),
	  rec(paintRegion.boundingRect()),
	  reg(paintRegion),
	  erase(erased){}
    QPaintEvent( const QRect &paintRect, bool erased = TRUE )
	: QEvent(Paint),
	  rec(paintRect),
	  reg(paintRect),
	  erase(erased){}
    QPaintEvent( const QRegion &paintRegion, const QRect &paintRect, bool erased = TRUE )
	: QEvent(Paint),
	  rec(paintRect),
	  reg(paintRegion),
	  erase(erased){}

    const QRect &rect() const	  { return rec; }
    const QRegion &region() const { return reg; }
    bool erased() const { return erase; }
protected:
    friend class QApplication;
    friend class QBaseApplication;
    QRect rec;
    QRegion reg;
    bool erase;
};


class Q_EXPORT QMoveEvent : public QEvent
{
public:
    QMoveEvent( const QPoint &pos, const QPoint &oldPos )
	: QEvent(Move), p(pos), oldp(oldPos) {}
    const QPoint &pos()	  const { return p; }
    const QPoint &oldPos()const { return oldp;}
protected:
    QPoint p, oldp;
    friend class QApplication;
    friend class QBaseApplication;
};


class Q_EXPORT QResizeEvent : public QEvent
{
public:
    QResizeEvent( const QSize &size, const QSize &oldSize )
	: QEvent(Resize), s(size), olds(oldSize) {}
    const QSize &size()	  const { return s; }
    const QSize &oldSize()const { return olds;}
protected:
    QSize s, olds;
    friend class QApplication;
    friend class QBaseApplication;
};


class Q_EXPORT QCloseEvent : public QEvent
{
public:
    QCloseEvent()
	: QEvent(Close), accpt(FALSE) {}
    bool   isAccepted() const	{ return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
protected:
    bool   accpt;
};


class Q_EXPORT QIconDragEvent : public QEvent
{
public:
    QIconDragEvent() 
	: QEvent(IconDrag), accpt(FALSE) {}

    bool   isAccepted() const	{ return accpt; }
    void   accept()		{ accpt = TRUE; }
    void   ignore()		{ accpt = FALSE; }
protected:
    bool   accpt;
};

class Q_EXPORT QShowEvent : public QEvent
{
public:
    QShowEvent()
	: QEvent(Show) {}
};


class Q_EXPORT QHideEvent : public QEvent
{
public:
    QHideEvent()
	: QEvent(Hide) {}
};

class Q_EXPORT QContextMenuEvent : public QEvent
{
public:
    enum Reason { Mouse, Keyboard, Other };
    QContextMenuEvent( Reason reason, const QPoint &pos, const QPoint &globalPos, int state )
	: QEvent( ContextMenu ), p( pos ), gp( globalPos ), accpt( TRUE ), consum( TRUE ),
	reas( reason ), s((ushort)state) {}
    QContextMenuEvent( Reason reason, const QPoint &pos, int state );

    int	    x() const { return p.x(); }
    int	    y() const { return p.y(); }
    int	    globalX() const { return gp.x(); }
    int	    globalY() const { return gp.y(); }

    const QPoint&   pos() const { return p; }
    const QPoint&   globalPos() const { return gp; }

    ButtonState state()	const	{ return (ButtonState) s; }
    bool    isAccepted() const	{ return accpt; }
    bool    isConsumed() const	{ return consum; }
    void    consume()		{ accpt = FALSE; consum = TRUE; }
    void    accept()		{ accpt = TRUE; consum = TRUE; }
    void    ignore()		{ accpt = FALSE; consum = FALSE; }

    Reason  reason() const { return Reason( reas ); }

protected:
    QPoint  p;
    QPoint  gp;
    bool    accpt;
    bool    consum;
    uint    reas:8;
    ushort s;
};


class Q_EXPORT QIMEvent : public QEvent
{
public:
    QIMEvent( Type type, const QString &text, int cursorPosition )
	: QEvent(type), txt(text), cpos(cursorPosition), a(TRUE) {}
    const QString &text() const { return txt; }
    int cursorPos() const { return cpos; }
    bool isAccepted() const { return a; }
    void accept() { a = TRUE; }
    void ignore() { a = FALSE; }
    int selectionLength() const;

private:
    QString txt;
    int cpos;
    bool a;
};

class Q_EXPORT QIMComposeEvent : public QIMEvent
{
public:
    QIMComposeEvent( Type type, const QString &text, int cursorPosition,
		     int selLength )
	: QIMEvent( type, text, cursorPosition ), selLen( selLength ) { }

private:
    int selLen;

    friend class QIMEvent;
};

inline int QIMEvent::selectionLength() const
{
    if ( type() != IMCompose ) return 0;
    QIMComposeEvent *that = (QIMComposeEvent *) this;
    return that->selLen;
}


#ifndef QT_NO_DRAGANDDROP

// This class is rather closed at the moment.  If you need to create your
// own DND event objects, write to qt-bugs@trolltech.com and we'll try to
// find a way to extend it so it covers your needs.

class Q_EXPORT QDropEvent : public QEvent, public QMimeSource
{
public:
    QDropEvent( const QPoint& pos, Type typ=Drop )
	: QEvent(typ), p(pos),
	  act(0), accpt(0), accptact(0), resv(0),
	  d(0)
	{}
    const QPoint &pos() const	{ return p; }
    bool isAccepted() const	{ return accpt || accptact; }
    void accept(bool y=TRUE)	{ accpt = y; }
    void ignore()		{ accpt = FALSE; }

    bool isActionAccepted() const { return accptact; }
    void acceptAction(bool y=TRUE) { accptact = y; }
    enum Action { Copy, Link, Move, Private, UserAction=100 };
    void setAction( Action a ) { act = (uint)a; }
    Action action() const { return Action(act); }

    QWidget* source() const;
    const char* format( int n = 0 ) const;
    QByteArray encodedData( const char* ) const;
    bool provides( const char* ) const;

    QByteArray data(const char* f) const { return encodedData(f); }

    void setPoint( const QPoint& np ) { p = np; }

protected:
    QPoint p;
    uint act:8;
    uint accpt:1;
    uint accptact:1;
    uint resv:5;
    void * d;
};



class Q_EXPORT QDragMoveEvent : public QDropEvent
{
public:
    QDragMoveEvent( const QPoint& pos, Type typ=DragMove )
	: QDropEvent(pos,typ),
	  rect( pos, QSize( 1, 1 ) ) {}
    QRect answerRect() const { return rect; }
    void accept( bool y=TRUE ) { QDropEvent::accept(y); }
    void accept( const QRect & r) { accpt = TRUE; rect = r; }
    void ignore( const QRect & r) { accpt =FALSE; rect = r; }
    void ignore()		{ QDropEvent::ignore(); }

protected:
    QRect rect;
};


class Q_EXPORT QDragEnterEvent : public QDragMoveEvent
{
public:
    QDragEnterEvent( const QPoint& pos ) :
	QDragMoveEvent(pos, DragEnter) { }
};


/* An internal class */
class Q_EXPORT QDragResponseEvent : public QEvent
{
public:
    QDragResponseEvent( bool accepted )
	: QEvent(DragResponse), a(accepted) {}
    bool   dragAccepted() const	{ return a; }
protected:
    bool a;
};


class Q_EXPORT QDragLeaveEvent : public QEvent
{
public:
    QDragLeaveEvent()
	: QEvent(DragLeave) {}
};

#endif // QT_NO_DRAGANDDROP

class Q_EXPORT QChildEvent : public QEvent
{
public:
    QChildEvent( Type type, QObject *child )
	: QEvent(type), c(child) {}
    QObject *child() const	{ return c; }
    bool inserted() const { return t == ChildInserted; }
    bool removed() const { return t == ChildRemoved; }
protected:
    QObject *c;
};


class Q_EXPORT QCustomEvent : public QEvent
{
public:
    QCustomEvent( int type );
    QCustomEvent( Type type, void *data )
	: QEvent(type), d(data) {};
    void       *data()	const	{ return d; }
    void	setData( void* data )	{ d = data; }
private:
    void       *d;
};

#endif // QEVENT_H
