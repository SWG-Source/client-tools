/****************************************************************************
** $Id: aclock.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef ACLOCK_H
#define ACLOCK_H

#include <qwidget.h>
#include <qdatetime.h>

class QTimer;
class AnalogClock : public QWidget		// analog clock widget
{
    Q_OBJECT
public:
    AnalogClock( QWidget *parent=0, const char *name=0 );
    void setAutoMask(bool b);
    
protected:
    void updateMask();
    void paintEvent( QPaintEvent *);
    void mousePressEvent( QMouseEvent *);
    void mouseMoveEvent( QMouseEvent *);
    void drawClock( QPainter* );
    
private slots:
    void timeout();

public slots:
    void setTime( const QTime & t );

private:
    QPoint clickPos;
    QTime time;
    QTimer *internalTimer;
};


#endif // ACLOCK_H
