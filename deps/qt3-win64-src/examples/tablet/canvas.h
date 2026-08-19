/****************************************************************************
** $Id: canvas.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qpen.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpointarray.h>
#include <qwidget.h>


#ifndef _MY_CANVAS_
#define _MY_CANVAS_


class Canvas : public QWidget
{
    Q_OBJECT

public:
    Canvas( QWidget *parent = 0, const char *name = 0, WFlags fl = 0 );
    virtual ~Canvas() {};

    void setPenColor( const QColor &c )
    { 	saveColor = c;
	pen.setColor( saveColor ); }

    void setPenWidth( int w )
    { pen.setWidth( w ); }

    QColor penColor()
    { return pen.color(); }

    int penWidth()
    { return pen.width(); }

    void save( const QString &filename, const QString &format );

    void clearScreen();

protected:
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent *e );
    virtual void mouseMoveEvent( QMouseEvent *e );
    virtual void resizeEvent( QResizeEvent *e );
    virtual void paintEvent( QPaintEvent *e );
    virtual void tabletEvent( QTabletEvent *e );

    QPen pen;
    QPointArray polyline;

    bool mousePressed;
    int oldPressure;
    QColor saveColor;

    QPixmap buffer;

};

#endif
