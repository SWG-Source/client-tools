/****************************************************************************
** $Id: forever.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Definition of something or other
**
** Created : 979899
**
** Copyright (C) 1997-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef FOREVER_H
#define FOREVER_H

#include <qwidget.h>


const int numColors = 120;


class Forever : public QWidget
{
    Q_OBJECT
public:
    Forever( QWidget *parent=0, const char *name=0 );
protected:
    void	paintEvent( QPaintEvent * );
    void	timerEvent( QTimerEvent * );
private slots:
    void	updateCaption();
private:
    int		rectangles;
    QColor	colors[numColors];
};


#endif
