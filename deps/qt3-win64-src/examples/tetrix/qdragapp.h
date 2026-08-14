/****************************************************************************
** $Id: qdragapp.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef QDRAGAPP_H
#define QDRAGAPP_H

#include "qapplication.h"

class QDragger;

class QDragApplication : public QApplication
{
    Q_OBJECT
public:
    QDragApplication( int &argc, char **argv );
    virtual ~QDragApplication();

    virtual bool notify( QObject *, QEvent * ); // event filter

private:
    QDragger *dragger;
};


#endif // QDRAGAPP_H
