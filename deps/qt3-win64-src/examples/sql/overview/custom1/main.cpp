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


CustomEdit::CustomEdit( QWidget *parent, const char *name ) : 
    QLineEdit( parent, name )
{
    connect( this, SIGNAL(textChanged(const QString &)), 
	     this, SLOT(changed(const QString &)) );
}


void CustomEdit::changed( const QString &line )
{
    setUpperLine( line );
}


void CustomEdit::setUpperLine( const QString &line )
{
    upperLineText = line.upper();
    setText( upperLineText );
}


QString CustomEdit::upperLine() const 
{
    return upperLineText;
}


FormDialog::FormDialog()
{
    QLabel	*forenameLabel	= new QLabel( "Forename:", this );
    CustomEdit	*forenameEdit	= new CustomEdit( this );
    QLabel	*surnameLabel   = new QLabel( "Surname:", this );
    CustomEdit	*surnameEdit	= new CustomEdit( this );
    QLabel	*salaryLabel	= new QLabel( "Salary:", this );
    QLineEdit	*salaryEdit	= new QLineEdit( this );
    salaryEdit->setAlignment( Qt::AlignRight );
    QPushButton *saveButton	= new QPushButton( "&Save", this );
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

    staffCursor = new QSqlCursor( "staff" );
    staffCursor->setTrimmed( "forename", TRUE );
    staffCursor->setTrimmed( "surname",  TRUE );
    idIndex = staffCursor->index( "id" );
    staffCursor->select( idIndex );
    staffCursor->first();

    propMap = new QSqlPropertyMap;
    propMap->insert( forenameEdit->className(), "upperLine" );

    sqlForm = new QSqlForm( this );
    sqlForm->setRecord( staffCursor->primeUpdate() );
    sqlForm->installPropertyMap( propMap );
    sqlForm->insert( forenameEdit, "forename" );
    sqlForm->insert( surnameEdit, "surname" );
    sqlForm->insert( salaryEdit, "salary" );
    sqlForm->readFields();
}


FormDialog::~FormDialog()
{
    delete staffCursor;
}


void FormDialog::save()
{
    sqlForm->writeFields();
    staffCursor->update();
    staffCursor->select( idIndex );
    staffCursor->first();
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
