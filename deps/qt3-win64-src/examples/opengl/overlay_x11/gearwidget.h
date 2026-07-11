/****************************************************************************
** $Id: gearwidget.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Definition of a simple Qt OpenGL widget
**
** Copyright (C) 1999-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef GEAR_H
#define GEAR_H

#include <qgl.h>

class GearWidget : public QGLWidget
{
public:
    GearWidget( QWidget *parent=0, const char *name=0 );

protected:
    void initializeGL();
    void resizeGL( int, int );
    void paintGL();
};


#endif
