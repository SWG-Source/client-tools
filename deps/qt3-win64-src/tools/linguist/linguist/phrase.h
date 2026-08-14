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

#ifndef PHRASE_H
#define PHRASE_H

#include <qstring.h>
#include <qvaluelist.h>

class Phrase
{
public:
    Phrase() { }
    Phrase( const QString& source, const QString& target,
	    const QString& definition );

    QString source() const { return s; }
    void setSource( const QString& ns ) { s = ns; }
    QString target() const { return t; }
    void setTarget( const QString& nt ) { t = nt; }
    QString definition() const { return d; }
    void setDefinition ( const QString& nd ) { d = nd; }

private:
    QString s;
    QString t;
    QString d;
};

bool operator==( const Phrase& p, const Phrase& q );
inline bool operator!=( const Phrase& p, const Phrase& q ) {
    return !( p == q );
}

class PhraseBook : public QValueList<Phrase>
{
public:
    PhraseBook() { }

    bool load( const QString& filename );
    bool save( const QString& filename ) const;
};

#endif
