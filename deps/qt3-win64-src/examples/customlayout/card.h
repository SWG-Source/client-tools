/****************************************************************************
** $Id: card.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Definition of simple flow layout for custom layout example
**
** Created : 979899
**
** Copyright (C) 1997-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef CARD_H
#define CARD_H

#include <qlayout.h>
#include <qptrlist.h>

class CardLayout : public QLayout
{
public:
    CardLayout( QWidget *parent, int dist )
	: QLayout( parent, 0, dist ) {}
    CardLayout( QLayout* parent, int dist)
	: QLayout( parent, dist ) {}
    CardLayout( int dist )
	: QLayout( dist ) {}
    ~CardLayout();

    void addItem( QLayoutItem *item );
    QSize sizeHint() const;
    QSize minimumSize() const;
    QLayoutIterator iterator();
    void setGeometry( const QRect &rect );

private:
    QPtrList<QLayoutItem> list;

};

#endif
