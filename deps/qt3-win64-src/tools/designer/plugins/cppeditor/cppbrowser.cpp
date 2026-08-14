/**********************************************************************
**
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Designer.
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

#include "cppbrowser.h"
#include <private/qrichtext_p.h>
#include <qprocess.h>
#include <qmainwindow.h>
#include <qstatusbar.h>
#include <editor.h>

CppEditorBrowser::CppEditorBrowser( Editor *e )
    : EditorBrowser( e )
{
}

void CppEditorBrowser::showHelp( const QString &w )
{
    QString word( w );
    if ( word[ 0 ] == 'Q' ) {
	if ( word[ (int)word.length() - 1 ] == '&' ||
	     word[ (int)word.length() - 1 ] == '*' )
	    word.remove( word.length() - 1, 1 );
	word = word.lower() + ".html";
	QStringList lst;
	lst << "assistant" << "-file" << word;
	QProcess proc( lst );
	proc.start();
	return;
    }

    if ( word.find( '(' ) != -1 ) {
	QString txt = "::" + word.left( word.find( '(' ) );
	QTextDocument *doc = curEditor->document();
	QTextParagraph *p = doc->firstParagraph();
	while ( p ) {
	    if ( p->string()->toString().find( txt ) != -1 ) {
		curEditor->setCursorPosition( p->paragId(), 0 );
		return;
	    }
	    p = p->next();
	}
    }

    QMainWindow *mw = ::qt_cast<QMainWindow*>(curEditor->topLevelWidget());
    if ( mw )
	mw->statusBar()->message( tr( "Nothing available for '%1'" ).arg( w ), 1500 );
}
