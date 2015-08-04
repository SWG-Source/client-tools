/****************************************************************************
** $Id: qt/qxtwidget.h   3.3.4   edited May 27 2003 $
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

#ifndef QXTWIDGET_H
#define QXTWIDGET_H

#include <qwidget.h>

#include <X11/Intrinsic.h>


class QXtWidget : public QWidget {
    Q_OBJECT
    Widget xtw;
    Widget xtparent;
    bool   need_reroot;
    void init(const char* name, WidgetClass widget_class,
	      Widget parent, QWidget* qparent,
	      ArgList args, Cardinal num_args,
	      bool managed);
    friend void qwidget_realize( Widget widget, XtValueMask* mask,
				 XSetWindowAttributes* attributes );

public:
    QXtWidget(const char* name, Widget parent, bool managed=FALSE);
    QXtWidget(const char* name, WidgetClass widget_class,
	      QWidget *parent=0, ArgList args=0, Cardinal num_args=0,
	      bool managed=FALSE);
    ~QXtWidget();

    Widget xtWidget() const { return xtw; }
    bool isActiveWindow() const;
    void setActiveWindow();

protected:
    void moveEvent( QMoveEvent* );
    void resizeEvent( QResizeEvent* );
    bool x11Event( XEvent * );
};

#endif // QXTWIDGET_H
