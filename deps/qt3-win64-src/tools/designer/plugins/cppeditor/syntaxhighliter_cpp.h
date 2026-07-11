/**********************************************************************
**
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
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

#ifndef QCPPSYNTAXHIGHLIGHTER_H
#define QCPPSYNTAXHIGHLIGHTER_H

#include <private/qrichtext_p.h>
#include <conf.h>

class SyntaxHighlighter_CPP : public QTextPreProcessor
{
public:
    enum CppIds {
	Comment = 1,
	Number,
	String,
	Type,
	Keyword,
	PreProcessor,
	Label
    };

    SyntaxHighlighter_CPP();
    virtual ~SyntaxHighlighter_CPP();
    void process( QTextDocument *doc, QTextParagraph *string, int start, bool invalidate = TRUE );
    void updateStyles( const QMap<QString, ConfigStyle> &styles );

    static const char * const keywords[];

    QTextFormat *format( int id );

private:
    void addFormat( int id, QTextFormat *f );
    void removeFormat( int id );

    QTextFormat *lastFormat;
    int lastFormatId;
    QIntDict<QTextFormat> formats;

};

#endif
