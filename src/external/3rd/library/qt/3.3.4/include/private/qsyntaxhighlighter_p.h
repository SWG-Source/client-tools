/****************************************************************************
** $Id: qt/qsyntaxhighlighter_p.h   3.3.4   edited Nov 11 2003 $
**
** Definition of the internal QSyntaxHighlighterInternal class
**
** Created : 031111
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QSYNTAXHIGHLIGHTER_P_H
#define QSYNTAXHIGHLIGHTER_P_H

#ifndef QT_NO_SYNTAXHIGHLIGHTER
#include "qsyntaxhighlighter.h"
#include "private/qrichtext_p.h"

class QSyntaxHighlighterPrivate
{
public:
    QSyntaxHighlighterPrivate() :
	currentParagraph( -1 )
	{}

    int currentParagraph;
};

class QSyntaxHighlighterInternal : public QTextPreProcessor
{
public:
    QSyntaxHighlighterInternal( QSyntaxHighlighter *h ) : highlighter( h ) {}
    void process( QTextDocument *doc, QTextParagraph *p, int, bool invalidate ) {
	if ( p->prev() && p->prev()->endState() == -1 )
	    process( doc, p->prev(), 0, FALSE );

	highlighter->para = p;
	QString text = p->string()->toString();
	int endState = p->prev() ? p->prev()->endState() : -2;
	int oldEndState = p->endState();
	highlighter->d->currentParagraph = p->paragId();
	p->setEndState( highlighter->highlightParagraph( text, endState ) );
	highlighter->d->currentParagraph = -1;
	highlighter->para = 0;

	p->setFirstPreProcess( FALSE );
	QTextParagraph *op = p;
	p = p->next();
	if ( (!!oldEndState || !!op->endState()) && oldEndState != op->endState() &&
	     invalidate && p && !p->firstPreProcess() && p->endState() != -1 ) {
	    while ( p ) {
		if ( p->endState() == -1 )
		    return;
		p->setEndState( -1 );
		p = p->next();
	    }
	}
    }
    QTextFormat *format( int ) { return 0; }

private:
    QSyntaxHighlighter *highlighter;

    friend class QTextEdit;
};

#endif // QT_NO_SYNTAXHIGHLIGHTER
#endif // QSYNTAXHIGHLIGHTER_P_H
