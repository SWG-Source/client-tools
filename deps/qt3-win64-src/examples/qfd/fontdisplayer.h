/****************************************************************************
** $Id: fontdisplayer.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef FontDisplayer_H
#define FontDisplayer_H

#include <qframe.h>
#include <qmainwindow.h>

class QSlider;

class FontRowTable : public QFrame {
    Q_OBJECT
public:
    FontRowTable( QWidget* parent=0, const char* name=0 );

    QSize sizeHint() const;

signals:
    void fontInformation(const QString&);

public slots:
    void setRow(int);
    void chooseFont();


protected:
    QSize cellSize() const;
    void paintEvent( QPaintEvent* );
private:
    QFont tablefont;
    int row;
};

class FontDisplayer : public QMainWindow {
    Q_OBJECT
public:
    FontDisplayer( QWidget* parent=0, const char* name=0 );
};

#endif
