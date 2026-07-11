/****************************************************************************
** $Id: qtetrix.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef QTETRIX_H
#define QTETRIX_H

#include "qtetrixb.h"
#include <qframe.h>
#include <qlcdnumber.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpainter.h>


class ShowNextPiece : public QFrame
{
    Q_OBJECT
    friend class QTetrix;
public:
    ShowNextPiece( QWidget *parent=0, const char *name=0  );
public slots:
    void drawNextSquare( int x, int y,QColor *color );
signals:
    void update();
private:
    void paintEvent( QPaintEvent * );
    void resizeEvent( QResizeEvent * );
    
    int      blockWidth,blockHeight;
    int      xOffset,yOffset;
};


class QTetrix : public QWidget
{
    Q_OBJECT
public:
    QTetrix( QWidget *parent=0, const char *name=0 );
    void startGame() { board->startGame(); }

public slots:
    void gameOver();
    void quit();
private:
    void keyPressEvent( QKeyEvent *e ) { board->keyPressEvent(e); }

    QTetrixBoard  *board;
    ShowNextPiece *showNext;
#ifndef QT_NO_LCDNUMBER
    QLCDNumber    *showScore;
    QLCDNumber    *showLevel;
    QLCDNumber    *showLines;
#else
    QLabel    *showScore;
    QLabel    *showLevel;
    QLabel    *showLines;
#endif
    QPushButton   *quitButton;
    QPushButton   *startButton;
    QPushButton   *pauseButton;
};


void drawTetrixButton( QPainter *, int x, int y, int w, int h,
		       const QColor *color, QWidget *widg);


#endif
