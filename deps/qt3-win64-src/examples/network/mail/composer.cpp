/****************************************************************************
** $Id: composer.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "composer.h"
#include "smtp.h"

#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>

Composer::Composer( QWidget *parent )
    : QWidget( parent )
{
    QGridLayout * layout = new QGridLayout( this, 1, 1, 6 );

    layout->addWidget( new QLabel( tr( "From:" ), this ), 0, 0 );
    from = new QLineEdit( this );
    layout->addWidget( from, 0, 1 );

    layout->addWidget( new QLabel( tr( "To:" ), this ), 1, 0 );
    to = new QLineEdit( this );
    layout->addWidget( to, 1, 1 );

    layout->addWidget( new QLabel( tr( "Subject:" ), this ), 2, 0 );
    subject = new QLineEdit( this );
    layout->addWidget( subject, 2, 1 );

    message = new QMultiLineEdit( this );
    layout->addMultiCellWidget( message, 3, 3, 0, 1 );

    send = new QPushButton( tr( "&Send" ), this );
    layout->addWidget( send, 4, 0 );
    connect( send, SIGNAL( clicked() ), this, SLOT( sendMessage() ) );

    sendStatus = new QLabel( this );
    layout->addWidget( sendStatus, 4, 1 );
}


void Composer::sendMessage()
{
    send->setEnabled( FALSE );
    sendStatus->setText( tr( "Looking up mail servers" ) );
    Smtp *smtp = new Smtp( from->text(), to->text(),
			   subject->text(),
			   message->text() );
    connect( smtp, SIGNAL(destroyed()),
	     this, SLOT(enableSend()) );
    connect( smtp, SIGNAL(status(const QString &)),
	     sendStatus, SLOT(setText(const QString &)) );
}


void Composer::enableSend()
{
    send->setEnabled( TRUE );
}
