/****************************************************************************
** $Id: infodata.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef INFODATA_H
#define INFODATA_H

#include <qdict.h>
#include <qstringlist.h>


// The InfoData class manages data, organized in tree structure.
class InfoData
{
public:
    InfoData();
    QStringList list( QString path, bool *found ) const;
    QString get( QString path, bool *found ) const;

private:
    QDict< QStringList > nodes;
    QDict< QString > data;
};

#endif // INFODATA_H

