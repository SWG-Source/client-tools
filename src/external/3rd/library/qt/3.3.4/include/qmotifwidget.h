/****************************************************************************
** $Id: qt/qmotifwidget.h   3.3.4   edited Sep 6 13:06 $
**
** Definition of Qt extension classes for Xt/Motif support.
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt extension for Xt/Motif support.
**
** Licensees holding valid Qt Enterprise Edition licenses for X11 may use
** this file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QMOTIFWIDGET_H
#define QMOTIFWIDGET_H

#include <qwidget.h>

#include <X11/Intrinsic.h>

class QMotifWidgetPrivate;
class QKeyEvent;

class QMotifWidget : public QWidget
{
    Q_OBJECT

public:
    QMotifWidget( QWidget *, WidgetClass, ArgList = NULL, Cardinal = 0,
                  const char * = 0, WFlags = 0 );
    virtual ~QMotifWidget();

    Widget motifWidget() const;

    void show();
    void hide();

protected:
    bool event( QEvent * );
    bool eventFilter( QObject *object, QEvent *event );
    bool x11Event(XEvent *event);

private:
    QMotifWidgetPrivate *d;

    void realize( Widget );

    friend void qmotif_widget_shell_destroy(Widget w);
    friend void qmotif_widget_shell_realize( Widget, XtValueMask *,
                                             XSetWindowAttributes *);
    friend void qmotif_widget_shell_change_managed( Widget );
    static bool dispatchQEvent( QEvent*, QWidget*);
    friend class QMotifDialog;
};

#endif // QMOTIFWIDGET_H
