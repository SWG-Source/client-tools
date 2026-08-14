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
#include <qdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qsqlform.h>
#include <qsqlpropertymap.h>
#include "../connection.h"

class CustomEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY( QString upperLine READ upperLine WRITE setUpperLine )
    public:
	CustomEdit( QWidget *parent=0, const char *name=0 );
	QString upperLine() const;
	void setUpperLine( const QString &line );
    public slots:
	void changed( const QString &line );
    private:
	QString upperLineText;
};


class FormDialog : public QDialog
{
    Q_OBJECT
    public:
	FormDialog();
	~FormDialog();
    public slots:
	void save();
    private:
	QSqlCursor *staffCursor;
	QSqlForm *sqlForm;
	QSqlPropertyMap *propMap;
	QSqlIndex idIndex;
};


