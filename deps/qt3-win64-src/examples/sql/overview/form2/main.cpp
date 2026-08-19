/****************************************************************************
** $Id: main.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "main.h"

FormDialog::FormDialog()
    : staffCursor( "staff" )
{
    staffCursor.setTrimmed( "forename", TRUE );
    staffCursor.setTrimmed( "surname",  TRUE );

    QLabel	*forenameLabel = new QLabel( "Forename:", this );
    QLineEdit	*forenameEdit  = new QLineEdit( this );
    QLabel	*surnameLabel  = new QLabel( "Surname:", this );
    QLineEdit	*surnameEdit   = new QLineEdit( this );
    QLabel	*salaryLabel   = new QLabel( "Salary:", this );
    QLineEdit	*salaryEdit    = new QLineEdit( this );
    QPushButton *saveButton    = new QPushButton( "&Save", this );
    connect( saveButton, SIGNAL(clicked()), this, SLOT(save()) );

    QGridLayout *grid = new QGridLayout( this );
    grid->addWidget( forenameLabel, 0, 0 );
    grid->addWidget( forenameEdit,  0, 1 );
    grid->addWidget( surnameLabel,  1, 0 );
    grid->addWidget( surnameEdit,   1, 1 );
    grid->addWidget( salaryLabel,   2, 0 );
    grid->addWidget( salaryEdit,    2, 1 );
    grid->addWidget( saveButton,    3, 0 );
    grid->activate();

    idIndex = staffCursor.index( "id" );
    staffCursor.select( idIndex );
    staffCursor.first();

    sqlForm = new QSqlForm( this );
    sqlForm->setRecord( staffCursor.primeUpdate() );
    sqlForm->insert( forenameEdit, "forename" );
    sqlForm->insert( surnameEdit, "surname" );
    sqlForm->insert( salaryEdit, "salary" );
    sqlForm->readFields();
}


FormDialog::~FormDialog()
{

}


void FormDialog::save()
{
    sqlForm->writeFields();
    staffCursor.update();
    staffCursor.select( idIndex );
    staffCursor.first();
}


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    if ( ! createConnections() )
	return 1;

    FormDialog *formDialog = new FormDialog();
    formDialog->show();
    app.setMainWidget( formDialog );

    return app.exec();
}
