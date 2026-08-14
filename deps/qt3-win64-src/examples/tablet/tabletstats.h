/****************************************************************************
** $Id: tabletstats.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef _TABLET_STATS_
#define _TABLET_STATS_

#include <qwidget.h>
#include <qframe.h>
#include "canvas.h"
#include "tabletstatsbase.h"

class QLabel;

class MyOrientation : public QFrame  
{
    Q_OBJECT
public:
    MyOrientation( QWidget *parent = 0, const char *name = 0 );
    virtual ~MyOrientation();

public slots:
    void newOrient( int tiltX, int tiltY );

};

class StatsCanvas : public Canvas
{
    Q_OBJECT
public:
    StatsCanvas( QWidget *parent = 0, const char* name = 0 );
    ~StatsCanvas();
signals:
    void signalNewPressure( int );
    void signalNewTilt( int, int );
    void signalNewDev( int );
    void signalNewLoc( int, int );

protected:
    void tabletEvent( QTabletEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void paintEvent( QPaintEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );

private:
    QRect r;
};

class TabletStats : public TabletStatsBase
{
    Q_OBJECT
public:
    TabletStats( QWidget *parent, const char* name );
    ~TabletStats();

private slots:
    void slotTiltChanged( int newTiltX, int newTiltY );
    void slotDevChanged( int newDev );
    void slotLocationChanged( int newX, int newY );

protected:
};

#endif
