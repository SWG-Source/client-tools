/****************************************************************************
** $Id: glbox.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

/****************************************************************************
**
** This is a simple QGLWidget displaying an openGL wireframe box
**
****************************************************************************/

#ifndef GLBOX_H
#define GLBOX_H

#include "glcontrolwidget.h"

class GLBox : public GLControlWidget
{
    Q_OBJECT

public:

    GLBox( QWidget* parent = 0, const char* name = 0, WFlags f = 0 );
    ~GLBox();

protected:

    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );

    virtual GLuint 	makeObject();

private:
    GLuint object;
};


#endif // GLBOX_H
