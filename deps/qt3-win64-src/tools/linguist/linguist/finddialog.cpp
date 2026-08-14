/**********************************************************************
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Linguist.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/*  TRANSLATOR FindDialog

    Choose Edit|Find from the menu bar or press Ctrl+F to pop up the 
    Find dialog
*/

#include "finddialog.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

FindDialog::FindDialog( bool replace, QWidget *parent, const char *name,
			bool modal )
    : QDialog( parent, name, modal )
{
    sourceText = 0;
    
    led = new QLineEdit( this, "find line edit" );
    QLabel *findWhat = new QLabel( led, tr("Fi&nd what:"), this, "find what" );
    QLabel *replaceWith = 0;
    QPushButton *findNxt = new QPushButton( tr("&Find Next"), this,
					    "find next" );
    findNxt->setDefault( TRUE );
    connect( findNxt, SIGNAL(clicked()), this, SLOT(emitFindNext()) );
    QPushButton *cancel = new QPushButton( tr("Cancel"), this, "cancel find" );
    connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );

    QVBoxLayout *bl = new QVBoxLayout( 6, "find button layout" );
    bl->addWidget( findNxt );

    matchCase = new QCheckBox( tr("&Match case"), this, "find match case" );
    matchCase->setChecked( FALSE );

    if ( replace ) {
	QWhatsThis::add( this, tr("This window allows you to search and replace"
				  " some text in the translations.") );

	red = new QLineEdit( this, "replace line edit" );
	replaceWith = new QLabel( red, tr("Replace &with:"), this,
				  "replace with" );
	setTabOrder( led, red );

	QPushButton *replace = new QPushButton( tr("&Replace"), this,
						"replace" );
	connect( replace, SIGNAL(clicked()), this, SLOT(emitReplace()) );
	QPushButton *replaceAll = new QPushButton( tr("Replace &All"), this,
						   "replace all" );
	connect( replaceAll, SIGNAL(clicked()), this, SLOT(emitReplaceAll()) );
	setTabOrder( findNxt, replace );
	setTabOrder( replace, replaceAll );
	setTabOrder( replaceAll, cancel );

	QWhatsThis::add( replace, tr("Click here to replace the next occurrence"
				     " of the text you typed in.") );
	QWhatsThis::add( replaceAll, tr("Click here to replace all occurrences"
					" of the text you typed in.") );

	bl->addWidget( replace );
	bl->addWidget( replaceAll );

	QGridLayout *gl = new QGridLayout( this, 4, 3, 11, 11,
					   "find outer layout" );
	gl->addWidget( findWhat, 0, 0 );
	gl->addWidget( led, 0, 1 );
	gl->addWidget( replaceWith, 1, 0 );
	gl->addWidget( red, 1, 1 );
	gl->addMultiCellWidget( matchCase, 2, 2, 0, 1 );
	gl->addMultiCell( bl, 0, 3, 3, 3 );
    } else {
	QWhatsThis::add( this, tr("This window allows you to search for some"
				  " text in the translation source file.") );

	sourceText = new QCheckBox( tr("&Source texts"), this,
				    "find in source texts" );
	sourceText->setChecked( TRUE );
	translations = new QCheckBox( tr("&Translations"), this,
				      "find in translations" );
	translations->setChecked( TRUE );
	comments = new QCheckBox( tr("&Comments"), this, "find in comments" );
	comments->setChecked( TRUE );

	QWhatsThis::add( sourceText, tr("Source texts are searched when"
					" checked.") );
	QWhatsThis::add( translations, tr("Translations are searched when"
					  " checked.") );
	QWhatsThis::add( comments, tr("Comments and contexts are searched when"
				      " checked.") );

	QVBoxLayout *cl = new QVBoxLayout( 6, "find checkbox layout" );

	QGridLayout *gl = new QGridLayout( this, 3, 4, 11, 11,
					   "find outer layout" );
	gl->addWidget( findWhat, 0, 0 );
	gl->addMultiCellWidget( led, 0, 0, 1, 2 );
	gl->addWidget( matchCase, 1, 2 );
	gl->addMultiCell( bl, 0, 2, 3, 3 );
	gl->addMultiCell( cl, 1, 2, 0, 1 );
	gl->setColStretch( 0, 0 );
	gl->addColSpacing( 1, 40 );
	gl->setColStretch( 2, 1 );
	gl->setColStretch( 3, 0 );

	cl->addWidget( sourceText );
	cl->addWidget( translations );
	cl->addWidget( comments );
	cl->addStretch( 1 );
    }

    QWhatsThis::add( led, tr("Type in the text to search for.") );

    QWhatsThis::add( matchCase, tr("Texts such as 'TeX' and 'tex' are"
				   " considered as different when checked.") );
    QWhatsThis::add( findNxt, tr("Click here to find the next occurrence of the"
				 " text you typed in.") );
    QWhatsThis::add( cancel, tr("Click here to close this window.") );

    bl->addWidget( cancel );
    bl->addStretch( 1 );

    resize( 400, 1 );
    setMaximumHeight( height() );

    led->setFocus();
}

void FindDialog::emitFindNext()
{
    int where;
    if ( sourceText != 0 )
	where = ( sourceText->isChecked() ? SourceText : 0 ) |
		( translations->isChecked() ? Translations : 0 ) |
		( comments->isChecked() ? Comments : 0 );
    else
	where = Translations;
    emit findNext( led->text(), where, matchCase->isChecked() );
}

void FindDialog::emitReplace()
{
    emit replace( led->text(), red->text(), matchCase->isChecked(), FALSE );
}

void FindDialog::emitReplaceAll()
{
    emit replace( led->text(), red->text(), matchCase->isChecked(), TRUE );
}
