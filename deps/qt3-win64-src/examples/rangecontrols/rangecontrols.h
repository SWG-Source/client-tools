/****************************************************************************
** $Id: rangecontrols.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef RANGECONTROLS_H
#define RANGECONTROLS_H

#include <qvbox.h>

class QCheckBox;

class RangeControls : public QVBox
{
    Q_OBJECT

public:
    RangeControls( QWidget *parent = 0, const char *name = 0 );

private:
    QCheckBox *notches, *wrapping;
};

#endif
