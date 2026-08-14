/****************************************************************************
** $Id: rubberbandwidget.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Definition of a widget that draws a rubberband. Designed to be used 
** in an X11 overlay visual
**
** Copyright (C) 1999-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef RUBBERBANDWIDGET_H
#define RUBBERBANDWIDGET_H

#include <qwidget.h>


class RubberbandWidget : public QWidget
{
public:
    RubberbandWidget( QColor transparentColor, QWidget *parent=0, 
		      const char *name=0, WFlags f=0 );

protected:
    void mousePressEvent( QMouseEvent* e );
    void mouseMoveEvent( QMouseEvent* e );
    void mouseReleaseEvent( QMouseEvent* e );

    QColor c;
    QPoint p1;
    QPoint p2;
    QPoint p3;
    bool on;
};

#endif
