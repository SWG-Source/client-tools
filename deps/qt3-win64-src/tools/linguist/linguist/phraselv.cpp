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

/*  TRANSLATOR PhraseLV

  The phrase list in the right panel of the main window (with Source phrase,
  Target phrase, and Definition in its header) is a PhraseLV object.
*/

#include "phraselv.h"

#include <qregexp.h>
#include <qwhatsthis.h>
#include <qheader.h>

class WhatPhrase : public QWhatsThis
{
public:
    WhatPhrase( PhraseLV *w );

    virtual QString text( const QPoint& p );

private:
    PhraseLV *parent;
};

WhatPhrase::WhatPhrase( PhraseLV *w )
    : QWhatsThis( w )
{
    parent = w;
}

QString WhatPhrase::text( const QPoint& p )
{
    QListViewItem *item = parent->itemAt( p );
    if ( item == 0 )
	return PhraseLV::tr( "This is a list of phrase entries relevant to the"
		  " source text.  Each phrase is supplemented with a suggested"
		  " translation and a definition." );
    else
	return QString( PhraseLV::tr("<p><u>%1:</u>&nbsp;&nbsp;%2</p>"
				     "<p><u>%3:</u>&nbsp;&nbsp;%4</p>"
				     "<p><u>%5:</u>&nbsp;&nbsp;%6</p>") )
	       .arg( parent->columnText(PhraseLVI::SourceTextShown) )
	       .arg( item->text(PhraseLVI::SourceTextShown) )
	       .arg( parent->columnText(PhraseLVI::TargetTextShown) )
	       .arg( item->text(PhraseLVI::TargetTextShown) )
	       .arg( parent->columnText(PhraseLVI::DefinitionText) )
	       .arg( item->text(PhraseLVI::DefinitionText) );
}

PhraseLVI::PhraseLVI( PhraseLV *parent, const Phrase& phrase, int accelKey )
    : QListViewItem( parent ),
      akey( accelKey )
{
    setPhrase( phrase );
}

QString PhraseLVI::key( int column, bool ascending ) const
{
    if ( column == SourceTextShown ) {
	if ( sourceTextKey.isEmpty() ) {
	    if ( ascending ) {
		return "";
	    } else {
		return QString::null;
	    }
	} else {
	    return sourceTextKey;	
	}
    } else if ( column == TargetTextShown ) {
	return targetTextKey;
    } else {
	return QChar( '0' + akey ) + text( column );
    }
}

void PhraseLVI::setText( int column, const QString& text )
{
    if ( column == SourceTextShown ) {
	sourceTextKey = makeKey( text );
    } else if ( column == TargetTextShown ) {
	targetTextKey = makeKey( text );
    }
    QListViewItem::setText( column, text );
}

void PhraseLVI::setPhrase( const Phrase& phrase )
{
    setText( SourceTextShown, phrase.source().simplifyWhiteSpace() );
    setText( TargetTextShown, phrase.target().simplifyWhiteSpace() );
    setText( DefinitionText, phrase.definition() );
    setText( SourceTextOriginal, phrase.source() );
    setText( TargetTextOriginal, phrase.target() );
}

Phrase PhraseLVI::phrase() const
{
    return Phrase( text(SourceTextOriginal), text(TargetTextOriginal),
		   text(DefinitionText) );
}

QString PhraseLVI::makeKey( const QString& text ) const
{
    if ( text == NewPhrase )
	return QString::null;

    QString key;
    for ( int i = 0; i < (int) text.length(); i++ ) {
	if ( text[i] != QChar('&') )
	    key += text[i].lower();
    }
    // see Section 5, Exercise 4 of The Art of Computer Programming
    key += QChar::null;
    key += text;
    return key;
}

PhraseLV::PhraseLV( QWidget *parent, const char *name )
    : QListView( parent, name )
{
    setAllColumnsShowFocus( TRUE );
    setShowSortIndicator( TRUE );
    for ( int i = 0; i < 3; i++ )
	addColumn( QString::null, 120 );
    setColumnText( PhraseLVI::SourceTextShown, tr("Source phrase") );
    setColumnText( PhraseLVI::TargetTextShown, tr("Translation") );
    setColumnText( PhraseLVI::DefinitionText, tr("Definition") );
    header()->setStretchEnabled( TRUE, -1 );
    what = new WhatPhrase( this );
}

PhraseLV::~PhraseLV()
{
// delete what;
}

QSize PhraseLV::sizeHint() const
{
    return QSize( QListView::sizeHint().width(), 50 );
}
