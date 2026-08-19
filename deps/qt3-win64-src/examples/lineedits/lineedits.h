/****************************************************************************
** $Id: lineedits.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef LINEDITS_H
#define LINEDITS_H

#include <qgroupbox.h>

class QLineEdit;
class QComboBox;

class LineEdits : public QGroupBox
{
    Q_OBJECT

public:
    LineEdits( QWidget *parent = 0, const char *name = 0 );

protected:
    QLineEdit *lined1, *lined2, *lined3, *lined4, *lined5;
    QComboBox *combo1, *combo2, *combo3, *combo4, *combo5;

protected slots:
    void slotEchoChanged( int );
    void slotValidatorChanged( int );
    void slotAlignmentChanged( int );
    void slotInputMaskChanged( int );
    void slotReadOnlyChanged( int );
};

#endif
