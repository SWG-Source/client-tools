/****************************************************************************
** $Id: mainwindow.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qassistantclient.h>
#include <qfiledialog.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qstatusbar.h>
#include <qtable.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>

#include "mainwindow.h"
#include "tooltip.h"
#include "whatsthis.h"

MainWindow::MainWindow()
{
    statusBar();
    assistant = new QAssistantClient( QDir("../../bin").absPath(), this );

    QTable* table = new QTable( 2, 3, this );
    setCentralWidget( table );

    // populate table
    QStringList comboEntries;
    comboEntries << "one" << "two" << "three" << "four";
    QComboTableItem* comboItem1 = new QComboTableItem( table, comboEntries );
    QComboTableItem* comboItem2 = new QComboTableItem( table, comboEntries );
    QCheckTableItem* checkItem1 = new QCheckTableItem( table, "Check me" );
    QCheckTableItem* checkItem2 = new QCheckTableItem( table, "Check me" );

    table->setItem( 0, 0, comboItem1 );
    table->setItem( 1, 0, comboItem2 );

    table->setItem( 1, 1, checkItem1  );
    table->setItem( 0, 1, checkItem2 );

    table->setText( 1, 2, "Text" );

    table->horizontalHeader()->setLabel( 0, " Combos" );
    table->horizontalHeader()->setLabel( 1, "Checkboxes" );
    table->verticalHeader()->setLabel( 0, "1" );
    table->verticalHeader()->setLabel( 1, "2" );


    // populate menubar
    QPopupMenu* fileMenu = new QPopupMenu( this );
    QPopupMenu* helpMenu = new QPopupMenu( this );

    menuBar()->insertItem( "&File", fileMenu );
    menuBar()->insertItem( "&Help", helpMenu );

    int fileId = fileMenu->insertItem( "E&xit", this, SLOT(close()) );

    int helpId = helpMenu->insertItem( "Open Assistant", this, SLOT(assistantSlot()) );

    // populate toolbar
    QToolBar* toolbar = new QToolBar( this );
    QToolButton* assistantButton = new QToolButton( toolbar );
    assistantButton->setIconSet( QPixmap("appicon.png") );
    QWhatsThis::whatsThisButton( toolbar );

    //create tooltipgroup
    QToolTipGroup * tipGroup = new QToolTipGroup( this );
    connect( tipGroup, SIGNAL(showTip(const QString&)), statusBar(),
	SLOT(message(const QString&)) );
    connect( tipGroup, SIGNAL(removeTip()), statusBar(), SLOT(clear()) );

    // set up tooltips
    QToolTip::add( assistantButton, tr ("Open Assistant"), tipGroup, "Opens Qt Assistant" );

    horizontalTip = new HeaderToolTip( table->horizontalHeader(), tipGroup );
    verticalTip = new HeaderToolTip( table->verticalHeader(), tipGroup );

    cellTip = new TableToolTip( table, tipGroup );

    // set up whats this
    QWhatsThis::add ( assistantButton, "This is a toolbutton which opens Assistant" );

    HeaderWhatsThis *horizontalWhatsThis = new HeaderWhatsThis( table->horizontalHeader() );
    HeaderWhatsThis *verticalWhatsThis = new HeaderWhatsThis( table->verticalHeader() );

    TableWhatsThis *cellWhatsThis = new TableWhatsThis( table );

    fileMenu->setWhatsThis( fileId, "Click here to exit the application" );
    helpMenu->setWhatsThis( helpId, "Click here to open Assistant" );

    // connections
    connect( assistantButton, SIGNAL(clicked()), this, SLOT(assistantSlot()) );
    connect( horizontalWhatsThis, SIGNAL(linkClicked(const QString&)), assistant,
	SLOT(showPage(const QString&)) );
    connect( verticalWhatsThis, SIGNAL(linkClicked(const QString&)), assistant,
	SLOT(showPage(const QString&)) );
    connect( cellWhatsThis, SIGNAL(linkClicked(const QString&)), assistant,
	SLOT(showPage(const QString&)) );
}

MainWindow::~MainWindow()
{
    delete horizontalTip;
    delete verticalTip;
    delete cellTip;
}

void MainWindow::assistantSlot()
{
    QString docsPath = QDir("../../doc").absPath();
    assistant->showPage( QString("%1/html/qassistantclient.html").arg(docsPath) );
}
