/****************************************************************************
** $Id: secret.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Custom MIME type implementation example
**
** Created : 979899
**
** Copyright (C) 1997-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef SECRETDRAG_H
#define SECRETDRAG_H

#include <qdragobject.h>
#include <qlabel.h>

class SecretDrag: public QStoredDrag {
public:
    SecretDrag( uchar, QWidget * parent = 0, const char * name = 0 );
    ~SecretDrag() {};

    static bool canDecode( QDragMoveEvent* e );
    static bool decode( QDropEvent* e, QString& s );
};


class SecretSource: public QLabel
{
public:
    SecretSource( int secret, QWidget *parent = 0, const char * name = 0 );
    ~SecretSource();

protected:
    void mousePressEvent( QMouseEvent * );
private:
    int mySecret;
};

#endif
