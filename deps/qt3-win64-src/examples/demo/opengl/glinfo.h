/****************************************************************************
** $Id: glinfo.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef GLINFO_H
#define GLINFO_H

#include <qstring.h>

class GLInfo
{
public:
    GLInfo();
    QString info();//extensions();
/*     QString configs(); */

protected:
    QString infotext;
/*     QString config; */
};
#endif
