/****************************************************************************
** $Id: main.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qcombobox.h>
#include <qmap.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qsqleditorfactory.h>
#include <qsqlpropertymap.h>
#include "../connection.h"

class StatusPicker : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY( int statusid READ statusId WRITE setStatusId )
    public:
	StatusPicker( QWidget *parent=0, const char *name=0 );
	int statusId() const;
	void setStatusId( int id );
    private:
	QMap< int, int > index2id;
};


class CustomSqlEditorFactory : public QSqlEditorFactory
{
    Q_OBJECT
    public:
	QWidget *createEditor( QWidget *parent, const QSqlField *field );
};



