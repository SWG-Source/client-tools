/****************************************************************************
** $Id: buttongroups.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "buttongroups.h"

#include <qpopupmenu.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qpushbutton.h>

/*
 * Constructor
 *
 * Creates all child widgets of the ButtonGroups window
 */

ButtonsGroups::ButtonsGroups( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    // Create Widgets which allow easy layouting
    QVBoxLayout *vbox = new QVBoxLayout( this, 11, 6 );
    QHBoxLayout *box1 = new QHBoxLayout( vbox );
    QHBoxLayout *box2 = new QHBoxLayout( vbox );

    // ------- first group

    // Create an exclusive button group
    QButtonGroup *bgrp1 = new QButtonGroup( 1, QGroupBox::Horizontal, "Button Group 1 (exclusive)", this);
    box1->addWidget( bgrp1 );
    bgrp1->setExclusive( TRUE );

    // insert 3 radiobuttons
    QRadioButton *rb11 = new QRadioButton( "&Radiobutton 1", bgrp1 );
    rb11->setChecked( TRUE );
    (void)new QRadioButton( "R&adiobutton 2", bgrp1 );
    (void)new QRadioButton( "Ra&diobutton 3", bgrp1 );

    // ------- second group

    // Create a non-exclusive buttongroup
    QButtonGroup *bgrp2 = new QButtonGroup( 1, QGroupBox::Horizontal, "Button Group 2 (non-exclusive)", this );
    box1->addWidget( bgrp2 );
    bgrp2->setExclusive( FALSE );

    // insert 3 checkboxes
    (void)new QCheckBox( "&Checkbox 1", bgrp2 );
    QCheckBox *cb12 = new QCheckBox( "C&heckbox 2", bgrp2 );
    cb12->setChecked( TRUE );
    QCheckBox *cb13 = new QCheckBox( "Triple &State Button", bgrp2 );
    cb13->setTristate( TRUE );
    cb13->setChecked( TRUE );

    // ------------ third group

    // create a buttongroup which is exclusive for radiobuttons and non-exclusive for all other buttons
    QButtonGroup *bgrp3 = new QButtonGroup( 1, QGroupBox::Horizontal, "Button Group 3 (Radiobutton-exclusive)", this );
    box2->addWidget( bgrp3 );
    bgrp3->setRadioButtonExclusive( TRUE );

    // insert three radiobuttons
    rb21 = new QRadioButton( "Rad&iobutton 1", bgrp3 );
    rb22 = new QRadioButton( "Radi&obutton 2", bgrp3 );
    rb23 = new QRadioButton( "Radio&button 3", bgrp3 );
    rb23->setChecked( TRUE );

    // insert a checkbox...
    state = new QCheckBox( "E&nable Radiobuttons", bgrp3 );
    state->setChecked( TRUE );
    // ...and connect its SIGNAL clicked() with the SLOT slotChangeGrp3State()
    connect( state, SIGNAL( clicked() ), this, SLOT( slotChangeGrp3State() ) );

    // ------------ fourth group

    // create a groupbox which layouts its childs in a columns
    QGroupBox *bgrp4 = new QButtonGroup( 1, QGroupBox::Horizontal, "Groupbox with normal buttons", this );
    box2->addWidget( bgrp4 );

    // insert four pushbuttons...
    (void)new QPushButton( "&Push Button", bgrp4, "push" );

    // now make the second one a toggle button
    QPushButton *tb2 = new QPushButton( "&Toggle Button", bgrp4, "toggle" );
    tb2->setToggleButton( TRUE );
    tb2->setOn( TRUE );

    // ... and make the third one a flat button
    QPushButton *tb3 = new QPushButton( "&Flat Button", bgrp4, "flat" );
    tb3->setFlat(TRUE);

    // .. and the fourth a button with a menu
    QPushButton *tb4 = new QPushButton( "Popup Button", bgrp4, "popup" );
    QPopupMenu *menu = new QPopupMenu(tb4);
    menu->insertItem("Item1", 0);
    menu->insertItem("Item2", 1);
    menu->insertItem("Item3", 2);
    menu->insertItem("Item4", 3);
    tb4->setPopup(menu);
}

/*
 * SLOT slotChangeGrp3State()
 *
 * enables/disables the radiobuttons of the third buttongroup
 */

void ButtonsGroups::slotChangeGrp3State()
{
    rb21->setEnabled( state->isChecked() );
    rb22->setEnabled( state->isChecked() );
    rb23->setEnabled( state->isChecked() );
}
