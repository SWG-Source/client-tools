/****************************************************************************
** $Id: tabdialog.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef TABDIALOG_H
#define TABDIALOG_H

#include <qtabdialog.h>
#include <qstring.h>
#include <qfileinfo.h>

class TabDialog : public QTabDialog
{
    Q_OBJECT

public:
    TabDialog( QWidget *parent, const char *name, const QString &_filename );

protected:
    QString filename;
    QFileInfo fileinfo;

    void setupTab1();
    void setupTab2();
    void setupTab3();

};

#endif
