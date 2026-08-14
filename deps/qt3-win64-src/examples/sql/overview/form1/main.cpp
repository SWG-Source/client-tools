/****************************************************************************
** $Id: main.cpp 2051 2007-02-21 10:04:20Z chehrlic $
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
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qsqlform.h>
#include "../connection.h"

class FormDialog : public QDialog
{
    public:
	FormDialog();
};


FormDialog::FormDialog()
{
    QLabel *forenameLabel   = new QLabel( "Forename:", this );
    QLabel *forenameDisplay = new QLabel( this );
    QLabel *surnameLabel    = new QLabel( "Surname:", this );
    QLabel *surnameDisplay  = new QLabel( this );
    QLabel *salaryLabel	    = new QLabel( "Salary:", this );
    QLineEdit *salaryEdit   = new QLineEdit( this );

    QGridLayout *grid = new QGridLayout( this );
    grid->addWidget( forenameLabel,	0, 0 );
    grid->addWidget( forenameDisplay,	0, 1 );
    grid->addWidget( surnameLabel,	1, 0 );
    grid->addWidget( surnameDisplay,	1, 1 );
    grid->addWidget( salaryLabel,	2, 0 );
    grid->addWidget( salaryEdit,	2, 1 );
    grid->activate();

    QSqlCursor staffCursor( "staff" );
    staffCursor.select();
    staffCursor.next();

    QSqlForm sqlForm( this );
    sqlForm.setRecord( staffCursor.primeUpdate() );
    sqlForm.insert( forenameDisplay, "forename" );
    sqlForm.insert( surnameDisplay, "surname" );
    sqlForm.insert( salaryEdit, "salary" );
    sqlForm.readFields();
}


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( ! createConnections() ) return 1;

    FormDialog *formDialog = new FormDialog();
    formDialog->show();
    app.setMainWidget( formDialog );

    return app.exec();
}
