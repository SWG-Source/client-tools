/****************************************************************************
** $Id: tooltip.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <qtooltip.h>


class QTable;
class QHeader;

class HeaderToolTip : public QToolTip
{
public:
    HeaderToolTip( QHeader *header, QToolTipGroup *group = 0 );
    
protected:
    void maybeTip ( const QPoint &p );
};

class TableToolTip : public QToolTip
{
public:
    TableToolTip( QTable* table, QToolTipGroup *group = 0  );
    
protected:
    void maybeTip( const QPoint &p );
    
private:
    QTable *table;
};


#endif
